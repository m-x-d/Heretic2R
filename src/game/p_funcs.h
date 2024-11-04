// p_funcs.h
//
// Heretic II - Raven software
//
// Written by Marcus Whitlock
//

#ifndef _P_FUNCS_H_
#define _P_FUNCS_H_

#include "g_local.h"

extern void G_PlayerActionShrineEffect(playerinfo_t *playerinfo);
extern entity_state_t *G_GetEntityStatePtr(edict_t *entity);
extern int G_BranchLwrClimbing(playerinfo_t *playerinfo);
extern qboolean G_PlayerActionCheckRopeGrab(playerinfo_t *playerinfo, float stomp_org);
extern void G_PlayerClimbingMoveFunc(playerinfo_t *playerinfo, float height, float var2, float var3);
extern qboolean G_PlayerActionCheckPuzzleGrab(playerinfo_t *playerinfo);
extern void G_PlayerActionTakePuzzle(playerinfo_t *playerinfo);
extern qboolean G_PlayerActionUsePuzzle(playerinfo_t *playerinfo);
extern qboolean G_PlayerActionCheckPushPull_Ent(void *ent);
extern void PushPull_stop(edict_t *self);
extern void G_PlayerActionMoveItem(playerinfo_t *playerinfo,float distance);
extern qboolean G_PlayerActionCheckPushButton(playerinfo_t *playerinfo);
extern void G_PlayerActionPushButton(playerinfo_t *playerinfo);
extern qboolean G_PlayerActionCheckPushLever(playerinfo_t *playerinfo);
extern void G_PlayerActionPushLever(playerinfo_t *playerinfo);
extern qboolean G_HandleTeleport(playerinfo_t *playerinfo);
extern void G_SetJointAngles(playerinfo_t *playerinfo);
extern void G_ResetJointAngles(playerinfo_t *playerinfo);
extern void G_PlayerActionChickenBite(playerinfo_t *playerinfo);
extern void G_PlayerFallingDamage(playerinfo_t *playerinfo,float delta);
extern void G_PlayerActionSwordAttack(playerinfo_t *playerinfo,int value);
extern void G_PlayerActionSpellFireball(playerinfo_t *playerinfo);
extern void G_PlayerActionSpellBlast(playerinfo_t *playerinfo);
extern void G_PlayerActionSpellArray(playerinfo_t *playerinfo,int value);
extern void G_PlayerActionSpellSphereCreate(playerinfo_t *playerinfo,qboolean *Charging);
extern void G_PlayerActionSpellBigBall(playerinfo_t *playerinfo);
extern void G_PlayerActionSpellFirewall(playerinfo_t *playerinfo);
extern void G_PlayerActionRedRainBowAttack(playerinfo_t *playerinfo);
extern void G_PlayerActionPhoenixBowAttack(playerinfo_t *playerinfo);
extern void G_PlayerActionHellstaffAttack(playerinfo_t *playerinfo);
extern void G_PlayerActionSpellDefensive(playerinfo_t *playerinfo);
extern void G_PlayerActionChickenBite(playerinfo_t *playerinfo);
extern void G_PlayerSpellShieldAttack(playerinfo_t *playerinfo);
extern void G_PlayerSpellStopShieldAttack(playerinfo_t *playerinfo);
extern void G_PlayerVaultKick(playerinfo_t *playerinfo);
extern void G_PlayerActionCheckRopeMove(playerinfo_t *playerinfo);
extern qboolean G_EntIsAButton(edict_t *ent);
void Updatefmnodeinfo(edict_t *ent);
void Setfmnodeinfo(edict_t *ent);
#endif // _P_FUNCS_H_