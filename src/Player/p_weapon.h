//
// p_weapon2.h
//
// Heretic II
// Copyright 1998 Raven Software
//

#ifndef P_WEAPON2_H
#define P_WEAPON2_H

#include "p_types.h"

// ************************************************************************************************
// Weapon_XXX
// ----------
// Applicable to all player-weapon types.
// ************************************************************************************************

PLAYER_API void Weapon_Ready(playerinfo_t *playerinfo,gitem_t *Weapon);
PLAYER_API void Weapon_EquipSpell(playerinfo_t *playerinfo,gitem_t *Weapon);
PLAYER_API void Weapon_EquipSwordStaff(playerinfo_t *playerinfo,gitem_t *Weapon);
PLAYER_API void Weapon_EquipHellStaff(playerinfo_t *playerinfo,gitem_t *Weapon);
PLAYER_API void Weapon_EquipBow(playerinfo_t *playerinfo,gitem_t *Weapon);
PLAYER_API void Weapon_EquipArmor(playerinfo_t *playerinfo, gitem_t *Weapon);
PLAYER_API int Weapon_CurrentShotsLeft(playerinfo_t *playerinfo);
PLAYER_API int Defence_CurrentShotsLeft(playerinfo_t *playerinfo, int intent);

#endif // P_WEAPON_H
