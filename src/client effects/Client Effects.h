#ifndef CLIENT_H
#define CLIENT_H
#include "client.h"
#endif

extern client_fx_import_t fxi;

extern cvar_t	*r_farclipdist;
extern cvar_t	*r_nearclipdist;
extern cvar_t	*r_detail;
extern cvar_t	*clfx_gravity;
extern cvar_t	*fxTest1;
extern cvar_t	*fxTest2;
extern cvar_t	*fxTest3;
extern cvar_t	*fxTest4;
extern cvar_t	*cl_timedemo;
extern cvar_t	*compass;

extern qboolean fx_FreezeWorld;

typedef struct ClientEffect_s
{
	void (*SpawnCFX)(centity_t *owner, int type, int flags, vec3_t origin);
	void (*PrecacheCFX)(void);
	char *formatString;
} ClientEffect_t;

extern ClientEffect_t clientEffectSpawners[];

// ***************************************************************************************
// Client Sounds
// ***************************************************************************************

extern struct sfx_s	*cl_sfx_ric1;
extern struct sfx_s	*cl_sfx_ric2;
extern struct sfx_s	*cl_sfx_ric3;

// ***************************************************************************************
// Initialisers for ClassStatics used by client-effects.
// ***************************************************************************************

extern void InitDebrisStatics(void);
extern void InitWallStatics(void);

extern float cl_turbsin[];
// ***************************************************************************************
// Client-effect functions.
// ***************************************************************************************

void RemoveEffects(centity_t *owner, int type, int flags, vec3_t origin);
void GenericExplosion1(centity_t *owner, int type, int flags, vec3_t origin);
void GenericExplosion2(centity_t *owner, int type, int flags, vec3_t origin);
void WaterSplash(centity_t *owner, int type, int flags, vec3_t origin);
void GenericGibTrail(centity_t *owner, int type, int flags, vec3_t origin);
void FXBlood(centity_t *owner, int type, int flags, vec3_t origin);
void FXBloodTrail(centity_t *owner, int type, int flags, vec3_t origin);
void FXLinkedBlood(centity_t *owner, int type, int flags, vec3_t origin);
void FXGenericSparks(centity_t *owner, int type, int flags, vec3_t origin);
void PlayerTeleportin(centity_t *owner, int type, int flags, vec3_t origin);
void FXHealthPickup(centity_t *owner, int type, int flags, vec3_t origin);
void FXWeaponPickup(centity_t *owner, int type, int flags, vec3_t origin);
void FXDefensePickup(centity_t *owner, int type, int flags, vec3_t origin);
void FXPuzzlePickup(centity_t *owner, int type, int flags, vec3_t origin);
void FXAmmoPickup(centity_t *owner, int type, int flags, vec3_t origin);
void FXFlyingFist(centity_t *owner, int type, int flags, vec3_t origin);
void FXFlyingFistExplode(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXBlueRing(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXMeteorBarrier(centity_t *owner, int type, int flags, vec3_t origin);
void FXMeteorBarrierTravel(centity_t *owner, int type, int flags, vec3_t origin);
void FXMeteorBarrierExplode(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXLightningShield(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXSphereOfAnnihilation(centity_t *owner, int type, int flags, vec3_t origin);
void FXSphereOfAnnihilationGlowballs(centity_t *owner, int type, int flags, vec3_t origin);
void FXSphereOfAnnihilationExplode(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXSphereOfAnnihilationPower(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXSpherePlayerExplode(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXMagicMissile(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXMagicMissileExplode(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXBlast(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXRedRainMissile(centity_t *Owner, int Type, int Flags, vec3_t Origin);
void FXRedRain(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXRedRainGlow(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXMaceball(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXMaceballBounce(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXMaceballExplode(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXPhoenixMissile(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXPhoenixExplode(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXMorphMissile(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXMorphMissile_initial(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXMorphExplode(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXFireWave(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXFireWaveWorm(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXFireBurst(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXRipperExplode(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXWaterEntrySplash(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXWaterRipples(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXWaterWake(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXBubbler(centity_t *Owner, int Type, int Flags, vec3_t Origin);
void FXScorchmark(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXDebris(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXFleshDebris(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXShadow(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXAnimate(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXFountain(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXWaterfallBase(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXDripper(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXMist(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXPlagueMist(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXPlagueMistExplode(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXSpellHands(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXLensFlare(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXStaff(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXSpoo(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXHalo(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXRemoteCamera(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXHellbolt(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXHellboltExplode(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXHellstaffPower(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXHellstaffPowerBurn(centity_t *owner, int type, int flags, vec3_t origin);
void FXSpellChange(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXStaffCreate(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXStaffCreatePoof(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXStaffRemove(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXDustPuffOnGround(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXFire(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXSound(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXPickup(centity_t *owner, int type, int flags, vec3_t origin);
void FXGenericHitPuff(centity_t *owner, int type, int flags, vec3_t origin);
void FXDust(centity_t *owner, int type, int flags, vec3_t origin);
void FXEnvSmoke(centity_t *owner, int type, int flags, vec3_t origin);
void FXSpooSplat(centity_t *owner, int type, int flags, vec3_t origin);
//void FXSpooSneeze(centity_t *owner, int type, int flags, vec3_t origin);
void FXBodyPart(centity_t *owner,int type,int flags,vec3_t origin);
void PlayerTeleportout(centity_t *owner, int type, int flags, vec3_t origin);
void FXPlayerPersistant(centity_t *owner, int type, int flags, vec3_t origin);
void FXSsithraArrowGlow(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXSsithraArrowMissile(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXSsithraArrowExplode(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXplayertorch(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXTomeOfPower(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXFireOnEntity(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXFlareup(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXShrinePlayerEffect(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXShrineManaEffect(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXShrineLungsEffect(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXShrineLightEffect(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXShrineSpeedEffect(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXShrineArmorEffect(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXShrineHealthEffect(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXShrineStaffEffect(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXShrineGhostEffect(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXShrineEffect(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXShrineReflectEffect(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXShrinePowerUpEffect(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXRope(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXFireHands(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXShrineBall(centity_t *owner, int type, int flags, vec3_t origin);
void FXShrineBallExplode(centity_t *owner, int type, int flags, vec3_t origin);
void FXOgleHitPuff(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXHPMissile(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXIEffects(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXChickenExplode(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXTeleportPad(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXTPortSmoke(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXWaterParticles(centity_t *owner, int type, int flags, vec3_t origin);
void FXMEffects(centity_t *owner,int type,int flags,vec3_t origin);
void FXFlamethrower(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXflametest(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXQuake(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXLightning(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXPowerLightning(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXHPStaff(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXRandWaterBubble(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXBubble(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXMagicPortal(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXTBEffects(centity_t *owner,int type,int flags, vec3_t org);
void FXTestBBox(centity_t *Owner,int Type,int Flags,vec3_t Origin);
void FXSsithraArrow(centity_t *owner,int type,int flags, vec3_t org);
void FXPESpell(centity_t *owner,int type,int flags, vec3_t org);
void FXLightningHit(centity_t *owner,int type,int flags, vec3_t org);
void NullEffect(centity_t *owner, int type, int flags, vec3_t origin);
void FXStaffStrike(centity_t *owner, int type, int flags, vec3_t origin);
void FXCreateArmorHit(centity_t *owner,int Type,int Flags,vec3_t Origin);
void FXBarrelExplode(centity_t *owner,int Type,int Flags,vec3_t Origin);
void FXCWatcherEffects(centity_t *owner,int Type,int Flags,vec3_t Origin);
void FXCorpseRemove(centity_t *Owner, int Type, int Flags, vec3_t Origin);
void FXLeader(centity_t *Owner, int Type, int Flags, vec3_t Origin);
void FXTornado(centity_t *Owner, int Type, int Flags, vec3_t Origin);
void FXTornadoBall(centity_t *owner, int type, int flags, vec3_t origin);
void FXTornadoBallExplode(centity_t *owner, int type, int flags, vec3_t origin);
void FXFeetTrail(centity_t *owner, int type, int flags, vec3_t origin);


// client effect used by another client effect - needs its own wrapper
void FXClientScorchmark(vec3_t origin, vec3_t dir);
void FXCrosshair(centity_t *owner, int type, int flags, vec3_t origin);

void PreCacheDebris(void);
void PreCacheHalos(void);
void PreCacheMist(void);
void PreCacheBluering(void);
void PreCacheDripper(void);
void PreCacheBubbler(void);
void PreCacheHealth(void);
void PreCacheItemWeapons(void);
void PreCacheItemDefense(void);
void PreCachePuzzleItems(void);
void PreCacheItemAmmo(void);
void PreCacheHellstaff(void);
void PreCacheMorph(void);
void PreCachePhoenix(void);
void PreCacheRedrain(void);
void PreCacheRipples(void);
void PreCacheSparks(void);
void PreCacheMaceball(void);
void PreCacheTeleport(void);
void PreCacheSsithraArrow(void);
void PreCacheTorch(void);
void PreCacheFlareup(void);
void PreCacheRockchunks(void);
void PreCacheFist(void);
void PreCacheWall(void);
void PreCacheFlare(void);
void PreCacheArray(void);
void PreCacheMeteor(void);
void PreCacheShield(void);
void PreCachePickup(void);
void PreCacheScorch(void);
void PreCacheSmoke(void);
void PreCacheHands(void);
void PreCacheSphere(void);
void PreCacheSpoo(void);
void PreCacheStaff(void);
void PreCacheWaterSplash(void);
void PreCacheWake(void);
void PreCacheShrine(void);
void PreCacheRope(void);
void PreCacheFXAnimate(void);
void PreCacheHPMissile(void);
void PreCacheIEffects(void);
void PrecacheShadow(void);
void PrecacheOgleHitPuff(void);
void PreCacheTPortSmoke(void);
void PreCacheWaterParticles(void);
void PreCacheMEffects(void);
void PreCacheLightning(void);
void PreCacheHPStaff(void);
void PreCacheTB(void);
void PreCacheSplat(void);
void PrecacheSsithraArrow(void);
void PrecachePESpell(void);
void PreCachePortal(void);
void PreCacheCrosshair(void);
void PreCacheStaffHit(void);
void PreCacheArmorHit(void);
void PreCacheHitPuff(void);
void PreCacheObjects(void);
void PreCacheCWModels(void);
void PreCacheTorn(void);

// end
