//
// fx_Shield.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Particle.h"
#include "Random.h"
#include "Utilities.h"
#include "Vector.h"
#include "g_playstats.h"

#define SHIELD_TRAIL_DELAY	100
#define SHIELD_RADIUS		32.0f
#define NUM_SHIELD_SPARKS	16

static struct model_s* shield_model;

void PreCacheShield(void)
{
	shield_model = fxi.RegisterModel("sprites/spells/spark_blue.sp2");
}

static qboolean FXShieldSparkThink(struct client_entity_s* shield, const centity_t* owner)
{
	// Update the angle of the spark.
	VectorMA(shield->direction, (float)(fxi.cl->time - shield->lastThinkTime) / 1000.0f, shield->velocity2, shield->direction);

	// Update the position of the spark.
	vec3_t direction;
	AngleVectors(shield->direction, direction, NULL, NULL);
	VectorMA(owner->origin, shield->radius, direction, shield->r.origin);

	shield->lastThinkTime = fxi.cl->time;

	// Leave a trail sometimes.
	if (shield->SpawnDelay < fxi.cl->time)
	{
		int particle_type = PART_16x16_SPARK_B;
		paletteRGBA_t color = { .c = 0xffffffff };

		// If we are in software, make the blue bits single point particles half the time.
		if (ref_soft && ++shield->SpawnInfo & 1)
		{
			particle_type |= PFL_SOFT_MASK;
			color.c = 0xffff0000;
		}

		client_particle_t* spark = ClientParticle_new(particle_type, color, 500);

		VectorCopy(shield->r.origin, spark->origin);
		spark->scale = 6.0f;
		spark->d_scale = -10.0f;
		spark->acceleration[2] = 0.0f; // Don't fall due to gravity...

		AddParticleToList(shield, spark);

		// Do it again in 1/10 sec.
		shield->SpawnDelay = fxi.cl->time + SHIELD_TRAIL_DELAY;
	}

	return true;
}

static qboolean FXShieldTerminate(struct client_entity_s* shield, centity_t* owner)
{
	// Don't instantly delete yourself. Don't accept any more updates and die out within a second.
	shield->d_alpha = -1.2f; // Fade out.
	shield->SpawnDelay = fxi.cl->time + 2000; // No more particles.
	shield->updateTime = 1000; // Die in one second.
	shield->Update = RemoveSelfAI;

	return true;
}

void FXLightningShield(centity_t* owner, const int type, const int flags, const vec3_t origin)
{
	const int count = GetScaledCount(NUM_SHIELD_SPARKS, 0.5f);

	// Add spinning electrical sparks.
	for (int i = 0; i < count; i++)
	{
		client_entity_t* shield = ClientEntity_new(type, flags & (~CEF_OWNERS_ORIGIN), origin, NULL, SHIELD_DURATION * 1000);

		shield->radius = SHIELD_RADIUS;
		shield->flags |= CEF_ADDITIVE_PARTS | CEF_ABSOLUTE_PARTS;
		shield->r.flags = RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		shield->r.model = &shield_model;
		shield->color = color_white;
		shield->alpha = 0.1f;
		shield->d_alpha = 0.5f;

		shield->direction[YAW] = flrand(0.0f, 360.0f); // This angle is kept at a constant distance from org.
		shield->direction[PITCH] = flrand(0.0f, 360.0f);

		shield->velocity2[YAW] = flrand(-180.0f, 180.0f);
		if (shield->velocity2[YAW] < 0.0f) // Assure that the sparks are moving around at a pretty good clip.
			shield->velocity2[YAW] -= 180.0f;
		else
			shield->velocity2[YAW] += 180.0f;

		shield->velocity2[PITCH] = flrand(-180.0f, 180.0f); // This is a velocity around the sphere.
		if (shield->velocity2[PITCH] < 0.0f) // Assure that the sparks are moving around at a pretty good clip.
			shield->velocity2[PITCH] -= 180.0f;
		else
			shield->velocity2[PITCH] += 180.0f;

		shield->lastThinkTime = fxi.cl->time;
		shield->SpawnDelay = fxi.cl->time + SHIELD_TRAIL_DELAY;

		vec3_t direction;
		AngleVectors(shield->direction, direction, NULL, NULL);
		VectorMA(owner->origin, shield->radius, direction, shield->r.origin);

		shield->AddToView = FXShieldSparkThink;
		shield->Update = FXShieldTerminate;

		AddEffect(owner, shield);
	}
}