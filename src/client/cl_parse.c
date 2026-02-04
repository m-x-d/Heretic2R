//
// cl_parse.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "cl_effects.h"
#include "cl_messages.h"
#include "cmodel.h"
#include "Skeletons.h"
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

float sound_event_id_time_array[SND_PRED_MAX];

int COLOUR(const cvar_t* cvar)
{
	const int pal_index = (int)cvar->value % P_MAX_COLORS;
	return max(0, pal_index); //mxd. Add lower-bound check.
}

static void CenterPrint(const char* msg, const PalIdx_t color_index) // H2
{
	char line[MAX_MESSAGE_LINE_LENGTH + 4];

	if (msg == NULL)
		return;

	strncpy_s(display_msg.message, sizeof(display_msg.message), msg, sizeof(display_msg.message) - 1); //mxd. strncpy -> strncpy_s
	display_msg.dispay_time = scr_centertime->value;
	display_msg.num_lines = 1;
	display_msg.color = TextPalette[color_index];

	// Calculate display time and number of lines.
	int num_chars = 0;
	const char* s = msg;
	while (*s != 0)
	{
		num_chars++;

		if (*s == '\n')
		{
			display_msg.dispay_time += 0.8f;
			display_msg.num_lines++;
		}
		else if ((int)(scr_centertime->value * 10.0f) < num_chars)
		{
			display_msg.dispay_time += 0.2f;
		}

		s++;
	}

	Com_Printf("\n");

	// Print message to console.
	s = msg;
	while (true)
	{
		int line_len;
		for (line_len = 0; line_len < MAX_MESSAGE_LINE_LENGTH; line_len++)
			if (s[line_len] == 0 || s[line_len] == '\n')
				break;

		//mxd. Ignore trailing spaces when centering...
		int trimmed_len = line_len;
		while (trimmed_len > 0 && s[trimmed_len - 1] == ' ')
			trimmed_len--;

		// Calculate padding to center string. 
		const int padding = max(0, (MAX_MESSAGE_LINE_LENGTH - trimmed_len) / 2);
		if (padding > 0)
			memset(line, ' ', padding);

		if (trimmed_len > 0)
			memcpy(&line[padding], s, trimmed_len);

		line[trimmed_len + padding] = 0;
		Com_ColourPrintf(color_index, "%s\n", line);

		// Skip to next line.
		s += line_len;

		if (*s == 0)
			break;

		s++; // Skip newline char.
	}

	Com_Printf("\n\n");
	Con_ClearNotify();
}

//mxd. Added 'dest_size' arg.
static void GetObituaryString(char* dest, const int dest_size, const char* src, const byte client1, const byte client2) // H2
{
	char* name1 = (client1 > 0 ? cl.clientinfo[client1 - 1].name : NULL);
	char* name2 = (client2 > 0 ? cl.clientinfo[client2 - 1].name : NULL);

	char* pos1 = (client1 > 0 ? strstr(src, "%1") : NULL);
	char* pos2 = (client2 > 0 ? strstr(src, "%2") : NULL);

	if (pos1 != NULL)
		pos1[1] = 's';

	if (pos2 != NULL)
		pos2[1] = 's';

	if (pos1 != NULL && pos2 != NULL)
	{
		if (pos1 >= pos2)
			Com_sprintf(dest, dest_size, src, name2, name1);
		else
			Com_sprintf(dest, dest_size, src, name1, name2);
	}
	else if (pos1 != NULL)
	{
		Com_sprintf(dest, dest_size, src, name1);
	}
	else if (pos2 != NULL)
	{
		Com_sprintf(dest, dest_size, src, name2);
	}
	else // pos1 == NULL && pos2 == NULL
	{
		strcpy_s(dest, dest_size, src);
	}
}

static void ObituaryPrint(const char* text, const byte client1, const byte client2, const PalIdx_t color_index) // H2
{
	char message[256];
	char temp[256];

	strcpy_s(message, sizeof(message), "Invalid obituary\n");
	strcpy_s(temp, sizeof(temp), text);

	GetObituaryString(message, sizeof(message), temp, client1, client2);
	strcat_s(message, sizeof(message), "\n");
	Com_ColourPrintf(color_index, message);
}

// Q2 counterpart
static void CL_DownloadFileName(char* dest, const int destlen, char* fn)
{
	if (strncmp(fn, "players", 7) == 0)
		Com_sprintf(dest, destlen, "%s/%s", BASEDIRNAME, fn);
	else
		Com_sprintf(dest, destlen, "%s/%s", FS_Gamedir(), fn);
}

// Q2 counterpart
// Returns true if the file exists, otherwise attempts to start a download from the server.
qboolean CL_CheckOrDownloadFile(const char* filename)
{
	FILE* fp;
	char name[MAX_OSPATH];

	if (strstr(filename, ".."))
	{
		Com_Printf("Refusing to download a path with ..\n");
		return true;
	}

	if (FS_LoadFile(filename, NULL) != -1)
		return true; // It exists, no need to download.

	strcpy_s(cls.downloadname, sizeof(cls.downloadname), filename); //mxd. strcpy -> strcpy_s

	// Download to a temp name, and only rename when done, so if interrupted a runt file won't be left.
	COM_StripExtension(cls.downloadname, cls.downloadtempname);
	strcat_s(cls.downloadtempname, sizeof(cls.downloadtempname), ".tmp"); //mxd. strcat -> strcat_s

	// Check to see if we already have a tmp for this file, if so, try to resume.
	CL_DownloadFileName(name, sizeof(name), cls.downloadtempname);

	if (fopen_s(&fp, name, "r+b") == 0) //mxd. fopen -> fopen_s
	{
		// Download file exists.
		fseek(fp, 0, SEEK_END);
		const int len = ftell(fp);

		cls.download = fp;

		// Give the server an offset to start the download.
		Com_Printf("Resuming %s\n", cls.downloadname);
		MSG_WriteByte(&cls.netchan.message, clc_stringcmd);
		MSG_WriteString(&cls.netchan.message, va("download %s %i", cls.downloadname, len));
	}
	else
	{
		Com_Printf("Downloading %s\n", cls.downloadname);
		MSG_WriteByte(&cls.netchan.message, clc_stringcmd);
		MSG_WriteString(&cls.netchan.message, va("download %s", cls.downloadname));
	}

	cls.downloadnumber++;
	cls.force_packet = true; // YQ2

	return false;
}

void CL_RegisterSounds(void)
{
	se.BeginRegistration();

	if (fxe.RegisterSounds != NULL) // H2
		fxe.RegisterSounds();

	for (int i = 1; i < MAX_SOUNDS; i++)
	{
		if (cl.configstrings[CS_SOUNDS + i][0])
		{
			cl.sound_precache[i] = se.RegisterSound(cl.configstrings[CS_SOUNDS + i]);
			IN_Update(); // Pump message loop.
		}
	}

	se.EndRegistration();
}

// A download message has been received from the server.
static void CL_ParseDownload(void)
{
	// Read the data.
	const int size = MSG_ReadShort(&net_message);
	int percent = MSG_ReadByte(&net_message);

	if (size == -1)
	{
		Com_Printf("Server does not have this file.\n");

		if (cls.download != NULL)
		{
			// If here, we tried to resume a file but the server said no.
			fclose(cls.download);
			cls.download = NULL;
		}

		CL_RequestNextDownload();
		return;
	}

	// Open the file if not opened yet.
	if (cls.download == NULL)
	{
		char name[MAX_OSPATH];
		CL_DownloadFileName(name, sizeof(name), cls.downloadtempname);
		FS_CreatePath(name);

		if (fopen_s(&cls.download, name, "wb") != 0)
		{
			net_message.readcount += size;
			Com_Printf("Failed to open %s\n", cls.downloadtempname);
			CL_RequestNextDownload();

			return;
		}
	}

	fwrite(net_message.data + net_message.readcount, 1, size, cls.download);
	net_message.readcount += size;

	if (percent != 100)
	{
		// #if 0-ed in Q2
		Com_Printf(".");
		percent = percent / 10 * 10;

		if (percent != cls.downloadpercent)
		{
			cls.downloadpercent = percent;
			Com_Printf("\r                                                           ");
			Com_Printf("\r%i%%", cls.downloadpercent);

			SCR_UpdateScreen();
		}

		MSG_WriteByte(&cls.netchan.message, clc_stringcmd);
		SZ_Print(&cls.netchan.message, "nextdl");
		cls.force_packet = true; // YQ2

		return;
	}

	// Download finished.
	Com_Printf("\r100%%                                                           \n");
	SCR_UpdateScreen();

	fclose(cls.download);

	// Rename the temp file to it's final name.
	char new_name[MAX_OSPATH];
	char old_name[MAX_OSPATH];

	CL_DownloadFileName(old_name, sizeof(old_name), cls.downloadtempname);
	CL_DownloadFileName(new_name, sizeof(new_name), cls.downloadname);

	if (rename(old_name, new_name) != 0)
		Com_Printf("failed to rename.\n");

	cls.download = NULL;
	cls.downloadpercent = 0;

	// Get another file if needed.
	CL_RequestNextDownload();
}

#pragma region ========================== SERVER CONNECTING MESSAGES ==========================

static void CL_ParseServerData(void)
{
	Com_DPrintf("Serverdata packet received.\n");

	// Clear all key states.
	In_FlushQueue(); // YQ2

	// Wipe the client_state_t struct.
	CL_ClearState();
	cls.state = ca_connected;

	// Parse protocol version number.
	cls.serverProtocol = MSG_ReadLong(&net_message);
	if (cls.serverProtocol != PROTOCOL_VERSION && cls.serverProtocol != H2R_PROTOCOL_VERSION) //mxd. +H2R_PROTOCOL_VERSION.
		Com_Error(ERR_DROP, "Server returned unsupported protocol version %i (expected %i or %i)", cls.serverProtocol, PROTOCOL_VERSION, H2R_PROTOCOL_VERSION);

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
		Com_Printf("Error! Client effects on Server different from Local\n%s on server.\n%s on Local\n", str, client_string);
		Com_Error(ERR_DROP, "Dropping Connect.\n");
	}

	// Parse player entity number.
	cl.playernum = MSG_ReadShort(&net_message);

	// Get the full level name.
	str = MSG_ReadString(&net_message);
	if (cl.playernum == -1)
	{
		if (!(int)show_splash_movies->value && strcmp(str, "bumper.smk") == 0) //mxd. Add show_splash_movies option.
			SCR_FinishCinematic(); // Skipping a cinematic.
		else
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
	nullstate.skeletalType = SKEL_NULL; // H2
	nullstate.rootJoint = NULL_ROOT_JOINT; // H2
	nullstate.swapFrame = NO_SWAP_FRAME; // H2

	const int newnum = CL_ParseEntityBits(bits, &total);
	CL_ParseDelta(&nullstate, &cl_entities[newnum].baseline, newnum, bits);
}

void CL_LoadClientinfo(clientinfo_t* ci, const char* s, const int index)
{
	static struct model_s* player_models[MAX_PLAYER_MODELS + 1]; //mxd. player_models[MAX_CLIENTS] is the single-player model?

	qboolean skin_registered;
	char skin_name[MAX_QPATH];
	char model_name[MAX_QPATH];
	char model_filename[MAX_QPATH];
	char skin_filename[MAX_QPATH];

	assert(index >= 0 && index <= MAX_PLAYER_MODELS); //mxd. Added sanity check.

	// Isolate the player's name.
	strncpy_s(ci->name, sizeof(ci->name), s, sizeof(ci->name));
	ci->name[sizeof(ci->name) - 1] = 0;

	char* t = strchr(s, '\\'); //mxd. strstr() -> strchr().
	if (t != NULL)
	{
		ci->name[t - s] = 0;
		s = t + 1;
	}

	if ((int)cl_noskins->value || *s == 0)
	{
		Com_sprintf(model_filename, sizeof(model_filename), "players/male/tris.fm");
		Com_sprintf(skin_filename, sizeof(skin_filename), "players/male/Corvus.m8");
		sprintf_s(skin_name, sizeof(skin_name), "Corvus"); //mxd. sprintf -> sprintf_s // Q2: Com_sprintf
		sprintf_s(model_name, sizeof(model_name), "male"); //mxd. sprintf -> sprintf_s // Q2: Com_sprintf

		player_models[index] = re.RegisterModel(model_filename);
		ci->model = &player_models[index];

		ci->skin[0] = re.RegisterSkin(skin_filename, &skin_registered);

		if (skin_registered)
		{
			ci->skin[SKIN_DAMAGED] = re.RegisterSkin("players/male/CorvusDmg.m8", &skin_registered);
			if (!skin_registered)
				ci->skin[SKIN_DAMAGED] = ci->skin[0];

			ci->skin[SKIN_REFLECTION] = re.RegisterSkin("players/male/reflect.m8", &skin_registered);
			if (!skin_registered)
				ci->skin[SKIN_REFLECTION] = ci->skin[0];
		}
		else
		{
			ci->skin[0] = NULL;
			ci->skin[SKIN_DAMAGED] = NULL;
			ci->skin[SKIN_REFLECTION] = NULL;
		}
	}
	else
	{
		// Isolate the model name.
		strcpy_s(model_name, sizeof(model_name), s);

		// Isolate the skin name.
		t = strchr(model_name, '/'); //mxd. strstr() -> strchr().
		if (t == NULL)
			t = strchr(model_name, '\\'); //mxd. strstr() -> strchr().

		if (t != NULL)
		{
			*t = 0;
			strcpy_s(skin_name, sizeof(skin_name), &s[strlen(model_name) + 1]); //mxd. strcpy -> strcpy_s
		}
		else
		{
			strcpy_s(model_name, sizeof(model_name), "male"); //mxd. strcpy -> strcpy_s
			strcpy_s(skin_name, sizeof(skin_name), s); //mxd. strcpy -> strcpy_s
		}

		// Model file.
		Com_sprintf(model_filename, sizeof(model_filename), "players/%s/tris.fm", model_name);

		player_models[index] = re.RegisterModel(model_filename);
		ci->model = &player_models[index];

		if (*ci->model == NULL)
		{
			strcpy_s(model_name, sizeof(model_name), "male"); //mxd. strcpy -> strcpy_s
			Com_sprintf(model_filename, sizeof(model_filename), "players/male/tris.fm");

			player_models[index] = re.RegisterModel(model_filename);
			ci->model = &player_models[index];

			strcpy_s(skin_name, sizeof(skin_name), "Corvus");
		}

		// Skin file.
		Com_sprintf(skin_filename, sizeof(skin_filename), "players/%s/%s.m8", model_name, skin_name);
		ci->skin[0] = re.RegisterSkin(skin_filename, &skin_registered);

		if (!skin_registered)
		{
			// Try with default skin.
			if (Q_stricmp(model_name, "male") == 0) // When male
			{
				Com_DPrintf("Loading of model '%s' with skin '%s' failed.  Trying skin '%s'\n", model_name, skin_name, "Corvus");
				strcpy_s(skin_name, sizeof(skin_name), "Corvus"); //mxd. strcpy -> strcpy_s
			}
			else if (Q_stricmp(model_name, "female") == 0) // When female
			{
				Com_DPrintf("Loading of model '%s' with skin '%s' failed.  Trying skin '%s'\n", model_name, skin_name, "Kiera");
				strcpy_s(skin_name, sizeof(skin_name), "Kiera"); //mxd. strcpy -> strcpy_s
			}
			else // When?
			{
				Com_DPrintf("Loading of model '%s' with skin '%s' failed.  Trying skin '%s'\n", model_name, skin_name, "skin");
				strcpy_s(skin_name, sizeof(skin_name), "skin"); //mxd. strcpy -> strcpy_s
			}

			Com_sprintf(skin_filename, sizeof(skin_filename), "players/%s/%s.m8", model_name, skin_name);
			ci->skin[0] = re.RegisterSkin(skin_filename, &skin_registered);
		}

		char buffer[MAX_QPATH];

		Com_sprintf(buffer, sizeof(buffer), "players/%s/%sDmg.m8", model_name, skin_name);
		ci->skin[SKIN_DAMAGED] = re.RegisterSkin(buffer, &skin_registered);
		if (!skin_registered)
			ci->skin[SKIN_DAMAGED] = ci->skin[0];

		Com_sprintf(buffer, sizeof(buffer), "players/%s/reflect.m8", model_name);
		ci->skin[SKIN_REFLECTION] = re.RegisterSkin(buffer, &skin_registered);
		if (!skin_registered)
			ci->skin[SKIN_REFLECTION] = ci->skin[0];
	}

	strcpy_s(ci->model_name, sizeof(ci->model_name), model_name); //mxd. strcpy -> strcpy_s
	strcpy_s(ci->skin_name, sizeof(ci->skin_name), skin_name); //mxd. strcpy -> strcpy_s

	// Must have loaded all data types to be valid.
	if (ci->skin[0] == NULL || *ci->model == NULL) //mxd. ci->model NULL check in original logic.
	{
		ci->skin[0] = NULL;
		ci->model = NULL;
	}
}

// Load the skin and model for a client.
void CL_ParseClientinfo(const int player)
{
	CL_LoadClientinfo(&cl.clientinfo[player], cl.configstrings[player + CS_PLAYERSKINS], player);
	SCR_UpdateProgressbar(0, 6); // H2
}

static void CL_ParseConfigString(void)
{
	char buffer1[200];
	char buffer2[200];

	const int i = MSG_ReadShort(&net_message);

	if (i < 0 || i >= MAX_CONFIGSTRINGS)
		Com_Error(ERR_DROP, "configstring > MAX_CONFIGSTRINGS");

	const char* s = MSG_ReadString(&net_message);

	if (i >= CS_STATUSBAR && i < CS_MAXCLIENTS)
	{
		//mxd. Statusbar layout string takes several configstring slots (this is by design)...
		const uint len = strlen(s) + 1; // Count trailing zero
		if (len > (CS_MAXCLIENTS - i) * sizeof(cl.configstrings[0]))
			Com_Error(ERR_DROP, "configstring: too big statusbar layout string (%i at index %i)\n", len, i);

		memcpy(cl.configstrings[i], s, len);
	}
	else
	{
		strcpy_s(cl.configstrings[i], sizeof(cl.configstrings[i]), s); //mxd. strcpy -> strcpy_s
	}

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
		{
			//mxd. Parse track info.
			int track;
			uint track_pos;
			qboolean looping;
			sscanf_s(cl.configstrings[CS_CDTRACK], "%i %i %i", &track, &track_pos, &looping);

			se.MusicPlay(track, track_pos, looping); //mxd. CDAudio_Play() in original logic.
		}

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

		switch ((byte)buffer2[0])
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

		char* c = &buffer1[strlen(buffer1) - 1];
		if ((byte)*c == TOKEN_M_MODELS)
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

		switch ((byte)buffer2[0])
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

		char* c = &buffer1[strlen(buffer1) - 1];
		if ((byte)*c == TOKEN_S_AMBIENT)
		{
			*c = 0;
			sprintf_s(cl.configstrings[i], sizeof(cl.configstrings[i]), "%s.wav", buffer1); //mxd. sprintf -> sprintf_s
		}

		if (cl.refresh_prepped)
			cl.sound_precache[i - CS_SOUNDS] = se.RegisterSound(cl.configstrings[i]);

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

//mxd. Written by SV_StartSound() / SV_StartEventSound().
static void CL_ParseStartSoundPacket(void)
{
	int channel;
	int ent;
	int attenuation;
	float volume;
	float ofs;
	float* pos;
	int event_id = 0;
	float leveltime = 0.0f;

	const int flags = MSG_ReadByte(&net_message);
	const int soundindex = MSG_ReadShort(&net_message); // Q2: MSG_ReadByte

	const qboolean have_prediction_info = (flags & SND_PRED_INFO); // H2
	if (have_prediction_info)
	{
		event_id = MSG_ReadByte(&net_message);
		leveltime = MSG_ReadFloat(&net_message);
	}

	if (flags & SND_VOLUME)
		volume = (float)MSG_ReadByte(&net_message) / 255.0f;
	else
		volume = 1.0f;

	if (flags & SND_ATTENUATION)
		attenuation = MSG_ReadByte(&net_message);
	else
		attenuation = ATTN_NORM;

	if (flags & SND_OFFSET)
		ofs = (float)MSG_ReadByte(&net_message) / 1000.0f;
	else
		ofs = 0.0f;

	if (flags & SND_ENT)
	{
		// Entity-relative.
		const int val = MSG_ReadShort(&net_message);
		ent = val >> 3;

		if (ent < 0 || ent >= MAX_EDICTS) //mxd. Add lower-bound check, fix upper-bound check (ent > MAX_EDICTS in original logic).
			Com_Error(ERR_DROP, "CL_ParseStartSoundPacket: invalid entity index %i!", ent);

		channel = val & 7;
	}
	else
	{
		channel = CHAN_AUTO;
		ent = 0;
	}

	vec3_t pos_v;

	if (flags & SND_POS) // Positioned in space.
	{
		MSG_ReadPos(&net_message, pos_v);
		pos = pos_v;
	}
	else // Use entity number.
	{
		pos = NULL;
	}

	struct sfx_s* sfx = cl.sound_precache[soundindex];

	if (sfx == NULL)
		return;

	if (!(int)cl_predict->value || !have_prediction_info || sound_event_id_time_array[event_id] > leveltime || sound_event_id_time_array[event_id] == 0.0f)
		se.StartSound(pos, ent, channel, sfx, volume, attenuation, ofs);

	if (have_prediction_info && sound_event_id_time_array[event_id] <= leveltime) // H2
		sound_event_id_time_array[event_id] = 0.0f;
}

// Q2 counterpart
void SHOWNET(char* s)
{
	if ((int)cl_shownet->value >= 2)
		Com_Printf("%3i:%s\n", net_message.readcount - 1, s);
}

static void ChangeCDTrack(void) // H2
{
	const int track = MSG_ReadByte(&net_message);
	const int looping = MSG_ReadByte(&net_message);

	se.MusicPlay(track, 0, looping); //mxd. CDAudio_Play() in original logic.
}

void CL_MusicGetCurrentTrackInfo(int* track, uint* track_pos, qboolean* looping) //mxd
{
	se.MusicGetCurrentTrackInfo(track, track_pos, looping);
}

static void ParseFramenum(void) // H2
{
	cl.frame.serverframe = MSG_ReadLong(&net_message);
	cl.frame.servertime = cl.frame.serverframe * 100;
	cl.time = cl.frame.serverframe * 100;
}

static void CL_ParseDemoClientEffects(void) // H2
{
	net_message.data[net_message.readcount - 1] = svc_special_client_effect;
	const int size = MSG_ReadShort(&net_message);
	net_message.readcount += size;
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
			Com_Error(ERR_DROP, "CL_ParseServerMessage: Bad server message");

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

			case svc_inventory: //TODO: never sent. Remove?
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
					se.StartLocalSound("misc/talk.wav");
					Com_ColourPrintf(COLOUR(colour_chat), "%s", MSG_ReadString(&net_message)); // H2
				}
				else if (mode == PRINT_TEAM) // H2
				{
					se.StartLocalSound("misc/talk.wav");
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
					se.StartLocalSound(sound);
			} break;

			case svc_stufftext:
			{
				const char* text = MSG_ReadString(&net_message);
				Com_DDPrintf(2, "stufftext: %s\n", text); //mxd. Com_DPrintf() -> Com_DDPrintf(), to reduce console spam when developer 1.
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
				CenterPrint(MSG_ReadString(&net_message), P_WHITE); // H2
				break;

			case svc_gamemsg_centerprint: // H2
			{
				display_msg.is_caption = false;
				const int msg_index = MSG_ReadShort(&net_message);
				const char* msg = CL_GetGameString(msg_index);
				if (msg != NULL && !(int)cl_no_middle_text->value)
					CenterPrint(msg, COLOUR(colour_game));

				const char* sound = CL_GetGameWav(msg_index);
				if (sound != NULL)
					se.StartLocalSound(sound);
			} break;

			case svc_gamemsgvar_centerprint: // H2
			{
				display_msg.is_caption = false;
				const int msg_index = MSG_ReadShort(&net_message);
				const int msg_val = MSG_ReadLong(&net_message);
				
				if (!(int)cl_no_middle_text->value)
				{
					const char* msg = CL_GetGameString(msg_index);
					Com_sprintf(buffer, sizeof(buffer), msg, msg_val);
					CenterPrint(buffer, COLOUR(colour_level));
				}

				const char* sound = CL_GetGameWav(msg_index);
				if (sound != NULL)
					se.StartLocalSound(sound);
			} break;

			case svc_levelmsg_centerprint: // H2
			{
				display_msg.is_caption = false;
				const int msg_index = MSG_ReadShort(&net_message);
				const char* msg = CL_GetLevelString(msg_index);
				if (msg != NULL)
					CenterPrint(msg, COLOUR(colour_level));

				const char* sound = CL_GetLevelWav(msg_index);
				if (sound != NULL)
					se.StartLocalSound(sound);
			} break;

			case svc_captionprint: // H2
			{
				display_msg.is_caption = true;
				const int msg_index = MSG_ReadShort(&net_message);
				if ((int)cl_showcaptions->value)
				{
					const char* msg = CL_GetLevelString(msg_index);
					if (msg != NULL)
						CenterPrint(msg, P_CAPTION);
				}

				const char* sound = CL_GetLevelWav(msg_index);
				if (sound != NULL)
					se.StartLocalSound(sound);
			} break;

			case svc_obituary: // H2
			{
				const int msg_index = MSG_ReadShort(&net_message);
				if ((byte)msg_index == 132 && cl.configstrings[CS_WELCOME][0] != -2)
				{
					display_msg.is_caption = false;
					CenterPrint(cl.configstrings[CS_WELCOME], COLOUR(colour_game));
					cl.configstrings[CS_WELCOME][0] = -2;
				}

				const byte client1 = (byte)MSG_ReadByte(&net_message);
				const byte client2 = (byte)MSG_ReadByte(&net_message);

				const char* msg = CL_GetGameString(msg_index);
				if (msg != NULL)
					ObituaryPrint(msg, client1, client2, COLOUR(colour_obituary));

				const char* sound = CL_GetGameWav(msg_index);
				if (sound != NULL)
					se.StartLocalSound(sound);
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
				CL_ParseDemoClientEffects();
				break;

			case svc_special_client_effect: // H2
				MSG_ReadShort(&net_message);
				fxe.ParseClientEffects(NULL);
				break;

			case svc_gamemsgdual_centerprint: // H2
			{
				display_msg.is_caption = false;
				const int msg_index1 = MSG_ReadShort(&net_message);
				const int msg_index2 = MSG_ReadShort(&net_message);
				const char* msg1 = CL_GetGameString(msg_index1);
				const char* msg2 = CL_GetGameString(msg_index2);

				if (msg1 != NULL && msg2 != NULL)
				{
					strcpy_s(buffer, sizeof(buffer), msg1); //mxd. strcpy -> strcpy_s
					strcat_s(buffer, sizeof(buffer), msg2); //mxd. strcat -> strcat_s
					if (!(int)cl_no_middle_text->value)
						CenterPrint(buffer, COLOUR(colour_game));
				}

				const char* sound = CL_GetGameWav(msg_index1); //mxd. Done twice in original logic
				if (sound != NULL)
					se.StartLocalSound(sound);
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

				se.StartLocalSound("misc/talk.wav");

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