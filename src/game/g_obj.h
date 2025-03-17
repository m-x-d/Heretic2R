//
// g_obj.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "FX.h"
#include "q_Typedef.h"

// Spawnflags for object entities.
#define OBJ_INVULNERABLE	1
#define OBJ_ANIMATE			2
#define OBJ_EXPLODING		4
#define OBJ_NOPUSH			8

extern void BboxYawAndScale(edict_t* self);

extern void DefaultObjectDieHandler(edict_t* self, struct G_Message_s* msg);
extern void ObjectInit(edict_t* self, int health, int mass, MaterialID_t material_type, int solid);
extern void ObjectStaticsInit(void);
extern void LeverStaticsInit(void);