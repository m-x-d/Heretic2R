// G_flamethrow.c
// Heretic II
// 
// jweier

#include "g_local.h"
#include "Vector.h"
#include "FX.h"
#include "g_playstats.h"
#include "g_DefaultMessageHandler.h"

#define FLAMETHROWER_STEAM 1
#define FLAMETHROWER_MONSTERTOUCH 2

void flamethrower_use( edict_t *self, edict_t *other, edict_t *activator );
void flamethrower_touch( edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf );

void FlameThrower_Deactivate(edict_t *self, G_Message_t *msg)
{
	self->solid = SOLID_NOT;
	self->touch = NULL;
	self->use = NULL;
}


void FlameThrower_Activate(edict_t *self, G_Message_t *msg)
{
	self->solid = SOLID_TRIGGER;
	self->use = flamethrower_use;
	self->touch = flamethrower_touch;
	gi.linkentity (self);
}


void FlameThrowerStaticsInit()
{
	classStatics[CID_FLAMETHROWER].msgReceivers[G_MSG_SUSPEND] = FlameThrower_Deactivate;
	classStatics[CID_FLAMETHROWER].msgReceivers[G_MSG_UNSUSPEND] = FlameThrower_Activate;
}


void flamethrower_trigger( edict_t *self )
{
	vec3_t	dir;
	int		flags = 0;

	AngleVectors(self->s.angles, dir, NULL, NULL);

	if (self->spawnflags & FLAMETHROWER_STEAM)
		flags |= CEF_FLAG6;

	gi.CreateEffect( NULL, FX_FLAMETHROWER, flags, self->s.origin, "df", dir, self->speed);

	self->monsterinfo.attack_finished = level.time + self->wait;

	if (self->wait == -2)
	{
		self->nextthink = level.time + 1;
		self->think = flamethrower_trigger;
	}
	else
	{
		self->think = NULL;
	}
}

void flamethrower_use( edict_t *self, edict_t *other, edict_t *activator )
{
	vec3_t	dir;
	int		flags = 0;

	AngleVectors(self->s.angles, dir, NULL, NULL);

	if (self->monsterinfo.attack_finished < level.time)
	{
		//Toggle off?
		if (self->wait == -2)
		{
			//Toggle off
			self->wait = -1;
			//gi.RemoveEffects( &self->s, FX_FLAMETHROWER );
		}
		else if (self->wait == -1)
		{
			//Denote that we are toggled on
			self->wait = -2;
		}

		flamethrower_trigger( self );
	}
}

void flamethrower_touch( edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf )
{
	vec3_t	dir;
	int flags = 0;

	//Not toggled on, so don't damage
	if (self->wait == -1)
		return;

	if (!other->client && (!(other->svflags & SVF_MONSTER) || !(self->spawnflags&FLAMETHROWER_MONSTERTOUCH)))
		return;

	AngleVectors(self->s.angles, dir, NULL, NULL);

	if (other->takedamage)
	{
		if (self->spawnflags & FLAMETHROWER_STEAM)
			T_Damage(other, self, self, dir, other->s.origin, plane->normal, self->dmg, 0, 
					DAMAGE_AVOID_ARMOR|DAMAGE_NO_BLOOD,MOD_DIED);
		else
			T_Damage(other, self, self, dir, other->s.origin, plane->normal, self->dmg, 0, 
					DAMAGE_FIRE|DAMAGE_FIRE_LINGER|DAMAGE_AVOID_ARMOR|DAMAGE_NO_BLOOD,MOD_DIED);
	}

	if (self->monsterinfo.attack_finished < level.time && (self->wait > 0))
	{
		if (self->spawnflags & FLAMETHROWER_STEAM)
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
	
	self->use = flamethrower_use;
	self->touch = flamethrower_touch;
	gi.linkentity (self);
}

