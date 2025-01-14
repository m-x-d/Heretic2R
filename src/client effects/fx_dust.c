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
	ce = FXDebris_Throw(holdorigin, MAT_STONE, dir, 20000.0, flrand(0.75, 2.4), 0, false);

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

