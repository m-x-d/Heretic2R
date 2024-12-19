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
#define	SOUND_SWIM_UNDER	3

void PlayerActionHandFXStart(playerinfo_t* playerinfo, float value);
void PlayerActionSphereTrailEnd(playerinfo_t* playerinfo, float value);

void PlayerActionSpellChange(playerinfo_t* playerinfo, float value);
void PlayerActionWeaponChange(playerinfo_t* playerinfo, float value);
void PlayerActionArrowChange(playerinfo_t* info, float value);

void PlayerActionSwordAttack(playerinfo_t* playerinfo, float value);
void PlayerActionSpellFireball(playerinfo_t* playerinfo, float value);
void PlayerActionSpellBlast(playerinfo_t* playerinfo, float value);
void PlayerActionSpellArray(playerinfo_t* playerinfo, float value);
void PlayerActionSpellSphereCreate(playerinfo_t* playerinfo, float value);
void PlayerActionSpellSphereCharge(playerinfo_t* playerinfo, float value);
void PlayerActionSpellSphereRelease(playerinfo_t* playerinfo, float value);
void PlayerActionSpellBigBall(playerinfo_t* playerinfo, float value);
void PlayerActionSpellFirewall(playerinfo_t* playerinfo, float value);
void PlayerActionRedRainBowAttack(playerinfo_t* playerinfo, float value);
void PlayerActionPhoenixBowAttack(playerinfo_t* playerinfo, float value);
void PlayerActionHellstaffAttack(playerinfo_t* playerinfo, float value);
void PlayerActionSpellDefensive(playerinfo_t* playerinfo, float value);

void PlayerActionSwordTrailStart(playerinfo_t* playerinfo, float value);
void PlayerActionRedRainBowTrailStart(playerinfo_t* playerinfo, float value);
void PlayerActionPhoenixBowTrailStart(playerinfo_t* playerinfo, float value);
void PlayerActionBowTrailEnd(playerinfo_t* playerinfo, float value);
void PlayerActionStartStaffGlow(playerinfo_t* playerinfo, float value);
void PlayerActionEndStaffGlow(playerinfo_t* playerinfo, float value);

qboolean PlayerActionCheckVault(playerinfo_t* playerinfo, float value);
void PlayerActionSwimIdleSound(playerinfo_t* playerinfo, float value);
void PlayerActionSwimSound(playerinfo_t* playerinfo, float value);
void PlayerActionClimbWallSound(playerinfo_t* playerinfo, float value);
void PlayerActionClimbFinishSound(playerinfo_t* playerinfo, float value);

void PlayerActionFootstep(playerinfo_t* playerinfo, float value);
void PlayerActionSwim(playerinfo_t* playerinfo, float value);
void PlayerActionPullup(playerinfo_t* playerinfo, float value);
void PlayerActionSkidSmoke(playerinfo_t* playerinfo, float value);
void PlayerActionCheckGrab(playerinfo_t* playerinfo, float value);
void PlayerActionCheckFallingGrab(playerinfo_t* playerinfo, float value);
qboolean PlayerActionCheckJumpGrab(playerinfo_t* playerinfo, float value);
void PlayerActionSplashFeet(playerinfo_t* playerinfo, float value);
void PlayerActionBowSound(playerinfo_t* playerinfo, float value);
void PlayerActionIdleSound(playerinfo_t* playerinfo, float value);
void PlayerActionPushButton(playerinfo_t* playerinfo, float value);
void PlayerActionPushLever(playerinfo_t* playerinfo, float value);
void PlayerActionVaultSound(playerinfo_t* playerinfo, float value);
void PlayerActionBowReadySound(playerinfo_t* playerinfo, float value);
void PlayerActionTakePuzzle(playerinfo_t* playerinfo, float value);
void PlayerActionMoveItem(playerinfo_t* playerinfo, float distance);

qboolean PlayerActionCheckPuzzleGrab(playerinfo_t* playerinfo);
qboolean PlayerActionCheckPushPull(playerinfo_t* playerinfo);
qboolean PlayerActionCheckPushButton(playerinfo_t* playerinfo);
qboolean PlayerActionCheckPushLever(playerinfo_t* playerinfo);
qboolean PlayerActionCheckRopeGrab(playerinfo_t* playerinfo, float stomp_org);

qboolean PlayerActionUsePuzzle(playerinfo_t* playerinfo);

void PlayerMoveFunc(playerinfo_t* playerinfo, float fwd, float right, float up);
void PlayerClimbingMoveFunc(playerinfo_t* playerinfo, float height, float var2, float var3);
void PlayerMoveUpperFunc(playerinfo_t* playerinfo, float fwd, float right, float up);
void PlayerMoveForce(playerinfo_t* playerinfo, float fwd, float right, float up);
void PlayerMoveALittle(playerinfo_t* playerinfo, float fwd, float right, float up);
void PlayerPullupHeight(playerinfo_t* playerinfo, float height, float endseq, float nopushdown);

void PlayerActionFlipForward(playerinfo_t* playerinfo, float value);
void PlayerActionJump(playerinfo_t* playerinfo, float value);
void PlayerActionJumpBack(playerinfo_t* playerinfo, float value);
void PlayerActionPushAway(playerinfo_t* playerinfo, float value);
void PlayerActionShrineEffect(playerinfo_t* playerinfo, float value);

void PlayerActionCheckRunUnStrafe(playerinfo_t* playerinfo);

void PlayerActionCheckDoubleJump(playerinfo_t* playerinfo);
void PlayerMoveAdd(playerinfo_t* playerinfo);
void PlayerActionFlip(playerinfo_t* playerinfo, float value);
void PlayerActionTurn180(playerinfo_t* playerinfo);
void PlayerActionSetQTEndTime(playerinfo_t* playerinfo, float QTEndTime);
void PlayerActionCheckVaultKick(playerinfo_t* playerinfo);

void PlayerActionDrownFloatUp(playerinfo_t* playerinfo);

void PlayerJumpMoveForce(playerinfo_t* playerinfo, float fwd, float right, float up);
void PlayerActionCheckDive(playerinfo_t* playerinfo);

void PlayerActionCheckBowRefire(playerinfo_t* playerinfo);
void PlayerActionCheckRopeMove(playerinfo_t* playerinfo, float foo);

void PlayerActionSetCrouchHeight(playerinfo_t* playerinfo);
void PlayerActionCheckUncrouchToFinishSeq(playerinfo_t* playerinfo);
void PlayerActionCheckStrafe(playerinfo_t* playerinfo);

void PlayerJumpNudge(playerinfo_t* playerinfo, float fwd, float right, float up);

void PlayerActionCheckBranchRunningStrafe(playerinfo_t* playerinfo);

PLAYER_API void PlayerReleaseRope(playerinfo_t* playerinfo);
PLAYER_API void KnockDownPlayer(playerinfo_t* playerinfo);
PLAYER_API void PlayFly(playerinfo_t* playerinfo, float dist);
PLAYER_API void PlaySlap(playerinfo_t* playerinfo, float dist);
PLAYER_API void PlayScratch(playerinfo_t* playerinfo, float dist);
PLAYER_API void PlaySigh(playerinfo_t* playerinfo, float dist);
PLAYER_API void SpawnDustPuff(playerinfo_t* playerinfo, float dist);

void PlayerSwimMoveFunc(playerinfo_t* playerinfo, float fwd, float right, float up);

void PlayerActionClimbStartSound(playerinfo_t* playerinfo, float value);
void PlayerPlaySlide(playerinfo_t* playerinfo);

void PlayerActionCheckCreep(playerinfo_t* playerinfo);
void PlayerActionCheckCreepUnStrafe(playerinfo_t* playerinfo);

void PlayerActionCheckCreepBack(playerinfo_t* playerinfo);
void PlayerActionCheckCreepBackUnStrafe(playerinfo_t* playerinfo);

void PlayerActionCheckWalk(playerinfo_t* playerinfo);
void PlayerActionCheckWalkUnStrafe(playerinfo_t* playerinfo);

void PlayerActionCheckWalkBack(playerinfo_t* playerinfo);
void PlayerActionCheckWalkBackUnStrafe(playerinfo_t* playerinfo);

void PlayerActionCheckRun(playerinfo_t* playerinfo);

extern PLAYER_API void PlayerInterruptAction(playerinfo_t* playerinfo);