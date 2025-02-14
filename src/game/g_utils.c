//
// g_utils.c -- misc utility functions for game module
//
// Copyright 1998 Raven Software
//

#include "g_local.h"
#include "g_combat.h" //mxd
#include "g_Physics.h"
#include "g_Skeletons.h"
#include "FX.h"
#include "Random.h"
#include "Vector.h"

void G_ProjectSource(const vec3_t point, const vec3_t distance, const vec3_t forward, const vec3_t right, vec3_t result)
{
	result[0] = point[0] + forward[0] * distance[0] + right[0] * distance[1];
	result[1] = point[1] + forward[1] * distance[0] + right[1] * distance[1];
	result[2] = point[2] + forward[2] * distance[0] + right[2] * distance[1] + distance[2];
}

void G_SetToFree(edict_t* self)
{
	if (self->PersistantCFX > 0)
	{
		gi.RemovePersistantEffect(self->PersistantCFX, REMOVE_ENTITY);
		self->PersistantCFX = 0;
	}

	self->think = G_FreeEdict;
	self->nextthink = level.time + FRAMETIME;
	self->svflags &= ~SVF_NOCLIENT;

	self->next_pre_think = -1;
	self->next_post_think = -1;

	self->takedamage = DAMAGE_NO;
	self->movetype = PHYSICSTYPE_NONE;
	self->solid = SOLID_NOT;
	self->touch = NULL;
	self->blocked = NULL;
	self->isBlocked = NULL;
	self->isBlocking = NULL;
	self->bounced = NULL;
	VectorClear(self->mins);
	VectorClear(self->maxs);

	gi.linkentity(self);
}

// Searches all active entities for the next one that holds the matching string at fieldofs (use the FOFS() macro) in the structure.
// Searches beginning at the edict after 'from', or the beginning if NULL.
// NULL will be returned if the end of the list is reached.
edict_t* G_Find(edict_t* from, const int fieldofs, const char* match)
{
	// If we aren't trying to find anything, then exit.
	if (match == NULL)
		return NULL;

	if (from == NULL)
		from = &g_edicts[0];
	else
		from++;

	for (; from < &g_edicts[globals.num_edicts]; from++)
	{
		if (from->inuse)
		{
			const char* s = *(char**)((byte*)from + fieldofs);
			if (s != NULL && Q_stricmp(s, match) == 0)
				return from;
		}
	}

	return NULL;
}

// This works like findradius, except it uses the bbox of an ent to indicate the area to check.
edict_t* findinblocking(edict_t* from, const edict_t* checkent) //TODO: rename to FindInBlocking, rename checkent to check_ent.
{
	static vec3_t min;
	static vec3_t max;

	if (from == NULL)
	{
		VectorAdd(checkent->s.origin, checkent->mins, min);
		VectorAdd(checkent->s.origin, checkent->maxs, max);
	}

	while (true)
	{
		from = findinbounds(from, min, max);
		if (from == NULL)
			return NULL;

		if (from->inuse && from != checkent)
			return from;
	}
}

// Returns entities that have origins within a spherical area.
edict_t* findradius(edict_t* from, const vec3_t org, const float radius) //TODO: rename to FindRadius
{
	static float radius_sq;
	static vec3_t min;
	static vec3_t max;

	if (from == NULL)
	{
		radius_sq = radius * radius;
		VectorCopy(org, min);
		VectorCopy(org, max);

		for (int i = 0; i < 3; i++)
		{
			min[i] -= radius;
			max[i] += radius;
		}
	}

	while (true)
	{
		from = findinbounds(from, min, max);

		if (from == NULL)
			return NULL;

		if (from->inuse)
		{
			vec3_t ent_org;
			for (int i = 0; i < 3; i++)
				ent_org[i] = org[i] - (from->s.origin[i] + (from->mins[i] + from->maxs[i]) * 0.5f);

			if (VectorLengthSquared(ent_org) <= radius_sq)
				return from;
		}
	}
}

edict_t* findinbounds(const edict_t* from, const vec3_t min, const vec3_t max) //TODO: rename to FindInBounds
{
	static edict_t* touch_list[MAX_EDICTS];
	static int index = -1;
	static int count;

	if (from == NULL)
	{
		count = gi.BoxEdicts(min, max, touch_list, MAX_EDICTS, AREA_SOLID);
		index = 0;
	}
	else
	{
		// You cannot adjust the pointers yourself... This means you did not call it with the previous edict.
		assert(touch_list[index] == from);
		index++;
	}

	for (; index < count; index++)
		if (touch_list[index]->inuse)
			return touch_list[index];

	return NULL;
}

// Searches all active entities for the next one that holds the matching string at fieldofs (use the FOFS() macro) in the structure.
// Searches beginning at the edict after 'from', or the beginning if NULL.
// NULL will be returned if the end of the list is reached.
edict_t* G_PickTarget(const char* targetname)
{
#define MAXCHOICES	8

	edict_t* choices[MAXCHOICES];
	edict_t* ent = NULL;
	int num_choices = 0;

	if (targetname == NULL)
	{
		gi.dprintf("G_PickTarget called with NULL targetname\n");
		return NULL;
	}

	while (num_choices < MAXCHOICES)
	{
		ent = G_Find(ent, FOFS(targetname), targetname);
		if (ent == NULL)
			break;

		choices[num_choices++] = ent;
	}

	if (num_choices > 0)
		return choices[irand(0, num_choices - 1)];

	gi.dprintf("G_PickTarget: target '%s' not found\n", targetname);
	return NULL;
}

static void Delay_Think(edict_t* ent) //mxd. Named 'Think_Delay' in original version.
{
	G_UseTargets(ent, ent->activator);
	G_FreeEdict(ent);
}

// The global "activator" should be set to the entity that initiated the firing.
// If self.delay is set, a DelayedUse entity will be created that will actually do the SUB_UseTargets after that many seconds have passed.
// Centerprints any self.message to the activator.
// Searches for (string)targetname in all entities that match (string)self.target and calls their .use function.
void G_UseTargets(edict_t* ent, edict_t* activator)
{
	// Check for a delay.
	if (ent->delay > 0.0f)
	{
		if (activator == NULL) //mxd. Added sanity check.
		{
			gi.dprintf("Delayed G_UseTargets with no activator from '%s' at %s\n", ent->classname, vtos(ent->s.origin));
			return;
		}

		// Create a temp object to fire at a later time.
		edict_t* delay = G_Spawn();

		delay->movetype = PHYSICSTYPE_NONE;
		delay->classname = "DelayedUse";
		delay->nextthink = level.time + ent->delay;
		delay->think = Delay_Think;
		delay->activator = activator;
		delay->message = ent->message;
		delay->text_msg = ent->text_msg;
		delay->target = ent->target;
		delay->killtarget = ent->killtarget;

		return;
	}

	// Print messages.
	if (!(activator->svflags & SVF_MONSTER))
	{
		if (ent->message != NULL)
		{
			gi.levelmsg_centerprintf(activator, (short)Q_atoi(ent->message));

			if (ent->noise_index > 0)
				gi.sound(activator, CHAN_AUTO, ent->noise_index, 1.0f, ATTN_NORM, 0.0f);
		}

		if (ent->text_msg != NULL)
			gi.centerprintf(activator, "%s", ent->text_msg);
	}

	// Kill killtargets.
	if (ent->killtarget != NULL)
	{
		edict_t* killtarget = NULL;
		while ((killtarget = G_Find(killtarget, FOFS(targetname), ent->killtarget)) != NULL)
		{
			QPostMessage(killtarget, MSG_DEATH, PRI_DIRECTIVE, "eeei", killtarget, ent, activator, 100000);

			if (!ent->inuse)
			{
				gi.dprintf("Entity was removed while using killtargets\n");
				return;
			}
		}
	}

	// Fire targets.
	if (ent->target != NULL)
	{
		edict_t* target = NULL;
		while ((target = G_Find(target, FOFS(targetname), ent->target)) != NULL)
		{
			// Doors fire area portals in a specific way. //TODO: but what about func_door_secret?..
			if (Q_stricmp(target->classname, "func_areaportal") == 0 && (Q_stricmp(ent->classname, "func_door") == 0 || Q_stricmp(ent->classname, "func_door_rotating") == 0))
				continue;

			if (target == ent)
				gi.dprintf("WARNING: %s at %s used itself.\n", target->classname, vtos(target->s.origin)); //mxd. Print origin.
			else if (target->use != NULL)
				target->use(target, ent, activator);

			if (!ent->inuse)
			{
				gi.dprintf("Entity was removed while using targets\n");
				return;
			}
		}
	}
}

qboolean PossessCorrectItem(const edict_t* ent, const gitem_t* item)
{
	if (ent->target_ent == NULL)
		return false;

	ent = ent->target_ent;

	edict_t* target = NULL;
	while ((target = G_Find(target, FOFS(targetname), ent->target)))
	{
		// Doors fire area portals in a specific way. //TODO: but what about func_door_secret?..
		if (!Q_stricmp(target->classname, "func_areaportal") && (!Q_stricmp(ent->classname, "func_door") || !Q_stricmp(ent->classname, "func_door_rotating")))
			continue;

		if (target->item == item)
			return true;
	}

	return false;
}

// VectorToString. This is just a convenience function for printing vectors.
char* vtos(const vec3_t v)
{
	static int index;
	static char str[8][32];

	// Use an array so that multiple vtos calls won't collide.
	char* s = str[index];
	index = (index + 1) & 7;

	Com_sprintf(s, sizeof(str[0]), "(%i %i %i)", (int)v[0], (int)v[1], (int)v[2]);

	return s;
}

void G_SetMovedir(vec3_t angles, vec3_t movedir)
{
	static const vec3_t vec_up =		{ 0.0f, -1.0f,  0.0f };
	static const vec3_t movedir_up =	{ 0.0f,  0.0f,  1.0f };
	static const vec3_t vec_down =		{ 0.0f, -2.0f,  0.0f };
	static const vec3_t movedir_down =	{ 0.0f,  0.0f, -1.0f };

	if (VectorCompare(angles, vec_up))
		VectorCopy(movedir_up, movedir);
	else if (VectorCompare(angles, vec_down))
		VectorCopy(movedir_down, movedir);
	else
		AngleVectors(angles, movedir, NULL, NULL);

	VectorClear(angles);
}

float vectoyaw(const vec3_t v) //TODO: rename to VectorYaw.
{
	if (v[YAW] == 0.0f && v[PITCH] == 0.0f)
		return 0.0f;

	float yaw = atan2f(v[YAW], v[PITCH]) * RAD_TO_ANGLE;
	if (yaw < 0.0f)
		yaw += 360.0f;

	return yaw;
}

void G_InitEdict(edict_t* self)
{
	self->s.clientEffects.buf = NULL;
	self->s.clientEffects.bufSize = 0;
	self->s.clientEffects.freeBlock = 0;
	self->s.clientEffects.numEffects = 0;

	self->inuse = true;
	self->movetype = PHYSICSTYPE_NONE;
	self->classname = "noclass";
	self->gravity = 1.0f;
	self->friction = 1.0f;
	self->elasticity = ELASTICITY_SLIDE;
	self->s.number = (short)(self - g_edicts);
	self->s.scale = 1.0f;
	self->msgHandler = NULL;
	self->svflags = 0;
	self->client_sent = 0;
	self->just_deleted = 0;
	self->reflected_time = level.time;
}

/*
=================
G_Spawn

Either finds a free edict, or allocates a new one.
Try to avoid reusing an entity that was recently freed, because it
can cause the client to think the entity morphed into something else
instead of being removed and recreated, which can cause interpolated
angles and bad trails.
=================
*/
edict_t *G_Spawn (void)
{
	int			i;
	edict_t		*e;
	//static unsigned int entID = 0;

	e = &g_edicts[(int)maxclients->value+1];
	for(i=maxclients->value + 1; i < globals.num_edicts; ++i, ++e)
	{
		// the first couple seconds of server time can involve a lot of
		// freeing and allocating, so relax the replacement policy
		if(!e->inuse && e->freetime <= level.time)
		{
			G_InitEdict (e);

			++e->s.usageCount;
			return e;
		}
	}

	if (i == game.maxentities)
	{
		assert(0);
		gi.error ("ED_Alloc: Spawning more than %d edicts", game.maxentities);
	}
		
	globals.num_edicts++;
	G_InitEdict (e);
	return e;
}

/*
=================
G_FreeEdict

Marks the edict as free
=================
*/
void G_FreeEdict(edict_t *self)
{
	SinglyLinkedList_t msgs;
	char *temp;
	unsigned int	usageCount;
	int		server_seen;
	int		entnum;

	gi.unlinkentity (self);		// unlink from world

	// From Quake2 3.17 code release.

	if ((self - g_edicts) <= (maxclients->value + BODY_QUEUE_SIZE))
	{
#ifdef _DEVEL
		gi.dprintf("tried to free special edict\n");
#endif
		return;
	}

	// Start non-quake2.

	// Portals need to be marked as open even if they are freed in deathmatch, only when deliberately removed for netplay.
	if (self->classname && level.time <= 0.2)			// Just upon startup
	{
		if (Q_stricmp(self->classname, "func_areaportal") == 0)
			gi.SetAreaPortalState (self->style, true);
	}

	if(self->s.effects & EF_JOINTED)
	{
		FreeSkeleton(self->s.rootJoint);
	}

	if(self->s.clientEffects.buf)
	{
		temp = self->s.clientEffects.buf; // buffer needs to be stored to be cleared by the engine
	}
	else
	{
		temp = NULL;
	}

	msgs = self->msgQ.msgs;
	usageCount = self->s.usageCount;
	server_seen = self->client_sent;
	entnum = self->s.number;

	// End non-quake2.

	memset(self, 0, sizeof(*self));

	// Start non-quake2.

	self->s.usageCount = usageCount;
	self->msgQ.msgs = msgs;
	self->s.clientEffects.buf = temp;
	self->just_deleted = SERVER_DELETED;
	self->client_sent = server_seen;
	self->s.number = entnum;

	// End non-quake2.

	self->classname = "freed";
	self->freetime = level.time + 2.0;
	self->inuse = false;
	self->s.skeletalType = SKEL_NULL;

	self->svflags = SVF_NOCLIENT;	// so it will get removed from the client properly
}


/*
============
G_TouchTriggers

============
*/
void	G_TouchTriggers (edict_t *ent)
{
	int			i, num;
	edict_t		*touch[MAX_EDICTS], *hit;

	// dead things don't activate triggers!
	if ((ent->client || (ent->svflags & SVF_MONSTER)) && (ent->health <= 0))
		return;

	num = gi.BoxEdicts (ent->absmin, ent->absmax, touch
		, MAX_EDICTS, AREA_TRIGGERS);

	// be careful, it is possible to have an entity in this
	// list removed before we get to it (killtriggered)
	for (i=0 ; i<num ; i++)
	{
		hit = touch[i];
		if (!hit->inuse)
			continue;
		if (!hit->touch)
			continue;
		hit->touch (hit, ent, NULL, NULL);
	}
}

/*
============
G_TouchSolids

Call after linking a new trigger in during gameplay
to force all entities it covers to immediately touch it
============
*/
void	G_TouchSolids (edict_t *ent)
{
	int			i, num;
	edict_t		*touch[MAX_EDICTS], *hit;

	num = gi.BoxEdicts (ent->absmin, ent->absmax, touch
		, MAX_EDICTS, AREA_SOLID);

	// be careful, it is possible to have an entity in this
	// list removed before we get to it (killtriggered)
	for (i=0 ; i<num ; i++)
	{
		hit = touch[i];
		if (!hit->inuse)
			continue;
		if (ent->touch)
			ent->touch (hit, ent, NULL, NULL);
		if (!ent->inuse)
			break;
	}
}




/*
==============================================================================

Kill box

==============================================================================
*/

/*
=================
KillBox

Kills all entities that would touch the proposed new positioning
of ent.  Ent should be unlinked before calling this!
=================
*/
qboolean KillBox (edict_t *ent)
{
	edict_t *current=NULL;
	vec3_t	mins, maxs;

	// since we can't trust the absmin and absmax to be set correctly on entry, I'll create my own versions

	VectorAdd(ent->s.origin, ent->mins, mins);
	VectorAdd(ent->s.origin, ent->maxs, maxs);

	while (1)
	{
		current = findinbounds(current, mins, maxs);

		// don't allow us to kill the player
		if(current == ent)
			continue;

		// we've checked everything
		if(!current)
			break;

		// nail it
		if (current->takedamage)
			T_Damage (current, ent, ent, vec3_origin, ent->s.origin, vec3_origin, 100000, 0, 
					  DAMAGE_NO_PROTECTION|DAMAGE_AVOID_ARMOR|DAMAGE_HURT_FRIENDLY,MOD_TELEFRAG);
	
	}

	return true;		// all clear
}

/*
ClearBBox

returns true if there is nothing in you BBOX
*/

qboolean ClearBBox (edict_t *self)
{
	vec3_t	top, bottom, mins, maxs;
	trace_t	trace;
	VectorSet(mins, self->mins[0], self->mins[1], 0);
	VectorSet(maxs, self->maxs[0], self->maxs[1], 1);
	VectorSet(bottom, self->s.origin[0], self->s.origin[1], self->absmin[2]);
	VectorSet(top, self->s.origin[0], self->s.origin[1], self->absmax[2] - 1);

	gi.trace(top, mins, maxs, bottom, self, self->clipmask,&trace);
	if(trace.startsolid || trace.allsolid)
		return false;

	if(trace.fraction == 1.0)
		return true;

	return false;
}

/*
=================
oldfindradius

Returns entities that have origins within a spherical area

oldfindradius (origin, radius)
=================
*/
edict_t *oldfindradius (edict_t *from, vec3_t org, float rad)
{
	vec3_t	eorg;
	int		j;

	if (!from)
		from = g_edicts;
	else
		from++;
	for ( ; from < &g_edicts[globals.num_edicts]; from++)
	{
		if (!from->inuse)
			continue;
		if (from->solid == SOLID_NOT)
			continue;
		for (j=0 ; j<3 ; j++)
			eorg[j] = org[j] - (from->s.origin[j] + (from->mins[j] + from->maxs[j])*0.5);
		if (VectorLength(eorg) > rad)
			continue;
		return from;
	}

	return NULL;
}



// ========================================
// LinkMissile(edict_t *self)
//
// This is a way to kinda "cheat" the system.
// We don't want missiles to be considered for collision, 
// yet we want them to collide with other things.
// So when we link the entity (for rendering, etc) we set 
// SOLID_NOT so certain things don't happen.
// ========================================
void G_LinkMissile(edict_t *self) //TODO: mxd. Non-functional? Replace with gi.linkentity(self)?
{
    int oldsolid;
	
	oldsolid=self->solid;

//  self->solid=SOLID_NOT; // comment this line out for old behaviour
    gi.linkentity(self);
    self->solid=oldsolid;
}

