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

//mxd. Required by save system...
extern void TargetCrosslevelTargetThink(edict_t* self);
extern void TargetCrosslevelTriggerUse(edict_t* self, edict_t* other, edict_t* activator);
extern void TargetEarthquakeThink(edict_t* self);
extern void TargetEarthquakeUse(edict_t* self, edict_t* other, edict_t* activator);
extern void TargetExplosionExplodeThink(edict_t* self);
extern void TargetExplosionUse(edict_t* self, edict_t* other, edict_t* activator);
extern void TargetLightrampThink(edict_t* self);
extern void TargetLightrampUse(edict_t* self, edict_t* other, edict_t* activator);
extern void TargetTempEntityUse(edict_t* ent, edict_t* other, edict_t* activator);