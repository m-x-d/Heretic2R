//
// fx_AmmoPickup.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Particle.h"
#include "Vector.h"
#include "Random.h"
#include "g_items.h"

#define BOB_HEIGHT		6.0f
#define BOB_SPEED		ANGLE_10
#define MANA_RADIUS		6.0f

static struct model_s* ammo_models[9];

void PreCacheItemAmmo(void)
{
	ammo_models[0] = fxi.RegisterModel("models/items/mana/half/tris.fm");		// ITEM_AMMO_MANA_DEFENSIVE_HALF
	ammo_models[1] = fxi.RegisterModel("models/items/mana/full/tris.fm");		// ITEM_AMMO_MANA_DEFENSIVE_FULL
	ammo_models[2] = fxi.RegisterModel("models/items/mana/half/tris.fm");		// ITEM_AMMO_MANA_OFFENSIVE_HALF
	ammo_models[3] = fxi.RegisterModel("models/items/mana/full/tris.fm");		// ITEM_AMMO_MANA_OFFENSIVE_FULL
	ammo_models[4] = fxi.RegisterModel("models/items/mana/combo/tris.fm");		// ITEM_AMMO_MANA_COMBO_QUARTER
	ammo_models[5] = fxi.RegisterModel("models/items/mana/combo/tris.fm");		// ITEM_AMMO_MANA_COMBO_HALF
	ammo_models[6] = fxi.RegisterModel("models/items/ammo/hellstaff/tris.fm");	// ITEM_AMMO_HELLSTAFF
	ammo_models[7] = fxi.RegisterModel("models/items/ammo/redrain/tris.fm");	// ITEM_AMMO_REDRAIN
	ammo_models[8] = fxi.RegisterModel("models/items/ammo/phoenix/tris.fm");	// ITEM_AMMO_PHOENIX
}

static qboolean FXAmmoPickupThink(struct client_entity_s* self, const centity_t* owner)
{
	paletteRGBA_t color;

	// Rotate and bob.
	self->r.angles[YAW] += ANGLE_5;
	VectorCopy(owner->current.origin, self->r.origin);
	self->r.origin[2] += (cosf(self->SpawnData) * BOB_HEIGHT);
	self->SpawnData += BOB_SPEED;

	switch (self->SpawnInfo)
	{
		case 0:
		case 1:
			color.g = (byte)irand(50, 90);
			color.b = (byte)irand(210, 255);
			color.r = color.g;
			break;

		case 2:
		case 3:
			color.r = (byte)irand(50, 90);
			color.g = (byte)irand(210, 255);
			color.b = color.r;
			break;

		case 4:
		case 5:
			if (irand(0, 1))
			{
				color.g = (byte)irand(50, 90);
				color.b = (byte)irand(210, 255);
				color.r = color.g;
			}
			else
			{
				color.r = (byte)irand(50, 90);
				color.g = (byte)irand(210, 255);
				color.b = color.r;
			}
			break;

		default: //mxd
			return true; // No particles for ITEM_AMMO_HELLSTAFF, ITEM_AMMO_REDRAIN and ITEM_AMMO_PHOENIX.
	}

	// Spawn particles.
	color.a = 255;
	client_particle_t* p = ClientParticle_new(PART_4x4_WHITE | PFL_SOFT_MASK, color, 600);

	VectorSet(p->origin, flrand(-MANA_RADIUS, MANA_RADIUS), flrand(-MANA_RADIUS, MANA_RADIUS), 0.0f);
	VectorSet(p->velocity, 0.0f, 0.0f, flrand(20.0f, 40.0f));
	p->acceleration[2] = 20.0f;
	AddParticleToList(self, p);

	return true;
}

void FXAmmoPickup(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*ce;
	byte				tag;

	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_PICKUP_AMMO].formatString, &tag);

	flags &= ~CEF_OWNERS_ORIGIN;
	ce = ClientEntity_new(type, flags | CEF_DONT_LINK | CEF_CHECK_OWNER | CEF_VIEWSTATUSCHANGED, origin, NULL, 50);

	VectorCopy(ce->r.origin, ce->origin);
	ce->r.model = ammo_models + tag;

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
