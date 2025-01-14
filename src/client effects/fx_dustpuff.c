//
// fx_dustpuff.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "fx_dustpuff.h" //mxd
#include "Particle.h"
#include "Vector.h"
#include "Random.h"

static void FXDustPuff(client_entity_t* owner, const float scale)
{
	const paletteRGBA_t color = { .c = 0x80c0c0c0 };
	client_particle_t* puff = ClientParticle_new(PART_32x32_STEAM, color, 500);

	VectorSet(puff->velocity, flrand(-50.0f, 50.0f), flrand(-50.0f, 50.0f), flrand(0.0f, 25.0f));
	VectorScale(puff->velocity, -1.23f, puff->acceleration);
	puff->scale = scale;
	puff->d_scale = 10.0f;
	puff->d_alpha *= 0.5f;

	AddParticleToList(owner, puff);
}

void CreateSinglePuff(vec3_t origin, float scale)
{
 	client_entity_t	*ce;

	ce = ClientEntity_new(-1, CEF_NOMOVE | CEF_NO_DRAW, origin, NULL, 500);
	FXDustPuff(ce, scale);
	AddEffect(NULL, ce);
}
void FXDustPuffOnGround(centity_t *owner, int type, int flags, vec3_t origin)
{
	int				numPuffs, i;
	vec3_t			endpos;
	trace_t			trace;
	client_entity_t	*ce;

	VectorCopy(origin, endpos);
	endpos[2] -= 128.0;

	//Find out where the ground is
	fxi.Trace(	origin, 
				vec3_origin, 
				vec3_origin, 
				endpos, 
				CONTENTS_SOLID, 
				CEF_CLIP_TO_WORLD, 
				&trace);

	if (trace.fraction == 1.0)
		return;

	ce = ClientEntity_new(-1, CEF_NOMOVE | CEF_NO_DRAW, trace.endpos, NULL, 500);
	numPuffs = irand(3,4);
	for (i = 0; i < numPuffs; i++)
	{
		FXDustPuff(ce, 5.0F);
	}
	AddEffect(NULL, ce);
}

// end
