//
// menu.c -- Menu subsystem
//
// Copyright 1998 Raven Software
//

#include "client.h"
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

cvar_t* vid_menu_mode;
cvar_t* menus_active;
cvar_t* quick_menus;

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
cvar_t* m_origmode;

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

	m_item_tutorial = Cvar_Get("m_item_tutorial", "Tutorial", 0);
	m_item_easy = Cvar_Get("m_item_easy", "Easy", 0);
	m_item_medium = Cvar_Get("m_item_medium", "Medium", 0);
	m_item_hard = Cvar_Get("m_item_hard", "Hard", 0);
	m_item_nightmare = Cvar_Get("m_item_nightmare", "Insane", 0);
	m_item_refresh = Cvar_Get("m_item_refresh", "Refresh Server List", 0);
	m_item_begin = Cvar_Get("m_item_begin", "Start the Slaughter", 0);
	m_item_startmap = Cvar_Get("m_item_startmap", "Initial Map", 0);
	m_item_rules = Cvar_Get("m_item_rules", "Rules", 0);
	m_item_timelimit = Cvar_Get("m_item_timelimit", "Time Limit", 0);
	m_item_fraglimit = Cvar_Get("m_item_fraglimit", "Frag Limit", 0);
	m_item_maxplayers = Cvar_Get("m_item_maxplayers", "Max Players", 0);
	m_item_hostname = Cvar_Get("m_item_hostname", "Hostname", 0);
	m_item_deathmatch = Cvar_Get("m_item_deathmatch", "Deathmatch", 0);
	m_item_coop = Cvar_Get("m_item_coop", "Cooperative", 0);
	m_item_mousespeedx = Cvar_Get("m_item_mousespeedx", "Mouse Horizontal Speed", 0);
	m_item_mousespeedy = Cvar_Get("m_item_mousespeedy", "Mouse Vertical Speed", 0);
	m_item_alwaysrun = Cvar_Get("m_item_alwaysrun", "Always run", 0);
	m_item_mouseinvert = Cvar_Get("m_item_mouseinvert", "Invert Mouse", 0);
	m_item_lookspring = Cvar_Get("m_item_lookspring", "Lookspring", 0);
	m_item_freelook = Cvar_Get("m_item_freelook", "Freelook", 0);
	m_item_crosshair = Cvar_Get("m_item_crosshair", "Crosshair", 0);
	m_item_noalttab = Cvar_Get("m_item_noalttab", "Disable Alt-Tab", 0);
	m_item_joystick = Cvar_Get("m_item_joystick", "Use Joystick", 0);
	m_item_defaults = Cvar_Get("m_item_defaults", "Reset to Defaults", 0);
	m_item_autotarget = Cvar_Get("m_item_autotarget", "Auto Target", 0);
	m_item_caption = Cvar_Get("m_item_caption", "Captioning", 0);
	m_item_violence = Cvar_Get("m_item_violence", "Violence Level", 0);
	m_item_yawspeed = Cvar_Get("m_item_yawspeed", "Key Turn Speed", 0);
	m_item_console = Cvar_Get("m_item_console", "Go to console", 0);
	m_item_driver = Cvar_Get("m_item_driver", "Renderer", 0);
	m_item_vidmode = Cvar_Get("m_item_vidmode", "Video Resolution", 0);
	m_item_screensize = Cvar_Get("m_item_screensize", "Screen Size", 0);
	m_item_gamma = Cvar_Get("m_item_gamma", "Gamma", 0);
	m_item_brightness = Cvar_Get("m_item_brightness", "Brightness", 0);
	m_item_contrast = Cvar_Get("m_item_contrast", "Contrast", 0);
	m_item_detail = Cvar_Get("m_item_detail", "Detail Level", 0);
	m_item_fullscreen = Cvar_Get("m_item_fullscreen", "Fullscreen", 0);
	//m_item_palettedtextures = Cvar_Get("m_item_palettedtextures", "Paletted Textures", 0); //mxd. Disabled
	m_item_snddll = Cvar_Get("m_item_snddll", "Sound System", 0);
	m_item_effectsvol = Cvar_Get("m_item_effectsvol", "Effects Volume", 0);
	m_item_cdmusic = Cvar_Get("m_item_cdmusic", "CD Music", 0);
	m_item_soundquality = Cvar_Get("m_item_soundquality", "Sound Quality", 0);
	m_item_allowdownload = Cvar_Get("m_item_allowdownload", "Allow Downloads", 0);
	m_item_allowdownloadmap = Cvar_Get("m_item_allowdownloadmap", "Download Maps", 0);
	m_item_allowdownloadsound = Cvar_Get("m_item_allowdownloadsound", "Download Sounds", 0);
	m_item_allowdownloadplayer = Cvar_Get("m_item_allowdownloadplayer", "New Player Skins", 0);
	m_item_allowdownloadmodel = Cvar_Get("m_item_allowdownloadmodel", "Download Models", 0);
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
	m_item_name = Cvar_Get("m_item_name", "Name", 0);
	m_item_skin = Cvar_Get("m_item_skin", "Skin", 0);
	m_item_shownames = Cvar_Get("m_item_shownames", "Show Player Names", 0);
	m_item_autoweapon = Cvar_Get("m_item_autoweapon", "Auto Weapon Change", 0);
	m_item_none = Cvar_Get("m_item_none", "No objectives have been written", 0);
	m_item_cameradamp = Cvar_Get("m_item_cameradamp", "Camera Stiffness", 0);
	m_item_cameracombat = Cvar_Get("m_item_cameracombat", "Combat Camera", 0);
	m_item_helpscreen = Cvar_Get("m_item_helpscreen", "Help Screen", 0);
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
	m_item_nomap = Cvar_Get("m_item_nomap", "No Map Available", 0);

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
	m_dmlist = Cvar_Get("m_dmlist", "dm", 0);
	m_cooplist = Cvar_Get("m_cooplist", "coop", 0);
	m_origmode = Cvar_Get("m_origmode", "3", 0);

	Cvar_Get("blood_level", "0", CVAR_ARCHIVE);
	Cvar_Get("dm_no_bodies", "0", CVAR_ARCHIVE);
}