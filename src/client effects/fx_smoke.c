//
// fx_smoke.c
//
// Copyright 1998 Raven Software
//

#include "fx_smoke.h" //mxd
#include "Client Effects.h"
#include "Vector.h"
#include "Random.h"
#include "Motion.h"
#include "Utilities.h"

static struct model_s* smoke_model;

void PreCacheSmoke(void)
{
	smoke_model = fxi.RegisterModel("sprites/fx/steam.sp2");
}

//mxd. Added to reduce code duplication.
static void SpawnSmoke(const vec3_t origin, const float scale, const float range, const uint color)
{
	const int duration = (int)(GetTimeToReachDistance(50.0f, 0.0f, range));
	client_entity_t* smoke = ClientEntity_new(-1, RF_TRANSLUCENT, origin, NULL, 500);

	smoke->r.model = &smoke_model;
	smoke->r.scale = scale;
	smoke->r.color.c = color;
	VectorSet(smoke->velocity, flrand(-10.0f, 10.0f), flrand(-10.0f, 10.0f), 50.0f);
	smoke->alpha = 0.5f;
	smoke->d_alpha = -smoke->alpha * 1000.0f / (float)duration; // Rate of change in transparency.
	smoke->d_scale = 1.0f; // Rate of change in scale.
	smoke->nextThinkTime = smoke->startTime + duration;
	smoke->Update = KeepSelfAI;

	AddEffect(NULL, smoke); // Add the effect as independent world effect.
}

void FXDarkSmoke(const vec3_t origin, const float scale, const float range)
{
	SpawnSmoke(origin, scale, range, 0xaa777777); //mxd
}

void FXSmoke(const vec3_t origin, const float scale, const float range)
{
	SpawnSmoke(origin, scale, range, color_white.c); //mxd
}

static qboolean EnvSmokeSpawner(struct client_entity_s* self, centity_t* owner)
{
	FXSmoke(self->r.origin, self->r.scale, self->Scale);
	return true;
}

static qboolean EnvSmokeSpawner2(struct client_entity_s* self, centity_t* owner)
{
	if (self->LifeTime-- > 0)
	{
		FXSmoke(self->r.origin, flrand(0.5f, 1.0f), flrand(32.0f, 64.0f));
		self->updateTime = 30;

		return true;
	}

	return false;
}

void FXEnvSmoke(centity_t* owner, const int type, int flags, vec3_t origin)
{
	flags |= CEF_NO_DRAW | CEF_NOMOVE;
	client_entity_t* self = ClientEntity_new(type, flags, origin, NULL, 17);

	if (flags & CEF_FLAG6)
	{
		// Just a hiss and steam.
		FXSmoke(origin, flrand(0.5f, 1.0f), flrand(32.0f, 64.0f));
		fxi.S_StartSound(origin, -1, CHAN_AUTO, fxi.S_RegisterSound("misc/fout.wav"), 1.0f, ATTN_NORM, 0.0f);

		self->LifeTime = 33;
		self->Update = EnvSmokeSpawner2;

		AddEffect(NULL, self);
	}
	else
	{
		vec3_t dir;
		byte scale;
		byte speed;
		byte wait;
		byte maxrange;
		fxi.GetEffect(owner, flags, clientEffectSpawners[FX_ENVSMOKE].formatString, &scale, &dir, &speed, &wait, &maxrange);

		AnglesFromDir(dir, self->r.angles);
		self->velocity[0] = (float)speed * 10.0f;
		self->Scale = maxrange;
		self->r.scale = 32.0f / (float)scale;
		self->updateTime = wait * 1000;
		self->Update = EnvSmokeSpawner;

		AddEffect(owner, self);
	}
}