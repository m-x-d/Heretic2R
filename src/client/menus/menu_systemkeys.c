//
// menu_systemkeys.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "menu_systemkeys.h"
#include "menu_keys.h"

cvar_t* m_banner_system_keys;

static void SystemKeys_MenuDraw(void)
{
	Keys_MenuDraw(m_banner_system_keys->string);
}

void M_Menu_SystemKeys_f(void)
{
	keys_category_offset = 50;
	keys_count = 9;
	use_doublebind = false;

	Keys_MenuInit();
	M_PushMenu(SystemKeys_MenuDraw, Keys_MenuKey);
}