//
// menu_actionkeys.c
//
// Copyright 1998 Raven Software
//

#include "menu_actionkeys.h"
#include "menu_keys.h"

cvar_t* m_banner_action_keys;

static void ActionKeys_MenuDraw(void)
{
	NOT_IMPLEMENTED
}

void M_Menu_ActionKeys_f(void) // H2
{
	keys_count = 13;
	keys_category_offset = 0;
	use_doublebind = false;

	Keys_MenuInit();
	M_PushMenu(ActionKeys_MenuDraw, Keys_MenuKey);
}