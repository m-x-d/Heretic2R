//
// m_spreader.h -- mxd. Added public header.
//
// Copyright 2025 mxd
//

#pragma once

#include "g_Edict.h"

extern void SP_monster_spreader(edict_t* self);
extern void SpreaderStaticsInit(void);

//mxd. Required by save system...
extern void SpreaderDismember(edict_t* self, int damage, HitLocation_t hl);
extern void SpreaderDropDownThink(edict_t* self);
extern void SpreaderIsBlocked(edict_t* self, trace_t* trace);
extern void SpreaderSplatWhenBlocked(edict_t* self, trace_t* trace);
extern void SpreaderStopWhenBlocked(edict_t* self, trace_t* trace);