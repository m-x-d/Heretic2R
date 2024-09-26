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
	char name[MAX_QPATH];

	// Draw menu BG.
	re.BookDrawPic(0, 0, "book/back/b_conback8.bk", cls.m_menuscale);

	if (cls.m_menualpha == 0.0f)
		return;

	// Draw menu title.
	m_menu_side = 0;
	Com_sprintf(name, sizeof(name), "\x03%s", m_banner_action_keys->string);
	const int x = M_GetMenuLabelX(re.BF_Strlen(name));
	const int y = M_GetMenuOffsetY(&s_keys_menu);
	re.DrawBigFont(x, y, name, cls.m_menualpha);

	// Draw menu items.
	s_keys_menu.x = M_GetMenuLabelX(0);
	Menu_AdjustCursor(&s_keys_menu, 1);
	Menu_Draw(&s_keys_menu);
}

void M_Menu_ActionKeys_f(void) // H2
{
	keys_count = 13;
	keys_category_offset = 0;
	use_doublebind = false;

	Keys_MenuInit();
	M_PushMenu(ActionKeys_MenuDraw, Keys_MenuKey);
}