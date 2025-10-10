//
// Utilities.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "q_Typedef.h"

#define KNOCK_BACK_MULTIPLIER 1000.0f

extern float NormalizeAngleRad(float angle); //mxd
extern float NormalizeAngleDeg(float angle); //mxd

extern edict_t* FindNearestVisibleActorInFrustum(const edict_t* finder, const vec3_t finder_angles,	float near_dist, float far_dist, float h_fov, float v_fov, qboolean scale_fov_by_dist, const vec3_t los_start_pos); //mxd. Remove 'bb_min' and 'bb_max' args (never used), add 'scale_fov_by_dist' arg.
extern edict_t* FindSpellTargetInRadius(const edict_t* search_ent, float radius, const vec3_t search_pos, const vec3_t mins, const vec3_t maxs);

extern void SetAnim(edict_t* self, int anim);
extern void PostKnockBack(edict_t* target, const vec3_t dir, float knockback, int flags);
extern void GetAimVelocity(const edict_t* enemy, const vec3_t org, float speed, const vec3_t aim_angles, vec3_t out);
extern void AdjustAimDirection(const edict_t* caster, const vec3_t start_pos, const vec3_t aim_angles, float projectle_speed, float view_offset_z, vec3_t direction); //mxd
extern void AdjustAimVelocity(const edict_t* caster, const vec3_t start_pos, const vec3_t aim_angles, float projectle_speed, float view_offset_z, vec3_t velocity); //mxd
extern void AdjustAimAngles(const edict_t* caster, const vec3_t start_pos, const vec3_t aim_angles, float projectle_speed, float view_offset_z, vec3_t angles); //mxd
extern void CalculatePIV(const edict_t* player);

extern qboolean OkToAutotarget(const edict_t* shooter, const edict_t* target);
extern qboolean ThinkTime(const edict_t* self);

//mxd. Defined in g_misc.c in original logic.
extern void SpawnClientAnim(edict_t* self, byte type, const char* sound);
extern qboolean EntReflecting(const edict_t* ent, qboolean check_monster, qboolean check_player);
extern void SkyFly(edict_t* self);