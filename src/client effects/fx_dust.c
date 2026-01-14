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

static qboolean DustCloudUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXDustLandThink' in original logic.
{
	FXSmoke(self->r.origin, 1.0f, 40.0f);
	return false;
}

static qboolean DustUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXDustThink' in original logic.
{
	self->LifeTime++;

	if (self->LifeTime > self->SpawnDelay)
		return false;

	self->updateTime = irand(self->LifeTime * 17, self->LifeTime * 50);

	// Spawn a bit of smoke.
	const vec3_t hold_origin = VEC3_INITA(self->r.origin, flrand(0.0f, self->startpos[0]), flrand(0.0f, self->startpos[1]), 0.0f);
	FXSmoke(hold_origin, 3.0f, 25.0f);

	// Spawn a rock chunk.
	client_entity_t* rock = FXDebris_Throw(hold_origin, MAT_STONE, vec3_down, 20000.0f, flrand(0.75f, 2.4f), 0, false);

	// Create a cloud of dust when rock hits ground.
	trace_t trace;
	const int duration = GetFallTime(rock->origin, rock->velocity[2], rock->acceleration[2], rock->radius, 3.0f, &trace);

	client_entity_t* dust_cloud = ClientEntity_new(-1, CEF_NO_DRAW | CEF_NOMOVE, trace.endpos, NULL, duration);
	dust_cloud->Update = DustCloudUpdate;
	AddEffect(NULL, dust_cloud);

	return true;
}

void FXDust(centity_t* owner, int type, const int flags, vec3_t origin)
{
	byte num;
	vec3_t size;
	byte mag;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_DUST].formatString, &num, size, &mag);

	client_entity_t* dust = ClientEntity_new(-1, CEF_NOMOVE | CEF_NO_DRAW, origin, NULL, 100);
	VectorScale(size, mag, dust->startpos);
	dust->SpawnDelay = num;
	dust->LifeTime = 0;
	dust->Update = DustUpdate;

	AddEffect(NULL, dust);
}