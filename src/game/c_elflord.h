//
// c_elflord.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "q_Typedef.h"

extern void ElflordCinStaticsInit(void); //mxd
extern void SP_character_elflord(edict_t* self); //mxd

extern void elflord_c_boom(edict_t* self); //mxd
extern void elflord_c_throwhead(edict_t* self); //mxd
extern void elflord_c_mist(edict_t* self, float x, float y, float z); //mxd