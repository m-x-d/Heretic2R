//
// fx_hitpuff.c
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

void PreCacheHitPuff(void)
{
	hit_model = fxi.RegisterModel("sprites/fx/halo.sp2");
}

void FXLightningHit(centity_t *owner, int type, int flags, vec3_t origin)
{
	vec3_t			dir;
	paletteRGBA_t	color;
	int				part;
	client_entity_t	*blast;
	client_particle_t	*spark;
	int				i;

	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_LIGHTNING_HIT].formatString, dir);

	Vec3ScaleAssign(LIGHTNING_VEL*0.25, dir);

	if (flags&CEF_FLAG7)
	{	// RED hit puff
		color.c = 0xff4040ff;
		part=PART_16x16_SPARK_R;
	}
	else if (flags&CEF_FLAG8)
	{	// FIRE hit puff
		color.c = 0xffff80ff;
		part=PART_32x32_FIRE1;
	}
	else
	{	// BLUE hit puff
		color.c = 0xffff8080;
		part=PART_16x16_SPARK_B;
	}

	blast = ClientEntity_new(-1, CEF_ADDITIVE_PARTS, origin, NULL, 500);
	blast->r.model = &hit_model;
	blast->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_TRANSLUCENT; // | 
	blast->r.frame = 1;
	blast->radius = 64.0;
	blast->r.scale=1.0;
	blast->d_alpha=-4.0;
	blast->d_scale=-2.0;
	fxi.S_StartSound(blast->r.origin, -1, CHAN_WEAPON, fxi.S_RegisterSound("weapons/HellHit.wav"), 1, ATTN_NORM, 0);
	blast->dlight = CE_DLight_new(color, 75.0f, 0.0f);
	VectorClear(blast->velocity);
	AddEffect(NULL, blast);

	color.c = 0xffffffff;
	for(i = 0; i < NUM_LIGHTNING_BITS; i++)
	{
		spark = ClientParticle_new(part, color, 500);
		VectorRandomCopy(dir, spark->velocity, LIGHTNING_VEL);
//		VectorSet(dir, 0.0, 0.0, GetGravity() * 0.3);
		spark->scale = flrand(20.0, 32.0);
		spark->d_scale = -32.0;
		spark->d_alpha = flrand(-640.0, -512.0);
		AddParticleToList(blast, spark);
	}
}
