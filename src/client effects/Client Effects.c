#include "Client Effects.h"
#include "Client Entities.h"
#include "FX.h"

// NB. The assassin tport go is not precached

ClientEffect_t clientEffectSpawners[NUM_FX] =
{	
	// ***NOTE*** We currently have 113 client effects, and we don't want to exceed 32768!  Ha!

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
	{ FXMeteorBarrier,					PreCacheMeteor,			NULL		},	// see fx.h for an explanation of this
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
	{ FXMEffects,						PreCacheMEffects,		"bv"		},	// FX_M_EFFECTS - 120 - all of Morcalavin's effects
	{ FXHPStaff,						PreCacheHPStaff,		"bs"		},	// FX_HP_STAFF - 121 - staff effects for the high priestess
	{ FXRandWaterBubble,				NULL,					NULL		},		 
	{ FXMagicPortal,					PreCachePortal,			"vbb"		},
	{ FXTBEffects,						PreCacheTB,				"bv"		},	// FX_TB_EFFECTS - 124
	{ FXTestBBox,						NULL,					"fff"		},
	{ FXBodyPart,						NULL,					"ssbbb"		},	// FX_THROWWEAPON - uses body part, which just detects type for certain things - 126!!!
	{ FXSsithraArrow,					PrecacheSsithraArrow,	"bv"		},
	{ FXPESpell,						PrecachePESpell,		"bv"		},
	{ FXLightningHit,					PreCacheHitPuff,		"t"			},	// FX_LIGHTNING_HIT
	{ NullEffect,						NULL,					NULL		},	// FX_FOOTSTEP,    //114// Unimplemented fx
	{ NullEffect,						NULL,					NULL		},	// FX_FALLSHORT,   //115// Unimplemented fx
	{ FXStaffStrike,					PreCacheStaffHit,		"db"		},
	{ FXCreateArmorHit,					PreCacheArmorHit,		"d"			},
	{ FXBarrelExplode,					PreCacheObjects,		NULL,		},
	{ FXCWatcherEffects,				PreCacheCWModels,		"bv"		},
	{ FXCorpseRemove,					PreCacheCrosshair,		NULL		},	// naughty little hack here, crosshair has nothing to do with corpse removal
	{ FXLeader,							NULL,					NULL		},
	{ FXTornado,						PreCacheTorn,			NULL		},
	{ FXTornadoBall,					NULL,					NULL		},
	{ FXTornadoBallExplode,				NULL,					NULL		},
	{ FXFeetTrail,						NULL,					NULL		},
	{ FXGenericSparks,					PreCacheSparks,			"d"			},
	{ NULL,								NULL,					NULL		},	// FX_CROSSHAIR
};	

CE_ClassStatics_t classStatics[NUM_CLASSIDS];

void (*classStaticsInits[NUM_CLASSIDS])()= 
{
	InitDebrisStatics
};

void NullEffect(centity_t *owner, int type, int flags, vec3_t origin)
{
}

void RemoveEffects(centity_t *owner, int type, int flags, vec3_t origin)
{
	short		fx;

	assert(owner);
//	assert(owner->effects);		// FIXME:  This assert fires, but it should not.  We shouldn't be here anyway.

	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_REMOVE_EFFECTS].formatString, &fx);
	RemoveEffectTypeList(&owner->effects, fx, owner);
}

// ***************************************************************************************
// Client Sounds
// ***************************************************************************************

void RegisterSounds()
{
}

// ***************************************************************************************
// Client Models
// ***************************************************************************************

void RegisterModels()
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

float	cl_turbsin[] =
{
 0, 0.19633, 0.392541, 0.588517, 0.784137, 0.979285, 1.17384, 1.3677,
 1.56072, 1.75281, 1.94384, 2.1337, 2.32228, 2.50945, 2.69512, 2.87916,
 3.06147, 3.24193, 3.42044, 3.59689, 3.77117, 3.94319, 4.11282, 4.27998,
 4.44456, 4.60647, 4.76559, 4.92185, 5.07515, 5.22538, 5.37247, 5.51632,
 5.65685, 5.79398, 5.92761, 6.05767, 6.18408, 6.30677, 6.42566, 6.54068,
 6.65176, 6.75883, 6.86183, 6.9607, 7.05537, 7.14579, 7.23191, 7.31368,
 7.39104, 7.46394, 7.53235, 7.59623, 7.65552, 7.71021, 7.76025, 7.80562,
 7.84628, 7.88222, 7.91341, 7.93984, 7.96148, 7.97832, 7.99036, 7.99759,
 8, 7.99759, 7.99036, 7.97832, 7.96148, 7.93984, 7.91341, 7.88222,
 7.84628, 7.80562, 7.76025, 7.71021, 7.65552, 7.59623, 7.53235, 7.46394,
 7.39104, 7.31368, 7.23191, 7.14579, 7.05537, 6.9607, 6.86183, 6.75883,
 6.65176, 6.54068, 6.42566, 6.30677, 6.18408, 6.05767, 5.92761, 5.79398,
 5.65685, 5.51632, 5.37247, 5.22538, 5.07515, 4.92185, 4.76559, 4.60647,
 4.44456, 4.27998, 4.11282, 3.94319, 3.77117, 3.59689, 3.42044, 3.24193,
 3.06147, 2.87916, 2.69512, 2.50945, 2.32228, 2.1337, 1.94384, 1.75281,
 1.56072, 1.3677, 1.17384, 0.979285, 0.784137, 0.588517, 0.392541, 0.19633,
 9.79717e-16, -0.19633, -0.392541, -0.588517, -0.784137, -0.979285, -1.17384, -1.3677,
 -1.56072, -1.75281, -1.94384, -2.1337, -2.32228, -2.50945, -2.69512, -2.87916,
 -3.06147, -3.24193, -3.42044, -3.59689, -3.77117, -3.94319, -4.11282, -4.27998,
 -4.44456, -4.60647, -4.76559, -4.92185, -5.07515, -5.22538, -5.37247, -5.51632,
 -5.65685, -5.79398, -5.92761, -6.05767, -6.18408, -6.30677, -6.42566, -6.54068,
 -6.65176, -6.75883, -6.86183, -6.9607, -7.05537, -7.14579, -7.23191, -7.31368,
 -7.39104, -7.46394, -7.53235, -7.59623, -7.65552, -7.71021, -7.76025, -7.80562,
 -7.84628, -7.88222, -7.91341, -7.93984, -7.96148, -7.97832, -7.99036, -7.99759,
 -8, -7.99759, -7.99036, -7.97832, -7.96148, -7.93984, -7.91341, -7.88222,
 -7.84628, -7.80562, -7.76025, -7.71021, -7.65552, -7.59623, -7.53235, -7.46394,
 -7.39104, -7.31368, -7.23191, -7.14579, -7.05537, -6.9607, -6.86183, -6.75883,
 -6.65176, -6.54068, -6.42566, -6.30677, -6.18408, -6.05767, -5.92761, -5.79398,
 -5.65685, -5.51632, -5.37247, -5.22538, -5.07515, -4.92185, -4.76559, -4.60647,
 -4.44456, -4.27998, -4.11282, -3.94319, -3.77117, -3.59689, -3.42044, -3.24193,
 -3.06147, -2.87916, -2.69512, -2.50945, -2.32228, -2.1337, -1.94384, -1.75281,
 -1.56072, -1.3677, -1.17384, -0.979285, -0.784137, -0.588517, -0.392541, -0.19633,
};
// end