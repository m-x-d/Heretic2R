//
// fx_flyingfist.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Vector.h"
#include "ce_DLight.h"
#include "Random.h"
#include "Utilities.h"
#include "fx_debris.h"
#include "g_playstats.h"

#define FIST_SCALE				0.25f
#define FIST_WIMPY_SCALE		0.15f
#define FIST_BLAST_VEL			64.0f
#define FIST_POWER_BLAST_VEL	200.0f

static struct model_s* fist_models[3];
static struct sfx_s* fist_impact_sounds[2]; //mxd

void PreCacheFist(void)
{
	fist_models[0] = fxi.RegisterModel("Sprites/Spells/flyingfist.sp2");
	fist_models[1] = fxi.RegisterModel("Sprites/Spells/spellhands_red.sp2");
	fist_models[2] = fxi.RegisterModel("models/spells/meteorbarrier/tris.fm");
}

void PreCacheFistSFX(void) //mxd
{
	fist_impact_sounds[0] = fxi.S_RegisterSound("weapons/FlyingFistImpact.wav");
	fist_impact_sounds[1] = fxi.S_RegisterSound("weapons/FireballPowerImpact.wav");

	//mxd. Precache weapon sounds as well. Not the best place to do this, but since weapons have no precache logic at all...
	fxi.S_RegisterSound("weapons/FlyingFistCast.wav");
	fxi.S_RegisterSound("weapons/FireballPowerCast.wav");
	fxi.S_RegisterSound("weapons/FireballNoMana.wav");
}

static qboolean FlyingFistTrailUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXFlyingFistTrailThink' in original logic.
{
	self->updateTime = 20;

	if (self->SpawnInfo > 9)
		self->SpawnInfo--;

	qboolean is_wimpy = false;
	float trailscale = FIST_SCALE;
	int count = GetScaledCount(irand(self->SpawnInfo >> 3, self->SpawnInfo >> 2), 0.8f);

	if (self->flags & CEF_FLAG8)
	{
		is_wimpy = true;
		trailscale = FIST_WIMPY_SCALE;
		count /= 2;
	}

	for (int i = 0; i < count; i++)
	{
		client_entity_t* trail_ent = ClientEntity_new(FX_WEAPON_FLYINGFIST, 0, self->r.origin, NULL, 1000);
		trail_ent->r.flags = (RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);

		vec3_t accel_dir;
		VectorNormalize2(self->velocity, accel_dir);

		if (self->flags & CEF_FLAG7)
		{
			trail_ent->r.model = &fist_models[1];
			trail_ent->r.scale = 3.0f * (trailscale + flrand(0.0f, 0.05f));
			VectorRandomCopy(self->r.origin, trail_ent->r.origin, flrand(-8.0f, 8.0f));
			VectorScale(accel_dir, flrand(-100.0f, -400.0f), trail_ent->velocity);
		}
		else
		{
			trail_ent->r.model = &fist_models[0];
			trail_ent->r.scale = trailscale + flrand(0.0f, 0.05f);
			VectorRandomCopy(self->r.origin, trail_ent->r.origin, flrand(-5.0f, 5.0f));
			VectorScale(accel_dir, flrand(-50.0f, -400.0f), trail_ent->velocity);
		}

		if (is_wimpy) // Wimpy shot, because no mana.
			Vec3ScaleAssign(0.5f, trail_ent->velocity);

		trail_ent->d_alpha = flrand(-1.5f, -2.0f);
		trail_ent->d_scale = flrand(-1.0f, -1.25f);
		trail_ent->updateTime = (int)(trail_ent->alpha * 1000.0f / -trail_ent->d_scale);
		trail_ent->radius = 20.0f;

		AddEffect(NULL, trail_ent);
	}

	return true;
}

void FXFlyingFist(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	vec3_t vel;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_WEAPON_FLYINGFIST].formatString, vel);

	const float vel_scaler = FLYING_FIST_SPEED * (flags & CEF_FLAG6 ? 0.5f : 1.0f);
	Vec3ScaleAssign(vel_scaler, vel);

	client_entity_t* missile = ClientEntity_new(type, flags | CEF_DONT_LINK, origin, NULL, 100);

	paletteRGBA_t light_color;
	float light_size;

	if (flags & CEF_FLAG7)
	{
		// Powered up fireball. Use a meteor model.
		missile->r.model = &fist_models[2];
		missile->r.skinnum = 1;
		missile->r.scale = ((flags & CEF_FLAG8) ? 1.0f : 1.5f); // Wimpy shot?

		light_color = color_red; //mxd. Red light
		light_size = 160.0f;
	}
	else
	{
		// Just a normal fireball.
		missile->flags |= CEF_NO_DRAW;
		light_color = color_orange; //mxd. Orange light
		light_size = 120.0f;
	}

	VectorCopy(vel, missile->velocity);
	VectorNormalize(vel);
	AnglesFromDir(vel, missile->r.angles);

	missile->radius = 128.0f;
	missile->dlight = CE_DLight_new(light_color, light_size, 0.0f);
	missile->Update = FlyingFistTrailUpdate;

	missile->SpawnInfo = 32;

	AddEffect(owner, missile);
}

void FXFlyingFistExplode(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	vec3_t dir;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_WEAPON_FLYINGFISTEXPLODE].formatString, dir);

	if (flags & CEF_FLAG6)
		FXClientScorchmark(origin, dir);

	const qboolean is_powered = (flags & CEF_FLAG7);
	const qboolean is_wimpy = (flags & CEF_FLAG8);
	const float volume = (is_wimpy ? 0.75f : 1.0f);

	Vec3ScaleAssign(32.0f, dir);

	int count;
	paletteRGBA_t light_color;
	float light_radius;

	if (is_powered)
	{
		count = GetScaledCount(irand(12, 16), 0.8f);
		light_color = color_red; //mxd
		light_radius = (is_wimpy ? 160.0f : 200.0f);
	}
	else
	{
		count = GetScaledCount(irand(8, 12), 0.8f);
		light_color = color_orange; //mxd
		light_radius = (is_wimpy ? 120.0f : 150.0f);
	}

	for (int i = 0; i < count; i++)
	{
		const qboolean is_last_puff = (i == count - 1); //mxd
		client_entity_t* smoke_puff = ClientEntity_new(type, flags, origin, NULL, (is_last_puff ? 500 : 1000));

		smoke_puff->r.model = &fist_models[1];
		smoke_puff->d_scale = -2.0f;

		if (is_powered)
		{
			// Meteor impact!
			float blast_vel = FIST_POWER_BLAST_VEL;

			if (is_wimpy)
			{
				blast_vel *= 0.3f;
				smoke_puff->r.scale = flrand(0.8f, 1.4f);
			}
			else
			{
				smoke_puff->r.scale = flrand(1.2f, 2.0f);
			}

			VectorRandomCopy(dir, smoke_puff->velocity, blast_vel);
			smoke_puff->velocity[2] += 100.0f;
			smoke_puff->acceleration[2] = -400.0f;
		}
		else
		{
			// Non-powered up.
			float blast_vel = FIST_BLAST_VEL;

			if (is_wimpy)
			{
				blast_vel *= 0.5f;
				smoke_puff->r.scale = flrand(0.5f, 1.0f);
			}
			else
			{
				smoke_puff->r.scale = flrand(0.8f, 1.6f);
			}

			VectorRandomCopy(dir, smoke_puff->velocity, blast_vel);
			smoke_puff->acceleration[0] = flrand(-200.0f, 200.0f);
			smoke_puff->acceleration[1] = flrand(-200.0f, 200.0f);
			smoke_puff->acceleration[2] = flrand(-40.0f, -60.0f);
		}

		smoke_puff->r.flags = (RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
		smoke_puff->r.frame = 0;
		smoke_puff->d_alpha = -0.4f;
		smoke_puff->radius = 20.0f;

		if (is_last_puff)
		{
			fxi.S_StartSound(smoke_puff->r.origin, -1, CHAN_WEAPON, fist_impact_sounds[is_powered ? 1 : 0], volume, ATTN_NORM, 0.0f);

			smoke_puff->dlight = CE_DLight_new(light_color, light_radius, -50.0f);
			VectorClear(smoke_puff->velocity);
		}

		AddEffect(NULL, smoke_puff);
	}

	if (is_powered)
	{
		// Meteor throws out chunks.
		const vec3_t mins = { 2.0f, 2.0f, 2.0f }; // Because SpawnChunks needs a value for bounding box.

		// No mana meteors are wimpy! - clear out cef_flag# stuff, means different stuff to debris.
		const float chunk_scale = (is_wimpy ? 0.5f : 1.0f); //mxd
		FXDebris_SpawnChunks(type, flags & ~(CEF_FLAG6 | CEF_FLAG7 | CEF_FLAG8), origin, 5, MAT_GREYSTONE, vec3_up, 80000.0f, mins, chunk_scale, false);
	}
}