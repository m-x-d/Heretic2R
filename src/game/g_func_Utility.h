//
// g_func_Utility.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "g_Edict.h"

#define STATE_TOP			0
#define STATE_BOTTOM		1
#define STATE_UP			2
#define STATE_DOWN			3

extern void MoveCalc(edict_t* ent, const vec3_t dest, void(*func)(edict_t*));
extern void AngleMoveCalc(edict_t* ent, void(*func)(edict_t*));
extern void FuncTrainAngleMoveCalc(edict_t* self, const edict_t* ent, const vec3_t dest);
extern void FuncPlayMoveStartSound(edict_t* ent); //mxd
extern void FuncPlayMoveEndSound(edict_t* ent); //mxd

//mxd. Required by save system...
extern void AccelMoveThink(edict_t* ent);
extern void AngleMoveBegin(edict_t* ent);
extern void AngleMoveDone(edict_t* ent);
extern void AngleMoveFinal(edict_t* ent);
extern void MoveBegin(edict_t* ent);
extern void MoveDone(edict_t* ent);
extern void MoveFinal(edict_t* ent);