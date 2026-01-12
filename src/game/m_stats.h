//
// m_stats.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "g_ClassStatics.h"

extern int AttackRangesForClass[NUM_ATTACK_RANGES];
extern byte MaxBuoysForClass[NUM_CLASSIDS];
extern int JumpChanceForClass[NUM_CLASSIDS];
extern vec3_t STDMinsForClass[NUM_CLASSIDS];
extern vec3_t STDMaxsForClass[NUM_CLASSIDS];

typedef struct MonsterShadow_s
{
	qboolean use_shadow;
	float scale;
} MonsterShadow_t;

extern struct MonsterShadow_s G_MonsterShadow[NUM_CLASSIDS];

typedef struct EvadeChance_s //mxd
{
	const int duck_chance;
	const int dodgeleft_chance;
	const int dodgeright_chance;
	const int jump_chance;
	const int backflip_chance;
	const int frontflip_chance;
} EvadeChance_t;

#define ITEM_REWARD_CHANCE		1
#define MAX_SIGHT_PLAYER_DIST	1000.0f

#pragma region ========================== GENERIC MONSTER SPAWNFLAGS ==========================

#define MSF_NONE			0x000000
#define MSF_AMBUSH			0x000001
#define MSF_ASLEEP			0x000002 // Will not do anything until used.

// Next three are mutually exclusive, sorry - that's how it was made, would be a pain to change now - should make walking a separate one from other two...
#define MSF_WALKING			0x000004
#define MSF_EATING			0x000004
#define MSF_PERCHING		0x000004 // Only for harpies and imps.

// Spawnflags individual to the monster.
#define MSF_SPECIAL1		0x000008
#define MSF_SPECIAL2		0x000010 //TODO: unused.
#define MSF_SPECIAL3		0x000020 //TODO: unused.
#define MSF_SPECIAL4		0x000040 //TODO: unused.

#define MSF_FIXED			0x000080 // Monster will not move, only attack.

//mxd. Next are SPAWNFLAG_NOT_EASY (0x100) - SPAWNFLAG_NOT_COOP (0x1000) (defined in g_local.h).

// Extra spawnflags since our creatures are so freakin awesome!
// These 4 are for all creatures, cannot be creature-specific.
#define MSF_WANDER			0x002000
#define MSF_MELEE_LEAD		0x004000
#define MSF_STALK			0x008000
#define MSF_COWARD			0x010000

// Extra spawnflags - individual to the monster.
#define MSF_EXTRA1			0x020000
#define MSF_EXTRA2			0x040000 //TODO: unused.
#define MSF_EXTRA3			0x080000 //TODO: unused.
#define MSF_EXTRA4			0x100000

#pragma endregion

#pragma region ========================== MONSTER-SPECIFIC SPAWNFLAGS ==========================

// SSITHRA
#define MSF_SSITHRA_NAMOR		8	// Ssithra will jump out of water when woken up or used.
#define MSF_SSITHRA_SPIN		16	// Ssithra will spin around when woken up or used.
#define MSF_SSITHRA_ALPHA		32	// Tougher ssithra.
#define MSF_SSITHRA_CLOTHED		64	// Clothed, shoot explosive tip projectiles.

// ASSASSIN
#define MSF_ASS_JUMPAMBUSH		8	// Will jump when awoken.
#define MSF_ASS_NOSHADOW		16	// Cannot cloak.
#define MSF_ASS_NOTELEPORT		32	// Cannot teleprt.
#define MSF_ASS_CINEMATIC		64	// A cinematic monster.
#define MSF_ASS_TPORTAMBUSH		0x020000 // Will spawn in teleport when triggered.
#define MSF_ASS_STARTSHADOW		0x040000 // Assassin starts shadowed and de-cloaks when woken.
#define MSF_ASS_SIDEJUMPAMBUSH	0x080000 // Assassin will jump out sideways.
#define MSF_ASS_TELEPORTDODGE	0x100000 // Assassin will use teleports to dodge.

// GORGON
#define MSF_GORGON_SPEEDY		8 // 2x turn rate.
#define MSF_GORGON_COWARD		64

// Plague Elves
#define MSF_PELF_CINEMATIC		8	// Cinematic plague elf.
#define MSF_PELF_MISSILE		16	// Missile-bearing plague elf.
#define MSF_PELF_ONFIRE			32	// Starts on fire. //TODO: unused.

// Insects
#define MSF_INSECT_CINEMATIC	8	// Cinematic insect.
#define MSF_INSECT_BEAST_FODDER	16	// Looks for beasts.
#define MSF_INSECT_YELLOWJACKET	32	// Different skin and projectile - male.
#define MSF_INSECT_ALTERNATE	32	// Different tint and projectile - female.

// Ogles - special cases - have their own spawnflags.
#define OF_PUSHING				1
#define OF_PICK_UP				4
#define OF_PICK_DOWN			8
#define OF_CHISEL_UP			16
#define OF_CHISEL_DOWN			32 //TODO: unused.
#define OF_HAMMER_UP			64
#define OF_HAMMER_DOWN			128
#define OF_SONG_LEADER			256
#define OF_CINEMATIC			8448

#pragma endregion

#pragma region ========================== MONSTER STATS ==========================

#define MAX_BLOCKING_THING_HEALTH	100 // Highest health some blocking ent can be and let a monster still hack away at it to clear it's path //TODO: unused.

// ASSASSIN
#define ASSASSIN_HEALTH				250
#define ASSASSIN_MASS				180
#define ASSASSIN_MIN_DAMAGE			7
#define ASSASSIN_MAX_DAMAGE			13
#define ASSASSIN_MIN_CLOAK_RANGE	100
#define ASSASSIN_DAGGER_SPEED		500.0f
#define ASS_TP_OFF					1 //TODO: rename to ASSASSIN_TELEPORT_OFFENSIVE
#define ASS_TP_ANY					2 //TODO: rename to ASSASSIN_TELEPORT_ANY
#define ASS_TP_DEF					3 //TODO: rename to ASSASSIN_TELEPORT_DEFENSIVE

// TRIAL BEAST
#define TB_HEALTH				7000
#define TB_MASS					10000
#define TB_DMG_BITE_MIN			30
#define TB_DMG_BITE_MAX			50
#define TB_DMG_IMPACT_MIN		10
#define TB_DMG_IMPACT_MAX		100
#define TB_DMG_IMPACT_KB		150

// BEE
#define BEE_HEALTH				50 //TODO: unused.
#define BEE_MASS				10 //TODO: unused.

// CHICKEN
#define CHICKEN_HEALTH			10
#define CHICKEN_MASS			30

// ELF LORD
#define ELFLORD_HEALTH			2250
#define ELFLORD_MASS			2500
#define ELFLORD_STAR_MIN_DAMAGE	8
#define ELFLORD_STAR_MAX_DAMAGE	12
#define ELFLORD_BEAM_MIN_DAMAGE	4
#define ELFLORD_BEAM_MAX_DAMAGE	8

// FISH
#define FISH_HEALTH				50
#define FISH_MASS				120
#define FISH_DMG_BITE_MIN		5
#define FISH_DMG_BITE_MAX		10

// GKROKON
#define GKROKON_HEALTH			75
#define GKROKON_MASS			60
#define GKROKON_DMG_SPOO_MIN	4
#define GKROKON_DMG_SPOO_MAX	8
#define GKROKON_DMG_BITE_MIN	5
#define GKROKON_DMG_BITE_MAX	10

// GORGON
#define GORGON_HEALTH			100
#define GORGON_MASS				80
#define GORGON_SCALE_MIN		0.9f
#define GORGON_SCALE_MAX		1.2f
#define GORGON_DMG_MIN			5	// Note: This is multiplied by the scale of the creature.
#define GORGON_DMG_MAX			10	// Note: This is multiplied by the scale of the creature.
#define GORGON_LEADER_HEALTH	200 //TODO: unused.
#define GORGON_LEADER_MASS		300 //TODO: unused.
#define GORGON_ALERT_DIST		500.0f
#define GORGON_SKIN				0
#define GORGON_PAIN_SKIN		1

// HARPY
#define HARPY_HEALTH			60
#define HARPY_MASS				100
#define HARPY_DMG_MIN			10
#define HARPY_DMG_MAX			20

// IMP
#define IMP_HEALTH				200
#define IMP_MASS				75
#define IMP_DMG_MIN				10
#define IMP_DMG_MAX				20

// MORCALAVIN
#define MORK_HEALTH1			100
#define MORK_HEALTH				2500
#define MORK_MASS				2500
#define MORK_DMG_BEAM_MIN		10
#define MORK_DMG_BEAM_MAX		20
#define MORK_DMG_PROJ1_MIN		2
#define MORK_DMG_PROJ1_MAX		5
#define MORK_DMG_PUFF_RADIUS	32	//TODO: unused.
#define MORK_DMG_PUFF_MAX		128	//TODO: unused.
#define MORK_DMG_PUFF_MIN		8	//TODO: unused.
#define MORK_DMG_EYES_MULT		200	//TODO: unused.
#define MORK_DMG_EYES_MIN		10	//TODO: unused.

// MUTANT SSITHRA
#define MSSITHRA_HEALTH			2250
#define MSSITHRA_MASS			2000
#define MSSITHRA_DMG_MIN		8
#define MSSITHRA_DMG_MAX		16
#define MSSITHRA_DMG_SWIPE		40
#define MSSITHRA_ARROW_DMG_RADIUS	128.0f //mxd. 'MSSITHRA_DMG_ARROW_RADIUS 50' (unused) in original logic.
#define MSSITHRA_ARROW_SPEED	600.0f

// OGLE
#define OGLE_HEALTH				100
#define OGLE_MASS				100
#define OGLE_DMG_MIN			5
#define OGLE_DMG_MAX			8

// PLAGUE ELF
#define PLAGUEELF_HEALTH		50
#define PLAGUEELF_MASS			150
#define PLAGUEELF_DMG_MIN		2
#define PLAGUEELF_DMG_MAX		4
#define PLAGUEELF_DMG_HOE		1	// Modifiers to the min/max damage.
#define PLAGUEELF_DMG_GAFF		2	// Modifiers to the min/max damage.
#define PLAGUEELF_DMG_HAMMER	-1	// Modifiers to the min/max damage.
#define PLAGUEELF_DMG_SPELL_MIN	4
#define PLAGUEELF_DMG_SPELL_MAX	8
#define PLAGUEELF_GUARD_DMG_SPELL_MIN	5
#define PLAGUEELF_GUARD_DMG_SPELL_MAX	10
#define PALACE_ELF_SKIN			4	// For guards.
#define DYING_ELF_PAIN_VOICE	0
#define DYING_ELF_IDLE_VOICE	1
#define DYING_ELF_TOUCH_VOICE	2

// PLAGUE SSITHRA
#define SSITHRA_HEALTH				125
#define SSITHRA_MASS				200
#define SSITHRA_DMG_MIN				6
#define SSITHRA_DMG_MAX				8
#define SSITHRA_DMG_ARROW_RADIUS	50
#define SSITHRA_JUMP_VELOCITY		300.0f //TODO: unused.
#define SSITHRA_MAX_JUMP_VELOCITY	750.0f //mxd
#define SSITHRA_HOP_VELOCITY		128.0f
#define SSITHRA_SPOO_SPEED			450.0f //TODO: rename to SSITHRA_ARROW_SPEED.
#define SSITHRA_BIGARROW_DMG_MIN	16
#define SSITHRA_BIGARROW_DMG_MAX	24

// HIGH PRIESTESS
#define HP_HEALTH				2500
#define HP_MASS					2000
#define HP_DMG_FURY_MIN			3
#define HP_DMG_FURY_MAX			5
#define HP_DMG_BROOD_MIN		8
#define HP_DMG_BROOD_MAX		16
#define HP_DMG_RAIN				2
#define HP_DMG_MISSILE_MIN		2
#define HP_DMG_MISSILE_MAX		4
#define HP_DMG_FIRE_MIN			2
#define HP_DMG_FIRE_MAX			4

// RAT
#define RAT_HEALTH				10
#define RAT_MASS				10
#define RAT_DMG_BITE			1
#define RAT_GROUP_RANGE			150

// SERAPH OVERLORD
#define SERAPH_HEALTH			250
#define SERAPH_MASS				350
#define SERAPH_DMG_WHIP			5
#define SERAPH_DMG_WHIP_RUN		8

// SERAPH GUARD
#define SGUARD_HEALTH			300
#define SGUARD_MASS				1000
#define SGUARD_DMG_AXE			15
#define SGUARD_DMG_AXE_SPIN		30
#define SGUARD_DMG_SPELL_MIN	10
#define SGUARD_DMG_SPELL_MAX	15

// PLAGUE SPREADER
#define SPREADER_HEALTH				200
#define SPREADER_MASS				275
#define SPREADER_DMG_DEATH			1	//TODO: unused.
#define SPREADER_DMG_DEATH_RAD		30	//TODO: unused.
#define SPREADER_DMG_RAM			10	//TODO: unused.
#define SPREADER_DMG_RAM_KB			20	//TODO: unused.
#define SPREADER_DMG_TAKEOFF		1	//TODO: unused.
#define SPREADER_DMG_TAKEOFF_RAD	100	//TODO: unused.
#define SPREADER_GRENADE_DAMAGE		40		//mxd. 20 and unused in original logic.
#define SPREADER_GRENADE_RADIUS		100.0f	//mxd. Unused in original logic.
#define SPREADER_GRENADE_TIME		2.9f	//mxd. Unused in original logic. //TODO: rename to SPREADER_GRENADE_PAUSE_TIME.
#define SPREADER_MIST_DAMAGE		1	//TODO: unused.
#define SPREADER_MIST_RADIUS		40	//TODO: unused.
#define SPREADER_MIST_GRADIUS		30	//TODO: unused.

// TCHEKRIK
#define TC_HEALTH_MALE				200
#define TC_HEALTH_FEMALE			200
#define MASS_TC_MALE				300 //TODO: rename to TC_MASS_MALE
#define MASS_TC_FEMALE				250 //TODO: rename to TC_MASS_FEMALE
#define TC_DMG_STAB_MIN				5
#define TC_DMG_STAB_MAX				15
#define TC_MALE_DMG_HACK_MIN		15
#define TC_MALE_DMG_HACK_MAX		25
#define TC_FEMALE_DMG_HACK_MIN		3
#define TC_FEMALE_DMG_HACK_MAX		8
#define TC_DMG_FIREBALL_MIN			3 //TODO: unused.
#define TC_DMG_FIREBALL_MAX			5 //TODO: unused.
#define TC_DMG_SPEAR_MIN			1
#define TC_DMG_SPEAR_MAX			3
#define TC_DMG_YSPEAR_MIN			6
#define TC_DMG_YSPEAR_MAX			12
#define GLOBE_DAMAGE_MIN			10 //TODO: unused.
#define GLOBE_DAMAGE_RANGE			35 //TODO: unused.
#define GLOBE_EXPLOSION_RADIUS_MIN	89.0f	//TODO: unused.
#define GLOBE_EXPLOSION_RADIUS_MAX	160.0f	//TODO: unused.
#define INSECT_STAFF_SPEED			100.0f
#define INSECT_STAFF_AIMED_SPEED	750.0f

#pragma endregion