//
// fx_meteorbarrier.c
//
// Heretic II
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Client Entities.h"
#include "Particle.h"
#include "ResourceManager.h"
#include "FX.h"
#include "Vector.h"
#include "ce_DLight.h"
#include "random.h"
#include "fx_debris.h"
#include "Utilities.h"
#include "g_playstats.h"

#define	NUM_METEOR_MODELS	1

static struct model_s *meteor_models[NUM_METEOR_MODELS];

void PreCacheMeteor()
{
	meteor_models[0] = fxi.RegisterModel("models/spells/meteorbarrier/tris.fm");
}

// -------------------------------------------------

#define NUM_METEOR_CHUNKS		12
#define METEOR_DELTA_FORWARD	6.0
#define METEOR_TRAIL_ALPHA		0.8
#define METEOR_ROLL_SPEED		13.0
#define METEOR_YAW_SPEED		10.0

static qboolean FXMeteorBarriertrailThink(struct client_entity_s *self, centity_t *owner)
{
	
	vec3_t			org, delta;
	float			length, alpha;
	int				numtrails;
	client_particle_t	*ce;

	// we theoretically shouldn't need to do this.. Just in case.
	if (!(owner->flags & CF_INUSE))
		return(false);

	// no trails in low detail mode
	if (r_detail->value == DETAIL_LOW)
		return(true);

	// Length of trail
	VectorSubtract(self->origin, self->r.origin, delta);
	// Number of trails to render
	if (r_detail->value >= DETAIL_HIGH)
		length = VectorLength(delta) / (METEOR_DELTA_FORWARD);
	else
		length = VectorLength(delta) / (METEOR_DELTA_FORWARD * 1.5);
	// Set start
	VectorCopy(self->r.origin, org);
	// Work out increment between trails
	Vec3ScaleAssign(1.0 / length, delta);
	// Work out number of bits
	numtrails = Q_ftol(length);
	alpha = METEOR_TRAIL_ALPHA;

	VectorCopy(self->r.origin, self->origin);

	if (numtrails>10)
		return true;
	if (numtrails <= 0)
		numtrails = 1;
	while(numtrails-->0)
	{
		ce = ClientParticle_new(PART_16x16_SPARK_G, self->r.color, 400);
		ce->scale = 13;
		ce->d_scale = -10.0;
		ce->color.a = 255*alpha;
		VectorCopy(org, ce->origin);
		AddParticleToList(self, ce);

		Vec3AddAssign(delta, org);
		alpha *= 0.9;
	}
	return(true);

}

// Putting the angular velocity in here saves 3 bytes of net traffic
// per meteor per server frame

qboolean MeteorAddToView(client_entity_t *current, centity_t *owner)
{
	float	roll, yaw;
	int		d_time;
	float	Angle;

	d_time = fxi.cl->time - current->startTime;
	roll = d_time * 0.001 * METEOR_ROLL_SPEED;
	yaw = d_time * 0.001 * METEOR_YAW_SPEED;
	current->r.angles[ROLL] = roll;
	current->r.angles[YAW] = yaw;

	Angle = ((fxi.cl->time * .1500) + (90.0 * current->SpawnData)) * ANGLE_TO_RAD;
	current->r.origin[0] = cos(Angle) * 30.0;
	current->r.origin[1] = sin(Angle) * 30.0;
	current->r.origin[2] = cos(Angle / (M_PI / 5)) * 10.0;

	VectorAdd(owner->origin, current->r.origin, current->r.origin);
	return(true);
}

void FXMeteorBarrier(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t	*trail;
	paletteRGBA_t	lightcolor = {63, 255, 77, 255};

	// Add a fiery trail effect.
	trail = ClientEntity_new(type, flags|CEF_ABSOLUTE_PARTS|CEF_ADDITIVE_PARTS|CEF_VIEWSTATUSCHANGED, origin, 0, 50);

	trail->Update = FXMeteorBarriertrailThink;
	trail->r.model = meteor_models;
	trail->AddToView = MeteorAddToView;
	trail->SpawnData = (flags & (CEF_FLAG6|CEF_FLAG7)) >> 5;
	trail->radius = 10.0;
	trail->r.color.c = -1;
	if (r_detail->value >= DETAIL_NORMAL)
		trail->dlight = CE_DLight_new(lightcolor, 150.0, 0.0);

	AddEffect(owner, trail);
}

void FXMeteorBarrierTravel(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t	*trail;
	paletteRGBA_t	lightcolor = {63, 255, 77, 255};

	// Add a fiery trail effect.
	trail = ClientEntity_new(type, flags|CEF_ABSOLUTE_PARTS|CEF_ADDITIVE_PARTS|CEF_VIEWSTATUSCHANGED, origin, 0, 50);

	trail->Update = FXMeteorBarriertrailThink;
	trail->r.model = meteor_models;
	trail->AddToView = LinkedEntityUpdatePlacement;
	trail->radius = 10.0;
	trail->r.color.c = -1;
	if (r_detail->value >= DETAIL_NORMAL)
		trail->dlight = CE_DLight_new(lightcolor, 150.0, 0.0);

	AddEffect(owner, trail);
}



// -------------------------------------------------------

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