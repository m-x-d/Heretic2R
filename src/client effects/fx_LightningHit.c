//
// fx_LightningHit.c -- named fx_hitpuff.c in original logic.
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Particle.h"
#include "Random.h"
#include "Vector.h"
#include "ce_DLight.h"

#define LIGHTNING_VEL		256.0f
#define NUM_LIGHTNING_BITS	25

static struct model_s* hit_model;
static struct sfx_s* hit_sound; //mxd

void PreCacheLightningHit(void) //mxd. Named 'PreCacheHitPuff' in original logic.
{
	hit_model = fxi.RegisterModel("sprites/fx/halo.sp2");
}

void PreCacheLightningHitSFX(void) //mxd
{
	hit_sound = fxi.S_RegisterSound("weapons/HellHit.wav");
}

void FXLightningHit(centity_t* owner, int type, const int flags, vec3_t origin)
{
	paletteRGBA_t color;
	int part;

	vec3_t dir;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_LIGHTNING_HIT].formatString, dir);

	Vec3ScaleAssign(LIGHTNING_VEL * 0.25f, dir);

	if (flags & CEF_FLAG7) // RED hit puff.
	{
		color.c = 0xff4040ff;
		part = PART_16x16_SPARK_R;
	}
	else if (flags & CEF_FLAG8) // FIRE hit puff.
	{
		color.c = 0xffff80ff;
		part = PART_32x32_FIRE1;
	}
	else // BLUE hit puff.
	{
		color.c = 0xffff8080;
		part = PART_16x16_SPARK_B;
	}

	client_entity_t* blast = ClientEntity_new(-1, CEF_ADDITIVE_PARTS, origin, NULL, 500);

	blast->r.model = &hit_model;
	blast->r.flags = RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_TRANSLUCENT;
	blast->r.frame = 1;
	blast->radius = 64.0f;
	blast->d_alpha = -4.0f;
	blast->d_scale = -2.0f;
	blast->dlight = CE_DLight_new(color, 75.0f, 0.0f);

	fxi.S_StartSound(blast->r.origin, -1, CHAN_WEAPON, hit_sound, 1.0f, ATTN_NORM, 0.0f);
	AddEffect(NULL, blast);

	for (int i = 0; i < NUM_LIGHTNING_BITS; i++)
	{
		client_particle_t* spark = ClientParticle_new(part, color_white, 500);

		VectorRandomCopy(dir, spark->velocity, LIGHTNING_VEL);
		spark->scale = flrand(20.0f, 32.0f);
		spark->d_scale = -32.0f;
		spark->d_alpha = flrand(-640.0f, -512.0f);

		AddParticleToList(blast, spark);
	}
}