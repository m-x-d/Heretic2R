//
// g_func_Rotating.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "g_Edict.h"

extern void SP_func_rotating(edict_t* ent);
extern void FuncRotateStaticsInit(void);

//mxd. Required by save system...
extern void FuncRotatingBlocked(edict_t* self, edict_t* other);
extern void FuncRotatingTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf);
extern void FuncRotatingUse(edict_t* self, edict_t* other, edict_t* activator);