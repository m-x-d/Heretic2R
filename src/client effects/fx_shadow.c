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
#define SHADOW_REF_CHECK_DIST	16.0f //mxd. 64.0f in original logic. Seems way too far for feet shadows.
#define SHADOW_MIN_LIGHTLEVEL	25 //mxd. Don't draw shadow when cmd.lightlevel < this.
#define SHADOW_MAX_LIGHTLEVEL	150 //mxd. Matches value in R_SetLightLevel().

static struct model_s* shadow_model;

void PreCacheShadow(void) //mxd. Named 'PrecacheShadow' in original logic.
{
	shadow_model = fxi.RegisterModel("models/fx/shadow/tris.fm");
}

static qboolean ShadowAddToView(client_entity_t* self, centity_t* owner)
{
	VectorCopy(owner->origin, self->r.origin);

	const vec3_t start_pos = VEC3_INITA(owner->origin, 0.0f, 0.0f, 4.0f);
	const vec3_t end_pos =   VEC3_INITA(owner->origin, 0.0f, 0.0f, -SHADOW_CHECK_DIST);

	// Determine visibility.
	trace_t trace;
	fxi.Trace(start_pos, vec3_origin, vec3_origin, end_pos, CONTENTS_SOLID, CEF_CLIP_TO_ALL, &trace); //mxd. CEF_CLIP_TO_WORLD in original logic. Fixes shadows disappearing when standing on brush entities (like func_wall or func_breakable).

	// In air or in solid, so no shadow.
	if (trace.startsolid || trace.fraction == 1.0f)
		return false;

	// Did hit the ground.
	const float owner_alpha = (float)owner->current.color.a / 255.0f; //mxd
	self->alpha = (1.0f - trace.fraction) * owner_alpha * 0.5f + 0.01f;

	// If we are in ref soft, bring us out a touch, since we are having z buffer problems.
	const float offset = (ref_soft ? 0.9f : 0.2f); //mxd

	// Raise the shadow slightly off the target wall.
	VectorMA(trace.endpos, offset, trace.plane.normal, self->r.origin);
	AnglesFromDirI(trace.plane.normal, self->r.angles);

	return true;
}

static float GetPlayerShadowAlphaScaler(const centity_t* player) //mxd
{
	// Factor in player lightlevel.
	float alpha_scaler = (float)(fxi.cl->cmd.lightlevel - SHADOW_MIN_LIGHTLEVEL) / (float)(SHADOW_MAX_LIGHTLEVEL - SHADOW_MIN_LIGHTLEVEL) * 1.2f;

	// Factor in player ghost effect.
	if (player->current.renderfx & RF_TRANS_GHOST)
		alpha_scaler *= 0.25f;

	return alpha_scaler;
}

static qboolean PlayerShadowAddToView(client_entity_t* self, centity_t* owner) //mxd
{
	// Too dark. Show no shadows.
	if (fxi.cl->cmd.lightlevel < SHADOW_MIN_LIGHTLEVEL)
	{
		VectorCopy(owner->origin, self->r.origin);
		return false;
	}

	if (!ShadowAddToView(self, owner))
		return false;

	//mxd. Factor in player lightlevel and ghost effect.
	self->alpha = Clamp(self->alpha * GetPlayerShadowAlphaScaler(owner) * 1.2f, 0.01f, 1.0f);

	return true;
}

static qboolean ShadowReferenceAddToView(client_entity_t* self, centity_t* owner)
{
	VectorCopy(owner->origin, self->r.origin);

	// This tells if we are wasting our time, because the reference points are culled.
	if (!RefPointsValid(owner) || fxi.cl->cmd.lightlevel < SHADOW_MIN_LIGHTLEVEL) //mxd. No shadows in the dark (looks more natural, no?)
		return false;

	matrix3_t rotation;
	Matrix3FromAngles(owner->lerp_angles, rotation);

	// Take the start_pos from one of the reference points.
	vec3_t start_pos;
	Matrix3MultByVec3(rotation, owner->referenceInfo->references[self->refPoint].placement.origin, start_pos);

	// This may look weird, but by scaling the vector, I bring it closer to the center of the owner.
	Vec3ScaleAssign(0.95f, start_pos); //mxd. 0.5 in original version.
	Vec3AddAssign(owner->origin, start_pos);

	// Now trace from the start_pos down.
	const vec3_t end_pos = VEC3_INITA(start_pos, 0.0f, 0.0f, -SHADOW_REF_CHECK_DIST);

	// Determine visibility.
	trace_t trace;
	fxi.Trace(start_pos, vec3_origin, vec3_origin, end_pos, CONTENTS_SOLID, CEF_CLIP_TO_WORLD, &trace);

	// In air or in solid, so no shadow.
	if (trace.startsolid || trace.fraction == 1.0f)
		return false;

	// Did hit the ground.
	self->alpha = Clamp((1.0f - trace.fraction) * GetPlayerShadowAlphaScaler(owner), 0.01f, 1.0f); //mxd. Factor in player lightlevel and ghost effect.

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

	if (R_DETAIL < DETAIL_UBERHIGH)
		return;

	// Create shadow under the monster.
	client_entity_t* shadow = ClientEntity_new(type, flags, origin, NULL, INT_MAX);

	shadow->radius = SHADOW_CHECK_DIST;
	shadow->r.model = &shadow_model;
	shadow->r.flags = (RF_FULLBRIGHT | RF_TRANSLUCENT | RF_ALPHA_TEXTURE);
	shadow->r.scale = scale;
	shadow->nextThinkTime = INT_MAX;
	shadow->AddToView = ShadowAddToView;

	AddEffect(owner, shadow);
}

// Cast a shadow down from each foot and the player, too.
void FXPlayerShadow(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	// Create shadow under the player.
	client_entity_t* shadow_mid = ClientEntity_new(type, flags, origin, NULL, INT_MAX);

	shadow_mid->radius = SHADOW_CHECK_DIST;
	shadow_mid->r.model = &shadow_model;
	shadow_mid->r.flags = (RF_FULLBRIGHT | RF_TRANSLUCENT | RF_ALPHA_TEXTURE);
	shadow_mid->r.scale = 0.9f; //mxd. 1.0 in original logic.
	shadow_mid->nextThinkTime = INT_MAX;
	shadow_mid->AddToView = PlayerShadowAddToView;

	AddEffect(owner, shadow_mid);

	// Create shadow under the left foot.
	client_entity_t* shadow_left = ClientEntity_new(type, flags, origin, NULL, INT_MAX);

	shadow_left->radius = SHADOW_CHECK_DIST;
	shadow_left->r.model = &shadow_model;
	shadow_left->r.flags = (RF_FULLBRIGHT | RF_TRANSLUCENT | RF_ALPHA_TEXTURE);
	shadow_left->r.scale = 0.45f; //mxd. 0.8 in original logic.
	shadow_left->refPoint = CORVUS_LEFTFOOT;
	shadow_left->nextThinkTime = INT_MAX;
	shadow_left->AddToView = ShadowReferenceAddToView;

	AddEffect(owner, shadow_left);

	// Create shadow under the right foot.
	client_entity_t* shadow_right = ClientEntity_new(type, flags, origin, NULL, INT_MAX);

	shadow_right->radius = SHADOW_CHECK_DIST;
	shadow_right->r.model = &shadow_model;
	shadow_right->r.flags = (RF_FULLBRIGHT | RF_TRANSLUCENT | RF_ALPHA_TEXTURE);
	shadow_right->r.scale = 0.45f; //mxd. 0.8 in original logic.
	shadow_right->refPoint = CORVUS_RIGHTFOOT;
	shadow_right->nextThinkTime = INT_MAX;
	shadow_right->AddToView = ShadowReferenceAddToView;

	AddEffect(owner, shadow_right);
}