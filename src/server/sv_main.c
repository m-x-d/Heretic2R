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

netadr_t master_adr[MAX_MASTERS]; // Address of group servers

client_t* sv_client; // Current client

cvar_t* sv_paused;

static cvar_t* rcon_password; // Password for remote server commands

cvar_t* dmflags; // H2
static cvar_t* advancedstaff; // H2
static cvar_t* blood_level; // H2

static cvar_t* timeout; // Seconds without any message
static cvar_t* zombietime; // Seconds to sink messages after disconnect

cvar_t* allow_download;
cvar_t* allow_download_players;
cvar_t* allow_download_models;
cvar_t* allow_download_sounds;
cvar_t* allow_download_maps;

cvar_t* maxclients; // FIXME: rename to sv_maxclients
static cvar_t* sv_showclamp;

static cvar_t* hostname;
static cvar_t* public_server; // Should heartbeats be sent

static cvar_t* sv_reconnect_limit; // Minimum seconds between connect messages

// H2:
cvar_t* sv_welcome_mess;
cvar_t* sv_freezeworldset;
cvar_t* sv_enforcetime;
cvar_t* sv_noreload;
cvar_t* sv_pers_fx_send_cut_off;
static cvar_t* sv_noclientfx;
cvar_t* sv_cinematicfreeze;
cvar_t* sv_jumpcinematic;
cvar_t* sv_cooptimeout;
static cvar_t* sv_loopcoop;

cvar_t* r_farclipdist;

qboolean is_local_client; // H2

//mxd. Defined in server.h in Q2.
typedef enum
{
	RD_NONE,
	RD_CLIENT,
	RD_PACKET
} redirect_t;

#define SV_OUTPUTBUF_LENGTH	(MAX_MSGLEN - 16)
static char sv_outputbuf[SV_OUTPUTBUF_LENGTH];

static void SV_SendWelcomeMessage(const char* msg) // H2
{
	if (msg != NULL && *msg != 0)
	{
		sv.baselines[0].number = 0;
		const int size = sizeof(sv.configstrings[0]);
		strncpy_s(sv.configstrings[CS_WELCOME], size, msg, size); //mxd. strncpy -> strncpy_s

		if (sv.state != ss_loading)
		{
			SZ_Clear(&sv.multicast);
			MSG_WriteByte(&sv.multicast, svc_configstring);
			MSG_WriteShort(&sv.multicast, CS_WELCOME);
			MSG_WriteString(&sv.multicast, msg);
			SV_Multicast(vec3_origin, MULTICAST_ALL_R);
		}
	}
}

// Q2 counterpart
// Called when the player is totally leaving the server, either willingly or unwillingly.
// This is NOT called if the entire server is quitting or crashing.
void SV_DropClient(client_t* drop)
{
	// Add the disconnect.
	MSG_WriteByte(&drop->netchan.message, svc_disconnect);

	if (drop->state == cs_spawned)
	{
		// Call the game function for removing a client.
		// This will remove the body, among other things.
		ge->ClientDisconnect(drop->edict);
	}

	if (drop->download != NULL)
	{
		FS_FreeFile(drop->download);
		drop->download = NULL;
	}

	drop->state = cs_zombie; // Become free in a few seconds.
	drop->name[0] = 0;
}

#pragma region ========================== CONNECTIONLESS COMMANDS ==========================

// Q2 counterpart
// Builds the string that is sent as heartbeats and status replies.
static char* SV_StatusString(void)
{
	static char	status[MAX_MSGLEN - 16];

	strcpy_s(status, sizeof(status), Cvar_Serverinfo()); //mxd. strcpy -> strcpy_s
	strcat_s(status, sizeof(status), "\n"); //mxd. strcat -> strcat_s
	uint status_len = strlen(status);

	for (int i = 0; i < (int)maxclients->value; i++)
	{
		client_t* client = &svs.clients[i];

		if (client->state == cs_connected || client->state == cs_spawned)
		{
			char player[1024];
			Com_sprintf(player, sizeof(player), "%i %i \"%s\"\n", client->edict->client->ps.stats[STAT_FRAGS], client->ping, client->name);
			const uint player_len = strlen(player);

			if (status_len + player_len >= sizeof(status))
				break; // Can't hold any more

			strcpy_s(status + status_len, sizeof(status) - status_len - 1, player); //mxd. strcpy -> strcpy_s
			status_len += player_len;
		}
	}

	return status;
}

// Q2 counterpart
static void SVC_Ack(void)
{
	Com_Printf("Ping acknowledge from %s\n", NET_AdrToString(&net_from));
}

// Q2 counterpart
static void SV_FlushRedirect(const int sv_redirected, char* outputbuf)
{
	switch (sv_redirected)
	{
		case RD_PACKET:
			Netchan_OutOfBandPrint(NS_SERVER, &net_from, "print\n%s", outputbuf);
			break;

		case RD_CLIENT:
			MSG_WriteByte(&sv_client->netchan.message, svc_print);
			MSG_WriteByte(&sv_client->netchan.message, PRINT_HIGH);
			MSG_WriteString(&sv_client->netchan.message, outputbuf);
			break;

		case RD_NONE:
			break;
	}
}

//mxd. Uses logic #if 0-ed in Q2. //TODO: use Q2 logic instead?
// Responds with all the info that qplug or qspy can see.
static void SVC_Status(void)
{
	Com_BeginRedirect(RD_PACKET, sv_outputbuf, sizeof(sv_outputbuf), SV_FlushRedirect);
	Com_Printf(SV_StatusString());
	Com_EndRedirect();
}

// Responds with short info for broadcast scans.
// The second parameter should be the current protocol version number.
static void SVC_Info(void)
{
	char string[64];

	if (maxclients->value == 1.0f)
		return; // Ignore in single player.

	const int version = Q_atoi(Cmd_Argv(1));

	if (version != PROTOCOL_VERSION)
	{
		Com_sprintf(string, sizeof(string), "%s: wrong version\n", hostname->string);
	}
	else
	{
		int count = 0;
		for (int i = 0; i < (int)maxclients->value; i++)
			if (svs.clients[i].state >= cs_connected)
				count++;

		hostname->string[44] = 0; // H2
		Com_sprintf(string, sizeof(string), "%16s\n%8s %2i/%2i\n", hostname->string, sv.name, count, (int)maxclients->value); // H2: different format string.
	}

	Netchan_OutOfBandPrint(NS_SERVER, &net_from, "info\n%s", string);
}

// Q2 counterpart
// Just responds with an acknowledgement.
static void SVC_Ping(void)
{
	Netchan_OutOfBandPrint(NS_SERVER, &net_from, "ack");
}

// Returns a challenge number that can be used in a subsequent client_connect command.
// We do this to prevent denial of service attacks that flood the server with invalid connection IPs.
// With a challenge, they must give a valid IP address.
static void SVC_GetChallenge(void)
{
	int i;

	int oldest = 0;
	int oldest_time = 0x7fffffff;

	// See if we already have a challenge for this ip.
	for (i = 0; i < MAX_CHALLENGES; i++)
	{
		if (NET_CompareBaseAdr(&net_from, &svs.challenges[i].adr))
			break;

		if (svs.challenges[i].time < oldest_time)
		{
			oldest_time = svs.challenges[i].time;
			oldest = i;
		}
	}

	if (i == MAX_CHALLENGES)
	{
		// Overwrite the oldest.
		svs.challenges[oldest].challenge = rand() ^ (rand() << 16); // Q2: rand() & 0x7fff;
		svs.challenges[oldest].adr = net_from;
		svs.challenges[oldest].time = curtime;
		i = oldest;
	}

	// Send it back.
	Netchan_OutOfBandPrint(NS_SERVER, &net_from, "challenge %i", svs.challenges[i].challenge);
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
		Netchan_OutOfBandPrint(NS_SERVER, adr, "print\nServer is version %d.\n", PROTOCOL_VERSION); // Q2 uses VERSION macro here.
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
		Netchan_OutOfBandPrint(NS_SERVER, adr, "print\nConnection refused.\n");

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
			if (NET_CompareBaseAdr(adr, &svs.challenges[ci].adr))
			{
				if (challenge != svs.challenges[ci].challenge)
				{
					Netchan_OutOfBandPrint(NS_SERVER, adr, "print\nBad challenge.\n");
					return;
				}

				break; // Good
			}
		}

		if (ci == MAX_CHALLENGES)
		{
			Netchan_OutOfBandPrint(NS_SERVER, adr, "print\nNo challenge for address.\n");
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

		if (NET_CompareBaseAdr(adr, &client->netchan.remote_address) && (client->netchan.qport == qport || adr->port == client->netchan.remote_address.port))
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
			Netchan_OutOfBandPrint(NS_SERVER, adr, "print\nServer is full.\n");
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
		Netchan_OutOfBandPrint(NS_SERVER, adr, "client_connect");
		Netchan_Setup(NS_SERVER, &newcl->netchan, adr, qport);
		newcl->state = cs_connected;
		SV_RemoveEdictFromPersistantEffectsArray(newcl->edict); // H2

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
		Netchan_OutOfBandPrint(NS_SERVER, adr, "print\nConnection refused.\n");
		Com_DPrintf("Game rejected a connection.\n");
	}
}

// Q2 counterpart
static qboolean Rcon_Validate(void)
{
	const char* pwd = rcon_password->string;
	return pwd[0] != 0 && (strcmp(Cmd_Argv(1), pwd) == 0); //mxd. strlen(str) -> str[0] check.
}

// A client issued an rcon command. Shift down the remaining args. Redirect all printfs.
static void SVC_RemoteCommand(void)
{
	char remaining[1024];

	const qboolean rcon_valid = Rcon_Validate();
	const char* format = (rcon_valid ? "Rcon from %s:\n%s\n" : "Bad rcon from %s:\n%s\n");
	Com_Printf(format, NET_AdrToString(&net_from), net_message.data + 4);

	Com_BeginRedirect(RD_PACKET, sv_outputbuf, SV_OUTPUTBUF_LENGTH, SV_FlushRedirect);

	if (!rcon_valid)
	{
		Com_Printf("Bad rcon_password.\n");
		Com_EndRedirect();

		return;
	}

	// H2. Check map name.
	if (Q_stricmp(Cmd_Argv(2), "map") == 0)
	{
		if (!SV_ValidateMapFilename(Cmd_Argv(3)))
		{
			Com_Printf("Bad map name.\n");
			Com_EndRedirect();

			return;
		}

		Com_DPrintf("Map validated ok.\n");
	}

	remaining[0] = 0;

	for (int i = 2; i < Cmd_Argc(); i++)
	{
		strcat_s(remaining, sizeof(remaining), Cmd_Argv(i)); //mxd. strcat -> strcat_s
		strcat_s(remaining, sizeof(remaining), " "); //mxd. strcat -> strcat_s
	}

	Cmd_ExecuteString(remaining);
	Com_EndRedirect();
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

#pragma endregion

// Q2 counterpart
// Updates the cl->ping variables.
static void SV_CalcPings(void)
{
	for (int i = 0; i < (int)maxclients->value; i++)
	{
		client_t* client = &svs.clients[i];
		if (client->state != cs_spawned)
			continue;

		int total = 0;
		int count = 0;

		for (int j = 0; j < LATENCY_COUNTS; j++)
		{
			if (client->frame_latency[j] > 0)
			{
				total += client->frame_latency[j];
				count++;
			}
		}

		client->ping = (count > 0 ? total / count : 0);
		client->edict->client->ping = client->ping; // Let the game dll know about the ping.
	}
}

// Q2 counterpart
// Every few frames, gives all clients an allotment of milliseconds for their command moves.
// If they exceed it, assume cheating.
static void SV_GiveMsec(void)
{
	if (sv.framenum & 15)
		return;

	for (int i = 0; i < (int)maxclients->value; i++)
	{
		client_t* client = &svs.clients[i];
		if (client->state != cs_free)
			client->commandMsec = 1800; // 1600 + some slop
	}
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

			if (!NET_CompareBaseAdr(&net_from, &client->netchan.remote_address))
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
			SV_RemoveEdictFromPersistantEffectsArray(client->edict); // H2
			client->state = cs_free; // Can now be reused
		}
		else if ((client->state == cs_connected || client->state == cs_spawned) && client->lastmessage < droppoint)
		{
			SV_BroadcastObituary(PRINT_HIGH, GM_TIMEDOUT, client->edict->s.number, 0); // H2
			SV_DropClient(client);
			SV_RemoveEdictFromPersistantEffectsArray(client->edict); // H2
			client->state = cs_free; // Don't bother with zombie state.
		}
	}
}

// This has to be done before the world logic, because player processing happens outside RunWorldFrame.
void SV_PrepWorldFrame(void)
{
	for (int i = 0; i < ge->num_edicts; i++)
	{
		edict_t* ent = EDICT_NUM(i);
		EffectsBuffer_t* fx_buf = &ent->s.clientEffects; // H2

		// H2. Effects only last for a single message.
		if (fx_buf->buf != NULL)
		{
			ResMngr_DeallocateResource(&sv_FXBufMngr, fx_buf->buf, ENTITY_FX_BUF_SIZE);

			fx_buf->buf = NULL;
			fx_buf->bufSize = 0;
			fx_buf->freeBlock = 0;
			fx_buf->numEffects = 0;
		}
	}
}

// H2: missing 'host_speeds' cvar logic.
static void SV_RunGameFrame(void)
{
	// We always need to bump framenum, even if we don't run the world,
	// otherwise the delta compression can get confused when a client has the "current" frame.
	sv.framenum++;
	sv.time = sv.framenum * 100;

	// Don't run if paused // H2: extra 'sv_freezeworldset' check.
	if ((!(int)sv_paused->value && !(int)sv_freezeworldset->value) || maxclients->value > 1)
	{
		// H2: new loop logic.
		do
		{
			ge->RunFrame();
		} while ((int)sv_jumpcinematic->value && (int)sv_cinematicfreeze->value);

		// Never get more than one tic behind.
		if (sv.time < (uint)svs.realtime)
		{
			if ((int)sv_showclamp->value)
				Com_Printf("sv highclamp\n");

			svs.realtime = (uint)sv.time;
		}
	}
}

// Send a message to the master every few minutes to let it know we are alive, and log information.
static void Master_Heartbeat(void)
{
#define HEARTBEAT_SECONDS	300

	if (!(int)dedicated->value || !(int)public_server->value) // H2: missing dedicated / public_server NULL checks.
		return;

	// Check for time wraparound.
	svs.last_heartbeat = min(svs.realtime, svs.last_heartbeat);

	if (svs.realtime - svs.last_heartbeat < HEARTBEAT_SECONDS * 1000)
		return; // Not time to send yet.

	svs.last_heartbeat = svs.realtime;

	// Send the same string that we would give for a status OOB command.
	char* string = SV_StatusString();

	//mxd. Skip GameSpy default addresses initialization logic.

	// Send to group master.
	for (int i = 0; i < MAX_MASTERS; i++)
	{
		if (master_adr[i].port)
		{
			Com_Printf("Sending heartbeat to %s\n", NET_AdrToString(&master_adr[i]));
			Netchan_OutOfBandPrint(NS_SERVER, &master_adr[i], "heartbeat\n%s", string);
		}
	}

	//mxd. Skip GameSpy_OutOfBandPrint logic.
}

// Informs all masters that this server is going down.
static void Master_Shutdown(void)
{
	if (!(int)dedicated->value || !(int)public_server->value) // H2: missing dedicated / public_server NULL checks.
		return;

	// Send to group master
	for (int i = 0; i < MAX_MASTERS; i++)
	{
		if (master_adr[i].port != 0)
		{
			if (i > 0)
				Com_Printf("Sending heartbeat to %s\n", NET_AdrToString(&master_adr[i]));

			Netchan_OutOfBandPrint(NS_SERVER, &master_adr[i], "shutdown");
		}
	}
}

void SV_Frame(const int usec) // YQ2: msec -> usec.
{
	// If server is not active, do nothing.
	if (!svs.initialized)
		return;

	svs.realtime += usec / 1000; // YQ2: msec -> usec / 1000.

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
		SV_SendWelcomeMessage(sv_welcome_mess->string);
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
	if (rate[0] != 0)
		client->rate = ClampI(Q_atoi(rate), 100, 15000);
	else
		client->rate = 5000;

	// msg command.
	const char* msg = Info_ValueForKey(client->userinfo, "msg");
	if (msg[0] != 0)
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
	advancedstaff = Cvar_Get("advancedstaff", "1", CVAR_ARCHIVE | CVAR_SERVERINFO); // H2
	Cvar_Get("fraglimit", "0", CVAR_SERVERINFO);
	Cvar_Get("timelimit", "0", CVAR_SERVERINFO);
	Cvar_Get("cheats", "0", CVAR_SERVERINFO | CVAR_LATCH);
	Cvar_Get("protocol", va("%i", PROTOCOL_VERSION), CVAR_SERVERINFO | CVAR_NOSET);

	// H2:
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

	// H2:
	sv_welcome_mess = Cvar_Get("welcome_mess", "Welcome to "GAME_NAME, 0); //mxd. Use define.
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

// Used by SV_Shutdown to send a final message to all connected clients before the server goes down.
// The messages are sent immediately, not just stuck on the outgoing message list,
// because the server is going to totally exit after returning from this function.
static void SV_FinalMessage(const char* message, const qboolean reconnect)
{
	SZ_Clear(&net_message);
	MSG_WriteByte(&net_message, svc_print);
	MSG_WriteByte(&net_message, PRINT_HIGH);
	MSG_WriteString(&net_message, message);
	MSG_WriteByte(&net_message, (reconnect ? svc_reconnect : svc_disconnect));

	// Send it twice. Stagger the packets to crutch operating system limited buffers.
	for (int c = 0; c < 2; c++)
	{
		client_t* client = svs.clients;
		for (int i = 0; i < (int)maxclients->value; i++, client++)
			if (client->state >= cs_connected)
				Netchan_Transmit(&client->netchan, net_message.cursize, net_message.data);
	}
}

// Called when each game quits, before Sys_Quit or Sys_Error.
void SV_Shutdown(const char* finalmsg, const qboolean reconnect)
{
	if (svs.clients != NULL)
		SV_FinalMessage(finalmsg, reconnect);

	Master_Shutdown();
	SV_ShutdownGameProgs();

	// Free current level.
	if (sv.demofile != NULL)
		fclose(sv.demofile);

	memset(&sv, 0, sizeof(sv));
	Com_SetServerState(sv.state);
	Cvar_SetValue("server_machine", 0); // H2

	// Free server static data.
	if (svs.clients != NULL)
		Z_Free(svs.clients);

	if (svs.client_entities != NULL)
		Z_Free(svs.client_entities);

	memset(&svs, 0, sizeof(svs));
}
