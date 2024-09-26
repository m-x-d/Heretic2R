//
// menu_movekeys.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "menu_movekeys.h"
#include "menu_keys.h"

cvar_t* m_banner_move_keys;

static void MoveKeys_MenuDraw(void)
{
	char title[MAX_QPATH];

	// Draw menu BG.
	re.BookDrawPic(0, 0, "book/back/b_conback8.bk", cls.m_menuscale);

	if (cls.m_menualpha == 0.0f)
		return;

	// Draw menu title.
	m_menu_side = 0;
	Com_sprintf(title, sizeof(title), "\x03%s", m_banner_move_keys->string);
	const int x = M_GetMenuLabelX(re.BF_Strlen(title));
	const int y = M_GetMenuOffsetY(&s_keys_menu);
	re.DrawBigFont(x, y, title, cls.m_menualpha);

	// Draw menu items.
	s_keys_menu.x = M_GetMenuLabelX(0);
	Menu_AdjustCursor(&s_keys_menu, 1);
	Menu_Draw(&s_keys_menu);
}

void M_Menu_MoveKeys_f(void)
{
	keys_category_offset = 13;
	keys_count = 12;
	use_doublebind = false;

	Keys_MenuInit();
	M_PushMenu(MoveKeys_MenuDraw, Keys_MenuKey);
}