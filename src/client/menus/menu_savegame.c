//
// menu_savegame.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "menu_saveload.h"
#include "menu_savegame.h"

cvar_t* m_banner_save;

menuframework_t s_savegame_menu;
static menuaction_t s_savegame_actions[MAX_SAVEGAMES - 1]; //mxd. Don't include the autosave slot.

static void SaveGameCallback(void* self)
{
	const menuaction_t* action = (menuaction_t*)self;

	Cbuf_AddText(va("save save%i\n", action->generic.localdata[0]));
	M_ForceMenuOff();
}

static void SaveGame_MenuInit(void)
{
	s_savegame_menu.nitems = 0;
	Create_Savestrings();

	int y = 0;
	// Don't include the autosave slot.
	for (int i = 0; i < MAX_SAVEGAMES - 1; i++, y += 20)
	{
		s_savegame_actions[i].generic.name = m_savestrings[i + 1];
		s_savegame_actions[i].generic.type = MTYPE_ACTION;
		s_savegame_actions[i].generic.x = 0;
		s_savegame_actions[i].generic.y = y;
		s_savegame_actions[i].generic.width = re.BF_Strlen(m_savestrings[i + 1]);
		s_savegame_actions[i].generic.flags = QMF_LEFT_JUSTIFY | QMF_MULTILINE | QMF_SELECT_SOUND;
		s_savegame_actions[i].generic.localdata[0] = i + 1;
		s_savegame_actions[i].generic.callback = SaveGameCallback;

		Menu_AddItem(&s_savegame_menu, &s_savegame_actions[i]);
	}

	Menu_Center(&s_savegame_menu); // H2
}

static void SaveGame_MenuDraw(void)
{
	char title[MAX_QPATH];

	// Draw menu BG.
	re.BookDrawPic("book/back/b_conback8.bk", cls.m_menuscale);

	if (cls.m_menualpha == 0.0f)
		return;

	// Draw menu title.
	m_menu_side = 0;
	Com_sprintf(title, sizeof(title), "\x03%s", m_banner_save->string);
	const int x = M_GetMenuLabelX(re.BF_Strlen(title));
	const int y = M_GetMenuOffsetY(&s_savegame_menu);
	re.DrawBigFont(x, y, title, cls.m_menualpha);

	// Draw menu items.
	s_savegame_menu.x = M_GetMenuLabelX(s_savegame_menu.width);
	Menu_AdjustCursor(&s_savegame_menu, 1);
	Menu_Draw(&s_savegame_menu);
}

static const char* SaveGame_MenuKey(const int key)
{
	if (key == K_ENTER || key == K_ESCAPE)
		s_loadgame_menu.cursor = max(0, s_savegame_menu.cursor + 1); //mxd. 's_savegame_menu.cursor - 1' in Q2.

	return Default_MenuKey(&s_savegame_menu, key);
}

void M_Menu_Savegame_f(void)
{
	if (!Com_ServerState() || cl.frame.playerstate.cinematicfreeze)
		return; // Not playing a game.

	SaveGame_MenuInit();
	M_PushMenu(SaveGame_MenuDraw, SaveGame_MenuKey);
	//Create_Savestrings(); //mxd. Already called in SaveGame_MenuInit().
}