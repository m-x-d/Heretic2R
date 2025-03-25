//
// g_monster.h
//
// Heretic II
// Copyright 1998 Raven Software
//

#ifndef G_MONSTER_H
#define G_MONSTER_H

#include "g_local.h" //TODO: remove.
#include "g_ai.h" //mxd
#include "g_combat.h" //mxd
#include "Message.h"
#include "g_ClassStatics.h"

extern void M_WorldEffects (struct edict_s *ent);
extern void PM_CatagorizePosition (void);

extern void monster_use (struct edict_s *self, struct edict_s *other, struct edict_s *activator);

int MonsterHealth(int health);

qboolean monster_start (edict_t *self);
void walkmonster_start_go (edict_t *self);
extern void pitch_roll_for_slope(edict_t* forwhom, vec3_t pass_slope); //mxd

void DeadMsgHandler(edict_t *self, G_Message_t *msg);

//JWEIER START HELPER PROTO

qboolean M_ValidTarget( edict_t *self, edict_t *target );
qboolean M_CheckAlert( edict_t *self, int range );

edict_t	*M_CheckMeleeLineHit( edict_t *attacker, vec3_t start, vec3_t end, vec3_t mins, vec3_t maxs, trace_t *trace, vec3_t direction);
edict_t	*M_CheckMeleeHit( edict_t *attacker, float max_dist, trace_t *trace );

float M_DistanceToTarget ( edict_t *self, edict_t *target );

void M_Touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
void M_StartDeath( edict_t *self, int animID);
void M_EndDeath( edict_t *self);
void M_PredictTargetPosition( edict_t *target, vec3_t evade_vel, float pred_frames, vec3_t	pTargetPos);
void M_jump(edict_t *self, G_Message_t *msg);
void MG_parse_dismember_msg(edict_t *self, G_Message_t *msg);
void M_ShowLifeMeter( edict_t *self, int value, int max_value );

int M_PredictTargetEvasion( edict_t *attacker, edict_t *target, vec3_t pursue_vel, vec3_t evade_vel, float strike_dist, float pred_frames );
int M_FindSupport( edict_t *self, int range );

//JWEIER END HELPER PROTO

#endif // G_MONSTER_H