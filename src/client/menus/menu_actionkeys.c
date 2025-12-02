//
// menu_actionkeys.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "menu_actionkeys.h"
#include "menu_keys.h"

cvar_t* m_banner_action_keys;

static void ActionKeys_MenuDraw(void) // H2
{
	char title[MAX_QPATH];

	// Draw menu BG.
	Menu_DrawBG("book/back/b_conback8.bk", cls.m_menuscale);

	if (cls.m_menualpha == 0.0f)
		return;

	// Draw menu title.
	m_menu_side = 0;
	Com_sprintf(title, sizeof(title), "\x03%s", m_banner_action_keys->string);
	const int x = M_GetMenuLabelX(re.BF_Strlen(title));
	const int y = M_GetMenuOffsetY(&s_keys_menu);
	re.DrawBigFont(x, y, title, cls.m_menualpha);

	// Draw menu items.
	s_keys_menu.x = M_GetMenuLabelX(0);
	Menu_AdjustCursor(&s_keys_menu, 1);
	Menu_Draw(&s_keys_menu);
}

void M_Menu_ActionKeys_f(void) // H2
{
	keys_count = 14;
	keys_category_offset = 0;
	use_doublebind = false;

	Keys_MenuInit();
	M_PushMenu(ActionKeys_MenuDraw, Keys_MenuKey);
}