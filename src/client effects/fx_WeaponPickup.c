//
// fx_WeaponPickup.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Particle.h"
#include "Random.h"
#include "Vector.h"
#include "g_items.h"

#define BOB_HEIGHT			6.0f
#define BOB_SPEED			ANGLE_10
#define WP_PARTICLE_RADIUS	16.0f

static struct model_s* weapon_models[7];

void PreCacheItemWeapons(void)
{
	weapon_models[0] = fxi.RegisterModel("models/items/weapons/hellstaff/tris.fm");	// ITEM_WEAPON_HELLSTAFF
	weapon_models[1] = fxi.RegisterModel("models/items/weapons/array/tris.fm");		// ITEM_WEAPON_MAGICMISSILE
	weapon_models[2] = fxi.RegisterModel("models/items/weapons/bow/tris.fm");		// ITEM_WEAPON_REDRAINBOW
	weapon_models[3] = fxi.RegisterModel("models/items/weapons/sphere/tris.fm");	// ITEM_WEAPON_SPHEREOFANNIHILIATION
	weapon_models[4] = fxi.RegisterModel("models/items/weapons/bow/tris.fm");		// ITEM_WEAPON_PHOENIXBOW
	weapon_models[5] = fxi.RegisterModel("models/items/weapons/maceballs/tris.fm");	// ITEM_WEAPON_MACEBALLS
	weapon_models[6] = fxi.RegisterModel("models/items/weapons/firewall/tris.fm");	// ITEM_WEAPON_FIREWALL
}

static qboolean WeaponPickupThink(struct client_entity_s* self, centity_t* owner)
{
	int part;
	paletteRGBA_t color;

	// Rotate and bob.
	self->r.angles[YAW] += ANGLE_15;
	VectorCopy(owner->current.origin, self->r.origin);
	self->r.origin[2] += cosf(self->SpawnData) * BOB_HEIGHT;
	self->SpawnData += BOB_SPEED;

	switch (self->SpawnInfo)
	{
		case 0: // Hellstaff
		case 2: // Red rain bow
			part = PART_16x16_SPARK_R;
			color.c = 0xff0000ff;
			break;

		case 1: // Magic Missile
			part = PART_16x16_SPARK_I;
			color.c = 0xff00ff00;
			break;

		case 3: // Sphere
			part = PART_16x16_SPARK_B;
			color.c = 0xffff0000;
			break;

		case 4: // Phoenix bow
			part = irand(PART_32x32_FIRE0, PART_32x32_FIRE2);
			color.c = 0xff0080ff;
			break;

		case 5: // Maceballs
			part = PART_16x16_SPARK_G;
			color.c = 0xff00ff00;
			break;

		case 6: // Firewall
			part = irand(PART_16x16_FIRE1, PART_16x16_FIRE3);
			color.c = 0xff0080ff;
			break;

		default: // No effect
			return true;
	}

	if (ref_soft)
		part |= PFL_SOFT_MASK;
	else
		color.c = 0xffffffff;

	client_particle_t* spark = ClientParticle_new(part, color, 500);

	spark->scale = 6.0f;
	spark->origin[0] = cosf(self->SpawnData * 4.0f) * WP_PARTICLE_RADIUS;
	spark->origin[1] = sinf(self->SpawnData * 4.0f) * WP_PARTICLE_RADIUS;
	spark->origin[2] = -cosf(self->SpawnData) * BOB_HEIGHT;
	spark->acceleration[2] = flrand(128.0f, 256.0f);

	AddParticleToList(self, spark);

	return true;
}

void FXWeaponPickup(centity_t* owner, const int type, int flags, vec3_t origin)
{
	byte tag;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_PICKUP_WEAPON].formatString, &tag);

	//mxd. Skip non-pickable weapons.
	if (tag < ITEM_WEAPON_HELLSTAFF)
		return;

	flags &= ~CEF_OWNERS_ORIGIN;
	flags |= CEF_DONT_LINK | CEF_CHECK_OWNER | CEF_ADDITIVE_PARTS | CEF_VIEWSTATUSCHANGED;
	client_entity_t* ce = ClientEntity_new(type, flags, origin, NULL, 50);

	ce->radius = 10.0f;
	ce->SpawnInfo = tag - 2;
	ce->r.model = &weapon_models[ce->SpawnInfo];
	ce->r.flags = RF_TRANSLUCENT | RF_GLOW;
	ce->r.scale = ((tag == ITEM_WEAPON_FIREWALL) ? 1.0f : 0.5f);
	ce->r.skinnum = ((tag == ITEM_WEAPON_PHOENIXBOW) ? 1 : 0);
	ce->alpha = 0.8f;
	ce->Update = WeaponPickupThink;

	AddEffect(owner, ce);
}