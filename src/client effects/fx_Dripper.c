//
// fx_Dripper.c
//
// Copyright 1998 Raven Software
//

#include "Ambient effects.h"
#include "Client Effects.h"
#include "Vector.h"
#include "Random.h"
#include "Utilities.h"

#define	DRIP_MAX_DURATION	2.0f
#define DRIP_RADIUS			2.0f
#define DRIP_NUM_SPLASHES	8

static struct model_s* drip_models[3];

void PreCacheDripper(void)
{
	drip_models[0] = fxi.RegisterModel("sprites/fx/steamhalf.sp2");
	drip_models[1] = fxi.RegisterModel("sprites/fx/steam.sp2");
	drip_models[2] = fxi.RegisterModel("sprites/fx/waterdrop.sp2");
}

static qboolean DripThinkSolid(client_entity_t* drip, centity_t* owner)
{
	vec3_t origin;
	VectorCopy(drip->r.origin, origin);
	origin[2] = drip->SpawnData;

	client_entity_t* mist = ClientEntity_new(-1, 0, origin, NULL, 500);

	mist->r.model = &drip_models[0];
	mist->r.scale = 0.5f;
	mist->r.flags = RF_TRANSLUCENT;

	mist->alpha = 0.4f;
	mist->d_alpha = -0.8f;

	DoWaterSplash(mist, color_white, DRIP_NUM_SPLASHES);

	AddEffect(NULL, mist);

	fxi.S_StartSound(origin, -1, CHAN_AUTO, fxi.S_RegisterSound(va("ambient/soliddrop%i.wav", irand(1, 3))), 1.0f, ATTN_STATIC, 0.0f);

	//FIXME: Returning false here doesn't work.
	drip->Update = RemoveSelfAI;

	return true;
}

static qboolean DripThinkWater(client_entity_t* drip, centity_t* owner)
{
	vec3_t origin;
	VectorCopy(drip->r.origin, origin);
	origin[2] = drip->SpawnData;

	client_entity_t* mist = ClientEntity_new(-1, 0, origin, NULL, 500);

	mist->r.model = &drip_models[0];
	mist->r.scale = 0.5f;
	mist->r.flags = RF_TRANSLUCENT;

	mist->d_scale = -2.0f;
	mist->d_alpha = -8.0f;

	AddEffect(NULL, mist);

	DoWaterSplash(mist, color_white, DRIP_NUM_SPLASHES);
	FXWaterRipples(NULL, FX_WATER_RIPPLES, 0, drip->r.origin);

	fxi.S_StartSound(origin, -1, CHAN_AUTO, fxi.S_RegisterSound(va("ambient/waterdrop%i.wav", irand(1, 3))), 1.0f, ATTN_STATIC, 0.0f);

	//FIXME: Returning false here doesn't work.
	drip->Update = RemoveSelfAI;

	return true;
}

static qboolean DripThinkLava(client_entity_t* drip, centity_t* owner)
{
	vec3_t origin;
	VectorCopy(drip->r.origin, origin);
	origin[2] = drip->SpawnData;

	client_entity_t* mist = ClientEntity_new(-1, 0, origin, NULL, 500);

	mist->r.model = &drip_models[1];
	mist->r.scale = 0.5f;
	mist->r.flags = RF_TRANSLUCENT;

	mist->alpha = 0.4f;
	mist->d_alpha = -0.8f;

	mist->velocity[0] = flrand(-10.0f, 10.0f);
	mist->velocity[1] = flrand(-10.0f, 10.0f);
	mist->velocity[2] = flrand(20.0f, 30.0f);

	AddEffect(NULL, mist);

	fxi.S_StartSound(origin, -1, CHAN_AUTO, fxi.S_RegisterSound(va("ambient/lavadrop%i.wav", irand(1, 3))), 1.0f, ATTN_STATIC, 0.0f);

	//FIXME: Returning false here doesn't work.
	drip->Update = RemoveSelfAI;

	return true;
}

static qboolean DripperParticleSpawner(client_entity_t* spawner, centity_t* owner)
{
	// Refresh time so it gets updated a random amount.
	spawner->updateTime = irand(spawner->LifeTime / 2, spawner->LifeTime * 2);

	// Spawn a drip to fall.
	client_entity_t* drip = ClientEntity_new(-1, 0, spawner->r.origin, NULL, spawner->SpawnDelay);

	drip->r.model = &drip_models[2];
	drip->r.scale = 0.1f;
	drip->r.flags = RF_TRANSLUCENT | RF_ALPHA_TEXTURE;
	drip->r.frame = spawner->r.frame;

	drip->radius = 2.0f;
	drip->SpawnData = spawner->SpawnData;
	VectorCopy(spawner->acceleration, drip->acceleration);

	switch (spawner->SpawnInfo & (CONTENTS_SOLID | CONTENTS_WATER | CONTENTS_LAVA))
	{
		case CONTENTS_WATER:
			drip->Update = DripThinkWater;
			break;

		case CONTENTS_LAVA:
			drip->Update = DripThinkLava;
			break;

		default:
			drip->Update = DripThinkSolid;
			break;
	}

	AddEffect(NULL, drip);

	return true;
}

// Spawn a water drop spawner.
void FXDripper(centity_t* owner, const int type, int flags, vec3_t origin)
{
	byte drips_per_min;
	byte frame;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_DRIPPER].formatString, &drips_per_min, &frame);

	flags |= CEF_NO_DRAW | CEF_NOMOVE | CEF_VIEWSTATUSCHANGED;
	client_entity_t* dripper = ClientEntity_new(type, flags, origin, NULL, 1000);

	dripper->r.frame = frame;

	dripper->LifeTime = 60 * 1000 / drips_per_min;
	dripper->Update = DripperParticleSpawner;

	dripper->acceleration[2] = GetGravity();
	dripper->radius = DRIP_RADIUS;

	trace_t trace;
	dripper->SpawnDelay = GetFallTime(origin, 0, dripper->acceleration[2], DRIP_RADIUS, DRIP_MAX_DURATION, &trace);
	dripper->SpawnData = trace.endpos[2] + 4.0f;
	dripper->SpawnInfo = trace.contents;

	AddEffect(owner, dripper);
}