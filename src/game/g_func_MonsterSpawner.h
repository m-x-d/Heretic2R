//
// g_func_MonsterSpawner.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "g_Edict.h"

extern void SP_func_monsterspawner(edict_t* self);
extern void SP_monster_chkroktk(edict_t* self);
extern void SP_character_sidhe_guard(edict_t* self);

//mxd. Required by save system...
extern void FuncMonsterSpawnerGo(edict_t* self);
extern void FuncMonsterSpawnerUse(edict_t* self, edict_t* other, edict_t* activator);