//
// g_field.h
//
// Copyright 2025 mxd
//

#pragma once

#include "g_Edict.h"

extern void SP_trigger_fogdensity(edict_t* self);
extern void SP_trigger_push(edict_t* self);
extern void SP_trigger_Damage(edict_t* self);
extern void SP_trigger_Gravity(edict_t* self);
extern void SP_trigger_MonsterJump(edict_t* self);
extern void SP_trigger_goto_buoy(edict_t* self);

extern void TrigPushStaticsInit(void);
extern void TrigDamageStaticsInit(void);