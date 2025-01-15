//
// fx_DefensePickup.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Vector.h"
#include "Random.h"
#include "g_items.h"

#define BOB_HEIGHT					6.0f
#define BOB_SPEED					ANGLE_10
#define NUM_DEFENSE_PICKUPS			6 //mxd
#define NUM_DEFENSE_PICKUP_SPARKS	4
#define SPARK_TRAIL_DELAY			100
#define SPARK_RADIUS				10.0f

static struct model_s* defense_models[NUM_DEFENSE_PICKUPS];
static struct model_s* defense_sparks[NUM_DEFENSE_PICKUPS]; //mxd

void PreCacheItemDefense(void)
{
	defense_models[0] = fxi.RegisterModel("models/items/defense/repulsion/tris.fm");		// ITEM_DEFENSE_REPULSION
	defense_models[1] = fxi.RegisterModel("models/items/defense/meteorbarrier/tris.fm");	// ITEM_DEFENSE_METEORBARRIER
	defense_models[2] = fxi.RegisterModel("models/items/defense/polymorph/tris.fm");		// ITEM_DEFENSE_POLYMORPH
	defense_models[3] = fxi.RegisterModel("models/items/defense/teleport/tris.fm");			// ITEM_DEFENSE_TELEPORT
	defense_models[4] = fxi.RegisterModel("models/items/defense/lightshield/tris.fm");		// ITEM_DEFENSE_SHIELD
	defense_models[5] = fxi.RegisterModel("models/items/defense/tornado/tris.fm");			// ITEM_DEFENSE_TORNADO

	defense_sparks[0] = fxi.RegisterModel("sprites/spells/spark_cyan.sp2");					// Cyan spark.
	defense_sparks[1] = fxi.RegisterModel("sprites/spells/meteorbarrier.sp2");				// Meteor cloud.
	defense_sparks[2] = fxi.RegisterModel("sprites/spells/spark_green.sp2");				// Green spark.
	defense_sparks[3] = fxi.RegisterModel("sprites/spells/spark_red.sp2");					// Red spark.
	defense_sparks[4] = fxi.RegisterModel("sprites/spells/spark_blue.sp2");					// Blue spark.
	defense_sparks[5] = fxi.RegisterModel("sprites/spells/spark_blue.sp2");					// Also blue spark.
}

static qboolean FXDefensePickupSparkThink(struct client_entity_s* shield, centity_t* owner) //mxd. FXEggSparkThink in original version.
{
	vec3_t origin;
	VectorCopy(shield->origin, origin);
	origin[2] = shield->origin[2] + cosf(shield->d_scale2) * BOB_HEIGHT;

	shield->d_scale2 += BOB_SPEED;

	// Update the angle of the spark.
	VectorMA(shield->direction, (float)(fxi.cl->time - shield->lastThinkTime) / 1000.0f, shield->velocity2, shield->direction);

	// Update the position of the spark.
	vec3_t angvect;
	AngleVectors(shield->direction, angvect, NULL, NULL);
	VectorMA(origin, shield->radius, angvect, shield->r.origin);

	shield->lastThinkTime = fxi.cl->time;

	return true;
}

static qboolean FXDefensePickupThink(struct client_entity_s* self, const centity_t* owner)
{
	// Rotate and bob.
	self->r.angles[YAW] += ANGLE_5;
	VectorCopy(owner->current.origin, self->r.origin);
	self->r.origin[2] += cosf(self->SpawnData) * BOB_HEIGHT;
	self->SpawnData += BOB_SPEED;

	return true;
}

void FXDefensePickup(centity_t* owner, const int type, int flags, vec3_t origin)
{
	byte tag;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_PICKUP_DEFENSE].formatString, &tag);

	assert(tag < NUM_DEFENSE_PICKUPS); //mxd. A check in original version.

	flags &= ~CEF_OWNERS_ORIGIN;
	flags |= CEF_DONT_LINK | CEF_CHECK_OWNER | CEF_VIEWSTATUSCHANGED;
	client_entity_t* ce = ClientEntity_new(type, flags, origin, NULL, 50);

	VectorCopy(ce->r.origin, ce->origin);
	ce->r.flags = RF_TRANSLUCENT | RF_GLOW;
	ce->r.model = &defense_models[tag];

	if (tag == ITEM_DEFENSE_TELEPORT)
		ce->r.scale = 1.25f;

	ce->radius = 10.0f;
	ce->alpha = 0.8f;
	ce->Update = FXDefensePickupThink;

	AddEffect(owner, ce);

	// Add spinning electrical sparks.
	for (int i = 0; i < NUM_DEFENSE_PICKUP_SPARKS; i++)
	{
		client_entity_t* spark = ClientEntity_new(type, flags, origin, 0, 50);
		spark->flags |= CEF_ADDITIVE_PARTS | CEF_ABSOLUTE_PARTS | CEF_VIEWSTATUSCHANGED;
		spark->r.flags = RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		spark->r.model = &defense_sparks[tag];
		spark->r.scale = (tag == ITEM_DEFENSE_METEORBARRIER ? 0.2f : 0.8f);
		spark->radius = SPARK_RADIUS;
		spark->color = color_white; //mxd
		spark->alpha = 0.1f;
		spark->d_alpha = 0.5f;
		spark->SpawnData = tag;

		spark->Update = FXDefensePickupSparkThink;
		VectorCopy(spark->r.origin, spark->origin);

		VectorClear(spark->direction);
		spark->direction[YAW] =   flrand(0, 360.0f); // This angle is kept at a constant distance from org.
		spark->direction[PITCH] = flrand(0, 360.0f);

		// Assure that the sparks are moving around at a pretty good clip.
		spark->velocity2[YAW] =   flrand(0.0f, 360.0f) * Q_signf(flrand(-1.0f, 0.0f));
		spark->velocity2[PITCH] = flrand(0.0f, 360.0f) * Q_signf(flrand(-1.0f, 0.0f)); // This is a velocity around the sphere.

		spark->SpawnDelay = fxi.cl->time + SPARK_TRAIL_DELAY;
		spark->lastThinkTime = fxi.cl->time;

		AddEffect(owner, spark);
	}
}