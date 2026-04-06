//
// cmodel.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "qcommon.h"
#include "qfiles.h"

//mxd. Moved from cmodel_private.h, because they're also used by Qcommon_frame().
extern qboolean trace_check_water;
extern int c_pointcontents;
extern int c_traces;
extern int c_brush_traces;

//mxd. Moved from cmodel_private.h, because they're also used by CL_RequestNextDownload().
extern int numtexinfo;
extern csurface_t map_surfaces[MAX_MAP_TEXINFO]; // 65536 bytes

extern cmodel_t* CM_LoadMap(const char* name, qboolean clientload, uint* checksum);
extern cmodel_t* CM_InlineModel(const char* name); // *1, *2, etc

extern int CM_NumClusters(void);
extern int CM_NumInlineModels(void);
extern char* CM_EntityString(void);

// Creates a clipping hull for an arbitrary box.
extern int CM_HeadnodeForBox(const vec3_t mins, const vec3_t maxs);

// Returns an ORed contents mask.
extern int CM_PointContents(const vec3_t p, int headnode);
extern int CM_TransformedPointContents(const vec3_t p, int headnode, const vec3_t origin, const vec3_t angles);

extern void CM_BoxTrace(const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, int headnode, uint brushmask, trace_t* return_trace);
extern void CM_TransformedBoxTrace(const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, int headnode, uint brushmask, const vec3_t origin, const vec3_t angles, trace_t* return_trace);

extern byte* CM_ClusterPVS(int cluster);
extern byte* CM_ClusterPHS(int cluster);

extern int CM_PointLeafnum(const vec3_t p);

// Call with topnode set to the headnode, returns with topnode set to the first node that splits the box.
extern int CM_BoxLeafnums(vec3_t mins, vec3_t maxs, int* list, int listsize, int* topnode);

extern int CM_LeafCluster(int leafnum);
extern int CM_LeafArea(int leafnum);

extern void CM_SetAreaPortalState(int portalnum, qboolean open);
extern qboolean CM_AreasConnected(int area1, int area2);

extern int CM_WriteAreaBits(byte* buffer, int area);
extern qboolean CM_HeadnodeVisible(int headnode, byte* visbits);

extern void CM_WritePortalState(FILE* f);
extern void CM_ReadPortalState(FILE* f);