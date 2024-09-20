//
// menu_game.c
//
// Copyright 1998 Raven Software
//

#include "menu_game.h"

cvar_t* m_banner_game;

cvar_t* m_item_tutorial;
cvar_t* m_item_easy;
cvar_t* m_item_medium;
cvar_t* m_item_hard;
cvar_t* m_item_nightmare;

static void Game_MenuInit(void)
{
	NOT_IMPLEMENTED
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
