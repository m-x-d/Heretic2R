//
// g_breakable.c
//
// Copyright 1998 Raven Software
//

#include "g_breakable.h" //mxd
#include "g_obj.h" //mxd
#include "g_DefaultMessageHandler.h"
#include "FX.h"
#include "Vector.h"
#include "g_local.h"

#define SF_KILLALL			1
#define SF_NOLINK			2
#define SF_INVULNERABLE		16
#define SF_INVULNERABLE2	32 //mxd. Original logic sets brush as invulnerable when spawnflag 16 or 32 is set.
#define SF_PUSHABLE			64 //mxd
#define SF_NOPLAYERDAMAGE	128 //mxd

void KillBrush(edict_t* target, edict_t* inflictor, edict_t* attacker, const int damage)
{
	const edict_t* start = target;

	if (start->spawnflags & SF_KILLALL)
	{
		do
		{
			edict_t* other = target->enemy;
			G_PostMessage(target, MSG_DEATH, PRI_DIRECTIVE, "eeei", target, inflictor, attacker, damage);
			target = other;
		} while (target != start);
	}
	else
	{
		G_PostMessage(target, MSG_DEATH, PRI_DIRECTIVE, "eeei", target, inflictor, attacker, damage);
	}
}

static void KillBrushUse(edict_t* target, edict_t* inflictor, edict_t* attacker)
{
	G_PostMessage(target, MSG_DEATH, PRI_DIRECTIVE, "eeei", target, inflictor, attacker, 0);
}

void BBrushStaticsInit(void)
{
	classStatics[CID_BBRUSH].msgReceivers[MSG_DEATH] = DefaultObjectDieHandler;
}

static qboolean EntitiesTouching(const edict_t* e1, const edict_t* e2)
{
	for (int i = 0; i < 3; i++)
		if (e1->mins[i] > e2->maxs[i] || e1->maxs[i] < e2->mins[i])
			return false;

	return true;
}

// Used to link up brushes that have KILLALL set.
static void LinkBreakables(edict_t* self)
{
	self->think = NULL;

	if (self->enemy != NULL) // Already linked by another breakable.
		return;

	edict_t* t = self;
	edict_t* ent = self;

	while (true)
	{
		ent->owner = self; // Master breakable.

		if (ent->health > 0)
			self->health = ent->health;

		if (ent->targetname != NULL)
			self->targetname = ent->targetname;

		t = G_Find(t, FOFS(classname), ent->classname);

		if (t == NULL)
		{
			ent->enemy = self; // Make the chain a loop.
			return;
		}

		if (ent->spawnflags & SF_NOLINK)
			continue;

		if (EntitiesTouching(self, t))
		{
			if (t->enemy != NULL)
				return;

			ent->enemy = t;
			ent = t;
		}
	}
}

// QUAKED breakable_brush (1 .5 0) ? KILLALL NOLINK x x INVULNERABLE INVULNERABLE PUSHABLE NOPLAYERDAMAGE
// A brush that explodes.

// Spawnflags:
// KILLALL			- kills any brushes touching this one.
// NOLINK			- can touch a KILLALL brush and not be linked to it.
// INVULNERABLE		- if set, it can't be hurt.
// PUSHABLE			- can be pushed by player.
// NOPLAYERDAMAGE	- can't be damaged by player.

// Variables:
// health - amount of damage the brush can take before exploding.
// materialtype:	0 = STONE; 1 = GREYSTONE (default); 2 = CLOTH; 3 = METAL; 4 = FLESH; 5 = POTTERY;
//					6 = GLASS; 7 = LEAF; 8 = WOOD; 9 = BROWNSTONE; 10 = NONE - just makes smoke.
void SP_breakable_brush(edict_t* ent)
{
	ent->msgHandler = DefaultMsgHandler;

	if (ent->materialtype == 0) //TODO: it's impossible to set STONE (0) materialtype! Can't be fixed without either adjusting vanilla maps or adding vanilla map-fixing logic similar to Q2...
		ent->materialtype = MAT_GREYSTONE;

	if (ent->health == 0)
		ent->health = 1;

	if (ent->spawnflags & SF_NOPLAYERDAMAGE)
		ent->svflags |= SVF_NO_PLAYER_DAMAGE;

	ent->takedamage = ((ent->spawnflags & (SF_INVULNERABLE | SF_INVULNERABLE2)) ? DAMAGE_NO : DAMAGE_YES); //mxd. Preserve original logic... //TODO: are both of these spawnflags used in vanilla maps?
	ent->movetype = ((ent->spawnflags & SF_PUSHABLE) ? PHYSICSTYPE_PUSH : PHYSICSTYPE_NONE);
	ent->solid = SOLID_BSP;
	ent->use = KillBrushUse;

	gi.setmodel(ent, ent->model);
	gi.linkentity(ent);

	// Use size to calculate mass.
	vec3_t space;
	VectorSubtract(ent->maxs, ent->mins, space);
	ent->mass = (int)((space[0] * space[1] * space[2]) / 64.0f);

	ent->nextthink = level.time + FRAMETIME;
	ent->think = LinkBreakables;
}