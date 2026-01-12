//
// fx_Dripper.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "fx_WaterSplash.h"
#include "g_playstats.h"
#include "Vector.h"
#include "Random.h"
#include "Utilities.h"

#define	DRIP_MAX_DURATION	2.0f
#define DRIP_RADIUS			2.0f
#define DRIP_NUM_SPLASHES	8

static struct model_s* drip_models[3];

enum DripSoundID_e //mxd
{
	SND_SOLIDDROP1,
	SND_SOLIDDROP2,
	SND_SOLIDDROP3,

	SND_WATERDROP1,
	SND_WATERDROP2,
	SND_WATERDROP3,

	SND_LAVADROP1,
	SND_LAVADROP2,
	SND_LAVADROP3,

	NUM_SOUNDS
};

static struct sfx_s* drip_sounds[NUM_SOUNDS]; //mxd
static const paletteRGBA_t drip_yellow = { .c = 0xff09daf0 }; //mxd

void PreCacheDripper(void)
{
	drip_models[0] = fxi.RegisterModel("sprites/fx/steamhalf.sp2");
	drip_models[1] = fxi.RegisterModel("sprites/fx/steam.sp2");
	drip_models[2] = fxi.RegisterModel("sprites/fx/waterdrop.sp2");
}

void PreCacheDripperSFX(void) //mxd
{
	drip_sounds[SND_SOLIDDROP1] = fxi.S_RegisterSound("ambient/soliddrop1.wav");
	drip_sounds[SND_SOLIDDROP2] = fxi.S_RegisterSound("ambient/soliddrop2.wav");
	drip_sounds[SND_SOLIDDROP3] = fxi.S_RegisterSound("ambient/soliddrop3.wav");

	drip_sounds[SND_WATERDROP1] = fxi.S_RegisterSound("ambient/waterdrop1.wav");
	drip_sounds[SND_WATERDROP2] = fxi.S_RegisterSound("ambient/waterdrop2.wav");
	drip_sounds[SND_WATERDROP3] = fxi.S_RegisterSound("ambient/waterdrop3.wav");

	drip_sounds[SND_LAVADROP1] = fxi.S_RegisterSound("ambient/lavadrop1.wav");
	drip_sounds[SND_LAVADROP2] = fxi.S_RegisterSound("ambient/lavadrop2.wav");
	drip_sounds[SND_LAVADROP3] = fxi.S_RegisterSound("ambient/lavadrop3.wav");
}

static qboolean DripSolidUpdate(client_entity_t* self, centity_t* owner)
{
	vec3_t origin = VEC3_INIT(self->r.origin);
	origin[2] = self->SpawnData;

	client_entity_t* mist = ClientEntity_new(-1, 0, origin, NULL, 500);

	mist->r.model = &drip_models[0];
	mist->r.scale = 0.5f;
	mist->r.flags = RF_TRANSLUCENT;
	mist->r.color = self->r.color; //mxd. Colorize by drip type.

	if (R_DETAIL >= DETAIL_HIGH) //mxd. +RF_LM_COLOR.
		mist->r.flags |= RF_LM_COLOR;

	mist->alpha = 0.4f;
	mist->d_alpha = -0.8f;

	DoWaterSplash(mist, self->r.color, DRIP_NUM_SPLASHES, true); //mxd. Colorize by drip type.

	AddEffect(NULL, mist);

	fxi.S_StartSound(origin, -1, CHAN_AUTO, drip_sounds[irand(SND_SOLIDDROP1, SND_SOLIDDROP3)], 1.0f, ATTN_STATIC, 0.0f);

	//mxd. Original logic sets self->Update to RemoveSelfAI and returns true here (which results in waterdrop lingering for another updateTime).
	return false;
}

static qboolean DripWaterUpdate(client_entity_t* self, centity_t* owner)
{
	vec3_t origin = VEC3_INIT(self->r.origin);
	origin[2] = self->SpawnData;

	client_entity_t* mist = ClientEntity_new(-1, 0, origin, NULL, 500);

	mist->r.model = &drip_models[0];
	mist->r.scale = 0.5f;
	mist->r.flags = RF_TRANSLUCENT;
	mist->r.color = self->r.color; //mxd. Colorize by drip type.

	mist->d_scale = -2.0f;
	mist->d_alpha = -8.0f;

	AddEffect(NULL, mist);

	DoWaterSplash(mist, self->r.color, DRIP_NUM_SPLASHES, false); //mxd. Colorize by drip type.
	FXWaterRipples(NULL, FX_WATER_RIPPLES, 0, origin); //mxd. Original logic uses self->r.origin (which may already be below water surface).

	fxi.S_StartSound(origin, -1, CHAN_AUTO, drip_sounds[irand(SND_WATERDROP1, SND_WATERDROP3)], 1.0f, ATTN_STATIC, 0.0f);

	//mxd. Original logic sets self->Update to RemoveSelfAI and returns true here (which results in waterdrop lingering for another updateTime).
	return false;
}

static qboolean DripLavaUpdate(client_entity_t* self, centity_t* owner)
{
	vec3_t origin = VEC3_INIT(self->r.origin);
	origin[2] = self->SpawnData;

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

	fxi.S_StartSound(origin, -1, CHAN_AUTO, drip_sounds[irand(SND_LAVADROP1, SND_LAVADROP3)], 1.0f, ATTN_STATIC, 0.0f);

	//mxd. Original logic sets self->Update to RemoveSelfAI and returns true here (which results in waterdrop lingering for another updateTime).
	return false;
}

static qboolean DripperUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXDripperParticleSpawner' in original logic.
{
	// Refresh time so it gets updated a random amount.
	self->updateTime = irand(self->LifeTime / 2, self->LifeTime * 2);

	// Spawn a drip to fall.
	client_entity_t* drip = ClientEntity_new(-1, 0, self->r.origin, NULL, self->SpawnDelay);

	drip->r.model = &drip_models[2];
	drip->r.scale = 0.1f;
	drip->r.flags = RF_TRANSLUCENT; //mxd. Original logic also adds RF_ALPHA_TEXTURE flag, which is used only in conjunction with RF_TRANS_ADD flag in ref_gl1...
	drip->r.color = self->r.color; //mxd. Colorize by drip type.

	if (R_DETAIL >= DETAIL_HIGH) //mxd. +RF_LM_COLOR.
		drip->r.flags |= RF_LM_COLOR;

	//mxd. Add subtle fade-in effect.
	drip->alpha = 0.1f;
	drip->d_alpha = 6.0f;

	drip->radius = 2.0f;
	drip->SpawnData = self->SpawnData;
	VectorCopy(self->acceleration, drip->acceleration);

	switch (self->SpawnInfo & (CONTENTS_SOLID | CONTENTS_WATER | CONTENTS_LAVA))
	{
		case CONTENTS_WATER:
			drip->Update = DripWaterUpdate;
			break;

		case CONTENTS_LAVA:
			drip->Update = DripLavaUpdate;
			break;

		default:
			drip->Update = DripSolidUpdate;
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

	flags |= (CEF_NO_DRAW | CEF_NOMOVE | CEF_VIEWSTATUSCHANGED);
	client_entity_t* spawner = ClientEntity_new(type, flags, origin, NULL, 1000);

	spawner->r.color = (frame == 1 ? drip_yellow : color_white); //mxd. Original logic sets spawner->r.frame instead (but waterdrop.sp2 has single frame).

	spawner->LifeTime = 60 * 1000 / drips_per_min;
	spawner->Update = DripperUpdate;

	spawner->acceleration[2] = GetGravity();
	spawner->radius = DRIP_RADIUS;

	trace_t trace;
	spawner->SpawnDelay = GetFallTime(origin, 0.0f, spawner->acceleration[2], DRIP_RADIUS, DRIP_MAX_DURATION, &trace);
	spawner->SpawnData = trace.endpos[2] + 4.0f;
	spawner->SpawnInfo = trace.contents;

	AddEffect(owner, spawner);
}