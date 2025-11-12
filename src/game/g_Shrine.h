//
// g_Shrine.h
//
// Copyright 2025 mxd
//

#pragma once

#include "p_types.h"
#include "q_Typedef.h"

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

extern void SP_obj_shrine(edict_t* self);

//mxd. Required by save system...
extern void ShrineDelayThink(edict_t* self);
extern void ShrineArmorSilverTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf);
extern void ShrineArmorGoldTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf);
extern void ShrineRandomTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf);
extern void ShrineGhostTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf);
extern void ShrineHealTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf);
extern void ShrineLightTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf);
extern void ShrineLungsTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf);
extern void ShrineManaTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf);
extern void ShrinePowerupTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf);
extern void ShrineReflectTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf);
extern void ShrineSpeedTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf);
extern void ShrineStaffTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf);