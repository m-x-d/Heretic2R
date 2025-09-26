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
	const int duration = (R_DETAIL >= DETAIL_NORMAL ? 500 : 350); //mxd
	client_particle_t* mist = ClientParticle_new(PART_32x32_ALPHA_GLOBE, color, duration);

	VectorCopy(offset, mist->origin);

	mist->scale = 30.0f;
	mist->d_scale = 3.0f;
	mist->d_alpha *= 0.5f;
	mist->acceleration[2] = 0.0f;
	VectorSet(mist->velocity, flrand(-64.0f, 64.0f), flrand(-64.0f, 64.0f), 0.0f);

	AddParticleToList(owner, mist);
}

static qboolean WaterfallBaseSpawner(client_entity_t* spawner, centity_t* owner)
{
#define NUM_SPLASHES	0.005f

	VectorCopy(owner->current.origin, spawner->r.origin);
	const int count = GetScaledCount((int)(spawner->xscale * spawner->yscale * NUM_SPLASHES), 0.8f);
	for (int i = 0; i < count; i++)
		CreateFountainSplash(spawner, spawner->xscale, spawner->yscale, spawner->yaw);

	return true;
}

void FXWaterfallBase(centity_t* owner, const int type, int flags, vec3_t origin)
{
	byte xscale;
	byte yscale;
	byte yaw;

	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_WATERFALLBASE].formatString, &xscale, &yscale, &yaw);

	flags |= CEF_NO_DRAW | CEF_NOMOVE | CEF_VIEWSTATUSCHANGED;
	client_entity_t* wfb = ClientEntity_new(type, flags, origin, NULL, 100);

	wfb->xscale = xscale;
	wfb->yscale = yscale;
	wfb->yaw = (float)yaw * ANGLE_360 / 256.0f;
	wfb->radius = wfb->xscale + wfb->yscale;
	wfb->Update = WaterfallBaseSpawner;

	AddEffect(owner, wfb);
}

static qboolean WaterDropEnd(client_entity_t* waterdrop, centity_t* owner)
{
	CreateFountainSplash(waterdrop, 10.0f, 10.0f, 0.0f);
	waterdrop->Update = RemoveSelfAI;
	waterdrop->nextThinkTime = fxi.cl->time + 500;

	return true;
}

static qboolean FountainParticleSpawner(client_entity_t* spawner, centity_t* owner)
{
#define NUM_FOUNTAIN_PARTICLES	20

	vec3_t origin;
	vec3_t velocity;

	client_particle_t* drop = NULL;
	int time = 0;
	const int count = GetScaledCount(NUM_FOUNTAIN_PARTICLES, 0.9f);

	for (int i = 0; i < count; i++)
	{
		VectorRandomSet(origin, 4.0f);
		VectorRandomCopy(spawner->direction, velocity, 16.0f);

		const float accel = GetGravity();
		const float dist = spawner->SpawnData - origin[2];
		time = (int)(GetTimeToReachDistance(velocity[2], fabsf(accel), fabsf(dist))); //BUGFIX: mxd. GetTimeToReachDistance() expects positive acceleration and distance...

		drop = ClientParticle_new((int)(PART_32x32_WFALL | PFL_NEARCULL), spawner->color, time);

		VectorCopy(origin, drop->origin);
		VectorCopy(velocity, drop->velocity);
		drop->acceleration[2] = accel;
		drop->scale = 8.0f;
		drop->d_scale = 16.0f;
		drop->d_alpha *= 0.8f;
		drop->startTime += irand(-50, 0); //mxd. flrand in original version.

		AddParticleToList(spawner, drop);
	}

	if (drop != NULL) //mxd. Added sanity check.
	{
		GetPositionOverTime(spawner->r.origin, velocity, drop->acceleration, (float)time * 0.001f, origin);

		client_entity_t* splash = ClientEntity_new(-1, 0, origin, NULL, time);
		splash->Update = WaterDropEnd;
		splash->flags = CEF_NOMOVE | CEF_NO_DRAW;
		AddEffect(NULL, splash);
	}

	return true; // Never go away.
}

// Could send the 'v' as a 'ds' but we would lose some accuracy. As it is a persistent effect, it doesn't matter too much.
void FXFountain(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	const int next_think_time = (R_DETAIL >= DETAIL_HIGH ? 50 : 90); //mxd
	client_entity_t* fountain = ClientEntity_new(type, flags, origin, NULL, next_think_time);

	short drop;
	byte frame;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_FOUNTAIN].formatString, &fountain->direction, &drop, &frame);

	fountain->r.frame = frame;

	fountain->SpawnData = (float)drop * 0.125f;
	fountain->color = color_white; //mxd
	fountain->radius = 128.0f + fabsf(fountain->SpawnData);
	fountain->Update = FountainParticleSpawner;
	fountain->flags |= CEF_NO_DRAW | CEF_NOMOVE | CEF_CULLED | CEF_VIEWSTATUSCHANGED;

	AddEffect(owner, fountain);
}