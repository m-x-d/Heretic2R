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
#include "Random.h"
#include "g_playstats.h"

#define NUM_MIST_PARTS	7

static qboolean FXPlagueMistParticleSpawner(client_entity_t *spawner, centity_t *owner)
{
	client_particle_t	*p;
	paletteRGBA_t		color;
	int					i;
	int					count;
	int					duration;
	float				mist_scale, mist_d_scale;

	spawner->LifeTime -= 100;
	if(spawner->LifeTime < 0)
		return(false);

	count = (spawner->LifeTime - 1600) / 150;
	if(count > NUM_MIST_PARTS)
		count = NUM_MIST_PARTS;
	if(count < 1)
		return(true);

	if (r_detail->value >= DETAIL_HIGH)
	{
		duration = 1500;
		mist_scale = 12.0;
		mist_d_scale = 6.0;
	}
	else
	if (r_detail->value == DETAIL_NORMAL)
	{
		duration = 1175;
		mist_scale = 10.0;
		mist_d_scale = 5.5;
	}
	else
	{
		duration = 900;
		mist_scale = 8.0;
		mist_d_scale = 5.0;
	}


	for(i = 0; i < count; i++)
	{
		color.r = irand(200, 255);
		color.g = irand(200, 255);
		color.b = irand(200, 255);
		color.a = irand(200, 255);

		p = ClientParticle_new(PART_16x16_MIST | PFL_NEARCULL, color, duration);
		VectorSet(p->origin, flrand(-2.0F, 2.0F), flrand(-2.0F, 2.0F), flrand(-2.0F, 2.0F));
		VectorRandomCopy(spawner->direction, p->velocity, 20.0F);
		VectorScale(spawner->direction, -1.0F, p->acceleration);
		p->acceleration[2] += flrand(20.0F, 30.0F);
		p->scale = flrand(mist_scale, mist_scale+3.0);
		p->d_scale = flrand(mist_d_scale, mist_d_scale+3.0);
		AddParticleToList(spawner, p);
	}
	return(true);
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

