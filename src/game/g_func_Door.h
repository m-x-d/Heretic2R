//
// g_func_Door.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "g_Edict.h"

//mxd. Here, because SF_DOOR_CRUSHER is used by FuncPlatBlocked()...
#define SF_DOOR_START_OPEN	1
#define SF_DOOR_REVERSE		2
#define SF_DOOR_CRUSHER		4
#define SF_DOOR_NOMONSTER	8
#define SF_DOOR_TOGGLE		32
#define SF_DOOR_X_AXIS		64
#define SF_DOOR_Y_AXIS		128
#define SF_DOOR_SWINGAWAY	8192

extern void SP_func_door(edict_t* self);
extern void SP_func_door_rotating(edict_t* ent);
extern void SP_func_water(edict_t* self);
extern void SP_func_door_secret(edict_t* ent);

extern void FuncDoorStaticsInit(void);
extern void FuncDoorSetSounds(edict_t* ent);

//mxd. Local forward declarations for g_func_Door.c:
static void FuncDoorGoDown(edict_t* self);
static void FuncDoorGoUp(edict_t* self, edict_t* activator);

static void FuncDoorSecretMove1(edict_t* self);
static void FuncDoorSecretMove2(edict_t* self);
static void FuncDoorSecretMove3(edict_t* self);
static void FuncDoorSecretMove4(edict_t* self);
static void FuncDoorSecretMove5(edict_t* self);
static void FuncDoorSecretMove6(edict_t* self);
static void FuncDoorSecretDone(edict_t* self);