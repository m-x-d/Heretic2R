//
// menu_misc.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "menu_misc.h"

cvar_t* m_banner_misc;

cvar_t* m_item_alwaysrun;
cvar_t* m_item_crosshair;
cvar_t* m_item_noalttab;
cvar_t* m_item_joystick;
cvar_t* m_item_autotarget;
cvar_t* m_item_caption;
cvar_t* m_item_violence;
cvar_t* m_item_yawspeed;
cvar_t* m_item_console;
cvar_t* m_item_autoweapon;

static menuframework_s s_misc_menu;

static void Misc_MenuInit(void) // H2
{
	NOT_IMPLEMENTED
}

static void Misc_MenuDraw(void) // H2
{
	char title[MAX_QPATH];

	// Draw menu BG.
	re.BookDrawPic(0, 0, "book/back/b_conback8.bk", cls.m_menuscale);

	if (cls.m_menualpha == 0.0f)
		return;

	// Draw menu title.
	Com_sprintf(title, sizeof(title), "\x03%s", m_banner_misc->string);
	const int x = M_GetMenuLabelX(re.BF_Strlen(title));
	const int y = M_GetMenuOffsetY(&s_misc_menu);
	re.DrawBigFont(x, y, title, cls.m_menualpha);

	// Draw menu items.
	s_misc_menu.x = M_GetMenuLabelX(s_misc_menu.width);
	Menu_AdjustCursor(&s_misc_menu, 1);
	Menu_Draw(&s_misc_menu);
}

static const char* Misc_MenuKey(const int key) // H2
{
	return Default_MenuKey(&s_misc_menu, key);
}

void M_Menu_Misc_f(void) // H2
{
	Misc_MenuInit();
	M_PushMenu(Misc_MenuDraw, Misc_MenuKey);
}