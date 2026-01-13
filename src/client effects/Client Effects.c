//
// Client Effects.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"

#pragma region ========================== CLIENT EFFECT SPAWNERS ==========================

// NB. The assassin tport go is not precached.
ClientEffect_t clientEffectSpawners[NUM_FX] =
{
	{ // FX_REMOVE_EFFECTS
		.SpawnCFX = RemoveEffects,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = "s"
	},

	{ // FX_TEST
		.SpawnCFX = FXflametest,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = NULL
	},

	{ // FX_EXPLOSION1
		.SpawnCFX = FXExplosion1,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = NULL
	},

	{ // FX_EXPLOSION2
		.SpawnCFX = FXExplosion2,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = NULL
	},

	{ // FX_SPLASH
		.SpawnCFX = FXWaterSplash,
		.PrecacheCFX = PreCacheWaterSplash,
		.PrecacheSFX = NULL,
		.formatString = "b"
	},

	{ // FX_GIB_TRAIL
		.SpawnCFX = FXGibTrail,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = NULL
	},

	{ // FX_BLOOD
		.SpawnCFX = FXBlood,
		.PrecacheCFX = PreCacheSplat,
		.PrecacheSFX = NULL,
		.formatString = "ub"
	},

	{ // FX_BLOOD_TRAIL
		.SpawnCFX = FXBloodTrail,
		.PrecacheCFX = PreCacheSplat,
		.PrecacheSFX = PreCacheSplatSFX, //mxd
		.formatString = "d"
	},

	{ // FX_LINKEDBLOOD
		.SpawnCFX = FXLinkedBlood,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = "bb"
	},

	{ // FX_SPARKS
		.SpawnCFX = FXGenericSparks,
		.PrecacheCFX = PreCacheSparks,
		.PrecacheSFX = PreCacheSparksSFX, //mxd
		.formatString = "d"
	},

	{ // FX_PLAYER_TELEPORT_IN
		.SpawnCFX = FXPlayerTeleportIn,
		.PrecacheCFX = PreCacheTeleport,
		.PrecacheSFX = NULL,
		.formatString = NULL
	},

	{ // FX_PICKUP_HEALTH
		.SpawnCFX = FXHealthPickup,
		.PrecacheCFX = PreCacheHealth,
		.PrecacheSFX = NULL,
		.formatString = NULL
	},

	{ // FX_PICKUP_WEAPON
		.SpawnCFX = FXWeaponPickup,
		.PrecacheCFX = PreCacheItemWeapons,
		.PrecacheSFX = NULL,
		.formatString = "b"
	},

	{ // FX_PICKUP_DEFENSE
		.SpawnCFX = FXDefensePickup,
		.PrecacheCFX = PreCacheItemDefense,
		.PrecacheSFX = NULL,
		.formatString = "b"
	},

	{ // FX_PICKUP_PUZZLE
		.SpawnCFX = FXPuzzlePickup,
		.PrecacheCFX = PreCachePuzzleItems,
		.PrecacheSFX = NULL,
		.formatString = "bv"
	},

	{ // FX_PICKUP_AMMO
		.SpawnCFX = FXAmmoPickup,
		.PrecacheCFX = PreCacheItemAmmo,
		.PrecacheSFX = NULL,
		.formatString = "b"
	},

	{ // FX_WEAPON_FLYINGFIST
		.SpawnCFX = FXFlyingFist,
		.PrecacheCFX = PreCacheFist,
		.PrecacheSFX = PreCacheFistSFX, //mxd
		.formatString = "t"
	},

	{ // FX_WEAPON_FLYINGFISTEXPLODE
		.SpawnCFX = FXFlyingFistExplode,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = "d"
	},

	{ // FX_SPELL_BLUERING
		.SpawnCFX = FXBlueRing,
		.PrecacheCFX = PreCacheBluering,
		.PrecacheSFX = PreCacheBlueringSFX, //mxd
		.formatString = NULL
	},

	{ // FX_SPELL_METEORBARRIER
		.SpawnCFX = FXMeteorBarrier,
		.PrecacheCFX = PreCacheMeteor,
		.PrecacheSFX = NULL,
		.formatString = NULL
	},	// See fx.h for an explanation of this.

	{ // FX_SPELL_METEORBARRIER1
		.SpawnCFX = FXMeteorBarrier,
		.PrecacheCFX = PreCacheMeteor,
		.PrecacheSFX = NULL,
		.formatString = NULL
	},

	{ // FX_SPELL_METEORBARRIER2
		.SpawnCFX = FXMeteorBarrier,
		.PrecacheCFX = PreCacheMeteor,
		.PrecacheSFX = NULL,
		.formatString = NULL
	},

	{ // FX_SPELL_METEORBARRIER3
		.SpawnCFX = FXMeteorBarrier,
		.PrecacheCFX = PreCacheMeteor,
		.PrecacheSFX = NULL,
		.formatString = NULL
	},

	{ // FX_SPELL_METEORBARRIER_TRAVEL
		.SpawnCFX = FXMeteorBarrierTravel,
		.PrecacheCFX = PreCacheMeteor,
		.PrecacheSFX = NULL,
		.formatString = NULL
	},

	{ // FX_SPELL_METEORBARRIEREXPLODE
		.SpawnCFX = FXMeteorBarrierExplode,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = "d"
	},

	{ // FX_SPELL_LIGHTNINGSHIELD
		.SpawnCFX = FXLightningShield,
		.PrecacheCFX = PreCacheShield,
		.PrecacheSFX = NULL,
		.formatString = NULL
	},

	{ // FX_WEAPON_SPHERE
		.SpawnCFX = FXSphereOfAnnihilation,
		.PrecacheCFX = PreCacheSphere,
		.PrecacheSFX = NULL,
		.formatString = "s"
	},

	{ // FX_WEAPON_SPHEREGLOWBALLS
		.SpawnCFX = FXSphereOfAnnihilationGlowballs,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = "s"
	},

	{ // FX_WEAPON_SPHEREEXPLODE
		.SpawnCFX = FXSphereOfAnnihilationExplode,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = "db"
	},

	{ // FX_WEAPON_SPHEREPOWER
		.SpawnCFX = FXSphereOfAnnihilationPower,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = "xbb"
	},

	{ // FX_WEAPON_SPHEREPLAYEREXPLODE
		.SpawnCFX = FXSpherePlayerExplode,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = "db"
	},

	{ // FX_WEAPON_MAGICMISSILE
		.SpawnCFX = FXMagicMissile,
		.PrecacheCFX = PreCacheArray,
		.PrecacheSFX = NULL,
		.formatString = "ss"
	},

	{ // FX_WEAPON_MAGICMISSILEEXPLODE
		.SpawnCFX = FXMagicMissileExplode,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = "d"
	},

	{ // FX_WEAPON_BLAST
		.SpawnCFX = FXBlast,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = "sssssss"
	},

	{ // FX_WEAPON_REDRAINMISSILE
		.SpawnCFX = FXRedRainMissile,
		.PrecacheCFX = PreCacheRedrain,
		.PrecacheSFX = NULL,
		.formatString = "t"
	},

	{ // FX_WEAPON_REDRAIN
		.SpawnCFX = FXRedRain,
		.PrecacheCFX = NULL,
		.PrecacheSFX = PreCacheRedrainSFX, //mxd
		.formatString = NULL
	},

	{ // FX_WEAPON_REDRAINGLOW
		.SpawnCFX = FXRedRainGlow,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = "b"
	},

	{ // FX_WEAPON_MACEBALL
		.SpawnCFX = FXMaceball,
		.PrecacheCFX = PreCacheMaceball,
		.PrecacheSFX = NULL,
		.formatString = NULL
	},

	{ // FX_WEAPON_MACEBALLBOUNCE
		.SpawnCFX = FXMaceballBounce,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = "d"
	},

	{ // FX_WEAPON_MACEBALLEXPLODE
		.SpawnCFX = FXMaceballExplode,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = "d"
	},

	{ // FX_WEAPON_PHOENIXMISSILE
		.SpawnCFX = FXPhoenixMissile,
		.PrecacheCFX = PreCachePhoenix,
		.PrecacheSFX = NULL,
		.formatString = "t"
	},

	{ // FX_WEAPON_PHOENIXEXPLODE
		.SpawnCFX = FXPhoenixExplode,
		.PrecacheCFX = NULL,
		.PrecacheSFX = PreCachePhoenixExplodeSFX, //mxd
		.formatString = "td"
	},

	{ // FX_SPELL_MORPHMISSILE
		.SpawnCFX = FXMorphMissile,
		.PrecacheCFX = PreCacheMorph,
		.PrecacheSFX = PreCacheMorphSFX, //mxd
		.formatString = "bb"
	},

	{ // FX_SPELL_MORPHMISSILE_INITIAL
		.SpawnCFX = FXMorphMissileInitial,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = "bssssss"
	},

	{ // FX_SPELL_MORPHEXPLODE
		.SpawnCFX = FXMorphExplode,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = "d"
	},

	{ // FX_WEAPON_FIREWAVE
		.SpawnCFX = FXFireWave,
		.PrecacheCFX = PreCacheWall,
		.PrecacheSFX = NULL,
		.formatString = "ss"
	},

	{ // FX_WEAPON_FIREWAVEWORM
		.SpawnCFX = FXFireWaveWorm,
		.PrecacheCFX = PreCacheWall,
		.PrecacheSFX = NULL,
		.formatString = "t"
	},

	{ // FX_WEAPON_FIREBURST
		.SpawnCFX = FXFireBurst,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = "ss"
	},

	{ // FX_WEAPON_RIPPEREXPLODE
		.SpawnCFX = FXRipperExplode,
		.PrecacheCFX = NULL,
		.PrecacheSFX = PreCacheRipperSFX, //mxd
		.formatString = "vbssssssss"
	},

	{ // FX_WATER_ENTRYSPLASH
		.SpawnCFX = FXWaterEntrySplash,
		.PrecacheCFX = PreCacheWaterSplash, //mxd
		.PrecacheSFX = PreCacheWaterSplashSFX, //mxd
		.formatString = "bd"
	},

	{ // FX_WATER_RIPPLES
		.SpawnCFX = FXWaterRipples,
		.PrecacheCFX = PreCacheRipples,
		.PrecacheSFX = NULL,
		.formatString = NULL
	},

	{ // FX_WATER_WAKE
		.SpawnCFX = FXWaterWake,
		.PrecacheCFX = PreCacheWake,
		.PrecacheSFX = NULL,
		.formatString = "sbv"
	},

	{ // FX_BUBBLER
		.SpawnCFX = FXBubbler,
		.PrecacheCFX = PreCacheBubbler,
		.PrecacheSFX = NULL,
		.formatString = "b"
	},

	{ // FX_SCORCHMARK
		.SpawnCFX = FXScorchmark,
		.PrecacheCFX = PreCacheScorch,
		.PrecacheSFX = NULL,
		.formatString = "d"
	},

	{ // FX_DEBRIS
		.SpawnCFX = FXDebris,
		.PrecacheCFX = PreCacheDebris,
		.PrecacheSFX = PreCacheDebrisSFX, //mxd
		.formatString = "bbdb"
	},

	{ // FX_FLESH_DEBRIS
		.SpawnCFX = FXFleshDebris,
		.PrecacheCFX = PreCacheDebris,
		.PrecacheSFX = PreCacheDebrisSFX, //mxd
		.formatString = "bdb"
	},

	{ // FX_SHADOW
		.SpawnCFX = FXShadow,
		.PrecacheCFX = PreCacheShadow,
		.PrecacheSFX = NULL,
		.formatString = "f"
	},

	{ // FX_ANIMATE
		.SpawnCFX = FXAnimate,
		.PrecacheCFX = PreCacheFXAnimate,
		.PrecacheSFX = NULL,
		.formatString = "bbbv"
	},

	{ // FX_FOUNTAIN
		.SpawnCFX = FXFountain,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = "vsb"
	},

	{ // FX_WATERFALLBASE
		.SpawnCFX = FXWaterfallBase,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = "bbb"
	},

	{ // FX_DRIPPER
		.SpawnCFX = FXDripper,
		.PrecacheCFX = PreCacheDripper,
		.PrecacheSFX = PreCacheDripperSFX, //mxd
		.formatString = "bb"
	},

	{ // FX_MIST
		.SpawnCFX = FXMist,
		.PrecacheCFX = PreCacheMist,
		.PrecacheSFX = NULL,
		.formatString = "b"
	},

	{ // FX_PLAGUEMIST
		.SpawnCFX = FXPlagueMist,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = "vb"
	},

	{ // FX_PLAGUEMISTEXPLODE
		.SpawnCFX = FXPlagueMistExplode,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = "b"
	},

	{ // FX_SPELLHANDS
		.SpawnCFX = FXSpellHands,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = "b"
	},

	{ // FX_LENSFLARE
		.SpawnCFX = FXLensFlare,
		.PrecacheCFX = PreCacheFlare,
		.PrecacheSFX = NULL,
		.formatString = "bbbf"
	},

	{ // FX_STAFF
		.SpawnCFX = FXStaff,
		.PrecacheCFX = PreCacheStaff,
		.PrecacheSFX = NULL,
		.formatString = "bb"
	},

	{ // FX_SPOO
		.SpawnCFX = FXSpoo,
		.PrecacheCFX = PreCacheSpoo,
		.PrecacheSFX = NULL,
		.formatString = NULL
	},

	{ // FX_HALO
		.SpawnCFX = FXHalo,
		.PrecacheCFX = PreCacheHalos,
		.PrecacheSFX = NULL,
		.formatString = NULL
	},

	{ // FX_REMOTE_CAMERA
		.SpawnCFX = FXRemoteCamera,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = "s"
	},

	{ // FX_WEAPON_HELLBOLT
		.SpawnCFX = FXHellbolt,
		.PrecacheCFX = PreCacheHellstaff,
		.PrecacheSFX = PreCacheHellstaffSFX, //mxd
		.formatString = "t"
	},

	{ // FX_WEAPON_HELLBOLTEXPLODE
		.SpawnCFX = FXHellboltExplode,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = "d"
	},

	{ // FX_WEAPON_HELLSTAFF_POWER
		.SpawnCFX = FXHellstaffPower,
		.PrecacheCFX = PreCacheHellstaff,
		.PrecacheSFX = NULL,
		.formatString = "tb"
	},

	{ // FX_WEAPON_HELLSTAFF_POWER_BURN
		.SpawnCFX = FXHellstaffPowerBurn,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = "t"
	},

	{ // FX_SPELL_CHANGE
		.SpawnCFX = FXSpellChange,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = "db"
	},

	{ // FX_STAFF_CREATE
		.SpawnCFX = FXStaffCreate,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = NULL
	},

	{ // FX_STAFF_CREATEPOOF
		.SpawnCFX = FXStaffCreatePoof,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = NULL
	},

	{ // FX_STAFF_REMOVE
		.SpawnCFX = FXStaffRemove,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = NULL
	},

	{ // FX_DUST_PUFF
		.SpawnCFX = FXDustPuffOnGround,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = NULL
	},

	{ // FX_FIRE
		.SpawnCFX = FXFire,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = "b"
	},

	{ // FX_SOUND
		.SpawnCFX = FXSound,
		.PrecacheCFX = NULL,
		.PrecacheSFX = PreCacheFXSoundSFX, //mxd
		.formatString = "bbbb"
	},

	{ // FX_PICKUP
		.SpawnCFX = FXPickup,
		.PrecacheCFX = PreCachePickup,
		.PrecacheSFX = NULL,
		.formatString = NULL
	},

	{ // FX_HITPUFF
		.SpawnCFX = FXHitPuff,
		.PrecacheCFX = PreCacheHitPuff, //mxd
		.PrecacheSFX = NULL,
		.formatString = "db"
	},

	{ // FX_DUST
		.SpawnCFX = FXDust,
		.PrecacheCFX = PreCacheRockchunks,
		.PrecacheSFX = NULL,
		.formatString = "bdb"
	},

	{ // FX_ENVSMOKE
		.SpawnCFX = FXEnvSmoke,
		.PrecacheCFX = PreCacheSmoke,
		.PrecacheSFX = PreCacheSmokeSFX, //mxd
		.formatString = "bdbbb"
	},

	{ // FX_SPOO_SPLAT
		.SpawnCFX = FXSpooSplat,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = "d"
	},

	{ // FX_BODYPART
		.SpawnCFX = FXBodyPart,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = "ssbbb"
	},

	{ // FX_PLAYER_TELEPORT_OUT
		.SpawnCFX = FXPlayerTeleportOut,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = NULL
	},

	{ // FX_PLAYER_PERSISTANT
		.SpawnCFX = FXPlayerPersistant,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = NULL
	},

	{ // FX_PLAYER_TORCH
		.SpawnCFX = FXPlayerTorch,
		.PrecacheCFX = PreCacheTorch,
		.PrecacheSFX = NULL,
		.formatString = NULL
	},

	{ // FX_TOME_OF_POWER
		.SpawnCFX = FXTomeOfPower,
		.PrecacheCFX = PreCacheTome, //mxd. Added separate precache function.
		.PrecacheSFX = PreCacheTomeSFX, //mxd
		.formatString = NULL
	}, 

	{ // FX_FIRE_ON_ENTITY
		.SpawnCFX = FXFireOnEntity,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = "bbb"
	},

	{ // FX_FLAREUP
		.SpawnCFX = FXFlareup,
		.PrecacheCFX = PreCacheFlareup,
		.PrecacheSFX = NULL,
		.formatString = NULL
	},

	{ // FX_SHRINE_PLAYER
		.SpawnCFX = FXShrinePlayerEffect,
		.PrecacheCFX = PreCacheShrine,
		.PrecacheSFX = NULL,
		.formatString = "b"
	},

	{ // FX_SHRINE_MANA
		.SpawnCFX = FXShrineManaEffect,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = NULL
	},

	{ // FX_SHRINE_LUNGS
		.SpawnCFX = FXShrineLungsEffect,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = NULL
	},

	{ // FX_SHRINE_LIGHT
		.SpawnCFX = FXShrineLightEffect,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = NULL
	},

	{ // FX_SHRINE_REFLECT
		.SpawnCFX = FXShrineReflectEffect,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = NULL
	},

	{ // FX_SHRINE_ARMOR
		.SpawnCFX = FXShrineArmorEffect,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = NULL
	},

	{ // FX_SHRINE_HEALTH
		.SpawnCFX = FXShrineHealthEffect,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = NULL
	},

	{ // FX_SHRINE_STAFF
		.SpawnCFX = FXShrineStaffEffect,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = NULL
	},

	{ // FX_SHRINE_GHOST
		.SpawnCFX = FXShrineGhostEffect,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = NULL
	},

	{ // FX_SHRINE_SPEED
		.SpawnCFX = FXShrineSpeedEffect,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = NULL
	},

	{ // FX_SHRINE_POWERUP
		.SpawnCFX = FXShrinePowerupEffect,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = NULL
	},

	{ // FX_ROPE
		.SpawnCFX = FXRope,
		.PrecacheCFX = PreCacheRope,
		.PrecacheSFX = NULL,
		.formatString = "ssbvvv"
	},

	{ // FX_FIREHANDS
		.SpawnCFX = FXFireHands,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = "b"
	},

	{ // FX_SHRINE_BALL
		.SpawnCFX = FXShrineBall,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = "db"
	},

	{ // FX_SHRINE_BALL_EXPLODE
		.SpawnCFX = FXShrineBallExplode,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = "db"
	},

	{ // FX_OGLE_HITPUFF
		.SpawnCFX = FXOgleHitPuff,
		.PrecacheCFX = PreCacheOgleHitPuff,
		.PrecacheSFX = NULL,
		.formatString = "v"
	},

	{ // FX_HP_MISSILE
		.SpawnCFX = FXHPMissile,
		.PrecacheCFX = PreCacheHPMissile,
		.PrecacheSFX = NULL,
		.formatString = "vb"
	},

	{ // FX_I_EFFECTS
		.SpawnCFX = FXInsectEffects,
		.PrecacheCFX = PreCacheIEffects,
		.PrecacheSFX = PreCacheInsectStaffSFX, //mxd
		.formatString = "bv"
	},

	{ // FX_CHICKEN_EXPLODE
		.SpawnCFX = FXChickenExplode,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = NULL
	},

	{ // FX_FLAMETHROWER
		.SpawnCFX = FXFlamethrower,
		.PrecacheCFX = NULL,
		.PrecacheSFX = PreCacheFlamethrowerSFX, //mxd
		.formatString = "df"
	},

	{ // FX_TELEPORT_PAD
		.SpawnCFX = FXTeleportPad,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = NULL
	},

	{ // FX_QUAKE 
		.SpawnCFX = FXQuake,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = "bbb"
	},

	{ // FX_LIGHTNING
		.SpawnCFX = FXLightning,
		.PrecacheCFX = PreCacheLightning,
		.PrecacheSFX = NULL,
		.formatString = "vbb"
	},

	{ // FX_POWER_LIGHTNING
		.SpawnCFX = FXPowerLightning,
		.PrecacheCFX = PreCacheLightning,
		.PrecacheSFX = PreCachePowerLightningSFX, //mxd
		.formatString = "vb"
	},

	{ // FX_BUBBLE
		.SpawnCFX = FXBubble,
		.PrecacheCFX = PreCacheBubbler,
		.PrecacheSFX = PreCacheBubblerSFX,
		.formatString = NULL
	},

	{ // FX_TPORTSMOKE
		.SpawnCFX = FXTPortSmoke,
		.PrecacheCFX = NULL,
		.PrecacheSFX = PreCacheTPortSmokeSFX, //mxd
		.formatString = NULL
	},

	{ // FX_WATER_PARTICLES
		.SpawnCFX = FXWaterParticles,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = NULL
	},

	{ // FX_M_EFFECTS - all of Morcalavin's effects.
		.SpawnCFX = FXMEffects,
		.PrecacheCFX = PreCacheMEffects,
		.PrecacheSFX = NULL,
		.formatString = "bv"
	},

	{ // FX_HP_STAFF - staff effects for the high priestess.
		.SpawnCFX = FXHPStaff,
		.PrecacheCFX = PreCacheHPStaff,
		.PrecacheSFX = NULL,
		.formatString = "bs"
	},

	{ // FX_WATER_BUBBLE
		.SpawnCFX = FXRandWaterBubble,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = NULL
	},

	{ // FX_MAGIC_PORTAL
		.SpawnCFX = FXMagicPortal,
		.PrecacheCFX = PreCachePortal,
		.PrecacheSFX = NULL,
		.formatString = "vbb"
	},

	{ // FX_TB_EFFECTS
		.SpawnCFX = FXTBEffects,
		.PrecacheCFX = PreCacheTB,
		.PrecacheSFX = NULL,
		.formatString = "bv"
	},

	{ // FX_TEST_BBOX
		.SpawnCFX = FXTestBBox,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = "fff"
	},

	{ // FX_THROWWEAPON - uses body part, which just detects type for certain things.
		.SpawnCFX = FXBodyPart,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = "ssbbb"
	},

	{ // FX_SSITHRA_ARROW
		.SpawnCFX = FXSsithraArrow,
		.PrecacheCFX = PreCacheSsithraArrow,
		.PrecacheSFX = NULL,
		.formatString = "bv"
	},

	{ // FX_PE_SPELL
		.SpawnCFX = FXPESpell,
		.PrecacheCFX = PreCachePESpell,
		.PrecacheSFX = PreCachePESpellSFX, //mxd
		.formatString = "bv"
	},

	{ // FX_LIGHTNING_HIT
		.SpawnCFX = FXLightningHit,
		.PrecacheCFX = PreCacheLightningHit,
		.PrecacheSFX = PreCacheLightningHitSFX, //mxd
		.formatString = "t"
	},

	{ // FX_FOOTSTEP    // Unimplemented fx.
		.SpawnCFX = NullEffect,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = NULL
	},

	{ // FX_FALLSHORT	// Unimplemented fx.
		.SpawnCFX = NullEffect,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = NULL
	},

	{ // FX_WEAPON_STAFF_STRIKE
		.SpawnCFX = FXStaffStrike,
		.PrecacheCFX = PreCacheStaffHit,
		.PrecacheSFX = NULL,
		.formatString = "db"
	},

	{ // FX_ARMOR_HIT
		.SpawnCFX = FXArmorHit,
		.PrecacheCFX = PreCacheArmorHit,
		.PrecacheSFX = NULL,
		.formatString = "d"
	},

	{ // FX_BARREL_EXPLODE
		.SpawnCFX = FXBarrelExplode,
		.PrecacheCFX = PreCacheBarrelExplode,
		.PrecacheSFX = PreCacheBarrelExplodeSFX, //mxd
		.formatString = NULL,
	},

	{ // FX_CWATCHER
		.SpawnCFX = FXCWatcherEffects,
		.PrecacheCFX = PreCacheCWModels,
		.PrecacheSFX = PreCacheCWSFX, //mxd
		.formatString = "bv"
	},

	{ // FX_CORPSE_REMOVE
		.SpawnCFX = FXCorpseRemove,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = NULL
	},

	{ // FX_SHOW_LEADER
		.SpawnCFX = FXLeader,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = NULL
	},

	{ // FX_TORNADO
		.SpawnCFX = FXTornado,
		.PrecacheCFX = PreCacheTornado,
		.PrecacheSFX = NULL,
		.formatString = NULL
	},

	{ // FX_TORNADO_BALL
		.SpawnCFX = FXTornadoBall,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = NULL
	},

	{ // FX_TORNADO_BALL_EXPLODE
		.SpawnCFX = FXTornadoBallExplode,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = NULL
	},

	{ // FX_FOOT_TRAIL
		.SpawnCFX = FXFeetTrail,
		.PrecacheCFX = NULL,
		.PrecacheSFX = NULL,
		.formatString = NULL
	},

	{ // FX_BLOCK_SPARKS
		.SpawnCFX = FXGenericSparks,
		.PrecacheCFX = PreCacheSparks,
		.PrecacheSFX = PreCacheSparksSFX, //mxd
		.formatString = "d"
	},

	{ // FX_CROSSHAIR
		.SpawnCFX = NULL,
		.PrecacheCFX = PreCacheCrosshair, //mxd. Assigned in FX_CORPSE_REMOVE instead in original logic (why?..).
		.PrecacheSFX = NULL,
		.formatString = NULL
	},
};

#pragma endregion

const paletteRGBA_t color_white =	{ .c = 0xffffffff }; //mxd
const paletteRGBA_t color_red =		{ .c = 0xff0000ff }; //mxd
const paletteRGBA_t color_orange =	{ .c = 0xff2040ff }; //mxd. Actually, nearly-red as well...

CE_ClassStatics_t ce_class_statics[NUM_CLASSIDS];

void (*ce_class_statics_inits[NUM_CLASSIDS])(void) =
{
	InitDebrisStatics
};

void NullEffect(centity_t* owner, int type, int flags, vec3_t origin) {}

void RemoveEffects(centity_t* owner, int type, const int flags, vec3_t origin)
{
	assert(owner != NULL);

	// Need to parse server message even if owner has no effects attached.
	short fx;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_REMOVE_EFFECTS].formatString, &fx);

	if (owner->effects != NULL) //mxd. Original game logic calls gi.RemoveEffects() without checking if there are any effects attached.
		RemoveEffectTypeList(&owner->effects, fx);
}

void RegisterSounds(void) //mxd. Implemented sound precaching.
{
	for (int i = 0; i < NUM_FX; i++)
		if (clientEffectSpawners[i].PrecacheSFX != NULL)
			clientEffectSpawners[i].PrecacheSFX();
}

void RegisterModels(void)
{
	for (int i = 0; i < NUM_FX; i++)
		if (clientEffectSpawners[i].PrecacheCFX != NULL)
			clientEffectSpawners[i].PrecacheCFX();
}