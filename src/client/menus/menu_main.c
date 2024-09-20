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
static int m_main_cursor;

static void M_Main_Draw(void)
{
#define NUM_MENU_ITEMS  7

	static cvar_t** menu_labels[NUM_MENU_ITEMS] =
	{
		&m_banner_game,
		&m_banner_multi,
		&m_banner_options,
		&m_banner_video,
		&m_banner_sound,
		&m_banner_info,
		&m_banner_quit
	};

	char name[MAX_QPATH];

	// Draw menu BG.
	re.BookDrawPic(0, 0, "book/back/b_conback8.bk", cls.m_menuscale);

	if (cls.m_menualpha == 0.0f)
		return;

	// Draw menu title.
	Com_sprintf(name, sizeof(name), "\x03%s", m_banner_main->string);
	re.DrawBigFont(M_GetMenuOffsetX(re.BF_Strlen(name)), 96, name, cls.m_menualpha);

	// Draw menu items.
	int oy = 128;
	for (int i = 0; i < NUM_MENU_ITEMS; i++, oy += 32)
	{
		Com_sprintf(name, sizeof(name), "\x02%s", (*menu_labels[i])->string);
		Menu_DrawString(M_GetMenuOffsetX(re.BF_Strlen(name)), oy, name, cls.m_menualpha, i == m_main_cursor);
	}
}

static const char* M_Main_Key(int key)
{
	NOT_IMPLEMENTED
	return NULL;
}

void M_Menu_Main_f(void)
{
	if (cl.frame.playerstate.cinematicfreeze)
		cls.esc_cinematic = 1;
	else
		M_PushMenu(M_Main_Draw, M_Main_Key);
}