//
// cl_parse.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "cl_effects.h"
#include "cl_messages.h"
#include "cmodel.h"
#include "sound.h"
#include "tokens.h"

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
	Com_DPrintf("Serverdata packet received.\n");

	// Wipe the client_state_t struct.
	CL_ClearState();
	cls.state = 3;

	// Parse protocol version number.
	cls.serverProtocol = MSG_ReadLong(&net_message);
	if (cls.serverProtocol != PROTOCOL_VERSION)
		Com_Error(ERR_DROP, "Server returned version %i, not %i", cls.serverProtocol, PROTOCOL_VERSION);

	cl.servercount = MSG_ReadLong(&net_message);
	cl.attractloop = MSG_ReadByte(&net_message);

	// Parse game directory.
	const char* str = MSG_ReadString(&net_message);
	strncpy_s(cl.gamedir, sizeof(cl.gamedir), str, sizeof(cl.gamedir) - 1); //mxd. strncpy -> strncpy_s

	// Set game directory.
	if ((*str != 0 && (fs_gamedirvar->string == NULL || *fs_gamedirvar->string == 0 || strcmp(fs_gamedirvar->string, str) != 0)) ||
		(*str == 0 && (fs_gamedirvar->string != NULL && *fs_gamedirvar->string == 0))) //BUGFIX: mxd. 2-nd fs_gamedirvar check is broken in both Q2 and H2
		Cvar_Set("game", str);

	game_downloadable_type->value = (float)(int)MSG_ReadByte(&net_message); // H2

	// H2. Check client effects version.
	str = MSG_ReadString(&net_message);
	if (Q_stricmp(str, client_string) != 0)
	{
		Com_Printf("Error ! Client effects on Server different from Local\n%s on server.\n%s on Local\n", str, client_string);
		Com_Error(ERR_DROP, "Dropping Connect.\n");
	}

	// Parse player entity number.
	cl.playernum = MSG_ReadShort(&net_message);

	// Get the full level name.
	str = MSG_ReadString(&net_message);
	if (cl.playernum == -1)
	{
		SCR_PlayCinematic(str); // Playing a cinematic, not a level.
	}
	else
	{
		Com_Printf("\n%s\n\n", str);
		cl.refresh_prepped = false; // Need to prep refresh at next opportunity.
	}
}

static void CL_ParseBaseline(void)
{
	byte total;
	byte bits[5];
	entity_state_t nullstate;

	memset(bits, 0, sizeof(bits)); // H2
	memset(&nullstate, 0, sizeof(nullstate));

	total = 0; // H2
	nullstate.skeletalType = -1; // H2
	nullstate.rootJoint = -1; // H2
	nullstate.swapFrame = -1; // H2

	const int newnum = CL_ParseEntityBits(bits, &total);
	CL_ParseDelta(&nullstate, &cl_entities[newnum].baseline, newnum, bits);
}

void CL_LoadClientinfo(clientinfo_t* ci, char* s, int index)
{
	NOT_IMPLEMENTED
}

void CL_ParseClientinfo(int player)
{
	NOT_IMPLEMENTED
}

static void CL_ParseConfigString(void)
{
	char buffer1[200];
	char buffer2[200];

	const int i = MSG_ReadShort(&net_message);

	if (i < 0 || i >= MAX_CONFIGSTRINGS)
		Com_Error(ERR_DROP, "configstring > MAX_CONFIGSTRINGS");

	const char* s = MSG_ReadString(&net_message);
	strcpy_s(cl.configstrings[i], sizeof(cl.configstrings[i]), s); //mxd. strcpy -> strcpy_s

	// Set lightstyle?
	if (i >= CS_LIGHTS && i < CS_LIGHTS + MAX_LIGHTSTYLES)
	{
		fxe.SetLightstyle(i - CS_LIGHTS);
		return;
	}

	// Change CD track?
	if (i == CS_CDTRACK)
	{
		if (cl.refresh_prepped)
			CDAudio_Play(Q_atoi(cl.configstrings[CS_CDTRACK]), true);

		return;
	}

	// Load model?
	if (i >= CS_MODELS && i < CS_MODELS + MAX_MODELS)
	{
		if (cl.configstrings[i][0] == 0)
			return;

		if (i == CS_MODELS + 1)
		{
			CL_InitClientEffects(cl_fx_dll->string);
			strcpy_s(client_string, sizeof(client_string), fxe.client_string); //mxd. strcpy -> strcpy_s
			P_Load(player_dll->string);
		}

		// H2: expand model path.
		strcpy_s(buffer1, sizeof(buffer1), cl.configstrings[i]); //mxd. strcpy -> strcpy_s
		strcpy_s(buffer2, sizeof(buffer2), cl.configstrings[i]); //mxd. strcpy -> strcpy_s

		switch (buffer2[0])
		{
			case TOKEN_M_OBJECTS:
				buffer2[0] = '/';
				sprintf_s(buffer1, sizeof(buffer1), "models/objects%s", buffer2); //mxd. sprintf -> sprintf_s
				break;

			case TOKEN_M_MONSTERS:
				buffer2[0] = '/';
				sprintf_s(buffer1, sizeof(buffer1), "models/monsters%s", buffer2); //mxd. sprintf -> sprintf_s
				break;

			case TOKEN_M_MODELS:
				buffer2[0] = '/';
				sprintf_s(buffer1, sizeof(buffer1), "models%s", buffer2); //mxd. sprintf -> sprintf_s
				break;

			default:
				break;
		}

		char* c = &buffer2[strlen(buffer1) - 1];
		if (*c == -2)
		{
			*c = 0;
			sprintf_s(cl.configstrings[i], sizeof(cl.configstrings[i]), "%s/tris.fm", buffer1); //mxd. sprintf -> sprintf_s
		}

		if (cl.refresh_prepped)
		{
			cl.model_draw[i - CS_MODELS] = re.RegisterModel(cl.configstrings[i]);

			if (cl.configstrings[i][0] == '*')
				cl.model_clip[i - CS_MODELS] = CM_InlineModel(cl.configstrings[i]);
			else
				cl.model_clip[i - CS_MODELS] = NULL;
		}

		return;
	}

	// Pre-cache sound?
	if (i >= CS_SOUNDS && i < CS_SOUNDS + MAX_SOUNDS)
	{
		if (cl.configstrings[i][0] == 0)
			return;

		// H2: expand sound path.
		strcpy_s(buffer1, sizeof(buffer1), cl.configstrings[i]);
		strcpy_s(buffer2, sizeof(buffer2), cl.configstrings[i]);

		switch (buffer2[0])
		{
			case TOKEN_S_ITEMS:
				buffer2[0] = '/';
				sprintf_s(buffer1, sizeof(buffer1), "items%s", buffer2); //mxd. sprintf -> sprintf_s
				break;

			case TOKEN_S_PLAYER:
				buffer2[0] = '/';
				sprintf_s(buffer1, sizeof(buffer1), "player%s", buffer2); //mxd. sprintf -> sprintf_s
				break;

			case TOKEN_S_WEAPONS:
				buffer2[0] = '/';
				sprintf_s(buffer1, sizeof(buffer1), "weapons%s", buffer2); //mxd. sprintf -> sprintf_s
				break;

			case TOKEN_S_MONSTERS:
				buffer2[0] = '/';
				sprintf_s(buffer1, sizeof(buffer1), "monsters%s", buffer2); //mxd. sprintf -> sprintf_s
				break;

			case TOKEN_S_MISC:
				buffer2[0] = '/';
				sprintf_s(buffer1, sizeof(buffer1), "misc%s", buffer2); //mxd. sprintf -> sprintf_s
				break;

			case TOKEN_S_CORVUS:
				buffer2[0] = '/';
				sprintf_s(buffer1, sizeof(buffer1), "corvus%s", buffer2); //mxd. sprintf -> sprintf_s
				break;

			case TOKEN_S_CINEMATICS:
				buffer2[0] = '/';
				sprintf_s(buffer1, sizeof(buffer1), "cinematics%s", buffer2); //mxd. sprintf -> sprintf_s
				break;

			case TOKEN_S_AMBIENT:
				buffer2[0] = '/';
				sprintf_s(buffer1, sizeof(buffer1), "ambient%s", buffer2); //mxd. sprintf -> sprintf_s
				break;

			default:
				break;
		}

		char* c = &buffer2[strlen(buffer1) - 1];
		if (*c == -2)
		{
			*c = 0;
			sprintf_s(cl.configstrings[i], sizeof(cl.configstrings[i]), "%s.wav", buffer1); //mxd. sprintf -> sprintf_s
		}

		if (cl.refresh_prepped)
			cl.sound_precache[i - CS_SOUNDS] = S_RegisterSound(cl.configstrings[i]);

		return;
	}

	// Pre-cache image?
	if (i >= CS_IMAGES && i < CS_IMAGES + MAX_IMAGES)
	{
		if (cl.refresh_prepped)
			cl.image_precache[i - CS_IMAGES] = re.RegisterPic(cl.configstrings[i]);

		return;
	}

	// Load client info?
	if (i >= CS_PLAYERSKINS && i < CS_PLAYERSKINS + MAX_CLIENTS)
	{
		if (cl.refresh_prepped)
			CL_ParseClientinfo(i - CS_PLAYERSKINS);
	}
}

#pragma endregion

#pragma region ========================== ACTION MESSAGES ==========================

static void CL_ParseStartSoundPacket(void)
{
	NOT_IMPLEMENTED
}

// Q2 counterpart
void SHOWNET(char* s)
{
	if (cl_shownet->value >= 2.0f)
		Com_Printf("%3i:%s\n", net_message.readcount - 1, s);
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