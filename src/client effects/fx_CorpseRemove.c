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

// Used only on player bodies in level.body_que, only in COOP/DM --mxd.
void FXCorpseRemove(centity_t* owner, const int type, int flags, vec3_t origin)
{
#define	NUM_FLAME_ITEMS		20
#define NUM_FLAME_PARTS		40
#define FLAME_ABSVEL		120.0f

	static const paletteRGBA_t small_particle_color = { .c = 0xff4f4f4f };

	int count = GetScaledCount(NUM_FLAME_ITEMS, 0.95f);
	count = ClampI(count, 8, NUM_FLAME_ITEMS);

	// Create main client entity.
	client_entity_t* flame_fx = ClientEntity_new(type, (int)(flags | CEF_NO_DRAW), origin, NULL, 600);
	flame_fx->radius = 10.0f;
	AddEffect(NULL, flame_fx);

	// Large particles.
	const float angle_step = ANGLE_360 / (float)count;
	for (int i = 0; i < count; i++)
	{
		client_particle_t* bp = ClientParticle_new(PART_32x32_BLACKSMOKE, color_white, 600);

		bp->scale = 16.0f;
		bp->d_scale = -25.0f;

		const float angle = (float)i * angle_step;
		VectorSet(bp->velocity, FLAME_ABSVEL * cosf(angle), FLAME_ABSVEL * sinf(angle), 0.0f);
		VectorScale(bp->velocity, -0.3f, bp->acceleration);

		AddParticleToList(flame_fx, bp);
	}

	count = GetScaledCount(NUM_FLAME_PARTS, 0.1f);

	// Small particles.
	for (int i = 0; i < count; i++)
	{
		client_particle_t* sp = ClientParticle_new(PART_4x4_WHITE, small_particle_color, 600);

		sp->scale = 1.0f;
		sp->d_scale = -1.0f;

		const float angle = flrand(0, ANGLE_360);
		const float vel = flrand(FLAME_ABSVEL, FLAME_ABSVEL * 2.5f);
		VectorSet(sp->velocity, vel * cosf(angle), vel * sinf(angle), 0.0f);
		VectorScale(sp->velocity, -0.3f, sp->acceleration);
		sp->type |= (PFL_ADDITIVE | PFL_SOFT_MASK);

		AddParticleToList(flame_fx, sp);
	}
}