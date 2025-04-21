//
// m_gorgon.h -- mxd. Added public header.
//
// Copyright 2025 mxd
//

#pragma once

#include "g_Edict.h"

extern void SP_monster_gorgon(edict_t* self);
extern void SP_monster_gorgon_leader(edict_t* self);
extern void GorgonStaticsInit(void);

//mxd. Static forward declarations for m_gorgon.c
static void GorgonPreThink(edict_t* self);