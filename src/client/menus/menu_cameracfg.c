//
// menu_cameracfg.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "menu_cameracfg.h"

cvar_t* m_banner_cameracfg;

cvar_t* m_item_cameradamp;
cvar_t* m_item_cameracombat;

static menuframework_s s_cameracfg_menu;

static menuslider_s s_options_cameradamp_slider;
static menulist_s s_options_cameracombat_box;

static void CameraDampFactorFunc(void* self) // H2
{
	NOT_IMPLEMENTED
}

static void CameraCombatFunc(void* self) // H2
{
	NOT_IMPLEMENTED
}

static void CameraCfg_SetValues(void) // H2
{
	NOT_IMPLEMENTED
}

static void CameraCfg_MenuInit(void) // H2
{
	static char name_cameradamp[MAX_QPATH];
	static char name_cameracombat[MAX_QPATH];

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

	//TODO: not added to the menu! Add or remove logic?
	Com_sprintf(name_cameracombat, sizeof(name_cameracombat), "\x02%s", m_item_cameracombat->string);
	s_options_cameracombat_box.generic.type = MTYPE_SPINCONTROL;
	s_options_cameracombat_box.generic.x = 0;
	s_options_cameracombat_box.generic.y = 40;
	s_options_cameracombat_box.generic.name = name_cameracombat;
	s_options_cameracombat_box.generic.width = re.BF_Strlen(name_cameracombat);
	s_options_cameracombat_box.generic.flags = QMF_SINGLELINE;
	s_options_cameracombat_box.generic.callback = CameraCombatFunc;
	s_options_cameracombat_box.itemnames = yes_no_names;

	CameraCfg_SetValues();

	Menu_AddItem(&s_cameracfg_menu, &s_options_cameradamp_slider);
	Menu_Center(&s_cameracfg_menu);
}

static void CameraCfg_MenuDraw(void) // H2
{
	NOT_IMPLEMENTED
}

static const char* CameraCfg_MenuKey(int key) // H2
{
	NOT_IMPLEMENTED
	return NULL;
}

void M_Menu_CameraCfg_f(void) // H2
{
	CameraCfg_MenuInit();
	M_PushMenu(CameraCfg_MenuDraw, CameraCfg_MenuKey);
}