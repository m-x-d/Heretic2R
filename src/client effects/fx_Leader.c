//
// fx_Leader.c -- originally part of Generic Character Effects.c.
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Particle.h"
#include "Vector.h"
#include "Utilities.h"

// Create the two circles that ring the player.
static qboolean LeaderGlowUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXLeaderThink' in original logic.
{
#define LEADER_RAD				12
#define TOTAL_LEADER_EFFECTS	30

	if (--self->LifeTime == 0)
		self->LifeTime = TOTAL_LEADER_EFFECTS;

	// If we are ghosted or dead, don't do the effect.
	if ((owner->current.renderfx & RF_TRANS_GHOST) || (owner->current.effects & EF_CLIENT_DEAD))
		return true;

	const paletteRGBA_t color = { .c = 0x7fffffff };

	// Create the ring of particles that goes up.
	client_particle_t* ce = ClientParticle_new(PART_16x16_SPARK_Y, color, 800);
	ce->acceleration[2] = 0.0f;
	VectorSet(ce->origin, LEADER_RAD * cosf(self->Scale), LEADER_RAD * sinf(self->Scale), 4.0f);
	ce->scale = 8.0f;
	AddParticleToList(self, ce);

	// Create the ring of particles that goes down.
	ce = ClientParticle_new(PART_16x16_SPARK_Y, color, 800);
	ce->acceleration[2] = 0.0f;
	VectorSet(ce->origin, LEADER_RAD * cosf(self->Scale + 3.14f), LEADER_RAD * sinf(self->Scale + 3.14f), 4.0f);
	ce->scale = 8.0f;
	AddParticleToList(self, ce);

	// Move the rings up/down next frame.
	self->Scale += 0.17f;

	return true;
}

// Create the entity the flight loops are on.
void FXLeader(centity_t* owner, const int type, int flags, vec3_t origin)
{
	flags |= (int)(CEF_NO_DRAW | CEF_ADDITIVE_PARTS);
	client_entity_t* glow = ClientEntity_new(type, flags, origin, NULL, 60);

	VectorClear(glow->origin);
	glow->LifeTime = TOTAL_LEADER_EFFECTS;
	glow->Scale = 0.0f;
	glow->AddToView = LinkedEntityUpdatePlacement;
	glow->Update = LeaderGlowUpdate;

	AddEffect(owner, glow);
}