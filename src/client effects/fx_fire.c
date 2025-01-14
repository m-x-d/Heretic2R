//
// fx_fire.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Particle.h"
#include "Vector.h"
#include "Random.h"
#include "Utilities.h"
#include "ce_DLight.h"
#include "g_playstats.h"

#define FLARE_COUNT			16
#define FLARE_SPEED			64.0f
#define FLARE_SPAWN_RADIUS	2.0f
#define FLARE_ACCEL			128.0f
#define FLARE_SCALE			32.0f

#define FLAME_COUNT			4
#define FIRE_SPAWN_RADIUS	8.0f
#define FIRE_SCALE			12.0f
#define FIRE_ENT_SCALE		8.0f
#define FIRE_ACCEL			32.0f

static struct model_s* flareup_model;

void PreCacheFlareup(void)
{
	flareup_model = fxi.RegisterModel("sprites/fx/halo.sp2");
}

void FXFlareup(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	// Add a big ol' flash.
	client_entity_t* spawner = ClientEntity_new(type, flags | CEF_ADDITIVE_PARTS, origin, NULL, 500);
	spawner->r.model = &flareup_model; // The starry halo.
	spawner->r.flags = RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	spawner->radius = 128.0f;

	spawner->d_scale = -2.0f;
	spawner->alpha = 0.95f;
	spawner->d_alpha = -2.0f;
	spawner->color.c = 0xffffffff;

	AddEffect(NULL, spawner);

	// Add fire particles.
	const int count = GetScaledCount(FLARE_COUNT, 0.9f);
	for (int i = 0; i < count; i++)
	{
		const int p_type = irand(PART_32x32_FIRE0, PART_32x32_FIRE2); //mxd
		client_particle_t* flame = ClientParticle_new((int)(p_type | PFL_NEARCULL), spawner->color, 1000);

		const float radius = spawner->r.scale * FLARE_SPAWN_RADIUS;
		VectorRandomSet(flame->origin, radius); //mxd

		flame->scale = spawner->r.scale * FLARE_SCALE;
		VectorRandomSet(flame->velocity, FLARE_SPEED); //mxd
		flame->acceleration[2] = spawner->r.scale * FLARE_ACCEL;
		flame->d_scale = flrand(-20.0f, -10.0f);
		flame->d_alpha = flrand(-320.0f, -256.0f);

		flame->type |= PFL_ADDITIVE;

		AddParticleToList(spawner, flame);
	}
}

qboolean FXFireThink(client_entity_t *spawner, centity_t *owner)
{
	client_particle_t	*flame;
	paletteRGBA_t		color;
	float				radius;
	int					i, count, white;

	count = GetScaledCount(FLAME_COUNT, 0.9);
	if (count>FLAME_COUNT)		// Don't go over flame count
		count=FLAME_COUNT;		
	for(i = 0; i < count; i++)
	{
		if (!irand(0,15) && (r_detail->value >= DETAIL_NORMAL))		// no steam in software, its around too long and doesn't do enough for us
		{
			white = irand(50, 100);

			color.r = color.g = color.b = white; 
			color.a = irand(100, 150);

			flame = ClientParticle_new(PART_32x32_STEAM | PFL_NEARCULL, color, 2000);

			radius = spawner->r.scale * FIRE_SPAWN_RADIUS;
			VectorSet(flame->origin, flrand(-radius, radius), flrand(-radius, radius), 8 * spawner->r.scale);

			flame->scale = (FIRE_SCALE * spawner->r.scale) / 2;
			VectorSet(flame->velocity, flrand(-5.0, 5.0), flrand(-5, 5.0), flrand(15.0, 22.0));
			flame->acceleration[2] = FIRE_ACCEL * spawner->r.scale;
			flame->d_scale = flrand(15.0, 20.0);
			flame->d_alpha = flrand(-100.0, -50.0);
			flame->duration = (color.a * 1000.0) / -flame->d_alpha;		// time taken to reach zero alpha

			AddParticleToList(spawner, flame);
		}
		else
		{
			flame = ClientParticle_new(irand(PART_32x32_FIRE0, PART_32x32_FIRE2) | PFL_NEARCULL, spawner->color, 2000);

			radius = spawner->r.scale * FIRE_SPAWN_RADIUS;
			VectorSet(flame->origin, flrand(-radius, radius), flrand(-radius, radius), flrand(-16.0F, -8.0F) * spawner->r.scale);

			flame->scale = FIRE_SCALE * spawner->r.scale;
			VectorSet(flame->velocity, flrand(-5.0, 5.0), flrand(-5, 5.0), flrand(15.0, 22.0));
			flame->acceleration[2] = FIRE_ACCEL * spawner->r.scale;
			flame->d_scale = flrand(-10.0, -5.0);
			flame->d_alpha = flrand(-200.0, -160.0);
			flame->duration = (255.0 * 1000.0) / -flame->d_alpha;		// time taken to reach zero alpha
			
			flame->type |= PFL_ADDITIVE;

			AddParticleToList(spawner, flame);
		}
	}
	if (spawner->dlight)
		spawner->dlight->intensity = 150 + flrand(-8.0, 8.0);

	return(true);
}

void FXFire(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*spawner;
	byte				scale;

	spawner = ClientEntity_new(type, flags, origin, NULL, 17);

	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_FIRE].formatString, &scale);
	spawner->r.scale = scale / 32.0;

	spawner->r.flags |= RF_FULLBRIGHT|RF_TRANSLUCENT|RF_TRANS_ADD|RF_TRANS_ADD_ALPHA;
	spawner->flags |= CEF_NO_DRAW | CEF_NOMOVE | CEF_CULLED | CEF_CHECK_OWNER | CEF_VIEWSTATUSCHANGED;
	spawner->color.c = 0xe5007fff;
	spawner->radius = 96.0;
	spawner->Update = FXFireThink;

	if(flags & CEF_FLAG6)
		spawner->dlight = CE_DLight_new(spawner->color, 150.0F, 00.0F);

	AddEffect(owner, spawner);
}

qboolean FXFireOnEntityThink(client_entity_t *spawner, centity_t *owner)
{
	client_particle_t	*flame;
	int					i, count, white;
	float				radius;
	paletteRGBA_t		color;

	// For framerate-sensitive effect spawning
	count = GetScaledCount(FLAME_COUNT, 0.9);
	if (count>FLAME_COUNT)		// Don't go over flame count
		count=FLAME_COUNT;		
	VectorCopy(owner->origin, spawner->origin);
	VectorCopy(owner->origin, spawner->r.origin);

	if (!(owner->current.effects & EF_ON_FIRE) && (spawner->nextEventTime - fxi.cl->time >= 1000))
	{	// Set up the fire to finish early.
		spawner->nextEventTime = fxi.cl->time + 999;
		spawner->dlight->d_intensity = -200;
	}

	if (spawner->nextEventTime - fxi.cl->time > 1000)		// Until 1 second before finish.
	{
		for(i = 0; i < count; i++)
		{
			if (!irand(0,15) && (r_detail->value >= DETAIL_NORMAL))		// no steam in software, its around too long and doesn't do enough for us
			{
				white = irand(8, 16);

				color.r = color.g = color.b = white; 
				color.a = 168;

				flame = ClientParticle_new(PART_32x32_STEAM | PFL_NEARCULL, color, 2000);

				radius = spawner->r.scale;
				VectorSet(flame->origin, flrand(-radius, radius), flrand(-radius, radius), flrand(-0.25, 0.75) * spawner->r.scale);
				VectorAdd(flame->origin, spawner->origin, flame->origin);

				flame->scale = spawner->r.scale*2.0;
				VectorSet(flame->velocity, flrand(-5.0, 5.0), flrand(-5, 5.0), flrand(15.0, 22.0));
				flame->acceleration[2] = FIRE_ACCEL * spawner->r.scale;
				flame->d_scale = flrand(15.0, 20.0);
				flame->d_alpha = flrand(-100.0, -50.0);
				flame->duration = (255.0 * 1000.0) / -flame->d_alpha;		// time taken to reach zero alpha

				AddParticleToList(spawner, flame);
			}
			else
			{
				flame = ClientParticle_new(irand(PART_32x32_FIRE0, PART_32x32_FIRE2) | PFL_NEARCULL, spawner->color, 1000);

				radius = spawner->r.scale ;
				VectorSet(flame->origin, flrand(-radius, radius), flrand(-radius, radius), flrand(-0.25, 0.75)*radius);
				// If dead, then move the flame down a tad.
				if(owner->current.effects&EF_DISABLE_EXTRA_FX)
				{
					spawner->origin[2] -= radius;
				}
				VectorAdd(flame->origin, spawner->origin, flame->origin);

				flame->scale = spawner->r.scale*2.0;
				VectorSet(flame->velocity, flrand(-5.0, 5.0), flrand(-5, 5.0), flrand(32.0, 48.0));
				flame->acceleration[2] = 2.0 * spawner->r.scale;
				flame->d_scale = flrand(-20.0, -10.0);
				flame->d_alpha = flrand(-400.0, -320.0);
				flame->duration = (255.0 * 1000.0) / -flame->d_alpha;		// time taken to reach zero alpha

				AddParticleToList(spawner, flame);
			}
		}
		spawner->dlight->intensity = 150 + flrand(-8.0, 8.0);

		return(true);
	}
	else if (fxi.cl->time < spawner->nextEventTime)
	{
		spawner->dlight->intensity = (spawner->nextEventTime - fxi.cl->time)*0.3;

		return(true);
	}
	else
	{
		return(false);
	}
}


qboolean FXFireOnEntity2Think(client_entity_t *spawner, centity_t *owner)
{
	client_particle_t	*flame;
	int					i, count;
	float				radius;

	//fixme: can miss the message that tells you to remove the effect
	if (fxi.cl->time > spawner->nextEventTime)
		return (false);//just in case
	else if (spawner->nextEventTime - fxi.cl->time < 1000)
		return (true);	// Let the flames finish.

	if (!(owner->current.effects & EF_ON_FIRE) && spawner->nextEventTime-fxi.cl->time >= 1000)
	{
		spawner->nextEventTime = fxi.cl->time + 999;
		spawner->dlight->d_intensity = -200;
	}

	// For framerate-sensitive effect spawning
	count = GetScaledCount(FLAME_COUNT, 0.9);
	if (count>FLAME_COUNT)		// Don't go over flame count
		count=FLAME_COUNT;		
	VectorCopy(owner->origin, spawner->origin);
	VectorCopy(owner->origin, spawner->r.origin);
	for(i = 0; i < count; i++)
	{
		flame = ClientParticle_new(irand(PART_32x32_FIRE0, PART_32x32_FIRE2) | PFL_NEARCULL, spawner->color, 1000);

		radius = spawner->r.scale * FIRE_SPAWN_RADIUS;
		VectorSet(flame->origin, flrand(-radius, radius), flrand(-radius, radius), flrand(-8.0F, 0.0F) * spawner->r.scale);
		// If dead, then move the flame down a tad.
		if(owner->current.effects&EF_DISABLE_EXTRA_FX)
		{
			spawner->origin[2] -= radius;
		}
		VectorAdd(flame->origin, spawner->origin, flame->origin);

		flame->scale = FIRE_ENT_SCALE * spawner->r.scale;
		VectorSet(flame->velocity, flrand(-5.0, 5.0), flrand(-5, 5.0), flrand(15.0, 22.0));
		flame->acceleration[2] = FIRE_ACCEL * spawner->r.scale;
		flame->d_scale = flrand(-10.0, -5.0);
		flame->d_alpha = flrand(-200.0, -160.0);
		flame->duration = (255.0 * 1000.0) / -flame->d_alpha;		// time taken to reach zero alpha

		AddParticleToList(spawner, flame);
	}
	spawner->dlight->intensity = 150 + flrand(-8.0, 8.0);
	return(true);

}

//FIXME: have it constantly check a flag so it can go out if under water!
void FXFireOnEntity(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*spawner;
	byte				scale;
	byte				lifetime;
	byte				style;

	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_FIRE_ON_ENTITY].formatString, &scale, &lifetime, &style);

	spawner = ClientEntity_new(type, flags, origin, NULL, 17);

	spawner->r.scale = sqrt((float)scale)*0.5;
	spawner->nextEventTime = fxi.cl->time + (100*(int)lifetime);		// How long to last.  Lifetime was in 10th secs.

	spawner->r.flags |= RF_FULLBRIGHT|RF_TRANSLUCENT|RF_TRANS_ADD|RF_TRANS_ADD_ALPHA;
	spawner->flags |= CEF_NO_DRAW | CEF_NOMOVE | CEF_ADDITIVE_PARTS | CEF_ABSOLUTE_PARTS | CEF_CULLED | CEF_CHECK_OWNER;
//	spawner->flags |= CEF_NO_DRAW | CEF_NOMOVE | CEF_ADDITIVE_PARTS | CEF_ABSOLUTE_PARTS | CEF_CULLED | CEF_VIEWSTATUSCHANGED;
	spawner->color.c = 0xe5007fff;
	spawner->radius = 128;
	if (!style)//fire fades out- for things that catch fire
	{
		spawner->Update = FXFireOnEntityThink;
	}
	else//fire never goes away- for moving fire ents
	{
		spawner->Update = FXFireOnEntity2Think;
		spawner->nextEventTime = fxi.cl->time + 60000;//60 seconds max, just in case
	}
	spawner->dlight = CE_DLight_new(spawner->color, 150.0F, 00.0F);

	AddEffect(owner, spawner);
}
// end
