//
// menu_mousecfg.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "menu_mousecfg.h"

cvar_t* m_banner_mousecfg;

cvar_t* m_item_mousespeedx;
cvar_t* m_item_mousespeedy;
cvar_t* m_item_mouseinvert;
cvar_t* m_item_lookspring;
cvar_t* m_item_freelook;

static menuframework_t s_mousecfg_menu;

static menulist_t s_options_freelook_box;
static menuslider_t s_options_mousespeedx_slider;
static menuslider_t s_options_mousespeedy_slider;
static menulist_t s_mouseinvert_box;

static void FreeLookFunc(void* self)
{
	Cvar_SetValue("freelook", (float)s_options_freelook_box.curvalue);

	if (Cvar_VariableValue("freelook") != 0.0f) // H2
		Cvar_SetValue("cl_camera_combat", 0);
}

static void MouseSpeedXFunc(void* self) // H2
{
	Cvar_SetValue("mouse_sensitivity_x", s_options_mousespeedx_slider.curvalue * 0.25f);
}

static void MouseSpeedYFunc(void* self) // H2
{
	Cvar_SetValue("mouse_sensitivity_y", s_options_mousespeedy_slider.curvalue * 0.25f);
}

static void MouseInvertFunc(void* self) // H2
{
	Cvar_SetValue("m_pitch", -m_pitch->value);
}

static void MouseCfg_SetValues(void) // H2
{
	Cvar_SetValue("mouse_sensitivity_x", Clamp(mouse_sensitivity_x->value, 1, 25));
	Cvar_SetValue("mouse_sensitivity_y", Clamp(mouse_sensitivity_y->value, 1, 25));

	s_options_mousespeedx_slider.curvalue = mouse_sensitivity_x->value * 4.0f;
	s_options_mousespeedy_slider.curvalue = mouse_sensitivity_y->value * 4.0f;

	s_mouseinvert_box.curvalue = (int)(m_pitch->value < 0.0f);

	Cvar_SetValue("freelook", Clamp(freelook->value, 0, 1));
	s_options_freelook_box.curvalue = Q_ftol(freelook->value);
}

static void MouseCfg_MenuInit(void) // H2
{
	static char name_freelook[MAX_QPATH];
	static char name_mousespeedx[MAX_QPATH];
	static char name_mousespeedy[MAX_QPATH];
	static char name_mouseinvert[MAX_QPATH];

	s_mousecfg_menu.nitems = 0;

	Com_sprintf(name_freelook, sizeof(name_freelook), "\x02%s", m_item_freelook->string);
	s_options_freelook_box.generic.type = MTYPE_SPINCONTROL;
	s_options_freelook_box.generic.x = 0;
	s_options_freelook_box.generic.y = 0;
	s_options_freelook_box.generic.name = name_freelook;
	s_options_freelook_box.generic.width = re.BF_Strlen(name_freelook);
	s_options_freelook_box.generic.flags = QMF_SINGLELINE;
	s_options_freelook_box.generic.callback = FreeLookFunc;
	s_options_freelook_box.itemnames = yes_no_names;

	Com_sprintf(name_mousespeedx, sizeof(name_mousespeedx), "\x02%s", m_item_mousespeedx->string);
	s_options_mousespeedx_slider.generic.type = MTYPE_SLIDER;
	s_options_mousespeedx_slider.generic.x = 0;
	s_options_mousespeedx_slider.generic.y = 20;
	s_options_mousespeedx_slider.generic.name = name_mousespeedx;
	s_options_mousespeedx_slider.generic.width = re.BF_Strlen(name_mousespeedx);
	s_options_mousespeedx_slider.generic.callback = MouseSpeedXFunc;
	s_options_mousespeedx_slider.minvalue = 4.0f;
	s_options_mousespeedx_slider.maxvalue = 100.0f;

	Com_sprintf(name_mousespeedy, sizeof(name_mousespeedy), "\x02%s", m_item_mousespeedy->string);
	s_options_mousespeedy_slider.generic.type = MTYPE_SLIDER;
	s_options_mousespeedy_slider.generic.x = 0;
	s_options_mousespeedy_slider.generic.y = 60;
	s_options_mousespeedy_slider.generic.name = name_mousespeedy;
	s_options_mousespeedy_slider.generic.width = re.BF_Strlen(name_mousespeedy);
	s_options_mousespeedy_slider.generic.callback = MouseSpeedYFunc;
	s_options_mousespeedy_slider.minvalue = 4.0f;
	s_options_mousespeedy_slider.maxvalue = 100.0f;

	Com_sprintf(name_mouseinvert, sizeof(name_mouseinvert), "\x02%s", m_item_mouseinvert->string);
	s_mouseinvert_box.generic.type = MTYPE_SPINCONTROL;
	s_mouseinvert_box.generic.x = 0;
	s_mouseinvert_box.generic.y = 100;
	s_mouseinvert_box.generic.name = name_mouseinvert;
	s_mouseinvert_box.generic.width = re.BF_Strlen(name_mouseinvert);
	s_mouseinvert_box.generic.flags = QMF_SINGLELINE;
	s_mouseinvert_box.generic.callback = MouseInvertFunc;
	s_mouseinvert_box.itemnames = yes_no_names;

	MouseCfg_SetValues();

	Menu_AddItem(&s_mousecfg_menu, &s_options_freelook_box);
	Menu_AddItem(&s_mousecfg_menu, &s_options_mousespeedx_slider);
	Menu_AddItem(&s_mousecfg_menu, &s_options_mousespeedy_slider);
	Menu_AddItem(&s_mousecfg_menu, &s_mouseinvert_box);

	Menu_Center(&s_mousecfg_menu);
}

static void MouseCfg_MenuDraw(void) // H2
{
	char title[MAX_QPATH];

	// Draw menu BG.
	Menu_DrawBG("book/back/b_conback8.bk", cls.m_menuscale);

	if (cls.m_menualpha == 0.0f)
		return;

	// Draw menu title.
	Com_sprintf(title, sizeof(title), "\x03%s", m_banner_mousecfg->string);
	const int x = M_GetMenuLabelX(re.BF_Strlen(title));
	const int y = M_GetMenuOffsetY(&s_mousecfg_menu);
	re.DrawBigFont(x, y, title, cls.m_menualpha);

	// Draw menu items.
	s_mousecfg_menu.x = M_GetMenuLabelX(s_mousecfg_menu.width);
	Menu_AdjustCursor(&s_mousecfg_menu, 1);
	Menu_Draw(&s_mousecfg_menu);
}

static const char* MouseCfg_MenuKey(const int key) // H2
{
	return Default_MenuKey(&s_mousecfg_menu, key);
}

void M_Menu_MouseCfg_f(void) // H2
{
	MouseCfg_MenuInit();
	M_PushMenu(MouseCfg_MenuDraw, MouseCfg_MenuKey);
}