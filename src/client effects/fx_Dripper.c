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
#include "Ambient effects.h"

#define	DRIP_MAX_DURATION	2.0F
#define DRIP_RADIUS			2.0F
#define DRIP_NUM_SPLASHES	8

#define	NUM_DRIP_MODELS	3
static struct model_s *drip_models[NUM_DRIP_MODELS];
void PreCacheDripper()
{
	drip_models[0] = fxi.RegisterModel("sprites/fx/steamhalf.sp2");
	drip_models[1] = fxi.RegisterModel("sprites/fx/steam.sp2");
	drip_models[2] = fxi.RegisterModel("sprites/fx/waterdrop.sp2");
}

// -----------------------------------------------------------------------------------------

qboolean FXDripThinkSolid(client_entity_t *drip, centity_t *owner)
{
	client_entity_t		*mist;
	vec3_t				origin;
	paletteRGBA_t		color;

	VectorCopy(drip->r.origin, origin);
	origin[2] = drip->SpawnData;

	mist = ClientEntity_new(-1, 0, origin, NULL, 500);

	mist->r.model = drip_models;
	mist->r.scale = 0.5F;
	mist->r.flags = RF_TRANSLUCENT;

	mist->alpha = 0.4F;
	mist->d_alpha = -0.8F;

	color.c = 0xffffffff;
	DoWaterSplash(mist, color, DRIP_NUM_SPLASHES);

	AddEffect(NULL, mist); 

	fxi.S_StartSound(origin, -1, CHAN_AUTO,
		fxi.S_RegisterSound(va("ambient/soliddrop%c.wav", irand('1', '3'))), 1, ATTN_STATIC, 0);

	// FIXME : Returning false here doesn`t work
	drip->Update = RemoveSelfAI;
	return(true);
}

qboolean FXDripThinkWater(client_entity_t *drip, centity_t *owner)
{
	client_entity_t		*mist;
	paletteRGBA_t		color;
	vec3_t				origin;

	VectorCopy(drip->r.origin, origin);
	origin[2] = drip->SpawnData;

	mist = ClientEntity_new(-1, 0, origin, NULL, 500);

	mist->r.model = drip_models;
	mist->r.scale = 0.5F;
	mist->r.flags = RF_TRANSLUCENT;

	mist->d_scale = -2.0;
	mist->d_alpha = -8.0F;
//	mist->velocity[0] = flrand(-20.0F, 20.0F);
//	mist->velocity[1] = flrand(-20.0F, 20.0F);
	AddEffect(NULL, mist); 

	color.c = 0xffffffff;
	DoWaterSplash(mist, color, DRIP_NUM_SPLASHES);

	FXWaterRipples(NULL, FX_WATER_RIPPLES, 0, drip->r.origin);

	fxi.S_StartSound(origin, -1, CHAN_AUTO,
		fxi.S_RegisterSound(va("ambient/waterdrop%c.wav", irand('1', '3'))), 1, ATTN_STATIC, 0);

	// FIXME : Returning false here doesn`t work
	drip->Update = RemoveSelfAI;
	return(true);
}

qboolean FXDripThinkLava(client_entity_t *drip, centity_t *owner)
{
	client_entity_t		*mist;
	vec3_t				origin;

	VectorCopy(drip->r.origin, origin);
	origin[2] = drip->SpawnData;

	mist = ClientEntity_new(-1, 0, origin, NULL, 500);

	mist->r.model = drip_models + 1;
	mist->r.scale = 0.5F;
	mist->r.flags = RF_TRANSLUCENT;

	mist->alpha = 0.4F;
	mist->d_alpha = -0.8F;
	mist->velocity[0] = flrand(-10.0F, 10.0F);
	mist->velocity[1] = flrand(-10.0F, 10.0F);
	mist->velocity[2] = flrand(20.0F, 30.0F);
	AddEffect(NULL, mist);

	fxi.S_StartSound(origin, -1, CHAN_AUTO,
		fxi.S_RegisterSound(va("ambient/lavadrop%c.wav", irand('1', '3'))), 1, ATTN_STATIC, 0);

	// FIXME : Returning false here doesn`t work
	drip->Update = RemoveSelfAI;
	return(true);
}

static qboolean FXDripperParticleSpawner(client_entity_t *spawner, centity_t *owner)
{
	client_entity_t		*drip;

	// Refresh time so it gets updated a random amount
	spawner->updateTime = irand(spawner->LifeTime / 2, spawner->LifeTime * 2);

	// Spawn a drip to fall
	drip = ClientEntity_new(-1, 0, spawner->r.origin, NULL, spawner->SpawnDelay);

	drip->r.model = drip_models + 2;
	drip->r.scale = 0.1F;
	drip->r.flags = RF_TRANSLUCENT | RF_ALPHA_TEXTURE;
	drip->r.frame = spawner->r.frame;

	drip->radius = 2.0F;
	drip->SpawnData = spawner->SpawnData;
	VectorCopy(spawner->acceleration, drip->acceleration);

	switch(spawner->SpawnInfo & (CONTENTS_SOLID | CONTENTS_WATER | CONTENTS_LAVA))
	{
		case CONTENTS_WATER:
			drip->Update = FXDripThinkWater;
		break;
		case CONTENTS_LAVA:
			drip->Update = FXDripThinkLava;
		break;
		default:
			drip->Update = FXDripThinkSolid;
		break;
	}

	AddEffect(NULL, drip); 
	return(true);
}

// Spawn a water drop spawner

void FXDripper(centity_t *Owner, int Type, int Flags, vec3_t Origin)
{
	client_entity_t		*dripper;
	byte				dripspermin, frame;
	trace_t				trace;

	fxi.GetEffect(Owner, Flags, clientEffectSpawners[FX_DRIPPER].formatString, &dripspermin, &frame);

	Flags |= CEF_NO_DRAW | CEF_NOMOVE | CEF_VIEWSTATUSCHANGED;
	dripper = ClientEntity_new(Type, Flags, Origin, NULL, 1000);

	dripper->r.frame = frame;

	dripper->LifeTime = (60 * 1000) / dripspermin;
	dripper->Update = FXDripperParticleSpawner;

	dripper->acceleration[2] = GetGravity();
	dripper->radius = DRIP_RADIUS;
	dripper->SpawnDelay = GetFallTime(Origin, 0, dripper->acceleration[2], DRIP_RADIUS, DRIP_MAX_DURATION, &trace);
	dripper->SpawnData = trace.endpos[2] + 4.0F;
	dripper->SpawnInfo = trace.contents;

	AddEffect(Owner, dripper); 
}
// end
