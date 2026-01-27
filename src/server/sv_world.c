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

#define STRUCT_FROM_LINK(l,t,m)		((t *)((byte *)(l) - (int)&(((t *)0)->m)))
#define EDICT_FROM_AREA(l)			STRUCT_FROM_LINK(l,edict_t,area)

typedef struct areanode_s
{
	int axis; // -1 = leaf node
	float dist;
	struct areanode_s* children[2];
	link_t trigger_edicts;
	link_t solid_edicts;
} areanode_t;

static areanode_t sv_areanodes[AREA_NODES];
static int sv_numareanodes;

//mxd. Used by SV_AreaEdicts logic.
static const float* area_mins;
static const float* area_maxs;
static edict_t** area_list;
static int area_count;
static int area_maxcount;
static int area_type;

//mxd. Used by SV_FindEntitiesInBounds logic.
static SinglyLinkedList_t* edicts_list;

typedef struct
{
	vec3_t boxmins; // Enclose the test object along entire move.
	vec3_t boxmaxs;

	const float* mins; // Size of the moving object.
	const float* maxs;

	vec3_t mins2; // Size when clipping against monsters. //TODO: always the same values as mins/maxs? Remove?
	vec3_t maxs2;

	const float* start;
	const float* end;

	trace_t* trace; // Q2: trace_t
	const edict_t* passedict;
	uint contentmask; //mxd. int -> uint
} moveclip_t;

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

	vec3_t size;
	VectorSubtract(maxs, mins, size);

	if (size[0] > size[1])
		anode->axis = 0;
	else
		anode->axis = 1;

	anode->dist = (maxs[anode->axis] + mins[anode->axis]) * 0.5f;

	vec3_t mins1 = VEC3_INIT(mins);
	vec3_t maxs1 = VEC3_INIT(maxs);
	vec3_t mins2 = VEC3_INIT(mins);
	vec3_t maxs2 = VEC3_INIT(maxs);

	maxs1[anode->axis] = anode->dist;
	mins2[anode->axis] = anode->dist;

	anode->children[0] = SV_CreateAreaNode(depth + 1, mins2, maxs2);
	anode->children[1] = SV_CreateAreaNode(depth + 1, mins1, maxs1);

	return anode;
}

// Q2 counterpart
// Called after the world model has been loaded, before linking any entities.
void SV_ClearWorld(void)
{
	memset(sv_areanodes, 0, sizeof(sv_areanodes));
	sv_numareanodes = 0;
	SV_CreateAreaNode(0, sv.models[1]->mins, sv.models[1]->maxs);
}

// Q2 counterpart
// Call before removing an entity, and before trying to move one, so it doesn't clip against itself.
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
// Needs to be called any time an entity changes origin, mins, maxs, or solid. Automatically unlinks if needed.
// sets ent->v.absmin and ent->v.absmax.
// sets ent->leafnums[] for pvs determination even if the entity is not solid.
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
	if (ent->solid == SOLID_BBOX && !(ent->svflags & SVF_DEADMONSTER))
	{
		//TODO: H2 uses non-square bboxes on XY axis (like obj_biotank)!

		// Assume that x/y are equal and symmetric.
		const int x = ClampI((int)(ent->maxs[0] / 8), 1, 31);

		// Z is not symmetric.
		const int zd = ClampI((int)(-ent->mins[2] / 8), 1, 31);

		// And z maxs can be negative...
		const int zu = ClampI((int)((ent->maxs[2] + 32) / 8), 1, 63);

		ent->s.solid = (short)((zu << 10) | (zd << 5) | x);
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
	if (ent->solid == SOLID_BSP && Vec3NotZero(ent->s.angles))
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

// Q2 counterpart
static void SV_AreaEdicts_r(areanode_t* node)
{
	link_t* start;

	// Touch linked edicts.
	if (area_type == AREA_SOLID)
		start = &node->solid_edicts;
	else
		start = &node->trigger_edicts;

	for (link_t* l = start->next; l != start; l = l->next)
	{
		edict_t* check = EDICT_FROM_AREA(l);

		if (check->solid == SOLID_NOT)
			continue; // Deactivated

		if (check->absmin[0] > area_maxs[0] || check->absmin[1] > area_maxs[1] || check->absmin[2] > area_maxs[2] || 
			check->absmax[0] < area_mins[0] || check->absmax[1] < area_mins[1] || check->absmax[2] < area_mins[2])
			continue; // Not touching

		if (area_count == area_maxcount)
		{
			Com_Printf("SV_AreaEdicts: MAXCOUNT\n");
			return;
		}

		area_list[area_count] = check;
		area_count++;
	}

	if (node->axis == -1)
		return; // Terminal node.

	// Recurse down both sides.
	if (area_maxs[node->axis] > node->dist)
		SV_AreaEdicts_r(node->children[0]);

	if (area_mins[node->axis] < node->dist)
		SV_AreaEdicts_r(node->children[1]);
}

// Q2 counterpart
// Fills in a table of edict pointers with edicts that have bounding boxes that intersect the given area.
// It is possible for a non-axial bmodel to be returned that doesn't actually intersect the area on an exact test.
// Returns the number of pointers filled in.
int SV_AreaEdicts(const vec3_t mins, const vec3_t maxs, edict_t** list, const int maxcount, const int areatype)
{
	area_mins = mins;
	area_maxs = maxs;
	area_list = list;
	area_count = 0;
	area_maxcount = maxcount;
	area_type = areatype;
	SV_AreaEdicts_r(sv_areanodes);

	return area_count;
}

//mxd. Similar to SV_AreaEdicts_r().
static void SV_FindEntitiesInBounds_r(areanode_t* node)
{
	link_t* start;

	// Touch linked edicts.
	if (area_type == AREA_SOLID)
		start = &node->solid_edicts;
	else
		start = &node->trigger_edicts;

	for (link_t* l = start->next; l != start; l = l->next)
	{
		edict_t* check = EDICT_FROM_AREA(l);

		if (check->solid == SOLID_NOT)
			continue; // Deactivated

		if (check->absmin[0] > area_maxs[0] || check->absmin[1] > area_maxs[1] || check->absmin[2] > area_maxs[2] ||
			check->absmax[0] < area_mins[0] || check->absmax[1] < area_mins[1] || check->absmax[2] < area_mins[2])
			continue; // Not touching

		if (area_count >= MAX_NETWORKABLE_EDICTS)
		{
			Com_Printf("SV_AreaEdicts: area_count >= MAX_NETWORKABLE_EDICTS\n");
			return;
		}

		GenericUnion4_t temp;
		temp.t_edict_p = check;
		SLList_Push(edicts_list, temp);
		area_count++;
	}

	if (node->axis == -1)
		return; // Terminal node.

	// Recurse down both sides.
	if (area_maxs[node->axis] > node->dist)
		SV_AreaEdicts_r(node->children[0]);

	if (area_mins[node->axis] < node->dist)
		SV_AreaEdicts_r(node->children[1]);
}

int SV_FindEntitiesInBounds(const vec3_t mins, const vec3_t maxs, SinglyLinkedList_t* list, const int areatype) // H2
{
	area_mins = mins;
	area_maxs = maxs;
	edicts_list = list;
	area_count = 0;
	area_type = areatype;
	SV_FindEntitiesInBounds_r(sv_areanodes);

	return area_count;
}

// Q2 counterpart
// Returns a headnode that can be used for testing or clipping an object of mins / maxs size.
// Offset is filled in to contain the adjustment that must be added to the testing object's origin
// to get a point to use with the returned hull.
static int SV_HullForEntity(const edict_t* ent)
{
	// Decide which clipping hull to use, based on the size.
	if (ent->solid == SOLID_BSP)
	{
		// Explicit hulls in the BSP model.
		const cmodel_t* model = sv.models[ent->s.modelindex];

		if (model == NULL)
			Com_Error(ERR_FATAL, "MOVETYPE_PUSH with a non bsp model");

		return model->headnode;
	}

	// Create a temp hull from bounding box sizes.
	return CM_HeadnodeForBox(ent->mins, ent->maxs);
}

// Returns the CONTENTS_* value from the world at the given point.
// Quake 2 extends this to also check entities, to allow moving liquids.
int SV_PointContents(const vec3_t p)
{
	edict_t* touchlist[MAX_EDICTS];

	// Get base contents from world.
	int contents = CM_PointContents(p, sv.models[1]->headnode);

	// Or in contents from all the other entities.
	const int num = SV_AreaEdicts(p, p, touchlist, MAX_EDICTS, AREA_SOLID);

	for (int i = 0; i < num; i++)
	{
		const edict_t* hit = touchlist[i];

		// Might intersect, so do an exact clip.
		const int headnode = SV_HullForEntity(hit);
		const float* angles = (hit->solid == SOLID_BSP ? hit->s.angles : vec3_origin);

		// H2: Unlike Q2, actually uses 'angles' var.
		contents |= CM_TransformedPointContents(p, headnode, hit->s.origin, angles);
	}

	return contents;
}

static void SV_ClipMoveToEntities(const moveclip_t* clip)
{
	edict_t* touchlist[MAX_EDICTS];
	const int num = SV_AreaEdicts(clip->boxmins, clip->boxmaxs, touchlist, MAX_EDICTS, AREA_SOLID);

	// Be careful, it is possible to have an entity in this list removed before we get to it (killtriggered).
	for (int i = 0; i < num; i++)
	{
		edict_t* touch = touchlist[i];

		if (touch->solid == SOLID_NOT || touch == clip->passedict)
			continue;

		// Don't clip against own missiles or owner.
		if (clip->passedict != NULL && (clip->passedict == touch->owner || clip->passedict->owner == touch))
			continue;

		if (!(clip->contentmask & CONTENTS_DEADMONSTER) && (touch->svflags & SVF_DEADMONSTER))
			continue;

		// Might intersect, so do an exact clip.
		const int headnode = SV_HullForEntity(touch);
		const float* angles = ((touch->solid == SOLID_BSP) ? touch->s.angles : vec3_origin); // Boxes don't rotate.

		trace_t trace;
		if (touch->svflags & SVF_MONSTER)
			CM_TransformedBoxTrace(clip->start, clip->end, clip->mins2, clip->maxs2, headnode, clip->contentmask, touch->s.origin, angles, &trace);
		else
			CM_TransformedBoxTrace(clip->start, clip->end, clip->mins, clip->maxs, headnode, clip->contentmask, touch->s.origin, angles, &trace);

		if (trace.startsolid || trace.allsolid || trace.fraction < clip->trace->fraction)
		{
			trace.ent = touch;
			trace.architecture = (touch->solid == SOLID_BSP); // H2 //mxd. false in original logic. Changed, so climbing/vaulting logic works on func_nnn brush ents.

			// H2: copy to clip->trace.
			const qboolean startsolid = clip->trace->startsolid;
			memcpy(clip->trace, &trace, sizeof(trace_t));
			if (startsolid)
				clip->trace->startsolid = true;

			if (clip->trace->allsolid)
				return;
		}
	}
}

// Q2 counterpart
static void SV_TraceBounds(const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, vec3_t boxmins, vec3_t boxmaxs)
{
	for (int i = 0; i < 3; i++)
	{
		if (end[i] > start[i])
		{
			boxmins[i] = start[i] + mins[i] - 1.0f;
			boxmaxs[i] = end[i] + maxs[i] + 1.0f;
		}
		else
		{
			boxmins[i] = end[i] + mins[i] - 1.0f;
			boxmaxs[i] = start[i] + maxs[i] + 1.0f;
		}
	}
}

// Moves the given mins/maxs volume through the world from start to end. mins and maxs are relative.
// If the entire move stays in a solid volume, trace.startsolid and trace.allsolid will be set, and trace.fraction will be 0.
// If the starting point is in a solid, it will be allowed to move out to an open area.
// passedict is explicitly excluded from clipping checks (normally NULL).
void SV_Trace(const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, const edict_t* passent, const uint contentmask, trace_t* tr)
{
	moveclip_t clip;

	if (mins == NULL)
		mins = vec3_origin;

	if (maxs == NULL)
		maxs = vec3_origin;

	// Clip to world.
	CM_BoxTrace(start, end, mins, maxs, 0, (contentmask & ~CONTENTS_WORLD_ONLY), tr);

	tr->architecture = true; // H2
	tr->ent = ge->edicts;

	// H2: New 'contentmask' check.
	if ((contentmask & CONTENTS_WORLD_ONLY) == 0)
	{
		//mxd. Original logic makes copies of start/end vectors and assigns those to clip.start/clip.end.
		//mxd. Not needed, because clip.start/clip.end are never modified.
		clip.trace = tr;
		clip.contentmask = contentmask;
		clip.start = start;
		clip.end = end;
		clip.mins = mins;
		clip.maxs = maxs;
		clip.passedict = passent;

		VectorCopy(mins, clip.mins2);
		VectorCopy(maxs, clip.maxs2);

		// Create the bounding box of the entire move.
		SV_TraceBounds(start, clip.mins2, clip.maxs2, end, clip.boxmins, clip.boxmaxs);

		// Clip to other solid entities (including brush entities).
		SV_ClipMoveToEntities(&clip);
	}
}

//mxd. Very similar to SV_ClipMoveToEntities().
static void SV_FindCosestEntity(const moveclip_t* clip) // H2
{
	edict_t* touchlist[MAX_EDICTS];
	trace_t trace;

	const int num = SV_AreaEdicts(clip->boxmins, clip->boxmaxs, touchlist, MAX_EDICTS, AREA_SOLID);

	// Be careful, it is possible to have an entity in this list removed before we get to it (killtriggered).
	for (int i = 0; i < num; i++)
	{
		edict_t* touch = touchlist[i];

		if (touch->solid == SOLID_NOT || touch == clip->passedict)
			continue;

		// Don't clip against own missiles or owner.
		if (clip->passedict != NULL)
		{
			if (clip->passedict == touch->owner || clip->passedict->owner == touch)
				continue;

			if (clip->passedict->owner != NULL && touch->owner != NULL && clip->passedict->owner == touch->owner) // Extra check not present in SV_ClipMoveToEntities
				continue;
		}

		if (!(clip->contentmask & CONTENTS_DEADMONSTER) && (touch->svflags & SVF_DEADMONSTER))
			continue;

		// Might intersect, so do an exact clip.
		const int headnode = SV_HullForEntity(touch);
		const float* angles = ((touch->solid == SOLID_BSP) ? touch->s.angles : vec3_origin); // Boxes don't rotate.

		if (touch->svflags & SVF_MONSTER)
			CM_TransformedBoxTrace(clip->start, clip->end, clip->mins2, clip->maxs2, headnode, clip->contentmask, touch->s.origin, angles, &trace);
		else
			CM_TransformedBoxTrace(clip->start, clip->end, clip->mins, clip->maxs, headnode, clip->contentmask, touch->s.origin, angles, &trace);

		if (trace.startsolid || trace.allsolid || trace.fraction < clip->trace->fraction)
		{
			trace.ent = touch;

			// H2: copy to clip->trace.
			const qboolean startsolid = clip->trace->startsolid;
			memcpy(clip->trace, &trace, sizeof(trace));
			if (startsolid)
				clip->trace->startsolid = true;

			if (clip->trace->allsolid)
				return;
		}
	}
}

void SV_TraceBoundingForm(FormMove_t* formMove) // H2
{
	vec3_t start;
	vec3_t end;
	
	VectorCopy(formMove->start, start);
	VectorCopy(formMove->end, end);

	moveclip_t clip;
	clip.trace = &formMove->trace;

	CM_BoxTrace(start, end, formMove->mins, formMove->maxs, 0, formMove->clipmask, clip.trace);
	formMove->trace.ent = ge->edicts;

	if (clip.trace->fraction == 0.0f)
		return;

	clip.passedict = formMove->pass_entity;
	clip.contentmask = formMove->clipmask;
	clip.start = start;
	clip.end = end;
	clip.mins = formMove->mins;
	clip.maxs = formMove->maxs;

	VectorCopy(formMove->mins, clip.mins2);
	VectorCopy(formMove->maxs, clip.maxs2);

	SV_TraceBounds(start, clip.mins2, clip.maxs2, end, clip.boxmins, clip.boxmaxs);
	SV_FindCosestEntity(&clip);
}

qboolean SV_ResizeBoundingForm(edict_t* self, FormMove_t* formMove) // H2
{
	FormMove_t form;
	vec3_t start;
	vec3_t end;
	vec3_t intent_mins;
	vec3_t intent_maxs;
	vec3_t* v_src;
	vec3_t* v_dst;
	int axis;
	float sign;

	VectorCopy(self->intentMins, intent_mins);
	VectorCopy(self->intentMaxs, intent_maxs);
	VectorCopy(self->mins, form.mins);
	VectorCopy(self->maxs, form.maxs);
	VectorCopy(self->s.origin, start);

	form.pass_entity = self;
	form.clipmask = self->clipmask;
	form.start = start;
	form.end = end; //mxd. 'end' is uninitialized here.

	for (int i = 0; i < 6; i++)
	{
		if (i & 1)
		{
			v_src = &form.maxs;
			v_dst = &intent_maxs;

			if (i & 4)
				axis = 2;
			else if (i & 2)
				axis = 0;
			else
				axis = 1;

			sign = 1.0f;
		}
		else
		{
			v_src = &form.mins;
			v_dst = &intent_mins;

			if (i & 4)
				axis = 2;
			else if (i & 2)
				axis = 1;
			else
				axis = 0;

			sign = -1.0f;
		}

		float offset = (*v_dst)[axis] - (*v_src)[axis];

		if (offset * sign > 0.0f)
		{
			VectorCopy(start, end);

			if (i == 4 && self->groundentity != NULL)
			{
				form.trace.fraction = 0.0f;
			}
			else
			{
				end[axis] += offset;
				SV_TraceBoundingForm(&form);

				if (form.trace.startsolid || form.trace.allsolid)
				{
					if (form.trace.allsolid)
						Com_Printf("self %d, trace allsolid in Box_BoundingForm_Resize\n", self->s.number);
					else if (form.trace.startsolid)
						Com_Printf("self %d, trace startsolid in Box_BoundingForm_Resize\n", self->s.number);

					memcpy(&formMove->trace, &form.trace, sizeof(trace_t));
					return false;
				}
			}

			if (form.trace.fraction < 1.0f)
			{
				offset = (1.0f - form.trace.fraction) * offset;
				end[axis] = start[axis] - offset;

				SV_TraceBoundingForm(&form);

				if (form.trace.fraction != 1.0f)
				{
					memcpy(&formMove->trace, &form.trace, sizeof(trace_t));
					return false;
				}

				start[axis] -= offset;
			}
		}

		(*v_src)[axis] = (*v_dst)[axis];
	}

	VectorCopy(start, self->s.origin);
	VectorCopy(intent_mins, self->mins);
	VectorCopy(intent_maxs, self->maxs);

	SV_LinkEdict(self);
	memcpy(&formMove->trace, &form.trace, sizeof(trace_t));

	return true;
}

int SV_GetContentsAtPoint(const vec3_t point) // H2
{
	vec3_t mins;
	vec3_t maxs;
	SinglyLinkedList_t list;

	int contents = CM_PointContents(point, sv.models[1]->headnode);

	VectorCopy(point, mins);
	VectorCopy(mins, maxs);

	SLList_DefaultCon(&list);
	SV_FindEntitiesInBounds(mins, maxs, &list, AREA_SOLID);
  
	while (!SLList_IsEmpty(&list))
	{
		const edict_t* ent = SLList_Pop(&list).t_edict_p;
		contents |= CM_TransformedPointContents(point, SV_HullForEntity(ent), ent->s.origin, ent->s.angles);
	}

	SLList_Des(&list);

	return contents;
}

qboolean SV_CheckDistances(const vec3_t origin, const float dist) // H2
{
	client_t* cl = &svs.clients[0];
	for (int i = 0; i < (int)maxclients->value; i++, cl++)
	{
		if (cl->state != cs_spawned)
			continue;

		vec3_t diff;
		VectorSubtract(origin, cl->edict->s.origin, diff);

		if (VectorLength(diff) < dist)
			return true;
	}

	return false;
}