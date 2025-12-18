//
// Client Effects.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "Client Entities.h"

#define MIN_UPDATE_TIME	17 //mxd

extern const paletteRGBA_t color_white; //mxd
extern const paletteRGBA_t color_red; //mxd
extern const paletteRGBA_t color_orange; //mxd

extern client_fx_import_t fxi;

extern cvar_t* r_farclipdist;
extern cvar_t* r_nearclipdist;
extern cvar_t* r_detail;
extern cvar_t* clfx_gravity;

#define R_DETAIL	((int)r_detail->value) //mxd

extern qboolean ref_soft;
extern int numprocessedparticles;
extern int numrenderedparticles;

typedef struct ClientEffect_s
{
	void (*SpawnCFX)(centity_t* owner, int type, int flags, vec3_t origin);
	void (*PrecacheCFX)(void);
	void (*PrecacheSFX)(void); //mxd. Sound effects precache logic.
	char* formatString;
} ClientEffect_t;

extern ClientEffect_t clientEffectSpawners[];

extern void (*ce_class_statics_inits[NUM_CLASSIDS])(void); //mxd

// Initialisers for ClassStatics used by client-effects.
extern void InitDebrisStatics(void);

// Client-effect functions.
extern void RegisterSounds(void); //mxd
extern void RegisterModels(void); //mxd
extern void RemoveEffects(centity_t* owner, int type, int flags, vec3_t origin);

extern void MakeBubble(vec3_t loc, client_entity_t* spawner); //mxd
extern void DoWaterEntrySplash(int type, int flags, vec3_t origin, byte splash_size, vec3_t dir); //mxd

extern void FXExplosion1(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXExplosion2(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXGibTrail(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXBlood(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXBloodTrail(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXLinkedBlood(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXGenericSparks(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXPlayerTeleportIn(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXHealthPickup(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXWeaponPickup(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXDefensePickup(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXPuzzlePickup(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXAmmoPickup(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXFlyingFist(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXFlyingFistExplode(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXBlueRing(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXMeteorBarrier(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXMeteorBarrierTravel(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXMeteorBarrierExplode(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXLightningShield(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXSphereOfAnnihilation(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXSphereOfAnnihilationGlowballs(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXSphereOfAnnihilationExplode(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXSphereOfAnnihilationPower(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXSpherePlayerExplode(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXMagicMissile(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXMagicMissileExplode(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXBlast(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXRedRainMissile(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXRedRain(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXRedRainGlow(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXMaceball(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXMaceballBounce(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXMaceballExplode(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXPhoenixMissile(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXPhoenixExplode(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXMorphMissile(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXMorphMissileInitial(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXMorphExplode(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXFireWave(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXFireWaveWorm(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXFireBurst(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXRipperExplode(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXWaterEntrySplash(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXWaterRipples(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXWaterWake(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXBubbler(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXScorchmark(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXDebris(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXFleshDebris(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXShadow(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXPlayerShadow(centity_t* owner, int type, int flags, vec3_t origin); //mxd
extern void FXAnimate(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXFountain(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXWaterfallBase(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXDripper(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXMist(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXPlagueMist(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXPlagueMistExplode(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXSpellHands(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXLensFlare(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXStaff(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXSpoo(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXHalo(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXRemoteCamera(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXHellbolt(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXHellboltExplode(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXHellstaffPower(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXHellstaffPowerBurn(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXSpellChange(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXStaffCreate(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXStaffCreatePoof(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXStaffRemove(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXDustPuffOnGround(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXFire(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXSound(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXPickup(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXHitPuff(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXDust(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXEnvSmoke(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXSpooSplat(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXBodyPart(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXPlayerTeleportOut(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXPlayerPersistant(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXPlayerTorch(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXTomeOfPower(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXFireOnEntity(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXFlareup(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXShrinePlayerEffect(centity_t* owner, int type, int flags, vec3_t origin); //TODO: unused
extern void FXShrineManaEffect(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXShrineLungsEffect(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXShrineLightEffect(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXShrineSpeedEffect(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXShrineArmorEffect(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXShrineHealthEffect(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXShrineStaffEffect(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXShrineGhostEffect(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXShrineReflectEffect(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXShrinePowerupEffect(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXRope(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXFireHands(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXShrineBall(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXShrineBallExplode(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXOgleHitPuff(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXHPMissile(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXInsectEffects(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXChickenExplode(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXTeleportPad(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXTPortSmoke(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXWaterParticles(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXMEffects(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXFlamethrower(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXflametest(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXQuake(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXLightning(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXPowerLightning(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXHPStaff(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXRandWaterBubble(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXBubble(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXMagicPortal(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXTBEffects(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXTestBBox(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXSsithraArrow(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXPESpell(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXLightningHit(centity_t* owner, int type, int flags, vec3_t origin);
extern void NullEffect(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXStaffStrike(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXArmorHit(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXBarrelExplode(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXCWatcherEffects(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXCorpseRemove(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXLeader(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXTornado(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXTornadoBall(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXTornadoBallExplode(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXFeetTrail(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXWaterSplash(centity_t* owner, int type, int flags, vec3_t origin); //mxd

// Client effect used by another client effect - needs its own wrapper.
extern void FXClientScorchmark(vec3_t origin, vec3_t dir);
extern void FXCrosshair(centity_t* owner, int type, int flags, vec3_t origin);

extern void PreCacheDebris(void);
extern void PreCacheDebrisSFX(void); //mxd
extern void PreCacheHalos(void);
extern void PreCacheMist(void);
extern void PreCacheBluering(void);
extern void PreCacheBlueringSFX(void); //mxd
extern void PreCacheDripper(void);
extern void PreCacheDripperSFX(void); //mxd
extern void PreCacheBubbler(void);
extern void PreCacheBubblerSFX(void); //mxd
extern void PreCacheHealth(void);
extern void PreCacheItemWeapons(void);
extern void PreCacheItemDefense(void);
extern void PreCachePuzzleItems(void);
extern void PreCacheItemAmmo(void);
extern void PreCacheHellstaff(void);
extern void PreCacheHellstaffSFX(void); //mxd
extern void PreCacheMorph(void);
extern void PreCacheMorphSFX(void); //mxd
extern void PreCachePhoenix(void);
extern void PreCachePhoenixExplodeSFX(void); //mxd
extern void PreCacheRedrain(void);
extern void PreCacheRedrainSFX(void); //mxd
extern void PreCacheRipples(void);
extern void PreCacheSparks(void);
extern void PreCacheSparksSFX(void); //mxd
extern void PreCacheMaceball(void);
extern void PreCacheRipperSFX(void); //mxd
extern void PreCacheTeleport(void);
extern void PreCacheTome(void); //mxd
extern void PreCacheTomeSFX(void); //mxd
extern void PreCacheTorch(void);
extern void PreCacheFlareup(void);
extern void PreCacheRockchunks(void);
extern void PreCacheFist(void);
extern void PreCacheFistSFX(void); //mxd
extern void PreCacheWall(void);
extern void PreCacheFlare(void);
extern void PreCacheArray(void);
extern void PreCacheMeteor(void);
extern void PreCacheShield(void);
extern void PreCachePickup(void);
extern void PreCacheScorch(void);
extern void PreCacheSmoke(void);
extern void PreCacheSmokeSFX(void); //mxd
extern void PreCacheSphere(void);
extern void PreCacheSpoo(void);
extern void PreCacheStaff(void);
extern void PreCacheWaterSplash(void);
extern void PreCacheWaterSplashSFX(void); //mxd
extern void PreCacheWake(void);
extern void PreCacheShrine(void);
extern void PreCacheRope(void);
extern void PreCacheFXAnimate(void);
extern void PreCacheHPMissile(void);
extern void PreCacheIEffects(void);
extern void PreCacheInsectStaffSFX(void); //mxd
extern void PreCacheShadow(void);
extern void PreCacheHitPuff(void); //mxd
extern void PreCacheOgleHitPuff(void);
extern void PreCacheMEffects(void);
extern void PreCacheLightning(void);
extern void PreCachePowerLightningSFX(void); //mxd
extern void PreCacheHPStaff(void);
extern void PreCacheTB(void);
extern void PreCacheSplat(void);
extern void PreCacheSplatSFX(void); //mxd
extern void PreCacheSsithraArrow(void);
extern void PreCachePESpell(void);
extern void PreCachePESpellSFX(void);
extern void PreCachePortal(void);
extern void PreCacheCrosshair(void);
extern void PreCacheStaffHit(void);
extern void PreCacheArmorHit(void);
extern void PreCacheLightningHit(void);
extern void PreCacheLightningHitSFX(void); //mxd
extern void PreCacheBarrelExplode(void);
extern void PreCacheBarrelExplodeSFX(void); //mxd
extern void PreCacheCWModels(void);
extern void PreCacheCWSFX(void); //mxd
extern void PreCacheTornado(void);
extern void PreCacheTPortSmokeSFX(void); //mxd
extern void PreCacheFlamethrowerSFX(void); //mxd
extern void PreCacheFXSoundSFX(void); //mxd