//
// fx_HealthPickup.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Particle.h"
#include "Vector.h"
#include "Random.h"
#include "Utilities.h"

#define BOB_HEIGHT			6.0f
#define BOB_SPEED			ANGLE_10
#define HEALTH_RADIUS		6.0f
#define HEALTH_SMALL_RADIUS	3.0f //mxd
#define ANIMATION_SPEED		50 //mxd

static struct model_s* health_models[2];

void PreCacheHealth(void)
{
	health_models[0] = fxi.RegisterModel("models/items/health/healthsmall/tris.fm");
	health_models[1] = fxi.RegisterModel("models/items/health/healthbig/tris.fm");
}

static qboolean HealthPickupUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXHealthPickupThink' in original logic.
{
	// Rotate and bob.
	const int step = fx_time - self->nextThinkTime; //mxd
	const float lerp = (float)step / ANIMATION_SPEED; //mxd
	self->LifeTime += step; //mxd

	self->r.angles[YAW] += ANGLE_15 * lerp;
	VectorCopy(owner->origin, self->r.origin); //mxd. Use interpolated origin (to make items dropped by Drop_Item() fly smoothly).
	self->r.origin[2] += cosf(self->SpawnData) * BOB_HEIGHT;
	self->SpawnData += BOB_SPEED * lerp;

	if (self->SpawnData > ANGLE_360)
		self->SpawnData = fmodf(self->SpawnData, ANGLE_360); //mxd. Otherwise SpawnData will eventually (in ~8 minutes of runtime) cause floating point precision errors...

	//mxd. Spawn particles at original rate.
	if (self->LifeTime < ANIMATION_SPEED)
		return true;

	self->LifeTime %= ANIMATION_SPEED;

	// Spawn particles.
	paletteRGBA_t color;

	if (self->flags & CEF_FLAG6)
	{
		color.g = (byte)irand(80, 120);
		color.r = (byte)irand(210, 255);
		color.b = color.r;
	}
	else
	{
		color.r = (byte)irand(80, 120);
		color.b = (byte)irand(210, 255);
		color.g = color.r;
	}

	color.a = 255;
	client_particle_t* p = ClientParticle_new(PART_4x4_WHITE | PFL_SOFT_MASK, color, 600);

	if (self->flags & CEF_FLAG6)
	{
		VectorSet(p->origin, flrand(-HEALTH_RADIUS, HEALTH_RADIUS), flrand(-HEALTH_RADIUS, HEALTH_RADIUS), 0.0f);
	}
	else //mxd. Align with the bottle cap.
	{
		const float yaw = -self->r.angles[YAW] - ANGLE_180;
		VectorSet(p->origin, sinf(yaw) * 3.0f + flrand(-HEALTH_SMALL_RADIUS, HEALTH_SMALL_RADIUS), 
							 cosf(yaw) * 3.0f + flrand(-HEALTH_SMALL_RADIUS, HEALTH_SMALL_RADIUS), 6.0f);
	}
	
	VectorSet(p->velocity, 0.0f, 0.0f, flrand(20.0f, 40.0f));
	p->acceleration[2] = 20.0f;
	AddParticleToList(self, p);

	return true;
}

void FXHealthPickup(centity_t* owner, const int type, int flags, vec3_t origin)
{
	flags &= ~CEF_OWNERS_ORIGIN;
	flags |= (CEF_DONT_LINK | CEF_CHECK_OWNER | CEF_VIEWSTATUSCHANGED);
	client_entity_t* ce = ClientEntity_new(type, flags, origin, NULL, 0); //mxd. next_think_time 50 in original logic. Set to 0, so self->nextThinkTime holds previous update time in AmmoPickupThink()...

	ce->radius = 10.0f;
	const int model_index = (flags & CEF_FLAG6) >> 5; // 0 - small, 1 - big.
	ce->r.model = &health_models[model_index];
	ce->r.flags = (RF_GLOW | RF_TRANSLUCENT | RF_TRANS_ADD);
	ce->alpha = 0.8f;

	if (model_index == 0) // Bigger scale for Half Health.
		ce->r.scale = 1.5f;

	ce->SpawnData = GetPickupBobPhase(origin); //mxd
	ce->Update = HealthPickupUpdate;

	AddEffect(owner, ce);
}