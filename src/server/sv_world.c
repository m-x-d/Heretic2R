//
// sv_world.c -- world query functions
//
// Copyright 1998 Raven Software
//

#include "server.h"

void SV_ClearWorld(void)
{
	NOT_IMPLEMENTED
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