//
// fx_Explosion.c -- originally part of Generic Weapon Effects.c.
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Particle.h"
#include "Vector.h"
#include "Random.h"
#include "Utilities.h"

static void CreateExplosionParticles(client_entity_t* this)
{
#define NUM_EXPLODE_PARTS	256
#define EXP_RANGE			16.0f
#define EXP_SPEED			192.0f

	const int count = GetScaledCount(NUM_EXPLODE_PARTS, 0.9f);

	for (int i = 0; i < count; i++)
	{
		const paletteRGBA_t color = { .r = (byte)irand(127, 255), .g = (byte)irand(127, 255), .b = 0, .a = 255 };
		client_particle_t* p = ClientParticle_new(PART_4x4_WHITE, color, 500);

		VectorRandomSet(p->origin, EXP_RANGE); //mxd
		VectorRandomSet(p->velocity, EXP_SPEED); //mxd

		AddParticleToList(this, p);
	}
}

void FXExplosion1(centity_t* owner, const int type, const int flags, vec3_t origin) //mxd. Named 'GenericExplosion1' in original logic.
{
	client_entity_t* effect = ClientEntity_new(type, flags, origin, NULL, 500);

	effect->alpha = 0.75f;
	effect->flags |= CEF_NO_DRAW;

	AddEffect(NULL, effect); // Add the effect as independent world effect.
	CreateExplosionParticles(effect);
}

void FXExplosion2(centity_t* owner, const int type, const int flags, vec3_t origin) //mxd. Named 'GenericExplosion2' in original logic.
{
	client_entity_t* effect = ClientEntity_new(type, flags, origin, NULL, 500);

	effect->flags |= CEF_NO_DRAW;

	AddEffect(NULL, effect); // Add the effect as independent world effect.
	CreateExplosionParticles(effect);
}