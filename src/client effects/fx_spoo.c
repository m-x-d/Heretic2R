//
// fx_spoo.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Random.h"
#include "Utilities.h"
#include "Vector.h"

static struct model_s* spoo_models[2];

void PreCacheSpoo(void)
{
	spoo_models[0] = fxi.RegisterModel("sprites/fx/spoo.sp2");
	spoo_models[1] = fxi.RegisterModel("sprites/fx/spoo2.sp2");
}

static qboolean SpooTrailUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXSpooTrailThink' in original logic.
{
	const int count = GetScaledCount(8, 0.85f);

	for (int i = 0; i < count; i++)
	{
		client_entity_t* trail = ClientEntity_new(FX_SPOO, (int)(self->flags & ~(CEF_OWNERS_ORIGIN | CEF_NO_DRAW)), owner->origin, NULL, 1000);

		trail->radius = 20.0f;
		trail->r.model = &spoo_models[irand(0, 1)];
		trail->r.flags = (RF_TRANSLUCENT | RF_FULLBRIGHT);
		trail->r.scale = 0.65f;
		trail->d_scale = flrand(-4.0f, -3.5f);
		trail->d_alpha = -2.0f;
		trail->color = color_white;

		for (int c = 0; c < 3; c++)
			trail->r.origin[c] += flrand(-3.0f, 3.0f);

		VectorSet(trail->velocity, flrand(-64.0f, 64.0f), flrand(-64.0f, 64.0f), -64.0f);

		AddEffect(NULL, trail);
	}

	VectorCopy(owner->current.origin, self->startpos);

	return true;
}

void FXSpoo(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	client_entity_t* trail = ClientEntity_new(type, flags, origin, NULL, 20);

	trail->flags |= CEF_NO_DRAW;
	VectorCopy(origin, trail->startpos);
	trail->Update = SpooTrailUpdate;

	AddEffect(owner, trail);
	SpooTrailUpdate(trail, owner);
}

void FXSpooSplat(centity_t* owner, int type, const int flags, vec3_t origin)
{
	vec3_t dir;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_SPOO_SPLAT].formatString, &dir);

	const int count = GetScaledCount(16, 0.85f);

	for (int i = 0; i < count; i++)
	{
		client_entity_t* trail = ClientEntity_new(FX_SPOO, 0, origin, NULL, 1000);

		trail->radius = 20.0f;
		trail->r.model = &spoo_models[irand(0, 1)];
		trail->r.flags = RF_TRANSLUCENT;
		trail->r.scale = flrand(0.75f, 1.0f);
		trail->d_scale = flrand(-1.25f, -1.0f);
		trail->d_alpha = flrand(-1.0f, -0.5f);
		trail->color.c = 0xa0ffffff;

		VectorRandomCopy(dir, trail->velocity, 16.0f);
		VectorNormalize(trail->velocity);
		Vec3ScaleAssign(flrand(100.0f, 200.0f), trail->velocity);

		VectorSet(trail->acceleration, 0.0f, 0.0f, -128.0f);

		AddEffect(NULL, trail);
	}
}