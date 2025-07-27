//
// menu_game.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "menu_game.h"
#include "menu_credits.h"
#include "menu_loadgame.h"
#include "menu_savegame.h"

cvar_t* m_banner_game;

cvar_t* m_item_tutorial;
cvar_t* m_item_easy;
cvar_t* m_item_medium;
cvar_t* m_item_hard;
cvar_t* m_item_nightmare;

static menuframework_t s_game_menu;

static menuaction_t s_tutorial_action;
static menuaction_t s_easy_game_action;
static menuaction_t s_medium_game_action;
static menuaction_t s_hard_game_action;
static menuaction_t s_nightmare_game_action; //mxd
static menuaction_t s_load_game_action;
static menuaction_t s_save_game_action;
static menuaction_t s_credits_action;

static void StartGame(char* mapname)
{
	// Disable updates and start the cinematic going.
	cl.servercount = -1;
	M_ForceMenuOff();

	Cvar_SetValue("deathmatch", 0);
	Cvar_SetValue("coop", 0);

	Cbuf_AddText(va("loading ; killserver ; wait ; deathmatch 0 ; maxclients 1 ; %s\n", mapname));
	cls.key_dest = key_game;
}

static void TutorialGameFunc(void* data)
{
	Cvar_ForceSet("skill", "0");
	StartGame("tutorial");
}

static void EasyGameFunc(void* data)
{
	Cvar_ForceSet("skill", "0");
	StartGame("newgame");
}

static void MediumGameFunc(void* data)
{
	Cvar_ForceSet("skill", "1");
	StartGame("newgame");
}

static void HardGameFunc(void* data)
{
	Cvar_ForceSet("skill", "2"); //H2: 3
	StartGame("newgame");
}

static void NightmareGameFunc(void* data) //mxd
{
	Cvar_ForceSet("skill", "3");
	StartGame("newgame");
}

static void LoadGameFunc(void* data)
{
	M_Menu_Loadgame_f();
}

static void SaveGameFunc(void* data)
{
	M_Menu_Savegame_f();
}

static void CreditsGameFunc(void* data)
{
	M_Menu_Credits_f();
}

static void Game_MenuInit(void)
{
	static char name_tutorial[MAX_QPATH];
	static char name_easy[MAX_QPATH];
	static char name_medium[MAX_QPATH];
	static char name_hard[MAX_QPATH];
	static char name_nightmare[MAX_QPATH]; //mxd
	static char name_load[MAX_QPATH];
	static char name_save[MAX_QPATH];
	static char name_credits[MAX_QPATH];

	s_game_menu.nitems = 0;

	Com_sprintf(name_tutorial, sizeof(name_tutorial), "\x02%s", m_item_tutorial->string);
	s_tutorial_action.generic.type = MTYPE_ACTION;
	s_tutorial_action.generic.flags = QMF_LEFT_JUSTIFY;
	s_tutorial_action.generic.x = 0;
	s_tutorial_action.generic.y = 0;
	s_tutorial_action.generic.name = name_tutorial;
	s_tutorial_action.generic.width = re.BF_Strlen(name_tutorial);
	s_tutorial_action.generic.callback = TutorialGameFunc;

	Com_sprintf(name_easy, sizeof(name_easy), "\x02%s", m_item_easy->string);
	s_easy_game_action.generic.type = MTYPE_ACTION;
	s_easy_game_action.generic.flags = QMF_LEFT_JUSTIFY;
	s_easy_game_action.generic.x = 0;
	s_easy_game_action.generic.y = 40;
	s_easy_game_action.generic.name = name_easy;
	s_easy_game_action.generic.width = re.BF_Strlen(name_easy);
	s_easy_game_action.generic.callback = EasyGameFunc;

	Com_sprintf(name_medium, sizeof(name_medium), "\x02%s", m_item_medium->string);
	s_medium_game_action.generic.type = MTYPE_ACTION;
	s_medium_game_action.generic.flags = QMF_LEFT_JUSTIFY;
	s_medium_game_action.generic.x = 0;
	s_medium_game_action.generic.y = 60;
	s_medium_game_action.generic.name = name_medium;
	s_medium_game_action.generic.width = re.BF_Strlen(name_medium);
	s_medium_game_action.generic.callback = MediumGameFunc;

	Com_sprintf(name_hard, sizeof(name_hard), "\x02%s", m_item_hard->string);
	s_hard_game_action.generic.type = MTYPE_ACTION;
	s_hard_game_action.generic.flags = QMF_LEFT_JUSTIFY;
	s_hard_game_action.generic.x = 0;
	s_hard_game_action.generic.y = 80;
	s_hard_game_action.generic.name = name_hard;
	s_hard_game_action.generic.width = re.BF_Strlen(name_hard);
	s_hard_game_action.generic.callback = HardGameFunc;

	Com_sprintf(name_nightmare, sizeof(name_nightmare), "\x02%s", m_item_nightmare->string); //mxd
	s_nightmare_game_action.generic.type = MTYPE_ACTION;
	s_nightmare_game_action.generic.flags = QMF_LEFT_JUSTIFY;
	s_nightmare_game_action.generic.x = 0;
	s_nightmare_game_action.generic.y = 100;
	s_nightmare_game_action.generic.name = name_nightmare;
	s_nightmare_game_action.generic.width = re.BF_Strlen(name_nightmare);
	s_nightmare_game_action.generic.callback = NightmareGameFunc;

	Com_sprintf(name_load, sizeof(name_load), "\x02%s", m_banner_load->string);
	s_load_game_action.generic.type = MTYPE_ACTION;
	s_load_game_action.generic.flags = QMF_LEFT_JUSTIFY | QMF_SELECT_SOUND; //mxd. Added QMF_SELECT_SOUND flag.
	s_load_game_action.generic.x = 0;
	s_load_game_action.generic.y = 140;
	s_load_game_action.generic.name = name_load;
	s_load_game_action.generic.width = re.BF_Strlen(name_load);
	s_load_game_action.generic.callback = LoadGameFunc;

	Com_sprintf(name_save, sizeof(name_save), "\x02%s", m_banner_save->string);
	s_save_game_action.generic.type = MTYPE_ACTION;
	s_save_game_action.generic.flags = QMF_LEFT_JUSTIFY | QMF_SELECT_SOUND; //mxd. Added QMF_SELECT_SOUND flag.
	s_save_game_action.generic.x = 0;
	s_save_game_action.generic.y = 160;
	s_save_game_action.generic.name = name_save;
	s_save_game_action.generic.width = re.BF_Strlen(name_save);
	s_save_game_action.generic.callback = SaveGameFunc;

	Com_sprintf(name_credits, sizeof(name_credits), "\x02%s", m_banner_credits->string);
	s_credits_action.generic.type = MTYPE_ACTION;
	s_credits_action.generic.flags = QMF_LEFT_JUSTIFY;
	s_credits_action.generic.x = 0;
	s_credits_action.generic.y = 200;
	s_credits_action.generic.name = name_credits;
	s_credits_action.generic.width = re.BF_Strlen(name_credits);
	s_credits_action.generic.callback = CreditsGameFunc;

	Menu_AddItem(&s_game_menu, &s_tutorial_action);
	Menu_AddItem(&s_game_menu, &s_easy_game_action);
	Menu_AddItem(&s_game_menu, &s_medium_game_action);
	Menu_AddItem(&s_game_menu, &s_hard_game_action);
	Menu_AddItem(&s_game_menu, &s_nightmare_game_action); //mxd
	Menu_AddItem(&s_game_menu, &s_load_game_action);
	Menu_AddItem(&s_game_menu, &s_save_game_action);
	Menu_AddItem(&s_game_menu, &s_credits_action);

	Menu_Center(&s_game_menu);
}

static void Game_MenuDraw(void)
{
	char title[MAX_QPATH];

	// Draw menu BG.
	re.BookDrawPic("book/back/b_conback8.bk", cls.m_menuscale);

	if (cls.m_menualpha == 0.0f)
		return;

	// Draw menu title.
	Com_sprintf(title, sizeof(title), "\x03%s", m_banner_game->string);
	const int x = M_GetMenuLabelX(re.BF_Strlen(title));
	const int y = M_GetMenuOffsetY(&s_game_menu);
	re.DrawBigFont(x, y, title, cls.m_menualpha);

	// Draw menu items.
	s_game_menu.x = M_GetMenuLabelX(s_game_menu.width);
	Menu_AdjustCursor(&s_game_menu, 1);
	Menu_Draw(&s_game_menu);
}

// Q2 counterpart
static const char* Game_MenuKey(const int key)
{
	return Default_MenuKey(&s_game_menu, key);
}

// Q2 counterpart
void M_Menu_Game_f(void)
{
	Game_MenuInit();
	M_PushMenu(Game_MenuDraw, Game_MenuKey);
}