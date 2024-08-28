//
// cl_main.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "clfx_dll.h"
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
static cvar_t* cl_doautoaim;

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

static qboolean ignored_players[MAX_CLIENTS];

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

void CL_Snd_Restart_f(void)
{
	NOT_IMPLEMENTED
}

static void CL_Changing_f(void)
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

static void InitMessages(void)
{
	NOT_IMPLEMENTED
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