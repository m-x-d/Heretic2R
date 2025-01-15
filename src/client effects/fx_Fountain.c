//
// fx_Fountain.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Particle.h"
#include "Vector.h"
#include "Random.h"
#include "Motion.h"
#include "Utilities.h"
#include "Matrix.h"
#include "g_playstats.h"

static void CreateFountainSplash(client_entity_t* owner, const float xspread, const float yspread, const float angle)
{
	matrix3_t mat;
	CreateYawMatrix(mat, angle);

	vec3_t work;
	VectorSet(work, flrand(-xspread, xspread), flrand(-yspread, yspread), 0.0f);

	vec3_t offset;
	Matrix3MultByVec3(mat, work, offset);

	const paletteRGBA_t color = { .c = 0x40ffffff };
	const int duration = (r_detail->value >= DETAIL_NORMAL ? 500 : 350); //mxd
	client_particle_t* mist = ClientParticle_new(PART_32x32_ALPHA_GLOBE, color, duration);

	VectorCopy(offset, mist->origin);

	mist->scale = 30.0f;
	mist->d_scale = 3.0f;
	mist->d_alpha *= 0.5f;
	mist->acceleration[2] = 0.0f;
	VectorSet(mist->velocity, flrand(-64.0f, 64.0f), flrand(-64.0f, 64.0f), 0.0f);

	AddParticleToList(owner, mist);
}

static qboolean FXWaterfallBaseSpawner(client_entity_t* spawner, const centity_t* owner)
{
#define NUM_SPLASHES	0.005f

	VectorCopy(owner->current.origin, spawner->r.origin);
	const int count = GetScaledCount(Q_ftol(spawner->xscale * spawner->yscale * NUM_SPLASHES), 0.8f);
	for (int i = 0; i < count; i++)
		CreateFountainSplash(spawner, spawner->xscale, spawner->yscale, spawner->yaw);

	return true;
}

void FXWaterfallBase(centity_t *owner, int type, int flags, vec3_t origin)
{
	byte			xs, ys, yaw;
	client_entity_t	*wfb;

	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_WATERFALLBASE].formatString, &xs, &ys, &yaw);

	flags |= CEF_NO_DRAW | CEF_NOMOVE | CEF_VIEWSTATUSCHANGED;
	wfb = ClientEntity_new(type, flags, origin, NULL, 100);

	wfb->xscale = xs;
	wfb->yscale = ys;
	wfb->yaw = (yaw * ANGLE_360) / 256.0;
	wfb->radius = wfb->xscale + wfb->yscale;
	wfb->Update = FXWaterfallBaseSpawner;

	AddEffect(owner, wfb); 
}

// -----------------------------------------------------------------------------------------

qboolean FXWaterDropEnd(client_entity_t *waterdrop, centity_t *owner)
{
	CreateFountainSplash(waterdrop, 10.0, 10.0, 0);
	waterdrop->Update = RemoveSelfAI;
	waterdrop->nextThinkTime = fxi.cl->time + 500;
	return(true);
}

#define	FOUNTAIN_SCALE	80.0F
#define NUM_FOUNT_PARTS	20

static qboolean FXFountainParticleSpawner(client_entity_t *spawner, centity_t *owner)
{
	client_particle_t	*drop;
	client_entity_t		*splash;
	vec3_t				origin;
	vec3_t				velocity;
	vec_t				accel;
	int					i, time, count;
	float				s;

	count = GetScaledCount(NUM_FOUNT_PARTS, 0.9);
	for(i = 0; i < count; i++)
	{
		VectorSet(origin, flrand(-4.0F, 4.0F), flrand(-4.0F, 4.0F), flrand(-4.0F, 4.0F));
		VectorRandomCopy(spawner->direction, velocity, 16.0F);

		accel = GetGravity();
		s = spawner->SpawnData - origin[2];
		time = GetTimeToReachDistance(velocity[2], accel, s);

		drop = ClientParticle_new(PART_32x32_WFALL | PFL_NEARCULL, spawner->color, time);

		VectorCopy(origin, drop->origin);
		VectorCopy(velocity, drop->velocity);
		drop->acceleration[2] = accel;
		drop->scale = 8.0F;
		drop->d_scale = 16.0F;
		drop->d_alpha *= 0.8;
		drop->startTime += flrand(-50.0F, 0.0F);

		AddParticleToList(spawner, drop); 
	}

	GetPositionOverTime(spawner->r.origin, velocity, drop->acceleration, time * 0.001, origin);
	splash = ClientEntity_new(-1, 0, origin, NULL, time);
	splash->Update = FXWaterDropEnd;
	splash->flags = CEF_NOMOVE | CEF_NO_DRAW;
	AddEffect(NULL, splash);

	return(true);				// Never go away
}

// Could send the 'v' as a 'ds' but we would lose some accuracy. As it
// is a persistant effect, it doesn`t matter too much

void FXFountain(centity_t *Owner, int Type, int Flags, vec3_t Origin)
{
	client_entity_t		*fountain;
	byte				frame;
	short				drop;

	if (r_detail->value >= DETAIL_HIGH)
		fountain = ClientEntity_new(Type, Flags, Origin, NULL, 50);
	else
		fountain = ClientEntity_new(Type, Flags, Origin, NULL, 90);

	fxi.GetEffect(Owner, Flags, clientEffectSpawners[FX_FOUNTAIN].formatString, &fountain->direction, &drop, &frame);

	fountain->r.frame = frame;

	fountain->SpawnData = drop * 0.125;
	fountain->color.c = 0xffffffff;
	fountain->radius = 128.0F + Q_fabs(fountain->SpawnData);
	fountain->Update = FXFountainParticleSpawner;
	fountain->flags |= CEF_NO_DRAW | CEF_NOMOVE | CEF_CULLED | CEF_VIEWSTATUSCHANGED;	// | CEF_ADDITIVE_PARTS;

	AddEffect(Owner, fountain); 
}

// end
