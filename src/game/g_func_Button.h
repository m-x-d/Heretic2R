//
// g_func_Button.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "g_Edict.h"

extern void SP_func_button(edict_t* ent);
extern void FuncButtonStaticsInit(void);

//mxd. Required by save system...
extern void FuncButtonReturn(edict_t* self);
extern void FuncButtonTouch(edict_t* self, trace_t* trace);
extern void FuncButtonUse(edict_t* self, edict_t* other, edict_t* activator);
extern void FuncButtonDone(edict_t* self);
extern void FuncButtonWait(edict_t* self);