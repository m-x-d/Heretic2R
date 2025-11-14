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

static qboolean LightningShieldSparkThink(struct client_entity_s* shield, centity_t* owner)
{
	// Update the angle of the spark.
	VectorMA(shield->direction, (float)(fx_time - shield->lastThinkTime) / 1000.0f, shield->velocity2, shield->direction);

	// Update the position of the spark.
	vec3_t direction;
	AngleVectors(shield->direction, direction, NULL, NULL);
	VectorMA(owner->origin, shield->radius, direction, shield->r.origin);

	shield->lastThinkTime = fx_time;

	// Leave a trail sometimes.
	if (shield->SpawnDelay < fx_time)
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
		shield->SpawnDelay = fx_time + SHIELD_TRAIL_DELAY;
	}

	return true;
}

static qboolean LightningShieldTerminate(struct client_entity_s* shield, centity_t* owner)
{
	// Don't instantly delete yourself. Don't accept any more updates and die out within a second.
	shield->d_alpha = -1.2f; // Fade out.
	shield->SpawnDelay = fx_time + 2000; // No more particles.
	shield->updateTime = 1000; // Die in one second.
	shield->Update = RemoveSelfAI;

	return true;
}

void FXLightningShield(centity_t* owner, const int type, const int flags, vec3_t origin)
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

		VectorClear(shield->direction);

		// This is a velocity around the sphere.
		for (int c = 0; c < 2; c++)
		{
			shield->direction[c] = flrand(0.0f, 360.0f); // This angle is kept at a constant distance from org.
			shield->velocity2[c] = flrand(-180.0f, 180.0f);
			shield->velocity2[c] += 180.0f * Q_signf(shield->velocity2[c]); // Assure that the sparks are moving around at a pretty good clip.
		}

		shield->lastThinkTime = fx_time;
		shield->SpawnDelay = fx_time + SHIELD_TRAIL_DELAY;

		vec3_t direction;
		AngleVectors(shield->direction, direction, NULL, NULL);
		VectorMA(owner->origin, shield->radius, direction, shield->r.origin);

		shield->AddToView = LightningShieldSparkThink;
		shield->Update = LightningShieldTerminate;

		AddEffect(owner, shield);
	}
}