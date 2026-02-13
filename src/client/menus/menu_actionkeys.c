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
	Keys_MenuDraw(m_banner_action_keys->string);
}

void M_Menu_ActionKeys_f(void) // H2
{
	keys_count = 14;
	keys_category_offset = 0;
	use_doublebind = false;

	Keys_MenuInit();
	M_PushMenu(ActionKeys_MenuDraw, Keys_MenuKey);
}