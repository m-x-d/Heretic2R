//
// sv_main.c
//
// Copyright 1998 Raven Software
//

#include <stdlib.h>
#include "server.h"
#include "clfx_dll.h"
#include "sv_effects.h"
#include "cl_strings.h"

client_t* sv_client; // Current client

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

static void SV_DropClient(client_t* drop)
{
	NOT_IMPLEMENTED
}

static void SVC_Ping(void)
{
	NOT_IMPLEMENTED
}

static void SVC_Ack(void)
{
	NOT_IMPLEMENTED
}

static void SVC_Status(void)
{
	NOT_IMPLEMENTED
}

static void SVC_Info(void)
{
	NOT_IMPLEMENTED
}

static void SVC_GetChallenge(void)
{
	NOT_IMPLEMENTED
}

// A connection request that did not come from the master.
static void SVC_DirectConnect(void)
{
	static char userinfo[MAX_INFO_STRING]; //mxd. Made static.
	
	const netadr_t* adr = &net_from; //mxd. 'adr' type changed to pointer.
	Com_DPrintf("SVC_DirectConnect()\n");

	const int version = Q_atoi(Cmd_Argv(1));
	if (version != PROTOCOL_VERSION)
	{
		Netchan_OutOfBandPrint(NS_SERVER, *adr, "print\nServer is version %d.\n", PROTOCOL_VERSION); // Q2 uses VERSION macro here.
		Com_DPrintf("	rejected connect from version %i\n", version);

		return;
	}

	const int qport = Q_atoi(Cmd_Argv(2));
	const int challenge = Q_atoi(Cmd_Argv(3));

	strncpy_s(userinfo, sizeof(userinfo), Cmd_Argv(4), sizeof(userinfo) - 1); //mxd. strncpy -> strncpy_s

	// Force the IP key/value pair so the game can filter based on ip
	Info_SetValueForKey(userinfo, "ip", NET_AdrToString(adr));

	// Attract loop servers are ONLY for local clients.
	if (sv.attractloop && !NET_IsLocalAddress(adr))
	{
		Com_Printf("Remote connect in attract loop.  Ignored.\n");
		Netchan_OutOfBandPrint(NS_SERVER, *adr, "print\nConnection refused.\n");

		return;
	}

	// See if the challenge is valid
	if (!NET_IsLocalAddress(adr))
	{
		// H2: check game type.
		if ((int)Cvar_VariableValue("coop") && !(int)Cvar_VariableValue("dedicated") && !is_local_client)
			return;

		int ci;
		for (ci = 0; ci < MAX_CHALLENGES; ci++)
		{
			if (NET_CompareBaseAdr(*adr, svs.challenges[ci].adr))
			{
				if (challenge != svs.challenges[ci].challenge)
				{
					Netchan_OutOfBandPrint(NS_SERVER, *adr, "print\nBad challenge.\n");
					return;
				}

				break; // Good
			}
		}

		if (ci == MAX_CHALLENGES)
		{
			Netchan_OutOfBandPrint(NS_SERVER, *adr, "print\nNo challenge for address.\n");
			return;
		}
	}

	client_t* newcl = NULL; //mxd

	// If there is already a slot for this ip, reuse it.
	client_t* client = svs.clients;
	for (int i = 0; i < (int)maxclients->value; i++, client++)
	{
		if (client->state == cs_free)
			continue;

		if (NET_CompareBaseAdr(*adr, client->netchan.remote_address) && (client->netchan.qport == qport || adr->port == client->netchan.remote_address.port))
		{
			if (svs.realtime - client->lastconnect < (int)(sv_reconnect_limit->value * 1000)) // H2: missing !NET_IsLocalAddress(adr) check
			{
				Com_DPrintf("%s:reconnect rejected : too soon\n", NET_AdrToString(adr));
				return;
			}

			Com_Printf("%s:reconnect\n", NET_AdrToString(adr));

			// Found existing client.
			newcl = client;
			break;
		}
	}

	// Find a free client slot?
	if (newcl == NULL)
	{
		client = svs.clients;
		for (int i = 0; i < (int)maxclients->value; i++, client++)
		{
			if (client->state == cs_free)
			{
				newcl = client;
				break;
			}
		}

		if (newcl == NULL)
		{
			Netchan_OutOfBandPrint(NS_SERVER, *adr, "print\nServer is full.\n");
			Com_DPrintf("Rejected a connection.\n");

			return;
		}
	}

	// Build a new connection and accept the new client.
	// This is the only place a client_t is ever initialized.
	const client_t temp = { 0 };
	memcpy(newcl, &temp, sizeof(client_t));
	sv_client = newcl;

	const int edictnum = newcl - svs.clients + 1;
	edict_t* ent = EDICT_NUM(edictnum);
	newcl->edict = ent;
	//mxd. Missing: newcl->challenge = challenge;

	// Get the game a chance to reject this connection or modify the userinfo.
	if (ge->ClientConnect(ent, userinfo))
	{
		// Parse some info from the info strings.
		strncpy_s(newcl->userinfo, sizeof(newcl->userinfo), userinfo, sizeof(newcl->userinfo) - 1); //mxd. strncpy -> strncpy_s
		SV_UserinfoChanged(newcl);

		// Send the connect packet to the client.
		Netchan_OutOfBandPrint(NS_SERVER, *adr, "client_connect");
		Netchan_Setup(NS_SERVER, &newcl->netchan, adr, qport);
		newcl->state = cs_connected;
		SV_RemoveEdictFromEffectsArray(newcl->edict); // H2

		SZ_Init(&newcl->datagram, newcl->datagram_buf, sizeof(newcl->datagram_buf));
		newcl->datagram.allowoverflow = true;
		newcl->lastmessage = svs.realtime; // Don't timeout.
		newcl->lastconnect = svs.realtime;

		if (NET_IsLocalAddress(adr)) // H2
			is_local_client = true;
	}
	else
	{
		//mxd. Missing: if (*Info_ValueForKey (userinfo, "rejmsg")) Q2 logic.
		Netchan_OutOfBandPrint(NS_SERVER, *adr, "print\nConnection refused.\n");
		Com_DPrintf("Game rejected a connection.\n");
	}
}

static void SVC_RemoteCommand(void)
{
	NOT_IMPLEMENTED
}

// Q2 counterpart
// A connectionless packet has four leading 0xff characters to distinguish it from a game channel.
// Clients that are in the game can still send connectionless packets.
static void SV_ConnectionlessPacket(void)
{
	MSG_BeginReading(&net_message);
	MSG_ReadLong(&net_message); // Skip the -1 marker.

	char* s = MSG_ReadStringLine(&net_message);
	Cmd_TokenizeString(s, false);

	char* c = Cmd_Argv(0);
	Com_DPrintf("Packet %s : %s\n", NET_AdrToString(&net_from), c);

	if (strcmp(c, "ping") == 0)
		SVC_Ping();
	else if (strcmp(c, "ack") == 0)
		SVC_Ack();
	else if (strcmp(c, "status") == 0)
		SVC_Status();
	else if (strcmp(c, "info") == 0)
		SVC_Info();
	else if (strcmp(c, "getchallenge") == 0)
		SVC_GetChallenge();
	else if (strcmp(c, "connect") == 0)
		SVC_DirectConnect();
	else if (strcmp(c, "rcon") == 0)
		SVC_RemoteCommand();
	else
		Com_Printf("bad connectionless packet from %s:\n%s\n", NET_AdrToString(&net_from), s);
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
	while (NET_GetPacket(NS_SERVER, &net_from, &net_message))
	{
		// Check for connectionless packet (0xffffffff) first.
		if (*(int*)net_message.data == -1)
		{
			SV_ConnectionlessPacket();
			continue;
		}

		// Read the qport out of the message so we can fix up stupid address translating routers.
		MSG_BeginReading(&net_message);
		MSG_ReadLong(&net_message); // Sequence number
		MSG_ReadLong(&net_message); // Sequence number
		const ushort qport = (ushort)MSG_ReadShort(&net_message);

		// Check for packets from connected clients.
		client_t* client = svs.clients;
		for (int i = 0; i < (int)maxclients->value; i++, client++)
		{
			if (client->state == cs_free)
				continue;

			if (!NET_CompareBaseAdr(net_from, client->netchan.remote_address))
				continue;

			if (client->netchan.qport != qport)
				continue;

			if (client->netchan.remote_address.port != net_from.port)
			{
				Com_Printf("SV_ReadPackets: fixing up a translated port\n");
				client->netchan.remote_address.port = net_from.port;
			}

			if (Netchan_Process(&client->netchan, &net_message) && client->state != cs_zombie)
			{
				// This is a valid, sequenced packet, so process it.
				client->lastmessage = svs.realtime;	// Don't timeout.
				SV_ExecuteClientMessage(client);
			}

			break;
		}
	}

	//mxd. H2 Gamespy logic skipped.
}

// If a packet has not been received from a client for timeout->value seconds, drop the connection.
// Server frames are used instead of realtime to avoid dropping the local client while debugging.

// When a client is normally dropped, the client_t goes into a zombie state for a few seconds
// to make sure any final reliable message gets resent if necessary.
static void SV_CheckTimeouts(void)
{
	const int droppoint = (int)((float)svs.realtime - timeout->value * 1000.0f);
	const int zombiepoint = (int)((float)svs.realtime - zombietime->value * 1000.0f);

	client_t* client = svs.clients;

	for (int i = 0; i < (int)maxclients->value; i++, client++)
	{
		// Message times may be wrong across a changelevel.
		client->lastmessage = min(svs.realtime, client->lastmessage);

		if (client->state == cs_zombie && client->lastmessage < zombiepoint)
		{
			SV_RemoveEdictFromEffectsArray(client->edict); // H2
			client->state = cs_free; // Can now be reused
		}
		else if ((client->state == cs_connected || client->state == cs_spawned) && client->lastmessage < droppoint)
		{
			SV_BroadcastObituary(PRINT_HIGH, GM_TIMEDOUT, client->edict->s.number, 0); // H2
			SV_DropClient(client);
			SV_RemoveEdictFromEffectsArray(client->edict); // H2
			client->state = cs_free; // Don't bother with zombie state.
		}
	}
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

// Pull specific info from a newly changed userinfo string into a more C-friendly form.
void SV_UserinfoChanged(client_t* client)
{
	// Call prog code to allow overrides.
	ge->ClientUserinfoChanged(client->edict, client->userinfo);

	// Name for C code.
	const char* name = Info_ValueForKey(client->userinfo, "name");
	strncpy_s(client->name, sizeof(client->name), name, sizeof(client->name) - 1); //mxd. strncpy -> strncpy_s
	//mxd. Missing: mask off high bit logic.

	// rate command.
	const char* rate = Info_ValueForKey(client->userinfo, "rate");
	if (strlen(rate) > 0)
		client->rate = ClampI(Q_atoi(rate), 100, 15000);
	else
		client->rate = 5000;

	// msg command.
	const char* msg = Info_ValueForKey(client->userinfo, "msg");
	if (strlen(msg) > 0)
		client->messagelevel = Q_atoi(msg);
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
