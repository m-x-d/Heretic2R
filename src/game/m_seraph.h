//
// m_seraph.h -- mxd. Added public header.
//
// Copyright 2025 mxd
//

#pragma once

#include "g_Edict.h"

extern void SP_monster_seraph_overlord(edict_t* self);
extern void SeraphOverlordStaticsInit(void);

//mxd. Required by save system...
extern qboolean SeraphAlert(edict_t* self, alertent_t* alerter, edict_t* enemy);
extern void SeraphDismember(edict_t* self, int damage, HitLocation_t hl);