//
// g_ai.h
//
// Copyright 2025 mxd
//

#pragma once

#include "q_Typedef.h"

extern void AlertMonsters(const edict_t* self, edict_t* enemy, float lifetime, qboolean ignore_shadows);
extern void AlertMonstersAt(const vec3_t alert_origin, edict_t* enemy, float lifetime, int alert_svflags); //mxd
extern void AI_SetSightClient(void);
extern void AI_FoundTarget(edict_t* self, qboolean set_sight_ent);
extern qboolean M_CheckAttack(edict_t* self);
extern void ExtrapolateFireDirection(const edict_t* self, const vec3_t origin, float proj_speed, const edict_t* target, float accepted_dot, vec3_t out_pos);
extern void AI_Flee(edict_t* self, float dist);
extern float AI_FaceGoal(edict_t* self);
extern qboolean AI_HaveEnemy(edict_t* self);
extern qboolean AI_IsVisible(const edict_t* self, const edict_t* other);
extern qboolean AI_IsClearlyVisible(const edict_t* self, const edict_t* other);
extern qboolean AI_IsInfrontOf(const edict_t* self, const edict_t* other);
extern qboolean AI_IsMovable(const edict_t* ent);
extern qboolean AI_OkToWake(const edict_t* monster, qboolean gorgon_roar, qboolean ignore_ambush);

// Action functions.
extern void ai_stand(edict_t* self, float dist);
extern void ai_move(edict_t* self, float dist);
extern void ai_walk(edict_t* self, float dist);
extern void ai_charge(edict_t* self, float dist);
extern void ai_charge2(edict_t* self, float dist);
extern void ai_eat(edict_t* self, float dist);
extern void ai_spin(edict_t* self, float amount);
extern void ai_moveright(edict_t* self, float dist);
extern void ai_goal_charge(edict_t* self, float dist);