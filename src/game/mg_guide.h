//
// mg_guide.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "g_Edict.h"

#define MONSTER_SEARCH_TIME		10 // Monsters search for player for 10 seconds after losing him before giving up.

extern void MG_InitMoods(edict_t* self); //mxd
extern qboolean clear_visible_pos(edict_t* self, vec3_t spot2); //mxd
extern void MG_Pathfind(edict_t* self, qboolean check_clear_path); //mxd
extern void MG_BuoyNavigate(edict_t* self); //mxd
extern qboolean MG_GoToRandomBuoy(edict_t* self); //mxd
extern qboolean MG_ReachedBuoy(edict_t* self, vec3_t pspot);
extern void MG_RemoveBuoyEffects(edict_t* self); //mxd
extern qboolean MG_CheckClearPathToEnemy(edict_t* self); //mxd
extern qboolean MG_MakeConnection(edict_t* self, buoy_t* first_buoy, qboolean skipjump); //mxd

//mxd. Local forward declarations for mg_guide.c:
static qboolean MG_CheckClearPathToSpot(edict_t* self, vec3_t spot);