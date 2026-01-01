//
// g_env.h
//
// Copyright 2025 mxd
//

#pragma once

#include "g_Edict.h"

extern void SP_env_dust(edict_t* self);
extern void SP_env_muck(edict_t* self);
extern void SP_env_smoke(edict_t* self);
extern void SP_env_sun1(edict_t* self);

//mxd. Required by save system...
extern void EnvDustUse(edict_t* self, edict_t* other, edict_t* activator);
extern void EnvSmokeUse(edict_t* self, edict_t* other, edict_t* activator);
extern void EnvSunInitThink(edict_t* self);