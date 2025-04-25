//
// m_plagueElf.h -- mxd. Added public header.
//
// Copyright 2025 mxd
//

#pragma once

#include "g_Edict.h"

extern void SP_monster_plagueElf(edict_t* self);
extern void SP_monster_palace_plague_guard(edict_t* self);
extern void SP_monster_palace_plague_guard_invisible(edict_t* self);
extern void PlagueElfStaticsInit(void);
extern void PlagueElfDyingSound(edict_t* self, int type);

// Local forward declarations for m_plagueElf.c.
static void PlagueElfSpellTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surface);
static void PlagueElfPhaseOutPreThink(edict_t* self);
static void PlagueElfPhaseInPreThink(edict_t* self);