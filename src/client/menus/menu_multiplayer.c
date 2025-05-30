//
// menu_multiplayer.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "menu_multiplayer.h"
#include "menu_downloadoptions.h"
#include "menu_joinserver.h"
#include "menu_playerconfig.h"
#include "menu_startserver.h"

cvar_t* m_banner_multi;

static menuframework_t s_multiplayer_menu;

static menuaction_t s_join_network_server_action;
static menuaction_t s_start_network_server_action;
static menuaction_t s_player_setup_action;
static menuaction_t s_download_setup_action;

static void JoinServerFunc(void* data)
{
	M_Menu_JoinServer_f();
}

static void StartServerFunc(void* data)
{
	M_Menu_StartServer_f();
}

static void PlayerConfigFunc(void* data)
{
	M_Menu_PlayerConfig_f();
}

static void DownloadOptionsFunc(void* data)
{
	M_Menu_DownloadOptions_f();
}

static void Multiplayer_MenuInit(void)
{
	static char name_join[MAX_QPATH];
	static char name_startserver[MAX_QPATH];
	static char name_pconfig[MAX_QPATH];
	static char name_download[MAX_QPATH];

	s_multiplayer_menu.nitems = 0;

	Com_sprintf(name_join, sizeof(name_join), "\x02%s", m_banner_join->string);
	s_join_network_server_action.generic.type = MTYPE_ACTION;
	s_join_network_server_action.generic.flags = QMF_LEFT_JUSTIFY | QMF_SELECT_SOUND;
	s_join_network_server_action.generic.x = 0;
	s_join_network_server_action.generic.y = 0;
	s_join_network_server_action.generic.name = name_join;
	s_join_network_server_action.generic.width = re.BF_Strlen(name_join);
	s_join_network_server_action.generic.callback = JoinServerFunc;

	Com_sprintf(name_startserver, sizeof(name_startserver), "\x02%s", m_banner_startserver->string);
	s_start_network_server_action.generic.type = MTYPE_ACTION;
	s_start_network_server_action.generic.flags = QMF_LEFT_JUSTIFY | QMF_SELECT_SOUND; //BUGFIX: mxd. QMF_SELECT_SOUND was missing in original logic.
	s_start_network_server_action.generic.x = 0;
	s_start_network_server_action.generic.y = 20;
	s_start_network_server_action.generic.name = name_startserver;
	s_start_network_server_action.generic.width = re.BF_Strlen(name_startserver);
	s_start_network_server_action.generic.callback = StartServerFunc;

	Com_sprintf(name_pconfig, sizeof(name_pconfig), "\x02%s", m_banner_pconfig->string);
	s_player_setup_action.generic.type = MTYPE_ACTION;
	s_player_setup_action.generic.flags = QMF_LEFT_JUSTIFY | QMF_SELECT_SOUND;
	s_player_setup_action.generic.x = 0;
	s_player_setup_action.generic.y = 40;
	s_player_setup_action.generic.name = name_pconfig;
	s_player_setup_action.generic.width = re.BF_Strlen(name_pconfig);
	s_player_setup_action.generic.callback = PlayerConfigFunc;

	Com_sprintf(name_download, sizeof(name_download), "\x02%s", m_banner_download->string);
	s_download_setup_action.generic.type = MTYPE_ACTION;
	s_download_setup_action.generic.flags = QMF_LEFT_JUSTIFY | QMF_SELECT_SOUND;
	s_download_setup_action.generic.x = 0;
	s_download_setup_action.generic.y = 60;
	s_download_setup_action.generic.name = name_download;
	s_download_setup_action.generic.width = re.BF_Strlen(name_download);
	s_download_setup_action.generic.callback = DownloadOptionsFunc;

	Menu_AddItem(&s_multiplayer_menu, &s_join_network_server_action);
	Menu_AddItem(&s_multiplayer_menu, &s_start_network_server_action);
	Menu_AddItem(&s_multiplayer_menu, &s_player_setup_action);
	Menu_AddItem(&s_multiplayer_menu, &s_download_setup_action);

	Menu_Center(&s_multiplayer_menu);
}

static void Multiplayer_MenuDraw(void)
{
	char title[MAX_QPATH];

	// Draw menu BG.
	re.BookDrawPic(0, 0, "book/back/b_conback8.bk", cls.m_menuscale);

	if (cls.m_menualpha == 0.0f)
		return;

	// Draw menu title.
	Com_sprintf(title, sizeof(title), "\x03%s", m_banner_multi->string);
	const int x = M_GetMenuLabelX(re.BF_Strlen(title));
	const int y = M_GetMenuOffsetY(&s_multiplayer_menu);
	re.DrawBigFont(x, y, title, cls.m_menualpha);

	// Draw menu items.
	s_multiplayer_menu.x = M_GetMenuLabelX(s_multiplayer_menu.width);
	Menu_AdjustCursor(&s_multiplayer_menu, 1);
	Menu_Draw(&s_multiplayer_menu);
}

// Q2 counterpart
static const char* Multiplayer_MenuKey(const int key)
{
	return Default_MenuKey(&s_multiplayer_menu, key);
}

// Q2 counterpart
void M_Menu_Multiplayer_f(void)
{
	Multiplayer_MenuInit();
	M_PushMenu(Multiplayer_MenuDraw, Multiplayer_MenuKey);
}