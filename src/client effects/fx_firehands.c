//
// fx_firehands.c
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
#include "Matrix.h"
#include "Random.h"
#include "Utilities.h"
#include "Reference.h"
#include "ce_DLight.h"
#include "g_playstats.h"


#define HANDFIRE_NUM	1
#define HANDFIRE_RADIUS	2.0
#define HANDFIRE_SCALE	8.0
#define HANDFIRE_ACCEL	32.0



static qboolean FXFireHandsThink(struct client_entity_s *self,centity_t *owner)
{

	client_particle_t	*flame;
	int					i;
	vec3_t				firestart, origin;
	matrix3_t			rotation;
	int					hand_flame_dur;

	// If we've timed out, stop the effect (allow for fading)
	if (self->LifeTime > 0)
	{
		if (self->LifeTime < fxi.cl->time)
		{	// Lifetime is up
			self->Update=RemoveSelfAI;
			self->updateTime = fxi.cl->time + 500;
			return true;
		}
	}
	else 
	{	// If we're not on a time limit, check the EF flag
		if (!(owner->current.effects & EF_TRAILS_ENABLED))
		{
			self->Update=RemoveSelfAI;
			self->updateTime = fxi.cl->time + 500;
			return true;
		}
	}

	// This tells if we are wasting our time, because the reference points are culled.
	if (!RefPointsValid(owner))
		return false;		// Remove the effect in this case.

	VectorCopy(owner->origin, self->r.origin);

	// Let's take the origin and transform it to the proper coordinate offset from the owner's origin.
	VectorCopy(owner->referenceInfo->references[self->refPoint].placement.origin, firestart);
	// Create a rotation matrix
	Matrix3FromAngles(owner->lerp_angles, rotation);
	Matrix3MultByVec3(rotation, firestart, origin);
	
	if (r_detail->value < DETAIL_NORMAL)
		hand_flame_dur = 1500;
	else
		hand_flame_dur = 2000;

	for(i = 0; i < HANDFIRE_NUM; i++)
	{
		flame = ClientParticle_new(irand(PART_32x32_FIRE0, PART_32x32_FIRE2), self->color, hand_flame_dur);
		VectorSet(	flame->origin, 
					flrand(-HANDFIRE_RADIUS, HANDFIRE_RADIUS), 
					flrand(-HANDFIRE_RADIUS, HANDFIRE_RADIUS), 
					flrand(-HANDFIRE_RADIUS, HANDFIRE_RADIUS));
		VectorAdd(flame->origin, origin, flame->origin);
	
		flame->scale = HANDFIRE_SCALE;
		VectorSet(flame->velocity, flrand(-5.0, 5.0), flrand(-5, 5.0), flrand(15.0, 22.0));
		flame->acceleration[2] = HANDFIRE_ACCEL;
		flame->d_scale = flrand(-10.0, -5.0);
		flame->d_alpha = flrand(-200.0, -160.0);
		flame->duration = (255.0 * 1000.0) / -flame->d_alpha;		// time taken to reach zero alpha

		AddParticleToList(self, flame);
	}
	
	return(true);
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
