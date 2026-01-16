//
// menu_loadgame.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "server.h"
#include "menu_loadgame.h"

cvar_t* m_banner_load;

static menuframework_t s_loadgame_menu;
static menu_saveload_action_t s_loadgame_actions[MAX_SAVEGAMES];

static void CheckSavegameDir(char* save_dir)
{
	char search_path[MAX_OSPATH];
	FILE* f;

	qboolean is_valid = true;
	Com_sprintf(search_path, sizeof(search_path), "%s/save/%s/*.*", FS_Userdir(), save_dir);

	const char* file_path = Sys_FindFirst(search_path, 0, 0);
	while (file_path != NULL)
	{
		if (fopen_s(&f, file_path, "rb") == 0) //mxd. fopen -> fopen_s
		{
			const int len = FS_FileLength(f);
			fclose(f);

			if (len == 0)
			{
				is_valid = false;
				break;
			}
		}

		file_path = Sys_FindNext(0, 0);
	}
 
	Sys_FindClose();

	if (!is_valid)
	{
		Com_Printf("Folder %s contains invalid files... deleting\n", save_dir);
		SV_WipeSavegame(save_dir);
	}
}

//mxd. Returns true if at least 1 savegame exists. For menu_game logic. Stripped version of InitSaveLoadActions()...
qboolean CanShowLoadgameMenu(void)
{
	char save_dir[64];

	for (int i = 0; i < MAX_SAVEGAMES; i++)
	{
		if (i == 0) // QUICKSAVE.
			sprintf_s(save_dir, sizeof(save_dir), "quick");
		else
			sprintf_s(save_dir, sizeof(save_dir), "save%i", i - 1);

		char file_name[MAX_OSPATH];
		Com_sprintf(file_name, sizeof(file_name), "%s/save/%s/server.ssv", FS_Userdir(), save_dir);

		FILE* f;
		if (fopen_s(&f, file_name, "rb") == 0)
		{
			fclose(f);
			return true;
		}
	}

	return false;
}

void InitSaveLoadActions(menu_saveload_action_t* items, const int num_items)
{
	menu_saveload_action_t* item = &items[0];

	for (int i = 0; i < num_items; i++, item++) // quick (QUICKSAVE), save0 (ENTERING), save1, save2...
	{
		if (i == 0) // QUICKSAVE.
		{
			sprintf_s(item->save_dir, sizeof(item->save_dir), "quick");
			item->load_only = true;
		}
		else
		{
			sprintf_s(item->save_dir, sizeof(item->save_dir), "save%i", i - 1);
			item->load_only = (i == 1); // save0 is ENTERING save.
		}

		CheckSavegameDir(item->save_dir); //TODO: vanilla logic skips ENTERING save check. Why?

		char file_name[MAX_OSPATH];
		Com_sprintf(file_name, sizeof(file_name), "%s/save/%s/server.ssv", FS_Userdir(), item->save_dir);

		FILE* f;
		if (fopen_s(&f, file_name, "rb") == 0) //mxd. fopen -> fopen_s
		{
			FS_Read(item->save_name, sizeof(item->save_name), f);
			fclose(f);
			item->is_valid = true;
		}
		else
		{
			strcpy_s(item->save_name, sizeof(item->save_name), MENU_EMPTY); //mxd. strcpy -> strcpy_s
			item->is_valid = false;
		}
	}
}

static void LoadGameCallback(void* self)
{
	const menu_saveload_action_t* action = (menu_saveload_action_t*)self;

	Cbuf_AddText(va("load %s\n", action->save_dir));
	M_ForceMenuOff();
}

static void LoadGame_MenuInit(void)
{
	s_loadgame_menu.nitems = 0;
	InitSaveLoadActions(s_loadgame_actions, MAX_SAVEGAMES);

	int y = 0;
	menu_saveload_action_t* item = &s_loadgame_actions[0];
	for (int i = 0; i < MAX_SAVEGAMES; i++, item++)
	{
		item->generic.name = item->save_name;
		item->generic.type = MTYPE_ACTION;
		item->generic.x = 0;
		item->generic.y = y;
		item->generic.width = re.BF_Strlen(item->save_name);
		item->generic.flags = (QMF_LEFT_JUSTIFY | QMF_MULTILINE | QMF_SELECT_SOUND);

		//mxd. Make empty slots grayed (and unselectable).
		if (item->is_valid)
		{
			item->generic.callback = LoadGameCallback;
		}
		else
		{
			item->generic.callback = NULL;
			item->generic.flags |= QMF_GRAYED;
		}

		Menu_AddItem(&s_loadgame_menu, item);
		y += 20;
	}

	Menu_Center(&s_loadgame_menu);
}

static void LoadGame_MenuDraw(void)
{
	char title[MAX_QPATH];

	// Draw menu BG.
	Menu_DrawBG("book/back/b_conback8.bk", cls.m_menuscale);

	if (cls.m_menualpha == 0.0f)
		return;

	// Draw menu title.
	m_menu_side = 0;
	Com_sprintf(title, sizeof(title), "\x03%s", m_banner_load->string);
	const int x = M_GetMenuLabelX(re.BF_Strlen(title));
	const int y = M_GetMenuOffsetY(&s_loadgame_menu);
	re.DrawBigFont(x, y, title, cls.m_menualpha);

	// Draw menu items.
	s_loadgame_menu.x = M_GetMenuLabelX(s_loadgame_menu.width);
	Menu_Draw(&s_loadgame_menu);
}

// Q2 counterpart
static const char* LoadGame_MenuKey(const int key)
{
	return Default_MenuKey(&s_loadgame_menu, key);
}

// Q2 counterpart
void M_Menu_Loadgame_f(void)
{
	if (!CanShowLoadgameMenu()) //mxd. No savegames to load.
		return;

	LoadGame_MenuInit();
	M_PushMenu(LoadGame_MenuDraw, LoadGame_MenuKey);
}