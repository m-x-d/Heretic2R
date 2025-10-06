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

	self->next_pre_think = -1.0f;
	self->next_post_think = -1.0f;

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

// This works like FindInRadius, except it uses the bbox of an ent to indicate the area to check.
edict_t* FindInBlocking(edict_t* from, const edict_t* check_ent) //mxd. Named 'findinblocking' in original version.
{
	static vec3_t min;
	static vec3_t max;

	if (from == NULL)
	{
		VectorAdd(check_ent->s.origin, check_ent->mins, min);
		VectorAdd(check_ent->s.origin, check_ent->maxs, max);
	}

	while (true)
	{
		from = FindInBounds(from, min, max);
		if (from == NULL)
			return NULL;

		if (from->inuse && from != check_ent)
			return from;
	}
}

// Returns entities that have origins within a spherical area.
edict_t* FindInRadius(edict_t* from, const vec3_t org, const float radius) //mxd. Named 'findradius' in original version.
{
	static float radius_sq;
	static vec3_t min;
	static vec3_t max;

	if (from == NULL) // First call.
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
		from = FindInBounds(from, min, max);

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

edict_t* FindInBounds(const edict_t* from, const vec3_t min, const vec3_t max) //mxd. Named 'findinbounds' in original version.
{
	static edict_t* touch_list[MAX_EDICTS];
	static int index = -1;
	static int count;

	if (from == NULL) // First call.
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

static void DelayThink(edict_t* ent) //mxd. Named 'Think_Delay' in original version.
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
		delay->think = DelayThink;
		delay->activator = activator;
		delay->message = ent->message;
		delay->text_msg = ent->text_msg;
		delay->target = ent->target;
		delay->killtarget = ent->killtarget;

		return;
	}

	// Print messages?
	if (activator != NULL && !(activator->svflags & SVF_MONSTER)) //mxd. Added activator sanity check (func_door with DOOR_MOVE_LOOP will call this with NULL activator).
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
			QPostMessage(killtarget, MSG_DEATH, PRI_DIRECTIVE, "eeei", killtarget, ent, activator, 100000); //TODO: activator can be NULL (e.g. func_door with DOOR_MOVE_LOOP enabled).

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
				target->use(target, ent, activator); //TODO: activator can be NULL (e.g. func_door with DOOR_MOVE_LOOP enabled).

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

float VectorYaw(const vec3_t v) //mxd. Named 'vectoyaw' in original version.
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

// Either finds a free edict, or allocates a new one.
// Try to avoid reusing an entity that was recently freed, because it can cause the client to think the entity morphed into something else
// instead of being removed and recreated, which can cause interpolated angles and bad trails.
edict_t* G_Spawn(void)
{
	int index = MAXCLIENTS + 1;
	edict_t* ent = &g_edicts[index];

	for (; index < globals.num_edicts; index++, ent++)
	{
		// The first couple seconds of server time can involve a lot of freeing and allocating, so relax the replacement policy.
		if (!ent->inuse && ent->freetime <= level.time)
		{
			G_InitEdict(ent);
			ent->s.usageCount++;

			return ent;
		}
	}

	if (index == game.maxentities)
	{
		gi.error("ED_Alloc: spawning more than %i edicts", game.maxentities);
		return NULL;
	}

	globals.num_edicts++;
	G_InitEdict(ent);

	return ent;
}

// Marks the edict as free.
void G_FreeEdict(edict_t* self)
{
	assert(self->PersistantCFX == 0); //mxd

	gi.unlinkentity(self); // Unlink from world.

	if (self - g_edicts <= MAXCLIENTS + BODY_QUEUE_SIZE)
	{
		gi.dprintf("Tried to free special edict\n");
		return;
	}

	// Upon startup, portals need to be marked as open even if they are freed in deathmatch, only when deliberately removed for netplay.
	if (level.time <= 0.2f && self->classname != NULL && Q_stricmp(self->classname, "func_areaportal") == 0)
		gi.SetAreaPortalState(self->style, true);

	if (self->s.effects & EF_JOINTED)
		FreeSkeleton(self->s.rootJoint);

	byte* fx_buf = self->s.clientEffects.buf; // Buffer needs to be stored to be cleared by the engine.
	const SinglyLinkedList_t msgs = self->msgQ.msgs;
	const int server_seen = self->client_sent;
	const byte usage_count = self->s.usageCount;
	const short ent_num = self->s.number;

	memset(self, 0, sizeof(*self));

	self->msgQ.msgs = msgs;
	self->s.clientEffects.buf = fx_buf;
	self->s.usageCount = usage_count;
	self->s.number = ent_num;
	self->s.skeletalType = SKEL_NULL;
	self->just_deleted = SERVER_DELETED;
	self->client_sent = server_seen;
	self->classname = "freed";
	self->freetime = level.time + 2.0f;
	self->inuse = false;

	self->svflags = SVF_NOCLIENT; // So it will get removed from the client properly.
}

void G_TouchTriggers(edict_t* ent)
{
	// Dead things don't activate triggers!
	if ((ent->client != NULL || (ent->svflags & SVF_MONSTER)) && ent->health <= 0)
		return;

	edict_t* touched[MAX_EDICTS];
	const int num_touched = gi.BoxEdicts(ent->absmin, ent->absmax, touched, MAX_EDICTS, AREA_TRIGGERS);

	// Be careful, it is possible to have an entity in this list removed before we get to it (killtriggered).
	for (int i = 0; i < num_touched; i++)
	{
		edict_t* hit = touched[i];
		if (hit->inuse && hit->touch != NULL)
			hit->touch(hit, ent, NULL, NULL);
	}
}

// Kills all entities that would touch the proposed new positioning of ent. Ent should be unlinked before calling this!
void KillBox(edict_t* ent)
{
	vec3_t mins;
	vec3_t maxs;

	// Since we can't trust the absmin and absmax to be set correctly on entry, I'll create my own versions.
	VectorAdd(ent->s.origin, ent->mins, mins);
	VectorAdd(ent->s.origin, ent->maxs, maxs);

	edict_t* e = NULL;
	const int dflags = DAMAGE_NO_PROTECTION | DAMAGE_AVOID_ARMOR | DAMAGE_HURT_FRIENDLY; //mxd

	while ((e = FindInBounds(e, mins, maxs)) != NULL)
		if (e != ent && e->takedamage != DAMAGE_NO)
			T_Damage(e, ent, ent, vec3_origin, ent->s.origin, vec3_origin, 100000, 0, dflags, MOD_TELEFRAG);
}

// Returns entities that have origins within a spherical area.
edict_t* FindInRadius_Old(edict_t* from, vec3_t org, const float radius) //mxd. Named 'oldfindradius' in original version. //TODO: used ONLY by ElfLord. Can be replaced with FindRadius()?
{
	if (from == NULL)
		from = &g_edicts[0];
	else
		from++;

	for (; from < &g_edicts[globals.num_edicts]; from++)
	{
		if (!from->inuse || from->solid == SOLID_NOT)
			continue;

		vec3_t ent_org;
		for (int i = 0; i < 3; i++)
			ent_org[i] = org[i] - (from->s.origin[i] + (from->mins[i] + from->maxs[i]) * 0.5f);

		if (VectorLength(ent_org) <= radius)
			return from;
	}

	return NULL;
}

// This is a way to kinda "cheat" the system.
// We don't want missiles to be considered for collision, yet we want them to collide with other things.
// So when we link the entity (for rendering, etc) we set SOLID_NOT so certain things don't happen.
void G_LinkMissile(edict_t* self) //TODO: mxd. Non-functional? Replace with gi.linkentity(self)?
{
	const int oldsolid = self->solid;
	//  self->solid=SOLID_NOT; // comment this line out for old behaviour
	gi.linkentity(self); //mxd. SV_LinkEdict() doesn't seem to change 'solid' prop.

	//mxd. dbg
	assert(oldsolid == self->solid);

	self->solid = oldsolid;
}