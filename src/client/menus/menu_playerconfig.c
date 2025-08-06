//
// menu_playerconfig.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "menu_playerconfig.h"

cvar_t* m_banner_pconfig;

cvar_t* m_item_name;
cvar_t* m_item_skin;
cvar_t* m_item_shownames;
cvar_t* skin_temp;

typedef struct
{
	char directory[MAX_OSPATH];
	char psex[MAX_QPATH]; // male/female
} playermodelinfo_s;

#define MAX_PLAYER_SKINS	128

static int s_num_player_skins;
static int s_current_skin_index;
static qboolean current_skin_exists;
static char* skin_names[MAX_PLAYER_SKINS];
static cvar_t* skin;

static menuframework_t s_player_config_menu;

static menufield_t s_player_name_field;
static menulist_t s_player_skin_box;
static menulist_t s_shownames_box;

static void SkinNameFunc(void* self) // H2
{
	Cvar_Set("skin_temp", skin_names[s_player_skin_box.curvalue]);
}

static void ShowNamesFunc(void* self) // H2
{
	Cvar_SetValue("shownames", (float)s_shownames_box.curvalue);
}

//TODO: this returns 8 categories instead of 2!
static int ScanPlayerModels(playermodelinfo_s* info) // H2
{
	char findname[MAX_OSPATH];

	int num_models = 0;
	char* path = FS_NextPath(NULL);

	while (path != NULL)
	{
		Com_sprintf(findname, sizeof(findname), "%s/%s/*.", path, playerdir->string);
		const char* subdir = Sys_FindFirst(findname, SFF_SUBDIR, SFF_HIDDEN | SFF_SYSTEM);

		while (subdir != NULL)
		{
			if (num_models >= MAX_PLAYER_MODELS)
				break;

			Com_sprintf(info->directory, sizeof(info->directory), "%s/%s", path, playerdir->string);
			const int len = (int)strlen(info->directory);
			strcpy_s(info->psex, sizeof(info->psex), &subdir[len + 1]); //mxd. strcpy -> strcpy_s
			subdir = Sys_FindNext(SFF_SUBDIR, SFF_HIDDEN | SFF_SYSTEM);

			num_models++;
			info++;
		}

		Sys_FindClose();
		path = FS_NextPath(path);
	}

	return num_models;
}

static void ScanPlayerSkins(playermodelinfo_s* info) // H2
{
	static char skin_names_array[MAX_PLAYER_SKINS][MAX_OSPATH];

	char path[MAX_OSPATH];
	char skin_name[MAX_OSPATH];
	char skin_path[MAX_OSPATH];
	char skin_icon_path[MAX_OSPATH];
	int icon_w;
	int icon_h;
	int skin_w;
	int skin_h;

	Com_sprintf(path, sizeof(path), "%s/%s/*_i.m8", info->directory, info->psex);
	const char* skin_file = Sys_FindFirst(path, 0, 0);

	while (skin_file != NULL && s_num_player_skins < MAX_PLAYER_SKINS)
	{
		strcpy_s(skin_name, sizeof(skin_name), &skin_file[strlen(info->directory) + 1]); //mxd. strcpy -> strcpy_s
		skin_name[strlen(skin_name) - 5] = 0; // Drop the '_i.m8' part.
		Com_sprintf(skin_path, sizeof(skin_path), "%s/%s.m8", playerdir->string, skin_name);
		Com_sprintf(skin_icon_path, sizeof(skin_icon_path), "%s/%s_i.m8", playerdir->string, skin_name);

		re.DrawGetPicSize(&skin_w, &skin_h, skin_path);
		re.DrawGetPicSize(&icon_w, &icon_h, skin_icon_path);

		if (skin_w == 256 && skin_h == 256 && icon_w > 0 && icon_h > 0)
		{
			skin_names[s_num_player_skins] = skin_names_array[s_num_player_skins];
			strcpy_s(skin_names_array[s_num_player_skins], sizeof(skin_names_array[s_num_player_skins]), skin_name); //mxd. strcpy -> strcpy_s

			if (Q_stricmp(skin->string, skin_name) == 0)
			{
				s_current_skin_index = s_num_player_skins;
				current_skin_exists = true;
			}

			s_num_player_skins++;
		}

		skin_file = Sys_FindNext(0, 0);
	}

	Sys_FindClose();
	skin_names[s_num_player_skins] = NULL;
}

//mxd. Returns the number of skins loaded.
static int PlayerConfig_ScanDirectories(void)
{
	playermodelinfo_s infos[MAX_PLAYER_MODELS];

	s_num_player_skins = 0;
	s_current_skin_index = 0;
	current_skin_exists = false;

	const int num_models = ScanPlayerModels(infos);
	for (int i = 0; i < num_models; i++)
		ScanPlayerSkins(&infos[i]);

	return s_num_player_skins;
}

static qboolean PlayerConfig_MenuInit(void)
{
	static char name_name[MAX_QPATH];
	static char name_skin[MAX_QPATH];
	static char name_shownames[MAX_QPATH];

	skin = Cvar_Get("skin", "", 0);
	skin_temp = Cvar_Get("skin_temp", "", 0);
	Cvar_Set("skin_temp", skin->string);

	if (PlayerConfig_ScanDirectories() == 0)
		return false;

	if (!current_skin_exists)
		Cvar_Set("skin_temp", skin_names[0]);

	s_player_config_menu.nitems = 0;

	Com_sprintf(name_name, sizeof(name_name), "\x02%s", m_item_name->string);
	s_player_name_field.generic.type = MTYPE_FIELD;
	s_player_name_field.generic.name = name_name;
	s_player_name_field.generic.width = re.BF_Strlen(name_name);
	s_player_name_field.generic.callback = NULL;
	s_player_name_field.generic.y = 0;
	s_player_name_field.length = 20;
	s_player_name_field.visible_length = 20;
	strcpy_s(s_player_name_field.buffer, sizeof(s_player_name_field.buffer), player_name->string); //mxd. strcpy -> strcpy_s
	s_player_name_field.cursor = (int)strlen(s_player_name_field.buffer);

	Com_sprintf(name_skin, sizeof(name_skin), "\x02%s", m_item_skin->string);
	s_player_skin_box.generic.type = MTYPE_PLAYER_SKIN;
	s_player_skin_box.generic.y = 220;
	s_player_skin_box.generic.name = name_skin;
	s_player_skin_box.generic.width = re.BF_Strlen(name_skin);
	s_player_skin_box.generic.callback = SkinNameFunc;
	s_player_skin_box.curvalue = s_current_skin_index;
	s_player_skin_box.itemnames = skin_names;

	Com_sprintf(name_shownames, sizeof(name_shownames), "\x02%s", m_item_shownames->string);
	s_shownames_box.generic.type = MTYPE_SPINCONTROL;
	s_shownames_box.generic.x = 0;
	s_shownames_box.generic.y = 280;
	s_shownames_box.generic.name = name_shownames;
	s_shownames_box.generic.width = re.BF_Strlen(name_shownames);
	s_shownames_box.generic.flags = QMF_SINGLELINE;
	s_shownames_box.generic.callback = ShowNamesFunc;
	s_shownames_box.itemnames = yes_no_names;
	s_shownames_box.curvalue = ((int)shownames->value != 0);

	Menu_AddItem(&s_player_config_menu, &s_player_name_field);
	Menu_AddItem(&s_player_config_menu, &s_player_skin_box);
	Menu_AddItem(&s_player_config_menu, &s_shownames_box);

	Menu_Center(&s_player_config_menu);

	return true;
}

static void PlayerConfig_MenuDraw(void)
{
	char title[MAX_QPATH];

	// Draw menu BG.
	Menu_DrawBG("book/back/b_conback8.bk", cls.m_menuscale);

	if (cls.m_menualpha == 0.0f)
		return;

	// Draw menu title.
	Com_sprintf(title, sizeof(title), "\x03%s", m_banner_pconfig->string);
	const int x = M_GetMenuLabelX(re.BF_Strlen(title));
	const int y = M_GetMenuOffsetY(&s_player_config_menu);
	re.DrawBigFont(x, y, title, cls.m_menualpha);

	// Draw menu items.
	s_player_config_menu.x = M_GetMenuLabelX(s_player_config_menu.width);
	Menu_Draw(&s_player_config_menu);
}

static const char* PlayerConfig_MenuKey(const int key)
{
	if (cls.m_menustate != 2)
		return NULL;

	switch (key)
	{
		case K_ENTER:
		case K_KP_ENTER:
			Cvar_Set("skin", skin_names[s_player_skin_box.curvalue]);
			M_PopMenu();
			// Intentional fallthrough.

		case K_ESCAPE:
			if (!Menu_SelectItem(&s_player_config_menu))
			{
				Cvar_Set("name", s_player_name_field.buffer);
				Cvar_Set("skin", skin_names[s_player_skin_box.curvalue]);
			}
			break;

		default:
			break;
	}

	return Default_MenuKey(&s_player_config_menu, key);
}

void M_Menu_PlayerConfig_f(void)
{
	if (PlayerConfig_MenuInit()) // H2
		M_PushMenu(PlayerConfig_MenuDraw, PlayerConfig_MenuKey);
}