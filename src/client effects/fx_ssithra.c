//
// fx_ssithra.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Random.h"
#include "Utilities.h"
#include "Vector.h"
#include "ce_DLight.h"
#include "fx_debris.h"

#define SSARROW_TRAIL_SCALE		0.25f

static struct model_s* arrow_models[3];

void PreCacheSsithraArrow(void) //mxd. Named 'PrecacheSsithraArrow' in original logic.
{
	arrow_models[0] = fxi.RegisterModel("sprites/fx/steam.sp2"); // Unpowered trail.
	arrow_models[1] = fxi.RegisterModel("sprites/fx/fire.sp2"); // Powered trail.
	arrow_models[2] = fxi.RegisterModel("models/objects/projectiles/sitharrow/tris.fm"); // Projectile model.
}

static qboolean SsithraArrowTrailUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXSsithraArrowTrailThink' in original logic.
{
	self->updateTime = 20;
	self->r.angles[ROLL] += 10.0f;

	if (self->SpawnInfo > 9)
		self->SpawnInfo--;

	vec3_t trail_vel;
	VectorNormalize2(self->velocity, trail_vel);

	const int count = GetScaledCount(irand(self->SpawnInfo >> 3, self->SpawnInfo >> 2), 0.8f);

	for (int i = 0; i < count; i++)
	{
		client_entity_t* trail = ClientEntity_new(FX_SSITHRA_ARROW, 0, self->r.origin, NULL, 1000);

		if (self->flags & CEF_FLAG7)
		{
			// Powered.
			trail->r.model = &arrow_models[1]; // fire sprite.
			trail->r.flags = (RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
			trail->r.scale = SSARROW_TRAIL_SCALE + flrand(0.0f, 0.05f);

			VectorRandomCopy(self->r.origin, trail->r.origin, flrand(-8.0f, 8.0f));
			VectorScale(trail_vel, flrand(-400.0f, -100.0f), trail->velocity);
		}
		else
		{
			// Make this use tinting instead of darken?
			trail->r.model = &arrow_models[0]; // steam sprite.
			trail->r.flags = RF_TRANSLUCENT; // Darken.
			trail->r.scale = SSARROW_TRAIL_SCALE + flrand(-0.2f, 0.2f);
			COLOUR_SETA(trail->r.color, 75, 50, 100, 100); //mxd. Use macro.

			VectorRandomCopy(self->r.origin, trail->r.origin, flrand(-5.0f, 5.0f));
			VectorScale(trail_vel, flrand(-400.0f, -50.0f), trail->velocity);
		}

		trail->d_alpha = flrand(-1.5f, -2.0f);
		trail->d_scale = flrand(-1.0f, -1.25f);
		trail->updateTime = (int)(trail->alpha * 1000.0f / -trail->d_scale);
		trail->radius = 20.0f;

		AddEffect(NULL, trail);
	}

	return true;
}

static void SpawnSsithraArrow(centity_t* owner, const int type, const int flags, const vec3_t origin, const vec3_t velocity) //mxd. Named 'FXDoSsithraArrow' in original logic.
{
	client_entity_t* missile = ClientEntity_new(type, flags | CEF_DONT_LINK, origin, NULL, 100);

	missile->radius = 128.0f;
	missile->r.model = &arrow_models[2]; // sitharrow model.
	missile->r.flags = RF_GLOW;

	VectorCopy(velocity, missile->velocity);

	vec3_t dir;
	VectorNormalize2(velocity, dir);
	AnglesFromDir(dir, missile->r.angles);

	missile->dlight = CE_DLight_new(color_orange, 120.0f, 0.0f);
	missile->SpawnInfo = 32;
	missile->Update = SsithraArrowTrailUpdate;

	AddEffect(owner, missile);
}

static void SpawnSsithraArrow2(centity_t* owner, const int type, const int flags, const vec3_t origin, const vec3_t velocity) //mxd. Named 'FXDoSsithraArrow2' in original logic.
{
	client_entity_t* missile = ClientEntity_new(type, flags | CEF_DONT_LINK, origin, NULL, 100);

	missile->radius = 128.0f;
	missile->flags |= CEF_FLAG7;
	missile->r.model = &arrow_models[2]; // sitharrow model.
	missile->r.flags = RF_GLOW;
	missile->r.scale = 1.5f;

	VectorCopy(velocity, missile->velocity);

	vec3_t dir;
	VectorNormalize2(velocity, dir);
	AnglesFromDir(dir, missile->r.angles);

	missile->dlight = CE_DLight_new(color_red, 160.0f, 0.0f);
	missile->SpawnInfo = 32;
	missile->Update = SsithraArrowTrailUpdate;

	AddEffect(owner, missile);
}

static void SpawnSsithraArrowExplosion(centity_t* owner, const int type, const int flags, const vec3_t origin, vec3_t direction) //mxd. Named 'FXSsithraArrowBoom' in original logic.
{
	Vec3ScaleAssign(32.0f, direction);

	const int count = GetScaledCount(irand(8, 12), 0.8f);

	for (int i = 0; i < count; i++)
	{
		const qboolean is_last_puff = (i == count - 1); //mxd
		const int next_think_time = (is_last_puff ? 500 : 1000); //mxd

		client_entity_t* smoke_puff = ClientEntity_new(type, flags, origin, NULL, next_think_time);

		smoke_puff->radius = 20.0f;
		smoke_puff->r.model = &arrow_models[0]; // steam sprite.
		smoke_puff->r.flags = RF_TRANSLUCENT; // Make this use tinting instead of darken?
		smoke_puff->r.scale = flrand(0.8f, 1.6f);
		smoke_puff->d_scale = -2.0f;
		smoke_puff->d_alpha = -0.4f;
		COLOUR_SETA(smoke_puff->r.color, 75, 50, 100, 100); //mxd. Use macro.

		VectorSet(smoke_puff->acceleration, flrand(-200.0f, 200.0f), flrand(-200.0f, 200.0f), flrand(-60.0f, -40.0f));

		if (is_last_puff)
		{
			fxi.S_StartSound(smoke_puff->r.origin, -1, CHAN_WEAPON, fxi.S_RegisterSound("weapons/SsithraArrowImpact.wav"), 1.0f, ATTN_NORM, 0.0f);
			smoke_puff->dlight = CE_DLight_new(color_orange, 150.0f, 0.0f);
		}
		else
		{
			VectorRandomCopy(direction, smoke_puff->velocity, 64.0f);
		}

		AddEffect(NULL, smoke_puff);
	}
}

static void SpawnSsithraArrow2Explosion(centity_t* owner, const int type, const int flags, const vec3_t origin, vec3_t direction) //mxd. Named 'FXSsithraArrow2Boom' in original logic.
{
	Vec3ScaleAssign(32.0f, direction);

	const int count = GetScaledCount(irand(12, 16), 0.8f);

	for (int i = 0; i < count; i++)
	{
		const qboolean is_last_puff = (i == count - 1); //mxd
		const int next_think_time = (is_last_puff ? 500 : 1000); //mxd

		client_entity_t* smoke_puff = ClientEntity_new(type, flags, origin, NULL, next_think_time);

		smoke_puff->radius = 20.0f;
		smoke_puff->r.model = &arrow_models[1]; // fire sprite.
		smoke_puff->r.flags = (RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
		smoke_puff->r.scale = flrand(1.2f, 2.0f);
		smoke_puff->d_scale = -2.0f;
		smoke_puff->d_alpha = -0.4f;
		smoke_puff->acceleration[2] = -400.0f;

		if (is_last_puff)
		{
			fxi.S_StartSound(smoke_puff->r.origin, -1, CHAN_WEAPON, fxi.S_RegisterSound("weapons/FireballPowerImpact.wav"), 1.0f, ATTN_NORM, 0.0f);
			smoke_puff->dlight = CE_DLight_new(color_red, 200.0f, 0.0f);
		}
		else
		{
			VectorRandomCopy(direction, smoke_puff->velocity, 200.0f);
			smoke_puff->velocity[2] += 100.0f;
		}

		AddEffect(NULL, smoke_puff);
	}

	// Clear out cef_flag# stuff, means different stuff to debris.
	const vec3_t mins = { 2.0f, 2.0f, 2.0f }; // Because SpawnChunks needs a value for bounding box.
	FXDebris_SpawnChunks(type, flags & ~(CEF_FLAG6 | CEF_FLAG7 | CEF_FLAG8), origin, 5, MAT_GREYSTONE, vec3_up, 80000.0f, mins, 1.0f, false); //TODO: pass material based on surface hit?
}

void FXSsithraArrow(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	byte effect = 0;
	vec3_t vel;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_SSITHRA_ARROW].formatString, &effect, vel);

	switch (effect)
	{
		case FX_SS_MAKE_ARROW:
			SpawnSsithraArrow(owner, type, flags, origin, vel);
			break;

		case FX_SS_MAKE_ARROW2:
			SpawnSsithraArrow2(owner, type, flags, origin, vel);
			break;

		case FX_SS_EXPLODE_ARROW:
			SpawnSsithraArrowExplosion(owner, type, flags, origin, vel);
			break;

		case FX_SS_EXPLODE_ARROW2:
			SpawnSsithraArrow2Explosion(owner, type, flags, origin, vel);
			break;

		default:
			Com_DPrintf("Unknown effect type (%d) for FXSsithraArrow\n", effect); //mxd. Changed from Com_Printf().
			break;
	}
}