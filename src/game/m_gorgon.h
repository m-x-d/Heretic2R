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

//mxd. Required by save system...
extern void GorgonPreThink(edict_t* self);
extern void GorgonRoarResponsePreThink(edict_t* self);