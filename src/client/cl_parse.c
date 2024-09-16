//
// cl_parse.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "sound.h"

char* svc_strings[256] =
{
	"svc_bad",

	"svc_layout",
	"svc_inventory",
	"svc_client_effect", // H2

	"svc_nop",
	"svc_disconnect",
	"svc_reconnect",
	"svc_sound",
	"svc_print",
	"svc_gamemsg_print", // H2
	"svc_stufftext",
	"svc_serverdata",
	"svc_configstring",
	"svc_spawnbaseline",
	"svc_centerprint",
	"svc_gamemsg_centerprint", // H2
	"svc_gamemsgvar_centerprint", // H2
	"svc_levelmsg_centerprint", // H2
	"svc_captionprint", // H2
	"svc_obituary", // H2
	"svc_download",
	"svc_playerinfo",
	"svc_packetentities",
	"svc_deltapacketentities",
	"svc_frame",
	"svc_removeentities", // H2
	"svc_changeCDtrack", // H2
	"svc_framenum", // H2
	"svc_demo_client_effect", // H2
	"svc_special_client_effect", // H2
	"svc_gamemsgdual_centerprint", // H2
	"svc_nameprint", // H2
};

char client_string[128];

// H2 game messages
char game_message[1024];
int game_message_num_lines;
paletteRGBA_t game_message_color;
qboolean game_message_show_at_top;
float game_message_dispay_time;

char* CL_GetGameString(int i) // H2
{
	NOT_IMPLEMENTED
	return NULL;
}

char* CL_GetGameWav(int i) // H2
{
	NOT_IMPLEMENTED
	return NULL;
}

char* CL_GetLevelString(int i) // H2
{
	NOT_IMPLEMENTED
	return NULL;
}

char* CL_GetLevelWav(int i) // H2
{
	NOT_IMPLEMENTED
	return NULL;
}

int COLOUR(cvar_t* cvar)
{
	NOT_IMPLEMENTED
	return 0;
}

static void PrintGameMessage(const char* msg, PalIdx_t color_index)
{
	NOT_IMPLEMENTED
}

static void PrintObituary(const char* text, byte client1, byte client2, PalIdx_t color_index)
{
	NOT_IMPLEMENTED
}

void CL_RegisterSounds(void)
{
	S_BeginRegistration();

	if (fxe.RegisterSounds != NULL) // H2
		fxe.RegisterSounds();

	for (int i = 1; i < MAX_SOUNDS; i++)
	{
		if (cl.configstrings[CS_SOUNDS + i][0])
		{
			cl.sound_precache[i] = S_RegisterSound(cl.configstrings[CS_SOUNDS + i]);
			Sys_SendKeyEvents(); // Pump message loop
		}
	}

	S_EndRegistration();
}

static void CL_ParseDownload(void)
{
	NOT_IMPLEMENTED
}

#pragma region ========================== SERVER CONNECTING MESSAGES ==========================

static void CL_ParseServerData(void)
{
	NOT_IMPLEMENTED
}

static void CL_ParseBaseline(void)
{
	NOT_IMPLEMENTED
}

static void CL_ParseConfigString(void)
{
	NOT_IMPLEMENTED
}

#pragma endregion

#pragma region ========================== ACTION MESSAGES ==========================

static void CL_ParseStartSoundPacket(void)
{
	NOT_IMPLEMENTED
}

void SHOWNET(char* s)
{
	NOT_IMPLEMENTED
}

static void ChangeCDTrack(void) // H2
{
	NOT_IMPLEMENTED
}

static void ParseFramenum(void) // H2
{
	NOT_IMPLEMENTED
}

static void StartParsingDemoClientEffect(void) // H2
{
	NOT_IMPLEMENTED
}

void CL_ParseServerMessage(void)
{
	char buffer[500];

	// If recording demos, copy the message out.
	if ((int)cl_shownet->value == 1)
		Com_Printf("%i ", net_message.cursize);
	else if ((int)cl_shownet->value >= 2)
		Com_Printf("------------------\n");

	// Parse the message
	while (true)
	{
		if (net_message.readcount > net_message.cursize)
		{
			Com_Error(ERR_DROP, "CL_ParseServerMessage: Bad server message");
			break;
		}

		const int cmd = MSG_ReadByte(&net_message);

		if (cmd == -1)
		{
			SHOWNET("END OF MESSAGE");
			break;
		}

		if ((int)cl_shownet->value >= 2)
		{
			if (svc_strings[cmd] == NULL)
				Com_Printf("%3i:BAD CMD %i\n", net_message.readcount - 1, cmd);
			else
				SHOWNET(svc_strings[cmd]);
		}

		// Other commands.
		switch (cmd)
		{
			case svc_layout:
			{
				const char* layout = MSG_ReadString(&net_message);
				strncpy_s(cl.layout, sizeof(cl.layout), layout, sizeof(cl.layout) - 1); //mxd. strncpy -> strncpy_s
			} break;

			case svc_inventory:
				CL_ParseInventory();
				break;

			case svc_client_effect:
				fxe.ParseClientEffects(NULL); // H2
				break;

			case svc_nop:
				break;

			case svc_disconnect:
				Com_Printf("Server disconnected\n");
				CL_OnServerDisconnected(); // H2
				break;

			case svc_reconnect:
				Com_Printf("Server disconnected, reconnecting\n");
				cls.state = ca_connecting;
				cls.connect_time = -99999.0f; // CL_CheckForResend() will fire immediately
				if (cls.download != NULL)
				{
					fclose(cls.download);
					cls.download = NULL;
				}
				break;

			case svc_sound:
				CL_ParseStartSoundPacket();
				break;

			case svc_print:
			{
				const int mode = MSG_ReadByte(&net_message);
				if (mode == PRINT_CHAT)
				{
					S_StartLocalSound("misc/talk.wav");
					Com_ColourPrintf(COLOUR(colour_chat), "%s", MSG_ReadString(&net_message)); // H2
				}
				else if (mode == PRINT_TEAM) // H2
				{
					S_StartLocalSound("misc/talk.wav");
					Com_ColourPrintf(COLOUR(colour_teamchat), "%s", MSG_ReadString(&net_message));
				}
				else
				{
					Com_ColourPrintf(P_WHITE, "%s", MSG_ReadString(&net_message)); // H2
				}
			} break;

			case svc_gamemsg_print: // H2
			{
				const int msg_index = MSG_ReadShort(&net_message);
				const char* msg = CL_GetGameString(msg_index);
				if (msg != NULL && !(int)cl_no_middle_text->value)
					Com_ColourPrintf(COLOUR(colour_game), "%s", msg);

				const char* sound = CL_GetGameWav(msg_index);
				if (sound != NULL)
					S_StartLocalSound(sound);
			} break;

			case svc_stufftext:
			{
				const char* text = MSG_ReadString(&net_message);
				Com_DPrintf("stufftext: %s\n", text);
				Cbuf_AddText(text);
			} break;

			case svc_serverdata:
				Cbuf_Execute(); // Make sure any stuffed commands are done.
				CL_ParseServerData();
				break;

			case svc_configstring:
				CL_ParseConfigString();
				break;

			case svc_spawnbaseline:
				CL_ParseBaseline();
				break;

			case svc_centerprint:
				PrintGameMessage(MSG_ReadString(&net_message), P_WHITE); // H2
				break;

			case svc_gamemsg_centerprint: // H2
			{
				game_message_show_at_top = false;
				const int msg_index = MSG_ReadShort(&net_message);
				const char* msg = CL_GetGameString(msg_index);
				if (msg != NULL && !(int)cl_no_middle_text->value)
					PrintGameMessage(msg, COLOUR(colour_game));

				const char* sound = CL_GetGameWav(msg_index);
				if (sound != NULL)
					S_StartLocalSound(sound);
			} break;

			case svc_gamemsgvar_centerprint: // H2
			{
				game_message_show_at_top = false;
				const int msg_index = MSG_ReadShort(&net_message);
				const int msg_val = MSG_ReadLong(&net_message);
				
				if (!(int)cl_no_middle_text->value)
				{
					const char* msg = CL_GetGameString(msg_index);
					Com_sprintf(buffer, sizeof(buffer), msg, msg_val);
					PrintGameMessage(buffer, COLOUR(colour_level));
				}

				const char* sound = CL_GetGameWav(msg_index);
				if (sound != NULL)
					S_StartLocalSound(sound);
			} break;

			case svc_levelmsg_centerprint: // H2
			{
				game_message_show_at_top = false;
				const int msg_index = MSG_ReadShort(&net_message);
				const char* msg = CL_GetLevelString(msg_index);
				if (msg != NULL)
					PrintGameMessage(msg, COLOUR(colour_level));

				const char* sound = CL_GetLevelWav(msg_index);
				if (sound != NULL)
					S_StartLocalSound(sound);
			} break;

			case svc_captionprint: // H2
			{
				game_message_show_at_top = true;
				const int msg_index = MSG_ReadShort(&net_message);
				if ((int)cl_showcaptions->value)
				{
					const char* msg = CL_GetLevelString(msg_index);
					if (msg != NULL)
						PrintGameMessage(msg, P_CAPTION);
				}

				const char* sound = CL_GetLevelWav(msg_index);
				if (sound != NULL)
					S_StartLocalSound(sound);
			} break;

			case svc_obituary: // H2
			{
				const int msg_index = MSG_ReadShort(&net_message);
				if ((byte)msg_index == 132 && cl.configstrings[CS_WELCOME][0] != -2)
				{
					game_message_show_at_top = false;
					PrintGameMessage(cl.configstrings[CS_WELCOME], COLOUR(colour_game));
					cl.configstrings[CS_WELCOME][0] = -2;
				}

				const byte client1 = (byte)MSG_ReadByte(&net_message);
				const byte client2 = (byte)MSG_ReadByte(&net_message);

				const char* msg = CL_GetGameString(msg_index);
				if (msg != NULL)
					PrintObituary(msg, client1, client2, COLOUR(colour_obituary));

				const char* sound = CL_GetGameWav(msg_index);
				if (sound != NULL)
					S_StartLocalSound(sound);
			} break;

			case svc_download:
				CL_ParseDownload();
				break;

			case svc_playerinfo:
			case svc_packetentities:
			case svc_deltapacketentities:
				Com_Error(ERR_DROP, "Out of place frame data");
				break;

			case svc_frame:
				CL_ParseFrame();
				break;

			case svc_changeCDtrack: // H2
				ChangeCDTrack();
				break;

			case svc_framenum: // H2
				ParseFramenum();
				break;

			case svc_demo_client_effect: // H2
				StartParsingDemoClientEffect();
				break;

			case svc_special_client_effect: // H2
				MSG_ReadShort(&net_message);
				fxe.ParseClientEffects(NULL);
				break;

			case svc_gamemsgdual_centerprint: // H2
			{
				game_message_show_at_top = false;
				const int msg_index1 = MSG_ReadShort(&net_message);
				const int msg_index2 = MSG_ReadShort(&net_message);
				const char* msg1 = CL_GetGameString(msg_index1);
				const char* msg2 = CL_GetGameString(msg_index2);

				if (msg1 != NULL && msg2 != NULL)
				{
					strcpy_s(buffer, sizeof(buffer), msg1); //mxd. strcpy -> strcpy_s
					strcat_s(buffer, sizeof(buffer), msg2); //mxd. strcat -> strcat_s
					if (!(int)cl_no_middle_text->value)
						PrintGameMessage(buffer, COLOUR(colour_game));
				}

				const char* sound = CL_GetGameWav(msg_index1); //mxd. Done twice in original logic
				if (sound != NULL)
					S_StartLocalSound(sound);
			} break;

			case svc_nameprint: // H2
			{
				const int client_index = MSG_ReadByte(&net_message);
				const qboolean team_chat = MSG_ReadByte(&net_message);
				const char* msg = MSG_ReadString(&net_message);

				if (team_chat)
					sprintf_s(buffer, sizeof(buffer), "(%s) : %s", cl.clientinfo[client_index].name, msg);
				else
					sprintf_s(buffer, sizeof(buffer), "%s : %s", cl.clientinfo[client_index].name, msg);

				if (ignored_players[client_index])
					continue;

				S_StartLocalSound("misc/talk.wav");

				if (team_chat)
					Com_ColourPrintf(COLOUR(colour_teamchat), "%s", buffer + 4);
				else
					Com_ColourPrintf(COLOUR(colour_chat), "%s", buffer + 4);
			} break;

			default:
				Com_Error(ERR_DROP, "CL_ParseServerMessage: Illegible server message\n");
				break;
		}
	}

	CL_AddNetgraph();

	// We don't know if it is ok to save a demo message until after we have parsed the frame.
	if (cls.demorecording && !cls.demowaiting)
		CL_WriteDemoMessage();
}

#pragma endregion