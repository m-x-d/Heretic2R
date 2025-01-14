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

static qboolean FXFireHandsThink(struct client_entity_s* self, const centity_t* owner)
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
		return false; // Remove the effect in this case.

	VectorCopy(owner->origin, self->r.origin);

	// Let's take the origin and transform it to the proper coordinate offset from the owner's origin.
	vec3_t firestart;
	VectorCopy(owner->referenceInfo->references[self->refPoint].placement.origin, firestart);

	// Create a rotation matrix
	matrix3_t rotation;
	Matrix3FromAngles(owner->lerp_angles, rotation);

	vec3_t origin;
	Matrix3MultByVec3(rotation, firestart, origin);

	const int flame_duration = (r_detail->value < DETAIL_NORMAL ? 1500 : 2000);
	client_particle_t* flame = ClientParticle_new(irand(PART_32x32_FIRE0, PART_32x32_FIRE2), self->color, flame_duration);

	VectorRandomSet(flame->origin, HANDFIRE_RADIUS); //mxd
	VectorAdd(flame->origin, origin, flame->origin);

	flame->scale = HANDFIRE_SCALE;
	VectorSet(flame->velocity, flrand(-5.0f, 5.0f), flrand(-5.0f, 5.0f), flrand(15.0f, 22.0f));
	flame->acceleration[2] = HANDFIRE_ACCEL;
	flame->d_scale = flrand(-10.0f, -5.0f);
	flame->d_alpha = flrand(-200.0f, -160.0f);
	flame->duration = (int)((255.0f * 1000.0f) / -flame->d_alpha); // Time taken to reach zero alpha.

	AddParticleToList(self, flame);

	return true;
}

// ************************************************************************************************
// FXFireHands
// ------------
// ************************************************************************************************

void FXFireHands(centity_t *owner,int type,int flags,vec3_t origin)
{
	short			refpoints;
	client_entity_t	*trail;
	int				i;
	int				flame_dur;
	char			lifetime;

	refpoints=0;

	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_FIREHANDS].formatString, &lifetime);

	if (flags & CEF_FLAG6)
		refpoints=(1 << CORVUS_LEFTHAND) | (1 << CORVUS_RIGHTHAND);
	else
		refpoints=(1 << CORVUS_LEFTHAND);

	VectorClear(origin);

	
	if (r_detail->value > DETAIL_NORMAL)
		flame_dur = 50;
	else
		flame_dur = 75;

	// Add a fiery trail effect to the player's hands / feet etc.

	for(i=0;i<16;i++)
	{
		if(!(refpoints & (1 << i)))
			continue;

		trail=ClientEntity_new(type,flags,origin,0,flame_dur);

		trail->Update=FXFireHandsThink;
		trail->flags|=CEF_NO_DRAW | CEF_OWNERS_ORIGIN | CEF_ADDITIVE_PARTS;
		trail->radius = 128;
		trail->AddToView = LinkedEntityUpdatePlacement;			
		trail->refPoint = i;
		trail->color.c = 0xe5007fff;

		if (lifetime > 0)
			trail->LifeTime = fxi.cl->time + (lifetime * 100);
		else
			trail->LifeTime = -1;


		AddEffect(owner,trail);
	}
}
