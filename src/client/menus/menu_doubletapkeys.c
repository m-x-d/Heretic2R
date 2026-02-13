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
	Keys_MenuDraw(m_banner_dt_keys->string);
}

void M_Menu_DoubletapKeys_f(void)
{
	keys_category_offset = 40;
	keys_count = 10;
	use_doublebind = true;

	Keys_MenuInit();
	M_PushMenu(DoubletapKeys_MenuDraw, Keys_MenuKey);
}