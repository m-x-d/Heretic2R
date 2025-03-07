//
// g_field.h
//
// Copyright 2025 mxd
//

#pragma once

#include "g_Edict.h"

extern void SP_trigger_fogdensity(edict_t* self);
extern void SP_trigger_push(edict_t* self);
extern void SP_trigger_damage(edict_t* self);
extern void SP_trigger_gravity(edict_t* self);
extern void SP_trigger_monsterjump(edict_t* self);
extern void SP_trigger_goto_buoy(edict_t* self);

extern void TriggerPushStaticsInit(void);
extern void TriggerDamageStaticsInit(void);