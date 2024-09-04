//
// cl_screen.c -- master for refresh, status bar, console, chat, notify, etc
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "sound.h"
#include "Vector.h"

float scr_con_current; // Aproaches scr_conlines at scr_conspeed.

qboolean scr_initialized; // Ready to draw

qboolean scr_draw_loading_plaque; // H2
static qboolean scr_draw_loading; // int in Q2
static int scr_progressbar_offset_x; // H2

vrect_t scr_vrect; // Position of render window on screen

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
static cvar_t* scr_drawall;

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

static void SCR_AddTimeGraph(void) // H2
{
	if ((int)scr_timegraph->value)
		SCR_DebugGraph((float)frame_index, 0xffffff);
}

static void SCR_DrawDebugGraph(void)
{
	NOT_IMPLEMENTED
}

#pragma endregion

// Sets scr_vrect, the coordinates of the rendered window.
static void SCR_CalcVrect(void)
{
	// Bound viewsize
	if (scr_viewsize->value < 30.0f) // 40 in Q2
		Cvar_Set("viewsize", "30");

	if (scr_viewsize->value > 100.0f)
		Cvar_Set("viewsize", "100");

	const int size = (int)scr_viewsize->value;

	scr_vrect.width = viddef.width * size / 100;
	scr_vrect.width &= ~7;

	scr_vrect.height = viddef.height * size / 100;
	scr_vrect.height &= ~1;

	scr_vrect.x = (viddef.width - scr_vrect.width) / 2;
	scr_vrect.y = (viddef.height - scr_vrect.height) / 2;
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
	if (cls.netchan.outgoing_sequence - cls.netchan.incoming_acknowledged >= CMD_BACKUP - 1)
		re.DrawPic(scr_vrect.x + 16, scr_vrect.y + 16, "misc/net.m8", 1.0f); // Q2: re.DrawPic(scr_vrect.x + 64, scr_vrect.y, "net");
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

//TODO: remove - irrelevant in OpenGL
// Clear any parts of the tiled background that were drawn on last frame
static void SCR_TileClear(void)
{
	static dirty_t scr_old_dirty[2]; //mxd. Made static

	// For power vr or broken page flippers...
	if ((int)scr_drawall->value)
		SCR_DirtyScreen();

	// Skip when fullscreen console, rendering or cinematic.
	if (scr_con_current == 1.0f || scr_viewsize->value == 100.0f || cl.cinematictime > 0)
		return;

	// Erase rect will be the union of the past three frames so triple buffering works properly
	dirty_t clear = scr_dirty;
	for (int i = 0; i < 2; i++)
	{
		if (scr_old_dirty[i].x1 < clear.x1)
			clear.x1 = scr_old_dirty[i].x1;
		if (scr_old_dirty[i].x2 > clear.x2)
			clear.x2 = scr_old_dirty[i].x2;
		if (scr_old_dirty[i].y1 < clear.y1)
			clear.y1 = scr_old_dirty[i].y1;
		if (scr_old_dirty[i].y2 > clear.y2)
			clear.y2 = scr_old_dirty[i].y2;
	}

	scr_old_dirty[1] = scr_old_dirty[0];
	scr_old_dirty[0] = scr_dirty;

	scr_dirty.x1 = 9999;
	scr_dirty.x2 = -9999;
	scr_dirty.y1 = 9999;
	scr_dirty.y2 = -9999;

	// Don't bother with anything covered by the console
	const int console_top = (int)(scr_con_current * (float)viddef.height);
	if (console_top >= clear.y1)
		clear.y1 = console_top;

	if (clear.y2 <= clear.y1)
		return; // Nothing disturbed

	const int top = scr_vrect.y;
	const int bottom = top + scr_vrect.height - 1;
	const int left = scr_vrect.x;
	const int right = left + scr_vrect.width - 1;

	// Clear above view screen?
	if (clear.y1 < top)
	{
		const int i = (clear.y2 < top - 1 ? clear.y2 : top - 1);
		re.DrawTileClear(clear.x1, clear.y1, clear.x2 - clear.x1 + 1, i - clear.y1 + 1, "misc/backtile.m8"); // H2: backtile -> misc/backtile.m8
		clear.y1 = top;
	}

	// Clear below view screen?
	if (clear.y2 > bottom)
	{
		const int i = (clear.y1 > bottom + 1 ? clear.y1 : bottom + 1);
		re.DrawTileClear(clear.x1, i, clear.x2 - clear.x1 + 1, clear.y2 - i + 1, "misc/backtile.m8"); // H2: backtile -> misc/backtile.m8
		clear.y2 = bottom;
	}

	// Clear left of view screen?
	if (clear.x1 < left)
	{
		const int i = (clear.x2 < left - 1 ? clear.x2 : left - 1);
		re.DrawTileClear(clear.x1, clear.y1, i - clear.x1 + 1, clear.y2 - clear.y1 + 1, "misc/backtile.m8"); // H2: backtile -> misc/backtile.m8
		clear.x1 = left;
	}

	// Clear left of view screen?
	if (clear.x2 > right)
	{
		const int i = (clear.x1 > right + 1 ? clear.x1 : right + 1);
		re.DrawTileClear(i, clear.y1, clear.x2 - i + 1, clear.y2 - clear.y1 + 1, "misc/backtile.m8"); // H2: backtile -> misc/backtile.m8
		//clear.x2 = right;
	}
}

static void DrawPic(int x, int y, char* str, qboolean use_alpha)
{
	NOT_IMPLEMENTED
}

static void DrawTeamBlock(int x, int y, char* str)
{
	NOT_IMPLEMENTED
}

static void DrawClientBlock(int x, int y, char* str)
{
	NOT_IMPLEMENTED
}

static void DrawAClientBlock(int x, int y, char* str)
{
	NOT_IMPLEMENTED
}

static void DrawHudNum(int x, int y, int width, int value, qboolean is_red)
{
	NOT_IMPLEMENTED
}

static void DrawBar(int x, int y, int width, int height, int stat_index)
{
	NOT_IMPLEMENTED
}

static void SCR_ExecuteLayoutString(char* s)
{
	if (cls.state != ca_active || !cl.refresh_prepped || s[0] == 0)
		return;

	//TODO: add SCR_GetHUDScale logic from YQ2

	int x = 0;
	int y = 0;
	int width = 3;

	while (s != NULL)
	{
		const char* token = COM_Parse(&s);

		if (strcmp(token, "xl") == 0)
		{
			x = Q_atoi(COM_Parse(&s));
		}
		else if (strcmp(token, "xr") == 0)
		{
			x = viddef.width + Q_atoi(COM_Parse(&s));
		}
		else if (strcmp(token, "xv") == 0)
		{
			x = viddef.width / 2 - 160 + Q_atoi(COM_Parse(&s));
		}
		else if (strcmp(token, "xc") == 0) // H2
		{
			const int offset = cl.frame.playerstate.stats[STAT_PUZZLE_COUNT] * 40;
			x = (viddef.width - offset) / 2 + Q_atoi(COM_Parse(&s));
		}
		else if (strcmp(token, "yt") == 0)
		{
			y = Q_atoi(COM_Parse(&s));
		}
		else if (strcmp(token, "yb") == 0)
		{
			y = viddef.height + Q_atoi(COM_Parse(&s));
		}
		else if (strcmp(token, "yv") == 0)
		{
			y = viddef.height / 2 - 120 + Q_atoi(COM_Parse(&s));
		}
		else if (strcmp(token, "yp") == 0) // H2
		{
			y += Q_atoi(COM_Parse(&s));
		}
		else if (strcmp(token, "pic") == 0)
		{
			// Draw a pic from a stat number.
			DrawPic(x, y, s, false);
		}
		else if (strcmp(token, "pici") == 0) // H2
		{
			// When 'Show puzzle inventory' flag is set.
			if ((cl.frame.playerstate.stats[STAT_LAYOUTS] & 4) != 0)
				DrawPic(x, y, s, true);
		}
		else if (strcmp(token, "tm") == 0) // H2
		{
			DrawTeamBlock(x, y, s);
		}
		else if (strcmp(token, "client") == 0)
		{
			// Draw a deathmatch client block.
			DrawClientBlock(x, y, s);
		}
		else if (strcmp(token, "aclient") == 0) // H2
		{
			// Draw a ??? client block.
			DrawAClientBlock(x, y, s);
		}
		else if (strcmp(token, "picn") == 0)
		{
			// Draw a pic from a name.
			SCR_AddDirtyPoint(x, y);
			SCR_AddDirtyPoint(x + 32, y + 32);

			re.DrawPic(x, y, COM_Parse(&s), 1.0f);
		}
		else if (strcmp(token, "num") == 0)
		{
			// Draw a number.
			width = Q_atoi(COM_Parse(&s));
			const int value = cl.frame.playerstate.stats[Q_atoi(COM_Parse(&s))];

			DrawHudNum(x, y, width, value, value <= 0);
		}
		else if (strcmp(token, "hnum") == 0) // H2
		{
			// Draw health number.
			const int amount = max(-99, cl.frame.playerstate.stats[STAT_HEALTH]);
			DrawHudNum(x, y, width, amount, amount <= 25);
		}
		else if (strcmp(token, "arm") == 0) // H2
		{
			// Draw armor number.
			if (cl.frame.playerstate.stats[STAT_ARMOUR] != 0)
			{
				const int amount = max(-99, cl.frame.playerstate.stats[STAT_ARMOUR]);
				DrawHudNum(x, y, width, amount, amount <= 25);
			}
		}
		else if (strcmp(token, "am") == 0) // H2
		{
			// Draw ammo number.
			if (cl.frame.playerstate.stats[STAT_AMMO_ICON] != 0)
			{
				const int amount = max(0, cl.frame.playerstate.stats[STAT_AMMO]);
				DrawHudNum(x, y, width, amount, amount <= 10);
			}
		}
		else if (strcmp(token, "bar") == 0) // H2
		{
			const int stat_index = Q_atoi(COM_Parse(&s));
			width = Q_atoi(COM_Parse(&s));
			const int height = Q_atoi(COM_Parse(&s));

			DrawBar(x, y, width, height, stat_index);
		}
		else if (strcmp(token, "gbar") == 0) // H2
		{
			const int index = Q_atoi(COM_Parse(&s));
			width = cl.frame.playerstate.stats[index];
			const int height = cl.frame.playerstate.stats[index + 1];

			DrawBar(x, y, width, height, index + 2);
		}
		else if (strcmp(token, "stat_string") == 0)
		{
			int index = Q_atoi(COM_Parse(&s));
			if (index < 0 || index >= MAX_CONFIGSTRINGS)
				Com_Error(ERR_DROP, "Bad stat_string index");

			index = cl.frame.playerstate.stats[index];
			if (index < 0 || index >= MAX_CONFIGSTRINGS)
				Com_Error(ERR_DROP, "Bad stat_string index");

			DrawString(x, y, cl.configstrings[index], TextPalette[P_WHITE].c, -1);
		}
		else if (strcmp(token, "hstring") == 0) // H2
		{
			x = viddef.width / 2 - 160 + Q_atoi(COM_Parse(&s));
			y = viddef.height / 2 - 120 + Q_atoi(COM_Parse(&s));
			const int pal_index = Q_atoi(COM_Parse(&s));
			const char* str = COM_Parse(&s);

			DrawString(x, y, str, TextPalette[pal_index].c, -1);
		}
		else if (strcmp(token, "string") == 0)
		{
			x = Q_atoi(COM_Parse(&s));
			y = Q_atoi(COM_Parse(&s));
			const int pal_index = Q_atoi(COM_Parse(&s));
			const char* str = COM_Parse(&s);

			DrawString(x, y, str, TextPalette[pal_index].c, -1);
		}
		else if (strcmp(token, "if") == 0)
		{
			// Draw a number
			token = COM_Parse(&s);
			if (cl.frame.playerstate.stats[Q_atoi(token)] == 0)
				while (s != NULL && strcmp(token, "endif") != 0) // Skip to endif
					token = COM_Parse(&s);
		}
	}
}

// The status bar is a small layout program that is based on the stats array.
static void SCR_DrawStats(void)
{
	if ((int)scr_statbar->value && !cl.frame.playerstate.cinematicfreeze) // H2: extra checks
		SCR_ExecuteLayoutString(cl.configstrings[CS_STATUSBAR]);
}

static void SCR_DrawLayout(void)
{
	if ((cl.frame.playerstate.stats[STAT_LAYOUTS] & 1) != 0) // H2: proceed when 'Show scores' flag is set.
		SCR_ExecuteLayoutString(cl.layout);
}

static void SCR_DrawNames(void) // H2
{
	vec3_t origin;
	paletteRGBA_t color;

	if (!(int)shownames->value)
		return;

	for (int i = 0; i < Q_atoi(cl.configstrings[CS_MAXCLIENTS]); i++)
	{
		// Undefined skin (???)
		if (cl.configstrings[CS_PLAYERSKINS + i] == 0)
			continue;

		// Not in player's view
		if ((cl.PIV & cl.frame.playerstate.PIV & (1 << i)) == 0)
			continue;

		if (cl.frame.playerstate.dmflags & DF_SKINTEAMS)
		{
			if (Q_stricmp(cl.clientinfo[i].skin_name, cl.clientinfo[cl.playernum].skin_name) == 0)
				color = TextPalette[P_GREEN];
			else
				color = TextPalette[P_RED];
		}
		else if (cl.frame.playerstate.dmflags & DF_MODELTEAMS)
		{
			if (Q_stricmp(cl.clientinfo[i].model_name, cl.clientinfo[cl.playernum].model_name) == 0)
				color = TextPalette[P_GREEN];
			else
				color = TextPalette[P_RED];
		}
		else
		{
			color = TextPalette[COLOUR(colour_names)];
		}

		VectorCopy(cl.clientinfo[i].origin, origin);
		origin[2] += 64.0f - shownames->value * 32.0f;

		re.Draw_Name(origin, cl.clientinfo[i].name, color);
	}
}

static void SCR_DrawCinematicBorders(void) // H2
{
	if (cls.key_dest != key_console && cl.frame.playerstate.cinematicfreeze)
	{
		const int top_height = (viddef.height * 48) / 480;
		const int bottom_height = (viddef.height * 64) / 480;

		re.DrawFill(0, 0, viddef.width, top_height, 0, 0, 0);
		re.DrawFill(0, viddef.height - bottom_height, viddef.width, bottom_height, 0, 0, 0);
	}
}

static void SCR_DrawGameMessage(void) // H2
{
	int line_len;

	game_message_dispay_time -= cls.frametime;

	if (game_message_dispay_time <= 0.0f)
		return;
		
	//mxd. The above code was in a separate function in original version.
	if (strlen(game_message) > 9999)
		return;

	const float scaler = (game_message_show_at_top ? 0.9f : 0.4f);
	int y = (int)((float)viddef.height * scaler) - (game_message_num_lines / 2) * 8;

	for (char* s = game_message; *s != 0; s += line_len, y += 8)
	{
		for (line_len = 0; line_len < 60; line_len++)
		{
			const char c = s[line_len];
			if (c == 0 || c == '\n')
				break;
		}

		const int x = (viddef.width - line_len * 8) / 2;
		SCR_AddDirtyPoint(x, y);
		DrawString(x, y, s, game_message_color.c, line_len);
		SCR_AddDirtyPoint(x + line_len * 8, y + 8);
	}
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
			SCR_DrawCinematicBorders(); // H2
			SCR_DrawGameMessage(); // H2

			SCR_AddTimeGraph(); // H2
			SCR_DrawDebugGraph();
			SCR_DrawConsole();
			SCR_DrawPause();
			SCR_DrawLoading();

			M_Draw();
		}
	}

	re.EndFrame();
}
