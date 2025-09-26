//
// menu_help.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "cl_messages.h"
#include "cl_strings.h"
#include "vid.h"
#include "menu_help.h"

cvar_t* m_item_helpscreen;

//TODO: is drawn incorrectly when m_menu_side == 1 (eg. when invoked from Main menu). 
static void Help_MenuDraw(void) // H2
{
	// Draw menu BG.
	Menu_DrawBG("book/back/b_conback8.bk", cls.m_menuscale);

	if (cls.m_menualpha == 0.0f)
		return;

	const int line_length = viddef.width * 18 / DEF_WIDTH + 8;
	const uint menu_side = m_menu_side; //mxd

	// Draw left page.
	m_menu_side = 0; //mxd. Always draw on left side.

	const char* help_msg = CL_GetGameString(GM_HELP1);
	const char* help_snd = CL_GetGameWav(GM_HELP1);

	if (help_msg != NULL)
	{
		Menu_DrawTitle(m_item_helpscreen);
		Menu_DrawObjectives(help_msg, line_length);
	}

	if (help_snd != NULL)
		se.StartLocalSound(help_snd);

	// Draw right page.
	m_menu_side = 1; //mxd. Always draw on right side.

	help_msg = CL_GetGameString(GM_HELP2);
	help_snd = CL_GetGameWav(GM_HELP2);

	if (help_msg != NULL)
		Menu_DrawObjectives(help_msg, line_length);

	if (help_snd != NULL)
		se.StartLocalSound(help_snd);

	//mxd. Restore menu_side state.
	m_menu_side = menu_side;
}

void M_Menu_Help_f(void) // H2
{
	M_PushMenu(Help_MenuDraw, Generic_MenuKey);
}