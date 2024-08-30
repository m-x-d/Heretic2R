//
// sv_init.c
//
// Copyright 1998 Raven Software
//

#include "server.h"

server_static_t svs; // Persistent server info
server_t sv; // Local server

static void SV_SpawnServer(char* server, char* spawnpoint, server_state_t serverstate, qboolean attractloop, qboolean loadgame)
{
	NOT_IMPLEMENTED
}

// A brand new game has been started
static void SV_InitGame(void)
{
	if (svs.initialized)
		SV_Shutdown("Server restarted\n", true); // Cause any connected clients to reconnect
	else
		CL_Drop(); // Make sure the client is down

	// Get any latched variable changes (maxclients, etc).
	Cvar_GetLatchedVars();

	svs.initialized = true;

	if ((int)Cvar_VariableValue("coop") && (int)Cvar_VariableValue("deathmatch"))
	{
		Com_Printf("Deathmatch and Coop both set, disabling Coop\n");
		Cvar_FullSet("coop", "0", CVAR_SERVERINFO | CVAR_LATCH);
	}

	// Dedicated servers are can't be single player and are usually DM
	// so unless they explicitly set coop, force them to deathmatch.
	if ((int)dedicated->value && !(int)Cvar_VariableValue("coop"))
		Cvar_FullSet("deathmatch", "1", CVAR_SERVERINFO | CVAR_LATCH);

	// Init clients
	if ((int)Cvar_VariableValue("deathmatch"))
	{
		if (maxclients->value <= 1)
			Cvar_FullSet("maxclients", "8", CVAR_SERVERINFO | CVAR_LATCH);
		else if (maxclients->value > MAX_CLIENTS)
			Cvar_FullSet("maxclients", va("%i", MAX_CLIENTS), CVAR_SERVERINFO | CVAR_LATCH);
	}
	else if ((int)Cvar_VariableValue("coop"))
	{
		if (maxclients->value <= 1 || maxclients->value > 4)
			Cvar_FullSet("maxclients", "4", CVAR_SERVERINFO | CVAR_LATCH);
	}
	else // Non-deathmatch, non-coop is one player.
	{
		Cvar_FullSet("maxclients", "1", CVAR_SERVERINFO | CVAR_LATCH);
	}

	svs.spawncount = rand();
	svs.clients = Z_Malloc((int)maxclients->value * (int)sizeof(client_t));
	svs.num_client_entities = (int)maxclients->value * UPDATE_BACKUP * 64;
	svs.client_entities = Z_Malloc(svs.num_client_entities * (int)sizeof(entity_state_t));

	// Init network stuff
	NET_Config(maxclients->value > 1);

	// Heartbeats will always be sent to the id master
	svs.last_heartbeat = -99999; // Send immediately

	// Init game
	SV_InitGameProgs();

	for (int i = 0; i < (int)maxclients->value; i++)
	{
		edict_t* ent = EDICT_NUM(i + 1);
		ent->s.number = (short)(i + 1);
		svs.clients[i].edict = ent;
		memset(&svs.clients[i].lastcmd, 0, sizeof(svs.clients[i].lastcmd));
	}
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