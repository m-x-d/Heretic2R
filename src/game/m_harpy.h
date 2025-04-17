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