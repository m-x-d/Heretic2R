/*
==============================================================================

// g_breakable.c
//
// Heretic II
// Copyright 1998 Raven Software


==============================================================================
*/
#include "g_local.h"
#include "g_misc.h"
#include "fx.h"
#include "g_DefaultMessageHandler.h"
#include "vector.h"

int BREAK_KILLALL		= 1;
int BREAK_NOLINK	    = 2;
int BREAK_CHECKNAME	    = 4;
int BREAK_ORDERED       = 8;
int BREAK_TRANSLUCENT   = 16;
int BREAK_INVULNERABLE  = 32;
int BREAK_INVISIBLE     = 64;


/*
void KillBrushOrdered(edict_t *self)
{
   local entity starte, oself, other;
	local float headNum;

	oself = starte = self;
	headNum = self.count;

	do
	{
	  if (self.count >= headNum) 
		self.th_die();

	  self = self.enemy;
	} while ( (self != starte) && (self != world) && (self.frags != self.cnt));
}
*/

void KillBrush(edict_t *targ,edict_t *inflictor,edict_t *attacker,int damage)
{
	edict_t *starte, *other;
	
	starte = targ;
	
	if (starte->spawnflags & BREAK_KILLALL)
	{
		do
		{
			other = targ->enemy;

			QPostMessage(targ,MSG_DEATH,PRI_DIRECTIVE,"eeei",targ,inflictor,attacker,damage);

			targ = other;
		} while ( (targ != starte) );
	}
//	else if (targ->spawnflags & BREAK_HIERARCH) 
//	{
//		;
//		KillBrushOrdered(targ);
//	}
	else 
	{
		QPostMessage(targ,MSG_DEATH,PRI_DIRECTIVE,"eeei",targ,inflictor,attacker,damage);
	}

//	this was firing off targets twice... fix it if this is not good,
//	but the MSG_DEATH handler for objects uses targets a second time!
//	G_UseTargets(targ, targ);
}


void KillBrushUse(edict_t *targ,edict_t *inflictor,edict_t *attacker)
{
	QPostMessage(targ,MSG_DEATH,PRI_DIRECTIVE,"eeei",targ,inflictor,attacker,0);
}

void BBrushStaticsInit(void) 
{
	classStatics[CID_BBRUSH].msgReceivers[MSG_DEATH] = DefaultObjectDieHandler;
}

void BBrushInit(edict_t *self)
{

	self->movetype = PHYSICSTYPE_NONE;
	self->msgHandler = DefaultMsgHandler;
	self->classID = CID_BBRUSH;

	if (self->spawnflags & BREAK_INVULNERABLE)
	{
		self->takedamage = DAMAGE_NO;
	}
	else
	{
		self->takedamage = DAMAGE_YES;
	}

}

qboolean EntitiesTouching(edict_t *e1,edict_t *e2)
{
	vec3_t e1max, e1min, e2max, e2min;

	VectorCopy(e1->maxs,e1max);
	VectorCopy(e1->mins,e1min);

	VectorCopy(e2->maxs,e2max);
	VectorCopy(e2->mins,e2min);

	if (e1min[0] > e2max[0])
		return (false);
	if (e1min[1] > e2max[1])
		return (false);
	if (e1min[2] > e2max[2])
		return (false);
	if (e1max[0] < e2min[0])
		return (false);
	if (e1max[1] < e2min[1])
		return (false);
	if (e1max[2] < e2min[2])
		return (false);
	return (true);
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

		if (ent->spawnflags & BREAK_NOLINK)
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
