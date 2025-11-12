//
// g_env.c
//
// Copyright 1998 Raven Software
//

#include "g_env.h" //mxd
#include "FX.h"
#include "Vector.h"
#include "g_local.h"

#pragma region ========================== env_dust, env_muck ==========================

void EnvDustUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'env_dust_use' in original logic.
{
	gi.sound(self, CHAN_NO_PHS_ADD + CHAN_VOICE, self->moveinfo.sound_middle, 2.0f, ATTN_NORM, 0.0f); //TODO: why volume is 2.0?

	byte count;
	if (self->count == 0)
	{
		count = (byte)(self->size[0] * self->size[1] / 800.0f); // 28 x 28
		count = min(16, count);
	}
	else
	{
		count = (byte)self->count;
	}

	const byte b_len = (byte)(Clamp(VectorLength(self->size), 1.0f, 255.0f));
	gi.CreateEffect(NULL, FX_DUST, 0, self->mins, "bdb", count, self->size, b_len);
}

// QUAKED env_dust (1 .5 0) ?
// Generates dust and rock over an area. This is triggerable.

// Variables:
// count - number of rocks (default 1 rock per 28 x 28 square).
void SP_env_dust(edict_t* self)
{
	self->movetype = PHYSICSTYPE_NONE;
	self->solid = SOLID_NOT;
	self->svflags |= SVF_NOCLIENT;
	self->moveinfo.sound_middle = gi.soundindex("world/quakeshort.wav"); //mxd. Only difference between this and SP_env_muck. Could've used a spawnflag instead...
	self->use = EnvDustUse;

	gi.setmodel(self, self->model);
	gi.linkentity(self);
}

// QUAKED env_muck (1 .5 0) ?
void SP_env_muck(edict_t* self)
{
	self->movetype = PHYSICSTYPE_NONE;
	self->solid = SOLID_NOT;
	self->svflags |= SVF_NOCLIENT;
	self->use = EnvDustUse;

	gi.setmodel(self, self->model);
	gi.linkentity(self);
}

#pragma endregion

#pragma region ========================== env_smoke ==========================

#define SF_START_OFF	8

static void SetupEnvSmokeEffect(edict_t* self) //mxd. Added to reduce code duplication.
{
	vec3_t dir;
	AngleVectors(self->s.angles, dir, NULL, NULL);

	const byte scale = (byte)(self->s.scale * 32.0f);
	const byte speed = (byte)self->speed;
	const byte wait = (byte)self->wait;
	const byte max_range = (byte)self->maxrange;

	self->PersistantCFX = gi.CreatePersistantEffect(&self->s, FX_ENVSMOKE, CEF_BROADCAST, self->s.origin, "bdbbb", scale, dir, speed, wait, max_range);
}

void EnvSmokeUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'smoke_use' in original logic.
{
	if (self->spawnflags & SF_START_OFF)
	{
		SetupEnvSmokeEffect(self); //mxd
		self->s.sound = (byte)gi.soundindex("ambient/fountainloop.wav");
		self->s.sound_data = (127 & ENT_VOL_MASK) | ATTN_STATIC;
		self->spawnflags &= ~SF_START_OFF;
	}
	else
	{
		if (self->PersistantCFX > 0)
		{
			gi.RemovePersistantEffect(self->PersistantCFX, REMOVE_SMOKE);
			self->PersistantCFX = 0;
		}

		gi.RemoveEffects(&self->s, FX_ENVSMOKE);
		self->spawnflags |= SF_START_OFF;
	}
}

// QUAKED env_smoke (1 .5 0) (-4 -4 -4) (4 4 4)  START_OFF
// Generates steady puffs of smoke. This is triggerable.

// Spawnflags:
// START_OFF - smoke will start off.

// Variables:
// scale	- size of puff (default 1) range 0 - 8.
// angle	- direction puff is to move (default 0).
// speed	- how quickly puffs move (default 100) range 10 - 2500.
// distance	- how far smoke will move before disappearing (default 100) range 1 - 255.
// wait		- time in seconds between puffs (default 5) range 1 - 255.
void SP_env_smoke(edict_t* self)
{
	// Set scale.
	if (self->s.scale == 0.0f)
		self->s.scale = 1.0f;

	// Allow us to use this stuff.
	if (self->targetname != NULL)
		self->use = EnvSmokeUse;

	// Set the wait between puffs.
	if (self->wait == 0.0f)
		self->wait = 5.0f;

	// Set the distance.
	self->maxrange = ((st.distance > 0) ? (float)st.distance : 100.0f);

	// Set the speed.
	if (self->speed == 0.0f)
		self->speed = 100.0f;

	// Reduce out the resolution.
	self->speed /= 10.0f;

	self->s.effects |= EF_NODRAW_ALWAYS_SEND;

	gi.linkentity(self);

	if (!(self->spawnflags & SF_START_OFF)) // Start off?
		SetupEnvSmokeEffect(self); //mxd
}

#pragma endregion

#pragma region ========================== env_sun1 ==========================

// QUAKED env_sun1 (1 .5 0) (-12 -12 0) (12 12 38)
// Lens flare effect (unfinished).
void SP_env_sun1(edict_t* self)
{
	self->solid = SOLID_NOT;
	self->movetype = PHYSICSTYPE_NONE;

	const vec3_t origin = { 200.0f, -100.0f, 4000.0f }; //TODO: get sun direction from info_null instead? Get color from _color field?..
	gi.CreatePersistantEffect(NULL, FX_LENSFLARE, CEF_FLAG7 | CEF_FLAG6, origin, "bbbf", 128, 108, 64, 0.75f);
}

#pragma endregion