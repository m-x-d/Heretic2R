//
// g_func_Button.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "g_Edict.h"

extern void SP_func_button(edict_t* ent);
extern void ButtonStaticsInit(void);

//mxd. Local forward declarations for g_func_Button.c:
static void FuncButtonMove(edict_t* self);