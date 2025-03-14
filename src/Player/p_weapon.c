//
// p_weapon.c
//
// Copyright 1998 Raven Software
//

#include "Player.h"
#include "p_anim_branch.h"
#include "p_main.h"
#include "p_weapon.h"
#include "g_items.h"
#include "cl_strings.h"

// Make the specified weapon ready if the owner has enough ammo for it.
// Assumes that the owner does actually have the weapon.
// Called by Cmd_InvUse_f() and other functions which do check the availability first anyhow.
PLAYER_API void Weapon_Ready(playerinfo_t* info, gitem_t* weapon)
{
	assert(weapon);

	// See if we're already using the weapon. Make sure we have an arm to use it.
	if (weapon == info->pers.weapon || !BranchCheckDismemberAction(info, weapon->tag))
		return;

	// Change to this weapon and set the weapon owner's ammo_index to reflect this.
	info->pers.lastweapon = info->pers.weapon;
	info->pers.weapon = weapon;

	if (info->pers.weapon != NULL && info->pers.weapon->ammo != NULL)
		info->weap_ammo_index = GetItemIndex(FindItem(info->pers.weapon->ammo));
	else
		info->weap_ammo_index = 0;
}

PLAYER_API void Weapon_EquipSwordStaff(playerinfo_t* info, gitem_t* weapon)
{
	assert(info);

	// Only spells allowed when swimming...
	if (info->pm_w_flags & WF_SURFACE || info->waterlevel >= 2)
		return;

	// See if we're already using the sword-staff. See if we're already switching weapons. Make sure we have an arm to use it.
	if (weapon == info->pers.weapon || info->pers.newweapon != NULL || !BranchCheckDismemberAction(info, weapon->tag))
		return;

	info->pers.newweapon = weapon;
	info->switchtoweapon = WEAPON_READY_SWORDSTAFF;
}

PLAYER_API void Weapon_EquipSpell(playerinfo_t* info, gitem_t* weapon)
{
	assert(info);

	// In blade-only DM, don't put away the staff and change weapons.
	if (info->dmflags & DF_NO_OFFENSIVE_SPELL)
		if (info->pm_w_flags & WF_SURFACE || info->waterlevel >= 2)
			return;

	// See if we're already using this particular spell. See if we're already switching weapons. Make sure we have an arm to use it.
	if (weapon == info->pers.weapon || (info->pers.newweapon != NULL && info->switchtoweapon != WEAPON_READY_HANDS) || !BranchCheckDismemberAction(info, weapon->tag))
		return;

	// If its anything other than the flying fist, see if we have mana for it.
	if (weapon->tag != ITEM_WEAPON_FLYINGFIST && info->pers.inventory.Items[GetItemIndex(FindItem(weapon->ammo))] < weapon->quantity)
	{
		info->G_gamemsg_centerprintf(info->self, GM_NOMANA);
		return;
	}

	info->pers.newweapon = weapon;
	info->switchtoweapon = WEAPON_READY_HANDS;
}

PLAYER_API void Weapon_EquipHellStaff(playerinfo_t* info, gitem_t* weapon)
{
	assert(info);

	// Only spells allowed when swimming...
	if (info->pm_w_flags & WF_SURFACE || info->waterlevel >= 2)
		return;

	// See if we're already using the hell-staff. See if we're already switching weapons. Make sure we have an arm to use it.
	if (weapon == info->pers.weapon || info->pers.newweapon != NULL || !BranchCheckDismemberAction(info, weapon->tag))
		return;

	// See if we actually have any ammo for it.
	if (info->pers.inventory.Items[GetItemIndex(FindItem(weapon->ammo))] == 0)
	{
		info->G_gamemsg_centerprintf(info->self, GM_NOAMMO);
		return;
	}

	info->pers.newweapon = weapon;
	info->switchtoweapon = WEAPON_READY_HELLSTAFF;
}

PLAYER_API void Weapon_EquipBow(playerinfo_t* info, gitem_t* weapon)
{
	assert(info);

	// Only spells allowed when swimming...
	if (info->pm_w_flags & WF_SURFACE || info->waterlevel >= 2)
		return;

	// See if we're already using the bow. See if we're already switching. Make sure we have an arm to use it.
	if (weapon == info->pers.weapon || (info->pers.newweapon != NULL && info->switchtoweapon != WEAPON_READY_BOW) || !BranchCheckDismemberAction(info, weapon->tag))
		return;

	// See if we actually have any ammo for it.
	if (info->pers.inventory.Items[GetItemIndex(FindItem(weapon->ammo))] == 0)
	{
		info->G_gamemsg_centerprintf(info->self, GM_NOAMMO);
		return;
	}

	info->pers.newweapon = weapon;
	info->switchtoweapon = WEAPON_READY_BOW;
}

//TODO: unused? If true, P_Weapon_EquipArmor() must be removed as well!
PLAYER_API void Weapon_EquipArmor(playerinfo_t* info, gitem_t* weapon)
{
	assert(info);

	// See if we're already using the armor.
	if (info->pers.armortype)
	{
		info->pers.armortype = ARMOR_TYPE_NONE;
		PlayerUpdateModelAttributes(info);
	}
	else
	{
		info->pers.armortype = ARMOR_TYPE_SILVER;
		PlayerUpdateModelAttributes(info);
	}
}

// Returns the number of shots that a weapon owner could make with the currently selected weapon,
// in respect to the amount of ammo for that weapon that the player has in their inventory.
PLAYER_API int Weapon_CurrentShotsLeft(const playerinfo_t* info)
{
	const gitem_t* weapon = info->pers.weapon;

	// If the weapon uses ammo, return the number of shots left, else return 0 (e.g. Sword-staff).
	if (weapon->ammo != NULL && weapon->quantity > 0)
	{
		int quantity = weapon->quantity;
		if (info->pers.weapon->tag == ITEM_WEAPON_MACEBALLS && info->powerup_timer > info->leveltime)
			quantity *= 2; // Double consumption for powered mace.

		return info->pers.inventory.Items[GetItemIndex(FindItem(weapon->ammo))] / quantity;
	}

	return 0;
}

// Returns the number of uses that a defence owner could make with the currently selected defence,
// in respect to the amount of ammo for that defence that the player has in their inventory.
PLAYER_API int Defence_CurrentShotsLeft(const playerinfo_t* info, int intent) //TODO: 'intent' arg is unused.
{
	const gitem_t* defence = info->pers.defence;

	// If the defense uses ammo, return the number of uses left, else return 0. //TODO: are there any defense spells which don't use ammo?..
	if (defence->ammo != NULL && defence->quantity > 0)
		return info->pers.inventory.Items[GetItemIndex(FindItem(defence->ammo))] / defence->quantity;

	return 0;
}