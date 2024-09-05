//
// sv_world.c -- world query functions
//
// Copyright 1998 Raven Software
//

#include "server.h"

#define AREA_NODES	32

typedef struct areanode_s
{
	int axis; // -1 = leaf node
	float dist;
	struct areanode_s* children[2];
	link_t trigger_edicts;
	link_t solid_edicts;
} areanode_t;

areanode_t sv_areanodes[AREA_NODES];
int sv_numareanodes;

static areanode_t* SV_CreateAreaNode(int depth, vec3_t mins, vec3_t maxs)
{
	NOT_IMPLEMENTED
	return NULL;
}

// Q2 counterpart
void SV_ClearWorld(void)
{
	memset(sv_areanodes, 0, sizeof(sv_areanodes));
	sv_numareanodes = 0;
	SV_CreateAreaNode(0, sv.models[1]->mins, sv.models[1]->maxs);
}

void SV_UnlinkEdict(edict_t* ent)
{
	NOT_IMPLEMENTED
}

void SV_LinkEdict(edict_t* ent)
{
	NOT_IMPLEMENTED
}

int SV_AreaEdicts(vec3_t mins, vec3_t maxs, edict_t** list, int maxcount, int areatype)
{
	NOT_IMPLEMENTED
	return 0;
}

int SV_FindEntitiesInBounds(vec3_t mins, vec3_t maxs, SinglyLinkedList_t* list, int areatype)
{
	NOT_IMPLEMENTED
	return 0;
}

int SV_PointContents(vec3_t p)
{
	NOT_IMPLEMENTED
	return 0;
}

void SV_Trace(vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, edict_t* passent, int contentmask, trace_t* tr)
{
	NOT_IMPLEMENTED
}

void SV_TraceBoundingForm(FormMove_t* formMove)
{
	NOT_IMPLEMENTED
}

qboolean SV_ResizeBoundingForm(edict_t* self, FormMove_t* formMove)
{
	NOT_IMPLEMENTED
	return false;
}

int SV_GetContentsAtPoint(vec3_t point)
{
	NOT_IMPLEMENTED
	return 0;
}

qboolean SV_CheckDistances(vec3_t origin, float dist)
{
	NOT_IMPLEMENTED
	return false;
}