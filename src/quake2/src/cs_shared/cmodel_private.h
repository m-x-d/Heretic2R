//
// cmodel_private.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "qcommon.h"

typedef struct
{
	cplane_t* plane;
	int children[2]; // Negative numbers are leafs.
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
	int checkcount; // To avoid repeated testings.
} cbrush_t;

typedef struct
{
	int numareaportals;
	int firstareaportal;
	int floodnum; // If two areas have equal floodnums, they are connected.
	int floodvalid;
} carea_t;

static void FloodAreaConnections(void);