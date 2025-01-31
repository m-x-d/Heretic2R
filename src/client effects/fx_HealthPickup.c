//
// fx_HealthPickup.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Particle.h"
#include "Vector.h"
#include "Random.h"

#define BOB_HEIGHT		6.0f
#define BOB_SPEED		ANGLE_10
#define HEALTH_RADIUS	6.0f

static struct model_s* health_models[2];

void PreCacheHealth(void)
{
	health_models[0] = fxi.RegisterModel("models/items/health/healthsmall/tris.fm");
	health_models[1] = fxi.RegisterModel("models/items/health/healthbig/tris.fm");
}

static qboolean HealthPickupThink(struct client_entity_s* self, centity_t* owner)
{
	paletteRGBA_t color;

	// Rotate and bob.
	self->r.angles[YAW] += ANGLE_15;
	VectorCopy(owner->current.origin, self->r.origin);
	self->r.origin[2] += cosf(self->SpawnData) * BOB_HEIGHT;
	self->SpawnData += BOB_SPEED;

	// Spawn particles.
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

	VectorSet(p->origin, flrand(-HEALTH_RADIUS, HEALTH_RADIUS), flrand(-HEALTH_RADIUS, HEALTH_RADIUS), 0.0f);
	VectorSet(p->velocity, 0.0f, 0.0f, flrand(20.0f, 40.0f));
	p->acceleration[2] = 20.0f;
	AddParticleToList(self, p);

	return true;
}

void FXHealthPickup(centity_t* owner, const int type, int flags, vec3_t origin)
{
	flags &= ~CEF_OWNERS_ORIGIN;
	flags |= CEF_DONT_LINK | CEF_CHECK_OWNER | CEF_VIEWSTATUSCHANGED;
	client_entity_t* ce = ClientEntity_new(type, flags, origin, NULL, 50);

	ce->radius = 10.0f;
	const int model_index = (flags & CEF_FLAG6) >> 5; // 0 - small, 1 - big.
	ce->r.model = &health_models[model_index];
	ce->r.flags = RF_GLOW | RF_TRANSLUCENT | RF_TRANS_ADD;
	ce->alpha = 0.8f;

	if (model_index == 0) // Bigger scale for Half Health.
		ce->r.scale = 1.5f;

	ce->Update = HealthPickupThink;

	AddEffect(owner, ce);
}