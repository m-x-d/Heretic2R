//
// fx_DefensePickup.c
//
// Heretic II
// Copyright 1998 Raven Software
//

#include "ce_DefaultMessageHandler.h"
#include "Client Effects.h"
#include "Client Entities.h"
#include "Particle.h"
#include "ResourceManager.h"
#include "FX.h"
#include "Vector.h"
#include "Random.h"
#include "Utilities.h"
#include "Angles.h"
#include "ce_Dlight.h"
#include "g_items.h"

#define BOB_HEIGHT				6.0
#define BOB_SPEED				ANGLE_10
#define HEALTH_RADIUS			6.0
#define NUM_DEF_PICKUP_SPARKS	4
#define EGG_TRAIL_DELAY			100
#define EGG_RADIUS				10.0
#define	NUM_ITEMDEFENSE			12
#define SPARK_OFFSET			6
#define EGG_INTENSITY			255

static struct model_s *defense_models[NUM_ITEMDEFENSE];
void PreCacheItemDefense()
{	
	defense_models[0] = fxi.RegisterModel("models/items/defense/repulsion/tris.fm");		// ITEM_DEFENSE_REPULSION
	defense_models[1] = fxi.RegisterModel("models/items/defense/meteorbarrier/tris.fm");	// ITEM_DEFENSE_METEORBARRIER
	defense_models[2] = fxi.RegisterModel("models/items/defense/polymorph/tris.fm");		// ITEM_DEFENSE_POLYMORPH
	defense_models[3] = fxi.RegisterModel("models/items/defense/teleport/tris.fm");			// ITEM_DEFENSE_TELEPORT
	defense_models[4] = fxi.RegisterModel("models/items/defense/lightshield/tris.fm");		// ITEM_DEFENSE_SHIELD
	defense_models[5] = fxi.RegisterModel("models/items/defense/tornado/tris.fm");			// ITEM_DEFENSE_TORNADO
	defense_models[6] = fxi.RegisterModel("sprites/spells/spark_cyan.sp2");					// cyan spark
	defense_models[7] = fxi.RegisterModel("sprites/spells/meteorbarrier.sp2");				// Meteor cloud
	defense_models[8] = fxi.RegisterModel("sprites/spells/spark_green.sp2");				// green spark
	defense_models[9] = fxi.RegisterModel("sprites/spells/spark_red.sp2");					// red spark
	defense_models[10] = fxi.RegisterModel("sprites/spells/spark_blue.sp2");					// blue spark
	defense_models[11] = fxi.RegisterModel("sprites/spells/spark_blue.sp2");					// blue spark
}


static qboolean FXEggSparkThink(struct client_entity_s *shield,centity_t *owner)
{
	vec3_t angvect;
	vec3_t	origin = {0,0,0};

	VectorCopy(shield->origin, origin);
	origin[2] = shield->origin[2] + (cos(shield->d_scale2) * BOB_HEIGHT); 
	shield->d_scale2 += BOB_SPEED;
	
	// Update the angle of the spark.
	VectorMA(shield->direction, (float)(fxi.cl->time-shield->lastThinkTime)/1000.0, shield->velocity2, shield->direction);

	// Update the position of the spark.
	AngleVectors(shield->direction, angvect, NULL, NULL);
	VectorMA(origin, shield->radius, angvect, shield->r.origin);

	shield->lastThinkTime = fxi.cl->time;
	return true;
}

// --------------------------------------------------------------

static qboolean FXDefensePickupThink(struct client_entity_s *self, centity_t *owner)
{

	// Rotate and bob
	self->r.angles[YAW] += ANGLE_5;
	VectorCopy(owner->current.origin, self->r.origin);
	self->r.origin[2] += (cos(self->SpawnData) * BOB_HEIGHT); 
	self->SpawnData += BOB_SPEED;

	return(true);
}

// Create effect FX_PICKUP_DEFENSE
void FXDefensePickup(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*ce;
	byte				tag;

	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_PICKUP_DEFENSE].formatString, &tag);

	flags &= ~CEF_OWNERS_ORIGIN;
	ce = ClientEntity_new(type, flags | CEF_DONT_LINK | CEF_CHECK_OWNER | CEF_VIEWSTATUSCHANGED, origin, NULL, 50);

	VectorCopy(ce->r.origin, ce->origin);
	ce->r.flags = RF_TRANSLUCENT | RF_GLOW;
	ce->r.model = defense_models + tag;
	ce->r.scale = 1.0;

	if (tag == ITEM_DEFENSE_TELEPORT)
		ce->r.scale = 1.25;

	ce->radius = 10.0;
	ce->alpha = 0.8;
	ce->Update = FXDefensePickupThink;

	AddEffect(owner, ce);

	// if we are looking at the polymorph egg, put a special effect around it
	// stolen wholesale from Pats Shield Effect. Cheers Pat.
	if (tag < SPARK_OFFSET)
	{
		client_entity_t	*shield;
		int				i;
		vec3_t			angvect;

		// Add spinning electrical sparks
		for(i=0; i<NUM_DEF_PICKUP_SPARKS; i++)
		{
			shield=ClientEntity_new(type, flags, origin, 0, 50);
			shield->flags |= CEF_ADDITIVE_PARTS | CEF_ABSOLUTE_PARTS | CEF_VIEWSTATUSCHANGED;
			shield->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
			shield->r.model = defense_models + SPARK_OFFSET + tag;
			shield->radius = EGG_RADIUS;
			shield->color.c = 0xffffffff;
			shield->alpha = 0.1;
			shield->d_alpha = 0.5;
			if (tag == 1)
			{
				shield->r.scale = 0.2;
			}
			else
			{
				shield->r.scale = 0.8;
			}

			shield->SpawnData = tag;
			
			shield->Update = FXEggSparkThink;
			VectorCopy(shield->r.origin, shield->origin);

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

			shield->SpawnDelay = fxi.cl->time + EGG_TRAIL_DELAY;
			shield->lastThinkTime = fxi.cl->time;

			AngleVectors(shield->direction, angvect, NULL, NULL);

			AddEffect(owner, shield);
		}
	}

}

// end
