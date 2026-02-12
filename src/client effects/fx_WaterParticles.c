//
// fx_WaterParticles.c -- originally part of Generic Character Effects.c.
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Particle.h"
#include "Vector.h"
#include "Random.h"
#include "Utilities.h"
#include "g_playstats.h"
#include "Matrix.h"
#include "Reference.h"
#include "turbsin.h"

#define SINEAMT					1
#define SINESCALE				(256.0f / (2 * M_PI))
#define WATER_DENSITY			150.0f
#define WATER_DIST				100.0f
#define WATERPARTICLE_CLIPDIST	(WATER_DIST * WATER_DIST)

static void SetupWaterParticle(client_particle_t* p, const qboolean recycle)
{
	const byte ishade = (byte)irand(50, 150);
	p->color.r = ishade;
	p->color.g = ishade;
	p->color.b = ishade;
	p->color.a = (byte)irand(75, 150);

	const float min_vel_z = (((float)ishade * 0.04f) - 3.1f) * 0.35f;
	VectorSet(p->velocity, flrand(-1.0f, 1.0f), flrand(-1.0f, 1.0f), flrand(min_vel_z, 0.0f));

	p->acceleration[2] = 0.0f;
	p->scale = flrand(0.3f, 0.7f);

	vec3_t dist;
	VectorRandomSet(dist, WATER_DIST); //mxd

	// If we are recycling, we want to respawn as far away as possible.
	if (recycle)
	{
		VectorNormalize(dist);
		Vec3ScaleAssign(WATER_DIST, dist);
	}

	VectorAdd(fxi.cl->refdef.vieworg, dist, p->origin);
}

static void CreateWaterParticles(client_entity_t* self)
{
	// Scale number of particles by detail level.
	const int detail = (int)(WATER_DENSITY * r_detail->value);

	for (int i = 0; i < detail; i++)
	{
		client_particle_t* p = ClientParticle_new(PART_4x4_WHITE | PFL_SOFT_MASK, self->color, 1000000);
		SetupWaterParticle(p, false);
		AddParticleToList(self, p);
	}
}

static qboolean WaterParticleGeneratorUpdate(client_entity_t* self, centity_t* owner)
{
	static qboolean water_particles_spawned;

	if ((int)cl_camera_under_surface->value)
	{
		// Create particles when we are under water.
		if (!water_particles_spawned)
		{
			CreateWaterParticles(self);
			water_particles_spawned = true;
		}

		// Update water particles.
		for (client_particle_t* p = self->p_root; p != NULL; p = p->next)
		{
			vec3_t part_dist;
			VectorSubtract(p->origin, fxi.cl->refdef.vieworg, part_dist);

			if (VectorLengthSquared(part_dist) >= WATERPARTICLE_CLIPDIST)
			{
				SetupWaterParticle(p, true);
				continue;
			}

			float add_val = SINEAMT / 128.0f * turbsin[(int)(((float)fx_time * 0.001f + (self->origin[0] * 2.3f + p->origin[1]) * 0.0015f) * SINESCALE) & 255];
			add_val += SINEAMT / 256.0f * turbsin[(int)(((float)fx_time * 0.002f + (self->origin[1] * 2.3f + p->origin[0]) * 0.0015f) * SINESCALE) & 255];

			p->origin[2] += add_val;
			p->duration = fx_time + 10000000;
		}
	}
	else
	{
		// Free up particles when we are not under water.
		if (water_particles_spawned)
		{
			FreeParticles(self);
			water_particles_spawned = false;
		}
	}

	return true;
}

static void DoWake(client_entity_t* self, const centity_t* owner, const int refpt)
{
	static int wake_particle[6] =
	{
		PART_4x4_WHITE,
		PART_8x8_BUBBLE,
		PART_16x16_WATERDROP,
		PART_32x32_WFALL,
		PART_32x32_STEAM,
		PART_32x32_BUBBLE
	};

	const paletteRGBA_t light_color = { .r = 200, .g = 255, .b = 255, .a = 140 };

	vec3_t diff;
	VectorSubtract(owner->referenceInfo->references[refpt].placement.origin, owner->referenceInfo->oldReferences[refpt].placement.origin, diff);

	vec3_t diff2;
	VectorSubtract(owner->origin, self->endpos, diff2);
	Vec3AddAssign(diff2, diff);

	int num_parts = (int)(VectorLength(diff));
	num_parts = min(6, num_parts);

	// Let's take the origin and transform it to the proper coordinate offset from the owner's origin.
	const vec3_t org = VEC3_INIT(owner->referenceInfo->references[refpt].placement.origin);

	// Create a rotation matrix.
	matrix3_t rotation;
	Matrix3FromAngles(owner->lerp_angles, rotation);

	vec3_t handpt;
	Matrix3MultByVec3(rotation, org, handpt);
	Vec3AddAssign(owner->origin, handpt);

	vec3_t right;
	AngleVectors(owner->lerp_angles, NULL, right, NULL);

	for (int i = 0; i < num_parts; i++)
	{
		int type = wake_particle[irand(0, 5)];
		if (R_DETAIL == DETAIL_LOW)
			type |= PFL_SOFT_MASK;

		client_particle_t* p = ClientParticle_new(type, light_color, irand(1000, 2000));

		VectorRandomSet(p->origin, 4.0f); //mxd
		Vec3AddAssign(handpt, p->origin);

		p->scale = flrand(0.75f, 1.5f);
		p->color.a = (byte)irand(100, 200);

		VectorRandomSet(p->velocity, 2.0f); //mxd

		const float sign = (irand(0, 1) ? -1.0f : 1.0f);
		VectorMA(p->velocity, flrand(10, 2) * sign, right, p->velocity);

		p->acceleration[2] = 16.0f;
		p->d_scale = flrand(-0.15f, -0.1f);

		AddParticleToList(self, p);
	}
}

static qboolean BubbleSpawnerUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'BubbleSpawner' in original logic.
{
	if (!(int)cl_camera_under_surface->value)
		return true;

	// Spawn some bubbles too.
	vec3_t org;
	VectorRandomSet(org, 20.0f); //mxd
	Vec3AddAssign(owner->origin, org);

	MakeBubble(org, self);

	// Create a wake of bubbles!
	if (R_DETAIL >= DETAIL_HIGH && RefPointsValid(owner))
	{
		DoWake(self, owner, CORVUS_RIGHTHAND);
		DoWake(self, owner, CORVUS_LEFTHAND);
		DoWake(self, owner, CORVUS_RIGHTFOOT);
		DoWake(self, owner, CORVUS_LEFTFOOT);

		VectorCopy(owner->origin, self->endpos);

		return true;
	}

	return false; // Remove the effect.
}

void FXWaterParticles(centity_t* owner, const int type, int flags, vec3_t origin)
{
	assert(owner);

	flags |= (CEF_NO_DRAW | CEF_ABSOLUTE_PARTS | CEF_OWNERS_ORIGIN | CEF_VIEWSTATUSCHANGED); //mxd

	// Spawn static water particle handler.
	client_entity_t* water_fx = ClientEntity_new(type, flags, origin, NULL, PARTICLE_TRAIL_THINK_TIME);

	water_fx->radius = 100.0f;
	water_fx->AddToView = LinkedEntityUpdatePlacement;
	water_fx->Update = WaterParticleGeneratorUpdate;

	AddEffect(owner, water_fx);

	// Spawn bubble spawner.
	client_entity_t* bubble_fx = ClientEntity_new(type, flags, origin, NULL, PARTICLE_TRAIL_THINK_TIME);

	VectorCopy(owner->origin, bubble_fx->endpos);
	bubble_fx->radius = 100.0f;
	bubble_fx->AddToView = LinkedEntityUpdatePlacement;
	bubble_fx->Update = BubbleSpawnerUpdate;

	AddEffect(owner, bubble_fx);
}