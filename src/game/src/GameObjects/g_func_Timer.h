//
// g_func_Timer.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "g_Edict.h"

extern void SP_func_timer(edict_t* self);

//mxd. Required by save system...
extern void FuncTimerThink(edict_t* self);
extern void FuncTimerUse(edict_t* self, edict_t* other, edict_t* activator);