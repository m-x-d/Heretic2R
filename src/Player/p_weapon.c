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

// ***********************************************************************************************
// Weapon_EquipSpell
// -----------------
// ************************************************************************************************

PLAYER_API void Weapon_EquipSpell(playerinfo_t *playerinfo,gitem_t *Weapon)
{
	assert(playerinfo);

	// See if we're already using this particular spell.
	if(Weapon==playerinfo->pers.weapon)
		return;
	
	// See if we're already switching...
	if(playerinfo->pers.newweapon != NULL)
	{
		if (playerinfo->switchtoweapon != WEAPON_READY_HANDS)
			return;
	}

	// Make sure we have an arm to do it.

	if (!BranchCheckDismemberAction(playerinfo, Weapon->tag))
		return;

	// In blade only DM, don't put away the staff and change weapons.

	if(playerinfo->dmflags&DF_NO_OFFENSIVE_SPELL)
		if (playerinfo->pm_w_flags & WF_SURFACE || playerinfo->waterlevel >= 2)
			return;

	// if its anything other than the flying fist, see if we have mana for it.
	if (Weapon->tag != ITEM_WEAPON_FLYINGFIST)
	{
		if (playerinfo->pers.inventory.Items[ITEM_INDEX(FindItem(Weapon->ammo))] < Weapon->quantity)
		{
			playerinfo->G_gamemsg_centerprintf (playerinfo->self, GM_NOMANA);
			return;
		}
	}
	
	playerinfo->pers.newweapon = Weapon;
	playerinfo->switchtoweapon = WEAPON_READY_HANDS;
}

// ************************************************************************************************
// Weapon_EquipHellStaff
// ---------------------
// ************************************************************************************************

PLAYER_API void Weapon_EquipHellStaff(playerinfo_t *playerinfo,gitem_t *Weapon)
{
	gitem_t	*AmmoItem;
	int		AmmoIndex;
	
	assert(playerinfo);

	// See if we're already using the hell-staff.
	if(Weapon==playerinfo->pers.weapon)
		return;
	
	// See if we're already switching...
	if(playerinfo->pers.newweapon != NULL)
		return;
	
	//Make sure we have an arm to do it
	if (!BranchCheckDismemberAction(playerinfo, Weapon->tag))
		return;

	if (playerinfo->pm_w_flags & WF_SURFACE || playerinfo->waterlevel >= 2)
		return;

	// see if we actually have any ammo for it
	AmmoItem=FindItem(Weapon->ammo);
	AmmoIndex=ITEM_INDEX(AmmoItem);

    if(!playerinfo->pers.inventory.Items[AmmoIndex])
	{
		playerinfo->G_gamemsg_centerprintf (playerinfo->self, GM_NOAMMO);
		return;
	}

	playerinfo->pers.newweapon = Weapon;
	playerinfo->switchtoweapon = WEAPON_READY_HELLSTAFF;
}

// ************************************************************************************************
// Weapon_EquipBow
// ---------------
// ************************************************************************************************

PLAYER_API void Weapon_EquipBow(playerinfo_t *playerinfo,gitem_t *Weapon)
{

	gitem_t	*AmmoItem;
	int		AmmoIndex;

	assert(playerinfo);

	// See if we're already using the bow.
	if(Weapon==playerinfo->pers.weapon)
		return;
	
	// See if we're already switching...
	if(playerinfo->pers.newweapon != NULL)
	{
		if (playerinfo->switchtoweapon != WEAPON_READY_BOW)
			return;
	}
							   
	//Make sure we have an arm to do it
	if (!BranchCheckDismemberAction(playerinfo, Weapon->tag))
		return;

	if (playerinfo->pm_w_flags & WF_SURFACE || playerinfo->waterlevel >= 2)
		return;

	// see if we actually have any ammo for it
	AmmoItem=FindItem(Weapon->ammo);
	AmmoIndex=ITEM_INDEX(AmmoItem);

    if(!playerinfo->pers.inventory.Items[AmmoIndex])
	{
		playerinfo->G_gamemsg_centerprintf (playerinfo->self, GM_NOAMMO);
		return;
	}

			   
	playerinfo->pers.newweapon = Weapon;
	playerinfo->switchtoweapon = WEAPON_READY_BOW;
}

// ************************************************************************************************
// Weapon_EquipArmor  - this is interesting - its never called from anywhere.
// ---------------
// ************************************************************************************************

PLAYER_API void Weapon_EquipArmor(playerinfo_t *playerinfo,gitem_t *Weapon)
{
	assert(playerinfo);

	// See if we're already using the armor.

	if (playerinfo->pers.armortype)
	{
		playerinfo->pers.armortype = ARMOR_TYPE_NONE;
 		PlayerUpdateModelAttributes(playerinfo);
	}
	else 
	{
		playerinfo->pers.armortype = ARMOR_TYPE_SILVER;
 		PlayerUpdateModelAttributes(playerinfo);
	}
}

// ************************************************************************************************
// Weapon_CurrentShotsLeft
// -----------------------
// Returns the number of shots that a weapon owner could make with the currently selected weapon,
// in respect to the amount of ammo for that weapon that the player has in their inventory.
// ************************************************************************************************

PLAYER_API int Weapon_CurrentShotsLeft(playerinfo_t *playerinfo)
{
	gitem_t	*Weapon,
			*AmmoItem;
	int		AmmoIndex;

	Weapon=playerinfo->pers.weapon;

	// If the weapon uses ammo, return the number of shots left, else return -1 (e.g. Sword-staff).

	if(Weapon->ammo&&(Weapon->quantity))
	{
		AmmoItem=FindItem(Weapon->ammo);
		AmmoIndex=ITEM_INDEX(AmmoItem);

		if (playerinfo->pers.weapon->tag == ITEM_WEAPON_MACEBALLS && playerinfo->powerup_timer > playerinfo->leveltime)
			return(playerinfo->pers.inventory.Items[AmmoIndex]/(Weapon->quantity*2.0));		// Double consumption for mace.
		else
			return(playerinfo->pers.inventory.Items[AmmoIndex]/Weapon->quantity);
	}
	else
		return(0);
}

// ************************************************************************************************
// Defence_CurrentShotsLeft
// -----------------------
// Returns the number of shots that a weapon owner could make with the currently selected weapon,
// in respect to the amount of ammo for that weapon that the player has in their inventory.
// ************************************************************************************************

PLAYER_API int Defence_CurrentShotsLeft(playerinfo_t *playerinfo, int intent)
{
	gitem_t	*Defence,
			*ManaItem;
	int		ManaIndex;

	Defence = playerinfo->pers.defence;

	// If the weapon uses ammo, return the number of shots left, else return -1 (e.g. Sword-staff).

	if(Defence->ammo && Defence->quantity)
	{
		ManaItem = FindItem(Defence->ammo);
		ManaIndex = ITEM_INDEX(ManaItem);

		return(playerinfo->pers.inventory.Items[ManaIndex]/Defence->quantity);

	}
	else
		return(0);
}
