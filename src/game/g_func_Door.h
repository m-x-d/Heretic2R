//
// g_func_Door.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "g_Edict.h"

//mxd. Here, because SF_DOOR_CRUSHER is used by FuncPlatBlocked()...
#define SF_DOOR_START_OPEN		1
#define SF_DOOR_REVERSE			2
#define SF_DOOR_CRUSHER			4
#define SF_DOOR_NOMONSTER		8
#define SF_DOOR_ANIMATED		16
#define SF_DOOR_TOGGLE			32
#define SF_DOOR_ANIMATED_FAST	64 //mxd
#define SF_DOOR_SWINGAWAY		8192

extern void SP_func_door(edict_t* self);
extern void SP_func_door_rotating(edict_t* ent);
extern void SP_func_water(edict_t* self);
extern void SP_func_door_secret(edict_t* ent);

extern void FuncDoorStaticsInit(void);
extern void FuncDoorSetSounds(edict_t* ent);

//mxd. Required by save system...
extern void FuncDoorBlocked(edict_t* self, edict_t* other);
extern void FuncDoorTriggerTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf);
extern void FuncDoorCalcMoveSpeedThink(edict_t* self);
extern void FuncDoorDie(edict_t* self, edict_t* inflictor, edict_t* attacker, int damage, const vec3_t point);
extern void FuncDoorTouch(edict_t* self, trace_t* trace);
extern void FuncDoorSecretBlocked(edict_t* self, edict_t* other);
extern void FuncDoorSecretDie(edict_t* self, edict_t* inflictor, edict_t* attacker, int damage, const vec3_t point);
extern void FuncDoorSecretUse(edict_t* self, edict_t* other, edict_t* activator);
extern void FuncDoorSpawnDoorTriggerThink(edict_t* self);
extern void FuncDoorUse(edict_t* self, edict_t* other, edict_t* activator);
extern void FuncDoorGoDown(edict_t* self);
extern void FuncDoorSecretMove2(edict_t* self);
extern void FuncDoorSecretMove4(edict_t* self);
extern void FuncDoorSecretMove6(edict_t* self);
extern void FuncDoorHitBottom(edict_t* self);
extern void FuncDoorHitTop(edict_t* self);
extern void FuncDoorSecretMove1(edict_t* self);
extern void FuncDoorSecretMove3(edict_t* self);
extern void FuncDoorSecretMove5(edict_t* self);
extern void FuncDoorSecretDone(edict_t* self);

//mxd. Local forward declarations for g_func_Door.c:
static void FuncDoorGoUp(edict_t* self, edict_t* activator);