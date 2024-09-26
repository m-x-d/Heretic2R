//
// menu_doubletapkeys.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "menu_doubletapkeys.h"
#include "menu_keys.h"

cvar_t* m_banner_dt_keys;

void DoubletapKeys_MenuDraw(void)
{
	char name[MAX_QPATH];

	// Draw menu BG.
	re.BookDrawPic(0, 0, "book/back/b_conback8.bk", cls.m_menuscale);

	if (cls.m_menualpha == 0.0f)
		return;

	// Draw menu title.
	m_menu_side = 0;
	Com_sprintf(name, sizeof(name), "\x03%s", m_banner_dt_keys->string);
	const int x = M_GetMenuLabelX(re.BF_Strlen(name));
	const int y = M_GetMenuOffsetY(&s_keys_menu);
	re.DrawBigFont(x, y, name, cls.m_menualpha);

	// Draw menu items.
	s_keys_menu.x = M_GetMenuLabelX(0);
	Menu_AdjustCursor(&s_keys_menu, 1);
	Menu_Draw(&s_keys_menu);
}

void M_Menu_DoubletapKeys_f(void)
{
	keys_category_offset = 39;
	keys_count = 10;
	use_doublebind = true;

	Keys_MenuInit();
	M_PushMenu(DoubletapKeys_MenuDraw, Keys_MenuKey);
}