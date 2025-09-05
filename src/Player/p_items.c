//
// p_items.c
//
// Copyright 1998 Raven Software
//

#include "Player.h"
#include "p_anim_data.h"
#include "FX.h"
#include "items.h"
#include "g_items.h"
#include "g_itemstats.h"
#include "cl_strings.h"

PLAYER_API int p_num_items = 0;
PLAYER_API gitem_t* p_itemlist = NULL;

#pragma region ========================== ITEMS LIST ==========================

// The complete list of all items that may be picked up / dropped / used by players.
static gitem_t itemlist[] =
{
	// Leave index 0 empty.
	{ NULL },

#pragma region ========================== WEAPONS ==========================

	{ // 1
		"Weapon_SwordStaff",					// Spawnname (char *).
		"staff",								// Pickup name (char *).
		0,										// Pickup message.
		0,										// Can`t use message.
		NULL,									// Pickup function pointer.
		NULL,									// Use function pointer.
		NULL,									// Drop function pointer.
		NULL,									// Think function pointer.
		"player/getweapon.wav",					// Pickup sound (char *).
		NULL,									// World model (char *).
		0,										// World model flags.
		PICKUP_MIN,								// Bounding box mins.
		PICKUP_MAX,								// Bounding box maxs.
		ASEQ_WSWORD_STD1,						// Player animation sequence to engage when used.
		ASEQ_WSWORD_STD1,						// Player animation sequence to engage when powered.
		-1,										// Max uses (-1 = infinite).
		0,										// Number of digits to display.
		0,										// Ammo type/ammo use per shot.
		NULL,									// Ammo name (char *).
		IT_WEAPON,								// IT_XXX flags.
		NULL,									// void* info (unused).
		ITEM_WEAPON_SWORDSTAFF,					// Tag.
		"icons/i_staff.m8",						// Icon name (char *).
	},

	{ // 2
		"Weapon_FlyingFist",					// Spawnname (char *).
		"fball",								// Pickup name (char *).
		0,										// Pickup message.
		GM_NOFLYINGFIST,						// Can`t use message.
		NULL,									// Pickup function pointer.
		NULL,									// Use function pointer.
		NULL,									// Drop function pointer.
		NULL,									// Think function pointer.
		"player/getweapon.wav",					// Pickup sound (char *).
		NULL,									// World model (char *).
		0,										// World model flags.
		PICKUP_MIN,								// Bounding box mins.
		PICKUP_MAX,								// Bounding box maxs.
		ASEQ_WFIREBALL,							// Player animation sequence to engage when used.
		ASEQ_WFIREBALL,							// Player animation sequence to engage when powered.
		-1,										// Max uses (-1 = infinite).
		0,										// Number of digits to display.
		AMMO_USE_FIREBALL,						// Ammo type/ammo use per shot.
		"Off-mana",								// Ammo name (char *).
		IT_WEAPON | IT_OFFENSE,					// IT_XXX flags.
		NULL,									// void* info (unused).
		ITEM_WEAPON_FLYINGFIST,					// Tag.
		"icons/i_fball.m8",						// Icon name (char *).
	},

	{ // 3
		"item_weapon_hellstaff",				// Spawnname (char *).
		"hell",									// Pickup name (char *).
		GM_HELLSTAFF,							// Pickup message.
		GM_NOHELLORBS,							// Can`t use message.
		NULL,									// Pickup function pointer.
		NULL,									// Use function pointer.
		NULL,									// Drop function pointer.
		NULL,									// Think function pointer.
		"player/getweapon.wav",					// Pickup sound (char *).
		NULL,									// World model (char *).
		0,										// World model flags.
		PICKUP_MIN,								// Bounding box mins.
		PICKUP_MAX,								// Bounding box maxs.
		ASEQ_WHELL_GO,							// Player animation sequence to engage when used.
		ASEQ_WHELL_GO,							// Player animation sequence to engage when powered.
		-1,										// Max uses (-1 = infinite).
		2,										// Number of digits to display.
		AMMO_USE_HELLSTAFF,						// Ammo type/ammo use per shot.
		"Hell-staff-ammo",						// Ammo name (char *).
		IT_WEAPON | IT_OFFENSE,					// IT_XXX flags.
		NULL,									// void* info (unused).
		ITEM_WEAPON_HELLSTAFF,					// Tag.
		"icons/i_hell.m8",						// Icon name (char *).
	},

	{ // 4
		"item_weapon_magicmissile",				// Spawnname (char *).
		"array",								// Pickup name (char *).
		GM_FORCEBLAST,							// Pickup message.
		GM_NOFORCE,								// Can`t use message.
		NULL,									// Pickup function pointer.
		NULL,									// Use function pointer.
		NULL,									// Drop function pointer.
		NULL,									// Think function pointer.
		"player/getweapon.wav",					// Pickup sound (char *).
		NULL,									// World model (char *).
		EF_ROTATE,								// World model flags.
		PICKUP_MIN,								// Bounding box mins.
		PICKUP_MAX,								// Bounding box maxs.
		ASEQ_WBLAST,							// Player animation sequence to engage when used.
		ASEQ_WARRAY,							// Player animation sequence to engage when powered.
		-1,										// Max uses (-1 = infinite).
		0,										// Number of digits to display.
		AMMO_USE_MAGICMISSILE,					// Ammo type/ammo use per shot.
		"Off-mana",								// Ammo name (char *).
		IT_WEAPON | IT_OFFENSE,					// IT_XXX flags.
		NULL,									// void* info (unused).
		ITEM_WEAPON_MAGICMISSILE,				// Tag.
		"icons/i_array.m8",						// Icon name (char *).
	},

	{ // 5
		"item_weapon_redrain_bow",				// Spawnname (char *).
		"rain",									// Pickup name (char *).
		GM_STORMBOW,							// Pickup message.
		GM_NOSTORMBOW,							// Can`t use message.
		NULL,									// Pickup function pointer.
		NULL,									// Use function pointer.
		NULL,									// Drop function pointer.
		NULL,									// Think function pointer.
		"player/getweapon.wav",					// Pickup sound (char *).
		NULL,									// World model (char *).
		0,										// World model flags.
		PICKUP_MIN,								// Bounding box mins.
		PICKUP_MAX,								// Bounding box maxs.
		ASEQ_WRRBOW_GO,							// Player animation sequence to engage when used.
		ASEQ_WRRBOW_GO,							// Player animation sequence to engage when powered.
		-1,										// Max uses (-1 = infinite).
		2,										// Number of digits to display.
		AMMO_USE_REDRAIN,						// Ammo type/ammo use per shot.
		"Red-Rain-Arrows",						// Ammo name (char *).
		IT_WEAPON | IT_OFFENSE,					// IT_XXX flags.
		NULL,									// void* info (unused).
		ITEM_WEAPON_REDRAINBOW,					// Tag.
		"icons/i_rain.m8",						// Icon name (char *).
	},

	{ // 6
		"item_weapon_firewall",					// Spawnname (char *).
		"fwall",								// Pickup name (char *).
		GM_FIREWALL,							// Pickup message.
		GM_NOFIREWALL,							// Can`t use message.
		NULL,									// Pickup function pointer.
		NULL,									// Use function pointer.
		NULL,									// Drop function pointer.
		NULL,									// Think function pointer.
		"player/getweapon.wav",					// Pickup sound (char *).
		NULL,									// World model (char *).
		EF_ROTATE,								// World model flags. //BUGFIX: mxd. Goes AFTER PICKUP_MAX in original version.
		PICKUP_MIN,								// Bounding box mins.
		PICKUP_MAX,								// Bounding box maxs.
		ASEQ_WFIREWALL,							// Player animation sequence to engage when used.
		ASEQ_WFIREWALL,							// Player animation sequence to engage when powered.
		-1,										// Max uses (-1 = infinite).
		0,										// Number of digits to display.
		AMMO_USE_FIREWALL,						// Ammo type/ammo use per shot.
		"Off-mana",								// Ammo name (char *).
		IT_WEAPON | IT_OFFENSE,					// IT_XXX flags.
		NULL,									// void* info (unused).
		ITEM_WEAPON_FIREWALL,					// Tag.
		"icons/i_fwall.m8",						// Icon name (char *).
	},

	{ // 7
		"item_weapon_phoenixbow",				// Spawnname (char *).
		"phoen",								// Pickup name (char *).
		GM_PHOENIX,								// Pickup message.
		GM_NOPHOENIX,							// Can`t use message.
		NULL,	 								// Pickup function pointer.
		NULL,									// Use function pointer.
		NULL,									// Drop function pointer.
		NULL,									// Think function pointer.
		"player/getweapon.wav",					// Pickup sound (char *).
		NULL,									// World model (char *).
		0,										// World model flags.
		PICKUP_MIN,								// Bounding box mins.
		PICKUP_MAX,								// Bounding box maxs.
		ASEQ_WPHBOW_GO,							// Player animation sequence to engage when used.
		ASEQ_WPHBOW_GO,							// Player animation sequence to engage when powered.
		-1,										// Max uses (-1 = infinite).
		2,										// Number of digits to display.
		AMMO_USE_PHOENIX,						// Ammo type/ammo use per shot.
		"Phoenix-Arrows",						// Ammo name (char *).
		IT_WEAPON | IT_OFFENSE,					// IT_XXX flags.
		NULL,									// void* info (unused).
		ITEM_WEAPON_PHOENIXBOW,					// Tag.
		"icons/i_phoen.m8",						// Icon name (char *).
	},

	{ // 8
		"item_weapon_sphereofannihilation",		// Spawnname (char *).
		"sphere",								// Pickup name (char *).
		GM_SPHERE,								// Pickup message.
		GM_NOSPHERE,							// Can`t use message.
		NULL,									// Pickup function pointer.
		NULL,									// Use function pointer.
		NULL,									// Drop function pointer.
		NULL,									// Think function pointer.
		"player/getweapon.wav",					// Pickup sound (char *).
		NULL,									// World model (char *).
		EF_ROTATE,								// World model flags.
		PICKUP_MIN,								// Bounding box mins.
		PICKUP_MAX,								// Bounding box maxs.
		ASEQ_WSPHERE_GO,						// Player animation sequence to engage when used.
		ASEQ_WSPHERE_GO,						// Player animation sequence to engage when powered.
		-1,										// Max uses (-1 = infinite).
		0,										// Number of digits to display.
		AMMO_USE_SPHERE,						// Ammo type/ammo use per shot.
		"Off-mana",								// Ammo name (char *).
		IT_WEAPON | IT_OFFENSE,					// IT_XXX flags.
		NULL,									// void* info (unused).
		ITEM_WEAPON_SPHEREOFANNIHILATION,		// Tag.
		"icons/i_sphere.m8",					// Icon name (char *).
	},

	{ // 9
		"item_weapon_maceballs",				// Spawnname (char *).
		"mace",									// Pickup name (char *).
		GM_IRONDOOM,							// Pickup message.
		GM_NOIRONDOOM,							// Can`t use message.
		NULL,									// Pickup function pointer.
		NULL,									// Use function pointer.
		NULL,									// Drop function pointer.
		NULL,									// Think function pointer.
		"player/getweapon.wav",					// Pickup sound (char *).
		NULL,									// World model (char *).
		EF_ROTATE,								// World model flags.
		PICKUP_MIN,								// Bounding box mins.
		PICKUP_MAX,								// Bounding box maxs.
		ASEQ_WRIPPER,							// Player animation sequence to engage when used.
		ASEQ_WBIGBALL,							// Player animation sequence to engage when powered.
		-1,										// Max uses (-1 = infinite).
		0,										// Number of digits to display.
		AMMO_USE_MACEBALL,						// Ammo type/ammo use per shot.
		"Off-mana",								// Ammo name (char *).
		IT_WEAPON | IT_OFFENSE,					// IT_XXX flags.
		NULL,									// void* info (unused).
		ITEM_WEAPON_MACEBALLS,					// Tag.
		"icons/i_mace.m8",						// Icon name (char *).
	},

#pragma endregion

#pragma region ========================== DEFENSE POWERUPS ==========================

	{ // 10
		"item_defense_powerup",					// Spawnname (char *).
		"powerup",								// Pickup name (char *).
		GM_TOME,								// Pickup message.
		GM_NOTOME,								// Can`t use message.
		NULL,									// Pickup function pointer.
		NULL,									// Use function pointer.
		NULL,									// Drop function pointer.
		NULL,									// Think function pointer.
		"player/getweapon.wav",					// Pickup sound (char *).
		NULL,									// World model (char *).
		EF_ROTATE,								// World model flags.
		PICKUP_MIN,								// Bounding box mins.
		PICKUP_MAX,								// Bounding box maxs.
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when used.
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when powered.
		-1,										// Max uses (-1 = infinite).
		0,										// Number of digits to display.
		MANA_USE_POWERUP,						// Ammo type/ammo use per shot.
		"Def-mana",								// Ammo name (char *).
		IT_DEFENSE,								// IT_XXX flags.
		NULL,									// void* info (unused).
		ITEM_DEFENSE_POWERUP,					// Tag.
		"icons/i_tome.m8",						// Icon name (char *).
	},

	{ // 11
		"item_defense_ringofrepulsion",			// Spawnname (char *).
		"ring",									// Pickup name (char *).
		GM_RING,								// Pickup message.
		GM_NORING,								// Can`t use message.
		NULL,									// Pickup function pointer.
		NULL,									// Use function pointer.
		NULL,									// Drop function pointer.
		NULL,									// Think function pointer.
		"player/getweapon.wav",					// Pickup sound (char *).
		NULL,									// World model (char *).
		EF_ROTATE,								// World model flags.
		PICKUP_MIN,								// Bounding box mins.
		PICKUP_MAX,								// Bounding box maxs.
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when used.
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when powered.
		-1,										// Max uses (-1 = infinite).
		0,										// Number of digits to display.
		MANA_USE_RING,							// Ammo type/ammo use per shot.
		"Def-mana",								// Ammo name (char *).
		IT_DEFENSE,								// IT_XXX flags.
		NULL,									// void* info (unused).
		ITEM_DEFENSE_REPULSION,					// Tag.
		"icons/i_ring.m8",						// Icon name (char *).
	},

	{ // 12
		"item_defense_shield",					// Spawnname (char *).
		"lshield",								// Pickup name (char *).
		GM_SHIELD,								// Pickup message.
		GM_NOSHIELD,							// Can`t use message.
		NULL,									// Pickup function pointer.
		NULL,									// Use function pointer.
		NULL,									// Drop function pointer.
		NULL,									// Think function pointer.
		"player/getweapon.wav",					// Pickup sound (char *).
		NULL,									// World model (char *).
		EF_ROTATE,								// World model flags.
		PICKUP_MIN,								// Bounding box mins.
		PICKUP_MAX,								// Bounding box maxs.
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when used.
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when powered.
		1,										// Max uses (-1 = infinite).
		0,										// Number of digits to display.
		MANA_USE_SHIELD,						// Ammo type/ammo use per shot.
		"Def-mana",								// Ammo name (char *).
		IT_DEFENSE,								// IT_XXX flags.
		NULL,									// void* info (unused).
		ITEM_DEFENSE_SHIELD,					// Tag.
		"icons/i_shield.m8",					// Icon name (char *).
	},

	{ // 13
		"item_defense_teleport",				// Spawnname (char *).
		"tele",									// Pickup name (char *).
		GM_TELEPORT,							// Pickup message.
		GM_NOTELEPORT,							// Can`t use message.
		NULL,									// Pickup function pointer.
		NULL,									// Use function pointer.
		NULL,									// Drop function pointer.
		NULL,									// Think function pointer.
		"player/getweapon.wav",					// Pickup sound (char *).
		NULL,									// World model (char *).
		EF_ROTATE,								// World model flags.
		PICKUP_MIN,								// Bounding box mins.
		PICKUP_MAX,								// Bounding box maxs.
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when used.
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when powered.
		1,										// Max uses (-1 = infinite).
		0,										// Number of digits to display.
		MANA_USE_TELEPORT,						// Ammo type/ammo use per shot.
		"Def-mana",								// Ammo name (char *).
		IT_DEFENSE,								// IT_XXX flags.
		NULL,									// void* info (unused).
		ITEM_DEFENSE_TELEPORT,					// Tag.
		"icons/i_tele.m8",						// Icon name (char *).
	},

	{ // 14
		"item_defense_polymorph",				// Spawnname (char *).
		"morph",								// Pickup name (char *).
		GM_MORPH,								// Pickup message.
		GM_NOMORPH,								// Can`t use message.
		NULL,									// Pickup function pointer.
		NULL,									// Use function pointer.
		NULL,									// Drop function pointer.
		NULL,									// Think function pointer.
		"player/getweapon.wav",					// Pickup sound (char *).
		NULL,									// World model (char *).
		EF_ROTATE,								// World model flags.
		PICKUP_MIN,								// Bounding box mins.
		PICKUP_MAX,								// Bounding box maxs.
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when used.
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when powered.
		1,										// Max uses (-1 = infinite).
		0,										// Number of digits to display.
		MANA_USE_POLYMORPH,						// Ammo type/ammo use per shot.
		"Def-mana",								// Ammo name (char *).
		IT_DEFENSE,								// IT_XXX flags.
		NULL,									// void* info (unused).
		ITEM_DEFENSE_POLYMORPH,					// Tag.
		"icons/i_morph.m8",						// Icon name (char *).
	},

	{ // 15
		"item_defense_meteorbarrier",			// Spawnname (char *).
		"meteor",								// Pickup name (char *).
		GM_METEOR,								// Pickup message.
		GM_NOMETEOR,							// Can`t use message.
		NULL,									// Pickup function pointer.
		NULL,									// Use function pointer.
		NULL,									// Drop function pointer.
		NULL,									// Think function pointer.
		"player/getweapon.wav",					// Pickup sound (char *).
		NULL,									// World model (char *).
		EF_ROTATE,								// World model flags.
		PICKUP_MIN,								// Bounding box mins.
		PICKUP_MAX,								// Bounding box maxs.
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when used.
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when powered.
		1,										// Max uses (-1 = infinite).
		0,										// Number of digits to display.
		MANA_USE_METEORS,						// Ammo type/ammo use per shot.
		"Def-mana",								// Ammo name (char *).
		IT_DEFENSE,								// IT_XXX flags.
		NULL,									// void* info (unused).
		ITEM_DEFENSE_METEORBARRIER,				// Tag.
		"icons/i_meteor.m8",					// Icon name (char *).
	},

#pragma endregion

#pragma region ========================== AMMO ==========================

	{ // 16
		"item_mana_offensive_half",				// Spawnname (char *).
		"Off-mana",								// Pickup name (char *).
		GM_OFFMANAS,							// Pickup message.
		0,										// Can`t use message.
		NULL,									// Pickup function pointer.
		NULL,									// Use function pointer.
		NULL,									// Drop function pointer.
		NULL,									// Think function pointer.
		"player/picup.wav",						// Pickup sound (char *).
		NULL,									// World model (char *).
		0,										// World model flags.
		PICKUP_MIN,								// Bounding box mins.
		PICKUP_MAX,								// Bounding box maxs.
		ASEQ_NONE,								// Player animation sequence to engage when used.
		ASEQ_NONE,								// Alternate player animation sequence to engage when used.
		-1,										// Max uses (-1 = infinite).
		0,										// Number of digits to display.
		HALF_OFF_MANA,							// Ammo type/ammo use per shot.
		NULL,									// Ammo name (char *).
		IT_AMMO | IT_OFFENSE,					// IT_XXX flags.
		NULL,									// void* info (unused).
		ITEM_AMMO_MANA_OFFENSIVE_HALF,			// Tag.
		NULL,									// Icon name (char *).
	},

	{ // 17
		"item_mana_offensive_full",				// Spawnname (char *).
		"Off-mana",								// Pickup name (char *).
		GM_OFFMANAB,							// Pickup message.
		0,										// Can`t use message.
		NULL,									// Pickup function pointer.
		NULL,									// Use function pointer.
		NULL,									// Drop function pointer.
		NULL,									// Think function pointer.
		"player/picup.wav",						// Pickup sound (char *).
		NULL,									// World model (char *).
		0,										// World model flags.
		PICKUP_MIN,								// Bounding box mins.
		PICKUP_MAX,								// Bounding box maxs.
		ASEQ_NONE,								// Player animation sequence to engage when used.
		ASEQ_NONE,								// Alternate player animation sequence to engage when used.
		-1,										// Max uses (-1 = infinite).
		0,										// Number of digits to display.
		FULL_OFF_MANA,							// Ammo type/ammo use per shot.
		NULL,									// Ammo name (char *).
		IT_AMMO | IT_OFFENSE,					// IT_XXX flags.
		NULL,									// void* info (unused).
		ITEM_AMMO_MANA_OFFENSIVE_FULL,			// Tag.
		NULL,									// Icon name (char *).
	},

	{ // 18
		"item_mana_defensive_half",				// Spawnname (char *).
		"Def-mana",								// Pickup name (char *).
		GM_DEFMANAS,							// Pickup message.
		0,										// Can`t use message.
		NULL,									// Pickup function pointer.
		NULL,									// Use function pointer.
		NULL,									// Drop function pointer.
		NULL,									// Think function pointer.
		"player/picup.wav",						// Pickup sound (char *).
		NULL,									// World model (char *).
		0,										// World model flags.
		PICKUP_MIN,								// Bounding box mins.
		PICKUP_MAX,								// Bounding box maxs.
		ASEQ_NONE,								// Player animation sequence to engage when used.
		ASEQ_NONE,								// Alternate player animation sequence to engage when used.
		-1,										// Max uses (-1 = infinite).
		0,										// Number of digits to display.
		HALF_DEF_MANA,							// Ammo type/ammo use per shot.
		NULL,									// Ammo name (char *).
		IT_AMMO | IT_DEFENSE,					// IT_XXX flags.
		NULL,									// void* info (unused).
		ITEM_AMMO_MANA_DEFENSIVE_HALF,			// Tag.
		NULL,									// Icon name (char *).
	},

	{ // 19
		"item_mana_defensive_full",				// Spawnname (char *).
		"Def-mana",								// Pickup name (char *).
		GM_DEFMANAB,							// Pickup message.
		0,										// Can`t use message.
		NULL,									// Pickup function pointer.
		NULL,									// Use function pointer.
		NULL,									// Drop function pointer.
		NULL,									// Think function pointer.
		"player/picup.wav",						// Pickup sound (char *).
		NULL,									// World model (char *).
		0,										// World model flags.
		PICKUP_MIN,								// Bounding box mins.
		PICKUP_MAX,								// Bounding box maxs.
		ASEQ_NONE,								// Player animation sequence to engage when used.
		ASEQ_NONE,								// Alternate player animation sequence to engage when used.
		-1,										// Max uses (-1 = infinite).
		0,										// Number of digits to display.
		FULL_DEF_MANA,							// Ammo type/ammo use per shot.
		NULL,									// Ammo name (char *).
		IT_AMMO | IT_DEFENSE,					// IT_XXX flags.
		NULL,									// void* info (unused).
		ITEM_AMMO_MANA_DEFENSIVE_FULL,			// Tag.
		NULL,									// Icon name (char *).
	},

	{ // 20
		"item_mana_combo_quarter",				// Spawnname (char *).
		"Def-mana",								// Pickup name (char *).
		GM_COMBMANAS,							// Pickup message.
		0,										// Can`t use message.
		NULL,									// Pickup function pointer.
		NULL,									// Use function pointer.
		NULL,									// Drop function pointer.
		NULL,									// Think function pointer.
		"player/picup.wav",						// Pickup sound (char *).
		NULL,									// World model (char *).
		0,										// World model flags.
		PICKUP_MIN,								// Bounding box mins.
		PICKUP_MAX,								// Bounding box maxs.
		ASEQ_NONE,								// Player animation sequence to engage when used.
		ASEQ_NONE,								// Alternate player animation sequence to engage when used.
		-1,										// Max uses (-1 = infinite).
		0,										// Number of digits to display.
		HALF_COMBO_MANA,						// Ammo type/ammo use per shot.
		NULL,									// Ammo name (char *).
		IT_AMMO,								// IT_XXX flags.
		NULL,									// void* info (unused).
		ITEM_AMMO_MANA_COMBO_QUARTER,			// Tag.
		NULL,									// Icon name (char *).
	},

	{ // 21
		"item_mana_combo_half",					// Spawnname (char *).
		"Def-mana",								// Pickup name (char *).
		GM_COMBMANAB,							// Pickup message.
		0,										// Can`t use message.
		NULL,									// Pickup function pointer.
		NULL,									// Use function pointer.
		NULL,									// Drop function pointer.
		NULL,									// Think function pointer.
		"player/picup.wav",						// Pickup sound (char *).
		NULL,									// World model (char *).
		0,										// World model flags.
		PICKUP_MIN,								// Bounding box mins.
		PICKUP_MAX,								// Bounding box maxs.
		ASEQ_NONE,								// Player animation sequence to engage when used.
		ASEQ_NONE,								// Alternate player animation sequence to engage when used.
		-1,										// Max uses (-1 = infinite).
		0,										// Number of digits to display.
		FULL_COMBO_MANA,						// Ammo type/ammo use per shot.
		NULL,									// Ammo name (char *).
		IT_AMMO,								// IT_XXX flags.
		NULL,									// void* info (unused).
		ITEM_AMMO_MANA_COMBO_HALF,				// Tag.
		NULL,									// Icon name (char *).
	},

	{ // 22
		"item_ammo_redrain",					// Spawnname (char *).
		"Red-Rain-Arrows",						// Pickup name (char *).
		GM_STORMARROWS,							// Pickup message.
		0,										// Can`t use message.
		NULL,									// Pickup function pointer.
		NULL,									// Use function pointer.
		NULL,									// Drop function pointer.
		NULL,									// Think function pointer.
		"player/picup.wav",						// Pickup sound (char *).
		NULL,									// World model (char *).
		0,										// World model flags.
		PICKUP_MIN,								// Bounding box mins.
		PICKUP_MAX,								// Bounding box maxs.
		ASEQ_NONE,								// Player animation sequence to engage when used.
		ASEQ_NONE,								// Alternate player animation sequence to engage when used.
		-1,										// Max uses (-1 = infinite).
		0,										// Number of digits to display.
		AMMO_COUNT_REDRAINBOW,					// Ammo type/ammo use per shot.
		NULL,									// Ammo name (char *).
		IT_AMMO | IT_OFFENSE,					// IT_XXX flags.
		NULL,									// void* info (unused).
		ITEM_AMMO_REDRAIN,						// Tag.
		"icons/i_ammo-redrain.m8",				// Icon name (char *).
	},

	{ // 23
		"item_ammo_phoenix",					// Spawnname (char *).
		"Phoenix-Arrows",						// Pickup name (char *).
		GM_PHOENARROWS,							// Pickup message.
		0,										// Can`t use message.
		NULL,									// Pickup function pointer.
		NULL,									// Use function pointer.
		NULL,									// Drop function pointer.
		NULL,									// Think function pointer.
		"player/picup.wav",						// Pickup sound (char *).
		NULL,									// World model (char *).
		0,										// World model flags.
		PICKUP_MIN,								// Bounding box mins.
		PICKUP_MAX,								// Bounding box maxs.
		ASEQ_NONE,								// Player animation sequence to engage when used.
		ASEQ_NONE,								// Alternate player animation sequence to engage when used.
		-1,										// Max uses (-1 = infinite).
		0,										// Number of digits to display.
		AMMO_COUNT_PHOENIXBOW,					// Ammo type/ammo use per shot.
		NULL,									// Ammo name (char *).
		IT_AMMO | IT_OFFENSE,					// IT_XXX flags.
		NULL,									// void* info (unused).
		ITEM_AMMO_PHOENIX,						// Tag.
		"icons/i_ammo-phoen.m8",				// Icon name (char *).
	},

	{ // 24
		"item_ammo_hellstaff",					// Spawnname (char *).
		"Hell-staff-ammo",						// Pickup name (char *).
		GM_HELLORB,								// Pickup message.
		0,										// Can`t use message.
		NULL,									// Pickup function pointer.
		NULL,									// Use function pointer.
		NULL,									// Drop function pointer.
		NULL,									// Think function pointer.
		"player/picup.wav",						// Pickup sound (char *).
		NULL,									// World model (char *).
		0,										// World model flags.
		PICKUP_MIN,								// Bounding box mins.
		PICKUP_MAX,								// Bounding box maxs.
		ASEQ_NONE,								// Player animation sequence to engage when used.
		ASEQ_NONE,								// Alternate player animation sequence to engage when used.
		-1,										// Max uses (-1 = infinite).
		0,										// Number of digits to display.
		AMMO_COUNT_HELLSTAFF,					// Ammo type/ammo use per shot.
		NULL,									// Ammo name (char *).
		IT_AMMO | IT_OFFENSE,					// IT_XXX flags.
		NULL,									// void* info (unused).
		ITEM_AMMO_HELLSTAFF,					// Tag.
		"icons/i_ammo-hellstaff.m8",			// Icon name (char *).
	},

#pragma endregion

#pragma region ========================== HEALTH ==========================

	{ // 25
		"item_health_half",						// Spawnname (char *).
		"Minor health",							// Pickup name (char *).
		GM_HEALTHVIAL,							// Pickup message.
		0,										// Can`t use message.
		NULL,									// Pickup function pointer.
		NULL,									// Use function pointer.
		NULL,									// Drop function pointer.
		NULL,									// Think function pointer.
		"*gethealth.wav",						// Pickup sound (char *).
		"models/items/health/healthsmall/tris.fm",	// World model (char *).
		0,										// World model flags.
		PICKUP_MIN,								// Bounding box mins.
		PICKUP_MAX,								// Bounding box maxs.
		ASEQ_NONE,								// Player animation sequence to engage when used.
		ASEQ_NONE,								// Alternate player animation sequence to engage when used.
		-1,										// Max uses (-1 = infinite).
		0,										// Number of digits to display.
		10,										// Ammo type/ammo use per shot.
		NULL,									// Ammo name (char *).
		IT_HEALTH | EF_ALWAYS_ADD_EFFECTS, 		// IT_XXX flags.
		NULL,									// void* info (unused).
		ITEM_HEALTH1,							// Tag. //mxd. MODEL_HEALTH1 in original logic.
		NULL,									// Icon name (char *).
	},

	{ // 26
		"item_health_full",						// Spawnname (char *).
		"Major health",							// Pickup name (char *).
		GM_HEALTHPOTION,						// Pickup message.
		0,										// Can`t use message.
		NULL,									// Pickup function pointer.
		NULL,									// Use function pointer.
		NULL,									// Drop function pointer.
		NULL,									// Think function pointer.
		"*gethealth.wav",						// Pickup sound (char *).
		"models/items/health/healthbig/tris.fm",// World model (char *).
		0,										// World model flags.
		PICKUP_MIN,								// Bounding box mins.
		PICKUP_MAX,								// Bounding box maxs.
		ASEQ_NONE,								// Player animation sequence to engage when used.
		ASEQ_NONE,								// Alternate player animation sequence to engage when used.
		0,										// Max uses (-1 = infinite).
		0,										// Number of digits to display.
		30,										// Ammo type/ammo use per shot.
		NULL,									// Ammo name (char *).
		IT_HEALTH | EF_ALWAYS_ADD_EFFECTS,		// IT_XXX flags.
		NULL,									// void* info (unused).
		ITEM_HEALTH2,							// Tag. //mxd. MODEL_HEALTH2 in original logic.
		NULL,									// Icon name (char *).
	},

#pragma endregion

#pragma region ========================== PUZZLE PIECES ==========================

	{ // 27
		"item_puzzle_townkey",					// Spawnname (char *).
		"Town Key",								// Pickup name (char *).
		GM_F_TOWNKEY,							// Pickup message.
		GM_NEED_TOWNKEY,						// Can`t use message.
		NULL,				 					// Pickup function pointer.
		NULL,									// Use function pointer.
		NULL,									// Drop function pointer.
		NULL,									// Think function pointer.
		"player/picup.wav",						// Pickup sound (char *).
		NULL,									// World model (char *).
		0,										// World model flags.
		{ -8.0f, -8.0f, -4.0f },				// Bounding box mins.
		{  8.0f,  8.0f,  4.0f },				// Bounding box maxs.
		ASEQ_NONE,								// Player animation sequence to engage when used.
		ASEQ_NONE,								// Alternate player animation sequence to engage when used.
		0,										// Max uses (-1 = infinite).
		0,										// Number of digits to display.
		0,										// Ammo type/ammo use per shot.
		NULL,									// Ammo name (char *).
		IT_PUZZLE,								// IT_XXX flags.
		NULL,									// void* info (unused).
		ITEM_TOWNKEY,							// Tag.
		"icons/p_townkey.m8",					// Icon name (char *).
	},

	{ // 28
		"item_puzzle_cog",						// Spawnname (char *).
		"Cog",									// Pickup name (char *).
		GM_F_COG,								// Pickup message.
		GM_NEED_COG,							// Can`t use message.
		NULL,				 					// Pickup function pointer.
		NULL,									// Use function pointer.
		NULL,									// Drop function pointer.
		NULL,									// Think function pointer.
		"player/picup.wav",						// Pickup sound (char *).
		NULL,									// World model (char *).
		0,										// World model flags.
		{ -10.0f, -10.0f, -24.0f },				// Bounding box mins.
		{  10.0f,  10.0f,  20.0f },				// Bounding box maxs.
		ASEQ_NONE,								// Player animation sequence to engage when used.
		ASEQ_NONE,								// Alternate player animation sequence to engage when used.
		0,										// Max uses (-1 = infinite).
		0,										// Number of digits to display.
		0,										// Ammo type/ammo use per shot.
		NULL,									// Ammo name (char *).
		IT_PUZZLE,								// IT_XXX flags.
		NULL,									// void* info (unused).
		ITEM_COG,								// Tag.
		"icons/p_cog.m8",						// Icon name (char *).
	},

	{ // 29
		"item_puzzle_shield",					// Spawnname (char *).
		"Defensive Shield",						// Pickup name (char *).
		GM_F_SHIELD,							// Pickup message.
		GM_NEED_SHIELD,							// Can`t use message.
		NULL,				 					// Pickup function pointer.
		NULL,									// Use function pointer.
		NULL,									// Drop function pointer.
		NULL,									// Think function pointer.
		"player/picup.wav",						// Pickup sound (char *).
		NULL,									// World model (char *).
		0,										// World model flags.
		{ -2.0f, -6.0f, -12.0f },				// Bounding box mins.
		{  2.0f,  6.0f,  12.0f },				// Bounding box maxs.
		ASEQ_NONE,								// Player animation sequence to engage when used.
		ASEQ_NONE,								// Alternate player animation sequence to engage when used.
		0,										// Max uses (-1 = infinite).
		0,										// Number of digits to display.
		0,										// Ammo type/ammo use per shot.
		NULL,									// Ammo name (char *).
		IT_PUZZLE,								// IT_XXX flags.
		NULL,									// void* info (unused).
		ITEM_SHIELD,							// Tag.
		"icons/p_shield.m8",					// Icon name (char *).
	},

	{ // 30
		"item_puzzle_potion",					// Spawnname (char *).
		"Potion",								// Pickup name (char *).
		GM_F_POTION,							// Pickup message.
		GM_NEED_POTION,							// Can`t use message.
		NULL,									// Pickup function pointer.
		NULL,									// Use function pointer.
		NULL,									// Drop function pointer.
		NULL,									// Think function pointer.
		"player/picup.wav",						// Pickup sound (char *).
		NULL,									// World model (char *).
		0,										// World model flags.
		{ -3.0f, -3.0f, -10.0f },				// Bounding box mins.
		{  3.0f,  3.0f,  10.0f },				// Bounding box maxs.
		ASEQ_NONE,								// Player animation sequence to engage when used.
		ASEQ_NONE,								// Alternate player animation sequence to engage when used.
		0,										// Max uses (-1 = infinite).
		0,										// Number of digits to display.
		0,										// Ammo type/ammo use per shot.
		NULL,									// Ammo name (char *).
		IT_PUZZLE,								// IT_XXX flags.
		NULL,									// void* info (unused).
		ITEM_POTION,							// Tag.
		"icons/p_potion.m8",					// Icon name (char *).
	},

	{ // 31
		"item_puzzle_plazacontainer",			// Spawnname (char *).
		"Container",							// Pickup name (char *).
		GM_F_CONT,								// Pickup message.
		GM_NEED_CONT,							// Can`t use message.
		NULL,									// Pickup function pointer.
		NULL,									// Use function pointer.
		NULL,									// Drop function pointer.
		NULL,									// Think function pointer.
		"player/picup.wav",						// Pickup sound (char *).
		NULL,									// World model (char *).
		0,										// World model flags.
		{ -6.0f, -6.0f, -8.0f },				// Bounding box mins.
		{  6.0f,  6.0f,  6.0f },				// Bounding box maxs.
		ASEQ_NONE,								// Player animation sequence to engage when used.
		ASEQ_NONE,								// Alternate player animation sequence to engage when used.
		0,										// Max uses (-1 = infinite).
		0,										// Number of digits to display.
		0,										// Ammo type/ammo use per shot.
		NULL,									// Ammo name (char *).
		IT_PUZZLE,								// IT_XXX flags.
		NULL,									// void* info (unused).
		ITEM_CONT,								// Tag.
		"icons/p_plazajug.m8",					// Icon name (char *).
	},

	{ // 32
		"item_puzzle_slumcontainer",			// Spawnname (char *).
		"Full Container",						// Pickup name (char *).
		GM_F_CONTFULL,							// Pickup message.
		GM_NEED_CONTFULL,						// Can`t use message.
		NULL,									// Pickup function pointer.
		NULL,									// Use function pointer.
		NULL,									// Drop function pointer.
		NULL,									// Think function pointer.
		"player/picup.wav",						// Pickup sound (char *).
		NULL,									// World model (char *).
		0,										// World model flags.
		{ -6.0f, -6.0f, -8.0f },				// Bounding box mins.
		{  6.0f,  6.0f,  6.0f },				// Bounding box maxs.
		ASEQ_NONE,								// Player animation sequence to engage when used.
		ASEQ_NONE,								// Alternate player animation sequence to engage when used.
		0,										// Max uses (-1 = infinite).
		0,										// Number of digits to display.
		0,										// Ammo type/ammo use per shot.
		NULL,									// Ammo name (char *).
		IT_PUZZLE,								// IT_XXX flags.
		NULL,									// void* info (unused).
		ITEM_SLUMCONT,							// Tag.
		"icons/p_jugfill.m8",					// Icon name (char *).
	},

	{ // 33
		"item_puzzle_crystal",					// Spawnname (char *).
		"Crystal",								// Pickup name (char *).
		GM_F_CRYSTAL,							// Pickup message.
		GM_NEED_CRYSTAL,						// Can`t use message.
		NULL,									// Pickup function pointer.
		NULL,									// Use function pointer.
		NULL,									// Drop function pointer.
		NULL,									// Think function pointer.
		"player/picup.wav",						// Pickup sound (char *).
		NULL,									// World model (char *).
		0,										// World model flags.
		PICKUP_MIN,								// Bounding box mins.
		PICKUP_MAX,								// Bounding box maxs.
		ASEQ_NONE,								// Player animation sequence to engage when used.
		ASEQ_NONE,								// Alternate player animation sequence to engage when used.
		0,										// Max uses (-1 = infinite).
		0,										// Number of digits to display.
		0,										// Ammo type/ammo use per shot.
		NULL,									// Ammo name (char *).
		IT_PUZZLE,								// IT_XXX flags.
		NULL,									// void* info (unused).
		ITEM_CRYSTAL,							// Tag.
		"icons/p_crystal.m8",					// Icon name (char *).
	},

	{ // 34
		"item_puzzle_canyonkey",				// Spawnname (char *).
		"Canyon Key",							// Pickup name (char *).
		GM_F_CANYONKEY,							// Pickup message.
		GM_NEED_CANYONKEY,						// Can`t use message.
		NULL,									// Pickup function pointer.
		NULL,									// Use function pointer.
		NULL,									// Drop function pointer.
		NULL,									// Think function pointer.
		"player/picup.wav",						// Pickup sound (char *).
		NULL,									// World model (char *).
		0,										// World model flags.
		PICKUP_MIN,								// Bounding box mins.
		PICKUP_MAX,								// Bounding box maxs.
		ASEQ_NONE,								// Player animation sequence to engage when used.
		ASEQ_NONE,								// Alternate player animation sequence to engage when used.
		0,										// Max uses (-1 = infinite).
		0,										// Number of digits to display.
		0,										// Ammo type/ammo use per shot.
		NULL,									// Ammo name (char *).
		IT_PUZZLE,								// IT_XXX flags.
		NULL,									// void* info (unused).
		ITEM_CANKEY,							// Tag.
		"icons/p_canyonkey.m8",					// Icon name (char *).
	},

	{ // 35
		"item_puzzle_hive2amulet",				// Spawnname (char *).
		"Hive 2 Amulet",						// Pickup name (char *).
		GM_F_AMULET,							// Pickup message.
		GM_NEED_AMULET,							// Can`t use message.
		NULL,									// Pickup function pointer.
		NULL,									// Use function pointer.
		NULL,									// Drop function pointer.
		NULL,									// Think function pointer.
		"player/picup.wav",						// Pickup sound (char *).
		NULL,									// World model (char *).
		0,										// World model flags.
		PICKUP_MIN,								// Bounding box mins.
		PICKUP_MAX,								// Bounding box maxs.
		ASEQ_NONE,								// Player animation sequence to engage when used.
		ASEQ_NONE,								// Alternate player animation sequence to engage when used.
		0,										// Max uses (-1 = infinite).
		0,										// Number of digits to display.
		0,										// Ammo type/ammo use per shot.
		NULL,									// Ammo name (char *).
		IT_PUZZLE,								// IT_XXX flags.
		NULL,									// void* info (unused).
		ITEM_AMULET,							// Tag.
		"icons/p_tcheckrikbust.m8",				// Icon name (char *).
	},

	{ // 36
		"item_puzzle_hive2spear",				// Spawnname (char *).
		"Hive 2 Spear",							// Pickup name (char *).
		GM_F_SPEAR,								// Pickup message.
		GM_NEED_SPEAR,							// Can`t use message.
		NULL,									// Pickup function pointer.
		NULL,									// Use function pointer.
		NULL,									// Drop function pointer.
		NULL,									// Think function pointer.
		"player/picup.wav",						// Pickup sound (char *).
		NULL,									// World model (char *).
		0,										// World model flags.
		PICKUP_MIN,								// Bounding box mins.
		PICKUP_MAX,								// Bounding box maxs.
		ASEQ_NONE,								// Player animation sequence to engage when used.
		ASEQ_NONE,								// Alternate player animation sequence to engage when used.
		0,										// Max uses (-1 = infinite).
		0,										// Number of digits to display.
		0,										// Ammo type/ammo use per shot.
		NULL,									// Ammo name (char *).
		IT_PUZZLE,								// IT_XXX flags.
		NULL,									// void* info (unused).
		ITEM_SPEAR,								// Tag.
		"icons/p_spear.m8",						// Icon name (char *).
	},

	{ // 37
		"item_puzzle_hive2gem",					// Spawnname (char *).
		"Hive 2 Gem",							// Pickup name (char *).
		GM_F_GEM,								// Pickup message.
		GM_NEED_GEM,							// Can`t use message.
		NULL,									// Pickup function pointer.
		NULL,									// Use function pointer.
		NULL,									// Drop function pointer.
		NULL,									// Think function pointer.
		"player/picup.wav",						// Pickup sound (char *).
		NULL,									// World model (char *).
		0,										// World model flags.
		PICKUP_MIN,								// Bounding box mins.
		PICKUP_MAX,								// Bounding box maxs.
		ASEQ_NONE,								// Player animation sequence to engage when used.
		ASEQ_NONE,								// Alternate player animation sequence to engage when used.
		0,										// Max uses (-1 = infinite).
		0,										// Number of digits to display.
		0,										// Ammo type/ammo use per shot.
		NULL,									// Ammo name (char *).
		IT_PUZZLE,								// IT_XXX flags.
		NULL,									// void* info (unused).
		ITEM_GEM,								// Tag.
		"icons/p_tcheckrikgem.m8",				// Icon name (char *).
	},

	{ // 38
		"item_puzzle_minecartwheel",			// Spawnname (char *).
		"Minecart Wheel",						// Pickup name (char *).
		GM_F_CARTWHEEL,							// Pickup message.
		GM_NEED_CARTWHEEL,						// Can`t use message.
		NULL,									// Pickup function pointer.
		NULL,									// Use function pointer.
		NULL,									// Drop function pointer.
		NULL,									// Think function pointer.
		"player/picup.wav",						// Pickup sound (char *).
		NULL,									// World model (char *).
		0,										// World model flags.
		{ -1.0f, -6.0f, -6.0f },				// Bounding box mins.
		{  1.0f,  6.0f,  6.0f },				// Bounding box maxs.
		ASEQ_NONE,								// Player animation sequence to engage when used.
		ASEQ_NONE,								// Alternate player animation sequence to engage when used.
		0,										// Max uses (-1 = infinite).
		0,										// Number of digits to display.
		0,										// Ammo type/ammo use per shot.
		NULL,									// Ammo name (char *).
		IT_PUZZLE,								// IT_XXX flags.
		NULL,									// void* info (unused).
		ITEM_WHEEL,								// Tag.
		"icons/p_wheel.m8",						// Icon name (char *).
	},

	{ // 39
		"item_puzzle_ore",						// Spawnname (char *).
		"Ore",									// Pickup name (char *).
		GM_F_UNREFORE,							// Pickup message.
		GM_NEED_UNREFORE,						// Can`t use message.
		NULL,									// Pickup function pointer.
		NULL,									// Use function pointer.
		NULL,									// Drop function pointer.
		NULL,									// Think function pointer.
		"player/picup.wav",						// Pickup sound (char *).
		NULL,									// World model (char *).
		0,										// World model flags.
		{ -10.0f, -10.0f, -8.0f },				// Bounding box mins.
		{  10.0f,  10.0f,  8.0f },				// Bounding box maxs.
		ASEQ_NONE,								// Player animation sequence to engage when used.
		ASEQ_NONE,								// Alternate player animation sequence to engage when used.
		0,										// Max uses (-1 = infinite).
		0,										// Number of digits to display.
		0,										// Ammo type/ammo use per shot.
		NULL,									// Ammo name (char *).
		IT_PUZZLE,								// IT_XXX flags.
		NULL,									// void* info (unused).
		ITEM_ORE	,							// Tag.
		"icons/p_oreunrefined.m8",				// Icon name (char *).
	},

	{ // 40
		"item_puzzle_refinedore",				// Spawnname (char *).
		"Refined Ore",							// Pickup name (char *).
		GM_F_REFORE,							// Pickup message.
		GM_NEED_REFORE,							// Can`t use message.
		NULL,									// Pickup function pointer.
		NULL,									// Use function pointer.
		NULL,									// Drop function pointer.
		NULL,									// Think function pointer.
		"player/picup.wav",						// Pickup sound (char *).
		NULL,									// World model (char *).
		0,										// World model flags.
		{ -3.0f, -12.0f, -2.0f },				// Bounding box mins.
		{  3.0f,  12.0f,  2.0f },				// Bounding box maxs.
		ASEQ_NONE,								// Player animation sequence to engage when used.
		ASEQ_NONE,								// Alternate player animation sequence to engage when used.
		0,										// Max uses (-1 = infinite).
		0,										// Number of digits to display.
		0,										// Ammo type/ammo use per shot.
		NULL,									// Ammo name (char *).
		IT_PUZZLE,								// IT_XXX flags.
		NULL,									// void* info (unused).
		ITEM_REF_ORE,							// Tag.
		"icons/p_orerefined.m8",				// Icon name (char *).
	},

	{ // 41
		"item_puzzle_dungeonkey",				// Spawnname (char *).
		"Dungeon Key",							// Pickup name (char *).
		GM_F_DUNGEONKEY,						// Pickup message.
		GM_NEED_DUNGEONKEY,						// Can`t use message.
		NULL,									// Pickup function pointer.
		NULL,									// Use function pointer.
		NULL,									// Drop function pointer.
		NULL,									// Think function pointer.
		"player/picup.wav",						// Pickup sound (char *).
		NULL,									// World model (char *).
		0,										// World model flags.
		{ -1.0f, -18.0f, -9.0f },				// Bounding box mins.
		{  1.0f,  18.0f,  9.0f },				// Bounding box maxs.
		ASEQ_NONE,								// Player animation sequence to engage when used.
		ASEQ_NONE,								// Alternate player animation sequence to engage when used.
		0,										// Max uses (-1 = infinite).
		0,										// Number of digits to display.
		0,										// Ammo type/ammo use per shot.
		NULL,									// Ammo name (char *).
		IT_PUZZLE,								// IT_XXX flags.
		NULL,									// void* info (unused).
		ITEM_DUNKEY,							// Tag.
		"icons/p_dungeonkey.m8",				// Icon name (char *).
	},

	{ // 42
		"item_puzzle_cloudkey",					// Spawnname (char *).
		"Cloud Key",							// Pickup name (char *).
		GM_F_CLOUDKEY,							// Pickup message.
		GM_NEED_CLOUDKEY,						// Can`t use message.
		NULL,									// Pickup function pointer.
		NULL,									// Use function pointer.
		NULL,									// Drop function pointer.
		NULL,									// Think function pointer.
		"player/picup.wav",						// Pickup sound (char *).
		NULL,									// World model (char *).
		0,										// World model flags.
		{ -8.0f, -8.0f, -3.0f },				// Bounding box mins.
		{  8.0f,  8.0f,  3.0f },				// Bounding box maxs.
		ASEQ_NONE,								// Player animation sequence to engage when used.
		ASEQ_NONE,								// Alternate player animation sequence to engage when used.
		0,										// Max uses (-1 = infinite).
		0,										// Number of digits to display.
		0,										// Ammo type/ammo use per shot.
		NULL,									// Ammo name (char *).
		IT_PUZZLE,								// IT_XXX flags.
		NULL,									// void* info (unused).
		ITEM_CLOUDKEY,							// Tag.
		"icons/p_cloudkey.m8",					// Icon name (char *).
	},

	{ // 43
		"item_puzzle_highpriestesskey",			// Spawnname (char *).
		"Key",									// Pickup name (char *).
		GM_F_HIGHKEY,							// Pickup message.
		GM_NEED_HIGHKEY,						// Can`t use message.
		NULL,									// Pickup function pointer.
		NULL,									// Use function pointer.
		NULL,									// Drop function pointer.
		NULL,									// Think function pointer.
		"player/picup.wav",						// Pickup sound (char *).
		NULL,									// World model (char *).
		0,										// World model flags.
		{ -12.0f, -12.0f, -6.0f },				// Bounding box mins.
		{  12.0f,  12.0f,  6.0f },				// Bounding box maxs.
		ASEQ_NONE,								// Player animation sequence to engage when used.
		ASEQ_NONE,								// Alternate player animation sequence to engage when used.
		0,										// Max uses (-1 = infinite).
		0,										// Number of digits to display.
		0,										// Ammo type/ammo use per shot.
		NULL,									// Ammo name (char *).
		IT_PUZZLE,								// IT_XXX flags.
		NULL,									// void* info (unused).
		ITEM_HIVEKEY,							// Tag.
		"icons/p_hivekey.m8",					// Icon name (char *).
	},

	{ // 44
		"item_puzzle_highpriestesssymbol",		// Spawnname (char *).
		"Symbol",								// Pickup name (char *).
		GM_F_SYMBOL,							// Pickup message.
		GM_NEED_SYMBOL,							// Can`t use message.
		NULL,									// Pickup function pointer.
		NULL,									// Use function pointer.
		NULL,									// Drop function pointer.
		NULL,									// Think function pointer.
		"player/picup.wav",						// Pickup sound (char *).
		NULL,									// World model (char *).
		0,										// World model flags.
		{ -12.0f, -12.0f, -4.0f },				// Bounding box mins.
		{  12.0f,  12.0f,  4.0f },				// Bounding box maxs.
		ASEQ_NONE,								// Player animation sequence to engage when used.
		ASEQ_NONE,								// Alternate player animation sequence to engage when used.
		0,										// Max uses (-1 = infinite).
		0,										// Number of digits to display.
		0,										// Ammo type/ammo use per shot.
		NULL,									// Ammo name (char *).
		IT_PUZZLE,								// IT_XXX flags.
		NULL,									// void* info (unused).
		ITEM_HPSYM,								// Tag.
		"icons/p_queenkey.m8",					// Icon name (char *).
	},

	{ // 45
		"item_puzzle_tome",						// Spawnname (char *).
		"Tome",									// Pickup name (char *).
		GM_F_TOME,								// Pickup message.
		GM_NEED_TOME,							// Can`t use message.
		NULL,									// Pickup function pointer.
		NULL,									// Use function pointer.
		NULL,									// Drop function pointer.
		NULL,									// Think function pointer.
		"player/picup.wav",						// Pickup sound (char *).
		NULL,									// World model (char *).
		0,										// World model flags.
		{ -12.0f, -12.0f, -4.0f },				// Bounding box mins.
		{  12.0f,  12.0f,  4.0f },				// Bounding box maxs.
		ASEQ_NONE,								// Player animation sequence to engage when used.
		ASEQ_NONE,								// Alternate player animation sequence to engage when used.
		0,										// Max uses (-1 = infinite).
		0,										// Number of digits to display.
		0,										// Ammo type/ammo use per shot.
		NULL,									// Ammo name (char *).
		IT_PUZZLE,								// IT_XXX flags.
		NULL,									// void* info (unused).
		ITEM_TOME,								// Tag.
		"icons/p_tomepower.m8",					// Icon name (char *).
	},

	{ // 46
		"item_puzzle_tavernkey",				// Spawnname (char *).
		"Tavern Key",							// Pickup name (char *).
		GM_F_TAVERNKEY,							// Pickup message.
		GM_NEED_TAVERNKEY,						// Can`t use message.
		NULL,									// Pickup function pointer.
		NULL,									// Use function pointer.
		NULL,									// Drop function pointer.
		NULL,									// Think function pointer.
		"player/picup.wav",						// Pickup sound (char *).
		NULL,									// World model (char *).
		0,										// World model flags.
		{ -12.0f, -12.0f, -4.0f },				// Bounding box mins.
		{  12.0f,  12.0f,  4.0f },				// Bounding box maxs.
		ASEQ_NONE,								// Player animation sequence to engage when used.
		ASEQ_NONE,								// Alternate player animation sequence to engage when used.
		0,										// Max uses (-1 = infinite).
		0,										// Number of digits to display.
		0,										// Ammo type/ammo use per shot.
		NULL,									// Ammo name (char *).
		IT_PUZZLE,								// IT_XXX flags.
		NULL,									// void* info (unused).
		ITEM_TAVERNKEY,							// Tag.
		"icons/p_tavernkey.m8",					// Icon name (char *).
	},

#pragma endregion

	{ // 47 //TODO: move to 'defence powerups' category? Will that break vanilla compatibility?
		"item_defense_tornado",					// Spawnname (char *).
		"tornado",								// Pickup name (char *).
		GM_TORNADO,								// Pickup message.
		GM_NOTORNADO,							// Can`t use message.
		NULL,									// Pickup function pointer.
		NULL,									// Use function pointer.
		NULL,									// Drop function pointer.
		NULL,									// Think function pointer.
		"player/getweapon.wav",					// Pickup sound (char *).
		NULL,									// World model (char *).
		EF_ROTATE,								// World model flags.
		PICKUP_MIN,								// Bounding box mins.
		PICKUP_MAX,								// Bounding box maxs.
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when used.
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when powered
		1,										// Max uses (-1 = infinite).
		0,										// Number of digits to display.
		MANA_USE_TORNADO,						// Ammo type/ammo use per shot.
		"Def-mana",								// Ammo name (char *).
		IT_DEFENSE,								// IT_XXX flags.
		NULL,									// void* info (unused).
		ITEM_DEFENSE_TORNADO,					// Tag.
		"icons/i_tornado.m8",					// Icon name (char *).
	},

	// End of list marker.
	{ NULL }
};

#pragma endregion

PLAYER_API void InitItems(void)
{
	p_itemlist = itemlist;
	p_num_items = (sizeof(itemlist) / sizeof(itemlist[0])) - 1; //mxd. Item index 0 means no item.
}

PLAYER_API int GetItemIndex(const gitem_t* item)
{
	if (item != NULL)
	{
		assert(item >= p_itemlist && item < p_itemlist + p_num_items);
		return item - p_itemlist;
	}

	return 0;
}

PLAYER_API gitem_t* GetItemByIndex(const int index)
{
	if (index > 0 && index < p_num_items)
		return &p_itemlist[index];

	return NULL;
}

PLAYER_API gitem_t* FindItemByClassname(const char* classname)
{
	gitem_t* item = p_itemlist;
	for (int i = 0; i < p_num_items; i++, item++)
		if (item->classname != NULL && Q_stricmp(item->classname, classname) == 0)
			return item;

	return NULL;
}

PLAYER_API gitem_t* FindItem(const char* pickup_name)
{
	gitem_t* item = p_itemlist;
	for (int i = 0; i < p_num_items; i++, item++)
		if (item->pickup_name != NULL && Q_stricmp(item->pickup_name, pickup_name) == 0)
			return item;

	return NULL;
}