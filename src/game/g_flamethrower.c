//
// g_flamethrower.c
//
// Copyright 1998 Raven Software
//

#include "g_flamethrower.h" //mxd
#include "g_combat.h" //mxd
#include "g_DefaultMessageHandler.h"
#include "FX.h"
#include "Vector.h"
#include "g_local.h"

#define SF_STEAM			1
#define SF_MONSTERTOUCH		2

#define FLAMETHROWER_ON		(-2.0f) //mxd
#define FLAMETHROWER_OFF	(-1.0f) //mxd

void FlamethrowerThink(edict_t* self) //mxd. Named 'flamethrower_trigger' in original logic.
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

void FlamethrowerUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'flamethrower_use' in original logic.
{
	if (self->monsterinfo.attack_finished >= level.time)
		return;

	if (self->wait == FLAMETHROWER_ON)
		self->wait = FLAMETHROWER_OFF; // Toggle off.
	else if (self->wait == FLAMETHROWER_OFF)
		self->wait = FLAMETHROWER_ON; // Denote that we are toggled on.

	FlamethrowerThink(self);
}

void FlamethrowerTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf) //mxd. Named 'flamethrower_touch' in original logic.
{
	// Not toggled on, so don't damage.
	if (self->wait == FLAMETHROWER_OFF)
		return;

	if (other->client == NULL && (!(other->svflags & SVF_MONSTER) || !(self->spawnflags & SF_MONSTERTOUCH)))
		return;

	vec3_t dir;
	AngleVectors(self->s.angles, dir, NULL, NULL);

	if (other->takedamage != DAMAGE_NO)
	{
		int dmf_flags = (DAMAGE_AVOID_ARMOR | DAMAGE_NO_BLOOD);
		if (!(self->spawnflags & SF_STEAM))
			dmf_flags |= DAMAGE_FIRE | DAMAGE_FIRE_LINGER;

		T_Damage(other, self, self, dir, other->s.origin, plane->normal, self->dmg, 0, dmf_flags, MOD_DIED);
	}

	if (self->monsterinfo.attack_finished < level.time && self->wait > 0.0f)
	{
		const int fx_flags = ((self->spawnflags & SF_STEAM) ? CEF_FLAG6 : 0);
		gi.CreateEffect(NULL, FX_FLAMETHROWER, fx_flags, self->s.origin, "df", dir, self->speed);
		self->monsterinfo.attack_finished = level.time + self->wait;
	}
}

static void FlamethrowerDeactivate(edict_t* self, G_Message_t* msg) //mxd. Named 'FlameThrower_Deactivate' in original logic.
{
	self->solid = SOLID_NOT;
	self->use = NULL;
	self->touch = NULL;
}

static void FlamethrowerActivate(edict_t* self, G_Message_t* msg) //mxd. Named 'FlameThrower_Activate' in original logic.
{
	self->solid = SOLID_TRIGGER;
	self->use = FlamethrowerUse;
	self->touch = FlamethrowerTouch;

	gi.linkentity(self);
}

void FlamethrowerStaticsInit(void)
{
	classStatics[CID_FLAMETHROWER].msgReceivers[G_MSG_SUSPEND] = FlamethrowerDeactivate;
	classStatics[CID_FLAMETHROWER].msgReceivers[G_MSG_UNSUSPEND] = FlamethrowerActivate;
}

// QUAKED flamethrower (.5 .5 .5) ? STEAM MONSTERTOUCH
// A jet of flame.

// Spawnflags:
// STEAM		- If steam is checked, it is a steam jet.
// MONSTERTOUCH	- Will allow monsters to set it off.

// Variables:
// dmg		- Damage per frame (1/10 of a second) (default 2).
// wait		- Delay between each burst (default 2) (-1 signifies it is a toggled effect).
// angles	- Direction burst is to move in.
// speed	- Velocity of the burst (default 400).
void SP_flamethrower(edict_t* self)
{
	self->movetype = PHYSICSTYPE_NONE;
	self->solid = SOLID_TRIGGER;
	self->svflags |= SVF_NOCLIENT;
	self->msgHandler = DefaultMsgHandler;

	if (self->wait == 0.0f) //TODO: 'wait' is used to both store on/off state and active time duration. Store on/off state in different saveable property (like count or style)?
		self->wait = 2.0f;

	if (self->dmg == 0)
		self->dmg = 2;

	if (self->speed == 0.0f)
		self->speed = 400.0f;

	self->use = FlamethrowerUse;
	self->touch = FlamethrowerTouch;

	gi.setmodel(self, self->model);
	gi.linkentity(self);
}