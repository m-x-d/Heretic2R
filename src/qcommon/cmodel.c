//
// cmodel.c -- model loading
//
// Copyright 1998 Raven Software
//

#include "cmodel.h"
#include "cmodel_private.h"

char map_name[MAX_QPATH];

int numbrushsides;
cbrushside_t map_brushsides[MAX_MAP_BRUSHSIDES];

int numtexinfo;
csurface_t map_surfaces[MAX_MAP_TEXINFO];

int numplanes;
cplane_t map_planes[MAX_MAP_PLANES + 6]; // Extra for box hull

int numplanes;

int numnodes;
cnode_t map_nodes[MAX_MAP_NODES + 6]; // Extra for box hull

static int numleafs = 1; // Allow leaf funcs to be called without a map.
cleaf_t map_leafs[MAX_MAP_LEAFS];
int emptyleaf;

int numleafbrushes;
ushort map_leafbrushes[MAX_MAP_LEAFBRUSHES];

int numcmodels;
cmodel_t map_cmodels[MAX_MAP_MODELS];

int numbrushes;
cbrush_t map_brushes[MAX_MAP_BRUSHES];

int numvisibility;

int numentitychars;
char map_entitystring[MAX_MAP_ENTSTRING];

int numareas = 1;
int numclusters = 1;

qboolean portalopen[MAX_MAP_AREAPORTALS];

cvar_t* map_noareas;

int c_pointcontents;
int c_traces;
int c_brush_traces;

static byte* cmod_base;

static void CM_InitBoxHull(void)
{
	NOT_IMPLEMENTED
}

static void CMod_LoadSurfaces(const lump_t* l)
{
	texinfo_t* in = (void*)(cmod_base + l->fileofs);

	if (l->filelen % sizeof(*in))
		Com_Error(ERR_DROP, "MOD_LoadBmodel: funny lump size");

	const int count = l->filelen / (int)sizeof(*in);

	if (count < 1)
		Com_Error(ERR_DROP, "Map with no surfaces");

	if (count >= MAX_MAP_TEXINFO) //mxd. '>' in Q2 and original logic.
		Com_Error(ERR_DROP, "Map has too many surfaces");

	numtexinfo = count;
	csurface_t* out = map_surfaces; // Q2: mapsurface_t.

	for (int i = 0; i < count; i++, in++, out++)
	{
		strncpy_s(out->name, sizeof(out->name), in->texture, sizeof(out->name) - 1); //mxd. strncpy -> strncpy_s
		out->flags = in->flags;
		out->value = in->value;
	}
}

// Q2 counterpart
static void CMod_LoadLeafs(const lump_t* l)
{
	dleaf_t* in = (void*)(cmod_base + l->fileofs);

	if (l->filelen % sizeof(*in))
		Com_Error(ERR_DROP, "MOD_LoadBmodel: funny lump size");

	const int count = l->filelen / (int)sizeof(*in);

	if (count < 1)
		Com_Error(ERR_DROP, "Map with no leafs");

	// Need to save space for box planes.
	if (count >= MAX_MAP_PLANES) //mxd. '>' in Q2 and original logic.
		Com_Error(ERR_DROP, "Map has too many planes");

	cleaf_t* out = map_leafs;
	numclusters = 0;
	numleafs = count;

	for (int i = 0; i < count; i++, in++, out++)
	{
		out->contents = in->contents;
		out->cluster = in->cluster;
		out->area = in->area;
		out->firstleafbrush = in->firstleafbrush;
		out->numleafbrushes = in->numleafbrushes;

		if (out->cluster >= numclusters)
			numclusters = out->cluster + 1;
	}

	if (map_leafs[0].contents != CONTENTS_SOLID)
		Com_Error(ERR_DROP, "Map leaf 0 is not CONTENTS_SOLID");

	//solidleaf = 0; //mxd. Unused
	emptyleaf = -1;

	for (int i = 1; i < numleafs; i++)
	{
		if (map_leafs[i].contents == 0)
		{
			emptyleaf = i;
			break;
		}
	}

	if (emptyleaf == -1)
		Com_Error(ERR_DROP, "Map does not have an empty leaf");
}

// Q2 counterpart
static void CMod_LoadLeafBrushes(const lump_t* l)
{
	ushort* in = (void*)(cmod_base + l->fileofs);

	if (l->filelen % sizeof(*in))
		Com_Error(ERR_DROP, "MOD_LoadBmodel: funny lump size");

	const int count = l->filelen / (int)sizeof(*in);

	if (count < 1)
		Com_Error(ERR_DROP, "Map with no planes");

	// Need to save space for box planes.
	if (count >= MAX_MAP_LEAFBRUSHES) //mxd. '>' in Q2 and original logic.
		Com_Error(ERR_DROP, "Map has too many leafbrushes");

	ushort* out = map_leafbrushes;
	numleafbrushes = count;

	for (int i = 0; i < count; i++, in++, out++)
		*out = *in;
}

// Q2 counterpart
static void CMod_LoadPlanes(const lump_t* l)
{
	dplane_t* in = (void*)(cmod_base + l->fileofs);

	if (l->filelen % sizeof(*in))
		Com_Error(ERR_DROP, "MOD_LoadBmodel: funny lump size");

	const int count = l->filelen / (int)sizeof(*in);

	if (count < 1)
		Com_Error(ERR_DROP, "Map with no planes");

	// Need to save space for box planes.
	if (count >= MAX_MAP_PLANES) //mxd. '>' in Q2 and original logic.
		Com_Error(ERR_DROP, "Map has too many planes");

	cplane_t* out = map_planes;
	numplanes = count;

	for (int i = 0; i < count; i++, in++, out++)
	{
		byte bits = 0;
		for (int j = 0; j < 3; j++)
		{
			out->normal[j] = in->normal[j];
			if (out->normal[j] < 0)
				bits |= 1 << j;
		}

		out->dist = in->dist;
		out->type = (byte)in->type;
		out->signbits = bits;
	}
}

// Q2 counterpart
static void CMod_LoadBrushes(const lump_t* l)
{
	dbrush_t* in = (void*)(cmod_base + l->fileofs);

	if (l->filelen % sizeof(*in))
		Com_Error(ERR_DROP, "MOD_LoadBmodel: funny lump size");

	const int count = l->filelen / (int)sizeof(*in);

	if (count >= MAX_MAP_BRUSHES) //mxd. '>' in Q2 and original logic.
		Com_Error(ERR_DROP, "Map has too many brushes");

	cbrush_t* out = map_brushes;
	numbrushes = count;

	for (int i = 0; i < count; i++, out++, in++)
	{
		out->firstbrushside = in->firstside;
		out->numsides = in->numsides;
		out->contents = in->contents;
	}
}

// Q2 counterpart
static void CMod_LoadBrushSides(const lump_t* l)
{
	dbrushside_t* in = (void*)(cmod_base + l->fileofs);

	if (l->filelen % sizeof(*in))
		Com_Error(ERR_DROP, "MOD_LoadBmodel: funny lump size");

	const int count = l->filelen / (int)sizeof(*in);

	// Need to save space for box planes.
	if (count >= MAX_MAP_BRUSHSIDES) //mxd. '>' in Q2 and original logic.
		Com_Error(ERR_DROP, "Map has too many planes");

	cbrushside_t* out = map_brushsides;
	numbrushsides = count;

	for (int i = 0; i < count; i++, in++, out++)
	{
		out->plane = &map_planes[in->planenum];

		if (in->texinfo >= numtexinfo)
			Com_Error(ERR_DROP, "Bad brushside texinfo");

		out->surface = &map_surfaces[in->texinfo];
	}
}

// Q2 counterpart
static void CMod_LoadSubmodels(const lump_t* l)
{
	dmodel_t* in = (void*)(cmod_base + l->fileofs);

	if (l->filelen % sizeof(*in))
		Com_Error(ERR_DROP, "MOD_LoadBmodel: funny lump size");

	const int count = l->filelen / (int)sizeof(*in);

	if (count < 1)
		Com_Error(ERR_DROP, "Map with no models");

	if (count >= MAX_MAP_MODELS) //mxd. '>' in Q2 and original logic.
		Com_Error(ERR_DROP, "Map has too many models");

	numcmodels = count;
	cmodel_t* out = map_cmodels;

	for (int i = 0; i < count; i++, in++, out++)
	{
		for (int j = 0; j < 3; j++)
		{
			// Spread the mins / maxs by a pixel.
			out->mins[j] = in->mins[j] - 1;
			out->maxs[j] = in->maxs[j] + 1;
			out->origin[j] = in->origin[j];
		}

		out->headnode = in->headnode;
	}
}

// Q2 counterpart
static void CMod_LoadNodes(const lump_t* l)
{
	dnode_t* in = (void*)(cmod_base + l->fileofs);

	if (l->filelen % sizeof(*in))
		Com_Error(ERR_DROP, "MOD_LoadBmodel: funny lump size");

	const int count = l->filelen / (int)sizeof(*in);

	if (count < 1)
		Com_Error(ERR_DROP, "Map has no nodes");

	if (count >= MAX_MAP_NODES) //mxd. '>' in Q2 and original logic.
		Com_Error(ERR_DROP, "Map has too many nodes");

	cnode_t* out = map_nodes;
	numnodes = count;

	for (int i = 0; i < count; i++, out++, in++)
	{
		out->plane = &map_planes[in->planenum];
		for (int j = 0; j < 2; j++)
			out->children[j] = in->children[j];
	}
}

static void CMod_LoadAreas(lump_t* l)
{
	NOT_IMPLEMENTED
}

static void CMod_LoadAreaPortals(lump_t* l)
{
	NOT_IMPLEMENTED
}

static void CMod_LoadVisibility(lump_t* l)
{
	NOT_IMPLEMENTED
}

static void CMod_LoadEntityString(lump_t* l)
{
	NOT_IMPLEMENTED
}

// Q2 counterpart
// Loads in the map and all submodels.
cmodel_t* CM_LoadMap(const char* name, const qboolean clientload, uint* checksum)
{
	static uint last_checksum;
	uint* buf;

	map_noareas = Cvar_Get("map_noareas", "0", 0);

	if (strcmp(map_name, name) == 0 && (clientload || !(int)Cvar_VariableValue("flushmap")))
	{
		*checksum = last_checksum;
		if (!clientload)
		{
			memset(portalopen, 0, sizeof(portalopen));
			FloodAreaConnections();
		}

		// Still have the right version
		return &map_cmodels[0];
	}

	// Free old stuff
	numplanes = 0;
	numnodes = 0;
	numleafs = 0;
	numcmodels = 0;
	numvisibility = 0;
	numentitychars = 0;
	map_entitystring[0] = 0;
	map_name[0] = 0;

	if (name == NULL || *name == 0)
	{
		numleafs = 1;
		numclusters = 1;
		numareas = 1;
		*checksum = 0;

		// Cinematic servers won't have anything at all.
		return &map_cmodels[0];
	}

	// Load the file.
	const int length = FS_LoadFile(name, (void**)&buf);
	if (buf == NULL)
	{
		Com_Error(ERR_DROP, "Couldn`t load %s", name);
		return NULL; //mxd. Added to prevent false-positive code analysis warning.
	}

	last_checksum = Com_BlockChecksum(buf, length);
	*checksum = last_checksum;

	dheader_t header = *(dheader_t*)buf;

	if (header.version != BSPVERSION)
		Com_Error(ERR_DROP, "CMod_LoadBrushModel: %s has wrong version number (%i should be %i)", name, header.version, BSPVERSION);

	cmod_base = (byte*)buf;

	// Load into heap.
	CMod_LoadSurfaces(&header.lumps[LUMP_TEXINFO]);
	CMod_LoadLeafs(&header.lumps[LUMP_LEAFS]);
	CMod_LoadLeafBrushes(&header.lumps[LUMP_LEAFBRUSHES]);
	CMod_LoadPlanes(&header.lumps[LUMP_PLANES]);
	CMod_LoadBrushes(&header.lumps[LUMP_BRUSHES]);
	CMod_LoadBrushSides(&header.lumps[LUMP_BRUSHSIDES]);
	CMod_LoadSubmodels(&header.lumps[LUMP_MODELS]);
	CMod_LoadNodes(&header.lumps[LUMP_NODES]);
	CMod_LoadAreas(&header.lumps[LUMP_AREAS]);
	CMod_LoadAreaPortals(&header.lumps[LUMP_AREAPORTALS]);
	CMod_LoadVisibility(&header.lumps[LUMP_VISIBILITY]);
	CMod_LoadEntityString(&header.lumps[LUMP_ENTITIES]);

	FS_FreeFile(buf);
	CM_InitBoxHull();

	memset(portalopen, 0, sizeof(portalopen));
	FloodAreaConnections();

	strcpy_s(map_name, sizeof(map_name), name);

	return &map_cmodels[0];
}

cmodel_t* CM_InlineModel(char* name)
{
	NOT_IMPLEMENTED
	return NULL;
}

// Q2 counterpart
int CM_NumInlineModels(void)
{
	return numcmodels;
}

int CM_LeafCluster(int leafnum)
{
	NOT_IMPLEMENTED
	return 0;
}

int CM_LeafArea(int leafnum)
{
	NOT_IMPLEMENTED
	return 0;
}

int CM_PointLeafnum(const vec3_t p)
{
	NOT_IMPLEMENTED
	return 0;
}

// Q2 counterpart
char* CM_EntityString(void)
{
	return map_entitystring;
}

byte* CM_ClusterPVS(int cluster)
{
	NOT_IMPLEMENTED
	return NULL;
}

byte* CM_ClusterPHS(int cluster)
{
	NOT_IMPLEMENTED
	return NULL;
}

void FloodAreaConnections(void)
{
	NOT_IMPLEMENTED
}

void CM_SetAreaPortalState(int portalnum, qboolean open)
{
	NOT_IMPLEMENTED
}

qboolean CM_AreasConnected(int area1, int area2)
{
	NOT_IMPLEMENTED
	return false;
}