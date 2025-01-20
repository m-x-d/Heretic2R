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

static qboolean FXMeteorBarriertrailThink(struct client_entity_s* self, const centity_t* owner)
{
	// We theoretically shouldn't need to do this. Just in case.
	if (!(owner->flags & CF_INUSE))
		return false;

	// No trails in low detail mode.
	if ((int)r_detail->value == DETAIL_LOW)
		return true;

	// Length of trail.
	vec3_t delta;
	VectorSubtract(self->origin, self->r.origin, delta);

	// Number of trails to render.
	const float delta_scaler = (r_detail->value >= DETAIL_HIGH ? 1.0f : 1.5f);
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
static qboolean MeteorAddToView(client_entity_t* current, const centity_t* owner)
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

void FXMeteorBarrier(centity_t* owner, const int type, const int flags, const vec3_t origin)
{
	// Add a fiery trail effect.
	client_entity_t* trail = ClientEntity_new(type, flags | CEF_ABSOLUTE_PARTS | CEF_ADDITIVE_PARTS | CEF_VIEWSTATUSCHANGED, origin, NULL, 50);

	trail->r.model = &meteor_model;
	trail->SpawnData = (float)((flags & (CEF_FLAG6 | CEF_FLAG7)) >> 5);
	trail->radius = 10.0f;
	trail->AddToView = MeteorAddToView;
	trail->Update = FXMeteorBarriertrailThink;

	if (r_detail->value >= DETAIL_NORMAL)
		trail->dlight = CE_DLight_new(meteor_dlight_color, 150.0f, 0.0f);

	AddEffect(owner, trail);
}

void FXMeteorBarrierTravel(centity_t* owner, const int type, const int flags, const vec3_t origin)
{
	// Add a fiery trail effect.
	client_entity_t* trail = ClientEntity_new(type, flags | CEF_ABSOLUTE_PARTS | CEF_ADDITIVE_PARTS | CEF_VIEWSTATUSCHANGED, origin, NULL, 50);

	trail->r.model = &meteor_model;
	trail->radius = 10.0f;
	trail->AddToView = LinkedEntityUpdatePlacement;
	trail->Update = FXMeteorBarriertrailThink;

	if (r_detail->value >= DETAIL_NORMAL)
		trail->dlight = CE_DLight_new(meteor_dlight_color, 150.0f, 0.0f);

	AddEffect(owner, trail);
}

void FXMeteorBarrierExplode(centity_t *owner, int type, int flags, vec3_t origin)
{
	vec3_t			dir, mins;
	int				i;
	client_entity_t	*smokepuff;
	paletteRGBA_t	LightColor = {64, 255, 76, 255};
	int				count;
	client_particle_t	*ce;

	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_SPELL_METEORBARRIEREXPLODE].formatString, dir);
	Vec3ScaleAssign(32.0, dir);

	if(flags&CEF_FLAG6)
	{
		// Impact explosion: throw off chunks of glowing meteor-rock and puffs of green smoke.
		vec3_t			DebrisDir;

		VectorSet(DebrisDir, 0.0, 0.0, 1.0);
		VectorSet(mins, 2.0, 2.0, 2.0);	// because SpawnChunks needs a value for bounding box
		//clear out cef_flag# stuff, means different stuff to debris
		FXDebris_SpawnChunks(type, flags & ~(CEF_FLAG6|CEF_FLAG7|CEF_FLAG8), origin, 5, MAT_GREYSTONE, DebrisDir, 80000.0f, mins, 1.0, false);
	}

	smokepuff = ClientEntity_new(type, flags|CEF_NO_DRAW | CEF_ADDITIVE_PARTS, origin, 0, 500);
	smokepuff->radius = 10.0;
	if(r_detail->value != DETAIL_LOW)
		smokepuff->dlight = CE_DLight_new(LightColor, 150.0, 0.0);

	AddEffect(NULL, smokepuff);

	count = GetScaledCount(NUM_METEOR_CHUNKS, 0.4);
	for(i = 0; i < count; i++)
	{
		ce = ClientParticle_new(PART_16x16_SPARK_G, smokepuff->r.color, 500);
		ce->scale = flrand(11,15);
		ce->d_scale = -10.0;
		VectorRandomCopy(dir, ce->velocity, 64.0);
		ce->acceleration[2] = GetGravity() * 0.3;
		AddParticleToList(smokepuff, ce);
	}
}
// end