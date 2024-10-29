//
// Heretic II
// Copyright 1998 Raven Software
//
// Created by JJS

#include "Client Effects.h"
#include "Client Entities.h"
#include "ce_DefaultMessageHandler.h"
#include "Particle.h"
#include "ResourceManager.h"
#include "FX.h"
#include "Vector.h"
#include "random.h"
#include "g_playstats.h"

#define NUM_MIST_EXPLODE_PARTS	7

static qboolean FXPlagueMistExplodeSpawn(client_entity_t *spawner, centity_t *owner)
{
	int					i, count;
	client_particle_t	*p;
	paletteRGBA_t		color;
	int					duration;
	float				mist_scale, mist_d_scale;

	spawner->LifeTime -= spawner->SpawnInfo;
	if(spawner->LifeTime < 0)
		return(false);

	count = (spawner->LifeTime - 1600) / 200;
	if(count > NUM_MIST_EXPLODE_PARTS)
		count = NUM_MIST_EXPLODE_PARTS;
	if(count < 1)
		return(true);

	if (r_detail->value >= DETAIL_HIGH)
	{
		duration = 1500;
		mist_scale = 10.0;
		mist_d_scale = 6.0;
	}
	else
	if (r_detail->value == DETAIL_NORMAL)
	{
		duration = 1250;
		mist_scale = 9.0;
		mist_d_scale = 5.5;
	}
	else
	{
		duration = 1000;
		mist_scale = 8.0;
		mist_d_scale = 5.0;
	}

	color.c = 0xffffffff;
	for(i = 0; i < count; i++)
	{
		color.r = irand(140, 195);
		color.g = irand(140, 195);
		color.b = irand(140, 195);
		color.a = irand(220, 255);

		p = ClientParticle_new(PART_16x16_MIST | PFL_NEARCULL, color, duration);
		p->velocity[0] = flrand(-75.0F, 75.0F);
		p->velocity[1] = flrand(-75.0F, 75.0F);
		p->velocity[2] = flrand(-10.0F, 75.0F);
		VectorScale(p->velocity, -1.1F, p->acceleration);
		p->scale = mist_scale;
		p->d_scale = mist_d_scale;
		AddParticleToList(spawner, p);
	}

	return(true);
}

void FXPlagueMistExplode(centity_t *Owner, int Type, int Flags, vec3_t Origin)
{
	client_entity_t		*spawner;
	byte				lifetime;
	int					mist_life;
	int					mist_think_time;

	if (r_detail->value >= DETAIL_HIGH)
	{
		mist_life = 50;
		mist_think_time = 50;
	}
	else
	if (r_detail->value == DETAIL_NORMAL)
	{
		mist_life = 38;
		mist_think_time = 60;
	}
	else
	{
		mist_life = 33;
		mist_think_time = 70;
	}

	Flags = (Flags & ~CEF_OWNERS_ORIGIN) | CEF_NOMOVE | CEF_NO_DRAW;
	spawner = ClientEntity_new(Type, Flags, Origin, NULL, mist_think_time);
	fxi.GetEffect(Owner, Flags, clientEffectSpawners[FX_PLAGUEMISTEXPLODE].formatString, &lifetime);
	spawner->LifeTime = lifetime * mist_life;
	spawner->radius = 20.0F;
	spawner->Update = FXPlagueMistExplodeSpawn;
	spawner->SpawnInfo = mist_life;

	if(Owner)
		AddEffect(Owner, spawner); 
	else
		AddEffect(NULL, spawner); 
}
// end
