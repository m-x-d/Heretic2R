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

enum SparkSoundID_e //mxd
{
	SND_LAVADROP1,
	SND_LAVADROP2,
	SND_LAVADROP3,
	SND_LAVABURN,

	NUM_SOUNDS
};

static struct sfx_s* spark_sounds[NUM_SOUNDS]; //mxd

void PreCacheSparks(void)
{
	spark_models[0] = fxi.RegisterModel("sprites/fx/bluestreak.sp2");
	spark_models[1] = fxi.RegisterModel("sprites/fx/spark.sp2");
}

void PreCacheSparksSFX(void) //mxd
{
	spark_sounds[SND_LAVADROP1] = fxi.S_RegisterSound("ambient/lavadrop1.wav");
	spark_sounds[SND_LAVADROP2] = fxi.S_RegisterSound("ambient/lavadrop2.wav");
	spark_sounds[SND_LAVADROP3] = fxi.S_RegisterSound("ambient/lavadrop3.wav");
	spark_sounds[SND_LAVABURN] = fxi.S_RegisterSound("misc/lavaburn.wav");
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

void FXGenericSparks(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	vec3_t dir;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_SPARKS].formatString, dir); // Normalized direction vector.
	GenericSparks(owner, type, flags, origin, dir);
}

static qboolean FireSparkSpawnerUpdate(client_entity_t* spawner, centity_t* owner)
{
	//fixme- wigs out on hivetrialpit when the sparkers hit ground and separate?
	vec3_t diff;
	VectorSubtract(spawner->r.origin, spawner->startpos2, diff);

	vec3_t pos;
	VectorMA(spawner->startpos2, (float)spawner->LifeTime / 5.0f, diff, pos);

	spawner->LifeTime++;

	if (spawner->LifeTime >= 5)
	{
		FireSparks(NULL, FX_SPARKS, spawner->SpawnInfo, pos, spawner->direction);

		spawner->LifeTime = 1;
		VectorCopy(spawner->r.origin, spawner->startpos2);
	}
	else
	{
		FireSparks(NULL, FX_SPARKS, 0, pos, spawner->direction);
	}

	VectorCopy(owner->lerp_origin, spawner->r.origin);

	return true;
}

void FireSparks(centity_t* owner, const int type, int flags, const vec3_t origin, const vec3_t dir)
{
	if (owner && flags & CEF_FLAG8)
	{
		// Spawn a continuous thingy - fixme- delay 100 so can get a valid origin?
		flags &= ~CEF_FLAG8;
		client_entity_t* spark_spawner = ClientEntity_new(type, (int)(flags | CEF_NO_DRAW), origin, NULL, 20);

		VectorCopy(origin, spark_spawner->startpos2);
		VectorCopy(origin, spark_spawner->r.origin);
		VectorCopy(dir, spark_spawner->direction);

		spark_spawner->LifeTime = 1;
		spark_spawner->SpawnInfo = flags;

		if (owner->current.effects & EF_MARCUS_FLAG1)
			spark_spawner->SpawnInfo |= CEF_FLAG7;

		spark_spawner->Update = FireSparkSpawnerUpdate;

		AddEffect(owner, spark_spawner);

		return;
	}

	if (flags & CEF_FLAG6)
	{
		// Sound.
		struct sfx_s* sfx = (irand(0, 3) ? spark_sounds[irand(SND_LAVADROP1, SND_LAVADROP3)] : spark_sounds[SND_LAVABURN]); //mxd
		fxi.S_StartSound(origin, -1, CHAN_AUTO, sfx, 1.0f, ATTN_NORM, 0.0f);
	}

	client_entity_t* effect = ClientEntity_new(type, flags | CEF_ADDITIVE_PARTS, origin, NULL, 2000);

	effect->flags |= (CEF_NO_DRAW | CEF_NOMOVE);
	effect->color.c = 0xe5007fff;

	for (int i = 0; i < irand(7, 13); i++)
	{
		client_particle_t* flame = ClientParticle_new(irand(PART_16x16_FIRE1, PART_16x16_FIRE3), effect->color, 1000);

		vec3_t work;
		VectorRandomCopy(dir, work, 0.5f);

		if (flags & CEF_FLAG7) // Fireball poof effect.
		{
			flame->scale = flrand(5.0f, 10.0f);
			VectorScale(work, 20.0f, flame->velocity);
			flame->velocity[2] += 30.0f;
			flame->acceleration[2] = 2.0f;

			flame->d_scale = flrand(-10.0f, -5.0f);
			flame->d_alpha = -10.0f;
			flame->duration = (int)((float)flame->color.a * 1000.0f / -flame->d_alpha); // Time taken to reach zero alpha.
		}
		else
		{
			flame->scale = flrand(3.0f, 5.0f);
			VectorScale(work, 50.0f, flame->velocity);
			flame->velocity[2] += 50.0f;
			flame->acceleration[2] = -200.0f;

			flame->d_scale = flrand(-2.0f, -3.0f);
			flame->d_alpha = 0.0f;
			flame->duration = (int)(flame->scale * 1000.0f / -flame->d_scale); // Time taken to reach zero alpha.
		}

		flame->origin[2] -= 8.0f; //HACK!!!!

		AddParticleToList(effect, flame);
	}

	if (flags & CEF_FLAG6)
	{
		effect->color.c = 0xFF80FFFF;
		effect->dlight = CE_DLight_new(effect->color, 80.0f, -35.0f);
	}

	AddEffect(NULL, effect);
}