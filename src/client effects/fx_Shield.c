//
// fx_shield.c
//
// Heretic II
// Copyright 1998 Raven Software
//


#include "Client Effects.h"
#include "Client Entities.h"
#include "Particle.h"
#include "ResourceManager.h"
#include "FX.h"
#include "Vector.h"
#include "Matrix.h"
#include "Random.h"
#include "Utilities.h"
#include "Reference.h"
#include "ce_DLight.h"
#include "g_playstats.h"

#define SHIELD_TRAIL_DELAY	100
#define SHIELD_RADIUS		32.0
#define NUM_SHIELD_SPARKS	16
#define SHIELD_TRAIL_SPEED	32.0

extern int	ref_soft;

#define	NUM_SHIELD_MODELS	1
static struct model_s *shield_models[NUM_SHIELD_MODELS];
void PreCacheShield()
{
	shield_models[0] = fxi.RegisterModel("sprites/spells/spark_blue.sp2");
}

static qboolean FXShieldSparkThink(struct client_entity_s *shield,centity_t *owner)
{
	vec3_t angvect;
	client_particle_t *spark;
	int		part;
	paletteRGBA_t		color;

	// Update the angle of the spark.
	VectorMA(shield->direction, (float)(fxi.cl->time-shield->lastThinkTime)/1000.0, shield->velocity2, shield->direction);

	// Update the position of the spark.
	AngleVectors(shield->direction, angvect, NULL, NULL);
	VectorMA(owner->origin, shield->radius, angvect, shield->r.origin);

	shield->lastThinkTime = fxi.cl->time;

	// Leave a trail sometimes
	if (shield->SpawnDelay < fxi.cl->time)
	{
		part = PART_16x16_SPARK_B;
		color.c = 0xffffffff;
		// if we are in software, make the blue bits single point particles half the time
		if (ref_soft)
		{
			shield->SpawnInfo++;
			if (shield->SpawnInfo & 1)
			{
				part |= PFL_SOFT_MASK;
				color.c = 0xffff0000;
			}
		}

		spark = ClientParticle_new(part, color, 500);
		VectorCopy(shield->r.origin, spark->origin);
		spark->scale = 6.0;
		spark->d_scale = -10.0;
		spark->acceleration[2] = 0.0;		// Don't fall due to gravity...
		AddParticleToList(shield, spark);

		// Do it again in 1/10 sec.
		shield->SpawnDelay = fxi.cl->time + SHIELD_TRAIL_DELAY;
	}

	return true;
}


static qboolean FXShieldTerminate(struct client_entity_s *shield, centity_t *owner)
{
	// Don't instantly delete yourself.  Don't accept any more updates and die out within a second.
	shield->d_alpha = -1.2;						// Fade out.
	shield->SpawnDelay = fxi.cl->time + 2000;	// No more particles.
	shield->updateTime = 1000;	// Die in one second.
	shield->Update = RemoveSelfAI;

	return true;
}


// ************************************************************************************************
// FXLightningShield
// ------------
// ************************************************************************************************

// CreateEffect FX_SPELL_LIGHTNINGSHIELD
void FXLightningShield(centity_t *owner,int type,int flags,vec3_t origin)
{
	client_entity_t	*shield;
	int				i;
	vec3_t			angvect;
	int				count;

	count = GetScaledCount(NUM_SHIELD_SPARKS, 0.5);

	// Add spinning electrical sparks
	for(i=0; i<count; i++)
	{
		shield=ClientEntity_new(type, flags&(~CEF_OWNERS_ORIGIN), origin, 0, (int)SHIELD_DURATION*1000);
		shield->flags |= CEF_ADDITIVE_PARTS | CEF_ABSOLUTE_PARTS;
		shield->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		shield->r.model = shield_models;
		shield->radius = SHIELD_RADIUS;
		shield->AddToView = FXShieldSparkThink;			
		shield->color.c = 0xffffffff;
		shield->alpha = 0.1;
		shield->d_alpha = 0.5;
		shield->Update = FXShieldTerminate;

		VectorClear(shield->direction);
		shield->direction[YAW] = flrand(0, 360.0);		// This angle is kept at a constant distance from org.
		shield->direction[PITCH] = flrand(0, 360.0);

		shield->velocity2[YAW] = flrand(-180.0, 180.0);
		if (shield->velocity2[YAW] < 0)			// Assure that the sparks are moving around at a pretty good clip.
			shield->velocity2[YAW] -= 180.0;
		else
			shield->velocity2[YAW] += 180.0;

		shield->velocity2[PITCH] = flrand(-180.0, 180.0);	// This is a velocity around the sphere.
		if (shield->velocity2[PITCH] < 0)		// Assure that the sparks are moving around at a pretty good clip.
			shield->velocity2[PITCH] -= 180.0;
		else
			shield->velocity2[PITCH] += 180.0;

		shield->lastThinkTime = fxi.cl->time;
		shield->SpawnDelay = fxi.cl->time + SHIELD_TRAIL_DELAY;

		AngleVectors(shield->direction, angvect, NULL, NULL);
		VectorMA(owner->origin, shield->radius, angvect, shield->r.origin);

		AddEffect(owner, shield);
	}
}

