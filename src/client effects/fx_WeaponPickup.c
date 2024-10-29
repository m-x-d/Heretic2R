//
// fx_WeaponPickup.c
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
#define WP_PART_RADIUS	16.0

#define	NUM_ITEMWEAPONS	7

extern int ref_soft;

static struct model_s *weapon_models[NUM_ITEMWEAPONS];

void PreCacheItemWeapons()
{
	weapon_models[0] = fxi.RegisterModel("models/items/weapons/hellstaff/tris.fm");  // ITEM_WEAPON_HELLSTAFF
	weapon_models[1] = fxi.RegisterModel("models/items/weapons/array/tris.fm");  // ITEM_WEAPON_MAGICMISSILE
	weapon_models[2] = fxi.RegisterModel("models/items/weapons/bow/tris.fm");  // ITEM_WEAPON_REDRAINBOW
	weapon_models[3] = fxi.RegisterModel("models/items/weapons/sphere/tris.fm");  // ITEM_WEAPON_SPHEREOFANNIHILIATION
	weapon_models[4] = fxi.RegisterModel("models/items/weapons/bow/tris.fm");  // ITEM_WEAPON_PHOENIXBOW
	weapon_models[5] = fxi.RegisterModel("models/items/weapons/maceballs/tris.fm");  // ITEM_WEAPON_MACEBALLS
	weapon_models[6] = fxi.RegisterModel("models/items/weapons/firewall/tris.fm");  // ITEM_WEAPON_FIREWALL
}

// --------------------------------------------------------------

static qboolean FXWeaponPickupThink(struct client_entity_s *self, centity_t *owner)
{
	client_particle_t	*spark;
	paletteRGBA_t		color;
	int part;

	// Rotate and bob
	self->r.angles[YAW] += ANGLE_15;
	VectorCopy(owner->current.origin, self->r.origin);
	self->r.origin[2] += (cos(self->SpawnData) * BOB_HEIGHT); 
	self->SpawnData += BOB_SPEED;

	switch(self->SpawnInfo)
	{
	case 0:		// Hellstaff
		part = PART_16x16_SPARK_R;
		color.c = 0xff0000ff;
		break;
	case 1:		// Magic Missile
		part = PART_16x16_SPARK_I;
		color.c = 0xff00ff00;
		break;
	case 2:		// Red rain bow
		part = PART_16x16_SPARK_R;
		color.c = 0xff0000ff;
		break;
	case 3:		// Sphere
		part = PART_16x16_SPARK_B;
		color.c = 0xffff0000;
		break;
	case 4:		// Phoenix bow
		part = irand(PART_32x32_FIRE0, PART_32x32_FIRE2);
		color.c = 0xff0080ff;
		break;
	case 5:		// Maceballs
		part = PART_16x16_SPARK_G;
		color.c = 0xff00ff00;
		break;
	case 6:		// firewall
		part = irand(PART_16x16_FIRE1, PART_16x16_FIRE3);
		color.c = 0xff0080ff;
		break;
	default:
		return true;		// No effect
		break;
	}

	if (ref_soft)
		part |= PFL_SOFT_MASK;
	else
		color.c = 0xffffffff;

	spark = ClientParticle_new(part, color, 500);
	spark->origin[0] = cos(self->SpawnData*4.0) * WP_PART_RADIUS;
	spark->origin[1] = sin(self->SpawnData*4.0) * WP_PART_RADIUS;
	spark->origin[2] = -cos(self->SpawnData) * BOB_HEIGHT;
	spark->acceleration[2] = flrand(128.0, 256.0);
	spark->scale = 6.0;
	AddParticleToList(self, spark);

	return(true);
}

void FXWeaponPickup(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*ce;
	byte				tag;

	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_PICKUP_WEAPON].formatString, &tag);

	flags &= ~CEF_OWNERS_ORIGIN;
	ce = ClientEntity_new(type, flags | CEF_DONT_LINK | CEF_CHECK_OWNER | CEF_ADDITIVE_PARTS | CEF_VIEWSTATUSCHANGED, origin, NULL, 50);

	VectorCopy(ce->r.origin, ce->origin);
	ce->r.flags = RF_TRANSLUCENT | RF_GLOW;
	if(!tag)//sorry bob, just temporary...
		ce->flags|=CEF_NO_DRAW;
	else
		ce->r.model = weapon_models + (tag -2);
	ce->r.scale = 0.5;
	ce->radius = 10.0;
	ce->alpha = 0.8;
	ce->Update = FXWeaponPickupThink;

	if (tag == ITEM_WEAPON_FIREWALL)
		ce->r.scale = 1;

	if (tag == ITEM_WEAPON_PHOENIXBOW)
		ce->r.skinnum = 1;

	ce->SpawnInfo = tag-2;

	AddEffect(owner, ce);
}

// end
