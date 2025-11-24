//
// p_teleport.h -- mxd. Originally part of spl_teleport.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "g_Edict.h"

#define TELE_TIME		4						// Number of server frames we take to do the fades.
#define TELE_FADE		(255 / TELE_TIME)		// Amount to fade the player by each fade.

#define TELE_TIME_OUT	5						// Number of server frames we take to do the fades.
#define TELE_FADE_OUT	(255 / TELE_TIME_OUT)	// Amount to fade the player by each fade.

#define TELE_PM_DURATION	137 //mxd. Teleport/morph duration in pm_time units... ~= 1100 / 8 //TODO: why does this take 1100 ms. instead of expected 900 (TELE_TIME + TELE_TIME_OUT * 100)?..

#define DEATHMATCH_RANDOM	2

extern void CleanUpPlayerTeleport(edict_t* self);
extern void PerformPlayerTeleport(edict_t* self);
extern void PlayerTeleporterTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf);