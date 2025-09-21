//
// g_playstats.h
//
// Copyright 1998 Raven Software
//

#pragma once

#pragma region ========================== WEAPON STATS ==========================

#define MAX_REFLECT						8

// Sword staff.
#define SWORD_DMG_MIN					15
#define SWORD_DMG_MAX					25
#define SWORD_POWER1_DMG_MIN			30
#define SWORD_POWER1_DMG_MAX			40
#define SWORD_POWER2_DMG_MIN			45
#define SWORD_POWER2_DMG_MAX			55

#define SWORD_SPIN_DMG_MOD				1.75f
#define SWORD_BACK_DMG_MOD				0.7f
#define SWORD_STAB_DMG_MOD				2.5f

// Flying Fist.
#define FIREBALL_MIN_FIZZLE_DAMAGE		5		// Least amount of damage it will do after flying.
#define FIREBALL_DAMAGE_MIN				20	
#define FIREBALL_DAMAGE_MAX				25
#define FIREBALL_DAMAGE_MIN_POWER		30
#define FIREBALL_DAMAGE_MAX_POWER		40
#define FIREBALL_RADIUS					64.0f	// Radius of damage for the powered up version.
#define FLYING_FIST_SPEED				1000.0f

// Hellstaff.
#define HELLBOLT_DAMAGE_MIN				7
#define HELLBOLT_DAMAGE_MAX				10
#define HELLLASER_DIST					1024.0f
#define HELLLASER_DAMAGE_MIN			11
#define HELLLASER_DAMAGE_MAX			14
#define HELLBOLT_SPEED					1000.0f

// Blast (Arc of Thunder).
#define BLAST_DISTANCE					1000.0f	// Max range of the blast.
#define BLAST_DMG_MIN					10
#define BLAST_DMG_MAX					15
#define BLAST_ANGLE_INC					3.5f
#define BLAST_NUM_SHOTS					5

// Magic Missile array (powered up Arc).
#define MAGICMISSILE_DAMAGE_MIN			30
#define MAGICMISSILE_DAMAGE_MAX			40
#define MAGICMISSILE_DAMAGE_RAD			20
#define MAGICMISSILE_RADIUS				48.0f
#define MAGICMISSILE_SPEED				1500.0f

// Red Rain Arrows.
#define RED_RAIN_DMG_ARROW_MIN			35
#define RED_RAIN_DMG_ARROW_MAX			45
#define RED_RAIN_DURATION				8.0f
#define RED_RAIN_DAMAGE_INTERVAL		0.25f
#define RED_RAIN_DAMAGE					10		// 10 points every .25 seconds.
#define RED_RAIN_RADIUS					60.0f
#define MAX_REDRAINHEIGHT				200.0f
#define MAX_REDRAINFALLDIST				1000.0f
#define RED_RAIN_LIGHTNING_DURATION		(RED_RAIN_DURATION - 1.5f)
#define RED_RAIN_LIGHTNING_CHANCE		6		// 1 in 6 chance per target every .25 seconds
#define RED_RAIN_LIGHTNING_RADIUS		100.0f
#define RED_RAIN_LIGHTNING_WIDTH		6.0f
#define RED_RAIN_DMG_LIGHTNING_MIN		20
#define RED_RAIN_DMG_LIGHTNING_MAX		30
#define NUM_STORMS_PER_PLAYER			3		// Maybe 5, esp in DM?

#define RED_ARROW_SPEED					1000.0f

// Powered up red rain arrows.
#define POWER_RAIN_DMG_ARROW_MIN		45
#define POWER_RAIN_DMG_ARROW_MAX		55
#define POWER_RAIN_DAMAGE				15		// 15 points every .25 seconds.
#define POWER_RAIN_RADIUS				80.0f
#define POWER_RAIN_LIGHTNING_RADIUS		120.0f
#define POWER_RAIN_LIGHTNING_WIDTH		40.0f
#define POWER_RAIN_DMG_LIGHTNING_MIN	30		// Big nasty beam of thick lightning with radius damage.
#define POWER_RAIN_DMG_LIGHTNING_MAX	45
#define POWER_RAIN_DMG_LIGHTNING_RADIUS	100

// Fireblast (unpowered firewall).
#define FIREBLAST_SPEED					480.0f
#define FIREBLAST_RADIUS				48.0f
#define FIREBLAST_PROJ_RADIUS			8.0f
#define FIREBLAST_DRADIUS				1.0f
#define FIREBLAST_VRADIUS				16
#define FIREBLAST_DAMAGE				48
#define FIREBLAST_DAMAGE_MIN			32

// Firewall (powered fireblast!).
#define FIREWAVE_UP						80
#define FIREWAVE_DOWN					48
#define FIREWAVE_SPEED					320.0f	// Speed of the firewall.
#define FIREWAVE_DM_SPEED				450.0f	// Speed of the firewall in deathmatch.
#define FIREWAVE_PROJ_RADIUS			8.0f
#define FIREWAVE_RADIUS					20.0f	// Size of each element of the firewall.
#define FIREWAVE_DRADIUS				FIREWAVE_RADIUS
#define FIREWAVE_DAMAGE					75		// How much damage the wall does every think.
#define FIREWAVE_DAMAGE_MIN				50
#define FIREWAVE_WORM_DAMAGE			30

// Phoenix Arrows.
#define PHOENIX_EXPLODE_DAMAGE			175		// Silver armor + 100 health ends with 13 health. Demo: 225.
#define PHOENIX_EXPLODE_RADIUS			128.0f
#define PHOENIX_EXPLODE_DAMAGE_POWER	300		// Gold armor + 100 health ends with 25 health. Demo: 350.
#define PHOENIX_EXPLODE_RADIUS_POWER	144.0f
#define PHOENIX_ARROW_SPEED				640.0f

// Sphere of Annihilation.
#define SPHERE_DAMAGE					100
#define SPHERE_RADIUS_MIN				48.0f
#define SPHERE_RADIUS_MAX				256.0f
#define SPHERE_MANA_PER_CHARGE			1
#define SPHERE_MAX_MANA_CHARGE			7		// For a total of 20 mana in a shot.
#define SPHERE_MAX_CHARGES				(SPHERE_MAX_MANA_CHARGE * 2)	// 2 frames per charge.
#define SPHERE_FLY_SPEED				600.0f
#define SPHERE_LASER_SPEED				320.0f

// Ripper (Iron Doom).
#define RIPPER_DAMAGE_MIN				40
#define RIPPER_DAMAGE_MAX				50
#define RIPPER_BALLS					8
#define RIPPER_BALL_ANGLE				(ANGLE_360 / RIPPER_BALLS)
#define RIPPER_EXPLODE_DAMAGE			20
#define RIPPER_EXPLODE_SPEED			500.0f

// Mace balls (powered up Iron Doom).
#define MACEBALL_GRAVITY				0.5f
#define MACEBALL_LIFE					5.0f
#define MACEBALL_EXTRALIFE				5
#define MACEBALL_DAMAGE					1000
#define MACEBALL_BOSS_DAMAGE			100

// Tornado.
#define TORN_DUR						0.5f //TODO: rename to TORNADO_DURATION
#define	SPIN_DUR						15.0f //TODO: rename to TORNADO_SPIN_DURATION
#define TORNADO_DAMAGE					20

#define FIRE_LINGER_DMG_MIN				2
#define FIRE_LINGER_DMG_MAX				3

#pragma endregion

#pragma region ========================== DEFENSIVE SPELL STATS ==========================

// Delay before you can use a defensive spell again.
#define DEFENSE_DEBOUNCE		0.5f

// Blue Ring of repulsion.
#define	RING_EFFECT_RADIUS		200.0f
#define RING_MASS_FACTOR		200.0f
#define RING_KNOCKBACK_BASE		150
#define RING_KNOCKBACK_SCALE	200

// Meteor Barrier.
#define METEOR_DAMAGE_MIN		35
#define METEOR_DAMAGE_MAX		40
#define METEOR_SEARCH_CHANCE	12
#define METEOR_SEARCH_RADIUS	800.0f

// Morph ovum.
#define NUM_OF_OVUMS			6
#define MORPH_DURATION			30	// How long players / bad guys remain chickens - in seconds.

// Lightning Shield.
#define SHIELD_DAMAGE_MIN		15
#define SHIELD_DAMAGE_MAX		20
#define SHIELD_DURATION			23
#define SHIELD_DAMAGE_RADIUS	96.0f
#define SHIELD_LIGHTNING_WIDTH	8
#define SHIELD_ATTACK_CHANCE	3	// This is the chance of searching at all, and the chance of being zapped if found.

#pragma endregion

#pragma region ========================== OTHER EFFECT STATS ==========================

// Item stuff.
#define BARREL_EXPLODE_DMG_MIN	10
#define BARREL_EXPLODE_DMG_MAX	100
#define BARREL_EXPLODE_RADIUS	128.0f

// Screen shake defines.
#define SHAKE_LATERAL	1
#define SHAKE_VERTICAL	2
#define SHAKE_DEPTH		4
#define SHAKE_ALL_DIR	7

// Distance defines.
#define RUN_MULT		1.65f

#define FAR_CLIP_DIST		"4096.0"
#define FAR_CLIP_DIST_VAL	4096.0f
#define NEAR_CLIP_DIST		"64.0"

#define DETAIL_LOW		0
#define DETAIL_NORMAL	1
#define DETAIL_HIGH		2
#define DETAIL_UBERHIGH	3

#define DETAIL_DEFAULT	"3.0" //mxd. 2.0 in original logic.

#define VIOLENCE_NONE	0
#define VIOLENCE_BLOOD	1
#define VIOLENCE_NORMAL	2

#define VIOLENCE_DEFAULT		VIOLENCE_NORMAL
#define VIOLENCE_DEFAULT_STR	"2"

// Breath holding durations (in seconds).
#define HOLD_BREATH_TIME	20.0f
#define M_HOLD_BREATH_TIME	12.0f

#pragma endregion