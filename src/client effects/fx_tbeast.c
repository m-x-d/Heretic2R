//
// Heretic II
// Copyright 1998 Raven Software
//
// Created by JDW

#include "Client Effects.h"
#include "Client Entities.h"
#include "ce_DefaultMessageHandler.h"
#include "Particle.h"
#include "ResourceManager.h"
#include "FX.h"
#include "Vector.h"
#include "random.h"
#include "Utilities.h"

enum
{
	FX_TB_PUFF,
	FX_TB_SNORT,
};


#define	NUM_BEAST_MODELS	1
static struct model_s *tb_dustpuff_models[NUM_BEAST_MODELS];
void PreCacheTB(void)
{
	tb_dustpuff_models[0] = fxi.RegisterModel("sprites/fx/steam_add.sp2");
}

static qboolean FXTBDustPuffThink(client_entity_t *DustPuff, centity_t *owner)
{
	DustPuff->flags &= ~CEF_DISAPPEARED;
	
	if (DustPuff->alpha <= 0)
		return false;
	
	return(true);
}

void FXTBDustPuff(int type, int flags, vec3_t origin,float inangle)
{
	client_entity_t		*DustPuff;	
	vec3_t				angles, forward;
	
	VectorSet(angles, 0, inangle, 0);
	DustPuff = ClientEntity_new(type, flags, origin, NULL, 100);

	DustPuff->r.model = tb_dustpuff_models;
	DustPuff->r.flags |= RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	DustPuff->Update = FXTBDustPuffThink;
	DustPuff->radius = 1.0F;
	DustPuff->alpha = 0.5F;
	AngleVectors(angles, forward, NULL, NULL);
	VectorScale(forward, flrand(30.0F, 100.0F), DustPuff->velocity);
	DustPuff->velocity[2] = flrand(25.0F, 75.0F);
	DustPuff->acceleration[2] = DustPuff->velocity[2] * -1.23F;
	DustPuff->r.scale = flrand(0.15, 0.3);
	DustPuff->d_scale = 0.75F;
	DustPuff->d_alpha = -1.0F;

	AddEffect(NULL, DustPuff); 
}


void FXTBDustPuffOnGround(centity_t *owner, int type, int flags, vec3_t origin)
{
	int			i;
	
	for (i = 0; i < 8; i++)
	{
		FXTBDustPuff(type,flags,origin, i * flrand(30, 60));
	}
}


void FXTBSnort(centity_t *owner, int type, int flags, vec3_t origin)
{
}

/*===============================

 Trial Beast's FX handler

  ===============================*/

void FXTBEffects(centity_t *owner,int type,int flags, vec3_t org)
{
	paletteRGBA_t	LightColor={0,0,255,255};
	vec3_t			vel;
	byte			fx_index;
	
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_TB_EFFECTS].formatString, &fx_index, &vel);//fixme- make this 1 dir and 1 float

	switch (fx_index)
	{
		case FX_TB_PUFF:
			FXTBDustPuffOnGround(owner, type, flags, org);
			break;

		case FX_TB_SNORT:
			FXTBSnort(owner, type, flags, org);
			break;

		default:
			break;
	}
}

// end
