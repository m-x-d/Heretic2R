//
// sv_init.c
//
// Copyright 1998 Raven Software
//

#include "server.h"
#include "sv_effects.h"
#include "Vector.h"

server_static_t svs; // Persistent server info
server_t sv; // Local server

int SV_ModelIndex(char* name)
{
	NOT_IMPLEMENTED
	return 0;
}

int SV_SoundIndex(char* name)
{
	NOT_IMPLEMENTED
	return 0;
}

int SV_ImageIndex(char* name)
{
	NOT_IMPLEMENTED
	return 0;
}

void SV_ModelRemove(char* name)
{
	NOT_IMPLEMENTED
}

void SV_SoundRemove(char* name)
{
	NOT_IMPLEMENTED
}

// Q2 counterpart
// Entity baselines are used to compress the update messages to the clients.
// Only the fields that differ from the baseline will be transmitted.
static void SV_CreateBaseline(void)
{
	for (int entnum = 1; entnum < ge->num_edicts; entnum++)
	{
		edict_t* svent = EDICT_NUM(entnum);

		if (svent->inuse && (svent->s.modelindex || svent->s.sound || svent->s.effects))
		{
			svent->s.number = (short)entnum;

			// Take current state as baseline
			VectorCopy(svent->s.origin, svent->s.old_origin);
			sv.baselines[entnum] = svent->s;
		}
	}
}

static qboolean SV_CheckForSavegame(void)
{
	char temp[MAX_OSPATH];
	char name[MAX_OSPATH];

	if ((int)sv_noreload->value || (int)Cvar_VariableValue("deathmatch"))
		return false;

	// H2: strip directory path from sv.name... Checks only for '/' separator.
	strcpy_s(temp, sizeof(temp), sv.name); //mxd. strcpy -> strcpy_s

	//mxd. Rewritten the logic...
	const char* c = strrchr(temp, '/');
	if (c != NULL)
	{
		const int pos = (c - temp);
		memmove_s(temp, sizeof(temp), c + 1, strlen(temp) - pos);
		temp[pos] = 0;
	}

	Com_sprintf(name, sizeof(name), "%s/save/current/%s.sav", FS_Userdir(), temp); // H2: FS_Gamedir

	FILE* f;
	if (fopen_s(&f, name, "rb") == 0)
	{
		fclose(f);
		SV_ClearWorld();
		SV_ReadLevelFile(); // Get configstrings and areaportals

		return true;
	}

	return false; // No savegame
}

// Change the server to a new map, taking all connected clients along with it.
static void SV_SpawnServer(char* server, char* spawnpoint, const server_state_t serverstate, const qboolean attractloop, const qboolean loadgame)
{
	uint checksum;

	if (attractloop)
		Cvar_Set("paused", "0");

	Com_ColourPrintf(P_HEADER, "------- Server Initialization -------\n"); // Q2: Com_Printf

	Com_DPrintf("SpawnServer: %s\n", server);
	if (sv.demofile)
		fclose(sv.demofile);

	Cvar_SetValue("server_machine", 1.0f); // H2

	svs.spawncount++; // Any partially connected client will be restarted.

	sv.state = ss_dead;
	Com_SetServerState(sv.state);
	
	// Wipe the entire per-level structure.
	memset(&sv, 0, sizeof(sv));
	svs.realtime = 0;
	sv.loadgame = loadgame;
	sv.attractloop = attractloop;

	// Save name and welcome message for levels that don't set message.
	strcpy_s(sv.configstrings[CS_NAME], sizeof(sv.configstrings[CS_NAME]), server); //mxd. strcpy -> strcpy_s
	strcpy_s(sv.configstrings[CS_WELCOME], sizeof(sv.configstrings[CS_WELCOME]), sv_welcome_mess->string); // H2 //mxd. strcpy -> strcpy_s

	SZ_Init(&sv.multicast, sv.multicast_buf, sizeof(sv.multicast_buf));

	strcpy_s(sv.name, sizeof(sv.name), server); //mxd. strcpy -> strcpy_s

	// Leave slots at start for clients only.
	for (int i = 0; i < (int)maxclients->value; i++)
	{
		// Needs to reconnect
		if (svs.clients[i].state > cs_connected)
			svs.clients[i].state = cs_connected;

		svs.clients[i].lastframe = -1;
	}

	sv.time = 200; // Q2: 1000

	// Save name and welcome message for levels that don't set message. AGAIN. //TODO: why are these set twice?
	strcpy_s(sv.name, sizeof(sv.name), server);
	strcpy_s(sv.configstrings[CS_NAME], sizeof(sv.configstrings[CS_NAME]), server);
	strcpy_s(sv.configstrings[CS_WELCOME], sizeof(sv.configstrings[CS_WELCOME]), sv_welcome_mess->string); // H2

	if (serverstate == ss_game)
	{
		Com_sprintf(sv.configstrings[CS_MODELS + 1], sizeof(sv.configstrings[CS_MODELS + 1]), "maps/%s.bsp", server);
		sv.models[1] = CM_LoadMap(sv.configstrings[CS_MODELS + 1], false, &checksum);
	}
	else
	{
		sv.models[1] = CM_LoadMap("", false, &checksum); // No real map
	}

	Com_sprintf(sv.configstrings[CS_MAPCHECKSUM], sizeof(sv.configstrings[CS_MAPCHECKSUM]), "%i", checksum);

	// Clear physics interaction links.
	SV_ClearWorld();

	per_effects_buffers_size = 0; // H2
	memset(persistant_effects_array, 0, sizeof(persistant_effects_array)); // H2

	for (int i = 1; i < CM_NumInlineModels(); i++)
	{
		Com_sprintf(sv.configstrings[CS_MODELS + 1 + i], sizeof(sv.configstrings[CS_MODELS + 1 + i]), "*%i", i);
		sv.models[i + 1] = CM_InlineModel(sv.configstrings[CS_MODELS + 1 + i]);
	}

	// Spawn the rest of the entities on the map.

	// Precache and static commands can be issued during map initialization.
	sv.state = ss_loading;
	Com_SetServerState(sv.state);

	// Load and spawn all other entities
	ge->SpawnEntities(sv.name, CM_EntityString(), spawnpoint, sv.loadgame); // H2: extra arg

	// All precaches are complete.
	sv.state = serverstate;
	Com_SetServerState(serverstate);

	// Create a baseline for more efficient communications.
	SV_CreateBaseline();

	// Check for a savegame.
	const qboolean revisiting = SV_CheckForSavegame();

	ge->ConstructEntities(); // H2
	ge->CheckCoopTimeout(revisiting); // H2

	if ((int)dedicated->value) // H2
	{
		const int cooptimeout = (int)Cvar_VariableValue("sv_cooptimeout");
		if (cooptimeout > 0)
		{
			char msg[MAX_OSPATH];
			sprintf_s(msg, sizeof(msg), "Cinematic pending - waiting %i seconds for other players to join.\n\n", cooptimeout); //mxd. sprintf -> sprintf_s
			Com_Printf("%s", msg);
		}
	}

	// Run two frames to allow everything to settle.
	if (!sv.loadgame)
	{
		ge->RunFrame();
		ge->RunFrame();
	}

	if (svs.have_current_save) // H2
		for (int i = 0; i < 100; i++)
			ge->RunFrame();

	// Set serverinfo variable.
	Cvar_FullSet("mapname", sv.name, CVAR_SERVERINFO | CVAR_NOSET);

	Com_ColourPrintf(P_HEADER, "-------------------------------------\n"); // Q2: Com_Printf
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

// Modifies levelstring!
static void SetNextserver(char* levelstring)
{
	// If there is a + in the map, set nextserver to the remainder
	char* ch = strstr(levelstring, "+");
	if (ch != NULL)
	{
		*ch = 0;

		// Skip intro/outro cinematics in multiplayer.
		if (((int)Cvar_VariableValue("coop") || (int)Cvar_VariableValue("deathmatch")) && 
			(strcmp(levelstring, "intro.smk") == 0 || strcmp(levelstring, "outro.smk") == 0))
		{
			const int len = (int)strlen(levelstring);
			strcpy_s(levelstring, len - 10, levelstring + 10); // 10 == strlen("intro.smk")
			levelstring[10] = 0;

			Cvar_Set("nextserver", "");
		}
		else
		{
			Cvar_Set("nextserver", va("gamemap \"%s\"", ch + 1));
		}

		return;
	}

	ch = strstr(levelstring, "@");
	if (ch != NULL)
	{
		*ch = 0;
		Cvar_Set("nextserver", va("disconnect;%s", ch + 1));

		return;
	}

	Cvar_Set("nextserver", "");
}

//mxd. Sys_CopyProtect() logic skipped.
qboolean SV_ValidateMapFilename(char* level)
{
	char mappath[MAX_QPATH];

	// Check map file
	Com_sprintf(mappath, sizeof(mappath), "maps/%s.bsp", level);
	if (FS_LoadFile(mappath, 0) != -1)
		return true;

	// Check demo file
	Com_sprintf(mappath, sizeof(mappath), "demos/%s", level);
	if (FS_LoadFile(mappath, 0) != -1)
		return true;

	// Check movie file
	return (Q_stricmp(&level[strlen(level) - 4], ".smk") == 0);
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
	CL_ClearScreenShake(); // H2
}