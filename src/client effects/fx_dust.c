//
// fx_dust.c
//
// Heretic II
// Copyright 1998 Raven Software
//
#include "Angles.h"
#include "Client Entities.h"
#include "Client Effects.h"
#include "q_shared.h"
#include "ce_DefaultMessageHandler.h"
#include "EffectFlags.h"
#include "q_Physics.h"
#include "fx_debris.h"
#include "random.h"
#include "Vector.h"
#include "Particle.h"
#include "Utilities.h"
#include "fx.h"

typedef struct RockChunk
{
	char	*modelName;
	struct model_s *model;
	float	mass;
} RockChunk_t;

static void FXDust_RockThrow(int type, int flags, vec3_t origin);
static void FXDust_DustThrow(int type, int flags, vec3_t origin);
void FXSmoke(vec3_t origin, float scale, float range);
client_entity_t *FXDebris_Throw(vec3_t origin, int material, vec3_t dir, float ke,float scale, int flags);

#define	NUM_ROCK_MODELS	4
static struct model_s *rock_models[NUM_ROCK_MODELS];
RockChunk_t RockChunks[]=
{
	{"models/debris/stone/schunk1/tris.fm", 0, 3.0},
	{"models/debris/stone/schunk2/tris.fm", 0, 3.0},
	{"models/debris/stone/schunk3/tris.fm", 0, 3.0},
	{"models/debris/stone/schunk4/tris.fm", 0, 3.0},
};

void PreCacheRockchunks()
{
	int		i;

	for(i = 0; i < NUM_ROCK_MODELS; i++)
	{
		rock_models[i] = fxi.RegisterModel(RockChunks[i].modelName);
	}
}

//------------------------------------------------------------------
//	FX Dust spawn functions
//------------------------------------------------------------------

static qboolean FXDustLandThink(client_entity_t *dust, centity_t *owner)
{
	FXSmoke(dust->r.origin, 1.0, 40.0);
	return(false);
}

static qboolean FXDustThink(client_entity_t *dust, centity_t *owner)
{
	vec3_t			holdorigin, dir;
	trace_t			trace;
	client_entity_t	*ce;
	int				duration;

	dust->LifeTime++;
	if(dust->LifeTime > dust->SpawnDelay)
	{
		return(false);
	}
	dust->updateTime = irand(dust->LifeTime * 17, dust->LifeTime * 50);

	VectorCopy(dust->r.origin, holdorigin);
	holdorigin[0] += flrand(0.0, dust->startpos[0]);
	holdorigin[1] += flrand(0.0, dust->startpos[1]);

	// Spawn a bit of smoke
	FXSmoke(holdorigin, 3.0, 25.0);

	// Spawn a rock chunk
	VectorSet(dir, 0.0, 0.0, -1.0);
	ce = FXDebris_Throw(holdorigin, MAT_STONE, dir, 20000.0, flrand(0.75, 2.4), 0);

	// Create a cloud of dust when rock hits ground
	duration = GetFallTime(ce->origin, ce->velocity[2], ce->acceleration[2], ce->radius, 3.0F, &trace);
	ce = ClientEntity_new(-1, CEF_NO_DRAW | CEF_NOMOVE, trace.endpos, NULL, duration);
	ce->Update = FXDustLandThink;
	AddEffect(NULL, ce);

	return(true);
}

void FXDust(centity_t *owner, int type, int flags, vec3_t origin)
{
	byte  				num, mag;
	vec3_t 				size;
	client_entity_t		*ce;

	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_DUST].formatString, &num, size, &mag);

	ce = ClientEntity_new(-1, CEF_NOMOVE | CEF_NO_DRAW, origin, NULL, 100);
	VectorScale(size, mag, ce->startpos);
	ce->SpawnDelay = num;
	ce->LifeTime = 0;
	ce->Update = FXDustThink;
	AddEffect(NULL, ce);
}

// end

