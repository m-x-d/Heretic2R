//
// menu_shortkeys.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "menu_shortkeys.h"
#include "menu_keys.h"

cvar_t* m_banner_short_keys;

static void ShortKeys_MenuDraw(void)
{
	Keys_MenuDraw(m_banner_short_keys->string);
}

void M_Menu_ShortKeys_f(void)
{
	keys_category_offset = 26;
	keys_count = 14;
	use_doublebind = false;

	Keys_MenuInit();
	M_PushMenu(ShortKeys_MenuDraw, Keys_MenuKey);
}