//
// g_func_Train.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "g_Edict.h"

extern void SP_func_train(edict_t* self);
extern void FuncTrainResume(edict_t* self);

//mxd. Local forward declarations for g_func_Train.c:
static void FuncTrainNext(edict_t* self);