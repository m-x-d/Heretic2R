//
// g_monster.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "g_ai.h" //mxd
#include "g_ClassStatics.h"
#include "g_combat.h" //mxd
#include "m_move.h" //mxd

// Message handlers.
extern void DeadMsgHandler(edict_t* self, G_Message_t* msg);
extern void DyingMsgHandler(edict_t* self, G_Message_t* msg);
extern void DismemberMsgHandler(edict_t* self, G_Message_t* msg);

extern void M_WorldEffects(struct edict_s* ent);
extern void M_CatagorizePosition(edict_t* ent);
extern void MG_SetNormalizedVelToGoal(edict_t* self, vec3_t vec);
extern void M_DropToFloor(edict_t* ent);
extern void M_MoveFrame(edict_t* self);
extern int MonsterHealth(int health);

extern void M_Think(edict_t* self);
extern void M_Use(struct edict_s* self, struct edict_s* other, struct edict_s* activator);
extern void M_DeathUse(edict_t* self);
extern qboolean M_Start(edict_t* self);

extern qboolean M_WalkmonsterStart(edict_t* self);
extern void M_WalkmonsterStartGo(edict_t* self);
extern qboolean M_FlymonsterStart(edict_t* self);

extern void M_GetSlopePitchRoll(edict_t* ent, vec3_t pass_slope); //mxd

// JWEIER START HELPER PROTO
extern qboolean M_ValidTarget(edict_t* self, const edict_t* target);
extern qboolean M_CheckAlert(const edict_t* self, int range);

extern edict_t* M_CheckMeleeLineHit(edict_t* attacker, const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, trace_t* trace, vec3_t direction);
extern edict_t* M_CheckMeleeHit(edict_t* attacker, float max_dist, trace_t* trace);

extern float M_DistanceToTarget(const edict_t* self, const edict_t* target);

extern void M_Touch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf);
extern void M_StartDeath(edict_t* self, int animID);
extern void M_EndDeath(edict_t* self);
extern void M_PredictTargetPosition(const edict_t* target, const vec3_t evade_vel, float pred_frames, vec3_t pred_target_pos);
extern int M_PredictTargetEvasion(const edict_t* attacker, const edict_t* target, const vec3_t pursue_vel, const vec3_t evade_vel, float strike_dist, float pred_frames);
extern void M_jump(edict_t* self, G_Message_t* msg);
extern void M_ShowLifeMeter(int value, int max_value);
extern int M_FindSupport(const edict_t* self, int range);
extern void M_DeadFloatThink(edict_t* self); //mxd

//mxd. Required by save system...
extern void M_DeadBobThink(edict_t* self);
extern void M_FlymonsterStartGo(edict_t* self);
extern void M_TriggeredSpawnThink(edict_t* self);
extern void M_TriggeredSpawnUse(edict_t* self, edict_t* other, edict_t* activator);
extern qboolean GenericMonsterAlerted(edict_t* self, alertent_t* alerter, edict_t* enemy);

//mxd. Local forward declarations for g_monster.c:
static void M_MonsterStartGo(edict_t* self);