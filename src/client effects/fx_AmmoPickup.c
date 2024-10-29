//
// fx_AmmoPickup.c
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
#include "g_items.h"

#define BOB_HEIGHT		6.0
#define BOB_SPEED		ANGLE_10
#define HEALTH_RADIUS	6.0

#define	NUM_AMMO_MODELS	9
static struct model_s *ammomodels[NUM_AMMO_MODELS];
void PreCacheItemAmmo()
{
	ammomodels[0] = fxi.RegisterModel("models/items/mana/half/tris.fm");	// ITEM_AMMO_MANA_DEFENSIVE_HALF
	ammomodels[1] = fxi.RegisterModel("models/items/mana/full/tris.fm");	// ITEM_AMMO_MANA_DEFENSIVE_FULL
	ammomodels[2] = fxi.RegisterModel("models/items/mana/half/tris.fm");	// ITEM_AMMO_MANA_OFFENSIVE_HALF
	ammomodels[3] = fxi.RegisterModel("models/items/mana/full/tris.fm");	// ITEM_AMMO_MANA_OFFENSIVE_FULL
	ammomodels[4] = fxi.RegisterModel("models/items/mana/combo/tris.fm");		// ITEM_AMMO_MANA_COMBO_QUARTER
	ammomodels[5] = fxi.RegisterModel("models/items/mana/combo/tris.fm");		// ITEM_AMMO_MANA_COMBO_HALF
	ammomodels[6] = fxi.RegisterModel("models/items/ammo/hellstaff/tris.fm");		// ITEM_AMMO_HELLSTAFF
	ammomodels[7] = fxi.RegisterModel("models/items/ammo/redrain/tris.fm");		// ITEM_AMMO_REDRAIN
	ammomodels[8] = fxi.RegisterModel("models/items/ammo/phoenix/tris.fm");		// ITEM_AMMO_PHOENIX
}

// --------------------------------------------------------------

static qboolean FXAmmoPickupThink(struct client_entity_s *self, centity_t *owner)
{

	client_particle_t	*p;
	paletteRGBA_t		color;
	
	// Rotate and bob
	self->r.angles[YAW] += ANGLE_5;
	VectorCopy(owner->current.origin, self->r.origin);
	self->r.origin[2] += (cos(self->SpawnData) * BOB_HEIGHT); 
	self->SpawnData += BOB_SPEED;

	switch(self->SpawnInfo)
	{

	case 0:
	case 1:
		color.g = irand(50, 90);
		color.b = irand(210, 255);
		color.r = color.g;
		break;

	case 2:
	case 3:
		color.r = irand(50, 90);
		color.g = irand(210, 255);
		color.b = color.r;
		break;

	case 4:
	case 5:
		if (irand(0,1))
		{
			color.g = irand(50, 90);
			color.b = irand(210, 255);
			color.r = color.g;
		}
		else
		{
			color.r = irand(50, 90);
			color.g = irand(210, 255);
			color.b = color.r;
		}
		break;

	}

	if (self->SpawnInfo < 6)
	{
		// spawn particles
		color.a = 255;
		p = ClientParticle_new(PART_4x4_WHITE | PFL_SOFT_MASK, color, 600);

		VectorSet(p->origin, flrand(-HEALTH_RADIUS, HEALTH_RADIUS), flrand(-HEALTH_RADIUS, HEALTH_RADIUS), 0.0);
		VectorSet(p->velocity, 0.0, 0.0, flrand(20.0, 40.0));
		p->acceleration[2] = 20.0;
		AddParticleToList(self, p);
	}
	return(true);
}

void FXAmmoPickup(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*ce;
	byte				tag;

	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_PICKUP_AMMO].formatString, &tag);

	flags &= ~CEF_OWNERS_ORIGIN;
	ce = ClientEntity_new(type, flags | CEF_DONT_LINK | CEF_CHECK_OWNER | CEF_VIEWSTATUSCHANGED, origin, NULL, 50);

	VectorCopy(ce->r.origin, ce->origin);
	ce->r.model = ammomodels + tag;

	if (tag==0)		// Blue stuff
		ce->r.skinnum = 1;
	if (tag==1)		// Blue stuff
		ce->r.skinnum = 1;

	ce->r.flags = RF_TRANSLUCENT | RF_GLOW;

	if ((tag == ITEM_AMMO_MANA_COMBO_HALF) || (tag == ITEM_AMMO_MANA_DEFENSIVE_FULL) || 
		(tag == ITEM_AMMO_MANA_OFFENSIVE_FULL))
		ce->r.scale = 1.25;
	else
		ce->r.scale = 1.0;
	ce->radius = 10.0;
	ce->alpha = 0.8;
	ce->Update = FXAmmoPickupThink;
	ce->SpawnInfo = tag;

	AddEffect(owner, ce);
}

// end
