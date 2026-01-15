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
#include "Utilities.h"

#define BOB_HEIGHT			6.0f
#define BOB_SPEED			ANGLE_10
#define WP_PARTICLE_RADIUS	16.0f
#define ANIMATION_SPEED		50 //mxd

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

static qboolean WeaponPickupUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXWeaponPickupThink' in original logic.
{
	// Rotate and bob.
	const int step = fx_time - self->nextThinkTime; //mxd
	const float lerp = (float)step / ANIMATION_SPEED; //mxd
	self->LifeTime += step; //mxd

	self->r.angles[YAW] += ANGLE_15 * lerp;
	VectorCopy(owner->origin, self->r.origin); //mxd. Use interpolated origin (to make items dropped by Drop_Item() fly smoothly).
	self->r.origin[2] += cosf(self->SpawnData) * BOB_HEIGHT;
	self->SpawnData += BOB_SPEED * lerp;

	if (self->SpawnData > ANGLE_360)
		self->SpawnData = fmodf(self->SpawnData, ANGLE_360); //mxd. Otherwise SpawnData will eventually (in ~8 minutes of runtime) cause floating point precision errors...

	//mxd. Spawn particles at original rate.
	if (self->LifeTime < ANIMATION_SPEED)
		return true;

	self->LifeTime %= ANIMATION_SPEED;

	int part;
	paletteRGBA_t color;

	switch (self->SpawnInfo)
	{
		case ITEM_WEAPON_HELLSTAFF: // Hellstaff.
		case ITEM_WEAPON_REDRAINBOW: // Red rain bow.
			part = PART_16x16_SPARK_R;
			color.c = 0xff0000ff;
			break;

		case ITEM_WEAPON_MAGICMISSILE: // Magic Missile.
			part = PART_16x16_SPARK_I;
			color.c = 0xff00ff00;
			break;

		case ITEM_WEAPON_SPHEREOFANNIHILATION: // Sphere.
			part = PART_16x16_SPARK_B;
			color.c = 0xffff0000;
			break;

		case ITEM_WEAPON_PHOENIXBOW: // Phoenix bow.
			part = irand(PART_32x32_FIRE0, PART_32x32_FIRE2);
			color.c = 0xff0080ff;
			break;

		case ITEM_WEAPON_MACEBALLS: // Maceballs.
			part = PART_16x16_SPARK_G;
			color.c = 0xff00ff00;
			break;

		case ITEM_WEAPON_FIREWALL: // Firewall.
			part = irand(PART_16x16_FIRE1, PART_16x16_FIRE3);
			color.c = 0xff0080ff;
			break;

		default: // No effect
			return true;
	}

	if (ref_soft)
		part |= PFL_SOFT_MASK;
	else
		color.c = color_white.c;

	client_particle_t* spark = ClientParticle_new(part, color, 500);

	spark->scale = flrand(5.0f, 7.0f); //mxd. Randomize scale a bit.
	spark->origin[0] = cosf(self->SpawnData * 4.0f) * WP_PARTICLE_RADIUS;
	spark->origin[1] = sinf(self->SpawnData * 4.0f) * WP_PARTICLE_RADIUS;
	spark->origin[2] = -cosf(self->SpawnData) * BOB_HEIGHT * 1.8f; //mxd. Scale a bit to make the effect more pronounced.
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
	flags |= (CEF_DONT_LINK | CEF_CHECK_OWNER | CEF_ADDITIVE_PARTS | CEF_VIEWSTATUSCHANGED);
	client_entity_t* ce = ClientEntity_new(type, flags, origin, NULL, 0); //mxd. next_think_time 50 in original logic. Set to 0, so self->nextThinkTime holds previous update time in AmmoPickupThink()...

	ce->radius = 10.0f;
	ce->SpawnInfo = tag;
	ce->r.model = &weapon_models[tag - 2];
	ce->r.flags = RF_GLOW; //mxd. Remove RF_TRANSLUCENT flag and 0.8 alpha (looks broken with those enabled).
	ce->r.scale = ((tag == ITEM_WEAPON_FIREWALL) ? 1.0f : 0.5f);
	ce->r.skinnum = ((tag == ITEM_WEAPON_PHOENIXBOW) ? 1 : 0);

	ce->SpawnData = GetPickupBobPhase(origin); //mxd
	ce->Update = WeaponPickupUpdate;

	AddEffect(owner, ce);
}