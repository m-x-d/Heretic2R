//
// g_items.h
//
// Copyright 1998 Raven Software
//

#pragma once

// Health pickups.
typedef enum
{
	ITEM_HEALTH1,
	ITEM_HEALTH2
} itemhealth_t;

// Weapons.
typedef enum
{
	ITEM_WEAPON_SWORDSTAFF,
	ITEM_WEAPON_FLYINGFIST,
	ITEM_WEAPON_HELLSTAFF,
	ITEM_WEAPON_MAGICMISSILE,
	ITEM_WEAPON_REDRAINBOW,
	ITEM_WEAPON_SPHEREOFANNIHILATION,
	ITEM_WEAPON_PHOENIXBOW,
	ITEM_WEAPON_MACEBALLS,
	ITEM_WEAPON_FIREWALL,
} itemweapon_t;

// Defense spells.
typedef enum
{
	ITEM_DEFENSE_REPULSION,
	ITEM_DEFENSE_METEORBARRIER,
	ITEM_DEFENSE_POLYMORPH,
	ITEM_DEFENSE_TELEPORT,
	ITEM_DEFENSE_SHIELD,
	ITEM_DEFENSE_TORNADO,
	ITEM_DEFENSE_POWERUP, //TODO: unused item. Rename to NUM_DEFENSE_PICKUPS and use in fx_DefensePickup.c instead of local define?
} itemdefense_t;

// Mana pickups.
typedef enum
{
	ITEM_AMMO_MANA_DEFENSIVE_HALF,
	ITEM_AMMO_MANA_DEFENSIVE_FULL,
	ITEM_AMMO_MANA_OFFENSIVE_HALF,
	ITEM_AMMO_MANA_OFFENSIVE_FULL,
	ITEM_AMMO_MANA_COMBO_QUARTER,
	ITEM_AMMO_MANA_COMBO_HALF,
	ITEM_AMMO_HELLSTAFF,
	ITEM_AMMO_REDRAIN,
	ITEM_AMMO_PHOENIX,
} itemammo_t;

//mxd. For internal usage only.
#ifdef _GAME_DLL
	void G_InitItems(void);
	void SetItemNames(void);
	edict_t* Drop_Item(edict_t* ent, gitem_t* item);
	void SpawnItem(edict_t* ent, gitem_t* item);
	gitem_t* IsItem(const edict_t* ent);
	qboolean Add_Ammo(const edict_t* ent, const gitem_t* ammo, int count);
	qboolean AddDefenseToInventory(gitem_t* defence, const edict_t* player);
	qboolean AddWeaponToInventory(gitem_t* weapon, const edict_t* player);
#endif