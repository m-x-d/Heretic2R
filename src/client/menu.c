//
// menu.c -- Menu subsystem
//
// Copyright 1998 Raven Software
//

#include <ctype.h>
#include "client.h"
#include "cl_messages.h"
#include "vid_dll.h"
#include "menu.h"

#include "menus/menu_actionkeys.h"
#include "menus/menu_addressbook.h"
#include "menus/menu_cameracfg.h"
#include "menus/menu_citymap.h"
#include "menus/menu_credits.h"
#include "menus/menu_dmoptions.h"
#include "menus/menu_doubletapkeys.h"
#include "menus/menu_downloadoptions.h"
#include "menus/menu_game.h"
#include "menus/menu_help.h"
#include "menus/menu_info.h"
#include "menus/menu_joinserver.h"
#include "menus/menu_keys.h"
#include "menus/menu_loadcfg.h"
#include "menus/menu_loadgame.h"
#include "menus/menu_main.h"
#include "menus/menu_misc.h"
#include "menus/menu_mousecfg.h"
#include "menus/menu_movekeys.h"
#include "menus/menu_multiplayer.h"
#include "menus/menu_objectives.h"
#include "menus/menu_options.h"
#include "menus/menu_playerconfig.h"
#include "menus/menu_quit.h"
#include "menus/menu_savegame.h"
#include "menus/menu_shortkeys.h"
#include "menus/menu_sound.h"
#include "menus/menu_startserver.h"
#include "menus/menu_video.h"
#include "menus/menu_worldmap.h"

cvar_t* menus_active;
cvar_t* quick_menus; //TODO: add UI control (in menu_misc.c menu?).

cvar_t* m_item_defaults;

// Controls
cvar_t* m_item_attack;
cvar_t* m_item_defend;
cvar_t* m_item_action;
cvar_t* m_item_lookup;
cvar_t* m_item_lookdown;
cvar_t* m_item_centerview;
cvar_t* m_item_mouselook;
cvar_t* m_item_keyboardlook;
cvar_t* m_item_lookaround;
cvar_t* m_item_nextweapon;
cvar_t* m_item_prevweapon;
cvar_t* m_item_nextdef;
cvar_t* m_item_prevdef;
cvar_t* m_item_walkforward;
cvar_t* m_item_backpedal;
cvar_t* m_item_turnleft;
cvar_t* m_item_turnright;
cvar_t* m_item_creep;
cvar_t* m_item_run;
cvar_t* m_item_stepleft;
cvar_t* m_item_stepright;
cvar_t* m_item_sidestep;
cvar_t* m_item_up;
cvar_t* m_item_down;
cvar_t* m_item_quickturn;
cvar_t* m_item_powerup;
cvar_t* m_item_bluering;
cvar_t* m_item_meteor;
cvar_t* m_item_morph;
cvar_t* m_item_teleport;
cvar_t* m_item_shield;
cvar_t* m_item_tornado;
cvar_t* m_item_inventory;
cvar_t* m_item_messagemode;
cvar_t* m_item_frags;
cvar_t* m_item_flipleft;
cvar_t* m_item_flipright;
cvar_t* m_item_flipforward;
cvar_t* m_item_flipback;
cvar_t* m_item_rollleft;
cvar_t* m_item_rollright;
cvar_t* m_item_rollforward;
cvar_t* m_item_rollback;
cvar_t* m_item_spinattack;

// Generic menu labels
cvar_t* m_generic_yes;
cvar_t* m_generic_no;
cvar_t* m_generic_high;
cvar_t* m_generic_low;
cvar_t* m_generic_on;
cvar_t* m_generic_off;
cvar_t* m_generic_violence0;
cvar_t* m_generic_violence1;
cvar_t* m_generic_violence2;
cvar_t* m_generic_violence3;
cvar_t* m_generic_crosshair0;
cvar_t* m_generic_crosshair1;
cvar_t* m_generic_crosshair2;
cvar_t* m_generic_crosshair3;
cvar_t* m_dmlist;
cvar_t* m_cooplist;

// H2. Generic menu item labels texts.
char m_text_no[MAX_QPATH];
char m_text_yes[MAX_QPATH];
char m_text_off[MAX_QPATH];
char m_text_on[MAX_QPATH];
char m_text_low[MAX_QPATH];
char m_text_high[MAX_QPATH];

char* yes_no_names[] = { "no", "yes", 0 };

static qboolean m_entersound; // Play after drawing a frame, so caching won't disrupt the sound. //TODO: doesn't seem to be related to playing sounds. Rename?

m_drawfunc_t m_drawfunc;
m_keyfunc_t m_keyfunc;
m_keyfunc_t m_keyfunc2; // H2 //TODO: better name

#define MAX_MENU_DEPTH	8

typedef struct
{
	m_drawfunc_t draw;
	m_keyfunc_t key;
} menulayer_t;

uint m_menu_side; // H2 (0 - right, 1 - left)
static menulayer_t m_layers[MAX_MENU_DEPTH + 1]; // Q2: MAX_MENU_DEPTH
static int m_menudepth;

static void OnMainMenuOpened(void) // H2
{
	Com_sprintf(m_text_no, sizeof(m_text_no), "\x02%s", m_generic_no->string);
	Com_sprintf(m_text_yes, sizeof(m_text_yes), "\x02%s", m_generic_yes->string);
	Com_sprintf(m_text_off, sizeof(m_text_off), "\x02%s", m_generic_off->string);
	Com_sprintf(m_text_on, sizeof(m_text_on), "\x02%s", m_generic_on->string);
	Com_sprintf(m_text_low, sizeof(m_text_low), "\x02%s", m_generic_low->string);
	Com_sprintf(m_text_high, sizeof(m_text_high), "\x02%s", m_generic_high->string);

	Cvar_Set("menus_active", "1");

	if (se.SetEaxEnvironment != NULL)
		se.SetEaxEnvironment(0);
}

// Similar to M_PushMenu() in Q2.
static void PushMenu(const m_drawfunc_t draw, const m_keyfunc_t key) 
{
	int depth;

	if (Cvar_VariableValue("maxclients") == 1.0f && Com_ServerState())
		Cvar_Set("paused", "1");

	for (depth = 0; depth < m_menudepth; depth++)
		if (m_layers[depth + 1].draw == draw && m_layers[depth + 1].key == key)
			m_menudepth = depth;

	if (depth == m_menudepth)
	{
		if (m_menudepth >= MAX_MENU_DEPTH)
			Com_Error(ERR_FATAL, "M_PushMenu: MAX_MENU_DEPTH");

		m_layers[m_menudepth + 1].draw = draw;
		m_layers[m_menudepth + 1].key = key;
		m_menudepth++;
	}
}

void M_PushMenu(const m_drawfunc_t draw, const m_keyfunc_t key) // H2
{
	if (cl.cinematictime > 0)
	{
		SCR_FinishCinematic();
		return;
	}

	if (!scr_draw_loading_plaque && !m_entersound && !cl.frame.playerstate.cinematicfreeze)
	{
		m_entersound = true;
		m_drawfunc = draw;
		m_keyfunc = key;

		if (m_menudepth == 0)
		{
			cls.m_menustate = 5;
			OnMainMenuOpened();
		}
		else
		{
			cls.m_menustate = 3;
		}

		cls.key_dest = key_menu;
		PushMenu(draw, key);
	}
}

void M_PopMenu(void)
{
	if (m_menudepth < 1)
		Com_Error(ERR_FATAL, "M_PopMenu: depth < 1");

	m_menudepth--;
	if (m_menudepth > 0)
	{
		m_drawfunc = m_layers[m_menudepth].draw;
		m_keyfunc = m_layers[m_menudepth].key;
	}
	else
	{
		m_drawfunc = NULL;
		m_keyfunc = NULL;
	}

	cls.m_menustate = 3;
}

char* Default_MenuKey(menuframework_t* menu, const int key)
{
	if (cls.m_menustate != 2)
		return NULL;

	if (menu != NULL)
	{
		menucommon_t* item = Menu_ItemAtCursor(menu);
		if (item != NULL && item->type == MTYPE_FIELD && Field_Key((menufield_t*)item, key))
			return NULL;
	}

	switch (key)
	{
		case K_TAB:
		case K_DOWNARROW:
		case K_KP_DOWNARROW:
			if (menu != NULL)
			{
				menu->cursor++;
				Menu_AdjustCursor(menu, 1);
				return SND_MENU2;
			}
			break;

		case K_ENTER:
		case K_KP_ENTER:
		case K_MOUSE1:
		case K_MOUSE2:
		case K_MOUSE3:
		case K_JOY1:
		case K_JOY2:
		case K_JOY3:
		case K_JOY4:
		case K_AUX1:
		case K_AUX2:
		case K_AUX3:
		case K_AUX4:
		case K_AUX5:
		case K_AUX6:
		case K_AUX7:
		case K_AUX8:
		case K_AUX9:
		case K_AUX10:
		case K_AUX11:
		case K_AUX12:
		case K_AUX13:
		case K_AUX14:
		case K_AUX15:
		case K_AUX16:
		case K_AUX17:
		case K_AUX18:
		case K_AUX19:
		case K_AUX20:
		case K_AUX21:
		case K_AUX22:
		case K_AUX23:
		case K_AUX24:
		case K_AUX25:
		case K_AUX26:
		case K_AUX27:
		case K_AUX28:
		case K_AUX29:
		case K_AUX30:
		case K_AUX31:
		case K_AUX32:
			if (menu != NULL && Menu_SelectItem(menu))
			{
				const menucommon_t* item = Menu_ItemAtCursor(menu);
				if (item->flags & QMF_SELECT_SOUND)
					return SND_MENU1;
			}
			break;

		case K_ESCAPE:
			M_PopMenu();
			return SND_MENU3;

		case K_UPARROW:
		case K_KP_UPARROW:
			if (menu != NULL)
			{
				menu->cursor--;
				Menu_AdjustCursor(menu, -1);
				return SND_MENU2;
			}
			break;

		case K_LEFTARROW:
		case K_KP_LEFTARROW:
			if (menu != NULL)
			{
				Menu_SlideItem(menu, -1);
				return SND_MENU4;
			}
			break;

		case K_RIGHTARROW:
		case K_KP_RIGHTARROW:
			if (menu != NULL)
			{
				Menu_SlideItem(menu, 1);
				return SND_MENU4;
			}
			break;

		default:
			break;
	}

	return NULL;
}

const char* Generic_MenuKey(const int key) // H2
{
	if (cls.m_menustate == 2 && key != 0)
		M_PopMenu();

	return NULL;
}

qboolean Field_Key(menufield_t* field, int key)
{
	const int orig_key = key; //mxd

	switch (key)
	{
		case K_KP_HOME:
			key = '7';
			break;

		case K_KP_UPARROW:
			key = '8';
			break;

		case K_KP_PGUP:
			key = '9';
			break;

		case K_KP_LEFTARROW:
			key = '4';
			break;

		case K_KP_5:
			key = '5';
			break;

		case K_KP_RIGHTARROW:
			key = '6';
			break;

		case K_KP_END:
			key = '1';
			break;

		case K_KP_DOWNARROW:
			key = '2';
			break;

		case K_KP_PGDN:
			key = '3';
			break;

		case K_KP_INS:
			key = '0';
			break;

		case K_KP_DEL:
			key = '.';
			break;

		case K_KP_SLASH:
			key = K_SLASH;
			break;

		case K_KP_MINUS:
			key = '-';
			break;

		case K_KP_PLUS:
			key = '+';
			break;

		default:
			break;
	}

	// Support pasting from the clipboard.
	if ((toupper(key) == 'V' && keydown[K_CTRL]) || ((key == K_INS || orig_key == K_KP_INS) && keydown[K_SHIFT])) //mxd. K_KP_INS was already remapped to '0' above, so check orig_key.
	{
		char cliptext[256];
		IN_GetClipboardText(cliptext, sizeof(cliptext)); // YQ2

		char* ptr = NULL; //mxd
		strtok_s(cliptext, "\n\r\b", &ptr); //mxd. strtok -> strtok_s
		strncpy_s(field->buffer, sizeof(field->buffer), cliptext, field->length - 1); //mxd. strncpy -> strncpy_s

		field->cursor = (int)strlen(field->buffer);
		field->visible_offset = max(0, field->cursor - field->visible_length);

		return true;
	}

	switch (key)
	{
		case K_TAB:
		case K_ENTER:
		case K_ESCAPE:
		case K_KP_ENTER:
			return false;

		case K_BACKSPACE:
		case K_LEFTARROW:
			if (field->cursor > 0)
			{
				field->cursor--;
				field->buffer[field->cursor] = 0;
				if (field->visible_offset > 0)
					field->visible_offset--;
			}
			break;

		case K_RIGHTARROW: // H2
			if (field->cursor < (int)strlen(field->buffer))
				field->cursor++;
			break;

		case K_DEL:
			break;

		default:
			if (key > 127 || (!isdigit(key) && (field->generic.flags & QMF_NUMBERSONLY)))
				return false;

			if (field->cursor < field->length - 1) // Q2: if (field->cursor < field->length)
			{
				field->buffer[field->cursor++] = (char)key;
				field->buffer[field->cursor] = 0;
				if (field->visible_length < field->cursor)
					field->visible_offset++;
			}
			break;
	}

	return true;
}

void M_ForceMenuOff(void)
{
	m_layers[0].draw = NULL;
	m_keyfunc2 = NULL;
	cls.key_dest = key_game;
	cls.m_menustate = 0;
	m_menudepth = 0;

	Key_ClearStates();
	SCR_StopCinematic(); // H2

	Cvar_Set("paused", "0");
	Cvar_Set("menus_active", "0"); // H2

	// H2: update EAX preset.
	if (se.SetEaxEnvironment != NULL)
		se.SetEaxEnvironment((int)EAX_preset->value);

	m_entersound = false;
}

void M_Init(void)
{
	Cmd_AddCommand("menu_main", M_Menu_Main_f);
	Cmd_AddCommand("menu_game", M_Menu_Game_f);
	Cmd_AddCommand("menu_loadgame", M_Menu_Loadgame_f);
	Cmd_AddCommand("menu_savegame", M_Menu_Savegame_f);
	Cmd_AddCommand("menu_credits", M_Menu_Credits_f);
	Cmd_AddCommand("menu_multiplayer", M_Menu_Multiplayer_f);
	Cmd_AddCommand("menu_joinserver", M_Menu_JoinServer_f);
	Cmd_AddCommand("menu_addressbook", M_Menu_AddressBook_f);
	Cmd_AddCommand("menu_startserver", M_Menu_StartServer_f);
	Cmd_AddCommand("menu_dmoptions", M_Menu_DMOptions_f);
	Cmd_AddCommand("menu_playerconfig", M_Menu_PlayerConfig_f);
	Cmd_AddCommand("menu_download", M_Menu_DownloadOptions_f);
	Cmd_AddCommand("menu_video", M_Menu_Video_f);
	Cmd_AddCommand("menu_sound", M_Menu_Sound_f);
	Cmd_AddCommand("menu_options", M_Menu_Options_f);
	Cmd_AddCommand("menu_action_keys", M_Menu_ActionKeys_f);
	Cmd_AddCommand("menu_move_keys", M_Menu_MoveKeys_f);
	Cmd_AddCommand("menu_short_keys", M_Menu_ShortKeys_f);
	Cmd_AddCommand("menu_help", M_Menu_Help_f);
	Cmd_AddCommand("menu_dt_keys", M_Menu_DoubletapKeys_f);
	Cmd_AddCommand("menu_loadcfg", M_Menu_LoadCfg_f);
	Cmd_AddCommand("menu_mousecfg", M_Menu_MouseCfg_f);
	Cmd_AddCommand("menu_cameracfg", M_Menu_CameraCfg_f);
	Cmd_AddCommand("menu_misc", M_Menu_Misc_f);
	Cmd_AddCommand("menu_quit", M_Menu_Quit_f);
	Cmd_AddCommand("menu_info", M_Menu_Info_f);
	Cmd_AddCommand("menu_world_map", M_Menu_World_Map_f);
	Cmd_AddCommand("menu_city_map", M_Menu_City_Map_f);
	Cmd_AddCommand("menu_objectives", M_Menu_Objectives_f);

	quick_menus = Cvar_Get("quick_menus", "0", 0);
	menus_active = Cvar_Get("menus_active", "0", 0);

	m_banner_main = Cvar_Get("m_banner_main", "Main", 0);
	m_banner_game = Cvar_Get("m_banner_game", "Game", 0);
	m_banner_quit = Cvar_Get("m_banner_quit", "Quit", 0);
	m_banner_address = Cvar_Get("m_banner_address", "Address Book", 0);
	m_banner_options = Cvar_Get("m_banner_options", "Options", 0);
	m_banner_join = Cvar_Get("m_banner_join", "Join Server", 0);
	m_banner_load = Cvar_Get("m_banner_load", "Load Game", 0);
	m_banner_save = Cvar_Get("m_banner_save", "Save Game", 0);
	m_banner_multi = Cvar_Get("m_banner_multi", "Multiplayer", 0);
	m_banner_video = Cvar_Get("m_banner_video", "Video Settings", 0);
	m_banner_sound = Cvar_Get("m_banner_sound", "Sound Settings", 0);
	m_banner_startserver = Cvar_Get("m_banner_startserver", "Start Server", 0);
	m_banner_dmopt = Cvar_Get("m_banner_dmopt", "Deathmatch Flags", 0);
	m_banner_action_keys = Cvar_Get("m_banner_action_keys", "Action Keys", 0);
	m_banner_move_keys = Cvar_Get("m_banner_move_keys", "Move Keys", 0);
	m_banner_short_keys = Cvar_Get("m_banner_short_keys", "Shortcut Keys", 0);
	m_banner_dt_keys = Cvar_Get("m_banner_dt_keys", "Doubletap Keys", 0);
	m_banner_credits = Cvar_Get("m_banner_credits", "Credits", 0);
	m_banner_loadcfg = Cvar_Get("m_banner_loadcfg", "Load Config", 0);
	m_banner_savecfg = Cvar_Get("m_banner_savecfg", "Save Config", 0);
	m_banner_pconfig = Cvar_Get("m_banner_pconfig", "Player Config", 0);
	m_banner_download = Cvar_Get("m_banner_download", "Download Options", 0);
	m_banner_worldmap = Cvar_Get("m_banner_worldmap", "Map of Parthoris", 0);
	m_banner_citymap = Cvar_Get("m_banner_citymap", "City Map", 0);
	m_banner_objectives = Cvar_Get("m_banner_objectives", "Objective", 0);
	m_banner_mousecfg = Cvar_Get("m_banner_mousecfg", "Mouse Config", 0);
	m_banner_cameracfg = Cvar_Get("m_banner_cameracfg", "Camera Config", 0);
	m_banner_misc = Cvar_Get("m_banner_misc", "More Options", 0);
	m_banner_info = Cvar_Get("m_banner_info", "Maps/Objectives", 0);

	// Game menu.
	m_item_tutorial = Cvar_Get("m_item_tutorial", "Tutorial", 0);
	m_item_easy = Cvar_Get("m_item_easy", "Easy", 0);
	m_item_medium = Cvar_Get("m_item_medium", "Medium", 0);
	m_item_hard = Cvar_Get("m_item_hard", "Hard", 0);
	m_item_nightmare = Cvar_Get("m_item_nightmare", "Humiliation (Too hard)", 0); //H2: "Insane".

	// Join Server menu.
	m_item_refresh = Cvar_Get("m_item_refresh", "Refresh Server List", 0);

	// Start Server menu.
	m_item_begin = Cvar_Get("m_item_begin", "Start the Slaughter", 0);
	m_item_startmap = Cvar_Get("m_item_startmap", "Initial Map", 0);
	m_item_rules = Cvar_Get("m_item_rules", "Rules", 0);
	m_item_timelimit = Cvar_Get("m_item_timelimit", "Time Limit", 0);
	m_item_fraglimit = Cvar_Get("m_item_fraglimit", "Frag Limit", 0);
	m_item_maxplayers = Cvar_Get("m_item_maxplayers", "Max Players", 0);
	m_item_hostname = Cvar_Get("m_item_hostname", "Hostname", 0);
	m_item_deathmatch = Cvar_Get("m_item_deathmatch", "Deathmatch", 0);
	m_item_coop = Cvar_Get("m_item_coop", "Cooperative", 0);
	m_dmlist = Cvar_Get("m_dmlist", "dm", 0);
	m_cooplist = Cvar_Get("m_cooplist", "coop", 0);

	// Mouse Config menu.
	m_item_mousespeedx = Cvar_Get("m_item_mousespeedx", "Mouse Horizontal Speed", 0);
	m_item_mousespeedy = Cvar_Get("m_item_mousespeedy", "Mouse Vertical Speed", 0);
	m_item_freelook = Cvar_Get("m_item_freelook", "Freelook", 0);

	// Misc menu.
	m_item_alwaysrun = Cvar_Get("m_item_alwaysrun", "Always run", 0);
	m_item_mouseinvert = Cvar_Get("m_item_mouseinvert", "Invert Mouse", 0);
	m_item_lookspring = Cvar_Get("m_item_lookspring", "Lookspring", 0);
	m_item_crosshair = Cvar_Get("m_item_crosshair", "Crosshair", 0);
	m_item_autotarget = Cvar_Get("m_item_autotarget", "Auto Target", 0);
	m_item_caption = Cvar_Get("m_item_caption", "Captioning", 0);
	m_item_violence = Cvar_Get("m_item_violence", "Violence Level", 0);
	m_item_yawspeed = Cvar_Get("m_item_yawspeed", "Key Turn Speed", 0);
	m_item_console = Cvar_Get("m_item_console", "Go to console", 0);

	// Video Settings menu.
	m_item_driver = Cvar_Get("m_item_driver", "Renderer", 0);
	m_item_vidmode = Cvar_Get("m_item_vidmode", "Video Resolution", 0);
	m_item_gamma = Cvar_Get("m_item_gamma", "Gamma", 0);
	m_item_brightness = Cvar_Get("m_item_brightness", "Brightness", 0);
	m_item_contrast = Cvar_Get("m_item_contrast", "Contrast", 0);
	m_item_detail = Cvar_Get("m_item_detail", "Detail Level", 0);

	// Options / Video Settings menus.
	m_item_defaults = Cvar_Get("m_item_defaults", "Reset to Defaults", 0);

	// Sound Settings menu.
	m_item_effectsvol = Cvar_Get("m_item_effectsvol", "Effects Volume", 0);
	m_item_musicvol = Cvar_Get("m_item_musicvol", "Music Volume", 0); //mxd
	m_item_soundquality = Cvar_Get("m_item_soundquality", "Sound Quality", 0);

	// Download Options menu.
	m_item_allowdownload = Cvar_Get("m_item_allowdownload", "Allow Downloads", 0);
	m_item_allowdownloadmap = Cvar_Get("m_item_allowdownloadmap", "Download Maps", 0);
	m_item_allowdownloadsound = Cvar_Get("m_item_allowdownloadsound", "Download Sounds", 0);
	m_item_allowdownloadplayer = Cvar_Get("m_item_allowdownloadplayer", "New Player Skins", 0);
	m_item_allowdownloadmodel = Cvar_Get("m_item_allowdownloadmodel", "Download Models", 0);

	// DM Options menu.
	m_item_weaponsstay = Cvar_Get("m_item_weaponsstay", "Weapons Stay", 0);
	m_item_allowpowerup = Cvar_Get("m_item_allowpowerup", "Allow Shrines", 0);
	m_item_shrinechaos = Cvar_Get("m_item_shrinechaos", "All Shrines Chaos", 0);
	m_item_allowhealth = Cvar_Get("m_item_allowhealth", "Allow Health", 0);
	m_item_leader = Cvar_Get("m_item_leader", "Show Leader", 0);
	m_item_offensive_spell = Cvar_Get("m_item_offensive_spell", "Offensive Weapons", 0);
	m_item_defensive_spell = Cvar_Get("m_item_defensive_spell", "Defensive Spells", 0);
	m_item_samemap = Cvar_Get("m_item_samemap", "Same Map", 0);
	m_item_forcerespawn = Cvar_Get("m_item_forcerespawn", "Force Respawn", 0);
	m_item_teamplay = Cvar_Get("m_item_teamplay", "Team Play", 0);
	m_item_allowexit = Cvar_Get("m_item_allowexit", "Allow Exit", 0);
	m_item_infinitemana = Cvar_Get("m_item_infinitemana", "Infinite Mana", 0);
	m_item_friendlyfire = Cvar_Get("m_item_friendlyfire", "Friendly Fire", 0);
	m_item_dismember = Cvar_Get("m_item_dismember", "Dismemberment", 0);
	m_item_nonames = Cvar_Get("m_item_nonames", "No Names", 0);

	// Player Config menu.
	m_item_name = Cvar_Get("m_item_name", "Name", 0);
	m_item_skin = Cvar_Get("m_item_skin", "Skin", 0);
	m_item_shownames = Cvar_Get("m_item_shownames", "Show Player Names", 0);
	m_item_autoweapon = Cvar_Get("m_item_autoweapon", "Auto Weapon Change", 0);

	// Objectives menu.
	m_item_none = Cvar_Get("m_item_none", "No objectives have been written", 0);

	// City map menu.
	m_item_nomap = Cvar_Get("m_item_nomap", "No Map Available", 0);

	// Camera cfg menu.
	m_item_cameradamp = Cvar_Get("m_item_cameradamp", "Camera Stiffness", 0);
	m_item_cameracombat = Cvar_Get("m_item_cameracombat", "Combat Camera", 0);

	// Keys menu.
	m_item_helpscreen = Cvar_Get("m_item_helpscreen", "Help Screen", 0);
	m_item_attack = Cvar_Get("m_item_attack", "Attack", 0);
	m_item_defend = Cvar_Get("m_item_defend", "Defend", 0);
	m_item_action = Cvar_Get("m_item_action", "Action", 0);
	m_item_lookup = Cvar_Get("m_item_lookup", "Look Up", 0);
	m_item_lookdown = Cvar_Get("m_item_lookdown", "Look Down", 0);
	m_item_centerview = Cvar_Get("m_item_centerview", "Center View", 0);
	m_item_mouselook = Cvar_Get("m_item_mouselook", "Mouselook", 0);
	m_item_keyboardlook = Cvar_Get("m_item_keyboardlook", "Keyboardlook", 0);
	m_item_lookaround = Cvar_Get("m_item_lookaround", "Look Around", 0);
	m_item_nextweapon = Cvar_Get("m_item_nextweapon", "Next Weapon", 0);
	m_item_prevweapon = Cvar_Get("m_item_prevweapon", "Previous Weapon", 0);
	m_item_nextdef = Cvar_Get("m_item_nextdef", "Next Defence", 0);
	m_item_prevdef = Cvar_Get("m_item_prevdef", "Previous Defence", 0);
	m_item_walkforward = Cvar_Get("m_item_walkforward", "Walk Forward", 0);
	m_item_backpedal = Cvar_Get("m_item_backpedal", "Back Pedal", 0);
	m_item_turnleft = Cvar_Get("m_item_turnleft", "Turn Left", 0);
	m_item_turnright = Cvar_Get("m_item_turnright", "Turn Right", 0);
	m_item_creep = Cvar_Get("m_item_creep", "Creep", 0);
	m_item_run = Cvar_Get("m_item_run", "Run", 0);
	m_item_stepleft = Cvar_Get("m_item_stepleft", "Step Left", 0);
	m_item_stepright = Cvar_Get("m_item_stepright", "Step Right", 0);
	m_item_sidestep = Cvar_Get("m_item_sidestep", "Sidestep", 0);
	m_item_up = Cvar_Get("m_item_up", "Up / Jump", 0);
	m_item_down = Cvar_Get("m_item_down", "Down / Crouch", 0);
	m_item_quickturn = Cvar_Get("m_item_quickturn", "Quick Turn", 0);
	m_item_powerup = Cvar_Get("m_item_powerup", "Power Up", 0);
	m_item_bluering = Cvar_Get("m_item_bluering", "Ring of Repulsion", 0);
	m_item_meteor = Cvar_Get("m_item_meteor", "Meteor Swarm", 0);
	m_item_morph = Cvar_Get("m_item_morph", "Polymorph", 0);
	m_item_teleport = Cvar_Get("m_item_teleport", "Teleport", 0);
	m_item_shield = Cvar_Get("m_item_shield", "Lightning Shield", 0);
	m_item_tornado = Cvar_Get("m_item_tornado", "Whirlwind", 0);
	m_item_inventory = Cvar_Get("m_item_inventory", "Inventory", 0);
	m_item_messagemode = Cvar_Get("m_item_messagemode", "Say (Talk)", 0);
	m_item_frags = Cvar_Get("m_item_frags", "Frags (Score)", 0);
	m_item_flipleft = Cvar_Get("m_item_flipleft", "Flip Left", 0);
	m_item_flipright = Cvar_Get("m_item_flipright", "Flip Right", 0);
	m_item_flipforward = Cvar_Get("m_item_flipforward", "Flip Forward", 0);
	m_item_flipback = Cvar_Get("m_item_flipback", "Flip Back", 0);
	m_item_rollleft = Cvar_Get("m_item_rollleft", "Roll Left", 0);
	m_item_rollright = Cvar_Get("m_item_rollright", "Roll Right", 0);
	m_item_rollforward = Cvar_Get("m_item_rollforward", "Roll Forward", 0);
	m_item_rollback = Cvar_Get("m_item_rollback", "Roll Back", 0);
	m_item_spinattack = Cvar_Get("m_item_spinattack", "Spin Attack", 0);

	m_generic_yes = Cvar_Get("m_generic_yes", "Yes", 0);
	m_generic_no = Cvar_Get("m_generic_no", "No", 0);
	m_generic_high = Cvar_Get("m_generic_high", "High", 0);
	m_generic_low = Cvar_Get("m_generic_low", "Low", 0);
	m_generic_on = Cvar_Get("m_generic_on", "On", 0);
	m_generic_off = Cvar_Get("m_generic_off", "Off", 0);
	m_generic_violence0 = Cvar_Get("m_generic_violence0", "No Blood", 0);
	m_generic_violence1 = Cvar_Get("m_generic_violence1", "No Gibbing", 0);
	m_generic_violence2 = Cvar_Get("m_generic_violence2", "Normal", 0);
	m_generic_violence3 = Cvar_Get("m_generic_violence3", "Ultraviolent", 0);
	m_generic_crosshair0 = Cvar_Get("m_generic_crosshair0", "None", 0);
	m_generic_crosshair1 = Cvar_Get("m_generic_crosshair1", "Cross", 0);
	m_generic_crosshair2 = Cvar_Get("m_generic_crosshair2", "Circle", 0);
	m_generic_crosshair3 = Cvar_Get("m_generic_crosshair3", "Symbol", 0);

	Cvar_Get("blood_level", "0", CVAR_ARCHIVE);
	Cvar_Get("dm_no_bodies", "0", CVAR_ARCHIVE);
}

float M_GetMenuAlpha(void) // H2
{
	return cls.m_menualpha;
}

int M_GetMenuLabelX(const int text_width) // H2
{
#define MENU_CENTER_X		(DEF_WIDTH / 2) //mxd
#define BOOK_PAGE_PADDING	32 // Horizontal gap between book cover and page.
#define BOOK_PAGE_WIDTH		(MENU_CENTER_X - BOOK_PAGE_PADDING)

	const int x = (BOOK_PAGE_WIDTH - text_width) / 2;
	return x + ((m_menu_side & 1) ? MENU_CENTER_X + 6 : BOOK_PAGE_PADDING); //mxd. Right page is a bit off-center...
}

int M_GetMenuOffsetY(const menuframework_t* menu) // H2
{
	return menu->y - 24;
}

void Menu_AddItem(menuframework_t* menu, void* item)
{
	if (menu->nitems < MAXMENUITEMS)
	{
		menu->items[menu->nitems] = item;
		((menucommon_t*)menu->items[menu->nitems])->parent = menu;
		menu->nitems++;
	}
}

// Q2 counterpart
static qboolean Field_DoEnter(menufield_t* field)
{
	if (field->generic.callback != NULL)
	{
		field->generic.callback(field);
		return true;
	}

	return false;
}

// Q2 counterpart
static void Action_DoEnter(menuaction_t* action)
{
	if (action->generic.callback != NULL)
		action->generic.callback(action);
}

qboolean Menu_SelectItem(const menuframework_t* menu)
{
	menucommon_t* item = Menu_ItemAtCursor(menu);
	if (item != NULL)
	{
		switch (item->type)
		{
			case MTYPE_FIELD:
				return Field_DoEnter((menufield_t*)item);

			case MTYPE_ACTION:
				Action_DoEnter((menuaction_t*)item);
				return true;

			default:
				break;
		}
	}

	return false;
}

// Q2 counterpart
static void Slider_DoSlide(menuslider_t* slider, const int dir)
{
	slider->curvalue += (float)dir;
	slider->curvalue = Clamp(slider->curvalue, slider->minvalue, slider->maxvalue);

	if (slider->generic.callback != NULL)
		slider->generic.callback(slider);
}

// Q2 counterpart
static void SpinControl_DoSlide(menulist_t* spin_ctrl, const int dir)
{
	spin_ctrl->curvalue += dir;

	if (spin_ctrl->curvalue < 0)
		spin_ctrl->curvalue = 0;
	else if (spin_ctrl->itemnames[spin_ctrl->curvalue] == NULL)
		spin_ctrl->curvalue--;

	if (spin_ctrl->generic.callback != NULL)
		spin_ctrl->generic.callback(spin_ctrl);
}

void Menu_SlideItem(const menuframework_t* menu, const int dir)
{
	menucommon_t* item = Menu_ItemAtCursor(menu);
	if (item == NULL)
		return;

	switch (item->type)
	{
		case MTYPE_SLIDER:
			Slider_DoSlide((menuslider_t*)item, dir);
			break;

		case MTYPE_SPINCONTROL:
		case MTYPE_PLAYER_SKIN: // H2
			SpinControl_DoSlide((menulist_t*)item, dir);
			break;

		default:
			break;
	}
}

// This function takes the given menu, the direction, and attempts to adjust
// the menu's cursor so that it's at the next available slot.
void Menu_AdjustCursor(menuframework_t* menu, const int dir)
{
	// See if it's in a valid spot.
	if (menu->cursor >= 0 && menu->cursor < menu->nitems && Menu_ItemAtCursor(menu) != NULL)
		return;

	// It's not in a valid spot, so crawl in the direction indicated until we find a valid spot.
	if (dir == 1)
	{
		while (Menu_ItemAtCursor(menu) == NULL)
		{
			menu->cursor += dir;
			if (menu->cursor >= menu->nitems)
				menu->cursor = 0;
		}
	}
	else
	{
		while (Menu_ItemAtCursor(menu) == NULL)
		{
			menu->cursor += dir;
			if (menu->cursor < 0)
				menu->cursor = menu->nitems - 1;
		}
	}
}

void Menu_Center(menuframework_t* menu)
{
	int width = 0;
	for (int i = 0; i < menu->nitems; i++)
		width = max(width, ((menucommon_t*)menu->items[i])->width);

	menu->width = width;

	const int height = ((menucommon_t*)menu->items[menu->nitems - 1])->y + 10;
	menu->y = (DEF_HEIGHT - height) / 2;
}

static void Slider_Draw(menuslider_t* slider, const qboolean selected)
{
#define SLIDER_RANGE 10

	char item_name[64];

	const float alpha = M_GetMenuAlpha();
	const paletteRGBA_t color = { .a = (byte)Q_ftol(alpha * 255.0f), .r = 0xff, .g = 0xff, .b = 0xff};

	// Draw slider name.
	int x = M_GetMenuLabelX(slider->generic.width);
	int y = slider->generic.y + slider->generic.parent->y;
	strcpy_s(item_name, sizeof(item_name), slider->generic.name); //mxd. strcpy -> strcpy_s
	Menu_DrawString(x, y, item_name, alpha, selected);

	// Update range.
	slider->range = (slider->curvalue - slider->minvalue) / (slider->maxvalue - slider->minvalue);
	slider->range = Clamp(slider->range, 0, 1);

	// Draw BG left.
	x = M_GetMenuLabelX(8 * SLIDER_RANGE * DEF_WIDTH / viddef.width);
	x = x * viddef.width / DEF_WIDTH;
	y = (y + 10) * viddef.height / DEF_HEIGHT;
	re.DrawChar(x - 8, y, 15, color);

	// Draw BG mid.
	int ox = x;
	for (int i = 0; i < SLIDER_RANGE; i++, ox += 8)
		re.DrawChar(ox, y, 1, color);

	// Draw BG right.
	re.DrawChar(x + 8 * SLIDER_RANGE, y, 2, color);

	// Draw slider value.
	re.DrawChar(x + Q_ftol(slider->range * 8 * (SLIDER_RANGE - 1)), y, 3, color);
}

static void Field_Draw(const menufield_t* field, const qboolean selected)
{
	char tempbuffer[128];

	const float alpha = M_GetMenuAlpha();
	const paletteRGBA_t color = { .a = (byte)Q_ftol(alpha * 255.0f), .r = 0xff, .g = 0xff, .b = 0xff };
	int oy = field->generic.y + field->generic.parent->y;

	if (field->generic.name != NULL)
	{
		const int name_x = M_GetMenuLabelX(field->generic.width);
		Menu_DrawString(name_x, oy, field->generic.name, alpha, selected);
		oy += 20;
	}

	const int label_x = M_GetMenuLabelX(field->visible_length * 8 * DEF_WIDTH / viddef.width);
	const int ox = label_x * viddef.width / DEF_WIDTH;
	oy = (oy - 8) * viddef.height / DEF_HEIGHT;

	strncpy_s(tempbuffer, sizeof(tempbuffer), field->buffer + field->visible_offset, field->visible_length); //mxd. strncpy -> strncpy_s

	re.DrawChar(ox - 8, oy - 4, 18, color);
	re.DrawChar(ox - 8, oy + 4, 24, color);
	re.DrawChar(ox + field->visible_length * 8, oy - 4, 20, color);
	re.DrawChar(ox + field->visible_length * 8, oy + 4, 26, color);

	int field_x = ox;
	for (int i = 0; i < field->visible_length; i++, field_x += 8)
	{
		re.DrawChar(field_x, oy - 4, 19, color);
		re.DrawChar(field_x, oy + 4, 25, color);
	}

	DrawString(ox, oy, tempbuffer, TextPalette[P_MENUFIELD], -1);

	if ((menufield_t*)Menu_ItemAtCursor(field->generic.parent) == field)
	{
		const int offset = (field->visible_offset != 0 ? field->visible_length : field->cursor);
		const int ch = (Sys_Milliseconds() / 250 & 1 ? 11 : ' ');
		re.DrawChar(ox + offset * 8, oy, ch, color);
	}
}

static void Action_Draw(const menuaction_t* action, const qboolean selected)
{
	int x;
	char name[MAX_QPATH];

	const float alpha = M_GetMenuAlpha();
	const int y = action->generic.y + action->generic.parent->y;

	if (action->generic.flags & QMF_MULTILINE && strchr(action->generic.name, '\n') != NULL)
	{
		// Draw left part
		strcpy_s(name, sizeof(name), action->generic.name); //mxd. strcpy -> strcpy_s
		*strchr(name, '\n') = 0;
		x = M_GetMenuLabelX(re.BF_Strlen(name));
		Menu_DrawString(x, y, name, alpha, selected);
		m_menu_side ^= 1;

		// Draw right part
		strcpy_s(name, sizeof(name), strchr(action->generic.name, '\n') + 1); //mxd. strcpy -> strcpy_s
		x = M_GetMenuLabelX(re.BF_Strlen(name));
		Menu_DrawString(x, y, name, alpha, selected);
		m_menu_side ^= 1;
	}
	else
	{
		x = M_GetMenuLabelX(action->generic.width);
		strcpy_s(name, sizeof(name), action->generic.name); //mxd. strcpy -> strcpy_s
		Menu_DrawString(x, y, name, alpha, selected);
	}
}

static void InputKey_Draw(const menuinputkey_t* key, const qboolean selected) // H2
{
	char key_label[MAX_QPATH];
	char key_name[MAX_QPATH];
	int keys[2];

	// Draw key label (eg. "Attack")
	Com_sprintf(key_label, sizeof(key_label), "%s", key->generic.name);

	float alpha = M_GetMenuAlpha();
	int x = M_GetMenuLabelX(re.BF_Strlen(key_label));
	const int y = key->generic.y + key->generic.parent->y;
	Menu_DrawString(x, y, key_label, alpha, selected);
	m_menu_side ^= 1;

	// Draw keybind name(s) (eg. "Mouse1")
	M_FindKeysForCommand(key->generic.localdata[0], keys);

	if (keys[0] == -1)
		Com_sprintf(key_name, sizeof(key_name), "???");
	else if (keys[1] == -1)
		Com_sprintf(key_name, sizeof(key_name), "%s", Key_KeynumToString(keys[0]));
	else
		Com_sprintf(key_name, sizeof(key_name), "%s or %s", Key_KeynumToString(keys[0]), Key_KeynumToString(keys[1]));
	
	if (bind_grab && selected)
		alpha = fabsf(cosf((float)Sys_Milliseconds() * 0.005f));

	x = M_GetMenuLabelX(re.BF_Strlen(key_name));
	Menu_DrawString(x, y, key_name, alpha, selected);
	m_menu_side ^= 1;
}

static void PlayerSkin_Draw(const menucommon_t* item) // H2
{
	char skin_path[MAX_QPATH];

	Com_sprintf(skin_path, sizeof(skin_path), "/%s/%s_i.m8", playerdir->string, skin_temp->string);
	re.DrawStretchPic(M_GetMenuLabelX(64), item->y + item->parent->y - 160, 64, 128, skin_path, cls.m_menualpha, true);
}

static void SpinControl_Draw(const menulist_t* list, const qboolean selected)
{
	int x;
	char buffer[MAX_QPATH];

	const float alpha = M_GetMenuAlpha();
	int y = list->generic.y + list->generic.parent->y;

	// Draw as single line?
	if ((list->generic.flags & QMF_SINGLELINE) != 0)
	{
		Com_sprintf(buffer, sizeof(buffer), "%s : %s", list->generic.name, list->itemnames[list->curvalue]);
		x = M_GetMenuLabelX(re.BF_Strlen(buffer));
		Menu_DrawString(x, y, buffer, alpha, selected);

		return;
	}

	// Draw name?
	if (list->generic.name != NULL)
	{
		strcpy_s(buffer, sizeof(buffer), list->generic.name);
		x = M_GetMenuLabelX(list->generic.width);
		Menu_DrawString(x, y, buffer, alpha, selected);
		y += 20;
	}

	strcpy_s(buffer, sizeof(buffer), list->itemnames[list->curvalue]);
	char* newline = strchr(buffer, '\n');

	// Draw value on 2 lines?
	if (newline != NULL)
	{
		*newline = 0;
		x = M_GetMenuLabelX(re.BF_Strlen(buffer));
		Menu_DrawString(x, y, buffer, alpha, selected);

		newline++;
		x = M_GetMenuLabelX(re.BF_Strlen(newline));
		Menu_DrawString(x, y + 20, newline, alpha, selected);
	}
	else // Draw value on single line
	{
		x = M_GetMenuLabelX(re.BF_Strlen(buffer));
		Menu_DrawString(x, y, buffer, alpha, selected);
	}
}

void Menu_Draw(const menuframework_t* menu)
{
	// Draw contents.
	for (int i = 0; i < menu->nitems; i++)
	{
		menucommon_t* item = (menucommon_t*)menu->items[i];

		switch (item->type)
		{
			case MTYPE_SLIDER:
				Slider_Draw((menuslider_t*)item, i == menu->cursor);
				break;

			case MTYPE_FIELD:
				Field_Draw((menufield_t*)item, i == menu->cursor);
				break;

			case MTYPE_ACTION:
				Action_Draw((menuaction_t*)item, i == menu->cursor);
				break;

			case MTYPE_INPUT_KEY:
				InputKey_Draw((menuinputkey_t*)item, i == menu->cursor);
				break;

			case MTYPE_PLAYER_SKIN:
				PlayerSkin_Draw(item);
				SpinControl_Draw((menulist_t*)item, i == menu->cursor);
				break;

			case MTYPE_SPINCONTROL:
				SpinControl_Draw((menulist_t*)item, i == menu->cursor);
				break;

			default: //mxd. Added default case.
				Sys_Error("Unexpected menu item type %i", item->type);
				break;
		}
	}
}

void Menu_DrawString(const int x, const int y, const char* name, const float alpha, const qboolean selected)
{
	if (selected)
	{
		re.DrawBigFont(x, y, name, alpha * 0.2f);
		re.DrawBigFont(x + 1, y - 1, name, alpha * 0.5f);
		re.DrawBigFont(x + 2, y - 2, name, alpha);
	}
	else
	{
		re.DrawBigFont(x, y, name, alpha * 0.6f);
	}
}

static char* FindNewline(char* start, char* end)
{
	if (start < end)
	{
		char* s = start;
		while (s++ < end)
		{
			if (*s == '\n')
				return s;
		}
	}
	else
	{
		char* e = end;
		while (e-- >= start)
		{
			if (strchr("$% \n", *e) != NULL)
				return e;
		}
	}

	return NULL;
}

//mxd. Added 'dst_size' arg.
static int SplitLines(char* dst, const int dst_size, const char* src, const int line_length) // H2
{
	int src_len = (int)strlen(src);
	strcpy_s(dst, dst_size, src);
	int num_lines = 1;

	if (src_len < line_length)
		return num_lines;

	do
	{
		char* cur_end = &dst[min(line_length, src_len)];
		char* newline = FindNewline(dst, cur_end);

		src_len += dst - newline;
		dst = newline;
		*dst = 0;
		num_lines++;
	} while (src_len >= line_length);

	return num_lines;
}

static int GetLineLength(const char* text) // H2
{
	int len = 0;
	while (*text != 0 && *text != '\n')
	{
		text++;
		len++;
	}

	return len;
}

void Menu_DrawObjectives(const char* message, const int max_line_length) // H2
{
	char buffer[1024];

	int color_index = P_OBJ_NORMAL;
	const int num_lines = SplitLines(buffer, sizeof(buffer), message, max_line_length);
	int y = viddef.height * 80 / DEF_HEIGHT;
	char* s = buffer;

	for (int i = 0; i < num_lines; i++, s++, y += 8)
	{
		const int len = GetLineLength(s);
		int x = M_GetMenuLabelX(len * 8 * DEF_WIDTH / viddef.width) * viddef.width / DEF_WIDTH;

		for (int j = 0; j < len; j++, s++, x += 8)
		{
			switch (*s)
			{
				case '$':
					color_index = P_OBJ_BOLD;
					break;

				case '%':
					color_index = P_OBJ_NORMAL;
					break;

				default:
				{
					paletteRGBA_t color = TextPalette[color_index];
					color.a = (byte)Q_ftol(cls.m_menualpha * 255);
					re.DrawChar(x, y, *s, color);
				} break;
			}
		}
	}
}

void Menu_DrawTitle(const cvar_t* title) // H2
{
	char buffer[MAX_QPATH];

	Com_sprintf(buffer, sizeof(buffer), "\x03%s", title->string);
	const int x = M_GetMenuLabelX(re.BF_Strlen(buffer));
	re.DrawBigFont(x, 60, buffer, cls.m_menualpha);
}

// Q2 counterpart
menucommon_t* Menu_ItemAtCursor(const menuframework_t* menu)
{
	if (menu->cursor >= 0 && menu->cursor < menu->nitems)
		return (menucommon_t*)menu->items[menu->cursor];

	return NULL;
}

static float MenuAlpha(const int menutime) // H2
{
	const float alpha = (float)(Sys_Milliseconds() - menutime) / 250.0f;
	return min(alpha, 1.0f);
}

void M_Draw(void)
{
	if (cls.key_dest != key_menu)
		return;

	// Repaint everything next frame
	SCR_DirtyScreen();

	switch (cls.m_menustate)
	{
		case 0: // MENU_START_OPENING?
			m_entersound = false;
			cls.startmenu = Sys_Milliseconds();
			m_menu_side = ((m_menudepth & 1) == 0);
			m_layers[0].draw = m_drawfunc;
			m_keyfunc2 = m_keyfunc;
			cls.m_menustate = 1;
			cls.m_menualpha = 0.0f;
			cls.m_menuscale = 1.0f;
			m_drawfunc();
			break;

		case 1: // MENU_OPENING?
			cls.m_menualpha = MenuAlpha(cls.startmenu);
			m_layers[0].draw();
			if (cls.m_menualpha == 1.0f || quick_menus->value > 1.0f)
				cls.m_menustate = 2;
			break;

		case 2: // MENU_OPENED?
			cls.m_menualpha = 1.0f;
			m_layers[0].draw();
			break;

		case 3:
			cls.startmenu = Sys_Milliseconds();
			cls.m_menustate = 4;
			cls.m_menualpha = 1.0f;
			m_layers[0].draw();
			break;

		case 4:
			cls.m_menualpha = 1.0f - MenuAlpha(cls.startmenu);
			m_layers[0].draw();

			if (cls.m_menualpha == 0.0f || quick_menus->value > 1.0f)
				cls.m_menustate = (m_menudepth > 0 ? 0 : 6);
			break;

		case 5:
			cls.startmenuzoom = Sys_Milliseconds();
			m_layers[0].draw = m_drawfunc;
			m_menu_side = ((m_menudepth & 1) == 0);
			m_keyfunc2 = m_keyfunc;
			cls.m_menuscale = 0.0f;
			cls.m_menualpha = 0.0f;

			if ((int)quick_menus->value || cls.state == ca_disconnected)
				cls.m_menustate = 0;
			else
				cls.m_menustate = 7;
			break;

		case 6: // MENU_START_CLOSING?
			m_entersound = true;
			cls.startmenuzoom = Sys_Milliseconds();
			cls.m_menuscale = 1.0f;
			m_layers[0].draw();

			if ((int)quick_menus->value || cls.state == ca_disconnected)
			{
				cls.m_menustate = 0;
				M_ForceMenuOff();
			}
			else
			{
				cls.m_menustate = 8;
			}
			break;

		case 7:
			cls.m_menuscale = MenuAlpha(cls.startmenuzoom);
			m_layers[0].draw();

			if (cls.m_menuscale == 1.0f)
				cls.m_menustate = 0;
			break;

		case 8: // MENU_CLOSING?
			cls.m_menuscale = 1.0f - MenuAlpha(cls.startmenuzoom);
			m_layers[0].draw();

			if (cls.m_menuscale == 0.0f)
				M_ForceMenuOff();
			break;

		default: //mxd. Added default case.
			Sys_Error("Unexpected menu index %i", cls.m_menustate);
	}
}

// Draws one solid graphics character
// cx and cy are in 320 * 240 coordinates, and will be centered on higher res screens.
static void M_DrawCharacter(const int cx, const int cy, const int num, const paletteRGBA_t color)
{
	re.DrawChar(cx + (viddef.width - 320) / 2, cy + (viddef.height - 240) / 2, num, color);
}

void M_DrawTextBox(const int x, const int y, const int width, const int lines)
{
	static paletteRGBA_t c_white = { .c = 0xffffffff };

	// Draw left side
	int cx = x;
	int cy = y;
	M_DrawCharacter(cx, cy, 18, c_white);

	for (int i = 0; i < lines; i++)
	{
		cy += 8;
		M_DrawCharacter(cx, cy, 21, c_white);
	}

	M_DrawCharacter(cx, cy + 8, 24, c_white);

	// Draw middle
	cx += 8;
	for (int i = 0; i < width; i++)
	{
		cy = y;
		M_DrawCharacter(cx, cy, 19, c_white);

		for (int n = 0; n < lines; n++)
		{
			cy += 8;
			M_DrawCharacter(cx, cy, 22, c_white);
		}

		M_DrawCharacter(cx, cy + 8, 25, c_white);
		cx += 8;
	}

	// Draw right side
	cy = y;
	M_DrawCharacter(cx, cy, 20, c_white);

	for (int i = 0; i < lines; i++)
	{
		cy += 8;
		M_DrawCharacter(cx, cy, 23, c_white);
	}

	M_DrawCharacter(cx, cy + 8, 26, c_white);
}

void M_Print(const int cx, const int cy, const int msg_index, const paletteRGBA_t color)
{
	const char* str = CL_GetGameString(msg_index);
	int x = cx;
	int y = cy;

	while (*str != 0)
	{
		if (*str == '\n')
		{
			x = cx;
			y += 8;
		}
		else
		{
			M_DrawCharacter(x, y, *str, color);
			x += 8;
		}

		str++;
	}
}

void M_Keydown(const int key)
{
	if (m_keyfunc2 != NULL)
	{
		const char* sound_name = m_keyfunc2(key);
		if (sound_name != NULL)
			se.StartLocalSound(sound_name);
	}
}