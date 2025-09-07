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
	{ 0 },

#pragma region ========================== WEAPONS ==========================

	{ // 1
		.classname = "Weapon_SwordStaff",				// Spawnname (char *).
		.pickup_name = "staff",							// Pickup name (char *).
		.pickup_sound = "player/getweapon.wav",			// Pickup sound (char *).
		.mins = PICKUP_MIN,								// Bounding box mins.
		.maxs = PICKUP_MAX,								// Bounding box maxs.
		.playeranimseq = ASEQ_WSWORD_STD1,				// Player animation sequence to engage when used.
		.altanimseq = ASEQ_WSWORD_STD1,					// Player animation sequence to engage when powered.
		.flags = IT_WEAPON,								// IT_XXX flags.
		.tag = ITEM_WEAPON_SWORDSTAFF,					// Tag.
		.icon = "icons/i_staff.m8",						// Icon name (char *).
	},

	{ // 2
		.classname = "Weapon_FlyingFist",				// Spawnname (char *).
		.pickup_name = "fball",							// Pickup name (char *).
		.msg_nouse = GM_NOFLYINGFIST,					// Can`t use message.
		.pickup_sound = "player/getweapon.wav",			// Pickup sound (char *).
		.mins = PICKUP_MIN,								// Bounding box mins.
		.maxs = PICKUP_MAX,								// Bounding box maxs.
		.playeranimseq = ASEQ_WFIREBALL,				// Player animation sequence to engage when used.
		.altanimseq = ASEQ_WFIREBALL,					// Player animation sequence to engage when powered.
		.quantity = AMMO_USE_FIREBALL,					// Ammo type/ammo use per shot.
		.ammo = "Off-mana",								// Ammo name (char *).
		.flags = (IT_WEAPON | IT_OFFENSE),				// IT_XXX flags.
		.tag = ITEM_WEAPON_FLYINGFIST,					// Tag.
		.icon = "icons/i_fball.m8",						// Icon name (char *).
	},

	{ // 3
		.classname = "item_weapon_hellstaff",			// Spawnname (char *).
		.pickup_name = "hell",							// Pickup name (char *).
		.msg_pickup = GM_HELLSTAFF,						// Pickup message.
		.msg_nouse = GM_NOHELLORBS,						// Can`t use message.
		.pickup_sound = "player/getweapon.wav",			// Pickup sound (char *).
		.mins = PICKUP_MIN,								// Bounding box mins.
		.maxs = PICKUP_MAX,								// Bounding box maxs.
		.playeranimseq = ASEQ_WHELL_GO,					// Player animation sequence to engage when used.
		.altanimseq = ASEQ_WHELL_GO,					// Player animation sequence to engage when powered.
		.count_width = 2,								// Number of digits to display.
		.quantity = AMMO_USE_HELLSTAFF,					// Ammo type/ammo use per shot.
		.ammo = "Hell-staff-ammo",						// Ammo name (char *).
		.flags = (IT_WEAPON | IT_OFFENSE),				// IT_XXX flags.
		.tag = ITEM_WEAPON_HELLSTAFF,					// Tag.
		.icon = "icons/i_hell.m8",						// Icon name (char *).
	},

	{ // 4
		.classname = "item_weapon_magicmissile",		// Spawnname (char *).
		.pickup_name = "array",							// Pickup name (char *).
		.msg_pickup = GM_FORCEBLAST,					// Pickup message.
		.msg_nouse = GM_NOFORCE,						// Can`t use message.
		.pickup_sound = "player/getweapon.wav",			// Pickup sound (char *).
		.world_model_flags = EF_ROTATE,					// World model flags.
		.mins = PICKUP_MIN,								// Bounding box mins.
		.maxs = PICKUP_MAX,								// Bounding box maxs.
		.playeranimseq = ASEQ_WBLAST,					// Player animation sequence to engage when used.
		.altanimseq = ASEQ_WARRAY,						// Player animation sequence to engage when powered.
		.quantity = AMMO_USE_MAGICMISSILE,				// Ammo type/ammo use per shot.
		.ammo = "Off-mana",								// Ammo name (char *).
		.flags = (IT_WEAPON | IT_OFFENSE),				// IT_XXX flags.
		.tag = ITEM_WEAPON_MAGICMISSILE,				// Tag.
		.icon = "icons/i_array.m8",						// Icon name (char *).
	},

	{ // 5
		.classname = "item_weapon_redrain_bow",			// Spawnname (char *).
		.pickup_name = "rain",							// Pickup name (char *).
		.msg_pickup = GM_STORMBOW,						// Pickup message.
		.msg_nouse = GM_NOSTORMBOW,						// Can`t use message.
		.pickup_sound = "player/getweapon.wav",			// Pickup sound (char *).
		.mins = PICKUP_MIN,								// Bounding box mins.
		.maxs = PICKUP_MAX,								// Bounding box maxs.
		.playeranimseq = ASEQ_WRRBOW_GO,				// Player animation sequence to engage when used.
		.altanimseq = ASEQ_WRRBOW_GO,					// Player animation sequence to engage when powered.
		.count_width = 2,								// Number of digits to display.
		.quantity = AMMO_USE_REDRAIN,					// Ammo type/ammo use per shot.
		.ammo = "Red-Rain-Arrows",						// Ammo name (char *).
		.flags = (IT_WEAPON | IT_OFFENSE),				// IT_XXX flags.
		.tag = ITEM_WEAPON_REDRAINBOW,					// Tag.
		.icon = "icons/i_rain.m8",						// Icon name (char *).
	},

	{ // 6
		.classname = "item_weapon_firewall",			// Spawnname (char *).
		.pickup_name = "fwall",							// Pickup name (char *).
		.msg_pickup = GM_FIREWALL,						// Pickup message.
		.msg_nouse = GM_NOFIREWALL,						// Can`t use message.
		.pickup_sound = "player/getweapon.wav",			// Pickup sound (char *).
		.world_model_flags = EF_ROTATE,					// World model flags. //BUGFIX: mxd. Goes AFTER PICKUP_MAX in original version.
		.mins = PICKUP_MIN,								// Bounding box mins.
		.maxs = PICKUP_MAX,								// Bounding box maxs.
		.playeranimseq = ASEQ_WFIREWALL,				// Player animation sequence to engage when used.
		.altanimseq = ASEQ_WFIREWALL,					// Player animation sequence to engage when powered.
		.quantity = AMMO_USE_FIREWALL,					// Ammo type/ammo use per shot.
		.ammo = "Off-mana",								// Ammo name (char *).
		.flags = (IT_WEAPON | IT_OFFENSE),				// IT_XXX flags.
		.tag = ITEM_WEAPON_FIREWALL,					// Tag.
		.icon = "icons/i_fwall.m8",						// Icon name (char *).
	},

	{ // 7
		.classname = "item_weapon_phoenixbow",			// Spawnname (char *).
		.pickup_name = "phoen",							// Pickup name (char *).
		.msg_pickup = GM_PHOENIX,						// Pickup message.
		.msg_nouse = GM_NOPHOENIX,						// Can`t use message.
		.pickup_sound = "player/getweapon.wav",			// Pickup sound (char *).
		.mins = PICKUP_MIN,								// Bounding box mins.
		.maxs = PICKUP_MAX,								// Bounding box maxs.
		.playeranimseq = ASEQ_WPHBOW_GO,				// Player animation sequence to engage when used.
		.altanimseq = ASEQ_WPHBOW_GO,					// Player animation sequence to engage when powered.
		.count_width = 2,								// Number of digits to display.
		.quantity = AMMO_USE_PHOENIX,					// Ammo type/ammo use per shot.
		.ammo = "Phoenix-Arrows",						// Ammo name (char *).
		.flags = (IT_WEAPON | IT_OFFENSE),				// IT_XXX flags.
		.tag = ITEM_WEAPON_PHOENIXBOW,					// Tag.
		.icon = "icons/i_phoen.m8",						// Icon name (char *).
	},

	{ // 8
		.classname = "item_weapon_sphereofannihilation",// Spawnname (char *).
		.pickup_name = "sphere",						// Pickup name (char *).
		.msg_pickup = GM_SPHERE,						// Pickup message.
		.msg_nouse = GM_NOSPHERE,						// Can`t use message.
		.pickup_sound = "player/getweapon.wav",			// Pickup sound (char *).
		.world_model_flags = EF_ROTATE,					// World model flags.
		.mins =PICKUP_MIN,								// Bounding box mins.
		.maxs =PICKUP_MAX,								// Bounding box maxs.
		.playeranimseq = ASEQ_WSPHERE_GO,				// Player animation sequence to engage when used.
		.altanimseq = ASEQ_WSPHERE_GO,					// Player animation sequence to engage when powered.
		.quantity = AMMO_USE_SPHERE,					// Ammo type/ammo use per shot.
		.ammo = "Off-mana",								// Ammo name (char *).
		.flags = (IT_WEAPON | IT_OFFENSE),				// IT_XXX flags.
		.tag = ITEM_WEAPON_SPHEREOFANNIHILATION,		// Tag.
		.icon = "icons/i_sphere.m8",					// Icon name (char *).
	},

	{ // 9
		.classname = "item_weapon_maceballs",			// Spawnname (char *).
		.pickup_name = "mace",							// Pickup name (char *).
		.msg_pickup = GM_IRONDOOM,						// Pickup message.
		.msg_nouse = GM_NOIRONDOOM,						// Can`t use message.
		.pickup_sound = "player/getweapon.wav",			// Pickup sound (char *).
		.world_model_flags = EF_ROTATE,					// World model flags.
		.mins = PICKUP_MIN,								// Bounding box mins.
		.maxs = PICKUP_MAX,								// Bounding box maxs.
		.playeranimseq = ASEQ_WRIPPER,					// Player animation sequence to engage when used.
		.altanimseq = ASEQ_WBIGBALL,					// Player animation sequence to engage when powered.
		.quantity = AMMO_USE_MACEBALL,					// Ammo type/ammo use per shot.
		.ammo = "Off-mana",								// Ammo name (char *).
		.flags = (IT_WEAPON | IT_OFFENSE),				// IT_XXX flags.
		.tag = ITEM_WEAPON_MACEBALLS,					// Tag.
		.icon = "icons/i_mace.m8",						// Icon name (char *).
	},

#pragma endregion

#pragma region ========================== DEFENSE POWERUPS ==========================

	{ // 10
		.classname = "item_defense_powerup",			// Spawnname (char *).
		.pickup_name = "powerup",						// Pickup name (char *).
		.msg_pickup = GM_TOME,							// Pickup message.
		.msg_nouse = GM_NOTOME,							// Can`t use message.
		.pickup_sound = "player/getweapon.wav",			// Pickup sound (char *).
		.world_model_flags = EF_ROTATE,					// World model flags.
		.mins = PICKUP_MIN,								// Bounding box mins.
		.maxs = PICKUP_MAX,								// Bounding box maxs.
		.playeranimseq = ASEQ_SPELL_DEF,				// Player animation sequence to engage when used.
		.altanimseq = ASEQ_SPELL_DEF,					// Player animation sequence to engage when powered.
		.quantity = MANA_USE_POWERUP,					// Ammo type/ammo use per shot.
		.ammo = "Def-mana",								// Ammo name (char *).
		.flags = IT_DEFENSE,							// IT_XXX flags.
		.tag = ITEM_DEFENSE_POWERUP,					// Tag.
		.icon = "icons/i_tome.m8",						// Icon name (char *).
	},

	{ // 11
		.classname = "item_defense_ringofrepulsion",	// Spawnname (char *).
		.pickup_name = "ring",							// Pickup name (char *).
		.msg_pickup = GM_RING,							// Pickup message.
		.msg_nouse = GM_NORING,							// Can`t use message.
		.pickup_sound = "player/getweapon.wav",			// Pickup sound (char *).
		.world_model_flags = EF_ROTATE,					// World model flags.
		.mins = PICKUP_MIN,								// Bounding box mins.
		.maxs = PICKUP_MAX,								// Bounding box maxs.
		.playeranimseq = ASEQ_SPELL_DEF,				// Player animation sequence to engage when used.
		.altanimseq = ASEQ_SPELL_DEF,					// Player animation sequence to engage when powered.
		.quantity = MANA_USE_RING,						// Ammo type/ammo use per shot.
		.ammo = "Def-mana",								// Ammo name (char *).
		.flags = IT_DEFENSE,							// IT_XXX flags.
		.tag = ITEM_DEFENSE_REPULSION,					// Tag.
		.icon = "icons/i_ring.m8",						// Icon name (char *).
	},

	{ // 12
		.classname = "item_defense_shield",				// Spawnname (char *).
		.pickup_name = "lshield",						// Pickup name (char *).
		.msg_pickup = GM_SHIELD,						// Pickup message.
		.msg_nouse = GM_NOSHIELD,						// Can`t use message.
		.pickup_sound = "player/getweapon.wav",			// Pickup sound (char *).
		.world_model_flags = EF_ROTATE,					// World model flags.
		.mins = PICKUP_MIN,								// Bounding box mins.
		.maxs = PICKUP_MAX,								// Bounding box maxs.
		.playeranimseq = ASEQ_SPELL_DEF,				// Player animation sequence to engage when used.
		.altanimseq = ASEQ_SPELL_DEF,					// Player animation sequence to engage when powered.
		.quantity = MANA_USE_SHIELD,					// Ammo type/ammo use per shot.
		.ammo = "Def-mana",								// Ammo name (char *).
		.flags = IT_DEFENSE,							// IT_XXX flags.
		.tag = ITEM_DEFENSE_SHIELD,						// Tag.
		.icon = "icons/i_shield.m8",					// Icon name (char *).
	},

	{ // 13
		.classname = "item_defense_teleport",			// Spawnname (char *).
		.pickup_name = "tele",							// Pickup name (char *).
		.msg_pickup = GM_TELEPORT,						// Pickup message.
		.msg_nouse = GM_NOTELEPORT,						// Can`t use message.
		.pickup_sound = "player/getweapon.wav",			// Pickup sound (char *).
		.world_model_flags = EF_ROTATE,					// World model flags.
		.mins = PICKUP_MIN,								// Bounding box mins.
		.maxs = PICKUP_MAX,								// Bounding box maxs.
		.playeranimseq = ASEQ_SPELL_DEF,				// Player animation sequence to engage when used.
		.altanimseq = ASEQ_SPELL_DEF,					// Player animation sequence to engage when powered.
		.quantity = MANA_USE_TELEPORT,					// Ammo type/ammo use per shot.
		.ammo = "Def-mana",								// Ammo name (char *).
		.flags = IT_DEFENSE,							// IT_XXX flags.
		.tag = ITEM_DEFENSE_TELEPORT,					// Tag.
		.icon = "icons/i_tele.m8",						// Icon name (char *).
	},

	{ // 14
		.classname = "item_defense_polymorph",			// Spawnname (char *).
		.pickup_name = "morph",							// Pickup name (char *).
		.msg_pickup = GM_MORPH,							// Pickup message.
		.msg_nouse = GM_NOMORPH,						// Can`t use message.
		.pickup_sound = "player/getweapon.wav",			// Pickup sound (char *).
		.world_model_flags = EF_ROTATE,					// World model flags.
		.mins = PICKUP_MIN,								// Bounding box mins.
		.maxs = PICKUP_MAX,								// Bounding box maxs.
		.playeranimseq = ASEQ_SPELL_DEF,				// Player animation sequence to engage when used.
		.altanimseq = ASEQ_SPELL_DEF,					// Player animation sequence to engage when powered.
		.quantity = MANA_USE_POLYMORPH,					// Ammo type/ammo use per shot.
		.ammo = "Def-mana",								// Ammo name (char *).
		.flags = IT_DEFENSE,							// IT_XXX flags.
		.tag = ITEM_DEFENSE_POLYMORPH,					// Tag.
		.icon = "icons/i_morph.m8",						// Icon name (char *).
	},

	{ // 15
		.classname = "item_defense_meteorbarrier",		// Spawnname (char *).
		.pickup_name = "meteor",						// Pickup name (char *).
		.msg_pickup = GM_METEOR,						// Pickup message.
		.msg_nouse = GM_NOMETEOR,						// Can`t use message.
		.pickup_sound = "player/getweapon.wav",			// Pickup sound (char *).
		.world_model_flags = EF_ROTATE,					// World model flags.
		.mins = PICKUP_MIN,								// Bounding box mins.
		.maxs = PICKUP_MAX,								// Bounding box maxs.
		.playeranimseq = ASEQ_SPELL_DEF,				// Player animation sequence to engage when used.
		.altanimseq = ASEQ_SPELL_DEF,					// Player animation sequence to engage when powered.
		.quantity = MANA_USE_METEORS,					// Ammo type/ammo use per shot.
		.ammo = "Def-mana",								// Ammo name (char *).
		.flags = IT_DEFENSE,							// IT_XXX flags.
		.tag = ITEM_DEFENSE_METEORBARRIER,				// Tag.
		.icon = "icons/i_meteor.m8",					// Icon name (char *).
	},

#pragma endregion

#pragma region ========================== AMMO ==========================

	{ // 16
		.classname = "item_mana_offensive_half",		// Spawnname (char *).
		.pickup_name = "Off-mana",						// Pickup name (char *).
		.msg_pickup = GM_OFFMANAS,						// Pickup message.
		.pickup_sound = "player/picup.wav",				// Pickup sound (char *).
		.mins = PICKUP_MIN,								// Bounding box mins.
		.maxs = PICKUP_MAX,								// Bounding box maxs.
		.quantity = HALF_OFF_MANA,						// Ammo type/ammo use per shot.
		.flags = (IT_AMMO | IT_OFFENSE),				// IT_XXX flags.
		.tag = ITEM_AMMO_MANA_OFFENSIVE_HALF,			// Tag.
	},

	{ // 17
		.classname = "item_mana_offensive_full",		// Spawnname (char *).
		.pickup_name = "Off-mana",						// Pickup name (char *).
		.msg_pickup = GM_OFFMANAB,						// Pickup message.
		.pickup_sound = "player/picup.wav",				// Pickup sound (char *).
		.mins = PICKUP_MIN,								// Bounding box mins.
		.maxs = PICKUP_MAX,								// Bounding box maxs.
		.quantity = FULL_OFF_MANA,						// Ammo type/ammo use per shot.
		.flags = (IT_AMMO | IT_OFFENSE),				// IT_XXX flags.
		.tag = ITEM_AMMO_MANA_OFFENSIVE_FULL,			// Tag.
	},

	{ // 18
		.classname = "item_mana_defensive_half",		// Spawnname (char *).
		.pickup_name = "Def-mana",						// Pickup name (char *).
		.msg_pickup = GM_DEFMANAS,						// Pickup message.
		.pickup_sound = "player/picup.wav",				// Pickup sound (char *).
		.mins = PICKUP_MIN,								// Bounding box mins.
		.maxs = PICKUP_MAX,								// Bounding box maxs.
		.quantity = HALF_DEF_MANA,						// Ammo type/ammo use per shot.
		.flags = (IT_AMMO | IT_DEFENSE),				// IT_XXX flags.
		.tag = ITEM_AMMO_MANA_DEFENSIVE_HALF,			// Tag.
	},

	{ // 19
		.classname = "item_mana_defensive_full",		// Spawnname (char *).
		.pickup_name = "Def-mana",						// Pickup name (char *).
		.msg_pickup = GM_DEFMANAB,						// Pickup message.
		.pickup_sound = "player/picup.wav",				// Pickup sound (char *).
		.mins = PICKUP_MIN,								// Bounding box mins.
		.maxs = PICKUP_MAX,								// Bounding box maxs.
		.quantity = FULL_DEF_MANA,						// Ammo type/ammo use per shot.
		.flags = (IT_AMMO | IT_DEFENSE),				// IT_XXX flags.
		.tag = ITEM_AMMO_MANA_DEFENSIVE_FULL,			// Tag.
	},

	{ // 20
		.classname = "item_mana_combo_quarter",			// Spawnname (char *).
		.pickup_name = "Def-mana",						// Pickup name (char *).
		.msg_pickup = GM_COMBMANAS,						// Pickup message.
		.pickup_sound = "player/picup.wav",				// Pickup sound (char *).
		.mins = PICKUP_MIN,								// Bounding box mins.
		.maxs = PICKUP_MAX,								// Bounding box maxs.
		.quantity = HALF_COMBO_MANA,					// Ammo type/ammo use per shot.
		.flags = IT_AMMO,								// IT_XXX flags.
		.tag = ITEM_AMMO_MANA_COMBO_QUARTER,			// Tag.
	},

	{ // 21
		.classname = "item_mana_combo_half",			// Spawnname (char *).
		.pickup_name = "Def-mana",						// Pickup name (char *).
		.msg_pickup = GM_COMBMANAB,						// Pickup message.
		.pickup_sound = "player/picup.wav",				// Pickup sound (char *).
		.mins = PICKUP_MIN,								// Bounding box mins.
		.maxs = PICKUP_MAX,								// Bounding box maxs.
		.quantity = FULL_COMBO_MANA,					// Ammo type/ammo use per shot.
		.flags = IT_AMMO,								// IT_XXX flags.
		.tag = ITEM_AMMO_MANA_COMBO_HALF,				// Tag.
	},

	{ // 22
		.classname = "item_ammo_redrain",				// Spawnname (char *).
		.pickup_name = "Red-Rain-Arrows",				// Pickup name (char *).
		.msg_pickup = GM_STORMARROWS,					// Pickup message.
		.pickup_sound = "player/picup.wav",				// Pickup sound (char *).
		.mins = PICKUP_MIN,								// Bounding box mins.
		.maxs = PICKUP_MAX,								// Bounding box maxs.
		.quantity = AMMO_COUNT_REDRAINBOW,				// Ammo type/ammo use per shot.
		.flags = (IT_AMMO | IT_OFFENSE),				// IT_XXX flags.
		.tag = ITEM_AMMO_REDRAIN,						// Tag.
		.icon = "icons/i_ammo-redrain.m8",				// Icon name (char *).
	},

	{ // 23
		.classname = "item_ammo_phoenix",				// Spawnname (char *).
		.pickup_name = "Phoenix-Arrows",				// Pickup name (char *).
		.msg_pickup = GM_PHOENARROWS,					// Pickup message.
		.pickup_sound = "player/picup.wav",				// Pickup sound (char *).
		.mins = PICKUP_MIN,								// Bounding box mins.
		.maxs = PICKUP_MAX,								// Bounding box maxs.
		.quantity = AMMO_COUNT_PHOENIXBOW,				// Ammo type/ammo use per shot.
		.flags = (IT_AMMO | IT_OFFENSE),				// IT_XXX flags.
		.tag = ITEM_AMMO_PHOENIX,						// Tag.
		.icon = "icons/i_ammo-phoen.m8",				// Icon name (char *).
	},

	{ // 24
		.classname = "item_ammo_hellstaff",				// Spawnname (char *).
		.pickup_name = "Hell-staff-ammo",				// Pickup name (char *).
		.msg_pickup = GM_HELLORB,						// Pickup message.
		.pickup_sound = "player/picup.wav",				// Pickup sound (char *).
		.mins = PICKUP_MIN,								// Bounding box mins.
		.maxs = PICKUP_MAX,								// Bounding box maxs.
		.quantity = AMMO_COUNT_HELLSTAFF,				// Ammo type/ammo use per shot.
		.flags = (IT_AMMO | IT_OFFENSE),				// IT_XXX flags.
		.tag = ITEM_AMMO_HELLSTAFF,						// Tag.
		.icon = "icons/i_ammo-hellstaff.m8",			// Icon name (char *).
	},

#pragma endregion

#pragma region ========================== HEALTH ==========================

	{ // 25
		.classname = "item_health_half",				// Spawnname (char *).
		.pickup_name = "Minor health",					// Pickup name (char *).
		.msg_pickup = GM_HEALTHVIAL,					// Pickup message.
		.pickup_sound = "*gethealth.wav",				// Pickup sound (char *).
		//.world_model = "models/items/health/healthsmall/tris.fm", // World model (char *). //mxd. Unused.
		.mins = PICKUP_MIN,								// Bounding box mins.
		.maxs = PICKUP_MAX,								// Bounding box maxs.
		.quantity = 10,									// Ammo type/ammo use per shot.
		.flags = (IT_HEALTH | EF_ALWAYS_ADD_EFFECTS), 	// IT_XXX flags.
		.tag = ITEM_HEALTH1,							// Tag. //mxd. MODEL_HEALTH1 in original logic.
	},

	{ // 26
		.classname = "item_health_full",				// Spawnname (char *).
		.pickup_name = "Major health",					// Pickup name (char *).
		.msg_pickup = GM_HEALTHPOTION,					// Pickup message.
		.pickup_sound = "*gethealth.wav",				// Pickup sound (char *).
		//.world_model = "models/items/health/healthbig/tris.fm", // World model (char *). //mxd. Unused.
		.mins = PICKUP_MIN,								// Bounding box mins.
		.maxs = PICKUP_MAX,								// Bounding box maxs.
		.quantity = 30,									// Ammo type/ammo use per shot.
		.flags = (IT_HEALTH | EF_ALWAYS_ADD_EFFECTS),	// IT_XXX flags.
		.tag = ITEM_HEALTH2,							// Tag. //mxd. MODEL_HEALTH2 in original logic.
	},

#pragma endregion

#pragma region ========================== PUZZLE PIECES ==========================

	{ // 27
		.classname = "item_puzzle_townkey",				// Spawnname (char *).
		.pickup_name = "Town Key",						// Pickup name (char *).
		.msg_pickup = GM_F_TOWNKEY,						// Pickup message.
		.msg_nouse = GM_NEED_TOWNKEY,					// Can`t use message.
		.pickup_sound = "player/picup.wav",				// Pickup sound (char *).
		.mins = { -8.0f, -8.0f, -4.0f },				// Bounding box mins.
		.maxs = {  8.0f,  8.0f,  4.0f },				// Bounding box maxs.
		.flags = IT_PUZZLE,								// IT_XXX flags.
		.tag = ITEM_TOWNKEY,							// Tag.
		.icon = "icons/p_townkey.m8",					// Icon name (char *).
	},

	{ // 28
		.classname = "item_puzzle_cog",					// Spawnname (char *).
		.pickup_name = "Cog",							// Pickup name (char *).
		.msg_pickup = GM_F_COG,							// Pickup message.
		.msg_nouse = GM_NEED_COG,						// Can`t use message.
		.pickup_sound = "player/picup.wav",				// Pickup sound (char *).
		.mins = { -10.0f, -10.0f, -24.0f },				// Bounding box mins.
		.maxs = {  10.0f,  10.0f,  20.0f },				// Bounding box maxs.
		.flags = IT_PUZZLE,								// IT_XXX flags.
		.tag = ITEM_COG,								// Tag.
		.icon = "icons/p_cog.m8",						// Icon name (char *).
	},

	{ // 29
		.classname = "item_puzzle_shield",				// Spawnname (char *).
		.pickup_name = "Defensive Shield",				// Pickup name (char *).
		.msg_pickup = GM_F_SHIELD,						// Pickup message.
		.msg_nouse = GM_NEED_SHIELD,					// Can`t use message.
		.pickup_sound = "player/picup.wav",				// Pickup sound (char *).
		.mins = { -2.0f, -6.0f, -12.0f },				// Bounding box mins.
		.maxs = {  2.0f,  6.0f,  12.0f },				// Bounding box maxs.
		.flags = IT_PUZZLE,								// IT_XXX flags.
		.tag = ITEM_SHIELD,								// Tag.
		.icon = "icons/p_shield.m8",					// Icon name (char *).
	},

	{ // 30
		.classname = "item_puzzle_potion",				// Spawnname (char *).
		.pickup_name = "Potion",						// Pickup name (char *).
		.msg_pickup = GM_F_POTION,						// Pickup message.
		.msg_nouse = GM_NEED_POTION,					// Can`t use message.
		.pickup_sound = "player/picup.wav",				// Pickup sound (char *).
		.mins = { -3.0f, -3.0f, -10.0f },				// Bounding box mins.
		.maxs = {  3.0f,  3.0f,  10.0f },				// Bounding box maxs.
		.flags = IT_PUZZLE,								// IT_XXX flags.
		.tag = ITEM_POTION,								// Tag.
		.icon = "icons/p_potion.m8",					// Icon name (char *).
	},

	{ // 31
		.classname = "item_puzzle_plazacontainer",		// Spawnname (char *).
		.pickup_name = "Container",						// Pickup name (char *).
		.msg_pickup = GM_F_CONT,						// Pickup message.
		.msg_nouse = GM_NEED_CONT,						// Can`t use message.
		.pickup_sound = "player/picup.wav",				// Pickup sound (char *).
		.mins = { -6.0f, -6.0f, -8.0f },				// Bounding box mins.
		.maxs = {  6.0f,  6.0f,  6.0f },				// Bounding box maxs.
		.flags = IT_PUZZLE,								// IT_XXX flags.
		.tag = ITEM_CONT,								// Tag.
		.icon = "icons/p_plazajug.m8",					// Icon name (char *).
	},

	{ // 32
		.classname = "item_puzzle_slumcontainer",		// Spawnname (char *).
		.pickup_name = "Full Container",				// Pickup name (char *).
		.msg_pickup = GM_F_CONTFULL,					// Pickup message.
		.msg_nouse = GM_NEED_CONTFULL,					// Can`t use message.
		.pickup_sound = "player/picup.wav",				// Pickup sound (char *).
		.mins = { -6.0f, -6.0f, -8.0f },				// Bounding box mins.
		.maxs = {  6.0f,  6.0f,  6.0f },				// Bounding box maxs.
		.flags = IT_PUZZLE,								// IT_XXX flags.
		.tag = ITEM_SLUMCONT,							// Tag.
		.icon = "icons/p_jugfill.m8",					// Icon name (char *).
	},

	{ // 33
		.classname = "item_puzzle_crystal",				// Spawnname (char *).
		.pickup_name = "Crystal",						// Pickup name (char *).
		.msg_pickup = GM_F_CRYSTAL,						// Pickup message.
		.msg_nouse = GM_NEED_CRYSTAL,					// Can`t use message.
		.pickup_sound = "player/picup.wav",				// Pickup sound (char *).
		.mins = PICKUP_MIN,								// Bounding box mins.
		.maxs = PICKUP_MAX,								// Bounding box maxs.
		.flags = IT_PUZZLE,								// IT_XXX flags.
		.tag = ITEM_CRYSTAL,							// Tag.
		.icon = "icons/p_crystal.m8",					// Icon name (char *).
	},

	{ // 34
		.classname = "item_puzzle_canyonkey",			// Spawnname (char *).
		.pickup_name = "Canyon Key",					// Pickup name (char *).
		.msg_pickup = GM_F_CANYONKEY,					// Pickup message.
		.msg_nouse = GM_NEED_CANYONKEY,					// Can`t use message.
		.pickup_sound = "player/picup.wav",				// Pickup sound (char *).
		.mins = PICKUP_MIN,								// Bounding box mins.
		.maxs = PICKUP_MAX,								// Bounding box maxs.
		.flags = IT_PUZZLE,								// IT_XXX flags.
		.tag = ITEM_CANKEY,								// Tag.
		.icon = "icons/p_canyonkey.m8",					// Icon name (char *).
	},

	{ // 35
		.classname = "item_puzzle_hive2amulet",			// Spawnname (char *).
		.pickup_name = "Hive 2 Amulet",					// Pickup name (char *).
		.msg_pickup = GM_F_AMULET,						// Pickup message.
		.msg_nouse = GM_NEED_AMULET,					// Can`t use message.
		.pickup_sound = "player/picup.wav",				// Pickup sound (char *).
		.mins = PICKUP_MIN,								// Bounding box mins.
		.maxs = PICKUP_MAX,								// Bounding box maxs.
		.flags = IT_PUZZLE,								// IT_XXX flags.
		.tag = ITEM_AMULET,								// Tag.
		.icon = "icons/p_tcheckrikbust.m8",				// Icon name (char *).
	},

	{ // 36
		.classname = "item_puzzle_hive2spear",			// Spawnname (char *).
		.pickup_name = "Hive 2 Spear",					// Pickup name (char *).
		.msg_pickup = GM_F_SPEAR,						// Pickup message.
		.msg_nouse = GM_NEED_SPEAR,						// Can`t use message.
		.pickup_sound = "player/picup.wav",				// Pickup sound (char *).
		.mins = PICKUP_MIN,								// Bounding box mins.
		.maxs = PICKUP_MAX,								// Bounding box maxs.
		.flags = IT_PUZZLE,								// IT_XXX flags.
		.tag = ITEM_SPEAR,								// Tag.
		.icon = "icons/p_spear.m8",						// Icon name (char *).
	},

	{ // 37
		.classname = "item_puzzle_hive2gem",			// Spawnname (char *).
		.pickup_name = "Hive 2 Gem",					// Pickup name (char *).
		.msg_pickup = GM_F_GEM,							// Pickup message.
		.msg_nouse = GM_NEED_GEM,						// Can`t use message.
		.pickup_sound = "player/picup.wav",				// Pickup sound (char *).
		.mins = PICKUP_MIN,								// Bounding box mins.
		.maxs = PICKUP_MAX,								// Bounding box maxs.
		.flags = IT_PUZZLE,								// IT_XXX flags.
		.tag = ITEM_GEM,								// Tag.
		.icon = "icons/p_tcheckrikgem.m8",				// Icon name (char *).
	},

	{ // 38
		.classname = "item_puzzle_minecartwheel",		// Spawnname (char *).
		.pickup_name = "Minecart Wheel",				// Pickup name (char *).
		.msg_pickup = GM_F_CARTWHEEL,					// Pickup message.
		.msg_nouse = GM_NEED_CARTWHEEL,					// Can`t use message.
		.pickup_sound = "player/picup.wav",				// Pickup sound (char *).
		.mins = { -1.0f, -6.0f, -6.0f },				// Bounding box mins.
		.maxs = {  1.0f,  6.0f,  6.0f },				// Bounding box maxs.
		.flags = IT_PUZZLE,								// IT_XXX flags.
		.tag = ITEM_WHEEL,								// Tag.
		.icon = "icons/p_wheel.m8",						// Icon name (char *).
	},

	{ // 39
		.classname = "item_puzzle_ore",					// Spawnname (char *).
		.pickup_name = "Ore",							// Pickup name (char *).
		.msg_pickup = GM_F_UNREFORE,					// Pickup message.
		.msg_nouse = GM_NEED_UNREFORE,					// Can`t use message.
		.pickup_sound = "player/picup.wav",				// Pickup sound (char *).
		.mins = { -10.0f, -10.0f, -8.0f },				// Bounding box mins.
		.maxs = {  10.0f,  10.0f,  8.0f },				// Bounding box maxs.
		.flags = IT_PUZZLE,								// IT_XXX flags.
		.tag = ITEM_ORE,								// Tag.
		.icon = "icons/p_oreunrefined.m8",				// Icon name (char *).
	},

	{ // 40
		.classname = "item_puzzle_refinedore",			// Spawnname (char *).
		.pickup_name = "Refined Ore",					// Pickup name (char *).
		.msg_pickup = GM_F_REFORE,						// Pickup message.
		.msg_nouse = GM_NEED_REFORE,					// Can`t use message.
		.pickup_sound = "player/picup.wav",				// Pickup sound (char *).
		.mins = { -3.0f, -12.0f, -2.0f },				// Bounding box mins.
		.maxs = {  3.0f,  12.0f,  2.0f },				// Bounding box maxs.
		.flags = IT_PUZZLE,								// IT_XXX flags.
		.tag = ITEM_REF_ORE,							// Tag.
		.icon = "icons/p_orerefined.m8",				// Icon name (char *).
	},

	{ // 41
		.classname = "item_puzzle_dungeonkey",			// Spawnname (char *).
		.pickup_name = "Dungeon Key",					// Pickup name (char *).
		.msg_pickup = GM_F_DUNGEONKEY,					// Pickup message.
		.msg_nouse = GM_NEED_DUNGEONKEY,				// Can`t use message.
		.pickup_sound = "player/picup.wav",				// Pickup sound (char *).
		.mins = { -1.0f, -18.0f, -9.0f },				// Bounding box mins.
		.maxs = {  1.0f,  18.0f,  9.0f },				// Bounding box maxs.
		.flags = IT_PUZZLE,								// IT_XXX flags.
		.tag = ITEM_DUNKEY,								// Tag.
		.icon = "icons/p_dungeonkey.m8",				// Icon name (char *).
	},

	{ // 42
		.classname = "item_puzzle_cloudkey",			// Spawnname (char *).
		.pickup_name = "Cloud Key",						// Pickup name (char *).
		.msg_pickup = GM_F_CLOUDKEY,					// Pickup message.
		.msg_nouse = GM_NEED_CLOUDKEY,					// Can`t use message.
		.pickup_sound = "player/picup.wav",				// Pickup sound (char *).
		.mins = { -8.0f, -8.0f, -3.0f },				// Bounding box mins.
		.maxs = {  8.0f,  8.0f,  3.0f },				// Bounding box maxs.
		.flags = IT_PUZZLE,								// IT_XXX flags.
		.tag = ITEM_CLOUDKEY,							// Tag.
		.icon = "icons/p_cloudkey.m8",					// Icon name (char *).
	},

	{ // 43
		.classname = "item_puzzle_highpriestesskey",	// Spawnname (char *).
		.pickup_name = "Key",							// Pickup name (char *).
		.msg_pickup = GM_F_HIGHKEY,						// Pickup message.
		.msg_nouse = GM_NEED_HIGHKEY,					// Can`t use message.
		.pickup_sound = "player/picup.wav",				// Pickup sound (char *).
		.mins = { -12.0f, -12.0f, -6.0f },				// Bounding box mins.
		.maxs = {  12.0f,  12.0f,  6.0f },				// Bounding box maxs.
		.flags = IT_PUZZLE,								// IT_XXX flags.
		.tag = ITEM_HIVEKEY,							// Tag.
		.icon = "icons/p_hivekey.m8",					// Icon name (char *).
	},

	{ // 44
		.classname = "item_puzzle_highpriestesssymbol",	// Spawnname (char *).
		.pickup_name = "Symbol",						// Pickup name (char *).
		.msg_pickup = GM_F_SYMBOL,						// Pickup message.
		.msg_nouse = GM_NEED_SYMBOL,					// Can`t use message.
		.pickup_sound = "player/picup.wav",				// Pickup sound (char *).
		.mins = { -12.0f, -12.0f, -4.0f },				// Bounding box mins.
		.maxs = {  12.0f,  12.0f,  4.0f },				// Bounding box maxs.
		.flags = IT_PUZZLE,								// IT_XXX flags.
		.tag = ITEM_HPSYM,								// Tag.
		.icon = "icons/p_queenkey.m8",					// Icon name (char *).
	},

	{ // 45
		.classname = "item_puzzle_tome",				// Spawnname (char *).
		.pickup_name = "Tome",							// Pickup name (char *).
		.msg_pickup = GM_F_TOME,						// Pickup message.
		.msg_nouse = GM_NEED_TOME,						// Can`t use message.
		.pickup_sound = "player/picup.wav",				// Pickup sound (char *).
		.mins = { -12.0f, -12.0f, -4.0f },				// Bounding box mins.
		.maxs = {  12.0f,  12.0f,  4.0f },				// Bounding box maxs.
		.flags = IT_PUZZLE,								// IT_XXX flags.
		.tag = ITEM_TOME,								// Tag.
		.icon = "icons/p_tomepower.m8",					// Icon name (char *).
	},

	{ // 46
		.classname = "item_puzzle_tavernkey",			// Spawnname (char *).
		.pickup_name = "Tavern Key",					// Pickup name (char *).
		.msg_pickup = GM_F_TAVERNKEY,					// Pickup message.
		.msg_nouse = GM_NEED_TAVERNKEY,					// Can`t use message.
		.pickup_sound = "player/picup.wav",				// Pickup sound (char *).
		.mins = { -12.0f, -12.0f, -4.0f },				// Bounding box mins.
		.maxs = {  12.0f,  12.0f,  4.0f },				// Bounding box maxs.
		.flags = IT_PUZZLE,								// IT_XXX flags.
		.tag = ITEM_TAVERNKEY,							// Tag.
		.icon = "icons/p_tavernkey.m8",					// Icon name (char *).
	},

#pragma endregion

	{ // 47 //TODO: move to 'defence powerups' category? Will that break vanilla compatibility?
		.classname = "item_defense_tornado",			// Spawnname (char *).
		.pickup_name = "tornado",						// Pickup name (char *).
		.msg_pickup = GM_TORNADO,						// Pickup message.
		.msg_nouse = GM_NOTORNADO,						// Can`t use message.
		.pickup_sound = "player/getweapon.wav",			// Pickup sound (char *).
		.world_model_flags = EF_ROTATE,					// World model flags.
		.mins = PICKUP_MIN,								// Bounding box mins.
		.maxs = PICKUP_MAX,								// Bounding box maxs.
		.playeranimseq = ASEQ_SPELL_DEF,				// Player animation sequence to engage when used.
		.altanimseq = ASEQ_SPELL_DEF,					// Player animation sequence to engage when powered
		.quantity = MANA_USE_TORNADO,					// Ammo type/ammo use per shot.
		.ammo = "Def-mana",								// Ammo name (char *).
		.flags = IT_DEFENSE,							// IT_XXX flags.
		.tag = ITEM_DEFENSE_TORNADO,					// Tag.
		.icon = "icons/i_tornado.m8",					// Icon name (char *).
	},

	// End of list marker.
	{ 0 }
};

#pragma endregion

PLAYER_API void InitItems(void)
{
	p_itemlist = itemlist;
	p_num_items = (sizeof(itemlist) / sizeof(itemlist[0])) - 1; //mxd. Last item is NULL end of list marker.
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