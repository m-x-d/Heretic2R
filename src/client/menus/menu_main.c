//
// menu_main.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "menu_main.h"
#include "menu_game.h"
#include "menu_info.h"
#include "menu_multiplayer.h"
#include "menu_options.h"
#include "menu_quit.h"
#include "menu_sound.h"
#include "menu_video.h"

cvar_t* m_banner_main;

static menuframework_t s_main_menu;

static menuaction_t s_game_action;
static menuaction_t s_multiplayer_action;
static menuaction_t s_options_action;
static menuaction_t s_video_action;
static menuaction_t s_sound_action;
static menuaction_t s_info_action;
static menuaction_t s_quit_action;

static void MainGameFunc(void* data)		{ M_Menu_Game_f(); }
static void MainMultiplayerFunc(void* data)	{ M_Menu_Multiplayer_f(); }
static void MainOptionsFunc(void* data)		{ M_Menu_Options_f(); }
static void MainVideoFunc(void* data)		{ M_Menu_Video_f(); }
static void MainSoundFunc(void* data)		{ M_Menu_Sound_f(); }
static void MainInfoFunc(void* data)		{ M_Menu_Info_f(); }
static void MainQuitFunc(void* data)		{ M_Menu_Quit_f(); }

static void Main_MenuInit(void)
{
	static char name_game[MAX_QPATH];
	static char name_multiplayer[MAX_QPATH];
	static char name_options[MAX_QPATH];
	static char name_video[MAX_QPATH];
	static char name_sound[MAX_QPATH];
	static char name_info[MAX_QPATH];
	static char name_quit[MAX_QPATH];

	s_main_menu.nitems = 0;

	Com_sprintf(name_game, sizeof(name_game), "\x02%s", m_banner_game->string);
	s_game_action.generic.type = MTYPE_ACTION;
	s_game_action.generic.flags = (QMF_LEFT_JUSTIFY | QMF_SELECT_SOUND);
	s_game_action.generic.x = 0;
	s_game_action.generic.y = 0;
	s_game_action.generic.name = name_game;
	s_game_action.generic.width = re.BF_Strlen(name_game);
	s_game_action.generic.callback = MainGameFunc;

	Com_sprintf(name_multiplayer, sizeof(name_multiplayer), "\x02%s", m_banner_multi->string);
	s_multiplayer_action.generic.type = MTYPE_ACTION;
	s_multiplayer_action.generic.flags = (QMF_LEFT_JUSTIFY | QMF_SELECT_SOUND);
	s_multiplayer_action.generic.x = 0;
	s_multiplayer_action.generic.y = 32;
	s_multiplayer_action.generic.name = name_multiplayer;
	s_multiplayer_action.generic.width = re.BF_Strlen(name_multiplayer);
	s_multiplayer_action.generic.callback = MainMultiplayerFunc;

	Com_sprintf(name_options, sizeof(name_options), "\x02%s", m_banner_options->string);
	s_options_action.generic.type = MTYPE_ACTION;
	s_options_action.generic.flags = (QMF_LEFT_JUSTIFY | QMF_SELECT_SOUND);
	s_options_action.generic.x = 0;
	s_options_action.generic.y = 64;
	s_options_action.generic.name = name_options;
	s_options_action.generic.width = re.BF_Strlen(name_options);
	s_options_action.generic.callback = MainOptionsFunc;

	Com_sprintf(name_video, sizeof(name_video), "\x02%s", m_banner_video->string);
	s_video_action.generic.type = MTYPE_ACTION;
	s_video_action.generic.flags = (QMF_LEFT_JUSTIFY | QMF_SELECT_SOUND);
	s_video_action.generic.x = 0;
	s_video_action.generic.y = 96;
	s_video_action.generic.name = name_video;
	s_video_action.generic.width = re.BF_Strlen(name_video);
	s_video_action.generic.callback = MainVideoFunc;

	Com_sprintf(name_sound, sizeof(name_sound), "\x02%s", m_banner_sound->string);
	s_sound_action.generic.type = MTYPE_ACTION;
	s_sound_action.generic.flags = (QMF_LEFT_JUSTIFY | QMF_SELECT_SOUND);
	s_sound_action.generic.x = 0;
	s_sound_action.generic.y = 128;
	s_sound_action.generic.name = name_sound;
	s_sound_action.generic.width = re.BF_Strlen(name_sound);
	s_sound_action.generic.callback = MainSoundFunc;

	Com_sprintf(name_info, sizeof(name_info), "\x02%s", m_banner_info->string);
	s_info_action.generic.type = MTYPE_ACTION;
	s_info_action.generic.flags = (QMF_LEFT_JUSTIFY | QMF_SELECT_SOUND);
	s_info_action.generic.x = 0;
	s_info_action.generic.y = 160;
	s_info_action.generic.name = name_info;
	s_info_action.generic.width = re.BF_Strlen(name_info);
	s_info_action.generic.callback = MainInfoFunc;

	// Disable when not in-game.
	if (!Com_ServerState())
		s_info_action.generic.flags |= QMF_GRAYED;

	Com_sprintf(name_quit, sizeof(name_quit), "\x02%s", m_banner_quit->string);
	s_quit_action.generic.type = MTYPE_ACTION;
	s_quit_action.generic.flags = (QMF_LEFT_JUSTIFY | QMF_SELECT_SOUND);
	s_quit_action.generic.x = 0;
	s_quit_action.generic.y = 192;
	s_quit_action.generic.name = name_quit;
	s_quit_action.generic.width = re.BF_Strlen(name_quit);
	s_quit_action.generic.callback = MainQuitFunc;

	Menu_AddItem(&s_main_menu, &s_game_action);
	Menu_AddItem(&s_main_menu, &s_multiplayer_action);
	Menu_AddItem(&s_main_menu, &s_options_action);
	Menu_AddItem(&s_main_menu, &s_video_action);
	Menu_AddItem(&s_main_menu, &s_sound_action);
	Menu_AddItem(&s_main_menu, &s_info_action);
	Menu_AddItem(&s_main_menu, &s_quit_action);

	Menu_Center(&s_main_menu);
	s_main_menu.y = 128; // Match vanilla offset.
}

static void M_Main_Draw(void)
{
	// Draw menu BG.
	Menu_DrawBG("book/back/b_conback8.bk", cls.m_menuscale);

	if (cls.m_menualpha == 0.0f)
		return;

	// Draw menu title.
	char title[MAX_QPATH];
	Com_sprintf(title, sizeof(title), "\x03%s", m_banner_main->string);
	re.DrawBigFont(M_GetMenuLabelX(re.BF_Strlen(title)), 96, title, cls.m_menualpha);

	// Draw menu items.
	s_main_menu.x = M_GetMenuLabelX(s_main_menu.width);
	Menu_AdjustCursor(&s_main_menu, 1);
	Menu_Draw(&s_main_menu);
}

static const char* M_Main_Key(const int key)
{
	return Default_MenuKey(&s_main_menu, key);
}

void M_Menu_Main_f(void)
{
	if (cl.frame.playerstate.cinematicfreeze)
	{
		cls.esc_cinematic = true;
		return;
	}

	Main_MenuInit();
	M_PushMenu(M_Main_Draw, M_Main_Key);
}