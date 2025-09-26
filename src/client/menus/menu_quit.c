//
// menu_quit.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "menu_quit.h"

cvar_t* m_banner_quit;

static menuframework_t s_quit_menu;

static menuaction_t s_quit_yes_action;
static menuaction_t s_quit_no_action;

H2R_NORETURN static void QuitFunc(void* self) // H2
{
	cls.key_dest = key_console;
	CL_Quit_f();
}

static void CancelFunc(void* self) // H2
{
	M_PopMenu();
}

static void Quit_MenuInit(void) // H2
{
	static char name_yes[MAX_QPATH];
	static char name_no[MAX_QPATH];

	s_quit_menu.nitems = 0;

	Com_sprintf(name_yes, sizeof(name_yes), "\x02%s", m_generic_yes->string);
	s_quit_yes_action.generic.type = MTYPE_ACTION;
	s_quit_yes_action.generic.x = 0;
	s_quit_yes_action.generic.y = 0;
	s_quit_yes_action.generic.name = name_yes;
	s_quit_yes_action.generic.width = re.BF_Strlen(name_yes);
	s_quit_yes_action.generic.callback = QuitFunc;

	Com_sprintf(name_no, sizeof(name_no), "\x02%s", m_generic_no->string);
	s_quit_no_action.generic.type = MTYPE_ACTION;
	s_quit_no_action.generic.x = 0;
	s_quit_no_action.generic.y = 20;
	s_quit_no_action.generic.name = name_no;
	s_quit_no_action.generic.width = re.BF_Strlen(name_no);
	s_quit_no_action.generic.callback = CancelFunc;

	Menu_AddItem(&s_quit_menu, &s_quit_yes_action.generic);
	Menu_AddItem(&s_quit_menu, &s_quit_no_action.generic);
	Menu_Center(&s_quit_menu);
}

static void Quit_MenuDraw(void)
{
	char title[MAX_QPATH];

	// Draw menu BG.
	Menu_DrawBG("book/back/b_conback8.bk", cls.m_menuscale);

	if (cls.m_menualpha == 0.0f)
		return;

	// Draw menu title.
	Com_sprintf(title, sizeof(title), "\x03%s", m_banner_quit->string);
	const int x = M_GetMenuLabelX(re.BF_Strlen(title));
	const int y = M_GetMenuOffsetY(&s_quit_menu);
	re.DrawBigFont(x, y, title, cls.m_menualpha);

	// Draw menu items.
	s_quit_menu.x = M_GetMenuLabelX(s_quit_menu.width);
	Menu_Draw(&s_quit_menu);
}

static const char* Quit_MenuKey(const int key)
{
	if (cls.m_menustate != MS_OPENED)
		return NULL;

	switch (key)
	{
		case K_ENTER:
		case K_KP_ENTER:
			if (Menu_SelectItem(&s_quit_menu))
			{
				const menulist_t* item = (menulist_t*)Menu_ItemAtCursor(&s_quit_menu);
				if (item->generic.flags & QMF_SELECT_SOUND)
					return SND_MENU_ENTER;
			}
			return NULL;

		case K_ESCAPE:
			M_PopMenu();
			return SND_MENU_CLOSE;

		case 'N':
		case 'n':
			M_PopMenu();
			return SND_MENU_CLOSE; //mxd. Add sound.

		case 'Y':
		case 'y':
			QuitFunc(NULL);
			return NULL;

		case K_UPARROW:
		case K_KP_UPARROW:
			s_quit_menu.cursor--;
			Menu_AdjustCursor(&s_quit_menu, -1);
			return SND_MENU_SELECT;

		case K_DOWNARROW:
		case K_KP_DOWNARROW:
		case K_TAB:
			s_quit_menu.cursor++;
			Menu_AdjustCursor(&s_quit_menu, 1);
			return SND_MENU_SELECT;

		default:
			return NULL;
	}
}

void M_Menu_Quit_f(void)
{
	Quit_MenuInit(); // H2
	M_PushMenu(Quit_MenuDraw, Quit_MenuKey);
}