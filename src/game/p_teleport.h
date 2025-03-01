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

#define DEATHMATCH_RANDOM	2

void CleanUpPlayerTeleport(edict_t* self);
void PerformPlayerTeleport(edict_t* self);
void teleporter_touch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf);