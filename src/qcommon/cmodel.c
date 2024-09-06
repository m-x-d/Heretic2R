//
// cmodel.c -- model loading
//
// Copyright 1998 Raven Software
//

#include "cmodel.h"
#include "cmodel_private.h"

char map_name[MAX_QPATH];

int numplanes;
int numnodes;
int numleafs = 1; // Allow leaf funcs to be called without a map

int numcmodels;
cmodel_t map_cmodels[MAX_MAP_MODELS];

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

cmodel_t* CM_LoadMap(const char* name, const qboolean clientload, uint* checksum)
{
	static uint last_checksum;

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

	NOT_IMPLEMENTED
	return NULL;
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

int CM_PointLeafnum(vec3_t p)
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