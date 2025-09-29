//
// fx_firehands.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Particle.h"
#include "Vector.h"
#include "Matrix.h"
#include "Random.h"
#include "Utilities.h"
#include "Reference.h"
#include "g_playstats.h"

#define HANDFIRE_RADIUS	2.0f
#define HANDFIRE_SCALE	8.0f
#define HANDFIRE_ACCEL	32.0f

static qboolean FireHandsThink(struct client_entity_s* self, centity_t* owner)
{
#define FH_PARTICLE_DURATION	1000 //mxd

	// If we've timed out, stop the effect (allow for fading). If we're not on a time limit, check the EF flag.
	if ((self->LifeTime > 0 && self->LifeTime < fxi.cl->time) || (self->LifeTime <= 0 && !(owner->current.effects & EF_TRAILS_ENABLED)))
	{
		self->Update = RemoveSelfAI;
		self->updateTime = FH_PARTICLE_DURATION; //BUGFIX: mxd. 'fxi.cl->time + 500' in original logic (makes no sense: updateTime is ADDED to fxi.cl->time in UpdateEffects()).

		return true;
	}

	// This tells if we are wasting our time, because the reference points are culled.
	if (!RefPointsValid(owner))
		return false; // Remove the effect in this case.

	VectorCopy(owner->origin, self->r.origin);

	// Let's take the origin and transform it to the proper coordinate offset from the owner's origin.
	vec3_t firestart;
	VectorCopy(owner->referenceInfo->references[self->refPoint].placement.origin, firestart);

	// Create a rotation matrix.
	matrix3_t rotation;
	Matrix3FromAngles(owner->lerp_angles, rotation);

	vec3_t origin;
	Matrix3MultByVec3(rotation, firestart, origin);

	client_particle_t* flame = ClientParticle_new(irand(PART_32x32_FIRE0, PART_32x32_FIRE2), self->color, FH_PARTICLE_DURATION);

	VectorRandomSet(flame->origin, HANDFIRE_RADIUS); //mxd
	Vec3AddAssign(origin, flame->origin);

	flame->scale = HANDFIRE_SCALE;
	VectorSet(flame->velocity, flrand(-5.0f, 5.0f), flrand(-5.0f, 5.0f), flrand(15.0f, 22.0f));
	flame->acceleration[2] = HANDFIRE_ACCEL;
	flame->d_scale = flrand(-10.0f, -5.0f);
	flame->d_alpha = flrand(-200.0f, -160.0f);
	flame->duration = (int)(255.0f * FH_PARTICLE_DURATION / -flame->d_alpha); // Time taken to reach zero alpha.

	AddParticleToList(self, flame);

	return true;
}

void FXFireHands(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	char lifetime;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_FIREHANDS].formatString, &lifetime);

	short refpoints = (1 << CORVUS_LEFTHAND);
	if (flags & CEF_FLAG6)
		refpoints |= (1 << CORVUS_RIGHTHAND);

	VectorClear(origin);

	// Add a fiery trail effect to the player's hands / feet etc.
	int next_think_time;

	switch (R_DETAIL) //mxd. DETAIL_LOW ? 75 : 50 in original logic.
	{
		default:
		case DETAIL_LOW:		next_think_time = 75; break;
		case DETAIL_NORMAL:		next_think_time = 50; break;
		case DETAIL_HIGH:		next_think_time = 25; break;
		case DETAIL_UBERHIGH:	next_think_time = 0; break; // Update each frame --mxd.
	}

	// Add a fiery trail effect to the player's hands / feet etc.
	for (short p = 0; p < 16; p++)
	{
		if (!(refpoints & (1 << p)))
			continue;

		client_entity_t* trail = ClientEntity_new(type, flags, origin, 0, next_think_time);

		trail->flags |= (CEF_NO_DRAW | CEF_OWNERS_ORIGIN | CEF_ADDITIVE_PARTS);
		trail->LifeTime = (lifetime > 0 ? fxi.cl->time + lifetime * 100 : -1);
		trail->refPoint = p;
		trail->color.c = 0xe5007fff; // Used to color flame particles in FireHandsThink() --mxd.
		trail->AddToView = LinkedEntityUpdatePlacement;
		trail->Update = FireHandsThink;

		AddEffect(owner, trail);
	}
}