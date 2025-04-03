//
// m_assassin_local.h -- Local forward declarations for m_assassin.c.
//
// Copyright 2025 mxd
//

#pragma once

#include "q_Typedef.h"

static void AssassinDaggerInit(edict_t* dagger);
static void AssassinDropWeapon(edict_t* self, int whichknives);
static qboolean AssassinCheckTeleport(edict_t* self, int type);
static qboolean AssassinChooseTeleportDestination(edict_t* self, int type, qboolean imperative, qboolean instant);
static void AssassinCloakFadePreThink(edict_t* self);
static void assassinInitDeCloak(edict_t* self);
static void AssassinDeCloakFadePreThink(edict_t* self);