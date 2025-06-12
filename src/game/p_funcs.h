//
// p_funcs.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "p_types.h"

extern entity_state_t* G_GetEntityStatePtr(edict_t* entity);
extern int G_BranchLwrClimbing(playerinfo_t* info);
extern qboolean G_PlayerActionCheckRopeGrab(playerinfo_t* info, float stomp_org);
extern void G_PlayerClimbingMoveFunc(playerinfo_t* info, float height, float var2, float var3);
extern qboolean G_PlayerActionCheckPuzzleGrab(playerinfo_t* info);
extern void G_PlayerActionTakePuzzle(const playerinfo_t* info);
extern qboolean G_PlayerActionUsePuzzle(const playerinfo_t* info);
extern qboolean G_PlayerActionCheckPushPull_Ent(const edict_t* ent); //mxd. 'void* ent' in original logic.
extern void G_PlayerActionMoveItem(const playerinfo_t* info, float distance);
extern qboolean G_PlayerActionCheckPushButton(const playerinfo_t* info);
extern void G_PlayerActionPushButton(const playerinfo_t* info);
extern qboolean G_PlayerActionCheckPushLever(const playerinfo_t* info);
extern void G_PlayerActionPushLever(const playerinfo_t* info);
extern qboolean G_HandleTeleport(const playerinfo_t* info);
extern void G_SetJointAngles(const playerinfo_t* info);
extern void G_ResetJointAngles(const playerinfo_t* info);
extern void G_PlayerActionChickenBite(const playerinfo_t* info);
extern void G_PlayerFallingDamage(const playerinfo_t* info, float delta);
extern void G_PlayerActionSwordAttack(const playerinfo_t* info, int value);
extern void G_PlayerActionSpellFireball(const playerinfo_t* info);
extern void G_PlayerActionSpellBlast(const playerinfo_t* info);
extern void G_PlayerActionSpellArray(const playerinfo_t* info, int value);
extern void G_PlayerActionSpellSphereCreate(const playerinfo_t* info, qboolean* charging);
extern void G_PlayerActionSpellBigBall(const playerinfo_t* info);
extern void G_PlayerActionSpellFirewall(const playerinfo_t* info);
extern void G_PlayerActionRedRainBowAttack(const playerinfo_t* info);
extern void G_PlayerActionPhoenixBowAttack(const playerinfo_t* info);
extern void G_PlayerActionHellstaffAttack(const playerinfo_t* info);
extern void G_PlayerActionSpellDefensive(playerinfo_t* info);
extern void G_PlayerSpellShieldAttack(const playerinfo_t* info);
extern void G_PlayerSpellStopShieldAttack(const playerinfo_t* info);
extern void G_PlayerVaultKick(const playerinfo_t* info);
extern void G_PlayerActionCheckRopeMove(playerinfo_t* info);
extern qboolean G_EntIsAButton(const edict_t* ent);
extern void PlayerChickenDeath(edict_t* self); //mxd