//
// menu_citymap.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "clfx_dll.h"
#include "LevelMaps.h"
#include "menu_citymap.h"

#include "cl_messages.h"

cvar_t* m_banner_citymap;
cvar_t* m_item_nomap;

static qboolean HaveCityMap(void) // H2
{
	if (!fxapi_initialized)
		return false;

	const int level_num = Q_atoi(cl.configstrings[CS_LEVEL_NUMBER]);
	if (level_num < 2)
		return false;

	if (level_num >= fxe.GetLMIMax()) //mxd. '>' in original logic.
	{
		Com_DPrintf("Error : Invalid map index\n");
		return false;
	}

	const level_map_info_t* map_infos = fxe.GetLMI();
	return (map_infos[level_num].city_map != NULL);
}

static void CityMap_DefaultDraw(void) // H2
{
	char title[MAX_QPATH];

	// Draw menu BG.
	Menu_DrawBG("book/back/b_conback8.bk", cls.m_menuscale);

	if (cls.m_menualpha == 0.0f)
		return;

	// Draw menu title.
	Com_sprintf(title, sizeof(title), "\x03%s", m_item_nomap->string);
	const int x = M_GetMenuLabelX(re.BF_Strlen(title));
	re.DrawBigFont(x, 60, title, cls.m_menualpha);

	if (!fxapi_initialized)
		return;

	// Draw map title.
	const int level_num = Q_atoi(cl.configstrings[CS_LEVEL_NUMBER]);
	const level_map_info_t* map_infos = fxe.GetLMI();
	const char* message = CL_GetGameString(map_infos[level_num].message);
	if (message != NULL)
		Menu_DrawObjectives(message, viddef.width * 18 / DEF_WIDTH + 8);
}

static void CityMap_MenuDraw(void) // H2
{
	if (HaveCityMap())
	{
		const level_map_info_t* map_infos = fxe.GetLMI();
		const int level_num = Q_atoi(cl.configstrings[CS_LEVEL_NUMBER]);
		const level_map_info_t* info = &map_infos[level_num];

		if (info->city_map != NULL)
			Menu_DrawBG(info->city_map, cls.m_menuscale);
	}
	else
	{
		CityMap_DefaultDraw();
	}
}

void M_Menu_City_Map_f(void) // H2
{
	M_PushMenu(CityMap_MenuDraw, Generic_MenuKey);
}