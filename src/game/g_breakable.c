//
// g_breakable.c
//
// Copyright 1998 Raven Software
//

#include "g_misc.h"
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
			QPostMessage(target, MSG_DEATH, PRI_DIRECTIVE, "eeei", target, inflictor, attacker, damage);
			target = other;
		} while (target != start);
	}
	else
	{
		QPostMessage(target, MSG_DEATH, PRI_DIRECTIVE, "eeei", target, inflictor, attacker, damage);
	}
}

static void KillBrushUse(edict_t* target, edict_t* inflictor, edict_t* attacker)
{
	QPostMessage(target, MSG_DEATH, PRI_DIRECTIVE, "eeei", target, inflictor, attacker, 0);
}

void BBrushStaticsInit(void)
{
	classStatics[CID_BBRUSH].msgReceivers[MSG_DEATH] = DefaultObjectDieHandler;
}

static void BBrushInit(edict_t *self) //TODO: remove
{

	self->movetype = PHYSICSTYPE_NONE;
	self->msgHandler = DefaultMsgHandler;
	self->classID = CID_BBRUSH;

	if (self->spawnflags & SF_INVULNERABLE2)
	{
		self->takedamage = DAMAGE_NO;
	}
	else
	{
		self->takedamage = DAMAGE_YES;
	}

}

static qboolean EntitiesTouching(const edict_t* e1, const edict_t* e2)
{
	for (int i = 0; i < 3; i++)
		if (e1->mins[i] > e2->maxs[i] || e1->maxs[i] < e2->mins[i])
			return false;

	return true;
}

/*--------------------------------------
  LinkBreakables - used to link up brushes that have KILLALL set
----------------------------------------*/
void LinkBreakables(edict_t *self)
{
  edict_t   *t, *ent;
  vec3_t	cmins, cmaxs;

	self->think = NULL;

	if (self->enemy) // already linked by another breakable
	{
		return;		
	}
	
	VectorCopy(self->mins,cmins);
	VectorCopy(self->maxs,cmaxs);
	
	t = ent = self;
	
	do
	{
		ent->owner = self;			// master breakable

		if (ent->health) 
		{
			self->health = ent->health;
		}

		if (ent->targetname) 
		{
			self->targetname = ent->targetname;
		}

		t = G_Find (t, FOFS(classname), ent->classname);


		if (!t)
		{
			ent->enemy = self;		// make the chain a loop
			ent = ent->owner;
			return;
		}

		if (ent->spawnflags & SF_NOLINK)
			continue;	

		if (EntitiesTouching(self,t))
		{		
			if (t->enemy) 
			{
				return;
			}
			
			ent->enemy = t;
			ent = t;
		}
	} while (1);
}

/*QUAKED breakable_brush (1 .5 0) ? KILLALL NOLINK ORDERED TRANSLUCENT INVULNERABLE INVISIBLE PUSHPULL NOTPLAYERDAMAGE

	A brush that explodes. 

NOTPLAYERDAMAGE - players cannot damage this brush

KILLALL - kills any brushes touching this one 

HIERARCH - kills any brushes touching this one 

NOLINK - can touch a KILLALL brush and not be linked to it

INVULNERABLE - if set it can't be hurt

*** VARIABLES ***
health - amount of damage the brush can take before exploding
materialtype - 
0 = STONE
1 = GREYSTONE (default)map 
2 = CLOTH
3 = METAL
4 = FLESH
5 = POTTERY
6 = GLASS
7 = LEAF
8 = WOOD
9 = BROWNSTONE
10 = NONE - just makes smoke


*/
void SP_breakable_brush (edict_t *ent)
{
	vec3_t space;
	float spacecube;

	BBrushInit(ent);

	if (!ent->materialtype)
		ent->materialtype = MAT_GREYSTONE;

	if (!ent->health)
		ent->health = 1;


	if (ent->spawnflags & 16)	// Invulnerable
	{
		ent->takedamage = DAMAGE_NO;
	}
	else
	{
		ent->takedamage = DAMAGE_YES;
	}


	if (ent->spawnflags & 64)
	{
		ent->movetype = PHYSICSTYPE_PUSH;
//		ent->think = M_droptofloor;
//		ent->nextthink = level.time + 2 * FRAMETIME;
	}
	else
		ent->movetype = PHYSICSTYPE_NONE;

	if (ent->spawnflags & 128)
		ent->svflags |= SVF_NO_PLAYER_DAMAGE;

	ent->solid = SOLID_BSP;

	ent->use = KillBrushUse;

	gi.setmodel (ent, ent->model);
	gi.linkentity (ent);

	// Use size to calculate mass
	VectorSubtract(ent->maxs, ent->mins, space);
	spacecube = space[0] * space[1] * space[2];
	ent->mass = spacecube / 64;   // 

	ent->nextthink = level.time + FRAMETIME;
	ent->think = LinkBreakables;
}
