//
// p_dll.c
//
// Copyright 1998 Raven Software
//

#include <windows.h>
#include "p_dll.h" //mxd
#ifdef GAME_DLL
	#include "g_local.h" //mxd
#else
	#include "dll_io.h" //mxd
#endif
#include "qcommon.h"

// Structure containing functions and data pointers exported from the player DLL.
player_export_t	playerExport;

// Handle to player DLL.
static HINSTANCE player_library = NULL;

// Define pointers to all the .dll functions which other code will dynamically link with.
void (*P_Init)(void);
void (*P_Shutdown)(void);

void (*P_PlayerReleaseRope)(playerinfo_t* playerinfo);
void (*P_KnockDownPlayer)(playerinfo_t* playerinfo);
void (*P_PlayFly)(playerinfo_t* playerinfo, float dist);
void (*P_PlaySlap)(playerinfo_t* playerinfo, float dist);
void (*P_PlayScratch)(playerinfo_t* playerinfo, float dist);
void (*P_PlaySigh)(playerinfo_t* playerinfo, float dist);
void (*P_SpawnDustPuff)(playerinfo_t* playerinfo, float dist);
void (*P_PlayerInterruptAction)(playerinfo_t* playerinfo);

qboolean (*P_BranchCheckDismemberAction)(playerinfo_t* playerinfo, int weapon);

void (*P_TurnOffPlayerEffects)(playerinfo_t* playerinfo);
void (*P_AnimUpdateFrame)(playerinfo_t* playerinfo);
void (*P_PlayerFallingDamage)(playerinfo_t* playerinfo);

void (*P_PlayerBasicAnimReset)(playerinfo_t* playerinfo);
void (*P_PlayerAnimReset)(playerinfo_t* playerinfo);
void (*P_PlayerAnimSetLowerSeq)(playerinfo_t* playerinfo, int seq);
void (*P_PlayerAnimSetUpperSeq)(playerinfo_t* playerinfo, int seq);
void (*P_PlayerAnimUpperIdle)(playerinfo_t* playerinfo);
void (*P_PlayerAnimLowerIdle)(playerinfo_t* playerinfo);
void (*P_PlayerAnimUpperUpdate)(playerinfo_t* playerinfo);
void (*P_PlayerAnimLowerUpdate)(playerinfo_t* playerinfo);
void (*P_PlayerAnimSetVault)(playerinfo_t* playerinfo, int seq);
void (*P_PlayerPlayPain)(playerinfo_t* playerinfo, int type);

void (*P_PlayerIntLand)(playerinfo_t* playerinfo, float landspeed);

void (*P_PlayerInit)(playerinfo_t* playerinfo, int complete_reset);
void (*P_PlayerClearEffects)(const playerinfo_t* playerinfo);
void (*P_PlayerUpdate)(playerinfo_t* playerinfo);
void (*P_PlayerUpdateCmdFlags)(playerinfo_t* playerinfo);
void (*P_PlayerUpdateModelAttributes)(playerinfo_t* playerinfo);

void (*P_Weapon_Ready)(playerinfo_t* playerinfo, gitem_t* Weapon);
void (*P_Weapon_EquipSpell)(playerinfo_t* playerinfo, gitem_t* Weapon);
void (*P_Weapon_EquipSwordStaff)(playerinfo_t* playerinfo, gitem_t* Weapon);
void (*P_Weapon_EquipHellStaff)(playerinfo_t* playerinfo, gitem_t* Weapon);
void (*P_Weapon_EquipBow)(playerinfo_t* playerinfo, gitem_t* Weapon);
void (*P_Weapon_EquipArmor)(playerinfo_t* playerinfo, gitem_t* Weapon);
int (*P_Weapon_CurrentShotsLeft)(playerinfo_t* playerinfo);
int (*P_Defence_CurrentShotsLeft)(playerinfo_t* playerinfo, int intent);

int (*P_GetItemIndex)(const gitem_t* item);
gitem_t* (*P_GetItemByIndex)(int index);
gitem_t* (*P_FindItemByClassname)(const char* classname);
gitem_t* (*P_FindItem)(const char* pickup_name);
void (*P_InitItems)(void);

void P_Freelib(void)
{
	if (player_library != NULL)
	{
		P_Shutdown();

#ifdef GAME_DLL
		gi.Sys_UnloadGameDll("Player", &player_library);
#else
		Sys_UnloadGameDll("Player", &player_library);
#endif

		player_library = NULL;
	}
}

uint P_Load(char *name)
{
	DWORD playerdll_chksum;

	P_Freelib();

#ifdef GAME_DLL
	Com_Printf("---------- Loading %s ----------\n", name);
	gi.Sys_LoadGameDll(name, &player_library, &playerdll_chksum);
#else
	Com_ColourPrintf(P_HEADER, "---------- Loading %s ----------\n", name);
	Sys_LoadGameDll(name, &player_library, &playerdll_chksum);
#endif

	const GetPlayerAPI_t P_GetPlayerAPI = (GetPlayerAPI_t)GetProcAddress(player_library, "GetPlayerAPI");
	if (P_GetPlayerAPI == NULL)
		Sys_Error("GetProcAddress failed on GetPlayerAPI for library %s", name);

	P_Init = (void (*)(void))GetProcAddress(player_library, "P_Init");
	if (P_Init == NULL)
		Sys_Error("GetProcAddress failed on P_Init for library %s", name);

	P_Shutdown = (void (*)(void))GetProcAddress(player_library, "P_Shutdown");
	if (P_Shutdown == NULL)
		Sys_Error("GetProcAddress failed on P_Shutdown for library %s", name);

	P_PlayerReleaseRope = (void (*)(playerinfo_t*))GetProcAddress(player_library, "PlayerReleaseRope"); //TODO: unused?
	if (P_PlayerReleaseRope == NULL)
		Sys_Error("GetProcAddress failed on P_PlayerReleaseRope for library %s", name);

	P_KnockDownPlayer = (void (*)(playerinfo_t*))GetProcAddress(player_library, "KnockDownPlayer");
	if (P_KnockDownPlayer == NULL)
		Sys_Error("GetProcAddress failed on P_KnockDownPlayer for library %s", name);

	P_PlayFly = (void (*)(playerinfo_t*, float))GetProcAddress(player_library, "PlayFly"); //TODO: unused?
	if (P_PlayFly == NULL)
		Sys_Error("GetProcAddress failed on P_PlayFly for library %s", name);

	P_PlaySlap = (void (*)(playerinfo_t*, float))GetProcAddress(player_library, "PlaySlap"); //TODO: unused?
	if (P_PlaySlap == NULL)
		Sys_Error("GetProcAddress failed on P_PlaySlap for library %s", name);

	P_PlayScratch = (void (*)(playerinfo_t*, float))GetProcAddress(player_library, "PlayScratch"); //TODO: unused?
	if (P_PlayScratch == NULL)
		Sys_Error("GetProcAddress failed on P_PlayScratch for library %s", name);

	P_PlaySigh = (void (*)(playerinfo_t*, float))GetProcAddress(player_library, "PlaySigh"); //TODO: unused?
	if (P_PlaySigh == NULL)
		Sys_Error("GetProcAddress failed on P_PlaySigh for library %s", name);

	P_SpawnDustPuff = (void (*)(playerinfo_t*, float))GetProcAddress(player_library, "SpawnDustPuff"); //TODO: unused?
	if (P_SpawnDustPuff == NULL)
		Sys_Error("GetProcAddress failed on P_SpawnDustPuff for library %s", name);

	P_PlayerInterruptAction = (void (*)(playerinfo_t*))GetProcAddress(player_library, "PlayerInterruptAction");
	if (P_PlayerInterruptAction == NULL)
		Sys_Error("GetProcAddress failed on P_PlayerInterruptAction for library %s", name);

	P_BranchCheckDismemberAction = (qboolean (*)(playerinfo_t*, int))GetProcAddress(player_library, "BranchCheckDismemberAction");
	if (P_BranchCheckDismemberAction == NULL)
		Sys_Error("GetProcAddress failed on P_BranchCheckDismemberAction for library %s", name);

	P_TurnOffPlayerEffects = (void (*)(playerinfo_t*))GetProcAddress(player_library, "TurnOffPlayerEffects");
	if (P_TurnOffPlayerEffects == NULL)
		Sys_Error("GetProcAddress failed on P_TurnOffPlayerEffects for library %s", name);

	P_AnimUpdateFrame = (void (*)(playerinfo_t*))GetProcAddress(player_library, "AnimUpdateFrame");
	if (P_AnimUpdateFrame == NULL)
		Sys_Error("GetProcAddress failed on P_AnimUpdateFrame for library %s", name);

	P_PlayerFallingDamage = (void (*)(playerinfo_t*))GetProcAddress(player_library, "PlayerFallingDamage");
	if (P_PlayerFallingDamage == NULL)
		Sys_Error("GetProcAddress failed on P_PlayerFallingDamage for library %s", name);

	P_PlayerBasicAnimReset = (void (*)(playerinfo_t*))GetProcAddress(player_library, "PlayerBasicAnimReset");
	if (P_PlayerBasicAnimReset == NULL)
		Sys_Error("GetProcAddress failed on P_PlayerBasicAnimReset for library %s", name);

	P_PlayerAnimReset = (void (*)(playerinfo_t*))GetProcAddress(player_library, "PlayerAnimReset");
	if (P_PlayerAnimReset == NULL)
		Sys_Error("GetProcAddress failed on P_PlayerAnimReset for library %s", name);

	P_PlayerAnimSetLowerSeq = (void (*)(playerinfo_t*, int))GetProcAddress(player_library, "PlayerAnimSetLowerSeq");
	if (P_PlayerAnimSetLowerSeq == NULL)
		Sys_Error("GetProcAddress failed on P_PlayerAnimSetLowerSeq for library %s", name);

	P_PlayerAnimSetUpperSeq = (void (*)(playerinfo_t*, int))GetProcAddress(player_library, "PlayerAnimSetUpperSeq");
	if (P_PlayerAnimSetUpperSeq == NULL)
		Sys_Error("GetProcAddress failed on P_PlayerAnimSetUpperSeq for library %s", name);

	P_PlayerAnimUpperIdle = (void (*)(playerinfo_t*))GetProcAddress(player_library, "PlayerAnimUpperIdle"); //TODO: unused?
	if (P_PlayerAnimUpperIdle == NULL)
		Sys_Error("GetProcAddress failed on P_PlayerAnimUpperIdle for library %s", name);

	P_PlayerAnimLowerIdle = (void (*)(playerinfo_t*))GetProcAddress(player_library, "PlayerAnimLowerIdle"); //TODO: unused?
	if (P_PlayerAnimLowerIdle == NULL)
		Sys_Error("GetProcAddress failed on P_PlayerAnimLowerIdle for library %s", name);

	P_PlayerAnimUpperUpdate = (void (*)(playerinfo_t*))GetProcAddress(player_library, "PlayerAnimUpperUpdate"); //TODO: unused?
	if (P_PlayerAnimUpperUpdate == NULL)
		Sys_Error("GetProcAddress failed on P_PlayerAnimUpperUpdate for library %s", name);

	P_PlayerAnimLowerUpdate = (void (*)(playerinfo_t*))GetProcAddress(player_library, "PlayerAnimLowerUpdate"); //TODO: unused?
	if (P_PlayerAnimLowerUpdate == NULL)
		Sys_Error("GetProcAddress failed on P_PlayerReleaseRope for library %s", name);

	P_PlayerAnimSetVault = (void (*)(playerinfo_t*, int))GetProcAddress(player_library, "PlayerAnimSetVault"); //TODO: unused?
	if (P_PlayerAnimSetVault == NULL)
		Sys_Error("GetProcAddress failed on P_PlayerAnimSetVault for library %s", name);

	P_PlayerPlayPain = (void (*)(playerinfo_t*, int))GetProcAddress(player_library, "PlayerPlayPain");
	if (P_PlayerPlayPain == NULL)
		Sys_Error("GetProcAddress failed on P_PlayerPlayPain for library %s", name);

	P_PlayerIntLand = (void (*)(playerinfo_t*, float))GetProcAddress(player_library, "PlayerIntLand"); //TODO: unused?
	if (P_PlayerIntLand == NULL)
		Sys_Error("GetProcAddress failed on P_PlayerIntLand for library %s", name);

	P_PlayerInit = (void (*)(playerinfo_t*, int))GetProcAddress(player_library, "PlayerInit");
	if (P_PlayerInit == NULL)
		Sys_Error("GetProcAddress failed on P_PlayerInit for library %s", name);

	P_PlayerClearEffects = (void (*)(const playerinfo_t*))GetProcAddress(player_library, "PlayerClearEffects"); //TODO: unused?
	if (P_PlayerClearEffects == NULL)
		Sys_Error("GetProcAddress failed on P_PlayerClearEffects for library %s", name);

	P_PlayerUpdate = (void (*)(playerinfo_t*))GetProcAddress(player_library, "PlayerUpdate");
	if (P_PlayerUpdate == NULL)
		Sys_Error("GetProcAddress failed on P_PlayerUpdate for library %s", name);

	P_PlayerUpdateCmdFlags = (void (*)(playerinfo_t*))GetProcAddress(player_library, "PlayerUpdateCmdFlags");
	if (P_PlayerUpdateCmdFlags == NULL)
		Sys_Error("GetProcAddress failed on P_PlayerUpdateCmdFlags for library %s", name);

	P_PlayerUpdateModelAttributes = (void (*)(playerinfo_t*))GetProcAddress(player_library, "PlayerUpdateModelAttributes");
	if (P_PlayerUpdateModelAttributes == NULL)
		Sys_Error("GetProcAddress failed on P_PlayerUpdateModelAttributes for library %s", name);

	P_Weapon_Ready = (void (*)(playerinfo_t*, gitem_t*))GetProcAddress(player_library, "Weapon_Ready"); //TODO: unused?
	if (P_Weapon_Ready == NULL)
		Sys_Error("GetProcAddress failed on P_Weapon_Ready for library %s", name);

	P_Weapon_EquipSpell = (void (*)(playerinfo_t*, gitem_t*))GetProcAddress(player_library, "Weapon_EquipSpell");
	if (P_Weapon_EquipSpell == NULL)
		Sys_Error("GetProcAddress failed on P_Weapon_EquipSpell for library %s", name);

	P_Weapon_EquipSwordStaff = (void (*)(playerinfo_t*, gitem_t*))GetProcAddress(player_library, "Weapon_EquipSwordStaff");
	if (P_Weapon_EquipSwordStaff == NULL)
		Sys_Error("GetProcAddress failed on P_Weapon_EquipSwordStaff for library %s", name);

	P_Weapon_EquipHellStaff = (void (*)(playerinfo_t*, gitem_t*))GetProcAddress(player_library, "Weapon_EquipHellStaff");
	if (P_Weapon_EquipHellStaff == NULL)
		Sys_Error("GetProcAddress failed on P_Weapon_EquipHellStaff for library %s", name);

	P_Weapon_EquipBow = (void (*)(playerinfo_t*, gitem_t*))GetProcAddress(player_library, "Weapon_EquipBow");
	if (P_Weapon_EquipBow == NULL)
		Sys_Error("GetProcAddress failed on P_Weapon_EquipBow for library %s", name);

	P_Weapon_EquipArmor = (void (*)(playerinfo_t*, gitem_t*))GetProcAddress(player_library, "Weapon_EquipArmor"); //TODO: unused?
	if (P_Weapon_EquipArmor == NULL)
		Sys_Error("GetProcAddress failed on P_Weapon_EquipArmor for library %s", name);

	P_Weapon_CurrentShotsLeft = (int (*)(playerinfo_t*))GetProcAddress(player_library, "Weapon_CurrentShotsLeft"); //TODO: unused?
	if (P_Weapon_CurrentShotsLeft == NULL)
		Sys_Error("GetProcAddress failed on P_Weapon_CurrentShotsLeft for library %s", name);

	P_Defence_CurrentShotsLeft = (int (*)(playerinfo_t*, int))GetProcAddress(player_library, "Defence_CurrentShotsLeft");
	if (P_Defence_CurrentShotsLeft == NULL)
		Sys_Error("GetProcAddress failed on P_Defence_CurrentShotsLeft for library %s", name);

	P_GetItemIndex = (int (*)(const gitem_t*))GetProcAddress(player_library, "GetItemIndex");
	if (P_GetItemIndex == NULL)
		Sys_Error("GetProcAddress failed on P_GetItemIndex for library %s", name);

	P_GetItemByIndex = (gitem_t* (*)(int))GetProcAddress(player_library, "GetItemByIndex");
	if (P_GetItemByIndex == NULL)
		Sys_Error("GetProcAddress failed on P_GetItemByIndex for library %s", name);

	P_FindItemByClassname = (gitem_t* (*)(const char*))GetProcAddress(player_library, "FindItemByClassname");
	if (P_FindItemByClassname == NULL)
		Sys_Error("GetProcAddress failed on P_Defence_CurrentShotsLeft for library %s", name);

	P_FindItem = (gitem_t* (*)(const char*))GetProcAddress(player_library, "FindItem");
	if (P_FindItem == NULL)
		Sys_Error("GetProcAddress failed on P_FindItem for library %s", name);

	P_InitItems = (void (*)(void))GetProcAddress(player_library, "InitItems");
	if (P_InitItems == NULL)
		Sys_Error("GetProcAddress failed on P_InitItems for library %s", name);

	P_Init();

	playerExport = P_GetPlayerAPI();

#ifdef GAME_DLL
	Com_Printf("------------------------------------\n");
#else
	Com_ColourPrintf(P_HEADER, "------------------------------------\n");
#endif

	return playerdll_chksum;
}