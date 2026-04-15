//
// g_func_Plat.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "g_Edict.h"

extern void SP_func_plat(edict_t* ent);

//mxd. Required by save system...
extern void FuncPlatBlocked(edict_t* self, edict_t* other);
extern void FuncPlatCenterTouch(edict_t* ent, edict_t* other, cplane_t* plane, csurface_t* surf);
extern void FuncPlatUse(edict_t* ent, edict_t* other, edict_t* activator);
extern void FuncPlatGoDown(edict_t* ent);
extern void FuncPlatHitBottom(edict_t* ent);
extern void FuncPlatHitTop(edict_t* ent);