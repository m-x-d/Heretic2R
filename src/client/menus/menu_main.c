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
	Menu_DrawBG("book/back/b_conback8.bk", cls.m_menuscale);

	if (cls.m_menualpha == 0.0f)
		return;

	// Draw menu title.
	Com_sprintf(name, sizeof(name), "\x03%s", m_banner_main->string);
	re.DrawBigFont(M_GetMenuLabelX(re.BF_Strlen(name)), 96, name, cls.m_menualpha);

	// Draw menu items.
	int oy = 128;
	for (int i = 0; i < NUM_MENU_ITEMS; i++, oy += 32)
	{
		Com_sprintf(name, sizeof(name), "\x02%s", (*menu_labels[i])->string);
		Menu_DrawString(M_GetMenuLabelX(re.BF_Strlen(name)), oy, name, cls.m_menualpha, i == m_main_cursor);
	}
}

static const char* M_Main_Key(const int key)
{
#define MAIN_ITEMS	6

	if (cls.m_menustate != MS_OPENED)
		return NULL;

	switch (key)
	{
		case K_ENTER:
		case K_KP_ENTER:
			switch (m_main_cursor)
			{
				case 0:
					M_Menu_Game_f();
					break;

				case 1:
					M_Menu_Multiplayer_f();
					break;

				case 2:
					M_Menu_Options_f();
					break;

				case 3:
					M_Menu_Video_f();
					break;

				case 4:
					M_Menu_Sound_f();
					break;

				case 5:
					M_Menu_Info_f();
					break;

				case 6:
					M_Menu_Quit_f();
					break;

				default: //mxd. Added default case.
					Sys_Error("Unexpected main menu index %i", m_main_cursor);
					break;
			}
			return SND_MENU1;

		case K_ESCAPE:
			M_PopMenu();
			return SND_MENU3;

		case K_UPARROW:
		case K_KP_UPARROW:
			if (--m_main_cursor < 0)
				m_main_cursor = MAIN_ITEMS;
			return SND_MENU2;

		case K_DOWNARROW:
		case K_KP_DOWNARROW:
			if (++m_main_cursor > MAIN_ITEMS)
				m_main_cursor = 0;
			return SND_MENU2;

		default:
			return NULL;
	}
}

void M_Menu_Main_f(void)
{
	if (cl.frame.playerstate.cinematicfreeze)
		cls.esc_cinematic = 1;
	else
		M_PushMenu(M_Main_Draw, M_Main_Key);
}