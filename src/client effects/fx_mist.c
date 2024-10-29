//
// Heretic II
// Copyright 1998 Raven Software
//
// Created by JJS

#include "Client Effects.h"
#include "Client Entities.h"
#include "ce_DefaultMessageHandler.h"
#include "Particle.h"
#include "ResourceManager.h"
#include "FX.h"
#include "Vector.h"
#include "random.h"
#include "Utilities.h"

#define	MIST_ALPHA	0.6F
#define MIST_FAR	512.0F
#define MIST_NEAR	96.0F

#define	NUM_MIST_MODELS	1
static struct model_s *mist_models[NUM_MIST_MODELS];			   
void PreCacheMist()
{
	mist_models[0] = fxi.RegisterModel("sprites/fx/mist.sp2");
}

// -----------------------------------------------------------------------------------------

static qboolean FXMistThink(client_entity_t *mist, centity_t *owner)
{
	float	mod;

	mist->flags &= ~CEF_DISAPPEARED;

	mist->Scale += flrand(-0.05, 0.05) * mist->SpawnData;
	mist->Scale = Clamp(mist->Scale, 0.6 * mist->SpawnData, 1.4 * mist->SpawnData);
	mist->r.scale = Approach(mist->r.scale, mist->Scale, 0.003 * mist->SpawnData);

	mod = (mist->r.scale / mist->SpawnData) * MIST_ALPHA;
	if(mist->r.depth > MIST_FAR)
	{
		mist->alpha = mod;
		return(true);
	}
	if(mist->r.depth > MIST_NEAR)
	{
		mist->alpha = mod * (mist->r.depth - MIST_NEAR) * (1.0F / (MIST_FAR - MIST_NEAR));
		return(true);
	}
	mist->alpha = 0.0F;
	return(true);
}

void FXMist(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*mist;
	byte				scale;

	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_MIST].formatString, &scale);
	mist = ClientEntity_new(type, flags, origin, NULL, 100);

	mist->SpawnData = scale * 0.1;
	
	mist->r.model = mist_models;
	mist->r.flags |= RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	mist->r.scale = mist->SpawnData;

	mist->flags |= CEF_NOMOVE;
	mist->Update = FXMistThink;
	mist->radius = 1.0F;
	mist->alpha = 0.5F;

	AddEffect(NULL, mist); 
}
// end
