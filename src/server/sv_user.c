//
// sv_user.c
//
// Copyright 1998 Raven Software
//

#include "server.h"
#include "client.h"
#include "cl_strings.h"
#include "sv_effects.h"

edict_t* sv_player;

#pragma region ========================== USER STRINGCMD EXECUTION ==========================

// Q2 counterpart
static void SV_BeginDemoserver(void)
{
	char name[MAX_OSPATH];

	Com_sprintf(name, sizeof(name), "demos/%s", sv.name);
	FS_FOpenFile(name, &sv.demofile);

	if (sv.demofile == NULL)
		Com_Error(ERR_DROP, "Couldn't open %s\n", name);
}

// Sends the first message from the server to a connected client.
// This will be sent on the initial connection and upon each server load.
static void SV_New_f(void)
{
	Com_DPrintf("New() from %s\n", sv_client->name);

	if (sv_client->state != cs_connected)
	{
		Com_Printf("New not valid -- already spawned\n");
		return;
	}

	// Demo servers just dump the file message.
	if (sv.state == ss_demo)
	{
		SV_BeginDemoserver();
		return;
	}

	// serverdata needs to go over for all types of servers to make sure the protocol is right, and to set the gamedir.

	// Send the serverdata.
	MSG_WriteByte(&sv_client->netchan.message, svc_serverdata);
	MSG_WriteLong(&sv_client->netchan.message, PROTOCOL_VERSION);
	MSG_WriteLong(&sv_client->netchan.message, svs.spawncount);
	MSG_WriteByte(&sv_client->netchan.message, sv.attractloop);
	MSG_WriteString(&sv_client->netchan.message, Cvar_VariableString("gamedir"));
	MSG_WriteByte(&sv_client->netchan.message, (Cvar_VariableValue("deathmatch") != 0.0f)); // H2
	MSG_WriteString(&sv_client->netchan.message, client_string); // H2

	const int playernum = (sv.state == ss_cinematic ? -1 : sv_client - svs.clients);
	MSG_WriteShort(&sv_client->netchan.message, playernum);

	// Send full levelname.
	MSG_WriteString(&sv_client->netchan.message, sv.configstrings[CS_NAME]);

	// Game server.
	if (sv.state == ss_game)
	{
		// Set up the entity for the client.
		edict_t* ent = EDICT_NUM(playernum + 1);
		ent->s.number = (short)(playernum + 1);
		sv_client->edict = ent;
		memset(&sv_client->lastcmd, 0, sizeof(sv_client->lastcmd));

		// Begin fetching configstrings.
		MSG_WriteByte(&sv_client->netchan.message, svc_stufftext);
		MSG_WriteString(&sv_client->netchan.message, va("cmd configstrings %i 0\n", svs.spawncount));
	}

	sv_client->coop_state = cst_default; // H2
}

// Q2 counterpart
static void SV_Configstrings_f(void)
{
	Com_DPrintf("Configstrings() from %s\n", sv_client->name);

	if (sv_client->state != cs_connected)
	{
		Com_Printf("configstrings not valid -- allready spawned\n");
		return;
	}

	// Handle the case of a level changing while a client was connecting.
	if (Q_atoi(Cmd_Argv(1)) != svs.spawncount)
	{
		Com_Printf("SV_Configstrings_f from different level\n");
		SV_New_f();

		return;
	}

	int start = Q_atoi(Cmd_Argv(2));

	// Write a packet full of data.
	while (sv_client->netchan.message.cursize < MAX_MSGLEN / 2 && start < MAX_CONFIGSTRINGS)
	{
		if (sv.configstrings[start][0] != 0)
		{
			MSG_WriteByte(&sv_client->netchan.message, svc_configstring);
			MSG_WriteShort(&sv_client->netchan.message, start);
			MSG_WriteString(&sv_client->netchan.message, sv.configstrings[start]);
		}

		start++;
	}

	// Send next command.
	MSG_WriteByte(&sv_client->netchan.message, svc_stufftext);
	if (start == MAX_CONFIGSTRINGS)
		MSG_WriteString(&sv_client->netchan.message, va("cmd baselines %i 0\n", svs.spawncount));
	else
		MSG_WriteString(&sv_client->netchan.message, va("cmd configstrings %i %i\n", svs.spawncount, start));
}

static void SV_Baselines_f(void)
{
	entity_state_t nullstate;

	Com_DPrintf("Baselines() from %s\n", sv_client->name);

	if (sv_client->state != cs_connected)
	{
		Com_Printf("baselines not valid -- already spawned\n");
		return;
	}

	// Handle the case of a level changing while a client was connecting.
	if (Q_atoi(Cmd_Argv(1)) != svs.spawncount)
	{
		Com_Printf("SV_Baselines_f from different level\n");
		SV_New_f();

		return;
	}

	memset(&nullstate, 0, sizeof(nullstate));
	int start = Q_atoi(Cmd_Argv(2));

	// Write a packet full of data.
	while (sv_client->netchan.message.cursize < MAX_MSGLEN / 2 && start < MAX_EDICTS)
	{
		entity_state_t* base = &sv.baselines[start];
		if (base->modelindex != 0 || base->sound != 0 || base->effects != 0)
		{
			MSG_WriteByte(&sv_client->netchan.message, svc_spawnbaseline);
			MSG_WriteDeltaEntity(&nullstate, base, &sv_client->netchan.message, true); // H2: different function definition.
		}

		start++;
	}

	// Send next command
	MSG_WriteByte(&sv_client->netchan.message, svc_stufftext);
	if (start == MAX_EDICTS)
		MSG_WriteString(&sv_client->netchan.message, va("precache %i\n", svs.spawncount));
	else
		MSG_WriteString(&sv_client->netchan.message, va("cmd baselines %i %i\n", svs.spawncount, start));
}

static void SV_Begin_f(void)
{
	Com_DPrintf("Begin() from %s\n", sv_client->name);

	// Handle the case of a level changing while a client was connecting.
	const int client = Q_atoi(Cmd_Argv(1));
	if (client != svs.spawncount && client != -1) // H2: extra '-1' check
	{
		Com_Printf("SV_Begin_f from different level sever %i, client %i\n", svs.spawncount, client);
		SV_New_f();

		return;
	}

	if (Cvar_VariableValue("coop") != 0.0f) // H2
	{
		if (Cvar_VariableValue("sv_cinematicfreeze") != 0.0f)
		{
			if (sv_client->coop_state != cst_cinematic_freeze)
			{
				sv_client->coop_state = cst_cinematic_freeze;

				if ((int)dedicated->value)
				{
					char buffer[128];
					sprintf_s(buffer, sizeof(buffer), "Client: '%s' is waiting for cinematic currently in progress to end.\n", sv_client->name); //mxd. sprintf -> sprintf_s
					Com_Printf("%s", buffer);
				}

				SV_ClientGameMessage(sv_client, 2, GM_COOPWAITCIN);
			}

			MSG_WriteByte(&sv_client->netchan.message, svc_stufftext);
			MSG_WriteString(&sv_client->netchan.message, va("cmd begin %i\n", svs.spawncount));

			return;
		}

		if (Cvar_VariableValue("sv_cooptimeout") != 0.0f)
		{
			if (sv_client->coop_state != cst_coop_timeout)
			{
				sv_client->coop_state = cst_coop_timeout;
				SV_ClientGameMessage(sv_client, 2, GM_COOPTIMEOUT);
			}

			int c = 0;
			for (; c < (int)maxclients->value; c++)
				if (svs.clients[c].coop_state < cst_coop_timeout)
					break;

			if (c < (int)maxclients->value && sv.time < (uint)Cvar_VariableValue("sv_cooptimeout") * 1000)
			{
				MSG_WriteByte(&sv_client->netchan.message, svc_stufftext);
				MSG_WriteString(&sv_client->netchan.message, va("cmd begin %i\n", svs.spawncount));

				return;
			}
		}
	}

	sv_client->state = cs_spawned;
	ge->ClientBegin(sv_player); // Call the game begin function.

	Cbuf_InsertFromDefer();
}

void SV_Nextserver(void)
{
	if (sv.state == ss_game)
		return; // Can't nextserver while playing a normal game.

	svs.spawncount++; // Make sure another doesn't sneak in.
	const char* v = Cvar_VariableString("nextserver");

	if (*v == 0)
		Cbuf_AddText("killserver\n");
	else
		Cbuf_AddText(va("%s\n", v));

	Cvar_Set("nextserver", "");
}

// A cinematic has completed or been aborted by a client, so move to the next server.
static void SV_Nextserver_f(void)
{
	if (Q_atoi(Cmd_Argv(1)) == svs.spawncount)
		SV_Nextserver();
}

// Q2 counterpart
// The client is going to disconnect, so remove the connection immediately.
static void SV_Disconnect_f(void)
{
	SV_DropClient(sv_client);
}

// Q2 counterpart
// Dumps the serverinfo info string.
static void SV_ShowServerinfo_f(void)
{
	Info_Print(Cvar_Serverinfo());
}

// Q2 counterpart
static void SV_NextDownload_f(void)
{
	if (sv_client->download == NULL)
		return;

	const int r = min(sv_client->downloadsize - sv_client->downloadcount, 1024);

	MSG_WriteByte(&sv_client->netchan.message, svc_download);
	MSG_WriteShort(&sv_client->netchan.message, r);
	sv_client->downloadcount += r;

	const int size = max(sv_client->downloadsize, 1);
	const int percent = sv_client->downloadcount * 100 / size;

	MSG_WriteByte(&sv_client->netchan.message, percent);
	SZ_Write(&sv_client->netchan.message, sv_client->download + sv_client->downloadcount - r, r);

	// Download finished?
	if (sv_client->downloadcount == sv_client->downloadsize)
	{
		FS_FreeFile(sv_client->download);
		sv_client->download = NULL;
	}
}

static qboolean SV_IsValidDownloadPath(const char* path) //mxd
{
	if (!(int)allow_download->value) // No downloads allowed.
		return false;

	// Leading dot is no good. Leading slash bad as well, must be in subdir. No relative paths allowed. MUST be in a subdir.
	if (*path == '.' || *path == '/' || strstr(path, "..") != NULL || strchr(path, '/') == NULL)
		return false;

	if (!(int)allow_download_players->value && strncmp(path, "players/", 8) == 0) //mxd. Fixed strncmp length.
		return false;

	if (!(int)allow_download_models->value && strncmp(path, "models/", 7) == 0) //mxd. Fixed strncmp length.
		return false;

	if (!(int)allow_download_sounds->value && strncmp(path, "sound/", 6) == 0) //mxd. Fixed strncmp length.
		return false;

	if (!(int)allow_download_maps->value && strncmp(path, "maps/", 5) == 0) //mxd. Fixed strncmp length.
		return false;

	return true;
}

// Q2 counterpart
static void SV_BeginDownload_f(void)
{
	char* name = Cmd_Argv(1);
	const int offset = (Cmd_Argc() > 2 ? Q_atoi(Cmd_Argv(2)) : 0); // Downloaded offset.

	// Validate download path.
	if (!SV_IsValidDownloadPath(name))
	{
		MSG_WriteByte(&sv_client->netchan.message, svc_download);
		MSG_WriteShort(&sv_client->netchan.message, -1);
		MSG_WriteByte(&sv_client->netchan.message, 0);

		return;
	}

	if (sv_client->download != NULL)
		FS_FreeFile(sv_client->download);

	sv_client->downloadsize = FS_LoadFile(name, (void**)&sv_client->download);
	sv_client->downloadcount = offset;

	if (offset > sv_client->downloadsize)
		sv_client->downloadcount = sv_client->downloadsize;

	// Special check for maps, if it came from a pak file, don't allow download.
	if (file_from_pak && strncmp(name, "maps/", 5) == 0 && sv_client->download != NULL) //mxd. 'file_from_pak' value is updated by FS_LoadFile.
	{
		FS_FreeFile(sv_client->download);
		sv_client->download = NULL;
	}

	if (sv_client->download == NULL)
	{
		Com_DPrintf("Couldn't download %s to %s\n", name, sv_client->name);

		MSG_WriteByte(&sv_client->netchan.message, svc_download);
		MSG_WriteShort(&sv_client->netchan.message, -1);
		MSG_WriteByte(&sv_client->netchan.message, 0);
	}
	else
	{
		SV_NextDownload_f();
		Com_DPrintf("Downloading %s to %s\n", name, sv_client->name);
	}
}

#pragma endregion

typedef struct
{
	char* name;
	void (*func)(void);
} ucmd_t;

ucmd_t ucmds[] =
{
	// Auto-issued.
	{ "new", SV_New_f },
	{ "configstrings", SV_Configstrings_f },
	{ "baselines", SV_Baselines_f },
	{ "begin", SV_Begin_f },
	{ "nextserver", SV_Nextserver_f },
	{ "disconnect", SV_Disconnect_f },

	// Issued by hand at client consoles.
	{ "info", SV_ShowServerinfo_f },
	{ "download", SV_BeginDownload_f },
	{ "nextdl", SV_NextDownload_f },

	{ NULL, NULL }
};

// Q2 counterpart
static void SV_ExecuteUserCommand(char* s)
{
	ucmd_t* u;

	Cmd_TokenizeString(s, true);
	sv_player = sv_client->edict;

	for (u = ucmds; u->name != NULL; u++)
	{
		if (strcmp(Cmd_Argv(0), u->name) == 0)
		{
			u->func();
			break;
		}
	}

	if (u->name == NULL && sv.state == ss_game)
		ge->ClientCommand(sv_player);
}

#pragma region ========================== USER CMD EXECUTION ==========================

static void SV_ClientThink(client_t* client, usercmd_t* cmd)
{
	client->commandMsec -= cmd->msec;

	if (client->commandMsec < 0 && (int)sv_enforcetime->value)
	{
		Com_DPrintf("commandMsec underflow from %s\n", client->name);
		return;
	}

	if ((int)sv_cinematicfreeze->value && (cmd->buttons & BUTTON_ACTION)) // H2
	{
		Cvar_Set("sv_cinematicfreeze", "1");
		Cvar_Set("sv_jumpcinematic", "2");
	}

	if ((int)sv_cinematicfreeze->value == 0) // H2: new 'sv_cinematicfreeze' check
		ge->ClientThink(client->edict, cmd);
}

// The current net_message is parsed for the given client.
void SV_ExecuteClientMessage(client_t* cl)
{
#define MAX_STRINGCMDS	8

	usercmd_t nullcmd;
	usercmd_t oldest;
	usercmd_t oldcmd;
	usercmd_t newcmd;

	sv_client = cl;
	sv_player = cl->edict;

	// Only allow one move command.
	qboolean move_issued = false;
	int stringCmdCount = 0;

	while (true)
	{
		if (net_message.readcount > net_message.cursize)
		{
			Com_Printf("SV_ReadClientMessage: badread\n");
			SV_DropClient(cl);

			return;
		}

		const int c = MSG_ReadByte(&net_message);
		if (c == -1)
			return;

		switch (c)
		{
			case clc_nop:
				break;

			case clc_move:
			{
				if (move_issued)
					return; // Someone is trying to cheat...

				move_issued = true;
				const int checksum_index = net_message.readcount;
				const int checksum = MSG_ReadByte(&net_message);
				const int lastframe = MSG_ReadLong(&net_message);

				if (lastframe != cl->lastframe)
				{
					cl->lastframe = lastframe;

					if (lastframe > 0)
						cl->frame_latency[lastframe & (LATENCY_COUNTS - 1)] = svs.realtime - cl->frames[lastframe & UPDATE_MASK].senttime;
				}

				memset(&nullcmd, 0, sizeof(nullcmd));
				MSG_ReadDeltaUsercmd(&net_message, &nullcmd, &oldest);
				MSG_ReadDeltaUsercmd(&net_message, &oldest, &oldcmd);
				MSG_ReadDeltaUsercmd(&net_message, &oldcmd, &newcmd);

				if (cl->state != cs_spawned)
				{
					cl->lastframe = -1;
					SV_RemoveEdictFromPersistantEffectsArray(cl->edict); // H2

					break;
				}

				// If the checksum fails, ignore the rest of the packet.
				const byte* base = net_message.data + checksum_index + 1;
				const int length = net_message.readcount - checksum_index - 1;
				const int sequence = cl->netchan.incoming_sequence;
				const byte calculatedChecksum = COM_BlockSequenceCheckByte(base, length, sequence);
				if (calculatedChecksum != checksum)
				{
					Com_DPrintf("Failed command checksum for %s\n", cl->name); // H2: less verbose message
					return;
				}

				// H2: extra sv_freezeworldset check
				if (!(int)sv_paused->value && !(int)sv_freezeworldset->value)
				{
					int net_drop = cl->netchan.dropped;
					if (net_drop < 20)
					{
						while (net_drop > 2)
						{
							SV_ClientThink(cl, &cl->lastcmd);
							net_drop--;
						}

						if (net_drop > 1)
							SV_ClientThink(cl, &oldest);

						if (net_drop > 0)
							SV_ClientThink(cl, &oldcmd);
					}

					SV_ClientThink(cl, &newcmd);
				}

				cl->lastcmd = newcmd;
			} break;

			case clc_userinfo:
				strncpy_s(cl->userinfo, sizeof(cl->userinfo), MSG_ReadString(&net_message), sizeof(cl->userinfo) - 1); //mxd. strncpy -> strncpy_s
				SV_UserinfoChanged(cl);
				break;

			case clc_stringcmd:
			{
				char* s = MSG_ReadString(&net_message);

				// Malicious users may try using too many string commands.
				if (++stringCmdCount < MAX_STRINGCMDS)
					SV_ExecuteUserCommand(s);

				if (cl->state == cs_zombie)
					return; // Disconnect command.
			} break;

			case clc_startdemo: // H2
				if (cl->state == cs_zombie)
					return; // Disconnect command.

				SV_RemoveDemoEdictFromPersistantEffectsArray(cl);
				break;

			default:
				Com_Printf("SV_ReadClientMessage: unknown command char\n");
				SV_DropClient(cl);
				return;
		}
	}
}

#pragma endregion