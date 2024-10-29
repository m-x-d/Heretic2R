//
// fx_smoke.c
//
// Heretic II
// Copyright 1998 Raven Software
//

#include "Angles.h"
#include "Client Entities.h"
#include "Client Effects.h"
#include "q_shared.h"
#include "Vector.h"
#include "EffectFlags.h"
#include "Particle.h"
#include "Random.h"
#include "Motion.h"
#include "Utilities.h"
#include "fx.h"

#define	NUM_SMOKE_MODELS	1
static struct model_s *smoke_models[NUM_SMOKE_MODELS];

void PreCacheSmoke()
{
	smoke_models[0] = fxi.RegisterModel("sprites/fx/steam.sp2");
}

// --------------------------------------------------------------

void FXDarkSmoke(vec3_t origin, float scale, float range)
{
	client_entity_t		*effect;
	int					duration;

	effect = ClientEntity_new(-1, RF_TRANSLUCENT, origin, NULL, 500);

	effect->r.model = smoke_models;
	effect->r.scale = scale;
	effect->r.color.c = 0xaa777777;

	duration = Q_ftol(GetTimeToReachDistance(50.0, 0.0, range));

	VectorSet(effect->velocity, flrand(-10.0, 10.0), flrand(-10.0, 10.0), 50.0);
	effect->nextThinkTime = effect->startTime + duration;
	effect->alpha = 0.5;
	effect->d_alpha = (-effect->alpha * 1000.0) / duration;		// Rate of change in transparency
	effect->d_scale = 1.0;		// Rate of change in scale

	effect->Update = KeepSelfAI;
	AddEffect(NULL, effect);	// add the effect as independent world effect
}

void FXSmoke(vec3_t origin, float scale, float range)
{
	client_entity_t		*effect;
	int					duration;

	effect = ClientEntity_new(-1, RF_TRANSLUCENT, origin, NULL, 500);

	effect->r.model = smoke_models;
	effect->r.scale = scale;
	effect->r.color.c = 0xffffffff;

	duration = Q_ftol(GetTimeToReachDistance(50.0, 0.0, range));

	VectorSet(effect->velocity, flrand(-10.0, 10.0), flrand(-10.0, 10.0), 50.0);
	effect->nextThinkTime = effect->startTime + duration;
	effect->alpha = 0.5;
	effect->d_alpha = (-effect->alpha * 1000.0) / duration;		// Rate of change in transparency
	effect->d_scale = 1.0;		// Rate of change in scale

	effect->Update = KeepSelfAI;
	AddEffect(NULL, effect);	// add the effect as independent world effect
}

static qboolean FXSmokeSpawner(struct client_entity_s *self, centity_t *owner)
{
	FXSmoke(self->r.origin, self->r.scale, self->Scale);
	return(true);
}

static qboolean FXSmokeSpawner2(struct client_entity_s *self, centity_t *owner)
{
	if(self->LifeTime--)
	{
		FXSmoke(self->r.origin, flrand(0.5, 1.0), flrand(32, 64));
		self->updateTime = 30;
		return(true);
	}
	else
		return (false);
}

//------------------------------------------------------------------
//	FX Smoke spawn function
//------------------------------------------------------------------

void FXEnvSmoke(centity_t *owner,int type,int flags,vec3_t origin)
{
	client_entity_t	*self;
	vec3_t	dir;
	byte	scale;
	byte	speed,wait,maxrange;

	flags |= CEF_NO_DRAW | CEF_NOMOVE;
	self = ClientEntity_new(type, flags, origin, NULL, 17);

	if(flags&CEF_FLAG6)
	{//just a hiss and steam
		FXSmoke(origin, flrand(0.5, 1.0), flrand(32, 64));
		fxi.S_StartSound(origin, -1, CHAN_AUTO, fxi.S_RegisterSound("misc/fout.wav"), 1, ATTN_NORM, 0);
		self->Update = FXSmokeSpawner2;
		self->LifeTime = 33;
		AddEffect(NULL, self); 
	}
	else
	{
		fxi.GetEffect(owner,flags,clientEffectSpawners[FX_ENVSMOKE].formatString, &scale, &dir, &speed, &wait, &maxrange);
		AnglesFromDir(dir, self->r.angles);
		self->velocity[0] = speed * 10;
		self->Scale = maxrange;
		self->r.scale = 32.0 / scale;
		self->updateTime = wait * 1000;
		self->Update = FXSmokeSpawner;
		AddEffect(owner, self); 
	}
}

