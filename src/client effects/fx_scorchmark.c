//
// fx_scorchmark.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Vector.h"
#include "Utilities.h"
#include "g_playstats.h"
#include "Random.h"

static struct model_s* scorch_model;

void PreCacheScorch(void)
{
	scorch_model = fxi.RegisterModel("sprites/fx/scorchmark.sp2");
}

//mxd. Added to reduce code duplication. Expects normalized 'dir'. Modifies 'origin' and 'dir'.
static void CreateScorchmark(vec3_t origin, vec3_t dir, const int type, const int flags)
{
	// No scorchmarks in low detail mode.
	if (R_DETAIL == DETAIL_LOW)
		return;

	if (GetTruePlane(origin, dir, 64.0f, flrand(0.5f, 0.75f))) //mxd. Add offset_scale randomization (to reduce z-fighting among overlapping scorch marks).
	{
		client_entity_t* scorchmark = ClientEntity_new(type, flags, origin, dir, 1000);

		scorchmark->radius = 10.0f;
		scorchmark->r.model = &scorch_model;
		scorchmark->r.scale = 0.6f;
		scorchmark->r.flags = (RF_FIXED | RF_TRANSLUCENT);

		if (R_DETAIL >= DETAIL_HIGH) //mxd. +RF_LM_COLOR.
			scorchmark->r.flags |= RF_LM_COLOR;

		scorchmark->Update = KeepSelfAI;

		AddEffect(NULL, scorchmark);
		InsertInCircularList(scorchmark);
	}
}

void FXClientScorchmark(vec3_t origin, vec3_t dir)
{
	CreateScorchmark(origin, dir, FX_SCORCHMARK, CEF_NOMOVE); //mxd
}

void FXScorchmark(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	vec3_t dir;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_SCORCHMARK].formatString, dir);

	CreateScorchmark(origin, dir, type, flags | CEF_NOMOVE); //mxd
}