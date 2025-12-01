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
#include "Utilities.h"

#define BOB_HEIGHT			6.0f
#define BOB_SPEED			ANGLE_10
#define MANA_RADIUS_FULL	6.0f
#define MANA_RADIUS_HALF	2.0f //mxd
#define ANIMATION_SPEED		50 //mxd

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

static client_particle_t* InitDefensiveManaParticle(const qboolean is_half) //mxd. Split into separate function.
{
	if (is_half && irand(0, 2)) //mxd
		return NULL;

	paletteRGBA_t color;
	color.g = (byte)irand(90, 120); //mxd. Changed color to more light-blueish.
	color.b = (byte)irand(210, 255);
	color.r = color.g - 30;
	color.a = 255;

	// Spawn particle.
	client_particle_t* p = ClientParticle_new(PART_4x4_WHITE | PFL_SOFT_MASK, color, 600);

	const float radius = (is_half ? MANA_RADIUS_HALF : MANA_RADIUS_FULL);
	VectorSet(p->origin, flrand(-radius, radius), flrand(-radius, radius), 0.0f);

	VectorSet(p->velocity, 0.0f, 0.0f, flrand(20.0f, 40.0f));
	p->acceleration[2] = 20.0f;

	if (is_half)
	{
		Vec3ScaleAssign(0.75f, p->velocity);
		p->acceleration[2] *= 0.75f;
	}

	return p;
}

static client_particle_t* InitOffensiveManaParticle(const qboolean is_half) //mxd. Split into separate function.
{
	if (is_half && irand(0, 2)) //mxd
		return NULL;

	paletteRGBA_t color;
	color.r = (byte)irand(50, 90);
	color.g = (byte)irand(210, 255);
	color.b = color.r;
	color.a = 255;

	// Spawn particle.
	client_particle_t* p = ClientParticle_new(PART_4x4_WHITE | PFL_SOFT_MASK, color, 600);

	const float radius = (is_half ? MANA_RADIUS_HALF : MANA_RADIUS_FULL);
	VectorSet(p->origin, flrand(-radius, radius), flrand(-radius, radius), 0.0f);

	VectorSet(p->velocity, 0.0f, 0.0f, flrand(20.0f, 40.0f));
	p->acceleration[2] = 20.0f;

	if (is_half)
	{
		Vec3ScaleAssign(0.75f, p->velocity);
		p->acceleration[2] *= 0.75f;
	}

	return p;
}

static client_particle_t* InitComboManaParticle(const qboolean is_quarter) //mxd. Split into separate function.
{
	if (is_quarter && irand(0, 2)) //mxd
		return NULL;

	paletteRGBA_t color;

	if (irand(0, 1))
	{
		color.g = (byte)irand(90, 120); //mxd. Changed color to more light-blueish.
		color.b = (byte)irand(210, 255);
		color.r = color.g - 30;
	}
	else
	{
		color.r = (byte)irand(50, 90);
		color.g = (byte)irand(210, 255);
		color.b = color.r;
	}

	color.a = 255;

	// Spawn particle.
	client_particle_t* p = ClientParticle_new(PART_4x4_WHITE | PFL_SOFT_MASK, color, 600);

	const float radius = (is_quarter ? MANA_RADIUS_HALF : MANA_RADIUS_FULL);
	VectorSet(p->origin, flrand(-radius, radius), flrand(-radius, radius), 0.0f);

	VectorSet(p->velocity, 0.0f, 0.0f, flrand(20.0f, 40.0f));
	p->acceleration[2] = 20.0f;

	if (is_quarter)
	{
		Vec3ScaleAssign(0.75f, p->velocity);
		p->acceleration[2] *= 0.75f;
	}

	return p;
}

static client_particle_t* InitHellstaffAmmoParticle(void) //mxd
{
	if (irand(0, 2))
		return NULL;

	paletteRGBA_t color;
	COLOUR_SET(color, (byte)irand(210, 255), 0, 0);

	// Spawn particle.
	client_particle_t* p = ClientParticle_new(PART_4x4_WHITE | PFL_SOFT_MASK, color, 600);

	VectorSet(p->origin, flrand(-MANA_RADIUS_HALF, MANA_RADIUS_HALF), flrand(-MANA_RADIUS_HALF, MANA_RADIUS_HALF), 0.0f);
	VectorSet(p->velocity, flrand(-4.0f, 4.0f), flrand(-4.0f, 4.0f), flrand(-30.0f, -50.0f));
	p->acceleration[2] = flrand(-10.0f, -20.0f);

	return p;
}

static qboolean AmmoPickupThink(struct client_entity_s* self, centity_t* owner)
{
	// Rotate and bob.
	const int step = fx_time - self->nextThinkTime; //mxd
	const float lerp = (float)step / ANIMATION_SPEED; //mxd
	self->LifeTime += step; //mxd

	self->r.angles[YAW] += ANGLE_5 * lerp;
	VectorCopy(owner->origin, self->r.origin); //mxd. Use interpolated origin (to make items dropped by Drop_Item() fly smoothly).
	self->r.origin[2] += (cosf(self->SpawnData) * BOB_HEIGHT);
	self->SpawnData += BOB_SPEED * lerp;

	if (self->SpawnData > ANGLE_360)
		self->SpawnData = fmodf(self->SpawnData, ANGLE_360); //mxd. Otherwise SpawnData will eventually (in ~8 minutes of runtime) cause floating point precision errors...

	//mxd. Spawn particles at original rate.
	if (self->LifeTime < ANIMATION_SPEED)
		return true;

	self->LifeTime %= ANIMATION_SPEED;

	client_particle_t* p;

	switch (self->SpawnInfo)
	{
		case ITEM_AMMO_MANA_DEFENSIVE_HALF:
		case ITEM_AMMO_MANA_DEFENSIVE_FULL:
			p = InitDefensiveManaParticle(self->SpawnInfo == ITEM_AMMO_MANA_DEFENSIVE_HALF);
			break;

		case ITEM_AMMO_MANA_OFFENSIVE_HALF:
		case ITEM_AMMO_MANA_OFFENSIVE_FULL:
			p = InitOffensiveManaParticle(self->SpawnInfo == ITEM_AMMO_MANA_OFFENSIVE_HALF);
			break;

		case ITEM_AMMO_MANA_COMBO_QUARTER: //TODO: spawn green particles from center, blue from "horns"?
		case ITEM_AMMO_MANA_COMBO_HALF:
			p = InitComboManaParticle(self->SpawnInfo == ITEM_AMMO_MANA_COMBO_QUARTER);
			break;

		case ITEM_AMMO_HELLSTAFF: //mxd. Add hellstaff ammo particles, because why not!
			p = InitHellstaffAmmoParticle();
			break;

		default: //mxd
			return true; // No particles for ITEM_AMMO_REDRAIN and ITEM_AMMO_PHOENIX.
	}

	// Add particle?
	if (p != NULL)
		AddParticleToList(self, p);

	return true;
}

void FXAmmoPickup(centity_t* owner, const int type, int flags, vec3_t origin)
{
	byte tag;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_PICKUP_AMMO].formatString, &tag);

	flags &= ~CEF_OWNERS_ORIGIN;
	flags |= (CEF_DONT_LINK | CEF_CHECK_OWNER | CEF_VIEWSTATUSCHANGED);
	client_entity_t* ce = ClientEntity_new(type, flags, origin, NULL, 0); //mxd. next_think_time 50 in original logic. Set to 0, so self->nextThinkTime holds previous update time in AmmoPickupThink()...

	ce->radius = 10.0f;
	ce->r.model = &ammo_models[tag];
	ce->r.flags = RF_GLOW; //mxd. Remove RF_TRANSLUCENT flag and 0.8 alpha (looks broken with those enabled).
	ce->SpawnInfo = tag;

	if (tag == ITEM_AMMO_MANA_DEFENSIVE_HALF || tag == ITEM_AMMO_MANA_DEFENSIVE_FULL) // Blue stuff.
		ce->r.skinnum = 1;

	if (tag == ITEM_AMMO_MANA_COMBO_HALF || tag == ITEM_AMMO_MANA_DEFENSIVE_FULL || tag == ITEM_AMMO_MANA_OFFENSIVE_FULL)
		ce->r.scale = 1.25f;

	ce->SpawnData = GetPickupBobPhase(origin); //mxd
	ce->Update = AmmoPickupThink;

	AddEffect(owner, ce);
}