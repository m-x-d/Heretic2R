//
// fx_CorpseRemove.c -- originally part of Generic Character Effects.c.
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Particle.h"
#include "Vector.h"
#include "Random.h"
#include "Utilities.h"

void FXCorpseRemove(centity_t* owner, const int type, int flags, vec3_t origin)
{
#define	NUM_FLAME_ITEMS		20
#define NUM_FLAME_PARTS		40
#define FLAME_ABSVEL		120

	int count = GetScaledCount(NUM_FLAME_ITEMS, 0.95f);
	count = ClampI(count, 8, 20);

	// Create main client entity.
	flags |= CEF_NO_DRAW;
	client_entity_t* flame_fx = ClientEntity_new(type, flags, origin, NULL, 600);
	flame_fx->radius = 10.0f;
	flame_fx->color = color_white; //mxd
	AddEffect(NULL, flame_fx);

	// Are we destroying a rat?
	const float vel1 = (float)((flags & CEF_FLAG6) ? FLAME_ABSVEL / 2 : FLAME_ABSVEL);

	// Large particles.
	float angle = 0.0f;
	while (angle < ANGLE_360)
	{
		client_particle_t* bp = ClientParticle_new(PART_32x32_BLACKSMOKE, flame_fx->color, 600);

		bp->scale = 16.0f;
		bp->d_scale = -25.0f;

		VectorSet(bp->velocity, vel1 * cosf(angle), vel1 * sinf(angle), 0.0f);
		VectorScale(bp->velocity, -0.3f, bp->acceleration);

		AddParticleToList(flame_fx, bp);

		angle += ANGLE_360 / (float)count;
	}

	const paletteRGBA_t color = { .c = 0xff4f4f4f };
	count = GetScaledCount(NUM_FLAME_PARTS, 0.1f);

	// Small particles.
	for (int i = 0; i < count; i++)
	{
		client_particle_t* sp = ClientParticle_new(PART_4x4_WHITE, color, 600);

		sp->scale = 1.0f;
		sp->d_scale = -1.0f;

		angle = flrand(0, ANGLE_360);
		const float vel = flrand(vel1, vel1 * 2.5f);
		VectorSet(sp->velocity, vel * cosf(angle), vel * sinf(angle), 0);
		VectorScale(sp->velocity, -0.3f, sp->acceleration);
		sp->type |= (PFL_ADDITIVE | PFL_SOFT_MASK);

		AddParticleToList(flame_fx, sp);
	}
}