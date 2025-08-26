//
// cl_main.c
//
// Copyright 1998 Raven Software
//

#include <setjmp.h>
#include "client.h"
#include "clfx_dll.h"
#include "cl_effects.h"
#include "cl_messages.h"
#include "cl_skeletons.h"
#include "cmodel.h"
#include "ResourceManager.h"
#include "snd_dll.h"
#include "Vector.h"
#include "FlexModel.h"
#include "Reference.h"

//TODO: used only in CL_InitLocal(). Can be removed?
static cvar_t* adr0;
static cvar_t* adr1;
static cvar_t* adr2;
static cvar_t* adr3;
static cvar_t* adr4;
static cvar_t* adr5;
static cvar_t* adr6;
static cvar_t* adr7;
static cvar_t* adr8;

cvar_t* cl_stereo_separation;
cvar_t* cl_stereo;

static cvar_t* rcon_client_password;
static cvar_t* rcon_address;

cvar_t* cl_noskins;
cvar_t* cl_autoskins;
cvar_t* cl_footsteps;
static cvar_t* cl_timeout;
cvar_t* cl_predict;
cvar_t* cl_predict_local; // H2
cvar_t* cl_predict_remote; // H2
static cvar_t* cl_predict_lag; // H2
cvar_t* cl_maxfps;

cvar_t* cl_add_particles;
cvar_t* cl_add_lights;
cvar_t* cl_add_entities;
cvar_t* cl_add_blend;

cvar_t* cl_frametime;
cvar_t* cl_yawspeed;
cvar_t* cl_pitchspeed;
cvar_t* cl_anglespeedkey;

cvar_t* cl_run;
cvar_t* freelook;
cvar_t* lookspring;
cvar_t* lookstrafe;
cvar_t* mouse_sensitivity_x;
cvar_t* mouse_sensitivity_y;
cvar_t* doubletap_speed;

cvar_t* m_pitch;
cvar_t* m_yaw;
cvar_t* m_forward;
cvar_t* m_side;

cvar_t* cl_shownet;
cvar_t* cl_showmiss;
cvar_t* cl_showclamp;
static cvar_t* cl_timeout;
cvar_t* cl_paused;
cvar_t* cl_freezeworld;
cvar_t* cl_timedemo;
cvar_t* cl_no_middle_text;
cvar_t* cl_lightlevel;

// H2:
cvar_t* shownames;
cvar_t* r_detail;
cvar_t* cl_showcaptions;
cvar_t* cl_doautoaim;

cvar_t* cl_camera_combat;
cvar_t* cl_camera_clipdamp;
cvar_t* cl_camera_dampfactor;
cvar_t* cl_camera_fpoffs;
cvar_t* cl_camera_freeze;
cvar_t* cl_camera_under_surface;
cvar_t* cl_camera_viewdist;
cvar_t* cl_camera_viewmin;
cvar_t* cl_camera_viewmax;
cvar_t* cl_camera_fpmode;
cvar_t* cl_camera_fptrans;
cvar_t* cl_camera_fpdist;
cvar_t* cl_camera_fpheight;

cvar_t* cl_playertrans;
cvar_t* EAX_preset;
cvar_t* EAX_default;
cvar_t* quake_amount;
cvar_t* cl_fx_dll;
cvar_t* cl_cinematicfreeze;
static cvar_t* sc_framerate;

// userinfo
cvar_t* playerdir;
static cvar_t* info_password;
static cvar_t* skin;
static cvar_t* skindir;
static cvar_t* rate;
static cvar_t* msg;
static cvar_t* fov;

cvar_t* player_name; //mxd. Named 'name' in Q2. Renamed to avoid collisions with local vars with the same name.
cvar_t* autoweapon;
cvar_t* colour_obituary;
cvar_t* colour_chat;
cvar_t* colour_names;
cvar_t* colour_teamchat;
cvar_t* colour_level;
cvar_t* colour_game;
cvar_t* game_downloadable_type;

client_static_t cls;
client_state_t cl;

centity_t cl_entities[MAX_NETWORKABLE_EDICTS]; //mxd. MAX_EDICTS in Q2
entity_state_t cl_parse_entities[MAX_PARSE_ENTITIES];

qboolean ignored_players[MAX_CLIENTS];

static int precache_check; // For autodownload of precache items.
static int precache_spawncount;
static int precache_tex;
static int precache_model_skin;

static byte* precache_model; // Used for skin checking in alias models.

// Dumps the current net message, prefixed by the length.
void CL_WriteDemoMessage(void)
{
	// The first 8 bytes are just packet sequencing stuff.
	const int len = net_message.cursize - 8;
	uint num = fwrite(&len, 4, 1, cls.demofile);
	cls.demosavingok &= (num > 0); // H2

	num = fwrite(net_message.data + 8, len, 1, cls.demofile);
	cls.demosavingok &= (num > 0); // H2
}

void CL_ClearState(void)
{
	CL_ClearSkeletalEntities(); // H2
	memset(&cl, 0, sizeof(cl)); // Wipe the entire cl structure.
	SZ_Clear(&cls.netchan.message);
}

static void ClearIgnoredPlayersList(void)
{
	//mxd. Original code clears only first 8 entries. H2 bug?
	memset(ignored_players, 0, sizeof(ignored_players));
}

// Goes from a connected state to full screen console state.
// Sends a disconnect message to the server.
// This is also called on Com_Error, so it shouldn't cause any errors.
void CL_Disconnect(void)
{
	byte final[32];

	if (cls.state == ca_disconnected)
		return;

	if (cl_timedemo != NULL && (int)cl_timedemo->value)
	{
		const int time = Sys_Milliseconds() - cl.timedemo_start;
		if (time > 0)
			Com_Printf("%i frames, %3.1f seconds: %3.1f fps\n", cl.timedemo_frames, time / 1000.0, cl.timedemo_frames * 1000.0 / time);
	}

	VectorClear(cl.refdef.blend);
	ClearIgnoredPlayersList(); // H2
	M_ForceMenuOff();
	SCR_EndLoadingPlaque();

	cls.connect_time = 0;
	if (cls.demorecording)
		CL_Stop_f();

	// Send a disconnect message to the server.
	final[0] = clc_stringcmd;
	strcpy_s((char*)final + 1, sizeof(final) - 1, "disconnect"); //mxd. strcpy -> strcpy_s

	const int len = (int)strlen((const char*)final);
	Netchan_Transmit(&cls.netchan, len, final);
	Netchan_Transmit(&cls.netchan, len, final);
	Netchan_Transmit(&cls.netchan, len, final);

	CL_ClearState();

	// Stop download.
	if (cls.download != NULL)
	{
		fclose(cls.download);
		cls.download = NULL;
	}

	cls.state = ca_disconnected;
}

// Q2 counterpart
// Adds the current command line as a clc_stringcmd to the client message.
// Things like godmode, noclip, etc, are commands directed to the server,
// so when they are typed in at the console, they will need to be forwarded.
void Cmd_ForwardToServer(void)
{
	char* cmd = Cmd_Argv(0);
	if (cls.state <= ca_connected || *cmd == '-' || *cmd == '+')
	{
		Com_Printf("Unknown command \"%s\"\n", cmd);
		return;
	}

	MSG_WriteByte(&cls.netchan.message, clc_stringcmd);
	SZ_Print(&cls.netchan.message, cmd);
	if (Cmd_Argc() > 1)
	{
		SZ_Print(&cls.netchan.message, " ");
		SZ_Print(&cls.netchan.message, Cmd_Args());
	}
}

// Q2 counterpart
static void CL_ForwardToServer_f(void)
{
	if (cls.state != ca_connected && cls.state != ca_active)
	{
		Com_Printf("Can't '%s', not connected\n", Cmd_Argv(0));
		return;
	}

	// Don't forward the first argument.
	if (Cmd_Argc() > 1)
	{
		MSG_WriteByte(&cls.netchan.message, clc_stringcmd);
		SZ_Print(&cls.netchan.message, Cmd_Args());
	}
}

// Q2 counterpart
static void CL_Pause_f(void)
{
	// Never pause in multiplayer.
	if (Cvar_VariableValue("maxclients") > 1 || !Com_ServerState())
		Cvar_SetValue("paused", 0.0f);
	else
		Cvar_SetValue("paused", (cl_paused->value != 0.0f ? 0.0f : 1.0f));
}

static void CL_FreezeWorld_f(void) // H2
{
	// Never freeze world in multiplayer.
	if (Cvar_VariableValue("maxclients") > 1 || !Com_ServerState())
		Cvar_SetValue("freezeworldset", 0.0f);
	else
		Cvar_SetValue("freezeworldset", (cl_freezeworld->value != 0.0f ? 0.0f : 1.0f));
}

// Q2 counterpart
void CL_PingServers_f(void)
{
	netadr_t adr;
	char name[32];

	NET_Config(true); // Allow remote

	// Send a broadcast packet.
	Com_Printf("pinging broadcast...\n");

	const cvar_t* noudp = Cvar_Get("noudp", "0", CVAR_NOSET);
	if (!(int)noudp->value)
	{
		adr.type = NA_BROADCAST;
		adr.port = BigShort(PORT_SERVER);
		Netchan_OutOfBandPrint(NS_CLIENT, &adr, va("info %i", PROTOCOL_VERSION));
	}

	const cvar_t* noipx = Cvar_Get("noipx", "0", CVAR_NOSET);
	if (!(int)noipx->value)
	{
		adr.type = NA_BROADCAST_IPX;
		adr.port = BigShort(PORT_SERVER);
		Netchan_OutOfBandPrint(NS_CLIENT, &adr, va("info %i", PROTOCOL_VERSION));
	}

	// Send a packet to each address book entry.
	for (int i = 0; i < 16; i++)
	{
		Com_sprintf(name, sizeof(name), "adr%i", i);

		char* adrstring = Cvar_VariableString(name);
		if (adrstring == NULL || adrstring[0] == 0)
			continue;

		Com_Printf("pinging %s...\n", adrstring);

		if (!NET_StringToAdr(adrstring, &adr))
		{
			Com_Printf("Bad address: %s\n", adrstring);
			continue;
		}

		if (!adr.port)
			adr.port = BigShort(PORT_SERVER);

		Netchan_OutOfBandPrint(NS_CLIENT, &adr, va("info %i", PROTOCOL_VERSION));
	}
}

// Q2 counterpart
static void CL_Userinfo_f(void)
{
	Com_Printf("User info settings:\n");
	Info_Print(Cvar_Userinfo());
}

// Q2 counterpart
// Restart the sound subsystem so it can pick up new parameters and flush all sounds.
void CL_Snd_Restart_f(void)
{
	SndDll_Init();
	se.Init();
	CL_RegisterSounds();
}

// Just sent as a hint to the client that they should drop to full console.
static void CL_Changing_f(void)
{
	// If we are downloading, we don't change! This so we don't suddenly stop downloading a map.
	if (cls.download != NULL)
		return;

	if (cls.demorecording) // H2
		CL_Stop_f();

	cls.state = ca_connected; // Not active anymore, but not disconnected.
	SCR_BeginLoadingPlaque();
	Com_Printf("\nChanging map...\n");
}

H2R_NORETURN void CL_OnServerDisconnected(void) // H2
{
	CL_Drop();
	longjmp(abortframe, -1);
}

H2R_NORETURN void CL_Disconnect_f(void)
{
	SV_Shutdown("Disconnected from server\n", false);
	CL_OnServerDisconnected();
}

static void IgnoreClient(const char* player_id, const qboolean ignore)
{
	int idnum = -1;

	if (player_id[0] >= '0' && player_id[0] <= '9')
	{
		// Numeric values are just slot numbers.
		idnum = Q_atoi(player_id);

		if (idnum < 0 || idnum >= MAX_CLIENTS)
		{
			Com_Printf("Bad client slot: %i\n", idnum);
			return;
		}

		if (!cl.configstrings[CS_PLAYERSKINS + idnum][0])
		{
			Com_Printf("Client %i is not active\n", idnum);
			return;
		}
	}
	else
	{
		// Check for a name match.
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			if (cl.configstrings[CS_PLAYERSKINS + i][0] != 0 && Q_stricmp(cl.clientinfo[i].name, player_id) == 0)
			{
				idnum = i;
				break;
			}
		}

		if (idnum == -1)
		{
			Com_Printf("Userid %s is not on the server\n", player_id);
			return;
		}
	}

	// Update ignore state.
	ignored_players[idnum] = ignore;

	if (ignore)
		Com_Printf("Client %i named %s is now ignored\n", idnum, cl.clientinfo[idnum].name);
	else
		Com_Printf("Client %i named %s is now free to message you\n", idnum, cl.clientinfo[idnum].name);
}

static void CL_Ignore_f(void) // H2
{
	if (Cmd_Argc() == 2)
		IgnoreClient(Cmd_Argv(1), true);
	else
		Com_Printf("Usage : ignore <client name> or <client id number>\n");
}

static void CL_Unignore_f(void)
{
	if (Cmd_Argc() == 2)
		IgnoreClient(Cmd_Argv(1), false);
	else
		Com_Printf("Usage : unignore <client name> or <client id number>\n");
}

static void CL_ListIgnore_f(void) // H2
{
	qboolean have_ignored_clients = false;

	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (!ignored_players[i])
			continue;

		if (cl.configstrings[CS_PLAYERSKINS + i][0])
		{
			Com_Printf("Client id %i named %s is ignored.\n", i, cl.clientinfo[i].name);
			have_ignored_clients = true;
		}
		else
		{
			ignored_players[i] = false;
		}
	}

	if (!have_ignored_clients)
		Com_Printf("No one is Ignored\n");
}

// Q2 counterpart
H2R_NORETURN void CL_Quit_f(void)
{
	CL_Disconnect();
	Com_Quit();
}

// Q2 counterpart
// Called after an ERR_DROP was thrown
void CL_Drop(void)
{
	if (cls.state == ca_uninitialized || cls.state == ca_disconnected)
		return;

	CL_Disconnect();

	// Drop loading plaque unless this is the initial game start
	if (cls.disable_servercount != -1)
		SCR_EndLoadingPlaque(); // Get rid of loading plaque
}

// Q2 counterpart
// We have gotten a challenge from the server, so try and connect.
static void CL_SendConnectPacket(void)
{
	netadr_t adr;

	if (NET_StringToAdr(cls.servername, &adr))
	{
		if (adr.port == 0)
			adr.port = BigShort(PORT_SERVER);

		const int port = (int)Cvar_VariableValue("qport");
		userinfo_modified = false;

		Netchan_OutOfBandPrint(NS_CLIENT, &adr, "connect %i %i %i \"%s\"\n", PROTOCOL_VERSION, port, cls.challenge, Cvar_Userinfo());
	}
	else
	{
		Com_Printf("Bad server address\n");
		cls.connect_time = 0.0f;
	}
}

// Resend a connect message if the last one has timed out.
static void CL_CheckForResend(void)
{
	netadr_t adr;

	// If the local server is running and we aren't, then connect.
	if (cls.state == ca_disconnected && Com_ServerState())
	{
		cls.state = ca_connecting;
		strncpy_s(cls.servername, sizeof(cls.servername), "localhost", sizeof(cls.servername) - 1); //mxd. strncpy -> strncpy_s
		CL_SendConnectPacket(); // We don't need a challenge on the localhost.

		return;
	}

	// H2. Update predict cvars.
	if (cls.state >= ca_connected)
	{
		if (strcmp(cls.servername, "localhost") == 0)
		{
			if (cl_predict->value != cl_predict_local->value)
			{
				Cvar_SetValue("cl_predict", cl_predict->value);
				cl_predict_local->value = cl_predict->value;
				Cvar_SetValue("cl_predict_local", cl_predict_local->value);
			}
		}
		else if (cl_predict->value != cl_predict_remote->value)
		{
			Cvar_SetValue("cl_predict", cl_predict->value);
			cl_predict_remote->value = cl_predict->value;
			Cvar_SetValue("cl_predict_remote", cl_predict_remote->value);
		}
	}

	// Resend if we haven't gotten a reply yet.
	if (cls.state != ca_connecting || (float)cls.realtime - cls.connect_time < 3000.0f)
		return;

	if (!NET_StringToAdr(cls.servername, &adr))
	{
		Com_Printf("Bad server address\n");
		cls.state = ca_disconnected;

		return;
	}

	if (adr.port == 0)
		adr.port = BigShort(PORT_SERVER);

	// H2. Update cl_predict.
	if (strcmp(cls.servername, "localhost") == 0)
	{
		if (cl_predict->value != cl_predict_local->value)
		{
			cl_predict->value = cl_predict_local->value;
			Cvar_SetValue("cl_predict", cl_predict->value);
		}
	}
	else
	{
		if (cl_predict->value != cl_predict_remote->value)
		{
			cl_predict->value = cl_predict_remote->value;
			Cvar_SetValue("cl_predict", cl_predict->value);
		}
	}

	cls.connect_time = (float)cls.realtime; // For retransmit requests

	Com_Printf("Connecting to %s...\n", cls.servername);
	Netchan_OutOfBandPrint(NS_CLIENT, &adr, "getchallenge\n");
}

// Q2 counterpart
static void CL_Connect_f(void)
{
	if (Cmd_Argc() != 2)
	{
		Com_Printf("Usage: connect <server>\n");
		return;
	}

	if (Com_ServerState())
		SV_Shutdown("Server quit\n", false); // If running a local server, kill it and reissue.

	const char* server = Cmd_Argv(1);
	NET_Config(true); // Allow remote.
	CL_Disconnect();

	cls.state = ca_connecting;
	strncpy_s(cls.servername, sizeof(cls.servername), server, sizeof(cls.servername) - 1); //mxd. strncpy -> strncpy_s
	cls.connect_time = -99999; // CL_CheckForResend() will fire immediately.
}

// The server is changing levels.
static void CL_Reconnect_f(void)
{
	// If we are downloading, we don't change! This so we don't suddenly stop downloading a map.
	if (cls.download != NULL)
		return;

	if (cls.state == ca_connected)
	{
		se.StopAllSounds_Sounding(); // H2
		CL_ClearSkeletalEntities(); // H2

		Com_Printf("reconnecting...\n");

		MSG_WriteByte(&cls.netchan.message, clc_stringcmd);
		MSG_WriteString(&cls.netchan.message, "new");
	}
	else if (cls.servername[0] != 0)
	{
		if (cls.state >= ca_connected)
		{
			CL_Disconnect();
			cls.connect_time = (float)(cls.realtime - 1500);
		}
		else
		{
			cls.connect_time = -99999.0f; // Fire immediately.
		}

		Com_Printf("reconnecting...\n");
		cls.state = ca_connecting;
	}
}

// Send the rest of the command line over as an unconnected command.
static void CL_Rcon_f(void)
{
	char message[1024];
	netadr_t to;

	if (rcon_client_password->string == NULL || rcon_client_password->string[0] == 0) // H2: extra strlen check. //mxd. strlen(str) -> str[0] check.
	{
		Com_Printf("You must set 'rcon_password' before\nissuing an rcon command.\n");
		return;
	}

	message[0] = (char)255;
	message[1] = (char)255;
	message[2] = (char)255;
	message[3] = (char)255;
	message[4] = 0;

	NET_Config(true); // Allow remote

	strcat_s(message, sizeof(message), "rcon ");
	strcat_s(message, sizeof(message), rcon_client_password->string);
	strcat_s(message, sizeof(message), " ");

	for (int i = 1; i < Cmd_Argc(); i++)
	{
		strcat_s(message, sizeof(message), Cmd_Argv(i));
		strcat_s(message, sizeof(message), " ");
	}

	if (cls.state >= ca_connected)
	{
		to = cls.netchan.remote_address;
	}
	else
	{
		if (rcon_address->string[0] == 0) //mxd. strlen(str) -> str[0] check.
		{
			Com_Printf("You must either be connected,\nor set the 'rcon_address' cvar\nto issue rcon commands\n");
			return;
		}

		if (!NET_StringToAdr(rcon_address->string, &to)) //mxd. Added sanity check.
		{
			Com_Printf("Invalid rcon address '%s'\n", rcon_address->string);
			return;
		}

		if (to.port == 0)
			to.port = BigShort(PORT_SERVER);
	}

	NET_SendPacket(NS_CLIENT, (int)strlen(message) + 1, message, &to);
}

char* CL_GetConfigPath(void) //mxd
{
	static char cfg_name[MAX_QPATH];
	Com_sprintf(cfg_name, sizeof(cfg_name), "%s/config/%s.cfg", FS_Gamedir(), player_name->string);

	return cfg_name;
}

void CL_SaveConfig_f(void) // H2
{
	if (cls.state == ca_uninitialized)
		return;

	if (player_name == NULL || player_name->string[0] == 0) //mxd. strlen(str) -> str[0] check.
	{
		Com_Printf("ERROR: Set the 'player_name' cvar to use as config filename!\n");
		return;
	}

	char* cfg_name = CL_GetConfigPath(); //mxd
	FS_CreatePath(cfg_name);

	FILE* f;
	if (fopen_s(&f, cfg_name, "w") != 0) //mxd. fopen -> fopen_s
	{
		Com_Printf("Couldn't write '%s'.\n", cfg_name);
		return;
	}

	fprintf(f, "// Custom saved config from "GAME_FULLNAME"\n\n"); //mxd. Use define.
	fprintf(f, "echo Executing custom saved config from "GAME_FULLNAME"\n\n"); //mxd. Use define.
	fprintf(f, "unbindall\n");

	fprintf(f, "\n// Binds:\n"); //mxd
	Key_WriteBindings(f);

	fprintf(f, "\n// Double binds:\n"); //mxd
	Key_WriteBindings_Double(f);

	fprintf(f, "\n// Cvars:\n"); //mxd
	Cvar_WriteVariables(f);

	fprintf(f, "\necho ** Done **");
	fclose(f);

	Com_Printf("Saved personal config file to '%s'.\n", cfg_name);
}

static void CL_Setenv_f(void)
{
	Com_Printf("setenv command not implemented\n"); //TODO: implement? What can it be used for?
}

void CL_RequestNextDownload(void)
{
#define PLAYER_MULT	5

// ENV_CNT is map load, ENV_CNT + 1 is first env map.
#define ENV_CNT		(CS_PLAYERSKINS + MAX_CLIENTS * PLAYER_MULT)
#define TEXTURE_CNT	(ENV_CNT + 7) // Q2: ENV_CNT + 13

	typedef struct //mxd. Part of on-disk fmodel structure...
	{
		fmdl_blockheader_t block_header;
		fmheader_t header;
		fmdl_blockheader_t block_skins;
		char skins[1][MAX_FRAMENAME]; // Variable-sized.
	} dfmdl_t;

	static const char* env_suf[] = { "rt", "bk", "lf", "ft", "up", "dn" };
	char fn[MAX_OSPATH];

	if (cls.state != ca_connected)
		return;

	if (!(int)allow_download->value && precache_check < ENV_CNT)
		precache_check = ENV_CNT;

	if (precache_check == CS_MODELS) // Confirm map.
	{
		precache_check += 2; // 0 isn't used.

		if ((int)allow_download_maps->value && !CL_CheckOrDownloadFile(cl.configstrings[CS_MODELS + 1]))
			return; // Started a download.
	}

	if (precache_check < CS_MODELS + MAX_MODELS) // H2: no 'precache_check >= CS_MODELS' check.
	{
		if ((int)allow_download_models->value)
		{
			for (; (precache_check < CS_MODELS + MAX_MODELS && cl.configstrings[precache_check][0]); precache_check++)
			{
				if (cl.configstrings[precache_check][0] == '*' || cl.configstrings[precache_check][0] == '#')
					continue;

				if (precache_model_skin == 0)
				{
					precache_model_skin = 1;

					if (!CL_CheckOrDownloadFile(cl.configstrings[precache_check]))
						return; // Started a download
				}

				// Checking for skins in the model.
				if (precache_model == NULL)
					FS_LoadFile(cl.configstrings[precache_check], (void**)&precache_model);

				if (precache_model != NULL)
				{
					const dfmdl_t* fmdl = (dfmdl_t*)precache_model;

					for (; precache_model_skin - 1 < fmdl->header.num_skins; precache_model_skin++)
						if (!CL_CheckOrDownloadFile(fmdl->skins[precache_model_skin - 1]))
							return; // Started a download.

					FS_FreeFile(precache_model);
					precache_model = NULL;
				}

				precache_model_skin = 0;
			}
		}

		precache_check = CS_SOUNDS;
	}

	if (precache_check < CS_SOUNDS + MAX_SOUNDS) // H2: no 'precache_check >= CS_SOUNDS' check.
	{
		if ((int)allow_download_sounds->value)
		{
			if (precache_check == CS_SOUNDS)
				precache_check++; // Zero is blank.

			for (; precache_check < CS_SOUNDS + MAX_SOUNDS && cl.configstrings[precache_check][0] != 0; precache_check++)
			{
				if (cl.configstrings[precache_check][0] == '*')
					continue;

				Com_sprintf(fn, sizeof(fn), "sound/%s", cl.configstrings[precache_check]);

				if (!CL_CheckOrDownloadFile(fn))
					return; // Started a download.
			}
		}

		precache_check = CS_IMAGES;
	}

	if (precache_check < CS_IMAGES + MAX_IMAGES)
	{
		if (precache_check == CS_IMAGES)
			precache_check++; // Zero is blank.

		for (; precache_check < CS_IMAGES + MAX_IMAGES && cl.configstrings[precache_check][0] != 0; precache_check++)
		{
			Com_sprintf(fn, sizeof(fn), "pics/%s", cl.configstrings[precache_check]); // Q2: "pics/%s.pcx"

			if (!CL_CheckOrDownloadFile(fn))
				return; // Started a download.
		}

		precache_check = CS_PLAYERSKINS;
	}

	if (precache_check < ENV_CNT)
	{
		while (precache_check < ENV_CNT)
		{
			const int i = (precache_check - CS_PLAYERSKINS) / PLAYER_MULT;
			int n = (precache_check - CS_PLAYERSKINS) % PLAYER_MULT;

			if (cl.configstrings[CS_PLAYERSKINS + i][0] == 0)
			{
				precache_check = CS_PLAYERSKINS + (i + 1) * PLAYER_MULT;
				continue;
			}

			char* p = strchr(cl.configstrings[CS_PLAYERSKINS + i], '\\');
			if (p != NULL)
				p++;
			else
				p = cl.configstrings[CS_PLAYERSKINS + i];

			char model_name[MAX_QPATH];
			strcpy_s(model_name, sizeof(model_name), p);

			p = strchr(model_name, '/');
			if (p == NULL)
				p = strchr(model_name, '\\');

			char skin_name[MAX_QPATH];
			if (p != NULL)
			{
				strcpy_s(skin_name, sizeof(skin_name), p + 1);
				*p = 0;
			}
			else
			{
				strcpy_s(skin_name, sizeof(skin_name), model_name); // H2
				strcpy_s(model_name, sizeof(model_name), "male"); // H2
			}

			if (n++ == 0) // H2. Skin.
			{
				Com_sprintf(fn, sizeof(fn), "players/%s/%s.m8", model_name, skin_name);
				if (!CL_CheckOrDownloadFile(fn))
				{
					precache_check++;
					return; // Started a download.
				}
			}

			if (n++ == 1) // H2. Model icon.
			{
				Com_sprintf(fn, sizeof(fn), "players/%s/%s_i.m8", model_name, skin_name);
				if (!CL_CheckOrDownloadFile(fn))
				{
					precache_check++;
					return; // Started a download.
				}
			}

			if (n++ == 2) // H2. Damaged skin.
			{
				Com_sprintf(fn, sizeof(fn), "players/%s/%sDmg.m8", model_name, skin_name);
				if (!CL_CheckOrDownloadFile(fn))
				{
					precache_check++;
					return; // Started a download.
				}
			}

			if (n++ == 3) // H2. Reflect texture.
			{
				Com_sprintf(fn, sizeof(fn), "players/%s/reflect.m8", model_name);
				if (!CL_CheckOrDownloadFile(fn))
				{
					precache_check++;
					return; // Started a download.
				}
			}

			if (n == 4) // H2. Model.
			{
				Com_sprintf(fn, sizeof(fn), "players/%s/tris.fm", model_name);
				if (!CL_CheckOrDownloadFile(fn))
				{
					precache_check++;
					return; // Started a download.
				}
			}

			// Move on to the next model.
			precache_check = CS_PLAYERSKINS + (i + 1) * PLAYER_MULT;
		}

		// Precache phase completed.
		precache_check = ENV_CNT;
	}

	if (precache_check == ENV_CNT)
	{
		precache_check++;

		uint map_checksum; // For detecting cheater maps.
		CM_LoadMap(cl.configstrings[CS_MODELS + 1], true, &map_checksum);

		const uint cs_checksum = Q_atoi(cl.configstrings[CS_MAPCHECKSUM]);
		if (map_checksum != cs_checksum)
			Com_Error(ERR_DROP, "Local map version differs from server: %i != %i\n", map_checksum, cs_checksum);
	}

	if (precache_check < TEXTURE_CNT)
	{
		if ((int)allow_download->value && (int)allow_download_maps->value)
		{
			for (int i = 0; precache_check < TEXTURE_CNT; i++, precache_check++)
			{
				Com_sprintf(fn, sizeof(fn), "pics/skies/%s%s.m8", cl.configstrings[CS_SKY], env_suf[i]);
				if (!CL_CheckOrDownloadFile(fn))
					return; // Started a download.
			}
		}

		precache_check = TEXTURE_CNT;
	}

	if (precache_check == TEXTURE_CNT)
	{
		precache_check++;
		precache_tex = 0;
	}

	// Confirm existence of textures, download any that don't exist.
	if (precache_check == TEXTURE_CNT + 1)
	{
		if ((int)allow_download->value && (int)allow_download_maps->value)
		{
			for (; precache_tex < numtexinfo; precache_tex++)
			{
				Com_sprintf(fn, sizeof(fn), "textures/%s.m8", map_surfaces[precache_tex].name); //mxd. sprintf -> Com_sprintf
				if (!CL_CheckOrDownloadFile(fn))
					return; // Started a download.
			}
		}

		precache_check = TEXTURE_CNT + 999;
	}

	memset(skeletal_joints, 0, sizeof(skeletal_joints)); // H2
	memset(joint_nodes, 0, sizeof(joint_nodes)); // H2

	CL_RegisterSounds();
	CL_PrepRefresh();

	MSG_WriteByte(&cls.netchan.message, clc_stringcmd);
	MSG_WriteString(&cls.netchan.message, va("begin %i\n", precache_spawncount));
	cls.force_packet = true; // YQ2
}

// The server will send this command right before allowing the client into the server.
static void CL_Precache_f(void)
{
	precache_spawncount = Q_atoi(Cmd_Argv(1)); // H2

	if (Cmd_Argc() < 2 || strcmp(cls.servername, "localhost") == 0 || game_downloadable_type->value == 0.0f || Cvar_VariableValue("server_machine") != 0.0f) // H2: extra checks
	{
		cls.disable_screen = true;

		uint map_checksum; // For detecting cheater maps.
		CM_LoadMap(cl.configstrings[CS_MODELS + 1], true, &map_checksum);

		if (map_checksum != (uint)Q_atoi(cl.configstrings[CS_MAPCHECKSUM])) // H2
			Com_Error(ERR_DROP, "Local map version differs from server: %i != '%s'\n", map_checksum, cl.configstrings[CS_MAPCHECKSUM]);

		if (cl.configstrings[CS_MODELS + 1][0] != 0) // H2
		{
			CL_RegisterSounds();
			memset(&skeletal_joints, 0, sizeof(skeletal_joints));
			memset(&joint_nodes, 0, sizeof(joint_nodes));
			CL_PrepRefresh();

			if (precache_spawncount != -1)
			{
				MSG_WriteByte(&cls.netchan.message, clc_stringcmd);
				MSG_WriteString(&cls.netchan.message, va("begin %i\n", precache_spawncount));
			}
		}
	}
	else
	{
		precache_check = CS_MODELS;
		precache_model = NULL;
		precache_model_skin = 0;

		CL_RequestNextDownload();
	}
}

// Q2 counterpart
// Handle a reply from a ping.
static void CL_ParseStatusMessage(void)
{
	char* s = MSG_ReadString(&net_message);

	Com_Printf("%s\n", s);
	M_AddToServerList(&net_from, s);
}

// Q2 counterpart
// Responses to broadcasts, etc.
static void CL_ConnectionlessPacket(void)
{
	MSG_BeginReading(&net_message);
	MSG_ReadLong(&net_message);	// Skip the -1

	char* s = MSG_ReadStringLine(&net_message);
	Cmd_TokenizeString(s, false);

	char* c = Cmd_Argv(0);
	Com_Printf("%s: %s\n", NET_AdrToString(&net_from), c);

	if (strcmp(c, "client_connect") == 0) // Server connection.
	{
		if (cls.state == ca_connected)
		{
			Com_Printf("Dup connect received.  Ignored.\n");
			return;
		}

		Netchan_Setup(NS_CLIENT, &cls.netchan, &net_from, cls.quakePort);
		MSG_WriteChar(&cls.netchan.message, clc_stringcmd);
		MSG_WriteString(&cls.netchan.message, "new");
		cls.state = ca_connected;
	}
	else if (strcmp(c, "info") == 0) // Server responding to a status broadcast.
	{
		CL_ParseStatusMessage();
	}
	else if (strcmp(c, "cmd") == 0) // Remote command from gui front end.
	{
		if (!NET_IsLocalAddress(&net_from))
		{
			Com_Printf("Command packet from remote host.  Ignored.\n");
			return;
		}

		Cbuf_AddText(MSG_ReadString(&net_message));
		Cbuf_AddText("\n");
	}
	else if (strcmp(c, "print") == 0) // Print command from somewhere.
	{
		Com_Printf("%s", MSG_ReadString(&net_message));
	}
	else if (strcmp(c, "ping") == 0) // Ping from somewhere.
	{
		Netchan_OutOfBandPrint(NS_CLIENT, &net_from, "ack");
	}
	else if (strcmp(c, "challenge") == 0) // Challenge from the server we are connecting to.
	{
		cls.challenge = Q_atoi(Cmd_Argv(1));
		CL_SendConnectPacket();
	}
	else if (strcmp(c, "echo") == 0) // Echo request from server.
	{
		Netchan_OutOfBandPrint(NS_CLIENT, &net_from, "%s", Cmd_Argv(1));
	}
	else
	{
		Com_Printf("Unknown command.\n");
	}
}

// Q2 counterpart
void CL_ReadPackets(void)
{
	while (NET_GetPacket(NS_CLIENT, &net_from, &net_message))
	{
		// Remote command packet.
		if (*(int*)net_message.data == -1)
		{
			CL_ConnectionlessPacket();
			continue;
		}

		// Dump it if not connected.
		if (cls.state == ca_disconnected || cls.state == ca_connecting)
			continue;

		if (net_message.cursize < 8)
		{
			Com_Printf("%s: Runt packet\n", NET_AdrToString(&net_from));
			continue;
		}

		// Packet from server.
		if (!NET_CompareAdr(&net_from, &cls.netchan.remote_address))
		{
			Com_DPrintf("%s:sequenced packet without connection\n", NET_AdrToString(&net_from));
			continue;
		}

		if (Netchan_Process(&cls.netchan, &net_message))
			CL_ParseServerMessage();
	}

	// Check timeout.
	if (cls.state >= ca_connected && cls.realtime - cls.netchan.last_received > (int)(cl_timeout->value * 1000))
	{
		if (++cl.timeoutcount > 5) // timeoutcount saves debugger
		{
			Com_Printf("\nServer connection timed out.\n");
			CL_Disconnect();
		}
	}
	else
	{
		cl.timeoutcount = 0;
	}
}

static void CL_InitLocal(void)
{
	cls.state = ca_disconnected;
	cls.realtime = Sys_Milliseconds();
	cl.lastanimtime = 0; // H2

	CL_InitInput();
	ClearIgnoredPlayersList();

	adr0 = Cvar_Get("adr0", "", CVAR_ARCHIVE);
	adr1 = Cvar_Get("adr1", "", CVAR_ARCHIVE);
	adr2 = Cvar_Get("adr2", "", CVAR_ARCHIVE);
	adr3 = Cvar_Get("adr3", "", CVAR_ARCHIVE);
	adr4 = Cvar_Get("adr4", "", CVAR_ARCHIVE);
	adr5 = Cvar_Get("adr5", "", CVAR_ARCHIVE);
	adr6 = Cvar_Get("adr6", "", CVAR_ARCHIVE);
	adr7 = Cvar_Get("adr7", "", CVAR_ARCHIVE);
	adr8 = Cvar_Get("adr8", "", CVAR_ARCHIVE);

	// Register our variables
	cl_stereo_separation = Cvar_Get("cl_stereo_separation", "0.4", CVAR_ARCHIVE);
	cl_stereo = Cvar_Get("cl_stereo", "0", 0);
	cl_add_blend = Cvar_Get("cl_blend", "1", 0);
	cl_add_lights = Cvar_Get("cl_lights", "1", 0);
	cl_add_particles = Cvar_Get("cl_particles", "1", 0);
	cl_add_entities = Cvar_Get("cl_entities", "1", 0);
	cl_footsteps = Cvar_Get("cl_footsteps", "1", 0);
	cl_noskins = Cvar_Get("cl_noskins", "0", 0);
	cl_autoskins = Cvar_Get("cl_autoskins", "0", 0);
	cl_predict = Cvar_Get("cl_predict", "0", 0);
	cl_predict_local = Cvar_Get("cl_predict_local", "0", 0); // H2
	cl_predict_remote = Cvar_Get("cl_predict_remote", "1", 0); // H2
	cl_predict_lag = Cvar_Get("cl_predict_lag", "0.0", 0); // H2
	cl_maxfps = Cvar_Get("cl_maxfps", "30", 0); // H2_1.07: "30" -> "60".

	cl_frametime = Cvar_Get("cl_frametime", "0.0", 0);
	cl_yawspeed = Cvar_Get("cl_yawspeed", "70", CVAR_ARCHIVE);
	cl_pitchspeed = Cvar_Get("cl_pitchspeed", "150", 0);
	cl_anglespeedkey = Cvar_Get("cl_anglespeedkey", "1.5", 0);

	cl_run = Cvar_Get("cl_run", "0", CVAR_ARCHIVE);
	freelook = Cvar_Get("freelook", "0", CVAR_ARCHIVE);
	lookspring = Cvar_Get("lookspring", "0", CVAR_ARCHIVE);
	lookstrafe = Cvar_Get("lookstrafe", "0", CVAR_ARCHIVE);
	mouse_sensitivity_x = Cvar_Get("mouse_sensitivity_x", "4", CVAR_ARCHIVE);
	mouse_sensitivity_y = Cvar_Get("mouse_sensitivity_y", "4", CVAR_ARCHIVE);
	doubletap_speed = Cvar_Get("doubletap_speed", "100", CVAR_ARCHIVE);

	m_pitch = Cvar_Get("m_pitch", "0.022", CVAR_ARCHIVE);
	m_yaw = Cvar_Get("m_yaw", "0.022", 0);
	m_forward = Cvar_Get("m_forward", "1", 0);
	m_side = Cvar_Get("m_side", "1", 0);

	cl_shownet = Cvar_Get("cl_shownet", "0", 0);
	cl_showmiss = Cvar_Get("cl_showmiss", "0", 0);
	cl_showclamp = Cvar_Get("showclamp", "0", 0);
	cl_timeout = Cvar_Get("cl_timeout", "120", 0);
	cl_paused = Cvar_Get("paused", "0", 0);
	cl_freezeworld = Cvar_Get("freezeworldset", "0", 0);
	cl_timedemo = Cvar_Get("timedemo", "0", 0);
	cl_no_middle_text = Cvar_Get("no_middle_text", "0", CVAR_ARCHIVE);

	rcon_client_password = Cvar_Get("rcon_password", "", 0);
	rcon_address = Cvar_Get("rcon_address", "", 0);
	cl_lightlevel = Cvar_Get("r_lightlevel", "0", 0);

	// H2:
	shownames = Cvar_Get("shownames", "0", CVAR_ARCHIVE);
	r_detail = Cvar_Get("r_detail", "3.0", CVAR_ARCHIVE); // H2_1.07: "2.0" -> "3".
	cl_showcaptions = Cvar_Get("cl_showcaptions", "1", 0);
	cl_doautoaim = Cvar_Get("cl_doautoaim", "0.0", CVAR_ARCHIVE);
	cl_camera_combat = Cvar_Get("cl_camera_combat", "0.0", 0);
	cl_camera_clipdamp = Cvar_Get("cl_camera_clipdamp", "1.0", 0);
	cl_camera_dampfactor = Cvar_Get("cl_camera_dampfactor", "1.0", CVAR_ARCHIVE); // H2_1.07: "0.2" -> "1.0".
	cl_camera_fpoffs = Cvar_Get("cl_camera_fpoffs", "0.0", 0);
	cl_camera_freeze = Cvar_Get("cl_camera_freeze", "0.0", 0);
	cl_camera_under_surface = Cvar_Get("cl_camera_under_surface", "0.0", 0);
	cl_camera_viewdist = Cvar_Get("cl_camera_viewdist", "126.0", 0);
	cl_camera_viewmin = Cvar_Get("cl_camera_viewmin", "70.0", 0);
	cl_camera_viewmax = Cvar_Get("cl_camera_viewmax", "200.0", 0);
	cl_camera_fpmode = Cvar_Get("cl_camera_fpmode", "0", CVAR_ARCHIVE);
	cl_camera_fptrans = Cvar_Get("cl_camera_fptrans", "0.3", CVAR_ARCHIVE); // H2_1.07: "0.3" -> "0".
	cl_camera_fpdist = Cvar_Get("cl_camera_fpdist", "16.0", CVAR_ARCHIVE);
	cl_camera_fpheight = Cvar_Get("cl_camera_fpheight", "0.0", CVAR_ARCHIVE);
	cl_playertrans = Cvar_Get("cl_playertrans", "1.0", CVAR_ARCHIVE);
	EAX_preset = Cvar_Get("EAX_preset", "0", 0);
	EAX_default = Cvar_Get("EAX_default", "0", 0);
	quake_amount = Cvar_Get("quake_amount", "0.0", 0);
	cl_fx_dll = Cvar_Get("cl_fx_dll", "Client Effects", 0);
	cl_cinematicfreeze = Cvar_Get("cl_cinematicfreeze", "0", 0);
	sc_framerate = Cvar_Get("sc_framerate", "20", CVAR_ARCHIVE); // H2_1.07: "20" -> "60".

	// userinfo
	info_password = Cvar_Get("password", "", CVAR_USERINFO);
	player_name = Cvar_Get("name", "Corvus", CVAR_ARCHIVE | CVAR_USERINFO);
	skin = Cvar_Get("skin", "Corvus", CVAR_ARCHIVE | CVAR_USERINFO);
	skindir = Cvar_Get("skindir", "players", CVAR_ARCHIVE);
	playerdir = Cvar_Get("playerdir", "players", CVAR_ARCHIVE);
	rate = Cvar_Get("rate", "2800", CVAR_ARCHIVE | CVAR_USERINFO); // H2_1.07: "2800" -> "25000".
	msg = Cvar_Get("msg", "1", CVAR_ARCHIVE | CVAR_USERINFO);
	fov = Cvar_Get("fov", "75", CVAR_ARCHIVE | CVAR_USERINFO); // H2_1.07: "75" -> "90".
	autoweapon = Cvar_Get("autoweapon", "1", CVAR_ARCHIVE | CVAR_USERINFO);
	colour_obituary = Cvar_Get("colour_obituary", "20", CVAR_ARCHIVE);
	colour_chat = Cvar_Get("colour_chat", "22", CVAR_ARCHIVE); // H2_1.07: "22" -> "20".
	colour_names = Cvar_Get("colour_names", "7", CVAR_ARCHIVE); // H2_1.07: "7" -> "20".
	colour_teamchat = Cvar_Get("colour_teamchat", "23", CVAR_ARCHIVE); // H2_1.07: "23" -> "20".
	colour_level = Cvar_Get("colour_level", "16", CVAR_ARCHIVE); // H2_1.07: "16" -> "20".
	colour_game = Cvar_Get("colour_game", "17", CVAR_ARCHIVE); // H2_1.07: "17" -> "20".
	game_downloadable_type = Cvar_Get("game_downloadable_type", "0", 0);

	// Register our commands
	Cmd_AddCommand("cmd", CL_ForwardToServer_f);
	Cmd_AddCommand("pause", CL_Pause_f);

	// H2:
	Cmd_AddCommand("freezeworld", CL_FreezeWorld_f);
	Cmd_AddCommand("pingservers", CL_PingServers_f);
	Cmd_AddCommand("userinfo", CL_Userinfo_f);
	Cmd_AddCommand("snd_restart", CL_Snd_Restart_f);
	Cmd_AddCommand("changing", CL_Changing_f);
	Cmd_AddCommand("disconnect", CL_Disconnect_f);
	Cmd_AddCommand("record", CL_Record_f);
	Cmd_AddCommand("stop", CL_Stop_f);
	Cmd_AddCommand("ignore", CL_Ignore_f);
	Cmd_AddCommand("unignore", CL_Unignore_f);
	Cmd_AddCommand("listignore", CL_ListIgnore_f);
	Cmd_AddCommand("quit", CL_Quit_f);
	Cmd_AddCommand("exit", CL_Quit_f);
	Cmd_AddCommand("connect", CL_Connect_f);
	Cmd_AddCommand("reconnect", CL_Reconnect_f);
	Cmd_AddCommand("rcon", CL_Rcon_f);
	Cmd_AddCommand("savecfg", CL_SaveConfig_f);
	Cmd_AddCommand("setenv", CL_Setenv_f);
	Cmd_AddCommand("precache", CL_Precache_f);
	//Cmd_AddCommand("config", CL_Config_f); //mxd. Skip GameSpy config logic.

	// Forward to server commands
	Cmd_AddCommand("kill", NULL);
	Cmd_AddCommand("use", NULL);
	Cmd_AddCommand("say", NULL);
	Cmd_AddCommand("say_team", NULL);
	Cmd_AddCommand("info", NULL);
	Cmd_AddCommand("score", NULL); //mxd
	Cmd_AddCommand("players", NULL); //mxd
	Cmd_AddCommand("weapnext", NULL);
	Cmd_AddCommand("weapprev", NULL);
	Cmd_AddCommand("weaplast", NULL); //mxd
	Cmd_AddCommand("defnext", NULL); //mxd
	Cmd_AddCommand("defprev", NULL); //mxd

	// H2:
	Cmd_AddCommand("spawn", NULL);
	Cmd_AddCommand("crazymonsters", NULL);
	Cmd_AddCommand("angermonsters", NULL);
	Cmd_AddCommand("showcoords", NULL);
	Cmd_AddCommand("gameversion", NULL); //mxd

	// H2 cheats.
	Cmd_AddCommand("playbetter", NULL);
	Cmd_AddCommand("kiwi", NULL);
	Cmd_AddCommand("victor", NULL);
	Cmd_AddCommand("suckitdown", NULL);
	Cmd_AddCommand("twoweeks", NULL);
	Cmd_AddCommand("meatwagon", NULL);

	//mxd. Also classic cheats.
	Cmd_AddCommand("god", NULL);
	Cmd_AddCommand("noclip", NULL);
	Cmd_AddCommand("notarget", NULL);
	Cmd_AddCommand("give", NULL);
	Cmd_AddCommand("powerup", NULL);
	Cmd_AddCommand("killmonsters", NULL);
}

// Writes key bindings and archived cvars to config.cfg.
static void CL_WriteConfiguration(void)
{
	if (cls.state == ca_uninitialized)
		return;

	char path[MAX_QPATH];
	Com_sprintf(path, sizeof(path), "%s/config.cfg", FS_Userdir()); // FS_Gamedir in Q2
	FS_CreatePath(path); // H2

	FILE* f;
	if (fopen_s(&f, path, "w") != 0) //mxd. fopen -> fopen_s
	{
		Com_Printf("Couldn't write '%s'.\n", path);
		return;
	}

	fprintf(f, "// Generated by "GAME_FULLNAME", do not modify\n"); // H2: different message //mxd. Use define.
	Key_WriteBindings(f);
	Key_WriteBindings_Double(f); // H2
	Cvar_WriteVariables(f);
	fclose(f);
}

static void CL_StorePlayerInventoryInfo(void) // H2
{
	for (int i = 0; i < cl.frame.playerstate.NoOfItems; i++)
	{
		const int ic = cl.frame.playerstate.inventory_changes[i];
		const int ir = cl.frame.playerstate.inventory_remaining[i];

		cl.playerinfo.pers.inventory.Items[ic] = ir;
	}
}

static float GetFrameModifier(const float frametime) // H2
{
	#define NUM_FRAMETIMES	16

	static int frametime_index = 0;
	static float frametimes[NUM_FRAMETIMES];

	frametimes[(frametime_index++) & (NUM_FRAMETIMES - 1)] = frametime;

	float result = 0.0f;
	for (int i = 0; i < NUM_FRAMETIMES; i++)
		result += frametimes[i];

	result *= (sc_framerate->value / NUM_FRAMETIMES);

	if (result > 0.0f)
	{
		result = 1.0f / result;

		if (result < 0.25f)
			return 0.25f;

		if (result > 1.0f)
		{
			result = result * 0.25f + 1.0f;
			return min(2.0f, result);
		}

		return result;
	}

	return 1.0f;
}

typedef struct
{
	char* name;
	char* value;
	cvar_t* var;
} cheatvar_t;

// H2: missing 'sw_draworder', 'gl_lightmap' and 'gl_saturatelighting'.
cheatvar_t cheatvars[] =
{
	{ "timescale", "1", NULL },
	{ "timedemo", "0", NULL },
	{ "r_drawworld", "1", NULL },
	{ "cl_testlights", "0", NULL },
	{ "r_fullbright", "0", NULL },
	{ "r_drawflat", "0", NULL },
	{ "paused", "0", NULL },
	{ "freezeworldset", "0", NULL }, // H2
	{ "fixedtime", "0", NULL },
	{ NULL, NULL, NULL }
};

// Q2 counterpart
static void CL_FixCvarCheats(void)
{
	static int numcheatvars = 0; //mxd. Made local/static.

	// Can cheat in single player
	if (strcmp(cl.configstrings[CS_MAXCLIENTS], "1") == 0 || cl.configstrings[CS_MAXCLIENTS][0] == 0)
		return;

	// Find all the cvars if we haven't done it yet.
	if (numcheatvars == 0)
	{
		while (cheatvars[numcheatvars].name != NULL)
		{
			cheatvars[numcheatvars].var = Cvar_Get(cheatvars[numcheatvars].name, cheatvars[numcheatvars].value, 0);
			numcheatvars++;
		}
	}

	// Make sure they are all set to the proper values.
	cheatvar_t* var = cheatvars;
	for (int i = 0; i < numcheatvars; i++, var++)
		if (strcmp(var->var->string, var->value) != 0)
			Cvar_Set(var->name, var->value);
}

void CL_Frame(const int packetdelta, const int renderdelta, const int timedelta, qboolean packetframe, const qboolean renderframe)
{
	static int lasttimecalled;

	if ((int)dedicated->value)
		return;

	// Decide the simulation time.
	cls.nframetime = (float)packetdelta / 1000000.0f;
	cls.rframetime = (float)renderdelta / 1000000.0f;
	cls.framemodifier = GetFrameModifier(cls.rframetime); // H2
	cls.realtime = curtime;
	cl.time += timedelta / 1000;
	camera_timer += timedelta / 1000; // H2

	//mxd. Skip cl_frametime logic: now done in SCR_DrawFramecounter().

	// Don't extrapolate too far ahead.
	cls.nframetime = min(0.5f, cls.nframetime);
	cls.rframetime = min(0.5f, cls.rframetime);

	// If in the debugger last frame, don't timeout.
	if (timedelta > 5000000)
		cls.netchan.last_received = Sys_Milliseconds();

	// Don't throttle too much when connecting / loading.
	if (!(int)cl_timedemo->value && cls.state == ca_connected && packetdelta > 100000)
		packetframe = true;

	// Update input stuff.
	if (packetframe || renderframe)
	{
		// Fetch results from server.
		CL_ReadPackets();

		if ((int)cl_predict->value) // H2
			CL_StorePlayerInventoryInfo();

		IN_Update(); // YQ2		// Run SDL3 message loop.
		Cbuf_Execute();			// Process console commands.
		CL_FixCvarCheats();		// Fix any cheat cvars.

		if (cls.state > ca_connecting) // YQ2
			CL_RefreshCmd();
		else
			CL_RefreshMove();
	}

	if (cls.force_packet || userinfo_modified)
	{
		packetframe = true;
		cls.force_packet = false;
	}

	if (packetframe)
	{
		CL_SendCmd();			// Send intentions now.
		CL_CheckForResend();	// Resend a connection request if necessary.
	}

	if (renderframe)
	{
		// Predict all unacknowledged movements.
		CL_PredictMovement();

		// Allow rendering DLL change.
		VID_CheckChanges();
		if (!cl.refresh_prepped && cls.state == ca_active)
			CL_PrepRefresh();

		// Update the screen.
		SCR_RunConsole();
		SCR_UpdateScreen();

		// Update audio and music.
		se.Update(cl.refdef.vieworg, cl.v_forward, cl.v_right, cl.v_up);

		// Advance local effects for next frame.
		if (cl.frame.valid && !(int)cl_paused->value && !(int)cl_freezeworld->value) // H2
		{
			if (fxe.UpdateEffects != NULL) //TODO: should be called on packetframe to maintain vanilla compatibility?
				fxe.UpdateEffects();

			SK_UpdateSkeletons();
		}

		SCR_RunCinematic();

		cls.framecount++;

		if ((int)log_stats->value && log_stats_file != NULL && cls.state == ca_active)
		{
			if (lasttimecalled == 0)
			{
				lasttimecalled = Sys_Milliseconds();
				fprintf(log_stats_file, "0\n");
			}
			else
			{
				const int now = Sys_Milliseconds();
				fprintf(log_stats_file, "%d\n", now - lasttimecalled);
				lasttimecalled = now;
			}
		}
	}
}

void CL_Init(void)
{
	if ((int)dedicated->value)
		return; // Nothing running on the client.

	ResMngr_Con(&cl_FXBufMngr, sizeof(LERPedReferences_t), LERPEDREF_BLOCK_SIZE); // H2 //mxd. Was a separate function in original version.

	// All archived variables will now be loaded.
	Con_Init();
	VID_Init();
	se.Init(); // Sound must be initialized after window is created. Also initializes music backend --mxd.

	V_Init();

	net_message.data = net_message_buffer;
	net_message.maxsize = sizeof(net_message_buffer);

	M_Init();
	SCR_Init();

	// Missing: cls.disable_screen = true;
	CL_InitLocal();
	IN_Init();

	FS_ExecAutoexec();
	Cbuf_Execute();

	// H2:
	Cbuf_AddText("exec menus.cfg\n");
	Cbuf_AddText("exec user.cfg\n");
	Cbuf_Execute();

	Key_ReadConsoleHistory(); // YQ2
	CL_LoadStrings();
}

// FIXME: this is a callback from Sys_Quit and Com_Error.
// It would be better to run quit through here before the final handoff to the sys code.
void CL_Shutdown(void)
{
	static qboolean isdown = false;

	if (isdown)
	{
		printf("recursive shutdown\n");
		return;
	}

	isdown = true;

	ResMngr_Des(&cl_FXBufMngr); //mxd. Was a separate function in H2.
	CL_WriteConfiguration();
	Key_WriteConsoleHistory(); // YQ2

	if (fxapi_initialized)
		CL_UnloadClientEffects();

	P_Freelib();
	SMK_Shutdown();
	CL_ClearGameMessages(); // H2
	se.Shutdown(); //mxd. Also shuts down music backend.
	IN_Shutdown(); // YQ2
	VID_Shutdown();
	SndDll_FreeLibrary();
	NET_Shutdown();
	Z_FreeTags(0);
}