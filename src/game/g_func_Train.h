//
// g_func_Train.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "g_Edict.h"

extern void SP_func_train(edict_t* self);
extern void FuncTrainResume(edict_t* self);

//mxd. Required by save system...
extern void FuncTrainAnim(edict_t* self);
extern void FuncTrainAnimBackwards(edict_t* self);
extern void FuncTrainBlocked(edict_t* self, edict_t* other);
extern void FuncTrainFind(edict_t* self);
extern void FuncTrainUse(edict_t* self, edict_t* other, edict_t* activator);
extern void FuncTrainNext(edict_t* self);
extern void FuncTrainWait(edict_t* self);