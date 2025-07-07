//
// menu_loadgame.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "server.h"
#include "menu_saveload.h"
#include "menu_loadgame.h"

cvar_t* m_banner_load;

menuframework_t s_loadgame_menu;
static menuaction_t s_loadgame_actions[MAX_SAVEGAMES];

char m_savestrings[MAX_SAVEGAMES][64];
static qboolean m_savevalid[MAX_SAVEGAMES];

static void CheckSavegameDir(char* savedir)
{
	char search_path[MAX_OSPATH];
	FILE* f;

	qboolean is_valid = true;
	Com_sprintf(search_path, sizeof(search_path), "%s/save/%s/*.*", FS_Userdir(), savedir);

	const char* file_path = Sys_FindFirst(search_path, 0, 0);
	while (file_path != NULL)
	{
		if (fopen_s(&f, file_path, "rb") == 0) //mxd. fopen -> fopen_s
		{
			if (FS_FileLength(f) == 0)
				is_valid = false;

			fclose(f);
		}

		file_path = Sys_FindNext(0, 0);
	}
 
	Sys_FindClose();

	if (!is_valid)
	{
		Com_Printf("Folder %s contains invalid files.... deleting\n", savedir);
		SV_WipeSavegame(savedir);
	}
}

void Create_Savestrings(void)
{
	char file_name[MAX_OSPATH];
	FILE* f;

	for (int i = 0; i < MAX_SAVEGAMES; i++)
	{
		if (i > 0) // H2
			CheckSavegameDir(va("save%i", i));

		Com_sprintf(file_name, sizeof(file_name), "%s/save/save%i/server.ssv", FS_Userdir(), i);

		if (fopen_s(&f, file_name, "rb") == 0) //mxd. fopen -> fopen_s
		{
			FS_Read(m_savestrings[i], sizeof(m_savestrings[i]), f);
			fclose(f);
			m_savevalid[i] = true;
		}
		else
		{
			strcpy_s(m_savestrings[i], sizeof(m_savestrings[i]), MENU_EMPTY); //mxd. strcpy -> strcpy_s
			m_savevalid[i] = false;
		}
	}
}

static void LoadGameCallback(void* self)
{
	const menuaction_t* action = (menuaction_t*)self;
	const int save_index = action->generic.localdata[0];

	if (m_savevalid[save_index])
		Cbuf_AddText(va("load save%i\n", save_index));

	M_ForceMenuOff();
	M_UpdateOrigMode(); // H2
}

static void LoadGame_MenuInit(void)
{
	s_loadgame_menu.nitems = 0;
	Create_Savestrings();

	int y = 0;
	for (int i = 0; i < MAX_SAVEGAMES; i++, y += 20)
	{
		s_loadgame_actions[i].generic.name = m_savestrings[i];
		s_loadgame_actions[i].generic.type = MTYPE_ACTION;
		s_loadgame_actions[i].generic.x = 0;
		s_loadgame_actions[i].generic.y = y;
		s_loadgame_actions[i].generic.width = re.BF_Strlen(m_savestrings[i]);
		s_loadgame_actions[i].generic.flags = QMF_LEFT_JUSTIFY | QMF_MULTILINE | QMF_SELECT_SOUND;
		s_loadgame_actions[i].generic.localdata[0] = i;
		s_loadgame_actions[i].generic.callback = LoadGameCallback;
		
		Menu_AddItem(&s_loadgame_menu, &s_loadgame_actions[i]);
	}

	Menu_Center(&s_loadgame_menu);
}

static void LoadGame_MenuDraw(void)
{
	char title[MAX_QPATH];

	// Draw menu BG.
	re.BookDrawPic("book/back/b_conback8.bk", cls.m_menuscale);

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
	if (key == K_ESCAPE || key == K_ENTER)
		s_savegame_menu.cursor = max(0, s_loadgame_menu.cursor - 1);

	return Default_MenuKey(&s_loadgame_menu, key);
}

// Q2 counterpart
void M_Menu_Loadgame_f(void)
{
	LoadGame_MenuInit();
	M_PushMenu(LoadGame_MenuDraw, LoadGame_MenuKey);
}