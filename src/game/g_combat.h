//
// g_combat.h
//
// Copyright 2025 mxd
//

#pragma once

#include "g_local.h"

extern gitem_armor_t silver_armor_info;
extern gitem_armor_t gold_armor_info;

extern void Killed(edict_t* targ, edict_t* inflictor, edict_t* attacker, int damage, vec3_t point, int mod);