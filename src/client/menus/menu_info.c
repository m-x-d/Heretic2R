//
// menu_info.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "menu_info.h"
#include "menu_citymap.h"
#include "menu_objectives.h"
#include "menu_worldmap.h"

cvar_t* m_banner_info;

static menuframework_t s_info_menu;

static menuaction_t s_info_worldmap_action;
static menuaction_t s_info_citymap_action;
static menuaction_t s_info_objectives_action;

static void WorldMapFunc(void* self)
{
	M_Menu_World_Map_f();
}

static void CityMapFunc(void* self)
{
	M_Menu_City_Map_f();
}

static void ObjectivesFunc(void* self)
{
	M_Menu_Objectives_f();
}

static void Info_MenuInit(void)
{
	static char name_worldmap[MAX_QPATH];
	static char name_citymap[MAX_QPATH];
	static char name_objectives[MAX_QPATH];

	s_info_menu.nitems = 0;

	//TODO: better y-spacing?
	Com_sprintf(name_worldmap, sizeof(name_worldmap), "\x02%s", m_banner_worldmap->string);
	s_info_worldmap_action.generic.type = MTYPE_ACTION;
	s_info_worldmap_action.generic.flags = QMF_LEFT_JUSTIFY;
	s_info_worldmap_action.generic.x = 0;
	s_info_worldmap_action.generic.y = 0;
	s_info_worldmap_action.generic.name = name_worldmap;
	s_info_worldmap_action.generic.width = re.BF_Strlen(name_worldmap);
	s_info_worldmap_action.generic.callback = WorldMapFunc;

	Com_sprintf(name_citymap, sizeof(name_citymap), "\x02%s", m_banner_citymap->string);
	s_info_citymap_action.generic.type = MTYPE_ACTION;
	s_info_citymap_action.generic.flags = QMF_LEFT_JUSTIFY;
	s_info_citymap_action.generic.x = 0;
	s_info_citymap_action.generic.y = 40;
	s_info_citymap_action.generic.name = name_citymap;
	s_info_citymap_action.generic.width = re.BF_Strlen(name_citymap);
	s_info_citymap_action.generic.callback = CityMapFunc;

	Com_sprintf(name_objectives, sizeof(name_objectives), "\x02%s", m_banner_objectives->string);
	s_info_objectives_action.generic.type = MTYPE_ACTION;
	s_info_objectives_action.generic.flags = QMF_LEFT_JUSTIFY;
	s_info_objectives_action.generic.x = 0;
	s_info_objectives_action.generic.y = 80;
	s_info_objectives_action.generic.name = name_objectives;
	s_info_objectives_action.generic.width = re.BF_Strlen(name_objectives);
	s_info_objectives_action.generic.callback = ObjectivesFunc;

	Menu_AddItem(&s_info_menu, &s_info_worldmap_action.generic);
	Menu_AddItem(&s_info_menu, &s_info_citymap_action.generic);
	Menu_AddItem(&s_info_menu, &s_info_objectives_action.generic);

	Menu_Center(&s_info_menu);
}

static void Info_MenuDraw(void)
{
	char title[MAX_QPATH];

	// Draw menu BG.
	re.BookDrawPic(0, 0, "book/back/b_conback8.bk", cls.m_menuscale);

	if (cls.m_menualpha == 0.0f)
		return;

	// Draw menu title.
	Com_sprintf(title, sizeof(title), "\x03%s", m_banner_info->string);
	const int x = M_GetMenuLabelX(re.BF_Strlen(title));
	const int y = M_GetMenuOffsetY(&s_info_menu);
	re.DrawBigFont(x, y, title, cls.m_menualpha);

	// Draw menu items.
	s_info_menu.x = M_GetMenuLabelX(s_info_menu.width);
	Menu_AdjustCursor(&s_info_menu, 1);
	Menu_Draw(&s_info_menu);
}

static const char* Info_MenuKey(const int key)
{
	return Default_MenuKey(&s_info_menu, key);
}

void M_Menu_Info_f(void) // H2
{
	Info_MenuInit();
	M_PushMenu(Info_MenuDraw, Info_MenuKey);
}