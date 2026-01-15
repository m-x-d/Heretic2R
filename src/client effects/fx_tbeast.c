//
// fx_TrialBeast.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Random.h"
#include "Vector.h"

static struct model_s* tb_dustpuff_model;

void PreCacheTB(void)
{
	tb_dustpuff_model = fxi.RegisterModel("sprites/fx/steam_add.sp2");
}

static qboolean TBDustPuffUpdate(client_entity_t* puff, centity_t* owner) //mxd. Named 'FXTBDustPuffThink' in original logic.
{
	puff->flags &= ~CEF_DISAPPEARED;
	return puff->alpha > 0.0f;
}

static void TBDustPuff(const int type, const int flags, vec3_t origin, const float in_angle)
{
	client_entity_t* puff = ClientEntity_new(type, flags, origin, NULL, 100);

	puff->radius = 1.0f;
	puff->r.model = &tb_dustpuff_model; // steam_add sprite.
	puff->r.flags = (RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
	puff->r.scale = flrand(0.15f, 0.3f);
	puff->d_scale = 0.75f;
	puff->alpha = 0.5f;
	puff->d_alpha = -1.0f;

	vec3_t forward;
	const vec3_t angles = VEC3_SET(0.0f, in_angle, 0.0f);
	AngleVectors(angles, forward, NULL, NULL);

	VectorScale(forward, flrand(30.0f, 100.0f), puff->velocity);
	puff->velocity[2] = flrand(25.0f, 75.0f);
	puff->acceleration[2] = puff->velocity[2] * -1.23f;

	puff->Update = TBDustPuffUpdate;

	AddEffect(NULL, puff);
}

static void TBDustPuffOnGround(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	for (int i = 0; i < 8; i++)
		TBDustPuff(type, flags, origin, (float)i * flrand(30.0f, 60.0f));
}

void FXTBEffects(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	byte fx_index;
	vec3_t vel;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_TB_EFFECTS].formatString, &fx_index, &vel); //TODO: 'vel' arg is unused.

	if (fx_index == FX_TB_PUFF)
		TBDustPuffOnGround(owner, type, flags, origin);
}