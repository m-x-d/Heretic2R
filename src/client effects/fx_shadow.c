//
// fx_shadow.c
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
#include "Angles.h"
#include "Utilities.h"
#include "Matrix.h"
#include "Reference.h"
#include "q_Sprite.h"
#include "q_Surface.h"
#include "PrimitiveDisplayHack.h"
#include "g_playstats.h"

#include <limits.h>

#define SHADOW_CHECK_DIST 256.0
#define SHADOW_REF_CHECK_DIST 64.0

#define MAX_SHADOW_DIST 128
#define SHADOW_HEIGHT 0.5

extern int	ref_soft;

#define	NUM_SHADOW_MODELS	1

static struct model_s *shadow_models[NUM_SHADOW_MODELS];

void PrecacheShadow()
{
	shadow_models[0] = fxi.RegisterModel("models/fx/shadow/tris.fm");
}

static qboolean FXShadowUpdate(struct client_entity_s *self,centity_t *owner)
{
	vec3_t startpos, endpos;
	vec3_t minmax = {0, 0, 0};
	trace_t trace;

	VectorCopy(owner->origin, self->r.origin);
	VectorCopy(owner->origin, startpos);
		
	startpos[2] += 4;
	// Now trace from the startpos down
	VectorCopy(startpos, endpos);
	endpos[2] -= SHADOW_CHECK_DIST;

	//Determine Visibility
	fxi.Trace(	startpos, 
				minmax, 
				minmax, 
				endpos, 
				CONTENTS_SOLID, 
				CEF_CLIP_TO_WORLD, 
				&trace);

	if (trace.startsolid || trace.fraction >= 1.0)
	{	// no shadow, in something.
		self->alpha = 0.01;
		VectorCopy(endpos, self->r.origin);
		return true;
	}

	// Did hit the ground
	self->alpha = (1.0 - trace.fraction) * 0.5 + 0.01;
	// if we are in ref soft, bring us out a touch, since we are having z buffer problems
	if (ref_soft)
	{
		// Raise the shadow slightly off the target wall
		VectorMA(trace.endpos, 0.9, trace.plane.normal, self->r.origin);
	}
	else
		VectorMA(trace.endpos, 0.2, trace.plane.normal, self->r.origin);
	AnglesFromDirI(trace.plane.normal, self->r.angles);

	return true;
}



static qboolean FXShadowReferenceUpdate(struct client_entity_s *self,centity_t *owner)
{
	vec3_t startpos, endpos;
	vec3_t minmax = {0, 0, 0};
	trace_t trace;
	int refpoint;
	matrix3_t		rotmatrix;

	// This tells if we are wasting our time, because the reference points are culled.
	if (!RefPointsValid(owner))
	{	// The foot shadows should not be visible when there are no ref points.
		VectorCopy(owner->origin, self->r.origin);
		self->alpha = 0.01;
		return true;
	}

	VectorCopy(owner->origin, self->r.origin);

	// take the startpoint from one of the reference points
	refpoint = self->refPoint;

	Matrix3FromAngles(owner->lerp_angles,rotmatrix);

	Matrix3MultByVec3(rotmatrix, owner->referenceInfo->references[refpoint].placement.origin, startpos);
	
	// This may look weird, but by scaling the vector, I bring it closer to the center of the owner.  
	VectorScale(startpos, 0.5, startpos);
	VectorAdd(owner->origin, startpos, startpos);
		
	// Now trace fromt the startpos down
	VectorCopy(startpos, endpos);
	endpos[2] -= SHADOW_REF_CHECK_DIST;

	//Determine Visibility
	fxi.Trace(	startpos, 
				minmax, 
				minmax, 
				endpos, 
				CONTENTS_SOLID, 
				CEF_CLIP_TO_WORLD, 
				&trace);

	if (trace.startsolid || trace.fraction >= 1.0)
	{	// no shadow, in something.
		self->alpha = 0.01;
		VectorCopy(endpos, self->r.origin);
		return true;
	}

	// Did hit the ground
	self->alpha = (1.0 - trace.fraction) * 0.8 + 0.01;
	self->r.scale = (1.0 - trace.fraction) * 0.8;
	if (ref_soft)
	{
		// Raise the shadow slightly off the target wall
		VectorMA(trace.endpos, 0.9, trace.plane.normal, self->r.origin);
	}
	else
		VectorMA(trace.endpos, 0.2, trace.plane.normal, self->r.origin);
	AnglesFromDirI(trace.plane.normal, self->r.angles);

	return true;
}





void FXShadow(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*self;
	float				scale;

	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_SHADOW].formatString, &scale);

	if(r_detail->value < DETAIL_UBERHIGH)
		return;

	// Create shadow under the player
  	self = ClientEntity_new(type, flags, origin, NULL, INT_MAX);
	self->nextThinkTime = INT_MAX;
	self->r.model = shadow_models;
	self->r.flags |= RF_FULLBRIGHT|RF_TRANSLUCENT|RF_ALPHA_TEXTURE;
	self->radius = SHADOW_CHECK_DIST;
	self->r.scale = scale;
	self->AddToView = FXShadowUpdate;

	AddEffect(owner, self);
	
	FXShadowUpdate(self, owner);
}

// Cast a shadow down from each foot and the player, too
void FXPlayerShadow(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*self;

	// Create shadow under the player
  	self = ClientEntity_new(type, flags, origin, NULL, INT_MAX);
	self->nextThinkTime = INT_MAX;
	self->r.model = shadow_models;
	self->r.flags |= RF_FULLBRIGHT|RF_TRANSLUCENT|RF_ALPHA_TEXTURE;
	self->radius = SHADOW_CHECK_DIST;
	self->r.scale = 1.0;
	self->AddToView = FXShadowUpdate;
	AddEffect(owner, self);
	FXShadowUpdate(self, owner);

	// Create shadow under the left foot
  	self = ClientEntity_new(type, flags, origin, NULL, INT_MAX);
	self->nextThinkTime = INT_MAX;
	self->r.model = shadow_models;
	self->r.flags |= RF_FULLBRIGHT|RF_TRANSLUCENT|RF_ALPHA_TEXTURE;
	self->radius = SHADOW_CHECK_DIST;
	self->refPoint = CORVUS_LEFTFOOT;
	self->r.scale = 0.8;
	self->AddToView = FXShadowReferenceUpdate;
	AddEffect(owner, self);
	FXShadowUpdate(self, owner);

	// Create shadow under the right foot
  	self = ClientEntity_new(type, flags, origin, NULL, INT_MAX);
	self->nextThinkTime = INT_MAX;
	self->r.model = shadow_models;
	self->r.flags |= RF_FULLBRIGHT|RF_TRANSLUCENT|RF_ALPHA_TEXTURE;
	self->radius = SHADOW_CHECK_DIST;
	self->refPoint = CORVUS_RIGHTFOOT;
	self->r.scale = 0.8;
	self->AddToView = FXShadowReferenceUpdate;
	AddEffect(owner, self);
	FXShadowUpdate(self, owner);
}


