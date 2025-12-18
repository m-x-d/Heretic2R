//
// fx_GibTrail.c -- originally part of Generic Character Effects.c.
//
// Copyright 1998 Raven Software
//

#include "Client Entities.h"
#include "Particle.h"
#include "Vector.h"
#include "Random.h"

static qboolean GibTrailUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'ParticleTrailAI' in original logic.
{
#define PARTICLE_TRAIL_PUFF_TIME 1000 // Puffs last for 1 sec.

	assert(owner);

	client_entity_t* effect = ClientEntity_new(FX_PUFF, CEF_NO_DRAW, owner->current.old_origin, NULL, PARTICLE_TRAIL_PUFF_TIME);

	for (int i = 0; i < 40; i++)
	{
		client_particle_t* p = ClientParticle_new(PART_4x4_WHITE, self->color, PARTICLE_TRAIL_PUFF_TIME);
		VectorSet(p->velocity, flrand(-20.0f, 20.0f), flrand(-20.0f, 20.0f), flrand(30.0f, 80.0f));
		AddParticleToList(effect, p);
	}

	AddEffect(NULL, effect); // Add the puff to the world.

	return true; // Actual puff spawner only goes away when it owner has a FX_REMOVE_EFFECTS sent on it.
}

void FXGibTrail(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	assert(owner);

	client_entity_t* effect = ClientEntity_new(type, flags, origin, NULL, PARTICLE_TRAIL_THINK_TIME);
	effect->flags |= CEF_NO_DRAW;
	effect->color.c = 0xFF2020FF;
	effect->Update = GibTrailUpdate;

	AddEffect(owner, effect);
	GibTrailUpdate(effect, owner); // Think once right away, to spawn the first puff.
}