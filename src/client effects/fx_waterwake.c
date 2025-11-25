//
// fx_waterwake.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Random.h"
#include "Particle.h"
#include "Vector.h"
#include "g_playstats.h"

static struct model_s* wake_models[2];

void PreCacheWake(void)
{
	wake_models[0] = fxi.RegisterModel("sprites/fx/wake_add.sp2");
	wake_models[1] = fxi.RegisterModel("sprites/fx/wfall.sp2");
}

void FXWaterWake(centity_t* owner, int type, const int flags, vec3_t origin)
{
	short creator_ent_num;
	byte b_yaw;
	vec3_t velocity;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_WATER_WAKE].formatString, &creator_ent_num, &b_yaw, velocity);

	const float yaw = (float)b_yaw * BYTEANGLE_TO_RAD;
	const centity_t* creator = &fxi.server_entities[creator_ent_num];

	// Create a water wake.
	client_entity_t* wake = ClientEntity_new(FX_WATER_WAKE, flags, origin, vec3_up, 5000);

	wake->r.model = &wake_models[0]; // wake_add sprite.
	wake->r.flags = (RF_FIXED | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
	wake->r.scale = 0.3f;
	wake->d_scale = 1.1f;
	wake->alpha = 0.7f;
	wake->d_alpha = -0.8f;
	wake->r.origin[1] = creator->origin[1];
	wake->r.origin[0] = creator->origin[0];
	wake->r.angles[YAW] = yaw;

	float vel = VectorNormalize(velocity);
	const float len = min(vel - 30.0f, 50.0f);

	VectorScale(velocity, len, wake->velocity);
	wake->velocity[2] = 0.0f;

	AddEffect(NULL, wake);

	// Splashies too.
	vel = min(320.0f, vel);

	int num_particles;
	if (R_DETAIL == DETAIL_LOW)
		num_particles = 0;
	else if (R_DETAIL == DETAIL_NORMAL)
		num_particles = (int)(vel / 64.0f);
	else // DETAIL_HIGH+
		num_particles = (int)(vel / 32.0f); //TODO: separate case for DETAIL_UBERHIGH.

	if (num_particles < 4)
		return;

	const vec3_t fwd =   { -cosf(yaw), -sinf(yaw), 0.0f };
	const vec3_t right = { -sinf(yaw),  cosf(yaw), 0.0f };

	for (int i = 0; i < num_particles; i++)
	{
		// Big particle.
		client_particle_t* bp = ClientParticle_new(PART_32x32_WFALL, color_white, 500);

		bp->scale = 4.0f;
		bp->d_scale = vel / 5.0f;

		VectorScale(right, flrand(-12.0f, 12.0f), bp->origin);
		VectorScale(bp->origin, 0.01f * vel, bp->velocity);
		VectorMA(bp->velocity, flrand(0.2f * vel, 0.5f * vel), fwd, bp->velocity);
		bp->velocity[2] += flrand(30.0f, 40.0f);
		bp->acceleration[2] = -320.0f;

		AddParticleToList(wake, bp);

		// Small particle.
		client_particle_t* sp = ClientParticle_new(PART_4x4_WHITE, color_white, 750);

		sp->scale = 1.0f;
		sp->d_scale = -1.0f;
		sp->color.a = 128;
		sp->d_alpha = flrand(-200.0f, -160.0f);

		VectorScale(right, flrand(-0.2f * vel, 0.2f * vel), sp->velocity);
		VectorMA(sp->velocity, flrand(0.0f, 0.5f * vel), fwd, sp->velocity);
		sp->velocity[2] += flrand(0.1f * vel, 0.2f * vel);
		sp->acceleration[2] = -320.0f;

		AddParticleToList(wake, sp);
	}
}