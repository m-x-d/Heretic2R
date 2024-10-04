//
// cmodel.c -- model loading
//
// Copyright 1998 Raven Software
//

#include "cmodel.h"
#include "cmodel_private.h"
#include "Vector.h"

static int checkcount;

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
byte map_visibility[MAX_MAP_VISIBILITY];
dvis_t* map_vis = (dvis_t*)map_visibility;

int numentitychars;
char map_entitystring[MAX_MAP_ENTSTRING];

int numareas = 1;
carea_t map_areas[MAX_MAP_AREAS];

int numareaportals;
dareaportal_t map_areaportals[MAX_MAP_AREAPORTALS];

int numclusters = 1;

qboolean portalopen[MAX_MAP_AREAPORTALS];

cvar_t* map_noareas;

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

//mxd. Used by SV_Trace logic.
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

	cnode_t* out = map_nodes;
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

	const dheader_t header = *(dheader_t*)buf;

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

int CM_PointLeafnum(const vec3_t p)
{
	NOT_IMPLEMENTED
	return 0;
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
int CM_BoxLeafnums_headnode(vec3_t mins, vec3_t maxs, int* list, const int listsize, const int headnode, int* topnode)
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

static void CM_TraceToLeaf(int leafnum)
{
	NOT_IMPLEMENTED
}

static void CM_TestInLeaf(int leafnum)
{
	NOT_IMPLEMENTED
}

// Q2 counterpart
static void CM_RecursiveHullCheck(const int num, const float p1f, const float p2f, const vec3_t p1, const vec3_t p2)
{
	float t1;
	float t2;
	float offset;
	float frac;
	float frac2;
	float idist;
	vec3_t mid;
	int side;

	if (trace_trace->fraction <= p1f)
		return; // Already hit something nearer

	// If < 0, we are in a leaf node
	if (num < 0)
	{
		CM_TraceToLeaf(-1 - num);
		return;
	}

	// Find the point distances to the separating plane and the offset for the size of the box.
	const cnode_t* node = map_nodes + num;
	const cplane_t* plane = node->plane;

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
		frac = 1;
		frac2 = 0;
	}

	// Move up to the node.
	frac = Clamp(frac, 0.0f, 1.0f);

	float midf = p1f + (p2f - p1f) * frac;
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
	memset(trace_trace, 0, sizeof(*trace_trace));
	trace_trace->fraction = 1.0f;
	trace_trace->surface = &nullsurface; // H2

	if (numnodes == 0) // Map not loaded.
		return;

	// Check for position test special case.
	if (start[0] == end[0] && start[1] == end[1] && start[2] == end[2])
	{
		int leafs[1024];
		vec3_t c1;
		vec3_t c2;
		
		for (int i = 0; i < 3; i++)
		{
			c1[i] = start[i] + mins[i] - 1.0f;
			c2[i] = start[i] + maxs[i] + 1.0f;
		}

		int topnode;
		const int num_leafs = CM_BoxLeafnums_headnode(c1, c2, leafs, 1024, headnode, &topnode);

		for (int i = 0; i < num_leafs; i++)
		{
			CM_TestInLeaf(leafs[i]);
			if (trace_trace->allsolid)
				break;
		}

		VectorCopy(start, trace_trace->endpos);

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
	//trace_trace_p = trace_trace;

	if (trace_trace->fraction == 1.0f)
	{
		VectorCopy(end, trace_trace->endpos);
	}
	else
	{
		for (int i = 0; i < 3; i++)
			trace_trace->endpos[i] = start[i] + trace_trace->fraction * (end[i] - start[i]);
	}
}

void CM_TransformedBoxTrace(vec3_t start, vec3_t end, vec3_t mins, vec3_t maxs, int headnode, uint brushmask, vec3_t origin, vec3_t angles, trace_t* return_trace)
{
	NOT_IMPLEMENTED
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
void FloodAreaConnections(void)
{
	static int floodvalid; //mxd. Made local static

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

void CM_SetAreaPortalState(int portalnum, qboolean open)
{
	NOT_IMPLEMENTED
}

qboolean CM_AreasConnected(int area1, int area2)
{
	NOT_IMPLEMENTED
	return false;
}

#pragma endregion