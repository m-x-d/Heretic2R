//														  
// Heretic II
// Copyright 1998 Raven Software
//
// jweier

#include "Client Effects.h"
#include "Client Entities.h"
#include "ce_DefaultMessageHandler.h"
#include "Particle.h"
#include "ResourceManager.h"
#include "FX.h"
#include "Vector.h"
#include "Utilities.h"
#include "random.h"
#include "motion.h"
#include "g_playstats.h"
#include "ce_DLight.h"

#define FLAME_COUNT			4
#define FIRE_SPAWN_RADIUS	8.0
#define FIRE_SCALE 			14.0
#define FIRE_ACCEL			32.0

qboolean FXFlamethrower_trail(client_entity_t *self, centity_t *owner)
{
	client_particle_t	*flame;
	client_entity_t		*light;
	paletteRGBA_t		color;
	float				radius;
	int					count, white;

	if (self->LifeTime < fxi.cl->time)
	{
		self->Update = RemoveSelfAI;
		self->updateTime = fxi.cl->time + 2000;

		return true;
	}

	count = GetScaledCount(FLAME_COUNT, 0.9);

	while (count--)
	{
		color.c = 0xFFFFFFFF;
		
		flame = ClientParticle_new(irand(PART_16x16_FIRE1, PART_16x16_FIRE3), color, 2000);
		flame->d_scale = flrand(-10.0, -5.0);
		flame->d_alpha = flrand(-600.0, -560.0);
		flame->duration = (255.0 * 1000.0) / -flame->d_alpha;		// time taken to reach zero alpha
	
		radius = self->r.scale * FIRE_SPAWN_RADIUS;
		VectorCopy(self->direction, flame->velocity);
		VectorSet(flame->origin, irand(-2,2), irand(-2,2), irand(-2,2));
		VectorScale(flame->velocity, flrand(0.75, 1), flame->velocity);

		flame->scale = flrand(14.0, 20.0);

		flame->velocity[2] += flrand(0, 48);
		flame->acceleration[2] = 64;

		AddParticleToList(self, flame);
	}

	if(r_detail->value >= DETAIL_NORMAL)
	{
		//Spawn a dynamic light
		light = ClientEntity_new(FX_FLAMETHROWER, CEF_NO_DRAW, self->origin, 0, 500);
		VectorCopy(self->direction, light->velocity);

		white = irand(8, 16);

		color.r = 128 + irand(108, 127);
		color.g = 64  + white;
		color.b = 16  + white;
		color.a = 64  + irand(16, 128);

		light->dlight = CE_DLight_new(color, 150.0F, -100.0F);

		AddEffect(NULL, light);
	}


	return true;
}

static qboolean FXFlamethrower_steam_trail(client_entity_t *self, centity_t *owner)
{
	client_particle_t	*flame;
	paletteRGBA_t		color;
	float				radius;
	int					count;

	if (self->LifeTime < fxi.cl->time)
	{
		self->Update = RemoveSelfAI;
		self->updateTime = fxi.cl->time + 2000;

		return true;
	}

	count = GetScaledCount(FLAME_COUNT, 0.9);

	while (count--)
	{
		if (self->flags & CEF_FLAG7)
			color.c = 0x33777777;
		else
			color.c = 0x50FFFFFF;
		
		flame = ClientParticle_new(PART_32x32_STEAM, color, 2000);

		flame->d_alpha = flrand(-200.0, -150.0);
		flame->duration = (255.0 * 1000.0) / -flame->d_alpha;		// time taken to reach zero alpha
	
		radius = self->r.scale * FIRE_SPAWN_RADIUS;
		VectorCopy(self->direction, flame->velocity);
		VectorSet(flame->origin, irand(-2,2), irand(-2,2), irand(-2,2));
		VectorScale(flame->velocity, flrand(0.75, 1), flame->velocity);

		if (self->flags & CEF_FLAG7)
		{
			flame->d_scale = flrand(20.0, 30.0);
			flame->scale = flrand(2.0, 3.0);
			VectorScale(flame->velocity, -0.1, flame->acceleration);
		}
		else
		{
			flame->d_scale = flrand(-10.0, -5.0);
			flame->scale = flrand(14.0, 20.0);
		}

		flame->velocity[2] += flrand(0, 32);
		flame->acceleration[2] = 128;

		AddParticleToList(self, flame);
	}

	return true;
}

// create the initial fire entity that we can attach all the particles to
void FXFlamethrower(centity_t *owner, int type, int flags, vec3_t origin)
{
	float			distance;
	vec3_t			dir;
	client_entity_t	*glow;

	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_FLAMETHROWER].formatString, &dir, &distance);

	// create the dummy entity, so particles can be attached
	glow = ClientEntity_new(type, (flags | CEF_NO_DRAW | CEF_ADDITIVE_PARTS) , origin, 0, 17);
	
	VectorScale(dir, distance, glow->direction);
	glow->radius = 100;
	glow->LifeTime = fxi.cl->time + 1000;
	
	//Steam?
	if (flags & CEF_FLAG6)
	{
		if(!(flags & CEF_FLAG7))
			fxi.S_StartSound(origin, -1, CHAN_AUTO, fxi.S_RegisterSound("objects/steamjet.wav"), 1, ATTN_NORM, 0);
		else
			glow->LifeTime = fxi.cl->time + 200;

		glow->Update = FXFlamethrower_steam_trail;
	}
	else
	{
		fxi.S_StartSound(origin, -1, CHAN_AUTO, fxi.S_RegisterSound("misc/flamethrow.wav"), 1, ATTN_NORM, 0);
		glow->Update = FXFlamethrower_trail;
	}
	
	AddEffect(NULL, glow);
}



							    
// put out there just so we can make the real effect match this
void FXflametest(centity_t *owner, int type, int flags, vec3_t origin)
{
}
