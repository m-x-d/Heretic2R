//
// g_itemstats.h
//
// Copyright 1998 Raven Software
//

#pragma once

#pragma region ========================== SHRINE STATS ==========================

#define LIGHT_DURATION				90.0f	// Duration of shrine light on player - in seconds.
#define REFLECT_DURATION_SINGLE		30.0f	// Duration of shrine reflect on player - in seconds.
#define REFLECT_DURATION_DEATHMATCH	20.0f	// Duration of shrine reflect on player - in seconds.
#define GHOST_DURATION				60.0f	// Duration of shrine ghosting on player - in seconds.
#define POWERUP_DURATION			30.0f	// Duration of shrine powerup on player weapons - in seconds.
#define LUNGS_DURATION				20.0f	// Duration of shrine lungs on player - in seconds.
#define SHRINE_DELAY				45.0f	// Time till Shrine is useable again if so toggled - in seconds.
#define SPEED_DURATION				25.0f	// Duration of shrine speed on player - in seconds.

// Amount of armor you get at a shrine - note support is in for gold armor - you just can't actually get it at this stage.
#define MAX_GOLD_ARMOR		250.0f	// Amount of damage it can absorb.
#define MAX_SILVER_ARMOR	100.0f	// Amount of damage it can absorb.
#define SILVER_HIT_MULT		0.5f	// Percentage of hit damage we take on a physical hit - silver armor.
#define SILVER_SPELL_MULT	0.5f	// Percentage of hit damage we take on a spell hit - silver armor.
#define GOLD_HIT_MULT		0.25f	// Percentage of hit damage we take on a physical hit - gold armor.
#define GOLD_SPELL_MULT		0.25f	// Percentage of hit damage we take on a spell hit - gold armor.

#define SHRINE_HEALTH		100
#define SHRINE_MAX_HEALTH	150

#pragma endregion

#pragma region ========================== MANA ITEM STATS ==========================

#define HALF_OFF_MANA	20
#define FULL_OFF_MANA	40

#define HALF_DEF_MANA	20
#define FULL_DEF_MANA	40

#define HALF_COMBO_MANA	30
#define FULL_COMBO_MANA	30

#pragma endregion

#pragma region ========================== SPELL ITEM STATS ==========================

// In seconds.
#define RESPAWN_TIME_WEAPON		30.0f
#define RESPAWN_TIME_MACEBALL	60.0f
#define RESPAWN_TIME_DEFENSE	30.0f
#define RESPAWN_TIME_RING		20.0f
#define RESPAWN_TIME_TELEPORT	25.0f
#define RESPAWN_TIME_MORPH		40.0f
#define RESPAWN_TIME_AMMO		20.0f
#define RESPAWN_TIME_ARROWS		30.0f
#define RESPAWN_TIME_MISC		20.0f

// Ammunition usage
#define AMMO_USE_FIREBALL		2
#define AMMO_USE_HELLSTAFF		1
#define AMMO_USE_MAGICMISSILE	3
#define AMMO_USE_REDRAIN		1
#define AMMO_USE_SPHERE			7
#define AMMO_USE_PHOENIX		1
#define AMMO_USE_MACEBALL		12
#define AMMO_USE_FIREWALL		5

#define MANA_USE_POWERUP		60
#define MANA_USE_RING			5
#define MANA_USE_SHIELD			20
#define MANA_USE_TELEPORT		15
#define MANA_USE_POLYMORPH		30
#define MANA_USE_METEORS		5	// Five PER meteor: Total of 20.
#define MANA_USE_TORNADO		30

#define AMMO_COUNT_MOST			20
#define AMMO_COUNT_REDRAINBOW	5
#define AMMO_COUNT_PHOENIXBOW	5
#define AMMO_COUNT_HELLSTAFF	50

#define MAX_OFF_MANA			100
#define MAX_DEF_MANA			100
#define MAX_RAIN_AMMO			30
#define MAX_PHOENIX_AMMO		20
#define MAX_HELL_AMMO			200

#pragma endregion