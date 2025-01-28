//
// fx_sparks.c
//
// Copyright 1998 Raven Software
//

#include "fx_sparks.h" //mxd
#include "Particle.h"
#include "Random.h"
#include "Vector.h"
#include "ce_Dlight.h"
#include "q_Sprite.h"

static struct model_s* spark_models[2];

void PreCacheSparks(void)
{
	spark_models[0] = fxi.RegisterModel("sprites/fx/bluestreak.sp2");
	spark_models[1] = fxi.RegisterModel("sprites/fx/spark.sp2");
}

void GenericSparks(centity_t* owner, const int type, int flags, const vec3_t origin, const vec3_t dir)
{
	if (flags & CEF_FLAG7) // Fire sparks.
	{
		flags &= ~CEF_FLAG7;
		FireSparks(owner, type, flags, origin, dir);

		return;
	}

	int count;
	if (type == FX_BLOCK_SPARKS)
		count = 8;
	else if (flags & CEF_FLAG6)
		count = 5;
	else
		count = irand(2, 4);

	// Create spark balls.
	for (int i = 0; i < count; i++)
	{
		client_entity_t* spark = ClientEntity_new(type, flags, origin, NULL, 1000);

		spark->r.model = &spark_models[1]; // Blue spark sprite.
		spark->r.flags = RF_TRANS_ADD | RF_TRANSLUCENT | RF_TRANS_ADD_ALPHA;

		vec3_t work;
		VectorRandomCopy(dir, work, 0.5f);
		VectorScale(work, flrand(100.0f, 125.0f), spark->velocity); //mxd. irand() in original logic.
		spark->acceleration[2] = flrand(-200.0f, -100.0f);
		spark->r.scale = ((type == FX_BLOCK_SPARKS) ? 0.5f : 0.25f);
		spark->d_scale = flrand(-0.25f, -0.75f);

		AddEffect(NULL, spark); // Add the effect as independent world effect.
	}

	// Create spark streaks.
	for (int i = 0; i < count; i++)
	{
		client_entity_t* streak = ClientEntity_new(type, flags, origin, NULL, 1000);

		streak->r.model = &spark_models[0]; // Blue streak sprite.
		streak->r.flags = RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		streak->r.spriteType = SPRITE_LINE;
		streak->r.tile = 1.0f;
		streak->r.scale = 2.0f;
		streak->r.scale = flrand(0.5f, 1.0f);

		vec3_t work;
		VectorRandomCopy(dir, work, 0.5f);
		VectorScale(work, flrand(100.0f, 125.0f), streak->velocity); //mxd. irand() in original logic.

		VectorCopy(origin, streak->r.endpos);
		VectorMA(streak->r.endpos, flrand(8.0f, 16.0f), work, streak->r.startpos); //mxd. irand() in original logic.

		streak->d_alpha = flrand(-2.0f, -4.0f);
		streak->d_scale = flrand(-2.0f, -4.0f);

		AddEffect(NULL, streak);
	}

	vec3_t dlight_org; //mxd
	VectorMA(origin, -8.0f, dir, dlight_org);

	client_entity_t* dlight = ClientEntity_new(type, flags, dlight_org, NULL, 800);

	dlight->flags |= CEF_NO_DRAW | CEF_NOMOVE;
	dlight->dlight = CE_DLight_new(dlight->color, 80.0f, -35.0f);

	AddEffect(NULL, dlight);
}

void FXGenericSparks(centity_t *owner, int type, int flags, vec3_t origin)
{
	vec3_t				dir;

	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_SPARKS].formatString, dir );			// normalized direction vector
	GenericSparks(owner, type, flags, origin, dir);
}

qboolean FireSparkSpawnerUpdate(client_entity_t *spawner, centity_t *owner)
{//fixme- wigs out on hivetrialpit when the sparkers hit ground and seperate?
	vec3_t	diffvec, pos;

	VectorSubtract(spawner->r.origin, spawner->startpos2, diffvec);
	VectorMA(spawner->startpos2, (float)(spawner->LifeTime)/5, diffvec, pos);

	spawner->LifeTime++;
	
	if(spawner->LifeTime >= 5)
	{
		FireSparks(NULL, FX_SPARKS, spawner->SpawnInfo, pos, spawner->direction);
		spawner->LifeTime = 1;
		VectorCopy(spawner->r.origin, spawner->startpos2);
	}
	else
		FireSparks(NULL, FX_SPARKS, 0, pos, spawner->direction);

	VectorCopy(owner->lerp_origin, spawner->r.origin);
	return (true);
}

void FireSparks(centity_t *owner, int type, int flags, vec3_t origin, vec3_t dir)
{
	client_entity_t		*effect;
	client_particle_t	*flame;
	vec3_t				work;
	byte				count;
	int					i;

	if(owner && flags & CEF_FLAG8)
	{//spawn a continuous thingy - fixme- dalay 100 so can get a valid origin?
		flags &= ~CEF_FLAG8;
		effect = ClientEntity_new(type, flags | CEF_NO_DRAW, origin, NULL, 20);

		effect->LifeTime = 1;
		VectorCopy(origin, effect->startpos2);
		VectorCopy(origin, effect->r.origin);
		VectorCopy(dir, effect->direction);
		effect->SpawnInfo = flags;
		effect->Update = FireSparkSpawnerUpdate;
		if(owner->current.effects & EF_MARCUS_FLAG1)
			effect->SpawnInfo |= CEF_FLAG7;
		
		AddEffect(owner, effect);
		return;
	}

	if (flags & CEF_FLAG6)
	{//sound
		if(irand(0, 3))
		{
			fxi.S_StartSound(origin, -1, CHAN_AUTO,
				fxi.S_RegisterSound(va("ambient/lavadrop%c.wav", irand('1', '3'))), 1, ATTN_NORM, 0);
		}
		else
		{
			fxi.S_StartSound(origin, -1, CHAN_AUTO,
				fxi.S_RegisterSound("misc/lavaburn.wav"), 1, ATTN_NORM, 0);
		}
	}

	effect = ClientEntity_new(type, flags|CEF_ADDITIVE_PARTS, origin, NULL, 2000);
	effect->flags |= CEF_NO_DRAW | CEF_NOMOVE;
	effect->color.c = 0xe5007fff;

	count = irand(7, 13);
	for(i = 0; i < count; i++)
	{
		flame = ClientParticle_new(irand(PART_16x16_FIRE1, PART_16x16_FIRE3), effect->color, 1000);

		VectorRandomCopy(dir, work, 0.5);

		if(flags&CEF_FLAG7)//fireball poofy effect
		{
			flame->scale = flrand(5, 10);
			VectorScale(work, 20.0, flame->velocity);
			flame->velocity[2] += 30;
			flame->acceleration[2] = 2.0f;									  

			flame->d_scale = flrand(-10.0, -5.0);
			flame->d_alpha = -10.0f;
			flame->duration = (flame->color.a * 1000.0) / -flame->d_alpha;		// time taken to reach zero alpha
		}
		else
		{
			flame->scale = flrand(3, 5);
			VectorScale(work, 50.0, flame->velocity);
			flame->velocity[2] += 50;
			flame->acceleration[2] = -200.0f;									  

			flame->d_scale = flrand(-2.0, -3.0);
			flame->d_alpha = 0.0f;
			flame->duration = (flame->scale * 1000.0) / -flame->d_scale;		// time taken to reach zero alpha
		}
		
		flame->origin[2] -= 8;//HACK!!!!

		AddParticleToList(effect, flame);
	}

	if (flags & CEF_FLAG6)
	{
		effect->color.c = 0xFF80FFFF;
		effect->dlight = CE_DLight_new(effect->color, 80.0F, -35.0F);
	}
	AddEffect(NULL, effect);
}
// end

