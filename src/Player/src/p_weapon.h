//
// p_weapon.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "p_types.h"

PLAYER_API void Weapon_Ready(playerinfo_t* info, gitem_t* weapon);
PLAYER_API void Weapon_EquipSpell(playerinfo_t* info, gitem_t* weapon);
PLAYER_API void Weapon_EquipSwordStaff(playerinfo_t* info, gitem_t* weapon);
PLAYER_API void Weapon_EquipHellStaff(playerinfo_t* info, gitem_t* weapon);
PLAYER_API void Weapon_EquipBow(playerinfo_t* info, gitem_t* weapon);
PLAYER_API void Weapon_EquipArmor(playerinfo_t* info, gitem_t* weapon);
PLAYER_API int Weapon_CurrentShotsLeft(const playerinfo_t* info);
PLAYER_API int Defence_CurrentShotsLeft(const playerinfo_t* info, int intent);