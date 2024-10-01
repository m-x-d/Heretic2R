//
// sv_ccmds.c
//
// Copyright 1998 Raven Software
//

#include "server.h"

#pragma region ========================== SAVEGAME FILES ==========================

// Delete save/<XXX>/
//mxd. Same as Q2 version, except FS_Gamedir() -> FS_Userdir() change.
void SV_WipeSavegame(char* savename)
{
	char name[MAX_OSPATH];

	Com_sprintf(name, sizeof(name), "%s/save/%s/server.ssv", FS_Userdir(), savename);
	remove(name);

	Com_sprintf(name, sizeof(name), "%s/save/%s/game.ssv", FS_Userdir(), savename);
	remove(name);

	Com_sprintf(name, sizeof(name), "%s/save/%s/*.sav", FS_Userdir(), savename);
	const char* s = Sys_FindFirst(name, 0, 0);
	while (s != NULL)
	{
		remove(s);
		s = Sys_FindNext(0, 0);
	}
	Sys_FindClose();

	Com_sprintf(name, sizeof(name), "%s/save/%s/*.sv2", FS_Userdir(), savename);
	s = Sys_FindFirst(name, 0, 0);
	while (s != NULL)
	{
		remove(s);
		s = Sys_FindNext(0, 0);
	}
	Sys_FindClose();
}

static void SV_CopySaveGame(char* src, char* dst)
{
	NOT_IMPLEMENTED
}

static void SV_WriteLevelFile(void)
{
	NOT_IMPLEMENTED
}

void SV_ReadLevelFile(void)
{
	NOT_IMPLEMENTED
}

static void SV_WriteServerFile(qboolean autosave)
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

// Saves the state of the map just being exited and goes to a new map.
// If the initial character of the map string is '*', the next map is in a new unit,
// so the current savegame directory is cleared of map files.
// Example: *inter.cin+jail
// Clears the archived maps, plays the inter.cin cinematic, then goes to map jail.bsp.
static void SV_GameMap_f(void)
{
	int i;
	client_t* cl;
	char buffer[MAX_QPATH];
	char savepath[MAX_OSPATH];

	const int len = (int)strlen(Cmd_Argv(1)); // H2
	if (len > 4)
	{
		const char* arg = Cmd_Argv(1);
		if (Q_stricmp(&arg[len - 4], ".hd2") == 0)
		{
			Com_Printf("Use demomap to play back demos\n");
			return;
		}
	}

	svs.have_current_save = false; // H2

	if (Cmd_Argc() != 2)
	{
		Com_Printf("USAGE: gamemap <map>\n");
		return;
	}

	sv_cooptimeout = NULL; // H2

	if ((int)sv_cinematicfreeze->value)
	{
		Com_Printf("Cannot use 'gamemap' during a cinematic.\n");
		return;
	}

	FS_CreatePath(va("%s/save/current/", FS_Userdir()));

	// Check for clearing the current savegame
	const char* map = Cmd_Argv(1);
	if (map[0] == '*')
	{
		// Wipe all the *.sav files
		SV_WipeSavegame("current");
	}
	else if (sv.state == ss_game) // Save the map just exited
	{
		// H2: check if map savegame exists
		strcpy_s(buffer, sizeof(buffer), map);
		char* c = strstr(buffer, "$"); // Strip spawnpoint marker
		if (c != NULL)
			*c = 0;

		if (buffer[0] == '*')
			strcpy_s(buffer, sizeof(buffer), buffer + 1);

		Com_sprintf(savepath, sizeof(savepath), "%s/save/current/%s.sav", FS_Userdir(), buffer);

		FILE* f;
		if (fopen_s(&f, savepath, "rb") == 0)
		{
			svs.have_current_save = true;
			fclose(f);
		}

		// Clear all the client inuse flags before saving so that when the level is re-entered,
		// the clients will spawn at spawn points instead of occupying body shells.
		qboolean* savedInuse = malloc((int)maxclients->value * sizeof(qboolean));
		for (i = 0, cl = svs.clients; i < (int)maxclients->value; i++, cl++)
		{
			savedInuse[i] = cl->edict->inuse;
			cl->edict->inuse = false;
		}

		SV_WriteLevelFile();

		// We must restore these for clients to transfer over correctly.
		for (i = 0, cl = svs.clients; i < (int)maxclients->value; i++, cl++)
			cl->edict->inuse = savedInuse[i];

		free(savedInuse);
	}

	if (sv.state == ss_game) // H2
		SV_WriteServerFile(true);

	// Start up the next map
	SV_Map(false, Cmd_Argv(1), svs.have_current_save);

	// Archive server state
	strncpy_s(svs.mapcmd, sizeof(svs.mapcmd), Cmd_Argv(1), sizeof(svs.mapcmd) - 1); //mxd. strncpy -> strncpy_s

	// Copy off the level to the autosave slot
	if (!(int)dedicated->value && sv.state == ss_game) // H2: extra sv.state check
	{
		SV_WriteServerFile(true);
		SV_CopySaveGame("current", "save0");
	}

	SCR_BeginLoadingPlaque(); // H2
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

// Q2 counterpart
// Kick everyone off, possibly in preparation for a new game.
static void SV_KillServer_f(void)
{
	if (svs.initialized)
	{
		SV_Shutdown("Server was killed.\n", false);
		NET_Config(false); // Close network sockets.
	}
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
