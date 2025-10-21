//
// p_actions.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "p_types.h"

//TODO: convert to enum?
#define SOUND_SWIM_FORWARD	0
#define SOUND_SWIM_BACK		1
#define SOUND_SWIM_SIDE		2
#define SOUND_SWIM_UNDER	3

// Information for creep fall checking. //mxd. Originally defined in p_main.h
#define CREEP_MAXFALL		18
#define CREEP_STEPDIST		30

extern PLAYER_API void PlayerReleaseRope(playerinfo_t* info);
extern PLAYER_API void KnockDownPlayer(playerinfo_t* info);
extern PLAYER_API void PlayFly(const playerinfo_t* info, float dist);
extern PLAYER_API void PlaySlap(const playerinfo_t* info, float dist);
extern PLAYER_API void PlayScratch(const playerinfo_t* info, float dist);
extern PLAYER_API void PlaySigh(const playerinfo_t* info, float dist);
extern PLAYER_API void SpawnDustPuff(playerinfo_t* info, float dist);

extern PLAYER_API void PlayerInterruptAction(playerinfo_t* info);

extern void PlayerActionHandFXStart(playerinfo_t* info, float value);
extern void PlayerActionSphereTrailEnd(playerinfo_t* info, float value);

extern void PlayerActionSpellChange(playerinfo_t* info, float value);
extern void PlayerActionWeaponChange(playerinfo_t* info, float value);
extern void PlayerActionArrowChange(playerinfo_t* info, float value);

extern void PlayerActionSwordAttack(playerinfo_t* info, float value);
extern void PlayerActionSpellFireball(playerinfo_t* info, float value);
extern void PlayerActionSpellBlast(playerinfo_t* info, float value);
extern void PlayerActionSpellArray(playerinfo_t* info, float value);
extern void PlayerActionSpellSphereCreate(playerinfo_t* info, float value);
extern void PlayerActionSpellSphereCharge(playerinfo_t* info, float value);
extern void PlayerActionSpellSphereRelease(playerinfo_t* info, float value);
extern void PlayerActionSpellBigBall(playerinfo_t* info, float value);
extern void PlayerActionSpellFirewall(playerinfo_t* info, float value);
extern void PlayerActionRedRainBowAttack(playerinfo_t* info, float value);
extern void PlayerActionPhoenixBowAttack(playerinfo_t* info, float value);
extern void PlayerActionHellstaffAttack(playerinfo_t* info, float value);
extern void PlayerActionSpellDefensive(playerinfo_t* info, float value);

extern void PlayerActionStaffTrailStart(playerinfo_t* info, float value);
extern void PlayerActionRedRainBowTrailStart(playerinfo_t* info, float value);
extern void PlayerActionPhoenixBowTrailStart(playerinfo_t* info, float value);
extern void PlayerActionBowTrailEnd(playerinfo_t* info, float value);
extern void PlayerActionStartStaffGlow(const playerinfo_t* info, float value);
extern void PlayerActionEndStaffGlow(const playerinfo_t* info, float value);

extern qboolean PlayerActionCheckVault(playerinfo_t* info);
extern void PlayerActionSwimIdleSound(const playerinfo_t* info, float value);
extern void PlayerActionSwimSound(const playerinfo_t* info, float value);
extern void PlayerActionClimbWallSound(const playerinfo_t* info, float value);
extern void PlayerActionClimbFinishSound(const playerinfo_t* info, float value);

extern void PlayerActionFootstep(playerinfo_t* info, float value);
extern void PlayerActionSwim(const playerinfo_t* info, float value);
extern void PlayerActionCheckGrab(playerinfo_t* info, float value);
extern void PlayerActionCheckFallingGrab(playerinfo_t* info, float value);
extern qboolean PlayerActionCheckJumpGrab(playerinfo_t* info, float value);
extern void PlayerActionPushButton(playerinfo_t* info, float value);
extern void PlayerActionPushLever(playerinfo_t* info, float value);
extern void PlayerActionVaultSound(const playerinfo_t* info, float value);
extern void PlayerActionBowReadySound(const playerinfo_t* info, float value);
extern void PlayerActionTakePuzzle(playerinfo_t* info, float value);

extern qboolean PlayerActionCheckPuzzleGrab(playerinfo_t* info);
extern qboolean PlayerActionCheckPushButton(const playerinfo_t* info);
extern qboolean PlayerActionCheckPushLever(const playerinfo_t* info);
extern qboolean PlayerActionCheckRopeGrab(playerinfo_t* info, float stomp_org);

extern qboolean PlayerActionUsePuzzle(const playerinfo_t* info);

extern void PlayerMoveFunc(playerinfo_t* info, float fwd, float right, float up);
extern void PlayerClimbingMoveFunc(playerinfo_t* info, float height, float var2, float var3);
extern void PlayerMoveUpperFunc(playerinfo_t* info, float fwd, float right, float up);
extern void PlayerMoveForce(playerinfo_t* info, float fwd, float right, float up);
extern void PlayerJumpMoveForce(playerinfo_t* info, float fwd, float right, float up);
extern void PlayerJumpNudge(playerinfo_t* info, float fwd, float right, float up);
extern void PlayerMoveALittle(playerinfo_t* info, float fwd, float right, float up);
extern void PlayerPullupHeight(playerinfo_t* info, float height, float endseq, float nopushdown);

extern void PlayerActionJump(playerinfo_t* info, float value);
extern void PlayerActionJumpBack(playerinfo_t* info, float value);
extern void PlayerActionShrineEffect(playerinfo_t* info, float value);

extern void PlayerActionCheckRunUnStrafe(playerinfo_t* info);
extern void PlayerActionCheckDoubleJump(playerinfo_t* info);
extern void PlayerMoveAdd(playerinfo_t* info);
extern void PlayerActionFlip(playerinfo_t* info, float value);
extern void PlayerActionTurn180(playerinfo_t* info);
extern void PlayerActionSetQTEndTime(playerinfo_t* info, float QTEndTime);
extern void PlayerActionCheckVaultKick(playerinfo_t* info);
extern void PlayerActionDrownFloatUp(playerinfo_t* info);

extern void PlayerActionCheckBowRefire(playerinfo_t* info);
extern void PlayerActionCheckRopeMove(playerinfo_t* info, float foo);

extern void PlayerActionSetCrouchHeight(playerinfo_t* info);
extern void PlayerActionCheckUncrouchToFinishSeq(playerinfo_t* info);
extern void PlayerActionCheckStrafe(playerinfo_t* info);
extern void PlayerActionCheckBranchRunningStrafe(playerinfo_t* info);

extern void PlayerSwimMoveFunc(playerinfo_t* info, float fwd, float right, float up);

extern void PlayerActionClimbStartSound(const playerinfo_t* info, float value);
extern void PlayerPlaySlide(const playerinfo_t* info);

extern void PlayerActionCheckCreep(playerinfo_t* info);
extern void PlayerActionCheckCreepUnStrafe(playerinfo_t* info);

extern void PlayerActionCheckCreepBack(playerinfo_t* info);
extern void PlayerActionCheckCreepBackUnStrafe(playerinfo_t* info);

extern void PlayerActionCheckWalk(playerinfo_t* info);
extern void PlayerActionCheckWalkUnStrafe(playerinfo_t* info);

extern void PlayerActionCheckWalkBack(playerinfo_t* info);
extern void PlayerActionCheckWalkBackUnStrafe(playerinfo_t* info);

extern void PlayerActionCheckRun(playerinfo_t* info);

extern void PlayerActionSetDead(playerinfo_t* info); //mxd