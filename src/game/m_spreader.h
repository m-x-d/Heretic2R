//
// m_spreader.h -- mxd. Added public header.
//
// Copyright 2025 mxd
//

#pragma once

#include "g_Edict.h"

extern void SP_monster_spreader(edict_t* self);
extern void SpreaderStaticsInit(void);

// Local forward declarations for m_spreader.c.
static void SpreaderIsBlocked(edict_t* self, trace_t* trace);