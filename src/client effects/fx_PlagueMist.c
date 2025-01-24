//
// fx_PlagueMist.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Particle.h"
#include "Random.h"
#include "Vector.h"
#include "g_playstats.h"

#define NUM_MIST_PARTS	7

static qboolean FXPlagueMistParticleSpawner(client_entity_t* spawner, centity_t* owner)
{
	int duration;
	float mist_scale;
	float mist_d_scale;

	spawner->LifeTime -= 100;
	if (spawner->LifeTime < 0)
		return false;

	int count = (spawner->LifeTime - 1600) / 150;
	count = min(NUM_MIST_PARTS, count);

	if (count < 1)
		return true;

	if ((int)r_detail->value >= DETAIL_HIGH)
	{
		duration = 1500;
		mist_scale = 12.0f;
		mist_d_scale = 6.0f;
	}
	else if ((int)r_detail->value == DETAIL_NORMAL)
	{
		duration = 1175;
		mist_scale = 10.0f;
		mist_d_scale = 5.5f;
	}
	else
	{
		duration = 900;
		mist_scale = 8.0f;
		mist_d_scale = 5.0f;
	}

	for (int i = 0; i < count; i++)
	{
		paletteRGBA_t color;
		COLOUR_SETA(color, irand(200, 255), irand(200, 255), irand(200, 255), irand(200, 255));

		client_particle_t* p = ClientParticle_new((int)(PART_16x16_MIST | PFL_NEARCULL), color, duration);

		VectorRandomSet(p->origin, 2.0f);
		VectorRandomCopy(spawner->direction, p->velocity, 20.0f);
		VectorScale(spawner->direction, -1.0f, p->acceleration);
		p->acceleration[2] += flrand(20.0f, 30.0f);
		p->scale = flrand(mist_scale, mist_scale + 3.0f);
		p->d_scale = flrand(mist_d_scale, mist_d_scale + 3.0f);

		AddParticleToList(spawner, p);
	}

	return true;
}

void FXPlagueMist(centity_t *Owner, int Type, int Flags, vec3_t Origin)
{
	client_entity_t		*pm;
	byte				lifetime;
	int					mist_think_time;

	if (r_detail->value >= DETAIL_HIGH)
		mist_think_time = 100;
	else
	if (r_detail->value == DETAIL_NORMAL)
		mist_think_time = 125;
	else
		mist_think_time = 150;
	
	pm = ClientEntity_new(Type, Flags, Origin, NULL, mist_think_time);
	fxi.GetEffect(Owner, Flags, clientEffectSpawners[FX_PLAGUEMIST].formatString, pm->direction, &lifetime);

	pm->LifeTime = lifetime * 50;
	pm->Update = FXPlagueMistParticleSpawner;
	pm->flags |= CEF_NO_DRAW | CEF_NOMOVE;

	if(Owner)
		AddEffect(Owner, pm); 
	else
		AddEffect(NULL, pm); 
}

// end

