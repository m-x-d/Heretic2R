//
// m_morcalavin.h -- mxd. Added public header.
//
// Copyright 2025 mxd
//

#pragma once

#include "g_Edict.h"

extern void SP_monster_morcalavin(edict_t* self);
extern void MorcalavinStaticsInit(void);

// Local forward declarations for m_morcalavin.c.
static void MorcalavinProjectileInit(edict_t* self, edict_t* proj);
static void MorcalavinPhaseOutPreThink(edict_t* self);
static void MorcalavinPhaseInPreThink(edict_t* self);
static void MorcalavinLaughPostThink(edict_t* self);