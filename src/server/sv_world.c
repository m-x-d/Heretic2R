//
// sv_world.c -- world query functions
//
// Copyright 1998 Raven Software
//

#include "server.h"
#include "Vector.h"

#define AREA_DEPTH	4
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

// Q2 counterpart
// ClearLink is used for new headnodes.
static void ClearLink(link_t* l)
{
	l->next = l;
	l->prev = l;
}

static void RemoveLink(link_t* l)
{
	NOT_IMPLEMENTED
}

// Q2 counterpart
// Builds a uniformly subdivided tree for the given world size.
static areanode_t* SV_CreateAreaNode(const int depth, vec3_t mins, vec3_t maxs)
{
	vec3_t size;
	vec3_t mins1;
	vec3_t maxs1;
	vec3_t mins2;
	vec3_t maxs2;

	areanode_t* anode = &sv_areanodes[sv_numareanodes];
	sv_numareanodes++;

	ClearLink(&anode->trigger_edicts);
	ClearLink(&anode->solid_edicts);

	if (depth == AREA_DEPTH)
	{
		anode->axis = -1;
		anode->children[0] = NULL;
		anode->children[1] = NULL;

		return anode;
	}

	VectorSubtract(maxs, mins, size);
	if (size[0] > size[1])
		anode->axis = 0;
	else
		anode->axis = 1;

	anode->dist = (maxs[anode->axis] + mins[anode->axis]) * 0.5f;
	VectorCopy(mins, mins1);
	VectorCopy(mins, mins2);
	VectorCopy(maxs, maxs1);
	VectorCopy(maxs, maxs2);

	maxs1[anode->axis] = anode->dist;
	mins2[anode->axis] = anode->dist;

	anode->children[0] = SV_CreateAreaNode(depth + 1, mins2, maxs2);
	anode->children[1] = SV_CreateAreaNode(depth + 1, mins1, maxs1);

	return anode;
}

// Q2 counterpart
void SV_ClearWorld(void)
{
	memset(sv_areanodes, 0, sizeof(sv_areanodes));
	sv_numareanodes = 0;
	SV_CreateAreaNode(0, sv.models[1]->mins, sv.models[1]->maxs);
}

// Q2 counterpart
void SV_UnlinkEdict(edict_t* ent)
{
	if (ent->area.prev != NULL)
	{
		RemoveLink(&ent->area);
		ent->area.next = NULL;
		ent->area.prev = NULL;
	}
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