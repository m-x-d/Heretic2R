//
// fx_TestBBox.c -- originally named 'TestEffect.c'.
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Particle.h"
#include "Vector.h"
#include "Random.h"

void FXTestBBox(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	float radius;
	float bottom;
	float top;
	vec3_t loc[8];
	int max;

	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_TEST_BBOX].formatString, &radius, &bottom, &top);

	client_entity_t* cent = ClientEntity_new(type, CEF_NO_DRAW | CEF_ADDITIVE_PARTS, origin, NULL, 15000);
	AddEffect(NULL, cent);

	if (flags & CEF_FLAG6)
	{
		// Diamond.
		VectorSet(loc[0], 0, -radius, 0);
		VectorSet(loc[1], 0,  radius, 0);
		VectorSet(loc[2], -radius, 0, 0);
		VectorSet(loc[3],  radius, 0, 0);
		VectorSet(loc[4], 0, 0, bottom);
		VectorSet(loc[5], 0, 0, top);

		max = 6;
	}
	else
	{
		// Square.
		VectorSet(loc[0], -radius, -radius, bottom);
		VectorSet(loc[1],  radius, -radius, bottom);
		VectorSet(loc[2], -radius,  radius, bottom);
		VectorSet(loc[3],  radius,  radius, bottom);
		VectorSet(loc[4], -radius, -radius, top);
		VectorSet(loc[5],  radius, -radius, top);
		VectorSet(loc[6], -radius,  radius, top);
		VectorSet(loc[7],  radius,  radius, top);

		max = 8;
	}

	const int part_id = PART_8x8_RED_X + irand(0, 11);
	const paletteRGBA_t color = { .r = 255, .g = 255, .b = 255, .a = 255 };

	for (int i = 0; i < max; i++)
	{
		client_particle_t* part = ClientParticle_new(part_id, color, 15000);

		VectorCopy(loc[i], part->origin);
		part->acceleration[2] = 0;
		part->scale = 2.0f;
		AddParticleToList(cent, part);
	}
}