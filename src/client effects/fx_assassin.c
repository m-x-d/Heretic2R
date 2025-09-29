//
// fx_assassin.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Particle.h"
#include "Vector.h"
#include "Random.h"
#include "g_playstats.h"

static struct sfx_s* tport_smoke_sound; //mxd

void PreCacheTPortSmokeSFX(void) //mxd
{
	tport_smoke_sound = fxi.S_RegisterSound("monsters/assassin/smoke.wav");
}

void FXTPortSmoke(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	client_entity_t* smoke = ClientEntity_new(type, (uint)(flags | CEF_NO_DRAW), origin, NULL, 1650);

	smoke->r.color.a = 128;
	smoke->radius = 10.0f;

	AddEffect(NULL, smoke);

	int num_puffs;
	int duration; //mxd
	float scale_min; //mxd
	float scale_max; //mxd

	if (R_DETAIL >= DETAIL_HIGH) //TODO: separate case for DETAIL_UBERHIGH.
	{
		num_puffs = irand(20, 30);
		duration = 1600;
		scale_min = 60.0f;
		scale_max = 70.0f;
	}
	else if (R_DETAIL == DETAIL_NORMAL)
	{
		num_puffs = irand(10, 20);
		duration = 1500;
		scale_min = 50.0f;
		scale_max = 60.0f;
	}
	else
	{
		num_puffs = irand(8, 13);
		duration = 1300;
		scale_min = 30.0f;
		scale_max = 45.0f;
	}

	fxi.S_StartSound(origin, -1, CHAN_WEAPON, tport_smoke_sound, 1.0f, ATTN_NORM, 0.0f);

	for (int i = 0; i < num_puffs; i++)
	{
		client_particle_t* ce = ClientParticle_new((int)(PART_32x32_BLACKSMOKE | PFL_NEARCULL), smoke->r.color, duration);

		VectorClear(ce->origin);

		ce->velocity[0] = flrand(-100.0f, 100.0f);
		ce->velocity[1] = flrand(-100.0f, 100.0f);
		ce->velocity[2] = flrand(50.0f, 250.0f);
		VectorScale(ce->velocity, -1.23f, ce->acceleration);

		ce->scale = flrand(scale_min, scale_max);
		ce->d_scale = -20.0f;
		ce->d_alpha = -77.0f;

		AddParticleToList(smoke, ce);
	}
}