//
// fx_meteorbarrier.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Particle.h"
#include "Random.h"
#include "Vector.h"
#include "Utilities.h"
#include "ce_DLight.h"
#include "fx_debris.h"
#include "g_playstats.h"

#define NUM_METEOR_CHUNKS		12
#define METEOR_DELTA_FORWARD	6.0f
#define METEOR_TRAIL_ALPHA		0.8f
#define METEOR_ROLL_SPEED		13.0f
#define METEOR_YAW_SPEED		10.0f

static const paletteRGBA_t meteor_dlight_color = { .r = 63, .g = 255, .b = 77, .a = 255 }; //mxd
static struct model_s* meteor_model;

void PreCacheMeteor(void)
{
	meteor_model = fxi.RegisterModel("models/spells/meteorbarrier/tris.fm");
}

static qboolean MeteorBarrierTrailThink(struct client_entity_s* self, centity_t* owner)
{
	// We theoretically shouldn't need to do this. Just in case.
	if (!(owner->flags & CF_INUSE))
		return false;

	// No trails in low detail mode.
	if (R_DETAIL == DETAIL_LOW)
		return true;

	// Length of trail.
	vec3_t delta;
	VectorSubtract(self->origin, self->r.origin, delta);

	// Number of trails to render.
	const float delta_scaler = (R_DETAIL >= DETAIL_HIGH ? 1.0f : 1.5f);
	const float length = VectorLength(delta) / (METEOR_DELTA_FORWARD * delta_scaler);

	// Set start
	vec3_t org;
	VectorCopy(self->r.origin, org);

	// Work out increment between trails.
	Vec3ScaleAssign(1.0f / length, delta);

	// Work out number of bits.
	int num_trails = Q_ftol(length);
	num_trails = max(1, num_trails);
	float alpha = METEOR_TRAIL_ALPHA;

	VectorCopy(self->r.origin, self->origin);

	if (num_trails > 10)
		return true;

	for (int i = 0; i < num_trails; i++)
	{
		client_particle_t* ce = ClientParticle_new(PART_16x16_SPARK_G, self->r.color, 400);

		ce->scale = 13.0f;
		ce->d_scale = -10.0f;
		ce->color.a = (byte)(alpha * 255.0f);
		VectorCopy(org, ce->origin);

		AddParticleToList(self, ce);

		Vec3AddAssign(delta, org);
		alpha *= 0.9f;
	}

	return true;
}

// Putting the angular velocity in here saves 3 bytes of net traffic per meteor per server frame.
static qboolean MeteorAddToView(client_entity_t* current, centity_t* owner)
{
	const float d_time = (float)(fxi.cl->time - current->startTime);
	current->r.angles[ROLL] = d_time * 0.001f * METEOR_ROLL_SPEED;
	current->r.angles[YAW] =  d_time * 0.001f * METEOR_YAW_SPEED;

	const float angle = (((float)fxi.cl->time * 0.15f) + (current->SpawnData * 90.0f)) * ANGLE_TO_RAD;
	current->r.origin[0] = cosf(angle) * 30.0f;
	current->r.origin[1] = sinf(angle) * 30.0f;
	current->r.origin[2] = cosf(angle / (M_PI / 5.0f)) * 10.0f;

	VectorAdd(owner->origin, current->r.origin, current->r.origin);

	return true;
}

void FXMeteorBarrier(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	// Add a fiery trail effect.
	client_entity_t* trail = ClientEntity_new(type, flags | CEF_ABSOLUTE_PARTS | CEF_ADDITIVE_PARTS | CEF_VIEWSTATUSCHANGED, origin, NULL, 50);

	trail->r.model = &meteor_model;
	trail->SpawnData = (float)((flags & (CEF_FLAG6 | CEF_FLAG7)) >> 5);
	trail->radius = 10.0f;
	trail->AddToView = MeteorAddToView;
	trail->Update = MeteorBarrierTrailThink;

	if (R_DETAIL >= DETAIL_NORMAL)
		trail->dlight = CE_DLight_new(meteor_dlight_color, 150.0f, 0.0f);

	AddEffect(owner, trail);
}

void FXMeteorBarrierTravel(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	// Add a fiery trail effect.
	client_entity_t* trail = ClientEntity_new(type, flags | CEF_ABSOLUTE_PARTS | CEF_ADDITIVE_PARTS | CEF_VIEWSTATUSCHANGED, origin, NULL, 50);

	trail->r.model = &meteor_model;
	trail->radius = 10.0f;
	trail->AddToView = LinkedEntityUpdatePlacement;
	trail->Update = MeteorBarrierTrailThink;

	if (R_DETAIL >= DETAIL_NORMAL)
		trail->dlight = CE_DLight_new(meteor_dlight_color, 150.0f, 0.0f);

	AddEffect(owner, trail);
}

void FXMeteorBarrierExplode(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	vec3_t dir;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_SPELL_METEORBARRIEREXPLODE].formatString, dir);
	Vec3ScaleAssign(32.0f, dir);

	if (flags & CEF_FLAG6)
	{
		// Impact explosion: throw off chunks of glowing meteor-rock and puffs of green smoke.
		const vec3_t debris_dir = { 0.0f, 0.0f, 1.0f };
		const vec3_t mins = { 2.0f, 2.0f, 2.0f }; // Because SpawnChunks needs a value for bounding box.

		// Clear out cef_flag# stuff, means different stuff to debris.
		FXDebris_SpawnChunks(type, flags & ~(CEF_FLAG6 | CEF_FLAG7 | CEF_FLAG8), origin, 5, MAT_GREYSTONE, debris_dir, 80000.0f, mins, 1.0f, false);
	}

	client_entity_t* smoke_puff = ClientEntity_new(type, (int)(flags | CEF_NO_DRAW | CEF_ADDITIVE_PARTS), origin, NULL, 500);
	smoke_puff->radius = 10.0f;

	if (R_DETAIL > DETAIL_LOW)
		smoke_puff->dlight = CE_DLight_new(meteor_dlight_color, 150.0f, 0.0f);

	AddEffect(NULL, smoke_puff);

	const int count = GetScaledCount(NUM_METEOR_CHUNKS, 0.4f);
	for (int i = 0; i < count; i++)
	{
		client_particle_t* ce = ClientParticle_new(PART_16x16_SPARK_G, smoke_puff->r.color, 500);

		ce->scale = flrand(11.0f, 15.0f);
		ce->d_scale = -10.0f;
		VectorRandomCopy(dir, ce->velocity, 64.0f);
		ce->acceleration[2] = GetGravity() * 0.3f;

		AddParticleToList(smoke_puff, ce);
	}
}