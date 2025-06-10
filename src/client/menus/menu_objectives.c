//
// menu_objectives.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "cl_messages.h"
#include "menu_objectives.h"

cvar_t* m_banner_objectives;
cvar_t* m_item_none;

static void Objectives_MenuDraw(void)
{
	const char* message1 = NULL;
	const char* message2 = NULL;
	const char* sound1 = NULL;

	// Draw menu BG.
	re.BookDrawPic(0, 0, "book/back/b_conback8.bk", cls.m_menuscale);

	if (cls.m_menualpha == 0.0f)
		return;

	const int max_line_length = viddef.width * 18 / DEF_WIDTH + 8;

	// Draw first objective?
	if (cl.frame.playerstate.mission_num1 != 0)
	{
		message1 = CL_GetLevelString(cl.frame.playerstate.mission_num1);
		sound1 = CL_GetLevelWav(cl.frame.playerstate.mission_num1);

		if (message1 != NULL)
		{
			Menu_DrawTitle(m_banner_objectives);
			Menu_DrawObjectives(message1, max_line_length /*/ DEF_WIDTH + 8*/);
		}

		if (sound1 != NULL)
			se.StartLocalSound(sound1);
	}

	m_menu_side ^= 1;

	// Draw second objective?
	if (cl.frame.playerstate.mission_num2 != 0)
	{
		message2 = CL_GetLevelString(cl.frame.playerstate.mission_num2);
		const char* sound2 = CL_GetLevelWav(cl.frame.playerstate.mission_num2);

		if (message2 != NULL)
		{
			Menu_DrawTitle(m_banner_objectives);
			Menu_DrawObjectives(message2, max_line_length);
		}

		if (sound1 == NULL && sound2 != NULL)
			se.StartLocalSound(sound2);
	}

	m_menu_side ^= 1;

	// Draw 'no objectives' message?
	if (message1 == NULL && message2 == NULL)
	{
		Menu_DrawTitle(m_banner_objectives);
		Menu_DrawObjectives(m_item_none->string, max_line_length);
	}
}

void M_Menu_Objectives_f(void) // H2
{
	M_PushMenu(Objectives_MenuDraw, Generic_MenuKey);
}