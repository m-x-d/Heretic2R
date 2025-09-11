//
// fx_DefensePickup.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Vector.h"
#include "Random.h"
#include "g_items.h"
#include "Utilities.h"

#define BOB_HEIGHT					6.0f
#define BOB_SPEED					ANGLE_10
#define NUM_DEFENSE_PICKUPS			6 //mxd
#define NUM_DEFENSE_PICKUP_SPARKS	4
#define SPARK_TRAIL_DELAY			100
#define SPARK_RADIUS				10.0f
#define ANIMATION_SPEED				50 //mxd

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
	defense_sparks[1] = fxi.RegisterModel("sprites/spells/spark_green.sp2");				// Green spark. //mxd. meteorbarrier.sp2 in original logic. Visually indistinguishable from spark_green.sp2, but requires custom scaling.
	defense_sparks[2] = fxi.RegisterModel("sprites/spells/spark_yellow.sp2");				// Yellow spark. //mxd. spark_green.sp2 in original logic. Changed to differentiate from ITEM_DEFENSE_METEORBARRIER fx.
	defense_sparks[3] = fxi.RegisterModel("sprites/spells/spark_red.sp2");					// Red spark. //mxd. spark_red.sp2 in original logic.
	defense_sparks[4] = fxi.RegisterModel("sprites/spells/spark_blue.sp2");					// Blue spark.
	defense_sparks[5] = fxi.RegisterModel("sprites/spells/spark_ind.sp2");					// Indigo spark. //mxd. spark_blue.sp2 in original logic. Changed to differentiate from ITEM_DEFENSE_SHIELD fx.
}

static qboolean DefensePickupSparkThink(struct client_entity_s* self, centity_t* owner) //mxd. FXEggSparkThink in original version.
{
	const int step = fxi.cl->time - self->nextThinkTime; //mxd
	const float lerp = (float)step / ANIMATION_SPEED; //mxd

	vec3_t origin;
	VectorCopy(owner->current.origin, origin); //mxd. VectorCopy(self->origin, origin) in original logic. Changed, so sparks move in synch with pickup model.
	origin[2] += cosf(self->SpawnData) * BOB_HEIGHT;
	self->SpawnData += BOB_SPEED * lerp;

	// Update the angle of the spark.
	VectorMA(self->direction, (float)step / 1000.0f, self->velocity2, self->direction);

	// Update the position of the spark.
	vec3_t angvect;
	AngleVectors(self->direction, angvect, NULL, NULL);
	VectorMA(origin, self->radius, angvect, self->r.origin);

	if (self->SpawnInfo == ITEM_DEFENSE_METEORBARRIER) //mxd. Offset to better match with pickup model center.
		self->r.origin[2] += 8.0f;

	return true;
}

static qboolean DefensePickupThink(struct client_entity_s* self, centity_t* owner)
{
	// Rotate and bob.
	const int step = fxi.cl->time - self->nextThinkTime; //mxd
	const float lerp = (float)step / ANIMATION_SPEED; //mxd

	self->r.angles[YAW] += ANGLE_5 * lerp;
	VectorCopy(owner->origin, self->r.origin); //mxd. Use interpolated origin (to make items dropped by Drop_Item() fly smoothly).
	self->r.origin[2] += cosf(self->SpawnData) * BOB_HEIGHT;
	self->SpawnData += BOB_SPEED * lerp;

	return true;
}

void FXDefensePickup(centity_t* owner, const int type, int flags, vec3_t origin)
{
	byte tag;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_PICKUP_DEFENSE].formatString, &tag);

	assert(tag < NUM_DEFENSE_PICKUPS); //mxd. A check in original version.

	flags &= ~CEF_OWNERS_ORIGIN;
	flags |= (CEF_DONT_LINK | CEF_CHECK_OWNER | CEF_VIEWSTATUSCHANGED);
	client_entity_t* ce = ClientEntity_new(type, flags, origin, NULL, 0); //mxd. next_think_time 50 in original logic. Set to 0, so self->nextThinkTime holds previous update time in AmmoPickupThink()...

	VectorCopy(ce->r.origin, ce->origin);
	ce->radius = 10.0f;
	ce->r.model = &defense_models[tag];
	ce->r.flags = RF_GLOW; //mxd. Remove RF_TRANSLUCENT flag and 0.8 alpha (looks broken with those enabled).

	if (tag == ITEM_DEFENSE_TELEPORT)
		ce->r.scale = 1.25f;

	ce->SpawnData = GetPickupBobPhase(origin); //mxd
	ce->Update = DefensePickupThink;

	AddEffect(owner, ce);

	// Add spinning electrical sparks.
	for (int i = 0; i < NUM_DEFENSE_PICKUP_SPARKS; i++)
	{
		client_entity_t* spark = ClientEntity_new(type, flags, origin, NULL, 0); //mxd. next_think_time 50 in original logic. Set to 0, so self->nextThinkTime holds previous update time in AmmoPickupThink()...

		spark->flags |= (CEF_ADDITIVE_PARTS | CEF_ABSOLUTE_PARTS | CEF_VIEWSTATUSCHANGED);
		spark->r.flags = (RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
		spark->r.model = &defense_sparks[tag];
		spark->r.scale = flrand(0.45f, 0.8f); //mxd. Randomize scale a bit.
		spark->radius = (tag == ITEM_DEFENSE_TORNADO ? SPARK_RADIUS * 1.4f : SPARK_RADIUS); //mxd. Bigger radius for ITEM_DEFENSE_TORNADO, so particles don't fly inside pickup model.
		spark->color = color_white; //mxd
		spark->alpha = 0.65f; //mxd. 0.1 in original logic, but with d_alpha 0.5.
		spark->SpawnInfo = tag;

		spark->Update = DefensePickupSparkThink;
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