//
// m_assassin_local.h -- Local forward declarations for m_assassin.c.
//
// Copyright 2025 mxd
//

#pragma once

#include "q_Typedef.h"

static void create_assassin_dagger(edict_t* Arrow);
static void assassin_dropweapon(edict_t* self, int whichknives);
static qboolean assassinCheckTeleport(edict_t* self, int type);
static qboolean assassinChooseTeleportDestination(edict_t* self, int type, qboolean imperative, qboolean instant);
static void assassinCloak(edict_t* self);
static void assassinInitDeCloak(edict_t* self);
static void assassinDeCloak(edict_t* self);