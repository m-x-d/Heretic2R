//
// Client Effects.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "Client Entities.h"

extern client_fx_import_t fxi;

extern cvar_t* r_farclipdist;
extern cvar_t* r_nearclipdist;
extern cvar_t* r_detail;
extern cvar_t* clfx_gravity;
extern cvar_t* fxTest1;
extern cvar_t* fxTest2;
extern cvar_t* fxTest3;
extern cvar_t* fxTest4;
extern cvar_t* compass;

extern int ref_soft;
extern int numprocessedparticles;
extern int numrenderedparticles;

extern qboolean fx_FreezeWorld;

typedef struct ClientEffect_s
{
	void (*SpawnCFX)(centity_t* owner, int type, int flags, vec3_t origin);
	void (*PrecacheCFX)(void);
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
extern void FXDoWaterEntrySplash(centity_t* owner, int type, int flags, vec3_t origin, byte splash_size, vec3_t dir); //mxd
extern void FireSparks(centity_t* owner, int type, int flags, vec3_t origin, vec3_t dir); //mxd
extern void FXDarkSmoke(vec3_t origin, float scale, float range); //mxd
extern qboolean FXDebris_Vanish(struct client_entity_s* self, centity_t* owner); //mxd
extern qboolean FXDebris_Remove(struct client_entity_s* self, centity_t* owner); //mxd

extern void GenericExplosion1(centity_t* owner, int type, int flags, vec3_t origin);
extern void GenericExplosion2(centity_t* owner, int type, int flags, vec3_t origin);
extern void GenericGibTrail(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXBlood(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXBloodTrail(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXLinkedBlood(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXGenericSparks(centity_t* owner, int type, int flags, vec3_t origin);
extern void PlayerTeleportin(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXHealthPickup(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXWeaponPickup(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXDefensePickup(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXPuzzlePickup(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXAmmoPickup(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXFlyingFist(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXFlyingFistExplode(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXBlueRing(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXMeteorBarrier(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXMeteorBarrierTravel(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXMeteorBarrierExplode(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXLightningShield(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXSphereOfAnnihilation(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXSphereOfAnnihilationGlowballs(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXSphereOfAnnihilationExplode(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXSphereOfAnnihilationPower(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXSpherePlayerExplode(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXMagicMissile(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXMagicMissileExplode(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXBlast(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXRedRainMissile(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXRedRain(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXRedRainGlow(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXMaceball(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXMaceballBounce(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXMaceballExplode(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXPhoenixMissile(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXPhoenixExplode(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXMorphMissile(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXMorphMissile_initial(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXMorphExplode(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXFireWave(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXFireWaveWorm(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXFireBurst(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXRipperExplode(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXWaterEntrySplash(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXWaterRipples(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXWaterWake(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXBubbler(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXScorchmark(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXDebris(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXFleshDebris(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXShadow(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXPlayerShadow(centity_t* owner, int type, int flags, vec3_t origin); //mxd
extern void FXAnimate(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXFountain(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXWaterfallBase(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXDripper(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXMist(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXPlagueMist(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXPlagueMistExplode(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXSpellHands(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXLensFlare(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXStaff(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXSpoo(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXHalo(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXRemoteCamera(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXHellbolt(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXHellboltExplode(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXHellstaffPower(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXHellstaffPowerBurn(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXSpellChange(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXStaffCreate(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXStaffCreatePoof(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXStaffRemove(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXDustPuffOnGround(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXFire(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXSound(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXPickup(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXGenericHitPuff(centity_t* owner, int type, int flags, const vec3_t origin);
extern void FXDust(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXEnvSmoke(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXSpooSplat(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXBodyPart(centity_t* owner, int type, int flags, vec3_t origin);
extern void PlayerTeleportout(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXPlayerPersistant(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXSsithraArrowGlow(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXSsithraArrowMissile(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXSsithraArrowExplode(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXplayertorch(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXTomeOfPower(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXFireOnEntity(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXFlareup(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXShrinePlayerEffect(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXShrineManaEffect(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXShrineLungsEffect(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXShrineLightEffect(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXShrineSpeedEffect(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXShrineArmorEffect(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXShrineHealthEffect(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXShrineStaffEffect(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXShrineGhostEffect(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXShrineReflectEffect(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXShrinePowerUpEffect(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXRope(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXFireHands(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXShrineBall(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXShrineBallExplode(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXOgleHitPuff(centity_t* owner, int type, int flags, const vec3_t origin);
extern void FXHPMissile(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXIEffects(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXChickenExplode(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXTeleportPad(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXTPortSmoke(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXWaterParticles(centity_t* owner, int type, int flags, const vec3_t origin);
extern void FXMEffects(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXFlamethrower(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXflametest(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXQuake(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXLightning(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXPowerLightning(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXHPStaff(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXRandWaterBubble(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXBubble(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXMagicPortal(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXTBEffects(centity_t* owner, int type, int flags, vec3_t org);
extern void FXTestBBox(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXSsithraArrow(centity_t* owner, int type, int flags, vec3_t org);
extern void FXPESpell(centity_t* owner, int type, int flags, vec3_t org);
extern void FXLightningHit(centity_t* owner, int type, int flags, vec3_t org);
extern void NullEffect(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXStaffStrike(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXCreateArmorHit(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXBarrelExplode(centity_t* owner, int Type, int Flags, vec3_t Origin);
extern void FXCWatcherEffects(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXCorpseRemove(centity_t* owner, int type, int flags, const vec3_t origin);
extern void FXLeader(centity_t* owner, int type, int flags, const vec3_t origin);
extern void FXTornado(centity_t* Owner, int Type, int Flags, vec3_t Origin);
extern void FXTornadoBall(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXTornadoBallExplode(centity_t* owner, int type, int flags, vec3_t origin);
extern void FXFeetTrail(centity_t* owner, int type, int flags, vec3_t origin);

// Client effect used by another client effect - needs its own wrapper.
extern void FXClientScorchmark(vec3_t origin, vec3_t dir);
extern void FXCrosshair(centity_t* owner, int type, int flags, vec3_t origin);

extern void PreCacheDebris(void);
extern void PreCacheHalos(void);
extern void PreCacheMist(void);
extern void PreCacheBluering(void);
extern void PreCacheDripper(void);
extern void PreCacheBubbler(void);
extern void PreCacheHealth(void);
extern void PreCacheItemWeapons(void);
extern void PreCacheItemDefense(void);
extern void PreCachePuzzleItems(void);
extern void PreCacheItemAmmo(void);
extern void PreCacheHellstaff(void);
extern void PreCacheMorph(void);
extern void PreCachePhoenix(void);
extern void PreCacheRedrain(void);
extern void PreCacheRipples(void);
extern void PreCacheSparks(void);
extern void PreCacheMaceball(void);
extern void PreCacheTeleport(void);
extern void PreCacheSsithraArrow(void);
extern void PreCacheTorch(void);
extern void PreCacheFlareup(void);
extern void PreCacheRockchunks(void);
extern void PreCacheFist(void);
extern void PreCacheWall(void);
extern void PreCacheFlare(void);
extern void PreCacheArray(void);
extern void PreCacheMeteor(void);
extern void PreCacheShield(void);
extern void PreCachePickup(void);
extern void PreCacheScorch(void);
extern void PreCacheSmoke(void);
extern void PreCacheHands(void);
extern void PreCacheSphere(void);
extern void PreCacheSpoo(void);
extern void PreCacheStaff(void);
extern void PreCacheWaterSplash(void);
extern void PreCacheWake(void);
extern void PreCacheShrine(void);
extern void PreCacheRope(void);
extern void PreCacheFXAnimate(void);
extern void PreCacheHPMissile(void);
extern void PreCacheIEffects(void);
extern void PrecacheShadow(void);
extern void PrecacheOgleHitPuff(void);
extern void PreCacheTPortSmoke(void);
extern void PreCacheWaterParticles(void);
extern void PreCacheMEffects(void);
extern void PreCacheLightning(void);
extern void PreCacheHPStaff(void);
extern void PreCacheTB(void);
extern void PreCacheSplat(void);
extern void PrecacheSsithraArrow(void);
extern void PrecachePESpell(void);
extern void PreCachePortal(void);
extern void PreCacheCrosshair(void);
extern void PreCacheStaffHit(void);
extern void PreCacheArmorHit(void);
extern void PreCacheHitPuff(void);
extern void PreCacheObjects(void);
extern void PreCacheCWModels(void);
extern void PreCacheTorn(void);