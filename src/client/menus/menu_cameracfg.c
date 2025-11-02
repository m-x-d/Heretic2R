//
// menu_cameracfg.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "menu_cameracfg.h"

cvar_t* m_banner_cameracfg;
cvar_t* m_item_cameradamp;

static menuframework_t s_cameracfg_menu;
static menuslider_t s_options_cameradamp_slider;

static void CameraDampFactorFunc(void* self) // H2
{
	Cvar_SetValue("cl_camera_dampfactor", s_options_cameradamp_slider.curvalue * 0.01f);
}

static void CameraCfg_SetValues(void) // H2
{
	Cvar_SetValue("cl_camera_dampfactor", Clamp(cl_camera_dampfactor->value, 0, 1));
	s_options_cameradamp_slider.curvalue = cl_camera_dampfactor->value * 100.0f;
}

static void CameraCfg_MenuInit(void) // H2
{
	static char name_cameradamp[MAX_QPATH];

	s_cameracfg_menu.nitems = 0;

	Com_sprintf(name_cameradamp, sizeof(name_cameradamp), "\x02%s", m_item_cameradamp->string);
	s_options_cameradamp_slider.generic.type = MTYPE_SLIDER;
	s_options_cameradamp_slider.generic.x = 0;
	s_options_cameradamp_slider.generic.y = 0;
	s_options_cameradamp_slider.generic.name = name_cameradamp;
	s_options_cameradamp_slider.generic.width = re.BF_Strlen(name_cameradamp);
	s_options_cameradamp_slider.generic.callback = CameraDampFactorFunc;
	s_options_cameradamp_slider.minvalue = 0.0f;
	s_options_cameradamp_slider.maxvalue = 100.0f;

	CameraCfg_SetValues();

	Menu_AddItem(&s_cameracfg_menu, &s_options_cameradamp_slider);
	Menu_Center(&s_cameracfg_menu);
}

static void CameraCfg_MenuDraw(void) // H2
{
	char title[MAX_QPATH];

	// Draw menu BG.
	Menu_DrawBG("book/back/b_conback8.bk", cls.m_menuscale);

	if (cls.m_menualpha == 0.0f)
		return;

	// Draw menu title.
	Com_sprintf(title, sizeof(title), "\x03%s", m_banner_cameracfg->string);
	const int x = M_GetMenuLabelX(re.BF_Strlen(title));
	const int y = M_GetMenuOffsetY(&s_cameracfg_menu);
	re.DrawBigFont(x, y, title, cls.m_menualpha);

	// Draw menu items.
	s_cameracfg_menu.x = M_GetMenuLabelX(s_cameracfg_menu.width);
	Menu_AdjustCursor(&s_cameracfg_menu, 1);
	Menu_Draw(&s_cameracfg_menu);
}

static const char* CameraCfg_MenuKey(const int key) // H2
{
	return Default_MenuKey(&s_cameracfg_menu, key);
}

void M_Menu_CameraCfg_f(void) // H2
{
	CameraCfg_MenuInit();
	M_PushMenu(CameraCfg_MenuDraw, CameraCfg_MenuKey);
}