//
// g_Shrine.h
//
// Copyright 2025 mxd
//

#pragma once

#include "g_local.h"

extern void PlayerKillShrineFX(edict_t* self);
extern void PlayerRestartShrineFX(edict_t* self);
extern void G_PlayerActionShrineEffect(const playerinfo_t* playerinfo);

extern void SP_shrine_heal_trigger(edict_t* ent);
extern void SP_shrine_armor_silver_trigger(edict_t* ent);
extern void SP_shrine_armor_gold_trigger(edict_t* ent);
extern void SP_shrine_staff_trigger(edict_t* ent);
extern void SP_shrine_lungs_trigger(edict_t* ent);
extern void SP_shrine_light_trigger(edict_t* ent);
extern void SP_shrine_mana_trigger(edict_t* ent);
extern void SP_shrine_ghost_trigger(edict_t* ent);
extern void SP_shrine_reflect_trigger(edict_t* ent);
extern void SP_shrine_powerup_trigger(edict_t* ent);
extern void SP_shrine_speed_trigger(edict_t* ent);
extern void SP_shrine_random_trigger(edict_t* ent);