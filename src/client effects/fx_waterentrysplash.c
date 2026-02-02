//
// fx_WaterEntrySplash.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Particle.h"
#include "Vector.h"
#include "Random.h"
#include "Utilities.h"
#include "g_playstats.h"

#define WATER_SPLASH_SCALE	0.3f
#define WATER_SPLASH_RADIUS	20.0f
#define MAX_WATER_RIPPLES	4 //mxd

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

static qboolean WaterEntryRippleSpawnerUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXWaterEntrySplashThinkerThink' in original logic.
{
	// Have enough ripples been created yet? If not, create one, else free myself.
	if (self->NoOfAnimFrames < 1)
		return false;

	// Create a water entry ripple.
	const int flags = (int)(self->flags & ~(CEF_OWNERS_ORIGIN | CEF_NO_DRAW)); //mxd
	
	//mxd. Big random ripples.
	if (self->flags & CEF_FLAG8)
	{
		const int num_ripples = irand(1, 3);

		float angle = flrand(ANGLE_0, ANGLE_360);
		const float delta_angle = ANGLE_360 / (float)num_ripples;
		const float pos_scaler = 0.25f + (float)(MAX_WATER_RIPPLES - self->NoOfAnimFrames) * 0.25f; // [0.25 .. 1.0]

		for (int i = 0; i < num_ripples; i++) //mxd. Rewritten to take water normal into account.
		{
			client_entity_t* ripple = ClientEntity_new(FX_WATER_ENTRYSPLASH, flags, self->origin, self->direction, 1200);

			ripple->r.model = &water_models[0]; // waterentryripple sprite.
			ripple->r.flags = (RF_FIXED | RF_TRANSLUCENT | RF_ALPHA_TEXTURE);
			ripple->alpha = 0.6f;
			ripple->d_alpha = self->d_alpha * (1.0f + pos_scaler);

			ripple->r.scale = flrand(0.2f, 0.3f) * (1.0f - pos_scaler + 0.25f);
			ripple->d_scale = 0.7f;

			const float step_angle = angle + flrand(-delta_angle * 0.35f, delta_angle * 0.35f);

			ripple->r.origin[0] += sinf(step_angle) * flrand(32.0f, 48.0f) * pos_scaler;
			ripple->r.origin[1] += cosf(step_angle) * flrand(32.0f, 48.0f) * pos_scaler;

			AddEffect(NULL, ripple);

			angle += delta_angle;
		}
	}
	else
	{
		client_entity_t* ripple = ClientEntity_new(FX_WATER_ENTRYSPLASH, flags, self->origin, self->direction, 1200);

		ripple->r.model = &water_models[0]; // waterentryripple sprite.
		ripple->r.flags = (RF_FIXED | RF_TRANSLUCENT | RF_ALPHA_TEXTURE);
		ripple->alpha = 0.6f;
		ripple->d_alpha = self->d_alpha;

		// Random ripple.
		if (self->flags & CEF_FLAG7)
		{
			ripple->r.scale = 0.1f;
			ripple->d_scale = 0.7f;
			ripple->r.origin[0] += flrand(-6.0f, 6.0f);
			ripple->r.origin[1] += flrand(-6.0f, 6.0f);
		}
		else // Centered ripple.
		{
			ripple->r.scale = WATER_SPLASH_SCALE;
			ripple->d_scale = 1.0f;
		}

		AddEffect(NULL, ripple);
	}

	self->NoOfAnimFrames--;

	return true;
}

void DoWaterEntrySplash(const int type, const int flags, const vec3_t origin, const byte splash_size, const vec3_t dir)
{
	int ripples_count;

	if (splash_size >= 128)
		ripples_count = MAX_WATER_RIPPLES;
	else if (splash_size >= 96)
		ripples_count = MAX_WATER_RIPPLES - 1;
	else if (splash_size >= 64)
		ripples_count = MAX_WATER_RIPPLES - 2;
	else
		ripples_count = MAX_WATER_RIPPLES - 3;

	// They want a sound too?
	if (flags & CEF_FLAG6)
		fxi.S_StartSound(origin, -1, CHAN_AUTO, water_sounds[irand(0, 1)], 1.0f, ATTN_STATIC, 0.0f);

	const qboolean big_splash = (splash_size & 128); //mxd
	const qboolean spawn_ripples = (dir[2] >= 0.99f); //mxd. Spawn ripples only on horizontal surfaces.
	const float ripple_delta_alpha = -0.8f - (0.2f - (0.2f * (1.0f / 127.0f * (float)(splash_size & 127)))); //mxd
	client_entity_t* entry_ripple_spawner = NULL; //mxd

	// Create a water entry ripple THINKER that will create the actual water entry ripples.
	if (spawn_ripples) //mxd
	{
		const int update_time = (big_splash ? 120 : 200); //mxd
		entry_ripple_spawner = ClientEntity_new(type, flags, origin, dir, update_time);

		entry_ripple_spawner->flags |= CEF_NO_DRAW;

		if (big_splash) //mxd. Big ripples flag.
			entry_ripple_spawner->flags |= CEF_FLAG8;

		entry_ripple_spawner->NoOfAnimFrames = ripples_count;
		entry_ripple_spawner->d_alpha = ripple_delta_alpha;
		entry_ripple_spawner->Update = WaterEntryRippleSpawnerUpdate;

		AddEffect(NULL, entry_ripple_spawner);
	}

	//mxd. Setup direction vectors.
	vec3_t up;
	PerpendicularVector(up, dir);

	vec3_t right;
	CrossProduct(up, dir, right);

	// Big ripple and water entry splash.
	if (big_splash)
	{
		// Sideways splashes.
		int num_splashes = irand(12, 14); //mxd. Randomize count a bit.
		float angle = flrand(ANGLE_0, ANGLE_360); //mxd. Randomize.
		float delta_angle = ANGLE_360 / (float)num_splashes;
		
		for (int i = 0; i < num_splashes; i++) //mxd. Rewritten to take water normal into account.
		{
			client_entity_t* splash = ClientEntity_new(type, flags, origin, NULL, 500);

			splash->radius = 2.0f;
			splash->r.model = &water_models[1]; // wfall sprite.
			splash->r.frame = 1;
			splash->r.scale = flrand(0.15f, 0.25f);
			splash->r.flags = RF_TRANSLUCENT;
			splash->d_alpha = flrand(-2.2f, -1.8f); //mxd. Randomize a bit.
			splash->d_scale = flrand(1.0f, 1.2f); //mxd. Randomize a bit.

			const float step_angle = angle + flrand(-delta_angle * 0.35f, delta_angle * 0.35f);

			vec3_t vel_dir;
			const float vel_scaler = WATER_SPLASH_RADIUS * flrand(0.8f, 1.1f);
			VectorScale(up, sinf(step_angle) * vel_scaler, vel_dir);
			VectorMA(vel_dir, cosf(step_angle) * vel_scaler, right, vel_dir);

			for (int c = 0; c < 3; c++)
				splash->velocity[c] = flrand(4.0f, 6.0f) * vel_dir[c];

			VectorMA(splash->velocity, flrand(60.0f, 80.0f), dir, splash->velocity); // H2: flrand(30.0f, 40.0f).

			splash->acceleration[2] = flrand(-250.0f, -200.0f); // H2: -250.0f

			RE_SetupRollSprite(&splash->r, 64.0f, flrand(0.0f, 359.0f)); //mxd
			AddEffect(NULL, splash);

			angle += delta_angle;
		}

		// Vertical splashes.
		num_splashes = irand(6, 8); //mxd. Randomize count a bit.
		angle = flrand(ANGLE_0, ANGLE_360); //mxd. Randomize.
		delta_angle = ANGLE_360 / (float)num_splashes;

		for (int i = 0; i < num_splashes; i++) //mxd. Rewritten to take water normal into account.
		{
			client_entity_t* splash = ClientEntity_new(type, flags, origin, NULL, 800);

			splash->radius = 2.0f;
			splash->r.model = &water_models[1]; // wfall sprite. //mxd. Original logic uses r.frame:1 for these.
			splash->r.flags = RF_TRANSLUCENT;
			splash->r.scale = flrand(0.3f, 0.5f); // H2: flrand(0.1f, 0.2f).
			splash->d_alpha = flrand(-1.0f, -0.8f); //mxd. Randomize a bit.
			splash->d_scale = flrand(-0.5f, -0.2f); //mxd. Randomize a bit.

			const float step_angle = angle + flrand(-delta_angle * 0.35f, delta_angle * 0.35f);

			vec3_t vel_dir;
			const float vel_scaler = WATER_SPLASH_RADIUS * flrand(0.1f, 0.5f);
			VectorScale(up, sinf(step_angle) * vel_scaler, vel_dir);
			VectorMA(vel_dir, cosf(step_angle) * vel_scaler, right, vel_dir);
			Vec3AddAssign(vel_dir, splash->r.origin);

			for (int c = 0; c < 3; c++)
				splash->velocity[c] = flrand(2.0f, 4.0f) * vel_dir[c];

			VectorMA(splash->velocity, flrand(160.0f, 200.0f), dir, splash->velocity); // H2: flrand(100.0f, 150.0f).

			splash->acceleration[2] = flrand(-525.0f, -425.0f); // H2: -525.0f

			RE_SetupRollSprite(&splash->r, 64.0f, flrand(0.0f, 359.0f)); //mxd
			AddEffect(NULL, splash);

			angle += delta_angle;
		}

		// Big central water entry ripple.
		if (spawn_ripples)
		{
			client_entity_t* entry_ripple = ClientEntity_new(FX_WATER_ENTRYSPLASH, flags & (~CEF_OWNERS_ORIGIN), origin, dir, 1200);

			entry_ripple->r.model = &water_models[0]; // waterentryripple sprite.
			entry_ripple->r.flags = (RF_FIXED | RF_TRANSLUCENT | RF_ALPHA_TEXTURE);
			entry_ripple->r.scale = WATER_SPLASH_SCALE * 2.0f;
			entry_ripple->d_scale = 2.0f;
			entry_ripple->alpha = 0.6f;
			entry_ripple->d_alpha = ripple_delta_alpha;

			AddEffect(NULL, entry_ripple);
		}
	}
	else if ((flags & CEF_FLAG7) && R_DETAIL >= DETAIL_HIGH) // Small ripple and particle splash.
	{
		const int duration = (spawn_ripples ? 1200 : 750); //mxd
		client_entity_t* ripple = ClientEntity_new(FX_WATER_ENTRYSPLASH, flags & (~CEF_OWNERS_ORIGIN), origin, dir, duration);

		// Create an extra, centered water entry ripple.
		if (spawn_ripples)
		{
			ripple->r.model = &water_models[0]; // waterentryripple sprite.
			ripple->r.flags |= (RF_FIXED | RF_TRANSLUCENT | RF_ALPHA_TEXTURE);
			ripple->r.scale = WATER_SPLASH_SCALE;
			ripple->alpha = 0.6f;
			ripple->d_scale = 1.0f;
			ripple->d_alpha = ripple_delta_alpha;
		}
		else //mxd. Setup as invisible entity to attach particles to.
		{
			ripple->flags |= CEF_NO_DRAW;
		}

		AddEffect(NULL, ripple);

		// Add little splooshies.
		const int particles_count = splash_size / 12;
		float angle = flrand(ANGLE_0, ANGLE_360); //mxd
		const float delta_angle = ANGLE_360 / (float)particles_count; //mxd

		for (int i = 0; i < particles_count; i++) //mxd. Rewritten to take water normal into account.
		{
			client_particle_t* part = ClientParticle_new(PART_32x32_WFALL, color_white, 750);

			part->scale = flrand(4.0f, 6.0f); //mxd. Randomize a bit.
			part->d_scale = flrand(-5.33f, -4.7f); //mxd. Randomize a bit.
			part->d_alpha = flrand(-164.0f, -128.0f); //mxd. Randomize a bit.

			const float step_angle = angle + flrand(-delta_angle * 0.35f, delta_angle * 0.35f);

			const float pos_scaler = flrand(0.1f, 2.0f);
			VectorScale(up, sinf(step_angle) * pos_scaler, part->origin);
			VectorMA(part->origin, cosf(step_angle) * pos_scaler, right, part->origin);

			VectorScale(part->origin, 16.0f, part->velocity);
			VectorMA(part->velocity, flrand(60.0f, 120.0f), dir, part->velocity); // H2: flrand(50.0f, 100.0f)
			part->acceleration[2] = flrand(-320.0f, -280.0f); // H2: -320.0f

			AddParticleToList(ripple, part);

			angle += delta_angle;
		}
	}
	else if (entry_ripple_spawner != NULL)
	{
		entry_ripple_spawner->flags &= ~CEF_FLAG7; // Don't randomize subsequent ripples position.
	}
}

void FXWaterEntrySplash(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	byte splash_size;
	vec3_t dir;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_WATER_ENTRYSPLASH].formatString, &splash_size, dir);

	DoWaterEntrySplash(type, flags, origin, splash_size, dir);
}