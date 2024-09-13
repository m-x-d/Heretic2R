//
// sv_user.c
//
// Copyright 1998 Raven Software
//

#include "server.h"
#include "sv_effects.h"

edict_t* sv_player;

static void SV_ExecuteUserCommand(char* s)
{
	NOT_IMPLEMENTED
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