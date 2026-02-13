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
	Keys_MenuDraw(m_banner_move_keys->string);
}

void M_Menu_MoveKeys_f(void)
{
	keys_category_offset = 14;
	keys_count = 12;
	use_doublebind = false;

	Keys_MenuInit();
	M_PushMenu(MoveKeys_MenuDraw, Keys_MenuKey);
}