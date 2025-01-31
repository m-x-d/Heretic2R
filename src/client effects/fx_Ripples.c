//
// fx_Ripples.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Utilities.h"

static struct model_s* ripple_model;

void PreCacheRipples(void)
{
	ripple_model = fxi.RegisterModel("sprites/fx/waterentryripple.sp2");
}

static qboolean RippleSpawnerThink(client_entity_t* spawner, centity_t* owner)
{
	const float alpha = 1.0f / (float)((4 - spawner->SpawnInfo) * (4 - spawner->SpawnInfo));

	client_entity_t* ripple = ClientEntity_new(-1, 0, spawner->origin, spawner->direction, 1000);

	ripple->r.model = &ripple_model;
	ripple->r.flags = RF_FIXED | RF_TRANSLUCENT | RF_ALPHA_TEXTURE;
	ripple->r.scale = 0.01f;
	ripple->d_scale = 1.0f;
	ripple->alpha = alpha;
	ripple->d_alpha = -alpha;

	AddEffect(NULL, ripple);

	if (spawner->SpawnInfo-- < 0)
	{
		spawner->updateTime = 1000;
		spawner->Update = RemoveSelfAI;
	}

	return true;
}

void FXWaterRipples(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	vec3_t dir;
	float dist;

	if (GetWaterNormal(origin, 1.0f, 20.0f, dir, &dist))
	{
		const vec3_t pos = { origin[0], origin[1], origin[2] + dist };
		client_entity_t* spawner = ClientEntity_new(type, flags, pos, dir, 200);

		spawner->flags |= CEF_NO_DRAW | CEF_NOMOVE;
		spawner->SpawnInfo = 3;
		spawner->Update = RippleSpawnerThink;

		AddEffect(NULL, spawner);
	}
}