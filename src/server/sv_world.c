//
// sv_world.c -- world query functions
//
// Copyright 1998 Raven Software
//

#include "server.h"
#include "cmodel.h"
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

// Q2 counterpart
static void RemoveLink(const link_t* l)
{
	l->next->prev = l->prev;
	l->prev->next = l->next;
}

// Q2 counterpart
static void InsertLinkBefore(link_t* l, link_t* before)
{
	l->next = before;
	l->prev = before->prev;
	l->prev->next = l;
	l->next->prev = l;
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

// Q2 counterpart (except for extra 1.75 scaler in 'expand for rotation' block).
void SV_LinkEdict(edict_t* ent)
{
#define MAX_TOTAL_ENT_LEAFS	128

	int leafs[MAX_TOTAL_ENT_LEAFS];
	int clusters[MAX_TOTAL_ENT_LEAFS];
	
	// Unlink from old position?
	if (ent->area.prev != NULL)
		SV_UnlinkEdict(ent);

	if (ent == ge->edicts || !ent->inuse)
		return; // Don't add the world.

	// Set the size.
	VectorSubtract(ent->maxs, ent->mins, ent->size);

	// Encode the size into the entity_state for client prediction.
	if (ent->solid == SOLID_BBOX && (ent->svflags & SVF_DEADMONSTER) == 0)
	{
		// Assume that x/y are equal and symmetric.
		const int i = ClampI((int)(ent->maxs[0] / 8), 1, 31);

		// Z is not symmetric.
		const int j = ClampI((int)(-ent->mins[2] / 8), 1, 31);

		// And z maxs can be negative...
		const int k = ClampI((int)((ent->maxs[2] + 32) / 8), 1, 63);

		ent->s.solid = (short)((k << 10) | (j << 5) | i);
	}
	else if (ent->solid == SOLID_BSP)
	{
		ent->s.solid = 31; // A solid_bbox will never create this value.
	}
	else
	{
		ent->s.solid = 0;
	}

	// Set the abs box.
	if (ent->solid == SOLID_BSP && (ent->s.angles[0] != 0.0f || ent->s.angles[1] != 0.0f || ent->s.angles[2] != 0.0f))
	{
		// Expand for rotation.
		float max = 0.0f;

		for (int i = 0; i < 3; i++)
		{
			max = max(fabsf(ent->mins[i]), max);
			max = max(fabsf(ent->maxs[i]), max);
		}

		for (int i = 0; i < 3; i++)
		{
			ent->absmin[i] = ent->s.origin[i] - max * 1.75f; // H2: extra 1.75 scaler.
			ent->absmax[i] = ent->s.origin[i] + max * 1.75f; // H2: extra 1.75 scaler.
		}
	}
	else
	{
		// Normal
		VectorAdd(ent->s.origin, ent->mins, ent->absmin);
		VectorAdd(ent->s.origin, ent->maxs, ent->absmax);
	}

	// Because movement is clipped an epsilon away from an actual edge, we must fully check even when bounding boxes don't quite touch.
	VectorDec(ent->absmin);
	VectorInc(ent->absmax);

	// Link to PVS leafs.
	ent->num_clusters = 0;
	ent->areanum = 0;
	ent->areanum2 = 0;

	// Get all leafs, including solids.
	int topnode;
	const int num_leafs = CM_BoxLeafnums(ent->absmin, ent->absmax, leafs, MAX_TOTAL_ENT_LEAFS, &topnode);

	// Set areas.
	for (int i = 0; i < num_leafs; i++)
	{
		clusters[i] = CM_LeafCluster(leafs[i]);
		const int area = CM_LeafArea(leafs[i]);

		if (area != 0)
		{
			// Doors may legally straggle two areas, but nothing should ever need more than that.
			if (ent->areanum != 0 && ent->areanum != area)
			{
				if (ent->areanum2 != 0 && ent->areanum2 != area && sv.state == ss_loading)
					Com_DPrintf("Object touching 3 areas at %f %f %f\n", ent->absmin[0], ent->absmin[1], ent->absmin[2]);

				ent->areanum2 = area;
			}
			else
			{
				ent->areanum = area;
			}
		}
	}

	if (num_leafs >= MAX_TOTAL_ENT_LEAFS)
	{
		// Assume we missed some leafs, and mark by headnode.
		ent->num_clusters = -1;
		ent->headnode = topnode;
	}
	else
	{
		ent->num_clusters = 0;

		for (int i = 0; i < num_leafs; i++)
		{
			if (clusters[i] == -1)
				continue; // Not a visible leaf

			int j;
			for (j = 0; j < i; j++)
				if (clusters[j] == clusters[i])
					break;

			if (j == i)
			{
				if (ent->num_clusters == MAX_ENT_CLUSTERS)
				{
					// Assume we missed some leafs, and mark by headnode.
					ent->num_clusters = -1;
					ent->headnode = topnode;
					break;
				}

				ent->clusternums[ent->num_clusters++] = clusters[i];
			}
		}
	}

	// If first time, make sure old_origin is valid.
	if (ent->linkcount == 0)
		VectorCopy(ent->s.origin, ent->s.old_origin);

	ent->linkcount++;

	if (ent->solid == SOLID_NOT)
		return;

	// Find the first node that the ent's box crosses.
	areanode_t* node = sv_areanodes;

	while (true)
	{
		if (node->axis == -1)
			break;

		if (ent->absmin[node->axis] > node->dist)
			node = node->children[0];
		else if (ent->absmax[node->axis] < node->dist)
			node = node->children[1];
		else
			break; // Crosses the node
	}

	// Link it in.
	if (ent->solid == SOLID_TRIGGER)
		InsertLinkBefore(&ent->area, &node->trigger_edicts);
	else
		InsertLinkBefore(&ent->area, &node->solid_edicts);
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