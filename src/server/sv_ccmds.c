//
// sv_ccmds.c
//
// Copyright 1998 Raven Software
//

#include "server.h"

#pragma region ========================== SAVEGAME FILES ==========================

static void SV_WipeSavegame(char* savename)
{
	NOT_IMPLEMENTED
}

#pragma endregion

#pragma region ========================== OPERATOR CONSOLE ONLY COMMANDS ==========================
// These commands can only be entered from stdin or by a remote operator datagram

static void SV_Heartbeat_f(void)
{
	NOT_IMPLEMENTED
}

static void SV_Kick_f(void)
{
	NOT_IMPLEMENTED
}

static void SV_Status_f(void)
{
	NOT_IMPLEMENTED
}

static void SV_Serverinfo_f(void)
{
	NOT_IMPLEMENTED
}

static void SV_DumpUser_f(void)
{
	NOT_IMPLEMENTED
}

static void SV_GameMap_f(void)
{
	NOT_IMPLEMENTED
}

// Goes directly to a given map without any savegame archiving. For development work.
static void SV_Map_f(void)
{
	if (Cmd_Argc() == 2)
	{
		sv.state = ss_dead; // Don't save current level when changing
		SV_WipeSavegame("current");
		SV_GameMap_f();
	}
	else
	{
		Com_Printf("USAGE: map <map>\n");
	}
}

static void SV_DemoMap_f(void)
{
	NOT_IMPLEMENTED
}

static void SV_SetMaster_f(void)
{
	NOT_IMPLEMENTED
}

static void SV_ConSay_f(void)
{
	NOT_IMPLEMENTED
}

static void SV_ListDMFlags_f(void)
{
	NOT_IMPLEMENTED
}

static void SV_SetDMFlags_f(void)
{
	NOT_IMPLEMENTED
}

static void SV_UnsetDMFlags_f(void)
{
	NOT_IMPLEMENTED
}

static void SV_Savegame_f(void)
{
	NOT_IMPLEMENTED
}

static void SV_Loadgame_f(void)
{
	NOT_IMPLEMENTED
}

static void SV_KillServer_f(void)
{
	NOT_IMPLEMENTED
}

static void SV_ServerCommand_f(void)
{
	NOT_IMPLEMENTED
}

#pragma endregion

void SV_InitOperatorCommands(void)
{
	Cmd_AddCommand("heartbeat", SV_Heartbeat_f);
	Cmd_AddCommand("kick", SV_Kick_f);
	Cmd_AddCommand("status", SV_Status_f);
	Cmd_AddCommand("serverinfo", SV_Serverinfo_f);
	Cmd_AddCommand("dumpuser", SV_DumpUser_f);

	Cmd_AddCommand("map", SV_Map_f);
	Cmd_AddCommand("demomap", SV_DemoMap_f);
	Cmd_AddCommand("gamemap", SV_GameMap_f);
	Cmd_AddCommand("setmaster", SV_SetMaster_f);
	//Cmd_AddCommand("setgamespymaster", SV_SetGamespyMaster_f); //mxd. Skip Gamespy logic...

	if ((int)dedicated->value)
		Cmd_AddCommand("say", SV_ConSay_f);

	// New in H2:
	Cmd_AddCommand("list_dmflags", SV_ListDMFlags_f);
	Cmd_AddCommand("set_dmflags", SV_SetDMFlags_f);
	Cmd_AddCommand("unset_dmflags", SV_UnsetDMFlags_f);

	Cmd_AddCommand("save", SV_Savegame_f);
	Cmd_AddCommand("load", SV_Loadgame_f);

	Cmd_AddCommand("killserver", SV_KillServer_f);
	Cmd_AddCommand("sv", SV_ServerCommand_f);
}
