//
// fx_mist.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Random.h"

#define	MIST_ALPHA	0.6f
#define MIST_FAR	512.0f
#define MIST_NEAR	96.0f

static struct model_s* mist_model;

void PreCacheMist(void)
{
	mist_model = fxi.RegisterModel("sprites/fx/mist.sp2");
}

static qboolean FXMistThink(client_entity_t* mist, centity_t* owner)
{
	mist->flags &= ~CEF_DISAPPEARED;

	mist->Scale += flrand(-0.05f, 0.05f) * mist->SpawnData;
	mist->Scale = Clamp(mist->Scale, mist->SpawnData * 0.6f, mist->SpawnData * 1.4f);
	mist->r.scale = Approach(mist->r.scale, mist->Scale, mist->SpawnData * 0.003f);

	const float mod = mist->r.scale / mist->SpawnData * MIST_ALPHA;

	if (mist->r.depth > MIST_FAR)
		mist->alpha = mod;
	else if (mist->r.depth > MIST_NEAR)
		mist->alpha = mod * (mist->r.depth - MIST_NEAR) * (1.0f / (MIST_FAR - MIST_NEAR));
	else
		mist->alpha = 0.0f;

	return true;
}

void FXMist(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*mist;
	byte				scale;

	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_MIST].formatString, &scale);
	mist = ClientEntity_new(type, flags, origin, NULL, 100);

	mist->SpawnData = scale * 0.1;
	
	mist->r.model = &mist_model;
	mist->r.flags |= RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	mist->r.scale = mist->SpawnData;

	mist->flags |= CEF_NOMOVE;
	mist->Update = FXMistThink;
	mist->radius = 1.0F;
	mist->alpha = 0.5F;

	AddEffect(NULL, mist); 
}
// end
