//
// menu_savegame.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "menu_savegame.h"
#include "menu_loadgame.h" // For InitSaveLoadActions() --mxd.

cvar_t* m_banner_save;

static menuframework_t s_savegame_menu;
static menu_saveload_action_t s_savegame_actions[MAX_SAVEGAMES]; //mxd

static void SaveGameCallback(void* self)
{
	const menu_saveload_action_t* action = (menu_saveload_action_t*)self;

	Cbuf_AddText(va("save %s\n", action->save_dir));
	M_ForceMenuOff();
}

static void SaveGame_MenuInit(void)
{
	s_savegame_menu.nitems = 0;
	InitSaveLoadActions(s_savegame_actions, MAX_SAVEGAMES);

	int y = 0;
	menu_saveload_action_t* item = &s_savegame_actions[0];
	for (int i = 0; i < MAX_SAVEGAMES; i++, item++)
	{
		//mxd. Skip load-only slots.
		if (item->load_only)
			continue;

		item->generic.name = item->save_name;
		item->generic.type = MTYPE_ACTION;
		item->generic.x = 0;
		item->generic.y = y;
		item->generic.width = re.BF_Strlen(item->save_name);
		item->generic.flags = (QMF_LEFT_JUSTIFY | QMF_MULTILINE | QMF_SELECT_SOUND);
		item->generic.callback = SaveGameCallback;

		Menu_AddItem(&s_savegame_menu, &s_savegame_actions[i]);
		y += 20;
	}

	Menu_Center(&s_savegame_menu); // H2
}

static void SaveGame_MenuDraw(void)
{
	char title[MAX_QPATH];

	// Draw menu BG.
	Menu_DrawBG("book/back/b_conback8.bk", cls.m_menuscale);

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
	return Default_MenuKey(&s_savegame_menu, key);
}

void M_Menu_Savegame_f(void)
{
	if (!Com_ServerState() || cl.frame.playerstate.cinematicfreeze)
		return; // Not playing a game.

	SaveGame_MenuInit();
	M_PushMenu(SaveGame_MenuDraw, SaveGame_MenuKey);
}