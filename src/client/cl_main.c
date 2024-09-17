//
// cl_main.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "clfx_dll.h"
#include "cl_skeletons.h"
#include "cl_smk.h"
#include "ResourceManager.h"
#include "sound.h"

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
static cvar_t* cl_minfps;
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
cvar_t* cl_timeout;
cvar_t* cl_paused;
cvar_t* cl_freezeworld;
cvar_t* cl_timedemo;
cvar_t* cl_no_middle_text;
cvar_t* cl_lightlevel;

// New in H2:
cvar_t* shownames;
static cvar_t* r_detail;
static cvar_t* compass;
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
static cvar_t* cl_fx_dll;
cvar_t* cl_cinematicfreeze;
static cvar_t* sc_framerate;

// userinfo
static cvar_t* info_password;
static cvar_t* name;
static cvar_t* skin;
static cvar_t* skindir;
static cvar_t* playerdir;
static cvar_t* rate;
static cvar_t* msg;
static cvar_t* fov;

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

// Game / level messages
typedef struct GameMessage_s
{
	char* message;
	char* sound_name;
} GameMessage_t;

#define MAX_MESSAGES	1000

static char* game_messages_text;
static GameMessage_t game_messages[MAX_MESSAGES];

static char* level_messages_text;
static GameMessage_t level_messages[MAX_MESSAGES];

void CL_WriteDemoMessage(void)
{
	NOT_IMPLEMENTED
}

void CL_ClearState(void)
{
	CL_ClearSkeletalEntities(); // H2
	memset(&cl, 0, sizeof(cl)); // Wipe the entire cl structure.
	SZ_Clear(&cls.netchan.message);
}

void CL_Disconnect(void)
{
	NOT_IMPLEMENTED
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

static void CL_ForwardToServer_f(void)
{
	NOT_IMPLEMENTED
}

static void CL_Pause_f(void)
{
	NOT_IMPLEMENTED
}

static void CL_FreezeWorld_f(void)
{
	NOT_IMPLEMENTED
}

void CL_PingServers_f(void)
{
	NOT_IMPLEMENTED
}

static void CL_Userinfo_f(void)
{
	NOT_IMPLEMENTED
}

// Q2 counterpart
// Restart the sound subsystem so it can pick up new parameters and flush all sounds.
void CL_Snd_Restart_f(void)
{
	SndDll_Init();
	S_Init();
	CL_RegisterSounds();
}

static void CL_Changing_f(void)
{
	NOT_IMPLEMENTED
}

void CL_OnServerDisconnected(void) // H2
{
	NOT_IMPLEMENTED
}

static void SV_Disconnect_f(void)
{
	NOT_IMPLEMENTED
}

void CL_Record_f(void)
{
	NOT_IMPLEMENTED
}

void CL_Stop_f(void)
{
	NOT_IMPLEMENTED
}

static void CL_Ignore_f(void)
{
	NOT_IMPLEMENTED
}

static void CL_Unignore_f(void)
{
	NOT_IMPLEMENTED
}

static void CL_ListIgnore_f(void)
{
	NOT_IMPLEMENTED
}

void CL_Quit_f(void)
{
	NOT_IMPLEMENTED
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

		Netchan_OutOfBandPrint(NS_CLIENT, adr, "connect %i %i %i \"%s\"\n", PROTOCOL_VERSION, port, cls.challenge, Cvar_Userinfo());
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
	Netchan_OutOfBandPrint(NS_CLIENT, adr, "getchallenge\n");
}

static void CL_Connect_f(void)
{
	NOT_IMPLEMENTED
}

static void CL_Reconnect_f(void)
{
	NOT_IMPLEMENTED
}

static void CL_Rcon_f(void)
{
	NOT_IMPLEMENTED
}

static void CL_SaveConfig_f(void)
{
	NOT_IMPLEMENTED
}

static void CL_Setenv_f(void)
{
	NOT_IMPLEMENTED
}

static void CL_Precache_f(void)
{
	NOT_IMPLEMENTED
}

static void CL_Config_f(void)
{
	NOT_IMPLEMENTED
}

static void CL_ParseStatusMessage(void)
{
	NOT_IMPLEMENTED
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

		Sys_AppActivate();
		Cbuf_AddText(MSG_ReadString(&net_message));
		Cbuf_AddText("\n");
	}
	else if (strcmp(c, "print") == 0) // Print command from somewhere.
	{
		Com_Printf("%s", MSG_ReadString(&net_message));
	}
	else if (strcmp(c, "ping") == 0) // Ping from somewhere.
	{
		Netchan_OutOfBandPrint(NS_CLIENT, net_from, "ack");
	}
	else if (strcmp(c, "challenge") == 0) // Challenge from the server we are connecting to.
	{
		cls.challenge = Q_atoi(Cmd_Argv(1));
		CL_SendConnectPacket();
	}
	else if (strcmp(c, "echo") == 0) // Echo request from server.
	{
		Netchan_OutOfBandPrint(NS_CLIENT, net_from, "%s", Cmd_Argv(1));
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

static void ClearIgnoredPlayersList(void)
{
	//mxd. Original code clears only first 8 entries. H2 bug?
	memset(ignored_players, 0, sizeof(ignored_players));
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
	cl_minfps = Cvar_Get("cl_minfps", "5", 0); // Commented out in Q2
	cl_maxfps = Cvar_Get("cl_maxfps", "60", 0);

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

	// New in H2:
	shownames = Cvar_Get("shownames", "0", CVAR_ARCHIVE);
	r_detail = Cvar_Get("r_detail", "3", CVAR_ARCHIVE);
	compass = Cvar_Get("compass", "0", CVAR_ARCHIVE);
	game_downloadable_type = Cvar_Get("downloadable_game", "0", 0); //TODO: eh? Set again below
	cl_showcaptions = Cvar_Get("cl_showcaptions", "1", 0);
	cl_doautoaim = Cvar_Get("cl_doautoaim", "0.0", CVAR_ARCHIVE);
	cl_camera_combat = Cvar_Get("cl_camera_combat", "0.0", 0);
	cl_camera_clipdamp = Cvar_Get("cl_camera_clipdamp", "1.0", 0);
	cl_camera_dampfactor = Cvar_Get("cl_camera_dampfactor", "1", CVAR_ARCHIVE);
	cl_camera_fpoffs = Cvar_Get("cl_camera_fpoffs", "0.0", 0);
	cl_camera_freeze = Cvar_Get("cl_camera_freeze", "0.0", 0);
	cl_camera_under_surface = Cvar_Get("cl_camera_under_surface", "0.0", 0);
	cl_camera_viewdist = Cvar_Get("cl_camera_viewdist", "126.0", 0);
	cl_camera_viewmin = Cvar_Get("cl_camera_viewmin", "70.0", 0);
	cl_camera_viewmax = Cvar_Get("cl_camera_viewmax", "200.0", 0);
	cl_camera_fpmode = Cvar_Get("cl_camera_fpmode", "0", CVAR_ARCHIVE);
	cl_camera_fptrans = Cvar_Get("cl_camera_fptrans", "0", CVAR_ARCHIVE);
	cl_camera_fpdist = Cvar_Get("cl_camera_fpdist", "16.0", CVAR_ARCHIVE);
	cl_camera_fpheight = Cvar_Get("cl_camera_fpheight", "0.0", CVAR_ARCHIVE);
	cl_playertrans = Cvar_Get("cl_playertrans", "1.0", CVAR_ARCHIVE);
	EAX_preset = Cvar_Get("EAX_preset", "0", 0);
	EAX_default = Cvar_Get("EAX_default", "0", 0);
	quake_amount = Cvar_Get("quake_amount", "0.0", 0);
	cl_fx_dll = Cvar_Get("cl_fx_dll", "Client Effects", 0);
	cl_cinematicfreeze = Cvar_Get("cl_cinematicfreeze", "0", 0);
	sc_framerate = Cvar_Get("sc_framerate", "60", CVAR_ARCHIVE);

	// userinfo
	info_password = Cvar_Get("password", "", CVAR_USERINFO);
	name = Cvar_Get("name", "Corvus", CVAR_ARCHIVE | CVAR_USERINFO);
	skin = Cvar_Get("skin", "Corvus", CVAR_ARCHIVE | CVAR_USERINFO);
	skindir = Cvar_Get("skindir", "players", CVAR_ARCHIVE);
	playerdir = Cvar_Get("playerdir", "players", CVAR_ARCHIVE);
	rate = Cvar_Get("rate", "25000", CVAR_ARCHIVE | CVAR_USERINFO);
	msg = Cvar_Get("msg", "1", CVAR_ARCHIVE | CVAR_USERINFO);
	fov = Cvar_Get("fov", "90", CVAR_ARCHIVE | CVAR_USERINFO);
	autoweapon = Cvar_Get("autoweapon", "1", CVAR_ARCHIVE | CVAR_USERINFO);
	colour_obituary = Cvar_Get("colour_obituary", "20", CVAR_ARCHIVE);
	colour_chat = Cvar_Get("colour_chat", "20", CVAR_ARCHIVE);
	colour_names = Cvar_Get("colour_names", "20", CVAR_ARCHIVE);
	colour_teamchat = Cvar_Get("colour_teamchat", "20", CVAR_ARCHIVE);
	colour_level = Cvar_Get("colour_level", "20", CVAR_ARCHIVE);
	colour_game = Cvar_Get("colour_game", "20", CVAR_ARCHIVE);
	game_downloadable_type = Cvar_Get("game_downloadable_type", "0", 0);

	// Register our commands
	Cmd_AddCommand("cmd", CL_ForwardToServer_f);
	Cmd_AddCommand("pause", CL_Pause_f);

	// New in H2:
	Cmd_AddCommand("freezeworld", CL_FreezeWorld_f);
	Cmd_AddCommand("pingservers", CL_PingServers_f);
	Cmd_AddCommand("userinfo", CL_Userinfo_f);
	Cmd_AddCommand("snd_restart", CL_Snd_Restart_f);
	Cmd_AddCommand("changing", CL_Changing_f);
	Cmd_AddCommand("disconnect", SV_Disconnect_f);
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
	Cmd_AddCommand("config", CL_Config_f);

	// Forward to server commands
	Cmd_AddCommand("kill", NULL);
	Cmd_AddCommand("use", NULL);
	Cmd_AddCommand("say", NULL);
	Cmd_AddCommand("say_team", NULL);
	Cmd_AddCommand("info", NULL);
	Cmd_AddCommand("weapnext", NULL);
	Cmd_AddCommand("weapprev", NULL);

	// New in H2:
	Cmd_AddCommand("spawn", NULL);
	Cmd_AddCommand("nextmonsterframe", NULL);
	Cmd_AddCommand("crazymonsters", NULL);
	Cmd_AddCommand("angermonsters", NULL);
	Cmd_AddCommand("showcoords", NULL);
	Cmd_AddCommand("playbetter", NULL);
	Cmd_AddCommand("kiwi", NULL);
	Cmd_AddCommand("victor", NULL);
	Cmd_AddCommand("suckitdown", NULL);
	Cmd_AddCommand("twoweeks", NULL);
	Cmd_AddCommand("meatwagon", NULL);
}

// Writes key bindings and archived cvars to config.cfg
static void CL_WriteConfiguration(void)
{
	FILE* f;
	char path[MAX_QPATH];

	if (cls.state == ca_uninitialized)
		return;

	Com_sprintf(path, sizeof(path), "%s/config.cfg", FS_Userdir()); // FS_Gamedir in Q2
	FS_CreatePath(path); // H2

	if (fopen_s(&f, path, "w") != 0) //mxd. fopen -> fopen_s
	{
		Com_Printf("Couldn\'t write config.cfg.\n");
		return;
	}

	fprintf(f, "// Generated by Heretic 2, do not modify\n"); // H2: different message
	Key_WriteBindings(f);
	Key_WriteBindings_Double(f); // H2
	fclose(f);

	Cvar_WriteVariables(path);
}

static void CL_StorePredictInfo(void) // H2
{
	//mxd. Skip when unable to render //TODO: more reliable way to check we are ingame (sv.state does this)?
	if (!cl.refresh_prepped)
		return;

	NOT_IMPLEMENTED
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

// Q2 counterpart
static void CL_SendCommand(void)
{
	Sys_SendKeyEvents();	// Get new key events.
	IN_Commands();			// Allow gamepads to add commands.
	Cbuf_Execute();			// Process console commands.
	CL_FixCvarCheats();		// Fix any cheat cvars.
	CL_SendCmd();			// Send intentions now.
	CL_CheckForResend();	// Resend a connection request if necessary.
}

void CL_Frame(const int msec)
{
	static int extratime;
	static int lasttimecalled;

	if ((int)dedicated->value)
		return;

	extratime += msec;

	if (!(int)cl_timedemo->value && cl.cinematictime == 0) // H2: extra cl.cinematictime check
	{
		// Don't flood packets out while connecting.
		if (cls.state == ca_connected && extratime < 100)
			return;

		// Framerate is too high.
		if (extratime < (int)(1000.0f / cl_maxfps->value))
			return;
	}

	// let the mouse activate or deactivate.
	IN_Frame();

	// Decide the simulation time.
	cls.frametime = (float)extratime / 1000.0f;
	cls.framemodifier = GetFrameModifier(cls.frametime); // H2

	if ((int)cl_frametime->value && !(int)cl_paused->value) // H2
		Com_Printf("Framerate = %f.\n", (double)(1.0f / cls.frametime));

	cl.time += extratime;
	cls.realtime = curtime;
	camera_timer += extratime; // H2

	extratime = 0;

	// #if 0-ed in Q2
	cls.frametime = min(1.0f / cl_minfps->value, cls.frametime);

	// If in the debugger last frame, don't timeout.
	if (msec > 5000)
		cls.netchan.last_received = Sys_Milliseconds();

	// Fetch results from server.
	CL_ReadPackets();

	if ((int)cl_predict->value) // H2
		CL_StorePlayerInventoryInfo();

	// Send a new command message to the server.
	CL_SendCommand();

	// Predict all unacknowledged movements.
	CL_PredictMovement();

	if ((int)cl_predict->value) // H2
		CL_StorePredictInfo();

	// Allow rendering DLL change.
	VID_CheckChanges();
	if (!cl.refresh_prepped && cls.state == ca_active)
		CL_PrepRefresh();

	// Update the screen.
	Con_UpdateConsoleHeight(); // H2
	SCR_UpdateScreen();

	// Update audio.
	S_Update(cl.refdef.vieworg, cl.v_forward, cl.v_right, cl.v_up);
	// CDAudio_Update(); //mxd. Skip CDAudio logic.

	// Advance local effects for next frame.
	if (cl.frame.valid && !(int)cl_paused->value && !(int)cl_freezeworld->value) // H2
	{
		if (fxe.UpdateEffects != NULL)
			fxe.UpdateEffects();

		SK_UpdateSkeletons();
	}

	SCR_RunCinematic();
	//CDAudio_RestartTrackIfNecessary(); //mxd. Skip CDAudio logic.

	cls.framecount++;

	if ((int)log_stats->value && cls.state == ca_active)
	{
		if (lasttimecalled == 0)
		{
			lasttimecalled = Sys_Milliseconds();
			if (log_stats_file != NULL)
				fprintf(log_stats_file, "0\n");
		}
		else
		{
			const int now = Sys_Milliseconds();
			if (log_stats_file != NULL)
				fprintf(log_stats_file, "%d\n", now - lasttimecalled);

			lasttimecalled = now;
		}
	}
}

static int LoadMessages(char* filename, char** message_out)
{
	char* buf;

	const int size = FS_LoadFile(filename, (void**)&buf);
	if (size < 1)
		Sys_Error("Unable to load %s", filename);

	*message_out = Z_Malloc(size + 1);
	memcpy(*message_out, buf, size);
	(*message_out)[size] = 0;

	FS_FreeFile(buf);

	return size + 1;
}

//TODO: lines starting with ';' char are supposed to be comments?
static void SplitMessages(char* src, GameMessage_t* dst, const int src_len)
{
	const char* end = src + src_len;

	for (int i = 1; src < end; i++)
	{
		if (i >= MAX_MESSAGES)
		{
			Com_Printf("Too many strings\n");
			return;
		}

		dst[i].message = NULL;
		dst[i].sound_name = NULL;

		char* newline = strchr(src, '\r');
		if (newline == NULL)
			return;

		*newline = '\0';

		// # signifies a .wav file
		char* snd_id = strchr(src, '#');
		if (snd_id != NULL && snd_id < newline)
		{
			dst[i].sound_name = snd_id + 1;
			*snd_id = '\0';
		}

		dst[i].message = src;

		// @ signifies a newline
		while (src != NULL)
		{
			src = strchr(src, '@');
			if (src == NULL)
				break;

			*src = '\n';
			src++;
		}

		src = newline + 2;
	}
}

static void InitMessages(void)
{
	const cvar_t* cv = Cvar_Get("file_gamemsg", "gamemsg.txt", CVAR_ARCHIVE);
	int len = LoadMessages(cv->string, &game_messages_text);
	SplitMessages(game_messages_text, game_messages, len);

	cv = Cvar_Get("file_levelmsg", "levelmsg.txt", CVAR_ARCHIVE);
	len = LoadMessages(cv->string, &level_messages_text);
	SplitMessages(level_messages_text, level_messages, len);
}

static void ClearGameMessages(void)
{
	NOT_IMPLEMENTED
}

void CL_Init(void)
{
	if ((int)dedicated->value)
		return; // Nothing running on the client

	ResMngr_Con(&cl_FXBufMngr, 1292, 8); // New in H2 (was a separate function there)

	// All archived variables will now be loaded
	Con_Init();
	VID_Init();
	S_Init(); // Sound must be initialized after window is created

	V_Init();

	net_message.data = net_message_buffer;
	net_message.maxsize = sizeof(net_message_buffer);

	M_Init();
	SCR_Init();

	// Missing: cls.disable_screen = true;
	//CDAudio_Init(); //mxd. Skip CDAudio logic.
	CL_InitLocal();
	IN_Init();

	FS_ExecAutoexec();
	Cbuf_Execute();

	// New in H2:
	Cbuf_AddText("exec menus.cfg\n");
	Cbuf_AddText("exec user.cfg\n");
	Cbuf_Execute();

	InitMessages();
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

	Cvar_SetValue("win_ignore_destroy", 1.0f);

	if (SNDEAX_SetEnvironment != NULL)
	{
		SNDEAX_SetEnvironment(0);
		SNDEAX_SetEnvironment = NULL;
	}

	ResMngr_Des(&cl_FXBufMngr); //mxd. Was a separate function in H2

	CL_WriteConfiguration();

	if (fxapi_initialized)
		SV_UnloadClientEffects();

	P_Freelib();
	SMK_Stop();
	ClearGameMessages();
	//CDAudio_Shutdown(); //mxd. Skip CDAudio logic
	S_Shutdown();
	IN_DeactivateMouse();
	VID_Shutdown();
	SndDll_FreeLibrary();
	NET_Shutdown();
	Z_FreeTags(0);
}