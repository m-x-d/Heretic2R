#include "Client Effects.h"
#include "Client Entities.h"
#include "FX.h"
#include "Particle.h"
#include "ResourceManager.h"
#include "Vector.h"
#include "Random.h"
#include "ce_Dlight.h"
#include "q_Sprite.h"
#include "Utilities.h"
#include "reference.h"
#include "Matrix.h"
#include "g_playstats.h"

enum {
	MODEL_SPARKSTREAK,
	MODEL_SPARKFIRE,
	MODEL_SPARK,
	NUM_MODELS
};

static struct model_s *spark_models[NUM_MODELS];

void PreCacheSparks()
{
	spark_models[0] = fxi.RegisterModel("sprites/fx/bluestreak.sp2");
	spark_models[1] = fxi.RegisterModel("sprites/fx/fire.sp2");
	spark_models[2] = fxi.RegisterModel("sprites/fx/spark.sp2");
}

// --------------------------------------------------------------

void FireSparks(centity_t *owner, int type, int flags, vec3_t origin, vec3_t dir);

void GenericSparks(centity_t *owner, int type, int flags, vec3_t origin, vec3_t dir)
{
	client_entity_t		*effect;
	vec3_t				work;
	byte				count;
	int					i;

	if (flags & CEF_FLAG7)//fire sparks
	{
		flags &= ~CEF_FLAG7;
		FireSparks(owner, type, flags, origin, dir);
		return;
	}

	if (type == FX_BLOCK_SPARKS)
		count = 8;
	else if (flags & CEF_FLAG6)
		count = 5;
	else
		count = irand(2,4);

	//Create spark balls
	for(i = 0; i < count; i++)
	{
		effect = ClientEntity_new(type, flags, origin, NULL, 1000);

		effect->r.model = spark_models + MODEL_SPARK;
		effect->r.flags |= RF_TRANS_ADD | RF_TRANSLUCENT | RF_TRANS_ADD_ALPHA;

		VectorRandomCopy(dir, work, 0.5);
		VectorScale(work, irand(100.0, 125.0), effect->velocity);
		effect->acceleration[2] = flrand(-200.0, -100.0);
		
		if (type == FX_BLOCK_SPARKS)
			effect->r.scale = 0.5;
		else
			effect->r.scale = 0.25;

		effect->d_scale = flrand(-0.25, -0.75);
		effect->color.c = 0xFFFFFFFF;

		AddEffect(NULL, effect);	// add the effect as independent world effect
	}

	//Create spark streaks
	for(i = 0; i < count; i++)
	{
		effect = ClientEntity_new(type, flags, origin, NULL, 1000);

		effect->r.flags |= RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;

		effect->r.model = spark_models + MODEL_SPARKSTREAK;	
		effect->r.spriteType = SPRITE_LINE;
		effect->r.tile = 1.0;
		effect->r.scale = 2;
		effect->alpha = 1.0;
		effect->r.scale = flrand(0.5, 1.0);

		VectorRandomCopy(dir, work, 0.5);
		VectorScale(work, irand(100.0, 125.0), effect->velocity);
		
		VectorCopy(origin, effect->r.endpos);
		VectorMA(effect->r.endpos, irand(8, 16), work, effect->r.startpos);

		effect->d_alpha = flrand(-2.0, -4.0);
		effect->d_scale = flrand(-2.0, -4.0);

		AddEffect(NULL,effect);
	}

	VectorMA(origin, -8, dir, origin);
	effect = ClientEntity_new(type, flags, origin, NULL, 800);
	effect->flags |= CEF_NO_DRAW | CEF_NOMOVE;
	effect->color.c = 0xFFFFFFFF;
	effect->dlight = CE_DLight_new(effect->color, 80.0F, -35.0F);
	AddEffect(NULL, effect);
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

