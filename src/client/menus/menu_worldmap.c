//
// menu_worldmap.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "clfx_dll.h"
#include "menu_worldmap.h"

cvar_t* m_banner_worldmap;

static qboolean HaveWorldMap(void) // H2
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

	return true;
}

void M_WorldMap_MenuDraw(void) // H2
{
	if (!HaveWorldMap())
	{
		Menu_DrawBG("book/back/b_worldmap.bk", cls.m_menuscale);
		return;
	}

	level_map_info_t* map_infos = fxe.GetLMI();
	const int level_num = Q_atoi(cl.configstrings[CS_LEVEL_NUMBER]);
	const level_map_info_t* info = &map_infos[level_num];

	if (info->world_map != NULL)
		Menu_DrawBG(info->world_map, cls.m_menuscale);

	if (cls.m_menualpha == 0.0f || (info->flags & LMI_NODRAW) != 0)
		return;

	// Draw old maps progress.
	if (level_num > 2)
	{
		level_map_info_t* dot_info = &map_infos[2];
		for (int i = 0; i < level_num - 2; i++, dot_info++)
		{
			if (dot_info->flags & LMI_PROGRESS)
			{
				int* dot_coords = dot_info->dot_coords;
				for (int c = 0; c < dot_info->count; c++, dot_coords += 2)
					re.DrawStretchPic(dot_coords[0] - 4, dot_coords[1] - 4, 8, 8, "misc/dot.m32", cls.m_menualpha, DSP_SCALE_4x3);
			}

			if (dot_info->flags & LMI_DRAW)
				re.DrawStretchPic(dot_info->x - 8, dot_info->y - 8, 16, 16, "misc/bigdot.m32", cls.m_menualpha, DSP_SCALE_4x3);
		}
	}

	// Draw current map progress.
	if (info->flags & LMI_PROGRESS)
	{
		const int progress = cl.frame.playerstate.map_percentage * info->count / 100;
		int* dot_coords = info->dot_coords;
		for (int i = 0; i < progress; i++, dot_coords += 2)
			re.DrawStretchPic(dot_coords[0] - 4, dot_coords[1] - 4, 8, 8, "misc/dot.m32", cls.m_menualpha, DSP_SCALE_4x3);
	}

	if (info->flags & LMI_DRAW)
		re.DrawStretchPic(info->x - 8, info->y - 8, 16, 16, "misc/bigdot.m32", cls.m_menualpha, DSP_SCALE_4x3);
}

void M_Menu_World_Map_f(void) // H2
{
	M_PushMenu(M_WorldMap_MenuDraw, Generic_MenuKey);
}