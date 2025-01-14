//
// fx_dust.c
//
// Copyright 1998 Raven Software
//

#include "Client Entities.h"
#include "Client Effects.h"
#include "fx_debris.h" //mxd
#include "fx_smoke.h" //mxd
#include "Random.h"
#include "Vector.h"
#include "Utilities.h"

void PreCacheRockchunks(void) { } //TODO: remove?

static qboolean FXDustLandThink(client_entity_t* dust, centity_t* owner)
{
	FXSmoke(dust->r.origin, 1.0f, 40.0f);
	return false;
}

static qboolean FXDustThink(client_entity_t* dust, centity_t* owner)
{
	dust->LifeTime++;

	if (dust->LifeTime > dust->SpawnDelay)
		return false;

	dust->updateTime = irand(dust->LifeTime * 17, dust->LifeTime * 50);

	vec3_t hold_origin;
	VectorCopy(dust->r.origin, hold_origin);
	hold_origin[0] += flrand(0.0f, dust->startpos[0]);
	hold_origin[1] += flrand(0.0f, dust->startpos[1]);

	// Spawn a bit of smoke.
	FXSmoke(hold_origin, 3.0f, 25.0f);

	// Spawn a rock chunk.
	vec3_t dir = { 0.0f, 0.0f, -1.0f };
	client_entity_t* rock = FXDebris_Throw(hold_origin, MAT_STONE, dir, 20000.0f, flrand(0.75f, 2.4f), 0, false);

	// Create a cloud of dust when rock hits ground.
	trace_t trace;
	const int duration = GetFallTime(rock->origin, rock->velocity[2], rock->acceleration[2], rock->radius, 3.0f, &trace);

	client_entity_t* dust_cloud = ClientEntity_new(-1, CEF_NO_DRAW | CEF_NOMOVE, trace.endpos, NULL, duration);
	dust_cloud->Update = FXDustLandThink;
	AddEffect(NULL, dust_cloud);

	return true;
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

