//
// mg_guide.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "g_Edict.h"

#define MONSTER_SEARCH_TIME		10 // Monsters search for player for 10 seconds after losing him before giving up.

extern void MG_InitMoods(edict_t* self); //mxd
extern void MG_RemoveBuoyEffects(edict_t* self); //mxd