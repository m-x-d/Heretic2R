//
// fx_Ripples.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Utilities.h"

#define	SCALE		0.01F
#define DELTA_SCALE	1.0F

static struct model_s* ripple_model;

void PreCacheRipples(void)
{
	ripple_model = fxi.RegisterModel("sprites/fx/waterentryripple.sp2");
}

// --------------------------------------------------------------

static qboolean FXRippleSpawner(client_entity_t *spawner, centity_t *owner)
{
	client_entity_t		*ripple;
	float				alpha;

	alpha = 1.0F / ((4 - spawner->SpawnInfo) * (4 - spawner->SpawnInfo));

	ripple = ClientEntity_new(-1, 0, spawner->origin, spawner->direction, 1000);

 	ripple->r.model = &ripple_model;
	ripple->r.flags |= RF_FIXED | RF_TRANSLUCENT | RF_ALPHA_TEXTURE;
	ripple->r.scale = SCALE;
	ripple->d_scale = DELTA_SCALE;
	ripple->alpha = alpha;
	ripple->d_alpha = -alpha;

	AddEffect(NULL, ripple); 

	if(spawner->SpawnInfo-- < 0)
	{
		spawner->updateTime = 1000;
		spawner->Update = RemoveSelfAI;
	}
	return(true);
}

void FXWaterRipples(centity_t *Owner, int Type, int Flags, vec3_t Origin)
{
	client_entity_t		*spawner;
	vec3_t				dir;
	vec_t				dist;

	if(GetWaterNormal(Origin, 1.0, 20.0F, dir, &dist))
	{
		Origin[2] += dist;
		spawner = ClientEntity_new(Type, Flags, Origin, dir, 200);

		spawner->SpawnInfo = 3;
		spawner->Update = FXRippleSpawner;
		spawner->flags |= CEF_NO_DRAW | CEF_NOMOVE;

		AddEffect(NULL, spawner); 
	}
}
// end
