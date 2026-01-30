//
// fx_fire.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Particle.h"
#include "Vector.h"
#include "Random.h"
#include "Utilities.h"
#include "ce_DLight.h"
#include "g_playstats.h"

#define FLARE_COUNT			16
#define FLARE_SPEED			64.0f
#define FLARE_SPAWN_RADIUS	2.0f
#define FLARE_ACCEL			128.0f
#define FLARE_SCALE			32.0f

#define FLAME_COUNT			4
#define FIRE_SPAWN_RADIUS	8.0f
#define FIRE_SCALE			12.0f
#define FIRE_ENT_SCALE		8.0f
#define FIRE_ACCEL			32.0f

static struct model_s* flareup_model;
static const paletteRGBA_t fire_color = { .c = 0xe5007fff };

void PreCacheFlareup(void)
{
	flareup_model = fxi.RegisterModel("sprites/fx/halo.sp2");
}

void FXFlareup(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	// Add a big ol' flash.
	client_entity_t* spawner = ClientEntity_new(type, flags | CEF_ADDITIVE_PARTS, origin, NULL, 500);
	spawner->r.model = &flareup_model; // The starry halo.
	spawner->r.flags = RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	spawner->radius = 128.0f;

	spawner->d_scale = -2.0f;
	spawner->alpha = 0.95f;
	spawner->d_alpha = -2.0f;
	spawner->color = color_white;

	AddEffect(NULL, spawner);

	// Add fire particles.
	const int count = GetScaledCount(FLARE_COUNT, 0.9f);
	for (int i = 0; i < count; i++)
	{
		const int p_type = irand(PART_32x32_FIRE0, PART_32x32_FIRE2); //mxd
		client_particle_t* flame = ClientParticle_new((int)(p_type | PFL_NEARCULL), spawner->color, 1000);

		const float radius = spawner->r.scale * FLARE_SPAWN_RADIUS;
		VectorRandomSet(flame->origin, radius); //mxd

		flame->scale = spawner->r.scale * FLARE_SCALE;
		VectorRandomSet(flame->velocity, FLARE_SPEED); //mxd
		flame->acceleration[2] = spawner->r.scale * FLARE_ACCEL;
		flame->d_scale = flrand(-20.0f, -10.0f);
		flame->d_alpha = flrand(-320.0f, -256.0f);

		flame->type |= PFL_ADDITIVE;

		AddParticleToList(spawner, flame);
	}
}

static qboolean FireSpawnerUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXFireThink' in original logic.
{
	int count = GetScaledCount(FLAME_COUNT, 0.9f);
	count = min(FLAME_COUNT, count); // Don't go over flame count.

	for (int i = 0; i < count; i++)
	{
		if (irand(0, 15) == 0 && R_DETAIL >= DETAIL_NORMAL) // No steam in software, it's around too long and doesn't do enough for us.
		{
			const byte white = (byte)irand(50, 100);
			const paletteRGBA_t color = { .r = white, .g = white, .b = white, .a = (byte)irand(100, 150) };

			client_particle_t* flame = ClientParticle_new((int)(PART_32x32_STEAM | PFL_NEARCULL), color, 2000);

			const float radius = self->r.scale * FIRE_SPAWN_RADIUS;
			VectorSet(flame->origin, flrand(-radius, radius), flrand(-radius, radius), self->r.scale * 8.0f);

			flame->scale = self->r.scale * FIRE_SCALE / 2.0f;
			VectorSet(flame->velocity, flrand(-5.0f, 5.0f), flrand(-5.0f, 5.0f), flrand(15.0f, 22.0f));
			flame->acceleration[2] = self->r.scale * FIRE_ACCEL;
			flame->d_scale = flrand(15.0f, 20.0f);
			flame->d_alpha = flrand(-100.0f, -50.0f);
			flame->duration = (int)((float)color.a * 1000.0f / -flame->d_alpha); // Time taken to reach zero alpha.

			AddParticleToList(self, flame);
		}
		else
		{
			client_particle_t* flame = ClientParticle_new((int)(irand(PART_32x32_FIRE0, PART_32x32_FIRE2) | PFL_NEARCULL), self->color, 2000);

			const float radius = self->r.scale * FIRE_SPAWN_RADIUS;
			VectorSet(flame->origin, flrand(-radius, radius), flrand(-radius, radius), flrand(-16.0f, -8.0f) * self->r.scale);

			flame->scale = self->r.scale * FIRE_SCALE;
			VectorSet(flame->velocity, flrand(-5.0f, 5.0f), flrand(-5.0f, 5.0f), flrand(15.0f, 22.0f));
			flame->acceleration[2] = self->r.scale * FIRE_ACCEL;
			flame->d_scale = flrand(-10.0f, -5.0f);
			flame->d_alpha = flrand(-200.0f, -160.0f);
			flame->duration = (int)(255.0f * 1000.0f / -flame->d_alpha); // Time taken to reach zero alpha.
			flame->type |= PFL_ADDITIVE;

			AddParticleToList(self, flame);
		}
	}

	if (self->dlight != NULL)
	{
		//mxd. Randomize color instead of intensity.
		self->dlight->color.r = (byte)((float)fire_color.r * flrand(0.6f, 1.0f));
		self->dlight->color.g = (byte)((float)fire_color.g * flrand(0.8f, 1.2f));
	}

	return true;
}

void FXFire(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	client_entity_t* spawner = ClientEntity_new(type, flags, origin, NULL, 17);

	byte scale;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_FIRE].formatString, &scale);
	spawner->r.scale = (float)scale / 32.0f;

	spawner->r.flags = (RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
	spawner->flags |= (CEF_NO_DRAW | CEF_NOMOVE | CEF_CULLED | CEF_CHECK_OWNER | CEF_VIEWSTATUSCHANGED);
	spawner->color = fire_color;
	spawner->radius = 96.0f;
	spawner->Update = FireSpawnerUpdate;

	if (flags & CEF_FLAG6)
		spawner->dlight = CE_DLight_new(spawner->color, 110.0f, 0.0f); //mxd. intensity:150 in original logic.

	AddEffect(owner, spawner);
}

static qboolean FireOnEntitySpawnerUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXFireOnEntityThink' in original logic.
{
	// For framerate-sensitive effect spawning.
	int count = GetScaledCount(FLAME_COUNT, 0.9f);
	count = min(FLAME_COUNT, count); // Don't go over flame count.

	VectorCopy(owner->origin, self->origin);
	VectorCopy(owner->origin, self->r.origin);

	if (!(owner->current.effects & EF_ON_FIRE) && self->nextEventTime - fx_time >= 1000)
	{
		// Set up the fire to finish early.
		self->nextEventTime = fx_time + 999;
		self->dlight->d_intensity = -200.0f;
	}

	if (self->nextEventTime - fx_time > 1000) // Until 1 second before finish.
	{
		for (int i = 0; i < count; i++)
		{
			if (irand(0, 15) == 0 && R_DETAIL >= DETAIL_NORMAL) // No steam in software, it's around too long and doesn't do enough for us.
			{
				const byte white = (byte)irand(8, 16);
				const paletteRGBA_t color = { .r = white, .g = white, .b = white, .a = 168 };

				client_particle_t* flame = ClientParticle_new((int)(PART_32x32_STEAM | PFL_NEARCULL), color, 2000);

				const float radius = self->r.scale;
				VectorSet(flame->origin, flrand(-radius, radius), flrand(-radius, radius), flrand(-0.25f, 0.75f) * self->r.scale);
				Vec3AddAssign(self->origin, flame->origin);

				flame->scale = self->r.scale * 2.0f;
				VectorSet(flame->velocity, flrand(-5.0f, 5.0f), flrand(-5.0f, 5.0f), flrand(15.0f, 22.0f));
				flame->acceleration[2] = self->r.scale * FIRE_ACCEL;
				flame->d_scale = flrand(15.0f, 20.0f);
				flame->d_alpha = flrand(-100.0f, -50.0f);
				flame->duration = (int)(255.0f * 1000.0f / -flame->d_alpha); // Time taken to reach zero alpha.

				AddParticleToList(self, flame);
			}
			else
			{
				client_particle_t* flame = ClientParticle_new((int)(irand(PART_32x32_FIRE0, PART_32x32_FIRE2) | PFL_NEARCULL), self->color, 1000);

				const float radius = self->r.scale;
				VectorSet(flame->origin, flrand(-radius, radius), flrand(-radius, radius), flrand(-0.25f, 0.75f) * radius);

				// If dead, then move the flame down a tad.
				if (owner->current.effects & EF_DISABLE_EXTRA_FX)
					self->origin[2] -= radius;

				Vec3AddAssign(self->origin, flame->origin);

				flame->scale = self->r.scale * 2.0f;
				VectorSet(flame->velocity, flrand(-5.0f, 5.0f), flrand(-5.0f, 5.0f), flrand(32.0f, 48.0f));
				flame->acceleration[2] = self->r.scale * 2.0f;
				flame->d_scale = flrand(-20.0f, -10.0f);
				flame->d_alpha = flrand(-400.0f, -320.0f);
				flame->duration = (int)(255.0f * 1000.0f / -flame->d_alpha); // Time taken to reach zero alpha.

				AddParticleToList(self, flame);
			}
		}

		self->dlight->intensity = 150.0f + flrand(-8.0f, 8.0f);
		return true;
	}

	if (fx_time < self->nextEventTime)
	{
		self->dlight->intensity = (float)(self->nextEventTime - fx_time) * 0.3f;
		return true;
	}

	return false;
}

static qboolean FireOnEntityPersistentSpawnerUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXFireOnEntity2Think' in original logic.
{
	//FIXME: can miss the message that tells you to remove the effect.
	if (fx_time > self->nextEventTime)
		return false; // Just in case.

	if (self->nextEventTime - fx_time < 1000)
		return true; // Let the flames finish.

	if (!(owner->current.effects & EF_ON_FIRE) && self->nextEventTime - fx_time >= 1000)
	{
		self->nextEventTime = fx_time + 999;
		self->dlight->d_intensity = -200;
	}

	// For framerate-sensitive effect spawning.
	int count = GetScaledCount(FLAME_COUNT, 0.9f);
	count = min(FLAME_COUNT, count); // Don't go over flame count.

	VectorCopy(owner->origin, self->origin);
	VectorCopy(owner->origin, self->r.origin);

	for (int i = 0; i < count; i++)
	{
		client_particle_t* flame = ClientParticle_new((int)(irand(PART_32x32_FIRE0, PART_32x32_FIRE2) | PFL_NEARCULL), self->color, 1000);

		const float radius = self->r.scale * FIRE_SPAWN_RADIUS;
		VectorSet(flame->origin, flrand(-radius, radius), flrand(-radius, radius), flrand(-8.0f, 0.0f) * self->r.scale);

		// If dead, then move the flame down a tad.
		if (owner->current.effects & EF_DISABLE_EXTRA_FX)
			self->origin[2] -= radius;

		Vec3AddAssign(self->origin, flame->origin);

		flame->scale = self->r.scale * FIRE_ENT_SCALE;
		VectorSet(flame->velocity, flrand(-5.0f, 5.0f), flrand(-5.0f, 5.0f), flrand(15.0f, 22.0f));
		flame->acceleration[2] = self->r.scale * FIRE_ACCEL;
		flame->d_scale = flrand(-10.0f, -5.0f);
		flame->d_alpha = flrand(-200.0f, -160.0f);
		flame->duration = (int)(255.0f * 1000.0f / -flame->d_alpha); // Time taken to reach zero alpha.

		AddParticleToList(self, flame);
	}

	self->dlight->intensity = 150.0f + flrand(-8.0f, 8.0f);

	return true;
}

//FIXME: have it constantly check a flag so it can go out if under water!
//TODO: for skeletal entities, use refpoints to spawn fire instead of origin? 
void FXFireOnEntity(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	byte scale;
	byte lifetime;
	byte style;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_FIRE_ON_ENTITY].formatString, &scale, &lifetime, &style);

	client_entity_t* spawner = ClientEntity_new(type, flags, origin, NULL, 17);

	spawner->r.scale = sqrtf(scale) * 0.5f;
	spawner->nextEventTime = fx_time + (int)lifetime * 100; // How long to last. Lifetime was in 10th secs.

	spawner->r.flags = (RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
	spawner->flags |= (CEF_NO_DRAW | CEF_NOMOVE | CEF_ADDITIVE_PARTS | CEF_ABSOLUTE_PARTS | CEF_CULLED | CEF_CHECK_OWNER);
	spawner->color = fire_color;
	spawner->radius = 128.0f;

	if (style == 0) // Fire fades out - for things that catch fire.
	{
		spawner->Update = FireOnEntitySpawnerUpdate;
	}
	else // Fire never goes away - for moving fire ents.
	{
		spawner->Update = FireOnEntityPersistentSpawnerUpdate;
		spawner->nextEventTime = fx_time + 60000; // 60 seconds max, just in case.
	}

	spawner->dlight = CE_DLight_new(spawner->color, 150.0f, 0.0f);

	AddEffect(owner, spawner);
}