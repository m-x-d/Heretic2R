//
// cl_screen.c -- master for refresh, status bar, console, chat, notify, etc
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "sound.h"

qboolean scr_initialized; // Ready to draw

qboolean scr_draw_loading_plaque; // H2
static qboolean scr_draw_loading; // int in Q2
static int scr_progressbar_offset_x; // H2

cvar_t* scr_viewsize;
cvar_t* scr_centertime;
cvar_t* scr_showturtle;
cvar_t* scr_showpause;
cvar_t* scr_printspeed;

cvar_t* scr_netgraph;
cvar_t* scr_timegraph;
cvar_t* scr_debuggraph;
cvar_t* scr_graphheight;
cvar_t* scr_graphscale;
cvar_t* scr_graphshift;
cvar_t* scr_drawall;

// New in H2:
cvar_t* scr_statbar;
cvar_t* scr_item_paused;
cvar_t* scr_item_loading;
cvar_t* r_fog;
cvar_t* r_fog_density;

typedef struct
{
	int x1;
	int y1;
	int x2;
	int y2;
} dirty_t;

static dirty_t scr_dirty;

#pragma region ========================== BAR GRAPHS ==========================

void SCR_DebugGraph(float value, int color)
{
	NOT_IMPLEMENTED
}

#pragma endregion

static void SCR_TimeRefresh_f(void)
{
	NOT_IMPLEMENTED
}

static void SCR_Loading_f(void)
{
	SCR_BeginLoadingPlaque();
}

static void SCR_SizeUp_f(void)
{
	NOT_IMPLEMENTED
}

static void SCR_SizeDown_f(void)
{
	NOT_IMPLEMENTED
}

static void SCR_Sky_f(void)
{
	NOT_IMPLEMENTED
}

static void SCR_GammaUp_f(void)
{
	NOT_IMPLEMENTED
}

static void SCR_GammaDown_f(void)
{
	NOT_IMPLEMENTED
}

void SCR_BeginLoadingPlaque(void)
{
	S_StopAllSounds_Sounding();
	cl.sound_prepped = false; // Don't play ambients
	//CDAudio_Stop(); //mxd. Skip CDAudio logic.

	scr_draw_loading_plaque = true; // H2

	if (!(int)cls.disable_screen && !(int)developer->value && cls.key_dest != key_console)
	{
		if (cl.cinematictime == 0)
			scr_draw_loading = true;

		scr_progressbar_offset_x = 0; // H2

		SCR_UpdateScreen();
		cls.disable_screen = 1; // Q2: Sys_Milliseconds()
		cls.disable_servercount = cl.servercount;
	}
}

void SCR_EndLoadingPlaque(void)
{
	NOT_IMPLEMENTED
}

void SCR_Init(void)
{
	scr_viewsize = Cvar_Get("viewsize", "100", CVAR_ARCHIVE);
	scr_showturtle = Cvar_Get("scr_showturtle", "0", 0);
	scr_showpause = Cvar_Get("scr_showpause", "1", 0);
	scr_centertime = Cvar_Get("scr_centertime", "2.5", 0);
	scr_printspeed = Cvar_Get("scr_printspeed", "8", 0);
	scr_netgraph = Cvar_Get("scr_netgraph", "0", 0);
	scr_timegraph = Cvar_Get("scr_timegraph", "0", 0);
	scr_debuggraph = Cvar_Get("scr_debuggraph", "0", 0);
	scr_graphheight = Cvar_Get("scr_graphheight", "112", 0);
	scr_graphscale = Cvar_Get("scr_graphscale", "1", 0);
	scr_drawall = Cvar_Get("scr_drawall", "0", 0);

	// New in H2:
	scr_statbar = Cvar_Get("scr_statbar", "1", 0);
	scr_item_paused = Cvar_Get("scr_item_paused", "Paused", 0);
	scr_item_loading = Cvar_Get("scr_item_loading", "Loading", 0);
	r_fog = Cvar_Get("r_fog", "0", 0);
	r_fog_density = Cvar_Get("r_fog_density", "0", 0);
	//gl_lostfocus_broken = Cvar_Get("gl_lostfocus_broken", "0", 0); //mxd. Ignored

	// Register our commands
	Cmd_AddCommand("timerefresh", SCR_TimeRefresh_f);
	Cmd_AddCommand("loading", SCR_Loading_f);
	Cmd_AddCommand("sizeup", SCR_SizeUp_f);
	Cmd_AddCommand("sizedown", SCR_SizeDown_f);
	Cmd_AddCommand("sky", SCR_Sky_f);
	Cmd_AddCommand("vid_gamma_up", SCR_GammaUp_f);
	Cmd_AddCommand("vid_gamma_down", SCR_GammaDown_f);

	scr_initialized = true;
}

// Q2 counterpart
void SCR_AddDirtyPoint(const int x, const int y)
{
	scr_dirty.x1 = min(x, scr_dirty.x1);
	scr_dirty.x2 = max(x, scr_dirty.x2);
	scr_dirty.y1 = min(y, scr_dirty.y1);
	scr_dirty.y2 = max(y, scr_dirty.y2);
}

// Q2 counterpart
void SCR_DirtyScreen(void)
{
	SCR_AddDirtyPoint(0, 0);
	SCR_AddDirtyPoint(viddef.width - 1, viddef.height - 1);
}

void SCR_UpdateScreen(void)
{
	NOT_IMPLEMENTED
}
