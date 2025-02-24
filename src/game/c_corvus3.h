//
// c_corvus3.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "q_Typedef.h"

extern void Corvus3CinStaticsInit(void); //mxd
extern void SP_character_corvus3(edict_t* self); //mxd

extern void corvus3_teleporteffect(edict_t* self);
extern void corvus3_teleportsmalleffect(edict_t* self);
extern void corvus3_visible(edict_t* self);
extern void corvus3_invisible(edict_t* self);