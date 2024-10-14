//
// cl_player.c -- Client playerinfo_t handling logic.
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "cl_effects.h"
#include "cl_skeletons.h"
#include "Vector.h"

static void CL_Sound(byte EventId, vec3_t origin, int channel, char* soundname, float fvol, int attenuation, float timeofs)
{
	NOT_IMPLEMENTED
}

static int CL_Irand(const playerinfo_t* playerinfo, const int min, const int max)
{
	NOT_IMPLEMENTED
	return 0;
}

//TODO: are these needed?..
static void EmptyPlayerAction(playerinfo_t* playerinfo) { }
static void EmptyPlayerAction2(playerinfo_t* playerinfo, int val) { }
static void EmptyPlayerAction3(playerinfo_t* playerinfo, qboolean* val) { }

void CL_ResetPlayerInfo(void)
{
	cl.playerinfo.isclient = true;
	cl.playerinfo.Highestleveltime = -10000000000000000000000000000000.0f;
	cl.playerinfo.CL_Sound = CL_Sound;
	cl.playerinfo.CL_Trace = CL_Trace;
	cl.playerinfo.CL_CreateEffect = CL_CreateEffect;
	cl.playerinfo.CL_RemoveEffects = CL_RemoveEffects;
	cl.playerinfo.G_Sound = NULL;
	cl.playerinfo.G_L_Sound = NULL;
	cl.playerinfo.G_Trace = NULL;
	cl.playerinfo.G_CreateEffect = NULL;
	cl.playerinfo.G_RemoveEffects = NULL;
	cl.playerinfo.G_SoundIndex = NULL;
	cl.playerinfo.G_SoundRemove = NULL;
	cl.playerinfo.G_UseTargets = NULL;
	cl.playerinfo.G_GetEntityStatePtr = NULL;
	cl.playerinfo.G_BranchLwrClimbing = NULL;
	cl.playerinfo.G_PlayerActionCheckRopeGrab = NULL;
	cl.playerinfo.G_PlayerClimbingMoveFunc = NULL;
	cl.playerinfo.G_PlayerActionCheckPuzzleGrab = NULL;
	cl.playerinfo.G_PlayerActionTakePuzzle = NULL;
	cl.playerinfo.G_PlayerActionUsePuzzle = NULL;
	cl.playerinfo.G_PlayerActionCheckPushPull_Ent = NULL;
	cl.playerinfo.G_PlayerActionMoveItem = NULL;
	cl.playerinfo.G_PlayerActionCheckPushButton = NULL;
	cl.playerinfo.G_PlayerActionPushButton = NULL;
	cl.playerinfo.G_PlayerActionCheckPushLever = NULL;
	cl.playerinfo.G_HandleTeleport = NULL;
	cl.playerinfo.G_PlayerActionShrineEffect = NULL;
	cl.playerinfo.G_PlayerActionChickenBite = NULL;
	cl.playerinfo.G_PlayerSpellShieldAttack = NULL;
	cl.playerinfo.G_PlayerSpellStopShieldAttack = NULL;
	cl.playerinfo.G_PlayerVaultKick = NULL;
	cl.playerinfo.G_PlayerActionCheckRopeMove = NULL;
	cl.playerinfo.G_gamemsg_centerprintf = NULL;
	cl.playerinfo.G_levelmsg_centerprintf = NULL;
	cl.playerinfo.G_WeapNext = NULL;
	cl.playerinfo.G_UseItem = NULL;
	cl.playerinfo.G_EntIsAButton = NULL;
	cl.playerinfo.PointContents = CL_PMpointcontents;
	cl.playerinfo.SetJointAngles = SK_SetJointAngles;
	cl.playerinfo.ResetJointAngles = SK_ResetJointAngles;
	cl.playerinfo.PlayerActionSwordAttack = EmptyPlayerAction2;
	cl.playerinfo.PlayerActionSpellFireball = EmptyPlayerAction;
	cl.playerinfo.PlayerActionSpellBlast = EmptyPlayerAction;
	cl.playerinfo.PlayerActionSpellArray = EmptyPlayerAction2;
	cl.playerinfo.PlayerActionSpellSphereCreate = EmptyPlayerAction3;
	cl.playerinfo.PlayerActionSpellFirewall = EmptyPlayerAction;
	cl.playerinfo.PlayerActionSpellBigBall = EmptyPlayerAction;
	cl.playerinfo.PlayerActionRedRainBowAttack = EmptyPlayerAction;
	cl.playerinfo.PlayerActionPhoenixBowAttack = EmptyPlayerAction;
	cl.playerinfo.PlayerActionHellstaffAttack = EmptyPlayerAction;
	cl.playerinfo.PlayerActionSpellDefensive = EmptyPlayerAction;
	cl.playerinfo.irand = CL_Irand;

	P_InitItems();

	VectorClear(cl.prediction_error);
	memset(cl.predicted_origins, 0, sizeof(cl.predicted_origins));
}
