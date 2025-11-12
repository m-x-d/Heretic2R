//
// m_harpy.h -- mxd. Added public header.
//
// Copyright 2025 mxd
//

#pragma once

#include "g_Edict.h"

extern edict_t* harpy_head_carrier;
extern edict_t* harpy_head_source;

extern void SP_monster_harpy(edict_t* self);
extern void HarpyStaticsInit(void);
extern void HarpyTakeHead(edict_t* self, edict_t* victim, int bodypart_node_id, int frame, int flags);

//mxd. Required by save system...
extern void HarpyDismember(edict_t* self, int damage, HitLocation_t hl);
extern void HarpyHeadDie(edict_t* self, edict_t* inflictor, edict_t* attacker, int damage, const vec3_t point);
extern void HarpyHeadThink(edict_t* self);
extern void HarpyIsBlocked(edict_t* self, trace_t* trace);