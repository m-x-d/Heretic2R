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

//mxd. Required by save system...
extern void FogDensityTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf);
extern void TriggerDamageUse(edict_t* self, edict_t* other, edict_t* activator);
extern void TriggerDamageTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf);
extern void TriggerGotoBuoyTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf);
extern void TriggerGotoBuoyTouchThink(edict_t* self);
extern void TriggerGotoBuoyUse(edict_t* self, edict_t* other, edict_t* activator);
extern void TriggerGotoBuoyUseThink(edict_t* self);
extern void TriggerGravityTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf);
extern void TriggerMonsterJumpTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf);
extern void TriggerPushActivated(edict_t* self, edict_t* activator);
extern void TriggerPushTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf);