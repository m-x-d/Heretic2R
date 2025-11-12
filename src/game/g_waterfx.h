//
// g_waterfx.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "g_Edict.h"

extern void SP_env_water_drip(edict_t* self);
extern void SP_env_water_fountain(edict_t* self);
extern void SP_env_waterfall_base(edict_t* self);
extern void SP_obj_fishhead1(edict_t* self);
extern void SP_obj_fishhead2(edict_t* self);
extern void SP_env_mist(edict_t* self);
extern void SP_env_bubbler(edict_t* self);

//mxd. Required by save system...
extern void EnvWaterDripThink(edict_t* self);
extern void EnvWaterDripUse(edict_t* self, edict_t* other, edict_t* activator);
extern void EnvWaterFountainUse(edict_t* self, edict_t* other, edict_t* activator);