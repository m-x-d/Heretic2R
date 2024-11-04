/*
==============================================================================
// g_env.c
//
// Heretic II
// Copyright 1998 Raven Software
==============================================================================
*/
#include "fx.h"
#include "g_local.h"
#include "vector.h"
#include "random.h"


void env_dust_use(edict_t *self, edict_t *other, edict_t *activator)
{	
	byte	count, magb;
	float	mag;

	gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE,self->moveinfo.sound_middle, 2, ATTN_NORM, 0);

	if(!self->count)
	{
		count = (byte)(self->size[0] * self->size[1] / 800.0);   //  28 x 28
		if (count > 16)
		{
			count = 16;
		}
	}
	else
	{
		count = self->count;
	}

	mag = VectorLength(self->size);
	magb = Clamp(mag, 1.0, 255.0);
	gi.CreateEffect(NULL,
					FX_DUST,
					0,
					self->mins,
					"bdb",
					(byte) count,
					self->size,
					magb
					);

}

/*QUAKED env_dust (1 .5 0) ? 
Generates dust and rock over an area. This is triggerable.
-------KEYS--------
count - number of rocks (default 1 rock per 28 x 28 square)
*/
void SP_env_dust (edict_t *self)
{

	self->movetype = PHYSICSTYPE_NONE;
	self->solid = SOLID_NOT;

	self->svflags |= SVF_NOCLIENT;

	self->use = env_dust_use;

	self->moveinfo.sound_middle = gi.soundindex ("world/quakeshort.wav");

	gi.setmodel (self, self->model);
	gi.linkentity (self);
}

#define START_OFF 8

void smoke_use (edict_t *self, edict_t *other, edict_t *activator)
{
	vec3_t	dir;
	byte	scale,speed,wait,maxrange;
	if (self->spawnflags & START_OFF)
	{
		scale = (byte)(self->s.scale * 32.0);
		AngleVectors(self->s.angles, dir, NULL, NULL);

		speed = Q_ftol(self->speed);
		wait = Q_ftol(self->wait);
		maxrange = Q_ftol(self->maxrange);

		self->PersistantCFX = gi.CreatePersistantEffect(&self->s,
								FX_ENVSMOKE,
								CEF_BROADCAST,self->s.origin,
								"bdbbb",scale,dir,speed,wait,maxrange);

		self->s.sound = gi.soundindex("ambient/fountainloop.wav");
		self->s.sound_data = (127 & ENT_VOL_MASK) | ATTN_STATIC;
		self->spawnflags &= ~START_OFF;
	}
	else
	{
		if (self->PersistantCFX)
		{
			gi.RemovePersistantEffect(self->PersistantCFX, REMOVE_SMOKE);
			self->PersistantCFX = 0;
		}
		gi.RemoveEffects(&self->s, FX_ENVSMOKE);
		self->spawnflags |= START_OFF;
	}
}

/*QUAKED env_smoke (1 .5 0) (-4 -4 -4) (4 4 4)  START_OFF
Generates steady puffs of smoke.  This is triggerable.
START_OFF - smoke will start off
-------KEYS--------
scale - size of puff  (default 1) range 0 - 8.
angle - direction puff is to move (default 0)
speed - how quickly puffs move (default 100) range 10 - 2500
distance - how far smoke will move before disappearing (default 100) range 1 - 255
wait - time in seconds between puffs (default 5) range 1 - 255
*/
void SP_env_smoke (edict_t *self)
{
	vec3_t	dir;
	byte	scale,speed,wait,maxrange;

	// set scale
	if (!self->s.scale)
		self->s.scale = 1;
	scale = (byte)(self->s.scale * 32.0);

	// allow us to use this stuff
	if (self->targetname)
		self->use = smoke_use;

	// set the wait between puffs
   	if (!self->wait)
		self->wait = 5;

	// set the distance
	if (st.distance)
		self->maxrange = st.distance;
	else
		self->maxrange = 100;

	// set the speed
	if (!self->speed)
		self->speed = 100;

	// reduce out the resolution
	self->speed = self->speed / 10;

	// make us all bytes
	speed = Q_ftol(self->speed);
	wait = Q_ftol(self->wait);
	maxrange = Q_ftol(self->maxrange);

	self->s.effects |= EF_NODRAW_ALWAYS_SEND;
	gi.linkentity(self);

	if (self->spawnflags & START_OFF)	// Start off
	{
		return;
	}

	else
	{
		AngleVectors(self->s.angles, dir, NULL, NULL);
		self->PersistantCFX = gi.CreatePersistantEffect(&self->s,
								FX_ENVSMOKE,
								CEF_BROADCAST,self->s.origin,
								"bdbbb",scale,dir,speed,wait,maxrange);
	}
}

/*QUAKED env_muck (1 .5 0) ? 
-------KEYS--------
*/
void SP_env_muck (edict_t *self)
{

	self->movetype = PHYSICSTYPE_NONE;
	self->solid = SOLID_NOT;

	self->svflags |= SVF_NOCLIENT;

	self->use = env_dust_use;

	gi.setmodel (self, self->model);
	gi.linkentity (self);
}
