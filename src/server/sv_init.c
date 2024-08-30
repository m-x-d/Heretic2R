//
// sv_init.c
//
// Copyright 1998 Raven Software
//

#include "server.h"
#include "client.h"

server_static_t svs; // Persistent server info
server_t sv; // Local server

static void SV_SpawnServer(char* server, char* spawnpoint, server_state_t serverstate, qboolean attractloop, qboolean loadgame)
{
	NOT_IMPLEMENTED
}

static void SV_InitGame(void)
{
	NOT_IMPLEMENTED
}

static void SetNextserver(char* levelstring)
{
	NOT_IMPLEMENTED
}

qboolean SV_ValidateMapFilename(char* level)
{
	NOT_IMPLEMENTED
	return false;
}

// The full syntax is:
//	map [*]<map>$<startspot>+<nextserver>
// Map can also be a .smk or .hd2 file.
// Nextserver is used to allow a cinematic to play, then proceed to another level:
//	map tram.cin+jail_e3
void SV_Map(const qboolean attractloop, const char* levelstring, const qboolean loadgame)
{
	char level[MAX_QPATH];
	char spawnpoint[MAX_QPATH];

	sv.loadgame = loadgame;
	sv.attractloop = attractloop;

	if (sv.state == ss_dead && !sv.loadgame)
		SV_InitGame(); // The game is just starting

	strcpy_s(level, sizeof(level), levelstring); //mxd. strcpy -> strcpy_s

	SetNextserver(level); // H2

	// If there is a '$', use the remainder as a spawnpoint.
	char* ch = strstr(level, "$");
	if (ch != NULL)
	{
		*ch = 0;
		strcpy_s(spawnpoint, sizeof(spawnpoint), ch + 1); //mxd. strcpy -> strcpy_s
	}
	else
	{
		spawnpoint[0] = 0;
	}

	// Skip the end-of-unit flag if necessary
	if (level[0] == '*')
		strcpy_s(level, sizeof(level), level + 1); //mxd. strcpy -> strcpy_s

	// H2: validate map name
	if (!SV_ValidateMapFilename(level))
		Com_Error(ERR_DROP, "Trying to load Map (%s) which doesn't exist.", level);

	Com_DPrintf("Map validated ok.\n");

	// Spawn server
	SCR_BeginLoadingPlaque(); // For local system
	SV_BroadcastCommand("changing\n");

	const int len = (int)strlen(level);
	if (len > 4 && strcmp(level + len - 4, ".smk") == 0)
	{
		SV_SpawnServer(level, spawnpoint, ss_cinematic, attractloop, loadgame);
	}
	else if (len > 4 && strcmp(level + len - 4, ".hd2") == 0)
	{
		SV_SpawnServer(level, spawnpoint, ss_demo, attractloop, loadgame);
	}
	else
	{
		SV_SendClientMessages(false);
		SV_SpawnServer(level, spawnpoint, ss_game, attractloop, loadgame);
		Cbuf_CopyToDefer();
	}

	SV_BroadcastCommand("reconnect\n");
	CL_CleanScreenShake(); // H2
}