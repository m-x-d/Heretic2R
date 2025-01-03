//
// Client Effects.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Client Entities.h"
#include "FX.h"

#pragma region ========================== CLIENT EFFECT SPAWNERS ==========================

// NB. The assassin tport go is not precached.
ClientEffect_t clientEffectSpawners[NUM_FX] =
{
	{ RemoveEffects,					NULL,					"s"			},
	{ FXflametest,						NULL,					NULL		},
	{ GenericExplosion1,				NULL,					NULL		},
	{ GenericExplosion2,				NULL,					NULL		},
	{ WaterSplash,						PreCacheWaterSplash,	"b"			},
	{ GenericGibTrail,					NULL,					NULL		},
	{ FXBlood,							PreCacheSplat,			"ub"		},
	{ FXBloodTrail,						PreCacheSplat,			"d"			},
	{ FXLinkedBlood,					NULL,					"bb"		},
	{ FXGenericSparks,					PreCacheSparks,			"d"			},
	{ PlayerTeleportin,					PreCacheTeleport,		NULL		},
	{ FXHealthPickup,					PreCacheHealth,			NULL		},
	{ FXWeaponPickup,					PreCacheItemWeapons,	"b"			},
	{ FXDefensePickup,					PreCacheItemDefense,	"b"			},
	{ FXPuzzlePickup,					PreCachePuzzleItems,	"bv"		},
	{ FXAmmoPickup,						PreCacheItemAmmo,		"b"			},
	{ FXFlyingFist,						PreCacheFist,			"t"			},
	{ FXFlyingFistExplode,				NULL,					"d"			},
	{ FXBlueRing,						PreCacheBluering,		NULL		},
	{ FXMeteorBarrier,					PreCacheMeteor,			NULL		},	// See fx.h for an explanation of this.
	{ FXMeteorBarrier,					PreCacheMeteor,			NULL		},
	{ FXMeteorBarrier,					PreCacheMeteor,			NULL		},
	{ FXMeteorBarrier,					PreCacheMeteor,			NULL		},
	{ FXMeteorBarrierTravel,			PreCacheMeteor,			NULL		},
	{ FXMeteorBarrierExplode,			NULL,					"d"			},
	{ FXLightningShield,				PreCacheShield,			NULL		},
	{ FXSphereOfAnnihilation,			PreCacheSphere,			"s"			},
	{ FXSphereOfAnnihilationGlowballs,	NULL,					"s"			},
	{ FXSphereOfAnnihilationExplode,	NULL,					"db"		},
	{ FXSphereOfAnnihilationPower,		NULL,					"xbb"		},
	{ FXSpherePlayerExplode,			NULL,					"db"		},
	{ FXMagicMissile,					PreCacheArray,			"ss"		},
	{ FXMagicMissileExplode,			NULL,					"d"			},
	{ FXBlast,							NULL,					"sssssss"	},
	{ FXRedRainMissile,					PreCacheRedrain,		"t"			},
	{ FXRedRain,						NULL,					NULL		},
	{ FXRedRainGlow,					NULL,					"b"			},
	{ FXMaceball,						PreCacheMaceball,		NULL		},
	{ FXMaceballBounce,					NULL,					"d"			},
	{ FXMaceballExplode,				NULL,					"d"			},
	{ FXPhoenixMissile,					PreCachePhoenix,		"t"			},
	{ FXPhoenixExplode,					NULL,					"td"		},
	{ FXMorphMissile,					PreCacheMorph,			"bb"		},
	{ FXMorphMissile_initial,			NULL,					"bssssss"	},
	{ FXMorphExplode,					NULL,					"d"			},
	{ FXFireWave,						PreCacheWall,			"ss"		},
	{ FXFireWaveWorm,					PreCacheWall,			"t"			},
	{ FXFireBurst,						NULL,					"ss"		},
	{ FXRipperExplode,					NULL,					"vbssssssss"},
	{ FXWaterEntrySplash,				NULL,					"bd"		},
	{ FXWaterRipples,					PreCacheRipples,		NULL		},
	{ FXWaterWake,						PreCacheWake,			"sbv"		},
	{ FXBubbler,						PreCacheBubbler,		"b"			},
	{ FXScorchmark,						PreCacheScorch,			"d"			},
	{ FXDebris,							PreCacheDebris,			"bbdb"		},
	{ FXFleshDebris,					PreCacheDebris,			"bdb"		},
	{ FXShadow,							PrecacheShadow,			"f"			},
	{ FXAnimate,						PreCacheFXAnimate,		"bbbv"		},
	{ FXFountain,						NULL,					"vsb"		},
	{ FXWaterfallBase,					NULL,					"bbb"		},
	{ FXDripper,						PreCacheDripper,		"bb"		},
	{ FXMist,							PreCacheMist,			"b"			},
	{ FXPlagueMist,						NULL,					"vb"		},
	{ FXPlagueMistExplode,				NULL,					"b"			},
	{ FXSpellHands,						PreCacheHands,			"b"			},
	{ FXLensFlare,						PreCacheFlare,			"bbbf"		},
	{ FXStaff,							PreCacheStaff,			"bb"		},
	{ FXSpoo,							PreCacheSpoo,			NULL		},
	{ FXHalo,							PreCacheHalos,			NULL		},
	{ FXRemoteCamera,					NULL,					"s"			},
	{ FXHellbolt,						PreCacheHellstaff,		"t"			},
	{ FXHellboltExplode,				NULL,					"d"			},
	{ FXHellstaffPower,					PreCacheHellstaff,		"tb"		},
	{ FXHellstaffPowerBurn,				NULL,					"t"			},
	{ FXSpellChange,					NULL,					"db"		},
	{ FXStaffCreate,					NULL,					NULL		},
	{ FXStaffCreatePoof,				NULL,					NULL		},
	{ FXStaffRemove,					NULL,					NULL		},
	{ FXDustPuffOnGround,				NULL,					NULL		},
	{ FXFire,							NULL,					"b"			},
	{ FXSound,							NULL,					"bbbb"		},
	{ FXPickup,							PreCachePickup,			NULL		},
	{ FXGenericHitPuff,					NULL,					"db"		},
	{ FXDust,							PreCacheRockchunks,		"bdb"		},
	{ FXEnvSmoke,						PreCacheSmoke,			"bdbbb"		},
	{ FXSpooSplat,						NULL,					"d"			},
	{ FXBodyPart,						NULL,					"ssbbb"		},
	{ PlayerTeleportout,				NULL,					NULL		},
	{ FXPlayerPersistant,				NULL,					NULL		},
	{ FXplayertorch,					PreCacheTorch,			NULL		},
	{ FXTomeOfPower,					NULL,					NULL		},
	{ FXFireOnEntity,					NULL,					"bbb"		},
	{ FXFlareup,						PreCacheFlareup,		NULL		},
	{ FXShrinePlayerEffect,				PreCacheShrine,			"b"			},
	{ FXShrineManaEffect,				NULL,					NULL		},
	{ FXShrineLungsEffect,				NULL,					NULL		},
	{ FXShrineLightEffect,				NULL,					NULL		},
	{ FXShrineReflectEffect,			NULL,					NULL		},
	{ FXShrineArmorEffect,				NULL,					NULL		},
	{ FXShrineHealthEffect,				NULL,					NULL		},
	{ FXShrineStaffEffect,				NULL,					NULL		},
	{ FXShrineGhostEffect,				NULL,					NULL		},
	{ FXShrineSpeedEffect,				NULL,					NULL		},
	{ FXShrinePowerUpEffect,			NULL,					NULL		},
	{ FXRope,							PreCacheRope,			"ssbvvv"	},
	{ FXFireHands,						NULL,					"b"			},
	{ FXShrineBall,						NULL,					"db"		},
	{ FXShrineBallExplode,				NULL,					"db"		},
	{ FXOgleHitPuff,					PrecacheOgleHitPuff,	"v"			},
	{ FXHPMissile,						PreCacheHPMissile,		"vb"		},
	{ FXIEffects,						PreCacheIEffects,		"bv"		},
	{ FXChickenExplode,					NULL,					NULL		},
	{ FXFlamethrower,					NULL,					"df"		},
	{ FXTeleportPad,					NULL,					NULL		},	// 110 fx to here
	{ FXQuake,							NULL,					"bbb"		},	// FX_QUAKE 
	{ FXLightning,						PreCacheLightning,		"vbb"		},
	{ FXPowerLightning,					PreCacheLightning,		"vb"		},
	{ FXBubble,							PreCacheBubbler,		NULL		},
	{ FXTPortSmoke,						PreCacheTPortSmoke,		NULL		},	// FX_TPORTSMOKE - 117
	{ FXWaterParticles,					PreCacheWaterParticles,	NULL		},	// FX_WATER_PARTICLES - 119
	{ FXMEffects,						PreCacheMEffects,		"bv"		},	// FX_M_EFFECTS - 120 - all of Morcalavin's effects.
	{ FXHPStaff,						PreCacheHPStaff,		"bs"		},	// FX_HP_STAFF - 121 - staff effects for the high priestess.
	{ FXRandWaterBubble,				NULL,					NULL		},
	{ FXMagicPortal,					PreCachePortal,			"vbb"		},
	{ FXTBEffects,						PreCacheTB,				"bv"		},	// FX_TB_EFFECTS - 124
	{ FXTestBBox,						NULL,					"fff"		},
	{ FXBodyPart,						NULL,					"ssbbb"		},	// FX_THROWWEAPON - 126 - uses body part, which just detects type for certain things.
	{ FXSsithraArrow,					PrecacheSsithraArrow,	"bv"		},
	{ FXPESpell,						PrecachePESpell,		"bv"		},
	{ FXLightningHit,					PreCacheHitPuff,		"t"			},	// FX_LIGHTNING_HIT
	{ NullEffect,						NULL,					NULL		},	// FX_FOOTSTEP,    // Unimplemented fx
	{ NullEffect,						NULL,					NULL		},	// FX_FALLSHORT,   // Unimplemented fx
	{ FXStaffStrike,					PreCacheStaffHit,		"db"		},
	{ FXCreateArmorHit,					PreCacheArmorHit,		"d"			},
	{ FXBarrelExplode,					PreCacheObjects,		NULL,		},
	{ FXCWatcherEffects,				PreCacheCWModels,		"bv"		},
	{ FXCorpseRemove,					PreCacheCrosshair,		NULL		},	// Naughty little hack here, crosshair has nothing to do with corpse removal.
	{ FXLeader,							NULL,					NULL		},
	{ FXTornado,						PreCacheTorn,			NULL		},
	{ FXTornadoBall,					NULL,					NULL		},
	{ FXTornadoBallExplode,				NULL,					NULL		},
	{ FXFeetTrail,						NULL,					NULL		},
	{ FXGenericSparks,					PreCacheSparks,			"d"			},
	{ NULL,								NULL,					NULL		},	// FX_CROSSHAIR
};

#pragma endregion

CE_ClassStatics_t ce_class_statics[NUM_CLASSIDS];

void (*ce_class_statics_inits[NUM_CLASSIDS])(void) =
{
	InitDebrisStatics
};

void NullEffect(centity_t* owner, int type, int flags, vec3_t origin) {}

void RemoveEffects(centity_t* owner, int type, const int flags, vec3_t origin)
{
	assert(owner);
	assert(owner->effects); // FIXME: This assert fires, but it should not. We shouldn't be here anyway.

	short fx;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_REMOVE_EFFECTS].formatString, &fx);
	RemoveEffectTypeList(&owner->effects, fx, owner);
}

// ***************************************************************************************
// Client Sounds
// ***************************************************************************************

void RegisterSounds(void)
{
}

// ***************************************************************************************
// Client Models
// ***************************************************************************************

void RegisterModels(void)
{
	int		i;

	for(i = 0; i < NUM_FX; i++)
	{
		if(clientEffectSpawners[i].PrecacheCFX)
		{
			clientEffectSpawners[i].PrecacheCFX();
		}
	}
}