//
// cmodel.c -- model loading
//
// Copyright 1998 Raven Software
//

#include <float.h>
#include "cmodel.h"
#include "cmodel_private.h"
#include "Vector.h"

static int checkcount;

static char map_name[MAX_QPATH];

static int numbrushsides;
static cbrushside_t map_brushsides[MAX_MAP_BRUSHSIDES]; // 524288 bytes

int numtexinfo;
csurface_t map_surfaces[MAX_MAP_TEXINFO];

static int numplanes;
static cplane_t map_planes[MAX_MAP_PLANES + 12]; // Extra for box hull. 1310840 bytes //BUGFIX: MAX_MAP_PLANES + 6 in both Q2 and H2.

static int numplanes;

static int numnodes;
static cnode_t map_nodes[MAX_MAP_NODES + 6]; // Extra for box hull. 524336 bytes

static int numleafs = 1; // Allow leaf funcs to be called without a map.
static cleaf_t map_leafs[MAX_MAP_LEAFS]; // 1048576 bytes
static int emptyleaf;

static int numleafbrushes;
static ushort map_leafbrushes[MAX_MAP_LEAFBRUSHES]; // 131072 bytes

static int numcmodels;
static cmodel_t map_cmodels[MAX_MAP_MODELS]; // 40960 bytes

static int numbrushes;
static cbrush_t map_brushes[MAX_MAP_BRUSHES]; // 163840 bytes

static int numvisibility;
static byte map_visibility[MAX_MAP_VISIBILITY]; // 1048576 bytes
static dvis_t* map_vis = (dvis_t*)map_visibility;

static int numentitychars;
static char map_entitystring[MAX_MAP_ENTSTRING]; // 262144 bytes

static int numareas = 1;
static carea_t map_areas[MAX_MAP_AREAS]; // 4096 bytes

static int numareaportals;
static dareaportal_t map_areaportals[MAX_MAP_AREAPORTALS]; // 8192 bytes

static int numclusters = 1;

static qboolean portalopen[MAX_MAP_AREAPORTALS]; // 4096 bytes

static cvar_t* map_noareas;

int c_pointcontents;
int c_traces;
int c_brush_traces;

//mxd. Used by CM_BoxLeafnums logic.
#define DIST_EPSILON	0.03125f // 1/32 epsilon to keep floating point happy

static int leaf_count;
static int leaf_maxcount;
static int* leaf_list;
static float* leaf_mins;
static float* leaf_maxs;
static int leaf_topnode;

//mxd. Used by CL_Trace() and SV_Trace() logic.
qboolean trace_check_water;

static vec3_t trace_start;
static vec3_t trace_end;
static vec3_t trace_mins;
static vec3_t trace_maxs;
static vec3_t trace_extents;

static trace_t* trace_trace;
static uint trace_contents; //mxd. int -> uint
static qboolean trace_ispoint;

//mxd. Used by CM_LoadMap logic.
static byte* cmod_base;

static cplane_t* box_planes;
static int box_headnode;
static csurface_t nullsurface; // Q2: mapsurface_t

// Q2 counterpart
// Set up the planes and nodes so that the six floats of a bounding box
// can just be stored out and get a proper clipping hull structure.
static void CM_InitBoxHull(void)
{
	box_headnode = numnodes;
	box_planes = &map_planes[numplanes];

	if (numnodes + 6 > MAX_MAP_NODES || numbrushes + 1 > MAX_MAP_BRUSHES || numleafbrushes + 1 > MAX_MAP_LEAFBRUSHES ||
		numbrushsides + 6 > MAX_MAP_BRUSHSIDES || numplanes + 12 > MAX_MAP_PLANES)
	{
		Com_Error(ERR_DROP, "Not enough room for box tree");
	}

	cbrush_t* box_brush = &map_brushes[numbrushes]; //mxd. Made 'box_brush' local.
	box_brush->numsides = 6;
	box_brush->firstbrushside = numbrushsides;
	box_brush->contents = CONTENTS_MONSTER;

	cleaf_t* box_leaf = &map_leafs[numleafs]; //mxd. Made 'box_leaf' local.
	box_leaf->contents = CONTENTS_MONSTER;
	box_leaf->firstleafbrush = (ushort)numleafbrushes;
	box_leaf->numleafbrushes = 1;

	map_leafbrushes[numleafbrushes] = (ushort)numbrushes;

	for (int i = 0; i < 6; i++)
	{
		const int side = i & 1;

		// Brush sides.
		cbrushside_t* s = &map_brushsides[numbrushsides + i];
		s->plane = map_planes + (numplanes + i * 2 + side);
		s->surface = &nullsurface;

		// Nodes.
		cnode_t* c = &map_nodes[box_headnode + i];
		c->plane = map_planes + (numplanes + i * 2);
		c->children[side] = -1 - emptyleaf;

		if (i != 5)
			c->children[side ^ 1] = box_headnode + i + 1;
		else
			c->children[side ^ 1] = -1 - numleafs;

		// Planes.
		cplane_t* p = &box_planes[i * 2];
		p->type = (byte)(i >> 1);
		p->signbits = 0;
		VectorClear(p->normal);
		p->normal[i >> 1] = 1;

		p = &box_planes[i * 2 + 1];
		p->type = (byte)(3 + (i >> 1));
		p->signbits = 0;
		VectorClear(p->normal);
		p->normal[i >> 1] = -1;
	}
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

	cnode_t* out = &map_nodes[0];
	numnodes = count;

	for (int i = 0; i < count; i++, out++, in++)
	{
		out->plane = &map_planes[in->planenum];
		for (int j = 0; j < 2; j++)
			out->children[j] = in->children[j];
	}
}

// Q2 counterpart
static void CMod_LoadAreas(const lump_t* l)
{
	darea_t* in = (void*)(cmod_base + l->fileofs);

	if (l->filelen % sizeof(*in))
		Com_Error(ERR_DROP, "MOD_LoadBmodel: funny lump size");

	const int count = l->filelen / (int)sizeof(*in);

	if (count >= MAX_MAP_AREAS) //mxd. '>' in Q2 and original logic.
		Com_Error(ERR_DROP, "Map has too many areas");

	carea_t* out = map_areas;
	numareas = count;

	for (int i = 0; i < count; i++, in++, out++)
	{
		out->numareaportals = in->numareaportals;
		out->firstareaportal = in->firstareaportal;
		out->floodvalid = 0;
		out->floodnum = 0;
	}
}

// Q2 counterpart
static void CMod_LoadAreaPortals(const lump_t* l)
{
	dareaportal_t* in = (void*)(cmod_base + l->fileofs);

	if (l->filelen % sizeof(*in))
		Com_Error(ERR_DROP, "MOD_LoadBmodel: funny lump size");

	const int count = l->filelen / (int)sizeof(*in);

	if (count >= MAX_MAP_AREAS) //mxd. '>' in Q2 and original logic.
		Com_Error(ERR_DROP, "Map has too many areas");

	dareaportal_t* out = map_areaportals;
	numareaportals = count;

	for (int i = 0; i < count; i++, in++, out++)
	{
		out->portalnum = in->portalnum;
		out->otherarea = in->otherarea;
	}
}

static void CMod_LoadVisibility(const lump_t* l)
{
	numvisibility = l->filelen;

	if (l->filelen >= MAX_MAP_VISIBILITY) //mxd. '>' in Q2 and original logic.
		Com_Error(ERR_DROP, "Map has too large visibility lump (size %i greater than max %i)", l->filelen, MAX_MAP_VISIBILITY); // H2: more detailed error message.

	memcpy(map_visibility, cmod_base + l->fileofs, l->filelen);
}

static void CMod_LoadEntityString(const lump_t* l)
{
	numentitychars = l->filelen;

	if (l->filelen >= MAX_MAP_ENTSTRING) //mxd. '>' in Q2 and original logic.
		Com_Error(ERR_DROP, "Map has too large entity lump");

	memcpy(map_entitystring, cmod_base + l->fileofs, l->filelen);
}

// Q2 counterpart
// Loads in the map and all submodels.
cmodel_t* CM_LoadMap(const char* name, const qboolean clientload, uint* checksum)
{
	static uint last_checksum;
	uint* buf;

	map_noareas = Cvar_Get("map_noareas", "0", 0);

	if (strcmp(map_name, name) == 0 && (clientload || !(int)(Cvar_VariableValue("flushmap"))))
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

	if (name[0] == 0) //mxd. Removed unneeded NULL check.
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
		Com_Error(ERR_DROP, "Couldn't load '%s'", name);

	last_checksum = Com_BlockChecksum(buf, length);
	*checksum = last_checksum;

	const dheader_t header = *(dheader_t*)buf;

	if (header.version != BSPVERSION)
		Com_Error(ERR_DROP, "CMod_LoadBrushModel: '%s' has wrong version number (%i should be %i)", name, header.version, BSPVERSION);

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

// Q2 counterpart
cmodel_t* CM_InlineModel(const char* name)
{
	if (name == NULL || *name != '*')
		Com_Error(ERR_DROP, "CM_InlineModel: bad name");

	const int num = Q_atoi(name + 1);

	if (num < 1 || num >= numcmodels)
		Com_Error(ERR_DROP, "CM_InlineModel: bad number");

	return &map_cmodels[num];
}

// Q2 counterpart
int CM_NumClusters(void)
{
	return numclusters;
}

// Q2 counterpart
int CM_NumInlineModels(void)
{
	return numcmodels;
}

// Q2 counterpart
int CM_LeafCluster(const int leafnum)
{
	if (leafnum < 0 || leafnum >= numleafs)
		Com_Error(ERR_DROP, "CM_LeafCluster: bad number");

	return map_leafs[leafnum].cluster;
}

// Q2 counterpart
int CM_LeafArea(const int leafnum)
{
	if (leafnum < 0 || leafnum >= numleafs)
		Com_Error(ERR_DROP, "CM_LeafArea: bad number");

	return map_leafs[leafnum].area;
}

// Q2 counterpart
// To keep everything totally uniform, bounding boxes are turned into small BSP trees instead of being compared directly.
int CM_HeadnodeForBox(const vec3_t mins, const vec3_t maxs)
{
	box_planes[0].dist = maxs[0];
	box_planes[1].dist = -maxs[0];

	box_planes[2].dist = mins[0];
	box_planes[3].dist = -mins[0];

	box_planes[4].dist = maxs[1];
	box_planes[5].dist = -maxs[1];

	box_planes[6].dist = mins[1];
	box_planes[7].dist = -mins[1];

	box_planes[8].dist = maxs[2];
	box_planes[9].dist = -maxs[2];

	box_planes[10].dist = mins[2];
	box_planes[11].dist = -mins[2];

	return box_headnode;
}

// Q2 counterpart
static int CM_PointLeafnum_r(const vec3_t p, int num)
{
	float dist;

	while (num >= 0)
	{
		const cnode_t* node = &map_nodes[num];
		const cplane_t* plane = node->plane;

		if (plane->type < 3)
			dist = p[plane->type] - plane->dist;
		else
			dist = DotProduct(plane->normal, p) - plane->dist;

		if (dist < 0)
			num = node->children[1];
		else
			num = node->children[0];
	}

	c_pointcontents++; // Optimize counter.

	return -1 - num;
}

// Q2 counterpart
int CM_PointLeafnum(const vec3_t p)
{
	if (numplanes > 0)
		return CM_PointLeafnum_r(p, 0);

	return 0; // Sound logic may call this without map loaded.
}

// Q2 counterpart
static void CM_BoxLeafnums_r(int nodenum)
{
	while (true)
	{
		if (nodenum < 0)
		{
			if (leaf_count < leaf_maxcount)
			{
				leaf_list[leaf_count] = -1 - nodenum;
				leaf_count++;
			}

			return;
		}

		const cnode_t* node = &map_nodes[nodenum];
		const int side = BoxOnPlaneSide(leaf_mins, leaf_maxs, (struct cplane_s*)node->plane); //mxd. BoxOnPlaneSide call instead of BOX_ON_PLANE_SIDE macro.

		if (side == 1)
		{
			nodenum = node->children[0];
		}
		else if (side == 2)
		{
			nodenum = node->children[1];
		}
		else
		{
			// Go down both sides.
			if (leaf_topnode == -1)
				leaf_topnode = nodenum;

			CM_BoxLeafnums_r(node->children[0]);
			nodenum = node->children[1];
		}
	}
}

// Q2 counterpart
static int CM_BoxLeafnums_headnode(vec3_t mins, vec3_t maxs, int* list, const int listsize, const int headnode, int* topnode)
{
	leaf_list = list;
	leaf_count = 0;
	leaf_maxcount = listsize;
	leaf_mins = mins;
	leaf_maxs = maxs;

	leaf_topnode = -1;

	CM_BoxLeafnums_r(headnode);

	if (topnode != NULL)
		*topnode = leaf_topnode;

	return leaf_count;
}

// Q2 counterpart
// Fills in a list of all the leafs touched.
int CM_BoxLeafnums(vec3_t mins, vec3_t maxs, int* list, const int listsize, int* topnode)
{
	return CM_BoxLeafnums_headnode(mins, maxs, list, listsize, map_cmodels[0].headnode, topnode);
}

// Q2 counterpart
char* CM_EntityString(void)
{
	return map_entitystring;
}

static void CM_DecompressVis(const byte* in, byte* out)
{
	const int row = (numclusters + 7) >> 3;
	byte* out_p = out;

	// No vis info, so make all visible.
	if (in == NULL || numvisibility == 0)
	{
		memset(out, 0xff, row); // H2
		return;
	}

	do
	{
		if (*in != 0)
		{
			*out_p++ = *in++;
			continue;
		}

		int c = in[1];
		in += 2;

		if (out_p - out + c > row)
		{
			c = row - (out_p - out);
			Com_DPrintf("warning: Vis decompression overrun\n");
		}

		memset(out_p, 0, c); // H2
		out_p += c;
	} while (out_p - out < row);
}

// Q2 counterpart
byte* CM_ClusterPVS(const int cluster)
{
	static byte	pvsrow[MAX_MAP_LEAFS / 8]; //mxd. Made local static.

	if (cluster == -1)
		memset(pvsrow, 0, (numclusters + 7) >> 3);
	else
		CM_DecompressVis(map_visibility + map_vis->bitofs[cluster][DVIS_PVS], pvsrow);

	return pvsrow;
}

// Q2 counterpart
byte* CM_ClusterPHS(const int cluster)
{
	static byte	phsrow[MAX_MAP_LEAFS / 8]; //mxd. Made local static.

	if (cluster == -1)
		memset(phsrow, 0, (numclusters + 7) >> 3);
	else
		CM_DecompressVis(map_visibility + map_vis->bitofs[cluster][DVIS_PHS], phsrow);

	return phsrow;
}

// Q2 counterpart
int CM_PointContents(const vec3_t p, const int headnode)
{
	if (numnodes > 0)
	{
		const int l = CM_PointLeafnum_r(p, headnode);
		return map_leafs[l].contents;
	}

	return 0; // Map not loaded.
}

// Q2 counterpart
// Handles offsetting and rotation of the end points for moving and rotating entities.
int CM_TransformedPointContents(const vec3_t p, const int headnode, const vec3_t origin, const vec3_t angles)
{
	vec3_t p_l;
	vec3_t temp;
	vec3_t forward;
	vec3_t right;
	vec3_t up;

	// Subtract origin offset.
	VectorSubtract(p, origin, p_l);

	// Rotate start and end into the models frame of reference.
	if (headnode != box_headnode && Vec3NotZero(angles))
	{
		AngleVectors(angles, forward, right, up);

		VectorCopy(p_l, temp);
		p_l[0] = DotProduct(temp, forward);
		p_l[1] = -DotProduct(temp, right);
		p_l[2] = DotProduct(temp, up);
	}

	const int l = CM_PointLeafnum_r(p_l, headnode);

	return map_leafs[l].contents;
}

static void CM_ClipBoxToBrush(const vec3_t mins, const vec3_t maxs, const vec3_t p1, const vec3_t p2, trace_t* trace, const cbrush_t* brush)
{
	if (brush->numsides == 0)
		return;

	c_brush_traces++;

	float enterfrac = -1.0f;
	float leavefrac = 1.0f;
	const cplane_t* clipplane = NULL;

	qboolean getout = false;
	qboolean startout = false;
	const cbrushside_t* leadside = NULL;

	for (int i = 0; i < brush->numsides; i++)
	{
		const cbrushside_t* side = &map_brushsides[brush->firstbrushside + i];
		const cplane_t* plane = side->plane;
		float dist;

		// FIXME: special case for axial.
		if (!trace_ispoint)
		{
			vec3_t ofs;

			// General box case. Push the plane out appropriately for mins/maxs. //FIXME: use signbits into 8 way lookup for each mins/maxs.
			for (int c = 0; c < 3; c++)
				ofs[c] = ((plane->normal[c] < 0.0f) ? maxs[c] : mins[c]);

			dist = DotProduct(ofs, plane->normal);
			dist = plane->dist - dist;
		}
		else
		{
			// Special point case.
			dist = plane->dist;
		}

		const float d1 = DotProduct(p1, plane->normal) - dist;
		const float d2 = DotProduct(p2, plane->normal) - dist;

		if (d2 > 0.0f)
			getout = true; // Endpoint is not in solid.

		if (d1 > 0.0f)
			startout = true;

		// If completely in front of face, no intersection.
		if (d1 > 0.0f && d2 >= d1)
			return;

		if (d1 <= 0.0f && d2 <= 0.0f)
			continue;

		// Crosses face.
		if (d1 > d2)
		{
			// Enter.
			const float f = (d1 - DIST_EPSILON) / (d1 - d2);
			if (f > enterfrac)
			{
				enterfrac = f;
				clipplane = plane;
				leadside = side;
			}
		}
		else
		{
			// Leave.
			const float f = (d1 + DIST_EPSILON) / (d1 - d2);
			if (f < leavefrac)
				leavefrac = f;
		}
	}

	if (!startout)
	{
		// Original point was inside brush.
		trace->startsolid = true;
		trace->fraction = 0.0f; //mxd. Because we DON'T want to end up with trace.startsolid AND trace.fraction:1 at the same time.

		if (!getout)
			trace->allsolid = true;
	}
	else if (enterfrac < leavefrac && enterfrac > -1.0f && enterfrac < trace->fraction)
	{
		trace->fraction = max(0.0f, enterfrac);
		trace->plane = *clipplane;
		trace->surface = leadside->surface; // H2
		trace->contents = brush->contents;
	}
}

// Q2 counterpart
static void CM_TestBoxInBrush(vec3_t mins, vec3_t maxs, vec3_t p1, trace_t* trace, const cbrush_t* brush)
{
	vec3_t ofs;

	if (brush->numsides == 0)
		return;

	for (int i = 0; i < brush->numsides; i++)
	{
		const cbrushside_t* side = &map_brushsides[brush->firstbrushside + i];
		const cplane_t* plane = side->plane;

		// Push the plane out appropriately for mins/maxs.
		for (int j = 0; j < 3; j++)
		{
			if (plane->normal[j] < 0)
				ofs[j] = maxs[j];
			else
				ofs[j] = mins[j];
		}

		const float dist = plane->dist - DotProduct(ofs, plane->normal);
		const float d1 = DotProduct(p1, plane->normal) - dist;

		// If completely in front of face, no intersection.
		if (d1 > 0.0f)
			return;
	}

	// Inside this brush.
	trace->startsolid = true;
	trace->allsolid = true;
	trace->fraction = 0.0f;
	trace->contents = brush->contents;
}

static void CM_TraceToLeaf(const int leafnum)
{
	const cleaf_t* leaf = &map_leafs[leafnum];
	const qboolean check_camerablock = ((trace_contents & CONTENTS_CAMERABLOCK) != 0); // H2

	if (!(leaf->contents & trace_contents))
		return;

	if (check_camerablock && (leaf->contents & CONTENTS_CAMERANOBLOCK)) // H2
		return;

	// Trace line against all brushes in the leaf.
	for (int k = 0; k < leaf->numleafbrushes; k++)
	{
		const int brushnum = map_leafbrushes[leaf->firstleafbrush + k];
		cbrush_t* b = &map_brushes[brushnum];

		if (b->checkcount == checkcount)
			continue; // Already checked this brush in another leaf.

		b->checkcount = checkcount;

		if (!(b->contents & trace_contents))
			continue;

		if (check_camerablock && (b->contents & CONTENTS_CAMERANOBLOCK)) // H2
			continue;

		CM_ClipBoxToBrush(trace_mins, trace_maxs, trace_start, trace_end, trace_trace, b); //mxd. The only difference between this and CM_TestInLeaf...
		if (trace_trace->fraction == 0.0f)
			return;
	}
}

static void CM_TestInLeaf(const int leafnum)
{
	const cleaf_t* leaf = &map_leafs[leafnum];
	const qboolean check_camerablock = ((trace_contents & CONTENTS_CAMERABLOCK) != 0); // H2

	if (!(leaf->contents & trace_contents))
		return;

	if (check_camerablock && (leaf->contents & CONTENTS_CAMERANOBLOCK)) // H2
		return;

	// Trace line against all brushes in the leaf.
	for (int k = 0; k < leaf->numleafbrushes; k++)
	{
		const int brushnum = map_leafbrushes[leaf->firstleafbrush + k];
		cbrush_t* b = &map_brushes[brushnum];

		if (b->checkcount == checkcount)
			continue; // Already checked this brush in another leaf.

		b->checkcount = checkcount;

		if (!(b->contents & trace_contents))
			continue;

		if (check_camerablock && (b->contents & CONTENTS_CAMERANOBLOCK)) // H2
			continue;

		CM_TestBoxInBrush(trace_mins, trace_maxs, trace_start, trace_trace, b); //mxd. The only difference between this and CM_TraceToLeaf...
		if (trace_trace->fraction == 0.0f)
			return;
	}
}

// Q2 counterpart
static void CM_RecursiveHullCheck(const int num, const float p1f, const float p2f, const vec3_t p1, const vec3_t p2)
{
	if (trace_trace->fraction <= p1f)
		return; // Already hit something nearer.

	// If < 0, we are in a leaf node.
	if (num < 0)
	{
		CM_TraceToLeaf(-1 - num);
		return;
	}

	// Find the point distances to the separating plane and the offset for the size of the box.
	const cnode_t* node = &map_nodes[num];
	const cplane_t* plane = node->plane;

	float t1;
	float t2;
	float offset;

	if (plane->type < 3)
	{
		t1 = p1[plane->type] - plane->dist;
		t2 = p2[plane->type] - plane->dist;
		offset = trace_extents[plane->type];
	}
	else
	{
		t1 = DotProduct(plane->normal, p1) - plane->dist;
		t2 = DotProduct(plane->normal, p2) - plane->dist;

		if (trace_ispoint)
		{
			offset = 0.0f;
		}
		else
		{
			offset = fabsf(trace_extents[0] * plane->normal[0]) +
					 fabsf(trace_extents[1] * plane->normal[1]) +
					 fabsf(trace_extents[2] * plane->normal[2]);
		}
	}

	// See which sides we need to consider.
	if (t1 >= offset && t2 >= offset)
	{
		CM_RecursiveHullCheck(node->children[0], p1f, p2f, p1, p2);
		return;
	}

	if (t1 < -offset && t2 < -offset)
	{
		CM_RecursiveHullCheck(node->children[1], p1f, p2f, p1, p2);
		return;
	}

	// Put the crosspoint DIST_EPSILON pixels on the near side.
	float idist;
	int side;
	float frac;
	float frac2;

	if (t1 < t2)
	{
		idist = 1.0f / (t1 - t2);
		side = 1;
		frac2 = (t1 + offset + DIST_EPSILON) * idist;
		frac = (t1 - offset + DIST_EPSILON) * idist;
	}
	else if (t1 > t2)
	{
		idist = 1.0f / (t1 - t2);
		side = 0;
		frac2 = (t1 - offset - DIST_EPSILON) * idist;
		frac = (t1 + offset + DIST_EPSILON) * idist;
	}
	else
	{
		side = 0;
		frac = 1.0f;
		frac2 = 0.0f;
	}

	// Move up to the node.
	frac = Clamp(frac, 0.0f, 1.0f);

	float midf = p1f + (p2f - p1f) * frac;

	vec3_t mid;
	for (int i = 0; i < 3; i++)
		mid[i] = p1[i] + frac * (p2[i] - p1[i]);

	CM_RecursiveHullCheck(node->children[side], p1f, midf, p1, mid);

	// Go past the node.
	frac2 = Clamp(frac2, 0.0f, 1.0f);

	midf = p1f + (p2f - p1f) * frac2;
	for (int i = 0; i < 3; i++)
		mid[i] = p1[i] + frac2 * (p2[i] - p1[i]);

	CM_RecursiveHullCheck(node->children[side ^ 1], midf, p2f, mid, p2);
}

void CM_BoxTrace(const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, const int headnode, const uint brushmask, trace_t* return_trace)
{
	checkcount++; // For multi-check avoidance.
	c_traces++; // For statistics, may be zeroed.

	trace_contents = brushmask;
	VectorCopy(start, trace_start);
	VectorCopy(end, trace_end);
	VectorCopy(mins, trace_mins);
	VectorCopy(maxs, trace_maxs);

	// Fill in a default trace.
	trace_trace = return_trace;
	memset(trace_trace, 0, sizeof(*trace_trace)); //mxd. When return_trace start or end were used as start/end, this will zero them as well!
	trace_trace->fraction = 1.0f;
	trace_trace->surface = &nullsurface; // H2

	if (numnodes == 0) // Map not loaded.
		return;

	// Check for position test special case.
	if (VectorCompare(trace_start, trace_end))
	{
		vec3_t c1;
		vec3_t c2;
		
		for (int i = 0; i < 3; i++)
		{
			c1[i] = trace_start[i] + mins[i] - 1.0f;
			c2[i] = trace_start[i] + maxs[i] + 1.0f;
		}

		int topnode;
		int leafs[1024];
		const int num_leafs = CM_BoxLeafnums_headnode(c1, c2, leafs, 1024, headnode, &topnode);

		for (int i = 0; i < num_leafs; i++)
		{
			CM_TestInLeaf(leafs[i]);
			if (trace_trace->allsolid)
				break;
		}

		VectorCopy(trace_start, trace_trace->endpos);

		return;
	}

	// Check for point special case.
	if (Vec3IsZero(mins) && Vec3IsZero(maxs))
	{
		trace_ispoint = true;
		VectorClear(trace_extents);
	}
	else
	{
		trace_ispoint = false;

		for (int i = 0; i < 3; i++)
			trace_extents[i] = max(-mins[i], maxs[i]);
	}

	// General sweeping through world.
	CM_RecursiveHullCheck(headnode, 0.0f, 1.0f, trace_start, trace_end);

	if (trace_trace->fraction == 1.0f)
	{
		VectorCopy(trace_end, trace_trace->endpos);
	}
	else
	{
		for (int i = 0; i < 3; i++)
			trace_trace->endpos[i] = trace_start[i] + trace_trace->fraction * (trace_end[i] - trace_start[i]);
	}
}

//mxd. Duplicate of LUDecomposition() from g_Physics.c in original logic?
static qboolean LUDecomposition(matrix3_t m, int* axis_arr)
{
	vec3_t scaler;

	for (int i = 0; i < 3; i++)
	{
		float max = 0.0f;

		for (int c = 0; c < 3; c++)
			max = max(max, fabsf(m[i][c]));

		if (max < FLT_EPSILON) //mxd. < 1e-15f in original version.
			return false;

		scaler[i] = 1.0f / max;
	}

	int axis = 0;

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < i; j++)
			for (int k = 0; k < j; k++)
				m[j][i] -= m[j][k] * m[k][i];

		float max = 0.0f;

		for (int j = i; j < 3; j++)
		{
			for (int k = 0; k < i; k++)
				m[j][i] -= m[j][k] * m[k][i];

			const float cur_max = fabsf(m[j][i]) * scaler[j];
			if (max <= cur_max)
			{
				max = cur_max;
				axis = j;
			}
		}

		if (i != axis)
		{
			for (int j = 0; j < 3; j++)
			{
				const float val = m[axis][j];
				m[axis][j] = m[i][j];
				m[i][j] = val;
			}

			scaler[axis] = scaler[i];
		}

		axis_arr[i] = axis;

		if (fabsf(m[i][i]) < FLT_EPSILON) //mxd. < 1e-15f in original version.
			return false;

		for (int j = 0; j < 3 - (i + 1); j++)
			m[i + j + 1][i] *= 1.0f / m[i][i];
	}

	return true;
}

//mxd. Duplicate of BackSub() from g_Physics.c in original logic?
static void BackSub(const matrix3_t m, const int* axis_arr, vec3_t normal)
{
	int tgt_axis = -1;

	for (int i = 0; i < 3; i++)
	{
		const int axis = axis_arr[i];
		float val = normal[axis];

		normal[axis] = normal[i];

		if (tgt_axis < 0)
		{
			if (val != 0.0f)
				tgt_axis = i;
		}
		else if (tgt_axis < i)
		{
			for (int c = 0; c < i - tgt_axis; c++)
			{
				const float v1 = normal[tgt_axis + c];
				const float v2 = m[i][tgt_axis + c];

				val -= v1 * v2;
			}
		}

		normal[i] = val;
	}

	for (int i = 2; i > -1; i--)
	{
		float val = normal[i];

		if (i < 2)
		{
			for (int c = 0; c < 3 - (i + 1); c++)
			{
				const float v1 = m[i][i + c + 1];
				const float v2 = normal[i + c + 1];

				val -= v1 * v2;
			}
		}

		normal[i] = val / m[i][i];
	}
}

// Handles offsetting and rotation of the end points for moving and rotating entities.
void CM_TransformedBoxTrace(const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, const int headnode, const uint brushmask, const vec3_t origin, const vec3_t angles, trace_t* return_trace)
{
	// Subtract origin offset.
	vec3_t start_l;
	VectorSubtract(start, origin, start_l);

	vec3_t end_l;
	VectorSubtract(end, origin, end_l);

	// Rotate start and end into the models frame of reference.
	if (headnode != box_headnode && Vec3NotZero(angles))
	{
		vec3_t forward;
		vec3_t right;
		vec3_t up;
		AngleVectors(angles, forward, right, up);

		vec3_t temp;
		VectorCopy(start_l, temp);
		start_l[0] = DotProduct(temp, forward);
		start_l[1] = -DotProduct(temp, right);
		start_l[2] = DotProduct(temp, up);

		VectorCopy(end_l, temp);
		end_l[0] = DotProduct(temp, forward);
		end_l[1] = -DotProduct(temp, right);
		end_l[2] = DotProduct(temp, up);

		vec3_t mins_l;
		vec3_t maxs_l;

		for (int i = 0; i < 8; i++) // H2 //TODO: I've definitely seen similar logic somewhere in this project. But where?..
		{
			temp[0] = ((i & 1) != 0 ? mins[0] : maxs[0]);
			temp[1] = ((i & 2) != 0 ? mins[1] : maxs[1]);
			temp[2] = ((i & 4) != 0 ? mins[2] : maxs[2]);

			vec3_t diff;
			diff[0] = DotProduct(temp, forward);
			diff[1] = -DotProduct(temp, right);
			diff[2] = DotProduct(temp, up);

			if (i == 0)
			{
				VectorCopy(diff, mins_l);
				VectorCopy(diff, maxs_l);
			}
			else
			{
				for (int c = 0; c < 3; c++)
				{
					mins_l[c] = min(mins_l[c], diff[c]);
					maxs_l[c] = max(maxs_l[c], diff[c]);
				}
			}
		}

		CM_BoxTrace(start_l, end_l, mins_l, maxs_l, headnode, brushmask, return_trace);

		matrix3_t m;
		for (int i = 0; i < 3; i++)
		{
			m[0][i] = forward[i];
			m[1][i] = -right[i];
			m[2][i] = up[i];
		}

		int axis_arr[3];
		if (LUDecomposition(m, axis_arr))
			BackSub(m, axis_arr, return_trace->plane.normal);
		else
			VectorClear(return_trace->plane.normal);

		if (return_trace->fraction != 1.0f)
			return_trace->fraction *= 0.99f;
	}
	else
	{
		CM_BoxTrace(start_l, end_l, mins, maxs, headnode, brushmask, return_trace);
	}

	for (int i = 0; i < 3; i++)
		return_trace->endpos[i] = start[i] + return_trace->fraction * (end[i] - start[i]);
}

#pragma region ========================== AREAPORTALS ==========================

// Q2 counterpart
static void FloodArea_r(carea_t* area, const int floodnum, const int floodvalid) //mxd. Added 'floodvalid' arg.
{
	if (area->floodvalid == floodvalid)
	{
		if (area->floodnum != floodnum)
			Com_Error(ERR_DROP, "FloodArea_r: reflooded");
	}
	else
	{
		area->floodnum = floodnum;
		area->floodvalid = floodvalid;

		dareaportal_t* p = &map_areaportals[area->firstareaportal];
		for (int i = 0; i < area->numareaportals; i++, p++)
			if (portalopen[p->portalnum])
				FloodArea_r(&map_areas[p->otherarea], floodnum, floodvalid);
	}
}

// Q2 counterpart
static void FloodAreaConnections(void)
{
	static int floodvalid; //mxd. Made local static.

	// All current floods are now invalid.
	floodvalid++;
	int floodnum = 0;

	// Area 0 is not used.
	for (int i = 1; i < numareas; i++)
	{
		carea_t* area = &map_areas[i];

		if (area->floodvalid != floodvalid)
		{
			floodnum++;
			FloodArea_r(area, floodnum, floodvalid);
		}
	}
}

// Q2 counterpart
void CM_SetAreaPortalState(const int portalnum, const qboolean open)
{
	if (portalnum > numareaportals)
		Com_Error(ERR_DROP, "areaportal > numareaportals");

	portalopen[portalnum] = open;
	FloodAreaConnections();
}

// Q2 counterpart
qboolean CM_AreasConnected(const int area1, const int area2)
{
	if ((int)map_noareas->value)
		return true;

	if (area1 > numareas || area2 > numareas)
		Com_Error(ERR_DROP, "area > numareas");

	return map_areas[area1].floodnum == map_areas[area2].floodnum;
}

// Q2 counterpart
// Writes a length byte followed by a bit vector of all the areas that area in the same flood as the area parameter.
// This is used by the client refreshes to cull visibility.
int CM_WriteAreaBits(byte* buffer, const int area)
{
	const int bytes = (numareas + 7) >> 3;

	if ((int)map_noareas->value)
	{
		// For debugging, send everything.
		memset(buffer, 255, bytes);
	}
	else
	{
		memset(buffer, 0, bytes);

		const int floodnum = map_areas[area].floodnum;
		for (int i = 0; i < numareas; i++)
			if (area == 0 || map_areas[i].floodnum == floodnum)
				buffer[i >> 3] |= 1 << (i & 7);
	}

	return bytes;
}

// Q2 counterpart
// Writes the portal state to a savegame file.
void CM_WritePortalState(FILE* f)
{
	fwrite(portalopen, sizeof(portalopen), 1, f);
}

// Q2 counterpart
// Reads the portal state from a savegame file and recalculates the area connections.
void CM_ReadPortalState(FILE* f)
{
	FS_Read(portalopen, sizeof(portalopen), f);
	FloodAreaConnections();
}

// Q2 counterpart
// Returns true if any leaf under headnode has a cluster that is potentially visible.
qboolean CM_HeadnodeVisible(const int headnode, byte* visbits)
{
	if (headnode < 0)
	{
		const int leafnum = -1 - headnode;
		const int cluster = map_leafs[leafnum].cluster;

		if (cluster == -1)
			return false;

		// Check for door-connected areas.
		return (visbits[cluster >> 3] & (1 << (cluster & 7)));
	}

	const cnode_t* node = &map_nodes[headnode];
	if (CM_HeadnodeVisible(node->children[0], visbits))
		return true;

	return CM_HeadnodeVisible(node->children[1], visbits);
}

#pragma endregion