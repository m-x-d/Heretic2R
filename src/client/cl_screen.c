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

static void SCR_ShowDebugGraph(void)
{
	NOT_IMPLEMENTED
}

static void SCR_DrawDebugGraph(void)
{
	NOT_IMPLEMENTED
}

#pragma endregion

static void SCR_CalcVrect(void)
{
	NOT_IMPLEMENTED
}

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

static void SCR_DrawConsole(void)
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

static void SCR_DrawNet(void)
{
	NOT_IMPLEMENTED
}

static void SCR_DrawPause(void)
{
	NOT_IMPLEMENTED
}

static void SCR_DrawLoading(void)
{
	NOT_IMPLEMENTED
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

static void SCR_TileClear(void)
{
	NOT_IMPLEMENTED
}

static void SCR_DrawStats(void)
{
	NOT_IMPLEMENTED
}

static void SCR_DrawLayout(void)
{
	NOT_IMPLEMENTED
}

static void SCR_DrawNames(void)
{
	NOT_IMPLEMENTED
}

static void SCR_DrawFill(void)
{
	NOT_IMPLEMENTED
}

static void SCR_DrawGameMessageIfNecessary(void)
{
	NOT_IMPLEMENTED
}

static void SCR_UpdateFogDensity(void)
{
	static int old_msec;
	static float old_density;

	const int msec = Sys_Milliseconds();

	if (cl.frame.playerstate.fog_density > 0.0f) //TODO: != 0.0f in original logic. Can playerstate.fog_density be negative?
	{
		r_fog_density->value += (cl.frame.playerstate.fog_density - old_density) * (float)(msec - old_msec) * 0.0008f;
		old_density = r_fog_density->value;
		r_fog->value = 1.0f;
	}
	else
	{
		r_fog_density->value = 0.0f;
		old_density = 0.0f;
		r_fog->value = 0.0f;
	}

	old_msec = msec;
}

// This is called every frame, and can also be called explicitly to flush text to the screen.
void SCR_UpdateScreen(void)
{
	int numframes;
	float separation[2] = { 0.0f, 0.0f };

	// If the screen is disabled (loading plaque is up, or vid mode changing) or screen/console aren't initialized, do nothing at all.
	if ((int)cls.disable_screen || !scr_initialized || !con.initialized)
		return;

	// Range check cl_camera_separation so we don't inadvertently fry someone's brain.
	if (cl_stereo_separation->value > 1.0f)
		Cvar_SetValue("cl_stereo_separation", 1.0f);
	else if (cl_stereo_separation->value < 0.0f)
		Cvar_SetValue("cl_stereo_separation", 0.0f);

	if ((int)cl_stereo->value)
	{
		separation[0] = -cl_stereo_separation->value * 0.5f;
		separation[1] = cl_stereo_separation->value * 0.5f;
		numframes = 2;
	}
	else
	{
		separation[0] = 0.0f;
		separation[1] = 0.0f;
		numframes = 1;
	}

	SCR_UpdateFogDensity(); // H2

	for (int i = 0; i < numframes; i++)
	{
		re.BeginFrame(separation[i]);

		// If a cinematic is supposed to be running, handle menus and console specially.
		if (cl.cinematictime > 0)
		{
			SCR_DrawCinematic();

			if (cls.key_dest == key_menu)
				M_Draw();
			else if (cls.key_dest == key_console)
				SCR_DrawConsole();
		}
		else
		{
			// Do 3D refresh drawing, and then update the screen.
			SCR_CalcVrect();

			// Clear any dirty part of the background.
			SCR_TileClear();

			V_RenderView(separation[i]);

			SCR_DrawStats();
			SCR_DrawLayout();
			SCR_DrawNet();
			SCR_DrawNames(); // H2
			SCR_DrawFill(); // H2
			SCR_DrawGameMessageIfNecessary(); // H2

			SCR_ShowDebugGraph(); // H2
			SCR_DrawDebugGraph();
			SCR_DrawConsole();
			SCR_DrawPause();
			SCR_DrawLoading();

			M_Draw();
		}
	}

	re.EndFrame();
}
