//
// fx_WaterEntrySplash.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Particle.h"
#include "Vector.h"
#include "Random.h"
#include "g_playstats.h"

#define WATER_SPLASH_SCALE	0.3f
#define WATER_SPLASH_RADIUS	20.0f

static struct model_s* water_models[2];
static struct sfx_s* water_sounds[2]; //mxd

void PreCacheWaterSplash(void)
{
	water_models[0] = fxi.RegisterModel("sprites/fx/waterentryripple.sp2");
	water_models[1] = fxi.RegisterModel("sprites/fx/wfall.sp2");
}

void PreCacheWaterSplashSFX(void) //mxd
{
	water_sounds[0] = fxi.S_RegisterSound("misc/splish2.wav");
	water_sounds[1] = fxi.S_RegisterSound("misc/splish3.wav");
}

static qboolean WaterEntrySplashThinkerThink(struct client_entity_s* self, centity_t* owner)
{
	// Have enough ripples been created yet? If not, create one, else free myself.
	if (self->NoOfAnimFrames < 1)
		return false;

	// Create a water entry ripple.
	const int flags = (int)(self->flags & ~(CEF_OWNERS_ORIGIN | CEF_NO_DRAW)); //mxd
	client_entity_t* ripple = ClientEntity_new(FX_WATER_ENTRYSPLASH, flags, self->origin, self->direction, 1200);

	ripple->r.model = &water_models[0]; // waterentryripple sprite.
	ripple->r.flags = RF_FIXED | RF_TRANSLUCENT | RF_ALPHA_TEXTURE;
	ripple->alpha = 0.6f;
	ripple->d_alpha = self->d_alpha;

	if (self->flags & CEF_FLAG7)
	{
		// Random ripples.
		ripple->r.scale = 0.1f;
		ripple->d_scale = 0.7f;
		ripple->r.origin[0] += flrand(-6.0f, 6.0f);
		ripple->r.origin[1] += flrand(-6.0f, 6.0f);
	}
	else
	{
		ripple->r.scale = WATER_SPLASH_SCALE;
		ripple->d_scale = 1.0f;
	}

	AddEffect(NULL, ripple);
	self->NoOfAnimFrames--;

	return true;
}

void DoWaterEntrySplash(const int type, const int flags, vec3_t origin, const byte splash_size, vec3_t dir)
{
	int ripples_count;

	if (splash_size >= 128)
		ripples_count = 4;
	else if (splash_size >= 96)
		ripples_count = 3;
	else if (splash_size >= 64)
		ripples_count = 2;
	else
		ripples_count = 1;

	// They want a sound too?
	if (flags & CEF_FLAG6)
		fxi.S_StartSound(origin, -1, CHAN_AUTO, water_sounds[irand(0, 1)], 1.0f, ATTN_STATIC, 0.0f);

	// Create a water entry ripple THINKER that will create the actual water entry ripples.
	client_entity_t* entry_ripple_thinker = ClientEntity_new(type, flags, origin, dir, 200);

	entry_ripple_thinker->flags |= CEF_NO_DRAW;
	entry_ripple_thinker->NoOfAnimFrames = ripples_count;
	entry_ripple_thinker->Update = WaterEntrySplashThinkerThink;
	entry_ripple_thinker->d_alpha = -0.8f - (0.2f - (0.2f * (1.0f / 127.0f * (float)(splash_size & 127))));

	AddEffect(NULL, entry_ripple_thinker);

	// Create water entry particle splash if required.
	if (splash_size & 128)
	{
		float angle = 0.0f;
		float delta_angle = ANGLE_360 / 12.0f;

		for (int i = 0; i < 12; i++)
		{
			client_entity_t* splash = ClientEntity_new(type, flags, origin, dir, 500);

			splash->radius = 2.0f;
			splash->r.model = &water_models[1]; // wfall sprite.
			splash->r.scale = flrand(0.15f, 0.25f);
			splash->r.flags = RF_TRANSLUCENT;
			splash->r.frame = 1;
			splash->d_alpha = -2.0f;
			splash->d_scale = 1.0f;

			splash->origin[0] = WATER_SPLASH_RADIUS * cosf(angle);
			splash->origin[1] = WATER_SPLASH_RADIUS * sinf(angle);
			splash->origin[2] = 0.0f;

			splash->velocity[0] = flrand(4.0f, 6.0f) * splash->origin[0];
			splash->velocity[1] = flrand(4.0f, 6.0f) * splash->origin[1];
			splash->velocity[2] = flrand(30.0f, 40.0f);

			VectorSet(splash->acceleration, 0.0f, 0.0f, -250.0f);

			AddEffect(NULL, splash);

			angle += delta_angle;
		}

		angle = 0.0f;
		delta_angle = ANGLE_360 / 6.0f;

		for (int i = 0; i < 6; i++)
		{
			client_entity_t* splash = ClientEntity_new(type, flags, origin, dir, 800);

			splash->radius = 2.0f;
			splash->r.model = &water_models[1]; // wfall sprite.
			splash->r.frame = 1;
			splash->r.flags = RF_TRANSLUCENT;
			splash->r.scale = flrand(0.1f, 0.2f);
			splash->d_alpha = -1.0f;
			splash->d_scale = -0.5f;

			splash->origin[0] = WATER_SPLASH_RADIUS * cosf(angle);
			splash->origin[1] = WATER_SPLASH_RADIUS * sinf(angle);
			splash->origin[2] = 0.0f;

			splash->velocity[0] = flrand(1.0f, 2.0f) * splash->origin[0];
			splash->velocity[1] = flrand(1.0f, 2.0f) * splash->origin[1];
			splash->velocity[2] = flrand(100.0f, 150.0f);

			VectorSet(splash->acceleration, 0.0f, 0.0f, -525.0f);

			AddEffect(NULL, splash);

			angle += delta_angle;
		}

		// Create a water entry ripple.
		client_entity_t* entry_ripple = ClientEntity_new(FX_WATER_ENTRYSPLASH, flags & (~CEF_OWNERS_ORIGIN), origin, dir, 1200);

		entry_ripple->r.model = &water_models[0]; // waterentryripple sprite.
		entry_ripple->r.flags = RF_FIXED | RF_TRANSLUCENT | RF_ALPHA_TEXTURE;
		entry_ripple->r.scale = WATER_SPLASH_SCALE * 2.0f;
		entry_ripple->d_scale = 2.0f;
		entry_ripple->alpha = 0.6f;
		entry_ripple->d_alpha = entry_ripple_thinker->d_alpha;

		AddEffect(NULL, entry_ripple);
	}
	else if (flags & CEF_FLAG7 && R_DETAIL >= DETAIL_HIGH && dir[2] >= 0.99f) // Horizontal splash.
	{
		// Create an extra, centered water entry ripple.
		client_entity_t* splash = ClientEntity_new(FX_WATER_ENTRYSPLASH, flags & (~CEF_OWNERS_ORIGIN), origin, dir, 1200);

		splash->r.model = &water_models[0]; // waterentryripple sprite.
		splash->r.flags |= RF_FIXED | RF_TRANSLUCENT | RF_ALPHA_TEXTURE;
		splash->r.scale = WATER_SPLASH_SCALE;
		splash->alpha = 0.6f;
		splash->d_scale = 1.0f;
		splash->d_alpha = entry_ripple_thinker->d_alpha;

		AddEffect(NULL, splash);

		// Add little splooshies.
		const int particles_count = splash_size / 12;

		for (int i = 0; i < particles_count; i++)
		{
			client_particle_t* part = ClientParticle_new(PART_32x32_WFALL, color_white, 750);

			part->scale = 4.0f;
			part->d_scale = -5.33f;
			part->d_alpha = -1.3f;

			VectorSet(part->origin, flrand(-2.0f, 2.0f), flrand(-2.0f, 2.0f), 0.0f);
			VectorScale(part->origin, 8.0f, part->velocity);
			part->velocity[2] = flrand(50.0f, 100.0f);
			part->acceleration[2] = -320.0f;

			AddParticleToList(splash, part);
		}
	}
	else
	{
		entry_ripple_thinker->flags &= ~CEF_FLAG7; // Remove splooshy flag.
	}
}

void FXWaterEntrySplash(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	byte splash_size;
	vec3_t dir;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_WATER_ENTRYSPLASH].formatString, &splash_size, dir);

	DoWaterEntrySplash(type, flags, origin, splash_size, dir);
}