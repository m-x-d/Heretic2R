//
// fx_spellhands.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Matrix.h"
#include "Particle.h"
#include "Reference.h"
#include "Utilities.h"
#include "Vector.h"
#include "g_playstats.h"

static qboolean SpellHandsThink(struct client_entity_s* self, centity_t* owner)
{
	// If we've timed out, stop the effect (allow for fading). If we're not on a time limit, check the EF flag.
	if ((self->LifeTime > 0 && self->LifeTime < fxi.cl->time) || (self->LifeTime <= 0 && !(owner->current.effects & EF_TRAILS_ENABLED)))
	{
		self->Update = RemoveSelfAI;
		self->nextThinkTime = fxi.cl->time + 500; //BUGFIX: mxd. sets updateTime in original logic (makes no sense: updateTime is ADDED to fxi.cl->time in UpdateEffects()).

		return true;
	}

	// This tells if we are wasting our time, because the reference points are culled.
	if (!RefPointsValid(owner))
		return true;

	// Calculate the end position of the trail.
	vec3_t trail_end;
	VectorCopy(owner->referenceInfo->references[self->refPoint].placement.origin, trail_end);

	// We now have trail start and trail end in.
	vec3_t trail_start;
	VectorCopy(self->origin, trail_start);

	// Update where trail ends.
	VectorCopy(trail_end, self->origin);

	// Allow us adequate time to set up valid 'old' data because the reference points lag behind by a frame.
	self->AnimSpeed += 1.0f;
	if (self->AnimSpeed < 2.0f)
		return true;

	// Create a rotation matrix.
	matrix3_t rotation;
	Matrix3FromAngles(owner->lerp_angles, rotation);

	// Make the trail start and end a trail in real space.
	vec3_t real_trail_start;
	Matrix3MultByVec3(rotation, trail_start, real_trail_start);

	vec3_t real_trail_end;
	Matrix3MultByVec3(rotation, trail_end, real_trail_end);

	// Figure out the differences between them.
	vec3_t trail_delta;
	VectorSubtract(real_trail_end, real_trail_start, trail_delta);

	// Set the trail length.
	const int trail_length = GetScaledCount(4, 0.75f);

	// Scale that difference by the number of particles we are going to draw.
	Vec3ScaleAssign(1.0f / (float)trail_length, trail_delta);

	// Decide which particle type to use.
	int part_type;
	if (self->SpawnInfo == 0)
		part_type = PART_16x16_SPARK_R;
	else if (self->SpawnInfo == 1)
		part_type = PART_16x16_SPARK_B;
	else
		part_type = PART_16x16_SPARK_I;

	// Now draw the trail.
	for (int i = 0; i < trail_length; i++)
	{
		client_particle_t* ce = ClientParticle_new(part_type, color_white, 400);

		VectorCopy(real_trail_start, ce->origin);
		ce->scale = 8.0f;
		ce->acceleration[2] = 0.0f;
		VectorRandomSet(ce->velocity, 8.0f);

		AddParticleToList(self, ce);

		VectorAdd(real_trail_start, trail_delta, real_trail_start);
	}

	return true;
}

void FXSpellHands(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	char lifetime;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_SPELLHANDS].formatString, &lifetime);

	short refpoints = (1 << CORVUS_RIGHTHAND);
	if (flags & CEF_FLAG6)
		refpoints |= (1 << CORVUS_LEFTHAND);

	// Add a fiery trail effect to the player's hands / feet etc.
	const int cast_speed = ((int)r_detail->value == DETAIL_LOW ? 75 : 50);

	for (short p = 0; p < 16; p++)
	{
		if (!(refpoints & (1 << p)))
			continue;

		client_entity_t* trail = ClientEntity_new(type, (int)(flags | CEF_NO_DRAW | CEF_ADDITIVE_PARTS), origin, NULL, cast_speed);

		trail->r.flags = (RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
		trail->SpawnInfo = (flags & (CEF_FLAG7 | CEF_FLAG8)) >> 6;
		trail->LifeTime = ((lifetime > 0) ? fxi.cl->time + lifetime * 100 : -1);
		trail->refPoint = p;
		trail->AnimSpeed = 0.0f; // Hack: used as a counter.
		trail->AddToView = LinkedEntityUpdatePlacement;
		trail->Update = SpellHandsThink;

		AddEffect(owner, trail);
	}
}