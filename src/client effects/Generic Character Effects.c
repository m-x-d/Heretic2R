//
// Generic Character Effects.c
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

// Create the two circles that ring the player.
static qboolean FXLeaderUpdate(struct client_entity_s* self, centity_t* owner) //mxd. Named 'FXLeaderThink' in original logic.
{
#define LEADER_RAD				12
#define TOTAL_LEADER_EFFECTS	30

	if (--self->LifeTime == 0)
		self->LifeTime = TOTAL_LEADER_EFFECTS;

	// If we are ghosted or dead, don't do the effect.
	if ((owner->current.renderfx & RF_TRANS_GHOST) || (owner->current.effects & EF_CLIENT_DEAD))
		return true;

	const paletteRGBA_t color = { .c = 0x7fffffff };

	// Create the ring of particles that goes up.
	client_particle_t* ce = ClientParticle_new(PART_16x16_SPARK_Y, color, 800);
	ce->acceleration[2] = 0.0f;
	VectorSet(ce->origin, LEADER_RAD * cosf(self->Scale), LEADER_RAD * sinf(self->Scale), 4.0f);
	ce->scale = 8.0f;
	AddParticleToList(self, ce);

	// Create the ring of particles that goes down.
	ce = ClientParticle_new(PART_16x16_SPARK_Y, color, 800);
	ce->acceleration[2] = 0.0f;
	VectorSet(ce->origin, LEADER_RAD * cosf(self->Scale + 3.14f), LEADER_RAD * sinf(self->Scale + 3.14f), 4.0f);
	ce->scale = 8.0f;
	AddParticleToList(self, ce);

	// Move the rings up/down next frame.
	self->Scale += 0.17f;

	return true;
}

// Create the entity the flight loops are on.
void FXLeader(centity_t* owner, const int type, int flags, vec3_t origin)
{
	flags |= (int)(CEF_NO_DRAW | CEF_ADDITIVE_PARTS);
	client_entity_t* glow = ClientEntity_new(type, flags, origin, NULL, 60);

	VectorClear(glow->origin);
	glow->LifeTime = TOTAL_LEADER_EFFECTS;
	glow->Scale = 0.0f;
	glow->AddToView = LinkedEntityUpdatePlacement;
	glow->Update = FXLeaderUpdate;

	AddEffect(owner, glow);
}

static qboolean FXFeetTrailUpdate(struct client_entity_s* self, centity_t* owner) //mxd. Named 'FXFeetTrailThink' in original logic.
{
#define FOOTTRAIL_RADIUS	2.0f
#define FOOTTRAIL_SCALE		8.0f
#define FOOTTRAIL_ACCEL		20.0f

	// If the reference points are culled, or we are ghosted or dead, don't do the effect.
	if (!RefPointsValid(owner) || (owner->current.renderfx & RF_TRANS_GHOST) || (owner->current.effects & EF_CLIENT_DEAD))
		return true;

	if (!(owner->current.effects & EF_SPEED_ACTIVE))
	{
		self->Update = RemoveSelfAI;
		self->updateTime = 1500; //BUGFIX: mxd. 'fxi.cl->time + 1500' in original logic (makes no sense: updateTime is ADDED to fxi.cl->time in UpdateEffects()).

		return true;
	}

	// Let's take the origin and transform it to the proper coordinate offset from the owner's origin.
	vec3_t firestart;
	VectorCopy(owner->referenceInfo->references[self->refPoint].placement.origin, firestart);

	// Create a rotation matrix
	matrix3_t rotation;
	Matrix3FromAngles(owner->lerp_angles, rotation);

	vec3_t origin;
	Matrix3MultByVec3(rotation, firestart, origin);
	Vec3AddAssign(owner->origin, origin);

	if (Vec3NotZero(self->origin))
	{
		// Create small particles
		const int count = GetScaledCount(5, 0.5f);

		vec3_t diff;
		VectorSubtract(self->origin, origin, diff);
		Vec3ScaleAssign(1.0f / (float)count, diff);

		vec3_t cur_pos = { 0 };
		const int hand_flame_dur = (R_DETAIL < DETAIL_NORMAL ? 1500 : 2000);
		const paletteRGBA_t color = { .c = 0xffffff40 };

		for (int i = 0; i < count; i++)
		{
			client_particle_t* flame = ClientParticle_new(PART_32x32_STEAM, color, hand_flame_dur);

			VectorRandomSet(flame->origin, FOOTTRAIL_RADIUS); //mxd
			VectorAdd(flame->origin, self->origin, flame->origin);
			VectorAdd(flame->origin, cur_pos, flame->origin);

			flame->scale = FOOTTRAIL_SCALE;
			VectorSet(flame->velocity, flrand(-5.0f, 5.0f), flrand(-5.0f, 5.0f), flrand(5.0f, 15.0f));
			flame->acceleration[2] = FOOTTRAIL_ACCEL;
			flame->d_scale = flrand(-10.0f, -5.0f);
			flame->d_alpha = flrand(-200.0f, -160.0f);
			flame->duration = (int)((255.0f * 1000.0f) / -flame->d_alpha); // Time needed to reach zero alpha.

			AddParticleToList(self, flame);
			Vec3SubtractAssign(diff, cur_pos);
		}
	}

	VectorCopy(origin, self->origin);

	return true;
}

void FXFeetTrail(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	const short refpoints = (1 << CORVUS_LEFTFOOT) | (1 << CORVUS_RIGHTFOOT);
	const int flame_update = (R_DETAIL > DETAIL_NORMAL ? 50 : 75);

	VectorClear(origin);

	// Add a fiery trail effect to the player's hands / feet etc.
	for (int i = 0; i < 16; i++)
	{
		if (!(refpoints & (1 << i)))
			continue;

		client_entity_t* trail = ClientEntity_new(type, flags, origin, NULL, flame_update);

		VectorClear(trail->origin);
		trail->flags |= (int)(CEF_NO_DRAW | CEF_OWNERS_ORIGIN | CEF_ABSOLUTE_PARTS);
		trail->radius = 40;
		trail->refPoint = (short)i;
		trail->color.c = 0xe5007fff; //TODO: unused?
		trail->AddToView = LinkedEntityUpdatePlacement;
		trail->Update = FXFeetTrailUpdate;

		AddEffect(owner, trail);
	}
}