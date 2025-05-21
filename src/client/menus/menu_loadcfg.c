//
// menu_loadcfg.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "menu_loadcfg.h"

cvar_t* m_banner_loadcfg;

#define NUM_CONFIG_ITEMS	8

static menuframework_s s_loadcfg_menu;
static menuaction_s loadcfg_items[NUM_CONFIG_ITEMS];

static char cfg_filenames[NUM_CONFIG_ITEMS][MAX_QPATH];

static void LoadCfgFunc(void* self) // H2
{
	const menuaction_s* item = self;
	Cbuf_AddText(va("exec config/%s.cfg\n", cfg_filenames[item->generic.localdata[0]]));
	M_PopMenu();
}

static int LoadConfigFilenames(void) // H2
{
	char mask[MAX_OSPATH];
	Com_sprintf(mask, sizeof(mask), "%s/config/*.cfg", FS_GetPath("config/*.cfg"));
	const int len = (int)strlen(mask);

	const char* cfg_filename = Sys_FindFirst(mask, 0, 0);
	int num_configs = 0;

	while (cfg_filename != NULL && num_configs < NUM_CONFIG_ITEMS)
	{
		// Get cfg filename without extension.
		strncpy_s(cfg_filenames[num_configs], sizeof(cfg_filenames[num_configs]), &cfg_filename[len - 5], strlen(cfg_filename) - len + 1); //mxd. strncpy -> strncpy_s

		cfg_filename = Sys_FindNext(0, 0);
		num_configs++;
	}

	Sys_FindClose();

	return num_configs;
}

static int LoadCfg_MenuInit(void) // H2
{
	s_loadcfg_menu.nitems = 0;

	const int num_configs = LoadConfigFilenames();

	if (num_configs == 0)
		return 0;

	int oy = 0;
	for (int i = 0; i < num_configs; i++, oy += 20)
	{
		menuaction_s* item = &loadcfg_items[i];

		item->generic.type = MTYPE_ACTION;
		item->generic.flags = QMF_LEFT_JUSTIFY;
		item->generic.x = 0;
		item->generic.y = oy;
		item->generic.name = cfg_filenames[i];
		item->generic.width = re.BF_Strlen(cfg_filenames[i]);
		item->generic.localdata[0] = i;
		item->generic.callback = LoadCfgFunc;

		Menu_AddItem(&s_loadcfg_menu, item);
	}

	Menu_Center(&s_loadcfg_menu);

	return num_configs;
}

static void LoadCfg_MenuDraw(void) // H2
{
	char title[MAX_QPATH];

	// Draw menu BG.
	re.BookDrawPic(0, 0, "book/back/b_conback8.bk", cls.m_menuscale);

	if (cls.m_menualpha == 0.0f)
		return;

	// Draw menu title.
	Com_sprintf(title, sizeof(title), "\x03%s", m_banner_loadcfg->string);
	const int x = M_GetMenuLabelX(re.BF_Strlen(title));
	const int y = M_GetMenuOffsetY(&s_loadcfg_menu);
	re.DrawBigFont(x, y, title, cls.m_menualpha);

	// Draw menu items.
	s_loadcfg_menu.x = M_GetMenuLabelX(s_loadcfg_menu.width);
	Menu_Draw(&s_loadcfg_menu);
}

static const char* LoadCfg_MenuKey(const int key)
{
	return Default_MenuKey(&s_loadcfg_menu, key);
}

void M_Menu_LoadCfg_f(void) // H2
{
	if (LoadCfg_MenuInit() > 0)
		M_PushMenu(LoadCfg_MenuDraw, LoadCfg_MenuKey);
}