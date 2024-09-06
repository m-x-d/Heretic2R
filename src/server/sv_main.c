//
// sv_main.c
//
// Copyright 1998 Raven Software
//

#include <stdlib.h>
#include "server.h"
#include "clfx_dll.h"

cvar_t* sv_paused;

static cvar_t* rcon_password; // Password for remote server commands

// New in H2:
static cvar_t* dmflags;
static cvar_t* advancedstaff; //TODO: unused?
static cvar_t* blood_level; //TODO: unused?

cvar_t* timeout; // Seconds without any message
cvar_t* zombietime; // Seconds to sink messages after disconnect

cvar_t* allow_download;
cvar_t* allow_download_players;
cvar_t* allow_download_models;
cvar_t* allow_download_sounds;
cvar_t* allow_download_maps;

cvar_t* maxclients; // FIXME: rename to sv_maxclients
cvar_t* sv_showclamp;

cvar_t* hostname;
cvar_t* public_server; // Should heartbeats be sent

cvar_t* sv_reconnect_limit; // Minimum seconds between connect messages

// New in H2:
cvar_t* sv_welcome_mess;
cvar_t* sv_freezeworldset;
cvar_t* sv_enforcetime;
cvar_t* sv_noreload;
cvar_t* sv_pers_fx_send_cut_off;
cvar_t* sv_noclientfx;
cvar_t* sv_cinematicfreeze;
cvar_t* sv_jumpcinematic;
cvar_t* sv_cooptimeout;
cvar_t* sv_loopcoop;

cvar_t* r_farclipdist;

qboolean is_local_client; // H2

void SV_UnloadClientEffects(void)
{
	NOT_IMPLEMENTED
}

static void SV_SendWelcomeMessasge(char* msg) // H2
{
	NOT_IMPLEMENTED
}

static void SV_CalcPings(void)
{
	NOT_IMPLEMENTED
}

static void SV_GiveMsec(void)
{
	NOT_IMPLEMENTED
}

static void SV_ReadPackets(void)
{
	NOT_IMPLEMENTED
}

static void SV_CheckTimeouts(void)
{
	NOT_IMPLEMENTED
}

static void SV_PrepWorldFrame(void)
{
	NOT_IMPLEMENTED
}

static void SV_RunGameFrame(void)
{
	NOT_IMPLEMENTED
}

static void Master_Heartbeat(void)
{
	NOT_IMPLEMENTED
}

void SV_Frame(const int msec)
{
	// If server is not active, do nothing.
	if (!svs.initialized)
		return;

	svs.realtime += msec;

	rand(); // Keep the random time dependent.

	SV_CheckTimeouts();	// Check timeouts.
	SV_ReadPackets();	// Get packets from clients.

	// Move autonomous things around if enough time has passed.
	if ((uint)svs.realtime < sv.time)
	{
		// Never let the time get too far off.
		if (sv.time - svs.realtime > 100)
		{
			if ((int)sv_showclamp->value)
				Com_Printf("sv lowclamp\n");

			svs.realtime = (int)sv.time - 100;
		}

		return;
	}

	// H2: send welcome message to clients?
	if (sv_welcome_mess->modified)
	{
		SV_SendWelcomeMessasge(sv_welcome_mess->string);
		sv_welcome_mess->modified = false;
	}

	SV_CalcPings();		// Update ping based on the last known frame from all clients.
	SV_GiveMsec();		// Give the clients some timeslices.
	SV_RunGameFrame();	// Let everything in the world think and move. 
	SV_SendClientMessages(true); // Send messages back to the clients that had packets read this frame.
	Master_Heartbeat();	// Send a heartbeat to the master if needed.
	SV_PrepWorldFrame(); // Clear teleport flags, etc. for next frame.
}

// Only called at quake2.exe startup, not for each game
void SV_Init(void)
{
	SV_InitOperatorCommands();

	rcon_password = Cvar_Get("rcon_password", "", 0);
	Cvar_Get("skill", "1", 0);
	Cvar_Get("deathmatch", "0", CVAR_LATCH);
	Cvar_Get("coop", "0", CVAR_LATCH);
	dmflags = Cvar_Get("dmflags", va("%i", DF_DISMEMBER), CVAR_SERVERINFO); // H2: different dmflags
	advancedstaff = Cvar_Get("advancedstaff", "1", CVAR_ARCHIVE | CVAR_SERVERINFO); // New in H2
	Cvar_Get("fraglimit", "0", CVAR_SERVERINFO);
	Cvar_Get("timelimit", "0", CVAR_SERVERINFO);
	Cvar_Get("cheats", "0", CVAR_SERVERINFO | CVAR_LATCH);
	Cvar_Get("protocol", va("%i", PROTOCOL_VERSION), CVAR_SERVERINFO | CVAR_NOSET);

	// New in H2:
	Cvar_Get("flood_msgs", "4", 0);
	Cvar_Get("flood_persecond", "4", 0);
	Cvar_Get("flood_waitdelay", "10", 0);
	Cvar_Get("flood_killdelay", "10", 0);
	Cvar_Get("sv_maplist", "", 0);
	blood_level = Cvar_Get("blood_level", "0", 0);

	maxclients = Cvar_Get("maxclients", "1", CVAR_SERVERINFO | CVAR_LATCH);
	hostname = Cvar_Get("hostname", "Parthoris", CVAR_ARCHIVE | CVAR_SERVERINFO);
	timeout = Cvar_Get("timeout", "125", 0);
	zombietime = Cvar_Get("zombietime", "2", 0);
	sv_showclamp = Cvar_Get("showclamp", "0", 0);
	sv_paused = Cvar_Get("paused", "0", 0);
	sv_noreload = Cvar_Get("sv_noreload", "0", 0);

	// New in H2:
	sv_welcome_mess = Cvar_Get("welcome_mess", "Welcome to Heretic II", 0);
	sv_freezeworldset = Cvar_Get("freezeworldset", "0", 0);
	sv_enforcetime = Cvar_Get("sv_enforcetime", "0", 0);
	sv_pers_fx_send_cut_off = Cvar_Get("sv_pers_fx_send_cut_off", "300", 0);
	sv_noclientfx = Cvar_Get("sv_noclientfx", "0", 0);
	sv_cinematicfreeze = Cvar_Get("sv_cinematicfreeze", "0", 0);
	sv_jumpcinematic = Cvar_Get("sv_jumpcinematic", "0", CVAR_ARCHIVE);
	sv_cooptimeout = Cvar_Get("sv_cooptimeout", "0", 0);
	sv_loopcoop = Cvar_Get("sv_loopcoop", "0", 0);

	r_farclipdist = Cvar_Get("r_farclipdist", "4096.0", 0); // Not imported in Q2

	public_server = Cvar_Get("public", "0", 0);
	sv_reconnect_limit = Cvar_Get("sv_reconnect_limit", "3", CVAR_ARCHIVE);

	SZ_Init(&net_message, net_message_buffer, sizeof(net_message_buffer));
	CLFX_LoadDll(); // H2
}

void SV_Shutdown(char* finalmsg, qboolean reconnect)
{
	NOT_IMPLEMENTED
}
