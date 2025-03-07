//
// g_flamethrower.c
//
// Copyright 1998 Raven Software
//

#include "g_combat.h" //mxd
#include "g_DefaultMessageHandler.h"
#include "FX.h"
#include "Vector.h"
#include "g_local.h"

#define SF_STEAM			1
#define SF_MONSTERTOUCH		2

#define FLAMETHROWER_ON		(-2.0f) //mxd
#define FLAMETHROWER_OFF	(-1.0f) //mxd

void flamethrower_touch( edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf );

static void FlamethrowerThink(edict_t* self) //mxd. Named 'flamethrower_trigger' in original logic.
{
	vec3_t dir;
	AngleVectors(self->s.angles, dir, NULL, NULL);

	const int fx_flags = ((self->spawnflags & SF_STEAM) ? CEF_FLAG6 : 0);
	gi.CreateEffect(NULL, FX_FLAMETHROWER, fx_flags, self->s.origin, "df", dir, self->speed);

	self->monsterinfo.attack_finished = level.time + self->wait;

	if (self->wait == FLAMETHROWER_ON)
	{
		self->think = FlamethrowerThink;
		self->nextthink = level.time + 1.0f;
	}
	else
	{
		self->think = NULL;
	}
}

static void FlamethrowerUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'flamethrower_use' in original logic.
{
	if (self->monsterinfo.attack_finished >= level.time)
		return;

	if (self->wait == FLAMETHROWER_ON)
		self->wait = FLAMETHROWER_OFF; // Toggle off.
	else if (self->wait == FLAMETHROWER_OFF)
		self->wait = FLAMETHROWER_ON; // Denote that we are toggled on.

	FlamethrowerThink(self);
}

void FlameThrower_Deactivate(edict_t *self, G_Message_t *msg)
{
	self->solid = SOLID_NOT;
	self->touch = NULL;
	self->use = NULL;
}


void FlameThrower_Activate(edict_t *self, G_Message_t *msg)
{
	self->solid = SOLID_TRIGGER;
	self->use = FlamethrowerUse;
	self->touch = flamethrower_touch;
	gi.linkentity (self);
}


void FlameThrowerStaticsInit()
{
	classStatics[CID_FLAMETHROWER].msgReceivers[G_MSG_SUSPEND] = FlameThrower_Deactivate;
	classStatics[CID_FLAMETHROWER].msgReceivers[G_MSG_UNSUSPEND] = FlameThrower_Activate;
}

void flamethrower_touch( edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf )
{
	vec3_t	dir;
	int flags = 0;

	//Not toggled on, so don't damage
	if (self->wait == -1)
		return;

	if (!other->client && (!(other->svflags & SVF_MONSTER) || !(self->spawnflags&SF_MONSTERTOUCH)))
		return;

	AngleVectors(self->s.angles, dir, NULL, NULL);

	if (other->takedamage)
	{
		if (self->spawnflags & SF_STEAM)
			T_Damage(other, self, self, dir, other->s.origin, plane->normal, self->dmg, 0, 
					DAMAGE_AVOID_ARMOR|DAMAGE_NO_BLOOD,MOD_DIED);
		else
			T_Damage(other, self, self, dir, other->s.origin, plane->normal, self->dmg, 0, 
					DAMAGE_FIRE|DAMAGE_FIRE_LINGER|DAMAGE_AVOID_ARMOR|DAMAGE_NO_BLOOD,MOD_DIED);
	}

	if (self->monsterinfo.attack_finished < level.time && (self->wait > 0))
	{
		if (self->spawnflags & SF_STEAM)
			flags |= CEF_FLAG6;

		gi.CreateEffect( NULL, FX_FLAMETHROWER, flags, self->s.origin, "df", dir, self->speed);
		self->monsterinfo.attack_finished = level.time + self->wait;
	}
}

/*QUAKED flamethrower (.5 .5 .5) ? STEAM MONSTERTOUCH
A jet of flame

If steam is checked, it is a steam jet

MONSTERTOUCH - will allow monsters to set it off

--------SETUP----------

------KEYS-----------
dmg - damage per frame (1/10 of a second) (default 2)
wait - delay between each burst (default 2) (-1 signifies it is a toggled effect)
angles - Direction burst is to move in
speed - velocity of the burst (default 400)
*/

void SP_flamethrower(edict_t *self)
{
	self->msgHandler = DefaultMsgHandler;
	self->classID = CID_FLAMETHROWER;

	self->movetype = PHYSICSTYPE_NONE;
	self->solid = SOLID_TRIGGER;

	if (!self->wait)
		self->wait = 2;

	if (!self->dmg)
		self->dmg = 2;

	if (!self->speed)
		self->speed = 400.0f;

	self->svflags |= SVF_NOCLIENT;
	gi.setmodel (self, self->model);
	
	self->use = FlamethrowerUse;
	self->touch = flamethrower_touch;
	gi.linkentity (self);
}

