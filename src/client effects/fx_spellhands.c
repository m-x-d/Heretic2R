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

void PreCacheHands(void) { } //TODO: unused.

static qboolean FXSpellHandsThink(struct client_entity_s* self, const centity_t* owner)
{
	// If we've timed out, stop the effect (allow for fading). If we're not on a time limit, check the EF flag.
	if ((self->LifeTime > 0 && self->LifeTime < fxi.cl->time) || !(owner->current.effects & EF_TRAILS_ENABLED))
	{
		self->Update = RemoveSelfAI;
		self->updateTime = fxi.cl->time + 500;

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

// ************************************************************************************************
// FXSpellHands
// ------------
// ************************************************************************************************

void FXSpellHands(centity_t *Owner,int Type,int Flags,vec3_t Origin)
{
	short			Refpoints;
	client_entity_t	*Trail;
	char			lifetime;
	int				I;
	int				cast_speed;

	fxi.GetEffect(Owner, Flags, clientEffectSpawners[FX_SPELLHANDS].formatString, &lifetime);

	if (Flags & CEF_FLAG6)
		Refpoints=(1 << CORVUS_LEFTHAND) | (1 << CORVUS_RIGHTHAND);
	else
		Refpoints=(1 << CORVUS_RIGHTHAND);

	// Add a fiery trail effect to the player's hands / feet etc.

	if (r_detail->value < DETAIL_NORMAL)
		cast_speed = 75;
	else
		cast_speed = 50;

	for(I=0;I<16;I++)
	{
		if(!(Refpoints & (1 << I)))
			continue;

		Trail=ClientEntity_new(Type,Flags|CEF_NO_DRAW|CEF_ADDITIVE_PARTS,Origin,0,cast_speed);
		Trail->r.flags=RF_TRANSLUCENT|RF_TRANS_ADD|RF_TRANS_ADD_ALPHA;
		Trail->Update=FXSpellHandsThink;
		Trail->SpawnInfo= ((Flags&(CEF_FLAG7|CEF_FLAG8)) >> 6);
		Trail->AddToView = LinkedEntityUpdatePlacement;			
		
		if (lifetime > 0)
			Trail->LifeTime = fxi.cl->time + (lifetime * 100);
		else
			Trail->LifeTime = -1;

		Trail->refPoint = I;
		
		// Hack: used as a counter.
		Trail->AnimSpeed=0.0;

		AddEffect(Owner,Trail);
	}
}

