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

#define MAX_SKINS	128

static char* skin_names[MAX_SKINS];
static int num_player_skins;
static cvar_t* skin;
static cvar_t* skin_temp;
static qboolean current_skin_exists;

static menuframework_s s_player_config_menu;

static menufield_s s_player_name_field;
static menulist_s s_player_skin_box;
static menulist_s s_shownames_box;

static void SkinNameFunc(void* self)
{
	NOT_IMPLEMENTED
}

static void ShowNamesFunc(void* self)
{
	NOT_IMPLEMENTED
}

static int PlayerConfig_ScanDirectories(void)
{
	NOT_IMPLEMENTED
	return 0;
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
	strcpy_s(s_player_name_field.buffer, sizeof(s_player_name_field.buffer), name->string); //mxd. strcpy -> strcpy_s
	s_player_name_field.cursor = (int)strlen(s_player_name_field.buffer);

	Com_sprintf(name_skin, sizeof(name_skin), "\x02%s", m_item_skin->string);
	s_player_skin_box.generic.type = MTYPE_PLAYER_SKIN;
	s_player_skin_box.generic.y = 220;
	s_player_skin_box.generic.name = name_skin;
	s_player_skin_box.generic.width = re.BF_Strlen(name_skin);
	s_player_skin_box.generic.callback = SkinNameFunc;
	s_player_skin_box.curvalue = num_player_skins;
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
	NOT_IMPLEMENTED
}

static const char* PlayerConfig_MenuKey(int key)
{
	NOT_IMPLEMENTED
	return NULL;
}

void M_Menu_PlayerConfig_f(void)
{
	if (PlayerConfig_MenuInit()) // H2
		M_PushMenu(PlayerConfig_MenuDraw, PlayerConfig_MenuKey);
}