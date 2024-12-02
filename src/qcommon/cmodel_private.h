//
// cmodel_private.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "qcommon.h"
#include "qfiles.h" //mxd

typedef struct
{
	cplane_t* plane;
	int children[2]; // Negative numbers are leafs
} cnode_t;

typedef struct
{
	cplane_t* plane;
	csurface_t* surface;
} cbrushside_t;

typedef struct
{
	int contents;
	int cluster;
	int area;
	ushort firstleafbrush;
	ushort numleafbrushes;
} cleaf_t;

typedef struct
{
	int contents;
	int numsides;
	int firstbrushside;
	int checkcount; // To avoid repeated testings
} cbrush_t;

typedef struct
{
	int numareaportals;
	int firstareaportal;
	int floodnum; // If two areas have equal floodnums, they are connected
	int floodvalid;
} carea_t;

//extern int checkcount; //mxd. Not needed.

extern char map_name[MAX_QPATH];

extern int numbrushsides;
extern cbrushside_t map_brushsides[MAX_MAP_BRUSHSIDES]; // 524288 bytes

extern int numplanes;
extern cplane_t map_planes[MAX_MAP_PLANES + 6]; // Extra for box hull
// 1310840 bytes

extern int numnodes;
extern cnode_t map_nodes[MAX_MAP_NODES + 6]; // Extra for box hull
// 524336 bytes

//extern int numleafs; // Allow leaf funcs to be called without a map
extern cleaf_t map_leafs[MAX_MAP_LEAFS]; // 1048576 bytes
extern int emptyleaf;
//extern int solidleaf; //mxd. Unused

extern int numleafbrushes;
extern ushort map_leafbrushes[MAX_MAP_LEAFBRUSHES]; // 131072 bytes

extern int numcmodels;
extern cmodel_t map_cmodels[MAX_MAP_MODELS]; // 40960 bytes

extern int numbrushes;
extern cbrush_t map_brushes[MAX_MAP_BRUSHES]; // 163840 bytes

extern int numvisibility;
extern byte map_visibility[MAX_MAP_VISIBILITY]; // 1048576 bytes
extern dvis_t* map_vis;

extern int numentitychars;
extern char map_entitystring[MAX_MAP_ENTSTRING]; // 262144 bytes

extern int numareas;
extern carea_t map_areas[MAX_MAP_AREAS]; // 4096 bytes

extern int numareaportals;
extern dareaportal_t map_areaportals[MAX_MAP_AREAPORTALS]; // 8192 bytes

extern int numclusters;

//extern csurface_t nullsurface; //mxd. Disabled

//extern int floodvalid; //mxd. Disabled

extern qboolean portalopen[MAX_MAP_AREAPORTALS]; // 4096 bytes

extern cvar_t* map_noareas;

//mxd. Disabled
//extern cplane_t* box_planes;
//extern int box_headnode;
//extern cbrush_t* box_brush;
//extern cleaf_t* box_leaf;

//mxd. Disabled
/*extern int leaf_count;
extern int leaf_maxcount;
extern int* leaf_list;
extern float* leaf_mins;
extern float* leaf_maxs;
extern int leaf_topnode;*/

//mxd. Disabled
/*extern vec3_t trace_start;
extern vec3_t trace_end;
extern vec3_t trace_mins;
extern vec3_t trace_maxs;
extern vec3_t trace_extents;

extern trace_t* trace_trace;
extern int trace_contents;
extern qboolean trace_ispoint;*/ // Optimized case

#include "cmodel.h"

//void CM_InitBoxHull(void); //mxd. Not needed
int CM_BoxLeafnums_headnode(vec3_t mins, vec3_t maxs, int* list, int listsize, int headnode, int* topnode);
//void CM_TestInLeaf(int leafnum); //mxd. Not needed
//void CM_RecursiveHullCheck(int num, float p1f, float p2f, vec3_t p1, vec3_t p2); //mxd. Not needed
void FloodAreaConnections(void);