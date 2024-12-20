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

void PlayerActionHandFXStart(playerinfo_t* info, float value);
void PlayerActionSphereTrailEnd(playerinfo_t* info, float value);

void PlayerActionSpellChange(playerinfo_t* info, float value);
void PlayerActionWeaponChange(playerinfo_t* info, float value);
void PlayerActionArrowChange(playerinfo_t* info, float value);

void PlayerActionSwordAttack(playerinfo_t* info, float value);
void PlayerActionSpellFireball(playerinfo_t* info, float value);
void PlayerActionSpellBlast(playerinfo_t* info, float value);
void PlayerActionSpellArray(playerinfo_t* info, float value);
void PlayerActionSpellSphereCreate(playerinfo_t* info, float value);
void PlayerActionSpellSphereCharge(playerinfo_t* info, float value);
void PlayerActionSpellSphereRelease(playerinfo_t* info, float value);
void PlayerActionSpellBigBall(playerinfo_t* info, float value);
void PlayerActionSpellFirewall(playerinfo_t* info, float value);
void PlayerActionRedRainBowAttack(playerinfo_t* info, float value);
void PlayerActionPhoenixBowAttack(playerinfo_t* info, float value);
void PlayerActionHellstaffAttack(playerinfo_t* info, float value);
void PlayerActionSpellDefensive(playerinfo_t* info, float value);

void PlayerActionStaffTrailStart(playerinfo_t* info, float value);
void PlayerActionRedRainBowTrailStart(playerinfo_t* info, float value);
void PlayerActionPhoenixBowTrailStart(playerinfo_t* info, float value);
void PlayerActionBowTrailEnd(playerinfo_t* info, float value);
void PlayerActionStartStaffGlow(const playerinfo_t* info, float value);
void PlayerActionEndStaffGlow(const playerinfo_t* info, float value);

qboolean PlayerActionCheckVault(playerinfo_t* info, float value);
void PlayerActionSwimIdleSound(const playerinfo_t* info, float value);
void PlayerActionSwimSound(const playerinfo_t* info, float value);
void PlayerActionClimbWallSound(const playerinfo_t* info, float value);
void PlayerActionClimbFinishSound(const playerinfo_t* info, float value);

void PlayerActionFootstep(const playerinfo_t* info, float value);
void PlayerActionSwim(const playerinfo_t* info, float value);
void PlayerActionCheckGrab(playerinfo_t* info, float value);
void PlayerActionCheckFallingGrab(playerinfo_t* info, float value);
qboolean PlayerActionCheckJumpGrab(playerinfo_t* info, float value);
void PlayerActionPushButton(playerinfo_t* info, float value);
void PlayerActionPushLever(playerinfo_t* info, float value);
void PlayerActionVaultSound(const playerinfo_t* info, float value);
void PlayerActionBowReadySound(const playerinfo_t* info, float value);
void PlayerActionTakePuzzle(playerinfo_t* info, float value);

qboolean PlayerActionCheckPuzzleGrab(playerinfo_t* info);
qboolean PlayerActionCheckPushPull(playerinfo_t* info);
qboolean PlayerActionCheckPushButton(playerinfo_t* info);
qboolean PlayerActionCheckPushLever(playerinfo_t* info);
qboolean PlayerActionCheckRopeGrab(playerinfo_t* info, float stomp_org);

qboolean PlayerActionUsePuzzle(playerinfo_t* info);

void PlayerMoveFunc(playerinfo_t* info, float fwd, float right, float up);
void PlayerClimbingMoveFunc(playerinfo_t* info, float height, float var2, float var3);
void PlayerMoveUpperFunc(playerinfo_t* info, float fwd, float right, float up);
void PlayerMoveForce(playerinfo_t* info, float fwd, float right, float up);
void PlayerMoveALittle(playerinfo_t* info, float fwd, float right, float up);
void PlayerPullupHeight(playerinfo_t* info, float height, float endseq, float nopushdown);

void PlayerActionJump(playerinfo_t* info, float value);
void PlayerActionJumpBack(playerinfo_t* info, float value);
void PlayerActionPushAway(playerinfo_t* info, float value);
void PlayerActionShrineEffect(playerinfo_t* info, float value);

void PlayerActionCheckRunUnStrafe(playerinfo_t* playerinfo);

void PlayerActionCheckDoubleJump(playerinfo_t* info);
void PlayerMoveAdd(playerinfo_t* info);
void PlayerActionFlip(playerinfo_t* info, float value);
void PlayerActionTurn180(playerinfo_t* info);
void PlayerActionSetQTEndTime(playerinfo_t* info, float QTEndTime);
void PlayerActionCheckVaultKick(playerinfo_t* info);

void PlayerActionDrownFloatUp(playerinfo_t* info);

void PlayerJumpMoveForce(playerinfo_t* info, float fwd, float right, float up);

void PlayerActionCheckBowRefire(playerinfo_t* info);
void PlayerActionCheckRopeMove(playerinfo_t* info, float foo);

void PlayerActionSetCrouchHeight(playerinfo_t* info);
void PlayerActionCheckUncrouchToFinishSeq(playerinfo_t* info);
void PlayerActionCheckStrafe(playerinfo_t* info);

void PlayerJumpNudge(playerinfo_t* info, float fwd, float right, float up);

void PlayerActionCheckBranchRunningStrafe(playerinfo_t* info);

PLAYER_API void PlayerReleaseRope(playerinfo_t* info);
PLAYER_API void KnockDownPlayer(playerinfo_t* info);
PLAYER_API void PlayFly(const playerinfo_t* info, float dist);
PLAYER_API void PlaySlap(const playerinfo_t* info, float dist);
PLAYER_API void PlayScratch(const playerinfo_t* info, float dist);
PLAYER_API void PlaySigh(const playerinfo_t* info, float dist);
PLAYER_API void SpawnDustPuff(playerinfo_t* info, float dist);

void PlayerSwimMoveFunc(playerinfo_t* info, float fwd, float right, float up);

void PlayerActionClimbStartSound(playerinfo_t* playerinfo, float value);
void PlayerPlaySlide(playerinfo_t* playerinfo);

void PlayerActionCheckCreep(playerinfo_t* info);
void PlayerActionCheckCreepUnStrafe(playerinfo_t* info);

void PlayerActionCheckCreepBack(playerinfo_t* info);
void PlayerActionCheckCreepBackUnStrafe(playerinfo_t* info);

void PlayerActionCheckWalk(playerinfo_t* info);
void PlayerActionCheckWalkUnStrafe(playerinfo_t* info);

void PlayerActionCheckWalkBack(playerinfo_t* info);
void PlayerActionCheckWalkBackUnStrafe(playerinfo_t* info);

void PlayerActionCheckRun(playerinfo_t* info);

extern PLAYER_API void PlayerInterruptAction(playerinfo_t* playerinfo);