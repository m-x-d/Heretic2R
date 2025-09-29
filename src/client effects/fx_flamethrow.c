//
// fx_flamethrower.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "fx_flamethrow.h" //mxd
#include "Particle.h"
#include "Vector.h"
#include "Utilities.h"
#include "Random.h"
#include "g_playstats.h"
#include "ce_DLight.h"

#define FLAME_COUNT		4

static struct sfx_s* steamjet_sound;
static struct sfx_s* flamethrower_sound;

void PreCacheFlamethrowerSFX(void) //mxd
{
	steamjet_sound = fxi.S_RegisterSound("objects/steamjet.wav");
	flamethrower_sound = fxi.S_RegisterSound("misc/flamethrow.wav");
}

qboolean FXFlamethrowerTrail(client_entity_t* self, centity_t* owner)
{
	if (self->LifeTime < fxi.cl->time)
	{
		self->Update = RemoveSelfAI;
		self->nextThinkTime = fxi.cl->time + 2000; //BUGFIX: mxd. sets updateTime in original logic (makes no sense: updateTime is ADDED to fxi.cl->time in UpdateEffects()).

		return true;
	}

	const int count = GetScaledCount(FLAME_COUNT, 0.9f);

	for (int i = 0; i < count; i++)
	{
		client_particle_t* flame = ClientParticle_new(irand(PART_16x16_FIRE1, PART_16x16_FIRE3), color_white, 2000);
		flame->d_scale = flrand(-10.0f, -5.0f);
		flame->d_alpha = flrand(-600.0f, -560.0f);
		flame->duration = (int)(255.0f * 1000.0f / -flame->d_alpha); // Time taken to reach zero alpha.

		VectorCopy(self->direction, flame->velocity);
		VectorSet(flame->origin, (float)(irand(-2, 2)), (float)(irand(-2, 2)), (float)(irand(-2, 2))); //TODO: why irand?
		VectorScale(flame->velocity, flrand(0.75f, 1.0f), flame->velocity);

		flame->scale = flrand(14.0f, 20.0f);
		flame->velocity[2] += flrand(0.0f, 48.0f);
		flame->acceleration[2] = 64.0f;

		AddParticleToList(self, flame);
	}

	if (R_DETAIL >= DETAIL_NORMAL)
	{
		// Spawn a dynamic light.
		client_entity_t* light = ClientEntity_new(FX_FLAMETHROWER, CEF_NO_DRAW, self->origin, 0, 500);
		VectorCopy(self->direction, light->velocity);

		const byte white = (byte)irand(8, 16);
		const paletteRGBA_t color = { .r = 128 + (byte)irand(108, 127), .g = 64 + white, .b = 16 + white, .a = 64 + (byte)irand(16, 128) };

		light->dlight = CE_DLight_new(color, 150.0f, -100.0f);

		AddEffect(NULL, light);
	}

	return true;
}

static qboolean FlamethrowerSteamTrail(client_entity_t* self, centity_t* owner)
{
	if (self->LifeTime < fxi.cl->time)
	{
		self->Update = RemoveSelfAI;
		self->nextThinkTime = fxi.cl->time + 2000; //BUGFIX: mxd. sets updateTime in original logic (makes no sense: updateTime is ADDED to fxi.cl->time in UpdateEffects()).

		return true;
	}

	const int count = GetScaledCount(FLAME_COUNT, 0.9f);

	for (int i = 0; i < count; i++)
	{
		const paletteRGBA_t color = { .c = ((self->flags & CEF_FLAG7) ? 0x33777777 : 0x50ffffff) } ;
		client_particle_t* flame = ClientParticle_new(PART_32x32_STEAM, color, 2000);

		flame->d_alpha = flrand(-200.0f, -150.0f);
		flame->duration = (int)(255.0f * 1000.0f / -flame->d_alpha); // Time taken to reach zero alpha.

		VectorCopy(self->direction, flame->velocity);
		VectorSet(flame->origin, (float)(irand(-2, 2)), (float)(irand(-2, 2)), (float)(irand(-2, 2))); //TODO: why irand?
		VectorScale(flame->velocity, flrand(0.75f, 1.0f), flame->velocity);

		if (self->flags & CEF_FLAG7)
		{
			flame->d_scale = flrand(20.0f, 30.0f);
			flame->scale = flrand(2.0f, 3.0f);
			VectorScale(flame->velocity, -0.1f, flame->acceleration);
		}
		else
		{
			flame->d_scale = flrand(-10.0f, -5.0f);
			flame->scale = flrand(14.0f, 20.0f);
		}

		flame->velocity[2] += flrand(0.0f, 32.0f);
		flame->acceleration[2] = 128.0f;

		AddParticleToList(self, flame);
	}

	return true;
}

// Create the initial fire entity that we can attach all the particles to.
void FXFlamethrower(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	vec3_t dir;
	float distance;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_FLAMETHROWER].formatString, &dir, &distance);

	// Create the dummy entity, so particles can be attached.
	client_entity_t* glow = ClientEntity_new(type, (int)(flags | CEF_NO_DRAW | CEF_ADDITIVE_PARTS), origin, NULL, 17);

	VectorScale(dir, distance, glow->direction);
	glow->radius = 100.0f;
	glow->LifeTime = fxi.cl->time + 1000;

	// Steam?
	if (flags & CEF_FLAG6)
	{
		if (flags & CEF_FLAG7)
			glow->LifeTime = fxi.cl->time + 200;
		else
			fxi.S_StartSound(origin, -1, CHAN_AUTO, steamjet_sound, 1.0f, ATTN_NORM, 0.0f);

		glow->Update = FlamethrowerSteamTrail;
	}
	else
	{
		fxi.S_StartSound(origin, -1, CHAN_AUTO, flamethrower_sound, 1.0f, ATTN_NORM, 0.0f);
		glow->Update = FXFlamethrowerTrail;
	}

	AddEffect(NULL, glow);
}

// Put out there just so we can make the real effect match this.
void FXflametest(centity_t* owner, int type, int flags, vec3_t origin) { } //TODO: remove?