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

static menuframework_s s_game_menu;

static menuaction_s s_tutorial_action;
static menuaction_s s_easy_game_action;
static menuaction_s s_medium_game_action;
static menuaction_s s_hard_game_action;
static menuaction_s s_load_game_action;
static menuaction_s s_save_game_action;
static menuaction_s s_credits_action;

static void TutorialGameFunc(void* data)
{
	NOT_IMPLEMENTED
}

static void EasyGameFunc(void* data)
{
	NOT_IMPLEMENTED
}

static void MediumGameFunc(void* data)
{
	NOT_IMPLEMENTED
}

static void HardGameFunc(void* data)
{
	NOT_IMPLEMENTED
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

	Com_sprintf(name_load, sizeof(name_load), "\x02%s", m_banner_load->string);
	s_load_game_action.generic.type = MTYPE_ACTION;
	s_load_game_action.generic.flags = QMF_LEFT_JUSTIFY;
	s_load_game_action.generic.x = 0;
	s_load_game_action.generic.y = 120;
	s_load_game_action.generic.name = name_load;
	s_load_game_action.generic.width = re.BF_Strlen(name_load);
	s_load_game_action.generic.callback = LoadGameFunc;

	Com_sprintf(name_save, sizeof(name_save), "\x02%s", m_banner_save->string);
	s_save_game_action.generic.type = MTYPE_ACTION;
	s_save_game_action.generic.flags = QMF_LEFT_JUSTIFY;
	s_save_game_action.generic.x = 0;
	s_save_game_action.generic.y = 140;
	s_save_game_action.generic.name = name_save;
	s_save_game_action.generic.width = re.BF_Strlen(name_save);
	s_save_game_action.generic.callback = SaveGameFunc;

	Com_sprintf(name_credits, sizeof(name_credits), "\x02%s", m_banner_credits->string);
	s_credits_action.generic.type = MTYPE_ACTION;
	s_credits_action.generic.flags = QMF_LEFT_JUSTIFY;
	s_credits_action.generic.x = 0;
	s_credits_action.generic.y = 180;
	s_credits_action.generic.name = name_credits;
	s_credits_action.generic.width = re.BF_Strlen(name_credits);
	s_credits_action.generic.callback = CreditsGameFunc;

	Menu_AddItem(&s_game_menu, &s_tutorial_action.generic);
	Menu_AddItem(&s_game_menu, &s_easy_game_action.generic);
	Menu_AddItem(&s_game_menu, &s_medium_game_action.generic);
	Menu_AddItem(&s_game_menu, &s_hard_game_action.generic);
	Menu_AddItem(&s_game_menu, &s_load_game_action.generic);
	Menu_AddItem(&s_game_menu, &s_save_game_action.generic);
	Menu_AddItem(&s_game_menu, &s_credits_action.generic);

	Menu_Center(&s_game_menu);
}

static void Game_MenuDraw(void)
{
	NOT_IMPLEMENTED
}

static const char* Game_MenuKey(int key)
{
	NOT_IMPLEMENTED
	return NULL;
}

// Q2 counterpart
void M_Menu_Game_f(void)
{
	Game_MenuInit();
	M_PushMenu(Game_MenuDraw, Game_MenuKey);
	//m_game_cursor = 1; //mxd. Unused
}
