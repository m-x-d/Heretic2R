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
extern void PlagueElfDyingSound(const edict_t* self, int type);

//mxd. Required by save system...
extern void PlagueElfDismember(edict_t* self, int damage, HitLocation_t hl);
extern void PlagueElfSpellTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surface);
extern void PlagueElfPhaseOutPreThink(edict_t* self);
extern void PlagueElfPhaseInPreThink(edict_t* self);