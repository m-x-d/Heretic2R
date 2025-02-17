//
// Utilities.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "q_Typedef.h"

#define KNOCK_BACK_MULTIPLIER 1000.0f

extern edict_t* FindNearestVisibleActorInFrustum(const edict_t* finder, const vec3_t finder_angles,
	float near_dist, float far_dist, double h_fov, double v_fov, long flags, const vec3_t los_start_pos, const vec3_t bb_min, const vec3_t bb_max);

extern edict_t* FindSpellTargetInRadius(const edict_t* search_ent, float radius, const vec3_t search_pos, const vec3_t mins, const vec3_t maxs);

extern void SetAnim(edict_t* self, int anim);
extern void PostKnockBack(edict_t* target, const vec3_t dir, float knockback, int flags);
extern void GetAimVelocity(const edict_t* enemy, const vec3_t org, float speed, const vec3_t aim_angles, vec3_t out);
extern void CalculatePIV(const edict_t* player);

extern qboolean ok_to_autotarget(const edict_t* shooter, const edict_t* target);
extern qboolean ThinkTime(const edict_t* self);