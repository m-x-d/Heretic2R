//
// sv_user.c
//
// Copyright 1998 Raven Software
//

#include "server.h"
#include "client.h"
#include "sv_effects.h"

edict_t* sv_player;

#pragma region ========================== USER STRINGCMD EXECUTION ==========================

static void SV_BeginDemoserver(void)
{
	NOT_IMPLEMENTED
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

static void SV_Configstrings_f(void)
{
	NOT_IMPLEMENTED
}

static void SV_Baselines_f(void)
{
	NOT_IMPLEMENTED
}

static void SV_Begin_f(void)
{
	NOT_IMPLEMENTED
}

void SV_Nextserver(void)
{
	NOT_IMPLEMENTED
}

// A cinematic has completed or been aborted by a client, so move to the next server.
static void SV_Nextserver_f(void)
{
	if (Q_atoi(Cmd_Argv(1)) == svs.spawncount)
		SV_Nextserver();
}

static void SV_Disconnect_f(void)
{
	NOT_IMPLEMENTED
}

static void SV_ShowServerinfo_f(void)
{
	NOT_IMPLEMENTED
}

static void SV_BeginDownload_f(void)
{
	NOT_IMPLEMENTED
}

static void SV_NextDownload_f(void)
{
	NOT_IMPLEMENTED
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

static void SV_ClientThink(client_t* cl, usercmd_t* cmd)
{
	NOT_IMPLEMENTED
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
				byte* base = net_message.data + checksum_index + 1;
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

				SV_UpdatePersistantEffectsDemoMask(cl);
				break;

			default:
				Com_Printf("SV_ReadClientMessage: unknown command char\n");
				SV_DropClient(cl);
				return;
		}
	}
}

#pragma endregion