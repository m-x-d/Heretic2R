//
// fx_PlagueMistExplode.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Particle.h"
#include "Random.h"
#include "Vector.h"
#include "g_playstats.h"

#define NUM_MIST_EXPLODE_PARTS	7

static qboolean PlagueMistExplodeSpawn(client_entity_t* spawner, centity_t* owner)
{
	spawner->LifeTime -= spawner->SpawnInfo;
	if (spawner->LifeTime < 0)
		return false;

	int count = (spawner->LifeTime - 1600) / 200;
	count = min(NUM_MIST_EXPLODE_PARTS, count);

	if (count < 1)
		return true;

	int duration;
	float mist_scale;
	float mist_d_scale;

	if (R_DETAIL >= DETAIL_HIGH) //TODO: separate case for DETAIL_UBERHIGH.
	{
		duration = 1500;
		mist_scale = 10.0f;
		mist_d_scale = 6.0f;
	}
	else if (R_DETAIL == DETAIL_NORMAL)
	{
		duration = 1250;
		mist_scale = 9.0f;
		mist_d_scale = 5.5f;
	}
	else
	{
		duration = 1000;
		mist_scale = 8.0f;
		mist_d_scale = 5.0f;
	}

	for (int i = 0; i < count; i++)
	{
		paletteRGBA_t color;
		COLOUR_SETA(color, irand(140, 195), irand(140, 195), irand(140, 195), irand(220, 255)); //mxd. Use macro.

		client_particle_t* p = ClientParticle_new((int)(PART_16x16_MIST | PFL_NEARCULL), color, duration);

		VectorSet(p->velocity, flrand(-75.0f, 75.0f), flrand(-75.0f, 75.0f), flrand(-10.0f, 75.0f));
		VectorScale(p->velocity, -1.1f, p->acceleration);
		p->scale = mist_scale;
		p->d_scale = mist_d_scale;

		AddParticleToList(spawner, p);
	}

	return true;
}

void FXPlagueMistExplode(centity_t* owner, const int type, int flags, vec3_t origin)
{
	int mist_life;
	int mist_think_time;

	if (R_DETAIL >= DETAIL_HIGH) //TODO: separate case for DETAIL_UBERHIGH.
	{
		mist_life = 50;
		mist_think_time = 50;
	}
	else if (R_DETAIL == DETAIL_NORMAL)
	{
		mist_life = 38;
		mist_think_time = 60;
	}
	else
	{
		mist_life = 33;
		mist_think_time = 70;
	}

	flags = (int)((flags & ~CEF_OWNERS_ORIGIN) | CEF_NOMOVE | CEF_NO_DRAW);
	client_entity_t* spawner = ClientEntity_new(type, flags, origin, NULL, mist_think_time);

	byte lifetime;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_PLAGUEMISTEXPLODE].formatString, &lifetime);

	spawner->radius = 20.0f;
	spawner->SpawnInfo = mist_life;
	spawner->LifeTime = lifetime * mist_life;
	spawner->Update = PlagueMistExplodeSpawn;

	AddEffect(owner, spawner);
}