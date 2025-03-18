//
// g_target.h
//
// Copyright 2025 mxd
//

#pragma once

#include "q_Typedef.h"

extern void TargetChangelevelUse(edict_t* self, edict_t* other, edict_t* activator);

extern void SP_target_temp_entity(edict_t* ent);
extern void SP_target_explosion(edict_t* ent);
extern void SP_target_changelevel(edict_t* ent);
extern void SP_target_crosslevel_trigger(edict_t* self);
extern void SP_target_crosslevel_target(edict_t* self);
extern void SP_target_lightramp(edict_t* self);
extern void SP_target_earthquake(edict_t* self);