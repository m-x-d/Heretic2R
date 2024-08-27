//
// p_dll.c
//
// Copyright 1998 Raven Software
//
// Heretic II
//

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include "player.h"
#include "g_local.h"
#include "qcommon.h"

extern void Sys_LoadGameDll(const char *name, HINSTANCE *hinst, DWORD *chkSum);
extern void Sys_UnloadGameDll(const char *name, HINSTANCE *hinst);

// Structure containing functions and data pointers exported from the player DLL.

player_export_t	playerExport;

// Handle to player DLL. 

static HINSTANCE player_library = NULL;

// ************************************************************************************************
// Define pointers to all the .dll functions which other code will dynamically link with.
// ************************************************************************************************

void (*P_Init)(void);
void (*P_Shutdown)(void);

void (*P_PlayerReleaseRope)(playerinfo_t *playerinfo);
void (*P_KnockDownPlayer)(playerinfo_t *playerinfo);
void (*P_PlayFly)(playerinfo_t *playerinfo, float dist);
void (*P_PlaySlap)(playerinfo_t *playerinfo, float dist);
void (*P_PlayScratch)(playerinfo_t *playerinfo, float dist);
void (*P_PlaySigh)(playerinfo_t *playerinfo, float dist);
void (*P_SpawnDustPuff)(playerinfo_t *playerinfo, float dist);
void (*P_PlayerInterruptAction)(playerinfo_t *playerinfo);

qboolean (*P_BranchCheckDismemberAction)(playerinfo_t *playerinfo, int weapon);

void (*P_TurnOffPlayerEffects)(playerinfo_t *playerinfo);
void (*P_AnimUpdateFrame)(playerinfo_t *playerinfo);
void (*P_PlayerFallingDamage)(playerinfo_t *playerinfo);

void (*P_PlayerBasicAnimReset)(playerinfo_t *playerinfo);
void (*P_PlayerAnimReset)(playerinfo_t *playerinfo);
void (*P_PlayerAnimSetLowerSeq)(playerinfo_t *playerinfo, int seq);
void (*P_PlayerAnimSetUpperSeq)(playerinfo_t *playerinfo, int seq);
void (*P_PlayerAnimUpperIdle)(playerinfo_t *playerinfo);
void (*P_PlayerAnimLowerIdle)(playerinfo_t *playerinfo);
void (*P_PlayerAnimUpperUpdate)(playerinfo_t *playerinfo);
void (*P_PlayerAnimLowerUpdate)(playerinfo_t *playerinfo);
void (*P_PlayerAnimSetVault)(playerinfo_t *playerinfo, int seq);
void (*P_PlayerPlayPain)(playerinfo_t *playerinfo, int type);

void (*P_PlayerIntLand)(playerinfo_t *playerinfo_t, float landspeed);

void (*P_PlayerInit)(playerinfo_t *playerinfo, int complete_reset);
void (*P_PlayerClearEffects)(playerinfo_t *playerinfo);
void (*P_PlayerUpdate)(playerinfo_t *playerinfo);
void (*P_PlayerUpdateCmdFlags)(playerinfo_t *playerinfo);
void (*P_PlayerUpdateModelAttributes)(playerinfo_t *playerinfo);

void (*P_Weapon_Ready)(playerinfo_t *playerinfo,gitem_t *Weapon);
void (*P_Weapon_EquipSpell)(playerinfo_t *playerinfo,gitem_t *Weapon);
void (*P_Weapon_EquipSwordStaff)(playerinfo_t *playerinfo,gitem_t *Weapon);
void (*P_Weapon_EquipHellStaff)(playerinfo_t *playerinfo,gitem_t *Weapon);
void (*P_Weapon_EquipBow)(playerinfo_t *playerinfo,gitem_t *Weapon);
void (*P_Weapon_EquipArmor)(playerinfo_t *playerinfo, gitem_t *Weapon);
int (*P_Weapon_CurrentShotsLeft)(playerinfo_t *playerinfo);
int (*P_Defence_CurrentShotsLeft)(playerinfo_t *playerinfo, int intent);

int (*P_GetItemIndex)(gitem_t* x);
gitem_t* (*P_GetItemByIndex)(int index);
gitem_t* (*P_FindItemByClassname)(char *classname);
gitem_t* (*P_FindItem)(char *pickupname);
void (*P_InitItems)(void);

// ************************************************************************************************
// P_Freelib
// ---------
// ************************************************************************************************

void P_Freelib()
{
	if(!player_library)
	{
		return;
	}

	P_Shutdown();

#if	_GAME_DLL
	gi.Sys_UnloadGameDll("Player", &player_library);
#else
	Sys_UnloadGameDll("Player", &player_library);
#endif

	player_library = NULL;
}

// ************************************************************************************************
// P_Load
// ------
// ************************************************************************************************

unsigned int P_Load(char *name)
{
	DWORD			playerdll_chksum;
	GetPlayerAPI_t	P_GetPlayerAPI;

	P_Freelib();

#if	_GAME_DLL
	Com_Printf("------- Loading %s -------\n", name);
	gi.Sys_LoadGameDll(name, &player_library, &playerdll_chksum);
#else
	Com_ColourPrintf(P_HEADER, "------- Loading %s -------\n", name);
	Sys_LoadGameDll(name, &player_library, &playerdll_chksum);
#endif

	if((P_GetPlayerAPI = (void *)GetProcAddress(player_library, "GetPlayerAPI")) == 0)
		Sys_Error ("GetProcAddress failed on GetPlayerAPI for library %s", name);

	if((P_Init = (void *)GetProcAddress(player_library, "P_Init")) == 0)
		Sys_Error ("GetProcAddress failed on P_Init for library %s", name);
	
	if((P_Shutdown = (void *)GetProcAddress(player_library, "P_Shutdown")) == 0)
		Sys_Error("GetProcAddress failed on P_Shutdown for library %s", name);

	if((P_PlayerReleaseRope = (void *)GetProcAddress(player_library, "PlayerReleaseRope")) == 0)
		Sys_Error ("GetProcAddress failed on P_PlayerReleaseRope for library %s", name);

	if((P_KnockDownPlayer = (void *)GetProcAddress(player_library, "KnockDownPlayer")) == 0)
		Sys_Error ("GetProcAddress failed on P_KnockDownPlayer for library %s", name);

	if((P_PlayFly = (void *)GetProcAddress(player_library, "PlayFly")) == 0)
		Sys_Error ("GetProcAddress failed on P_PlayFly for library %s", name);

	if((P_PlaySlap = (void *)GetProcAddress(player_library, "PlaySlap")) == 0)
		Sys_Error ("GetProcAddress failed on P_PlaySlap for library %s", name);

	if((P_PlayScratch = (void *)GetProcAddress(player_library, "PlayScratch")) == 0)
		Sys_Error ("GetProcAddress failed on P_PlayScratch for library %s", name);

	if((P_PlaySigh = (void *)GetProcAddress(player_library, "PlaySigh")) == 0)
		Sys_Error ("GetProcAddress failed on P_PlaySighfor library %s", name);

	if((P_SpawnDustPuff = (void *)GetProcAddress(player_library, "SpawnDustPuff")) == 0)
		Sys_Error ("GetProcAddress failed on P_SpawnDustPuff for library %s", name);

	if((P_PlayerInterruptAction = (void *)GetProcAddress(player_library, "PlayerInterruptAction")) == 0)
		Sys_Error ("GetProcAddress failed on P_PlayerInterruptAction for library %s", name);

	if((P_BranchCheckDismemberAction = (void *)GetProcAddress(player_library, "BranchCheckDismemberAction")) == 0)
		Sys_Error ("GetProcAddress failed on P_BranchCheckDismemberAction for library %s", name);

	if((P_TurnOffPlayerEffects = (void *)GetProcAddress(player_library, "TurnOffPlayerEffects")) == 0)
		Sys_Error ("GetProcAddress failed on P_TurnOffPlayerEffects for library %s", name);

	if((P_AnimUpdateFrame = (void *)GetProcAddress(player_library, "AnimUpdateFrame")) == 0)
		Sys_Error ("GetProcAddress failed on P_AnimUpdateFrame for library %s", name);

	if((P_PlayerFallingDamage = (void *)GetProcAddress(player_library, "PlayerFallingDamage")) == 0)
		Sys_Error ("GetProcAddress failed on P_PlayerFallingDamage for library %s", name);

	if((P_PlayerBasicAnimReset = (void *)GetProcAddress(player_library, "PlayerBasicAnimReset")) == 0)
		Sys_Error ("GetProcAddress failed on P_PlayerBasicAnimReset for library %s", name);

	if((P_PlayerAnimReset = (void *)GetProcAddress(player_library, "PlayerAnimReset")) == 0)
		Sys_Error ("GetProcAddress failed on P_PlayerAnimReset for library %s", name);

	if((P_PlayerAnimSetLowerSeq = (void *)GetProcAddress(player_library, "PlayerAnimSetLowerSeq")) == 0)
		Sys_Error ("GetProcAddress failed on P_PlayerAnimSetLowerSeq for library %s", name);

	if((P_PlayerAnimSetUpperSeq = (void *)GetProcAddress(player_library, "PlayerAnimSetUpperSeq")) == 0)
		Sys_Error ("GetProcAddress failed on P_PlayerAnimSetUpperSeq for library %s", name);

	if((P_PlayerAnimUpperIdle = (void *)GetProcAddress(player_library, "PlayerAnimUpperIdle")) == 0)
		Sys_Error ("GetProcAddress failed on P_PlayerAnimUpperIdle for library %s", name);

	if((P_PlayerAnimLowerIdle = (void *)GetProcAddress(player_library, "PlayerAnimLowerIdle")) == 0)
		Sys_Error ("GetProcAddress failed on P_PlayerAnimLowerIdle for library %s", name);

	if((P_PlayerAnimUpperUpdate = (void *)GetProcAddress(player_library, "PlayerAnimUpperUpdate")) == 0)
		Sys_Error ("GetProcAddress failed on P_PlayerAnimUpperUpdate for library %s", name);

	if((P_PlayerAnimLowerUpdate= (void *)GetProcAddress(player_library, "PlayerAnimLowerUpdate")) == 0)
		Sys_Error ("GetProcAddress failed on P_PlayerReleaseRope for library %s", name);

	if((P_PlayerAnimSetVault = (void *)GetProcAddress(player_library, "PlayerAnimSetVault")) == 0)
		Sys_Error ("GetProcAddress failed on P_PlayerAnimSetVault for library %s", name);

	if((P_PlayerPlayPain = (void *)GetProcAddress(player_library, "PlayerPlayPain")) == 0)
		Sys_Error ("GetProcAddress failed on P_PlayerPlayPain for library %s", name);

	if((P_PlayerIntLand = (void *)GetProcAddress(player_library, "PlayerIntLand")) == 0)
		Sys_Error ("GetProcAddress failed on P_PlayerIntLand for library %s", name);

	if((P_PlayerInit = (void *)GetProcAddress(player_library, "PlayerInit")) == 0)
		Sys_Error ("GetProcAddress failed on P_PlayerInit for library %s", name);

	if((P_PlayerClearEffects = (void *)GetProcAddress(player_library, "PlayerClearEffects")) == 0)
		Sys_Error ("GetProcAddress failed on P_PlayerClearEffects for library %s", name);

	if((P_PlayerUpdate = (void *)GetProcAddress(player_library, "PlayerUpdate")) == 0)
		Sys_Error ("GetProcAddress failed on P_PlayerUpdate for library %s", name);

	if((P_PlayerUpdateCmdFlags = (void *)GetProcAddress(player_library, "PlayerUpdateCmdFlags")) == 0)
		Sys_Error ("GetProcAddress failed on P_PlayerUpdateCmdFlags for library %s", name);

	if((P_PlayerUpdateModelAttributes = (void *)GetProcAddress(player_library, "PlayerUpdateModelAttributes")) == 0)
		Sys_Error ("GetProcAddress failed on P_PlayerUpdateModelAttributes for library %s", name);

	if((P_Weapon_Ready = (void *)GetProcAddress(player_library, "Weapon_Ready")) == 0)
		Sys_Error ("GetProcAddress failed on P_Weapon_Ready for library %s", name);

	if((P_Weapon_EquipSpell = (void *)GetProcAddress(player_library, "Weapon_EquipSpell")) == 0)
		Sys_Error ("GetProcAddress failed on P_Weapon_EquipSpell for library %s", name);

	if((P_Weapon_EquipSwordStaff = (void *)GetProcAddress(player_library, "Weapon_EquipSwordStaff")) == 0)
		Sys_Error ("GetProcAddress failed on P_Weapon_EquipSwordStaff for library %s", name);

	if((P_Weapon_EquipSwordStaff = (void *)GetProcAddress(player_library, "Weapon_EquipSwordStaff")) == 0)
		Sys_Error ("GetProcAddress failed on P_Weapon_EquipSwordStaff for library %s", name);

	if((P_Weapon_EquipHellStaff = (void *)GetProcAddress(player_library, "Weapon_EquipHellStaff")) == 0)
		Sys_Error ("GetProcAddress failed on P_Weapon_EquipHellStaff for library %s", name);

	if((P_Weapon_EquipBow = (void *)GetProcAddress(player_library, "Weapon_EquipBow")) == 0)
		Sys_Error ("GetProcAddress failed on P_Weapon_EquipBow for library %s", name);

	if((P_Weapon_EquipArmor = (void *)GetProcAddress(player_library, "Weapon_EquipArmor")) == 0)
		Sys_Error ("GetProcAddress failed on P_Weapon_EquipArmor for library %s", name);

	if((P_Weapon_CurrentShotsLeft = (void *)GetProcAddress(player_library, "Weapon_CurrentShotsLeft")) == 0)
		Sys_Error ("GetProcAddress failed on P_Weapon_CurrentShotsLeft for library %s", name);

	if((P_Defence_CurrentShotsLeft = (void *)GetProcAddress(player_library, "Defence_CurrentShotsLeft")) == 0)
		Sys_Error ("GetProcAddress failed on P_Defence_CurrentShotsLeft for library %s", name);

	if((P_GetItemIndex = (void *)GetProcAddress(player_library, "GetItemIndex")) == 0)
		Sys_Error ("GetProcAddress failed on P_GetItemIndex for library %s", name);

	if((P_GetItemByIndex = (void *)GetProcAddress(player_library, "GetItemByIndex")) == 0)
		Sys_Error ("GetProcAddress failed on P_GetItemByIndex for library %s", name);

	if((P_FindItemByClassname = (void *)GetProcAddress(player_library, "FindItemByClassname")) == 0)
		Sys_Error ("GetProcAddress failed on P_Defence_CurrentShotsLeft for library %s", name);

	if((P_FindItem = (void *)GetProcAddress(player_library, "FindItem")) == 0)
		Sys_Error ("GetProcAddress failed on P_FindItem for library %s", name);

	if((P_InitItems = (void *)GetProcAddress(player_library, "InitItems")) == 0)
		Sys_Error ("GetProcAddress failed on P_InitItems for library %s", name);

	P_Init();

	playerExport=P_GetPlayerAPI();

#if	_GAME_DLL
	Com_Printf("------------------------------------\n");
#else
	Com_ColourPrintf(P_HEADER, "------------------------------------\n");
#endif
	return(playerdll_chksum);
}

