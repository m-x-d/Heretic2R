//
// fx_shadow.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Matrix.h"
#include "Reference.h"
#include "Utilities.h"
#include "Vector.h"
#include "g_playstats.h"

#define SHADOW_CHECK_DIST		256.0f
#define SHADOW_REF_CHECK_DIST	64.0f

static struct model_s* shadow_model;

void PrecacheShadow(void)
{
	shadow_model = fxi.RegisterModel("models/fx/shadow/tris.fm");
}

static qboolean ShadowUpdate(struct client_entity_s* self, centity_t* owner)
{
	VectorCopy(owner->origin, self->r.origin);

	const vec3_t start_pos = { owner->origin[0], owner->origin[1], owner->origin[2] + 4.0f };

	// Now trace from the start_pos down.
	const vec3_t end_pos = { start_pos[0], start_pos[1], start_pos[2] - SHADOW_CHECK_DIST };

	// Determine visibility.
	trace_t trace;
	fxi.Trace(start_pos, vec3_origin, vec3_origin, end_pos, CONTENTS_SOLID, CEF_CLIP_TO_WORLD, &trace);

	if (trace.startsolid || trace.fraction >= 1.0f)
	{
		// No shadow, in something.
		self->alpha = 0.01f;
		VectorCopy(end_pos, self->r.origin);

		return true;
	}

	// Did hit the ground.
	self->alpha = (1.0f - trace.fraction) * 0.5f + 0.01f;

	// If we are in ref soft, bring us out a touch, since we are having z buffer problems.
	const float offset = (ref_soft ? 0.9f : 0.2f); //mxd

	// Raise the shadow slightly off the target wall.
	VectorMA(trace.endpos, offset, trace.plane.normal, self->r.origin);
	AnglesFromDirI(trace.plane.normal, self->r.angles);

	return true;
}

static qboolean ShadowReferenceUpdate(struct client_entity_s* self, centity_t* owner)
{
	// This tells if we are wasting our time, because the reference points are culled.
	if (!RefPointsValid(owner))
	{
		// The foot shadows should not be visible when there are no ref points.
		VectorCopy(owner->origin, self->r.origin);
		self->alpha = 0.01f;

		return true;
	}

	VectorCopy(owner->origin, self->r.origin);

	matrix3_t rotation;
	Matrix3FromAngles(owner->lerp_angles, rotation);

	// Take the start_pos from one of the reference points.
	vec3_t start_pos;
	Matrix3MultByVec3(rotation, owner->referenceInfo->references[self->refPoint].placement.origin, start_pos);

	// This may look weird, but by scaling the vector, I bring it closer to the center of the owner.  
	VectorScale(start_pos, 0.5f, start_pos);
	VectorAdd(owner->origin, start_pos, start_pos);

	// Now trace from the start_pos down.
	const vec3_t end_pos = { start_pos[0], start_pos[1], start_pos[2] - SHADOW_REF_CHECK_DIST };

	// Determine visibility.
	trace_t trace;
	fxi.Trace(start_pos, vec3_origin, vec3_origin, end_pos, CONTENTS_SOLID, CEF_CLIP_TO_WORLD, &trace);

	if (trace.startsolid || trace.fraction >= 1.0f)
	{
		// No shadow, in something.
		VectorCopy(end_pos, self->r.origin);
		self->alpha = 0.01f;

		return true;
	}

	// Did hit the ground.
	const float scale = (1.0f - trace.fraction) * 0.8f; //mxd
	self->alpha = scale + 0.01f;
	self->r.scale = scale;

	// If we are in ref soft, bring us out a touch, since we are having z buffer problems.
	const float offset = (ref_soft ? 0.9f : 0.2f); //mxd

	// Raise the shadow slightly off the target wall.
	VectorMA(trace.endpos, offset, trace.plane.normal, self->r.origin);
	AnglesFromDirI(trace.plane.normal, self->r.angles);

	return true;
}

void FXShadow(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	float scale;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_SHADOW].formatString, &scale);

	if (r_detail->value < DETAIL_UBERHIGH)
		return;

	// Create shadow under the player.
	client_entity_t* self = ClientEntity_new(type, flags, origin, NULL, INT_MAX);

	self->radius = SHADOW_CHECK_DIST;
	self->r.model = &shadow_model;
	self->r.flags = RF_FULLBRIGHT | RF_TRANSLUCENT | RF_ALPHA_TEXTURE;
	self->r.scale = scale;
	self->nextThinkTime = INT_MAX;
	self->AddToView = ShadowUpdate;

	AddEffect(owner, self);
	ShadowUpdate(self, owner);
}

// Cast a shadow down from each foot and the player, too.
void FXPlayerShadow(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	// Create shadow under the player.
	client_entity_t* shadow_mid = ClientEntity_new(type, flags, origin, NULL, INT_MAX);

	shadow_mid->radius = SHADOW_CHECK_DIST;
	shadow_mid->r.model = &shadow_model;
	shadow_mid->r.flags = RF_FULLBRIGHT | RF_TRANSLUCENT | RF_ALPHA_TEXTURE;
	shadow_mid->nextThinkTime = INT_MAX;
	shadow_mid->AddToView = ShadowUpdate;

	AddEffect(owner, shadow_mid);
	ShadowUpdate(shadow_mid, owner);

	// Create shadow under the left foot.
	client_entity_t* shadow_left = ClientEntity_new(type, flags, origin, NULL, INT_MAX);

	shadow_left->radius = SHADOW_CHECK_DIST;
	shadow_left->r.model = &shadow_model;
	shadow_left->r.flags = RF_FULLBRIGHT | RF_TRANSLUCENT | RF_ALPHA_TEXTURE;
	shadow_left->nextThinkTime = INT_MAX;
	shadow_left->refPoint = CORVUS_LEFTFOOT;
	shadow_left->r.scale = 0.8f;
	shadow_left->AddToView = ShadowReferenceUpdate;

	AddEffect(owner, shadow_left);
	ShadowUpdate(shadow_left, owner);

	// Create shadow under the right foot.
	client_entity_t* shadow_right = ClientEntity_new(type, flags, origin, NULL, INT_MAX);

	shadow_right->radius = SHADOW_CHECK_DIST;
	shadow_right->r.model = &shadow_model;
	shadow_right->r.flags = RF_FULLBRIGHT | RF_TRANSLUCENT | RF_ALPHA_TEXTURE;
	shadow_right->nextThinkTime = INT_MAX;
	shadow_right->refPoint = CORVUS_RIGHTFOOT;
	shadow_right->r.scale = 0.8f;
	shadow_right->AddToView = ShadowReferenceUpdate;

	AddEffect(owner, shadow_right);
	ShadowUpdate(shadow_right, owner);
}