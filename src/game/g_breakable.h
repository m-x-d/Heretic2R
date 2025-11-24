//
// g_breakable.h
//
// Copyright 2025 mxd
//

#pragma once

#include "g_Edict.h"

extern void BBrushStaticsInit(void);
extern void KillBrush(edict_t* target, edict_t* inflictor, edict_t* attacker, int damage);
extern void SP_breakable_brush(edict_t* ent);

//mxd. Required by save system...
extern void BreakableBrushUse(edict_t* target, edict_t* inflictor, edict_t* attacker);
extern void LinkBreakableBrushesThink(edict_t* self);