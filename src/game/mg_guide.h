//
// mg_guide.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "g_Edict.h"

#define MONSTER_SEARCH_TIME		10 // Monsters search for player for 10 seconds after losing him before giving up.

extern void MG_InitMoods(edict_t* self); //mxd
extern qboolean MG_IsClearlyVisiblePos(const edict_t* self, const vec3_t spot); //mxd
extern void MG_Pathfind(edict_t* self, qboolean check_clear_path); //mxd
extern int MG_SetFirstBuoy(edict_t* self); //mxd
extern void MG_BuoyNavigate(edict_t* self); //mxd
extern qboolean MG_GoToRandomBuoy(edict_t* self); //mxd
extern qboolean MG_ReachedBuoy(const edict_t* self, const vec3_t p_spot); //mxd
extern qboolean MG_CheckClearPathToEnemy(edict_t* self); //mxd
extern qboolean MG_MonsterAttemptTeleport(edict_t* self, const vec3_t destination, qboolean ignore_los); //mxd
extern qboolean MG_MakeConnection(edict_t* self, const buoy_t* first_buoy, qboolean skip_jump); //mxd

//mxd. Required by save system...
extern void MG_GenericMoodSet(edict_t* self);

//mxd. Local forward declarations for mg_guide.c:
static qboolean MG_CheckClearPathToSpot(edict_t* self, const vec3_t spot);