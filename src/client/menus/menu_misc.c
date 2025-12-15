//
// menu_misc.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "menu_misc.h"

cvar_t* m_banner_misc;

cvar_t* m_item_alwaysrun;
cvar_t* m_item_crosshair;
cvar_t* m_item_caption;
cvar_t* m_item_violence;
cvar_t* m_item_yawspeed;
cvar_t* m_item_console;
cvar_t* m_item_autoweapon;

static menuframework_t s_misc_menu;

static menulist_t s_options_crosshair_box;
static menulist_t s_options_alwaysrun_box;
static menulist_t s_options_autoweapon_box;
static menulist_t s_options_captions_box;
static menuslider_t s_options_yawspeed_slider;
static menulist_t s_options_violence_box;
static menuaction_t s_options_console_action;

// Q2 counterpart
static void CrosshairFunc(void* self)
{
	Cvar_SetValue("crosshair", (float)s_options_crosshair_box.curvalue);
}

// Q2 counterpart
static void AlwaysRunFunc(void* self)
{
	Cvar_SetValue("cl_run", (float)s_options_alwaysrun_box.curvalue);
}

static void AutoWeaponFunc(void* self) // H2
{
	Cvar_SetValue("autoweapon", (float)s_options_autoweapon_box.curvalue);
}

static void ShowCaptionsFunc(void* self) // H2
{
	Cvar_SetValue("cl_showcaptions", (float)s_options_captions_box.curvalue);
}

static void YawSpeedFunc(void* self) // H2
{
	const menuslider_t* s = self;
	Cvar_SetValue("cl_yawspeed", s->curvalue * 15.0f);
}

static void ViolenceFunc(void* self) // H2
{
	const menulist_t* l = self;
	Cvar_SetValue("blood_level", (float)l->curvalue);
}

// Q2 counterpart
static void ConsoleFunc(void* self)
{
	// The proper way to do this is probably to have ToggleConsole_f accept a parameter.
	if (cl.attractloop)
	{
		Cbuf_AddText("killserver\n");
		return;
	}

	Key_ClearTyping();
	Con_ClearNotify();

	M_ForceMenuOff();
	cls.key_dest = key_console;
}

static void Misc_SetValues(void) // H2
{
	Cvar_SetValue("cl_run", Clamp(cl_run->value, 0.0f, 1.0f));
	s_options_alwaysrun_box.curvalue = (int)cl_run->value;

	Cvar_SetValue("crosshair", Clamp(crosshair->value, 0.0f, 3.0f));
	s_options_crosshair_box.curvalue = (int)crosshair->value;
}

static void Misc_MenuInit(void) // H2
{
#define NUM_CROSSHAIRS		4
#define NUM_VIOLENCE_LEVELS	4

	static char crosshair_names[NUM_CROSSHAIRS][MAX_QPATH];
	static char* crosshair_refs[NUM_CROSSHAIRS + 1];

	static char violence_names[NUM_VIOLENCE_LEVELS][MAX_QPATH];
	static char* violence_refs[NUM_VIOLENCE_LEVELS + 1];

	static char name_crosshair[MAX_QPATH];
	static char name_alwaysrun[MAX_QPATH];
	static char name_autoweapon[MAX_QPATH];
	static char name_caption[MAX_QPATH];
	static char name_yawspeed[MAX_QPATH];
	static char name_violence[MAX_QPATH];
	static char name_console[MAX_QPATH];

	char cvar_name[MAX_QPATH];

	// Init crosshair labels.
	for (int i = 0; i < NUM_CROSSHAIRS; i++)
	{
		crosshair_refs[i] = crosshair_names[i];
		Com_sprintf(cvar_name, sizeof(cvar_name), "m_generic_crosshair%d", i);

		const cvar_t* cvar = Cvar_Get(cvar_name, "", 0);
		strcpy_s(crosshair_names[i], sizeof(crosshair_names[i]), cvar->string);
	}

	crosshair_refs[NUM_CROSSHAIRS] = NULL;

	// Init violence level labels.
	for (int i = 0; i < NUM_VIOLENCE_LEVELS; i++)
	{
		violence_refs[i] = violence_names[i];
		Com_sprintf(cvar_name, sizeof(cvar_name), "m_generic_violence%d", i);

		const cvar_t* cvar = Cvar_Get(cvar_name, "", 0);
		strcpy_s(violence_names[i], sizeof(violence_names[i]), cvar->string);
	}

	violence_refs[NUM_VIOLENCE_LEVELS] = NULL;

	// Clamp blood level.
	Cvar_SetValue("blood_level", Clamp(Cvar_VariableValue("blood_level"), 0.0f, 3.0f));

	s_misc_menu.nitems = 0;

	Com_sprintf(name_crosshair, sizeof(name_crosshair), "\x02%s", m_item_crosshair->string);
	s_options_crosshair_box.generic.type = MTYPE_SPINCONTROL;
	s_options_crosshair_box.generic.x = 0;
	s_options_crosshair_box.generic.y = 0;
	s_options_crosshair_box.generic.name = name_crosshair;
	s_options_crosshair_box.generic.width = re.BF_Strlen(name_crosshair);
	s_options_crosshair_box.generic.flags = QMF_SINGLELINE;
	s_options_crosshair_box.generic.callback = CrosshairFunc;
	s_options_crosshair_box.itemnames = crosshair_refs;

	Com_sprintf(name_alwaysrun, sizeof(name_alwaysrun), "\x02%s", m_item_alwaysrun->string);
	s_options_alwaysrun_box.generic.type = MTYPE_SPINCONTROL;
	s_options_alwaysrun_box.generic.x = 0;
	s_options_alwaysrun_box.generic.y = 20;
	s_options_alwaysrun_box.generic.name = name_alwaysrun;
	s_options_alwaysrun_box.generic.width = re.BF_Strlen(name_alwaysrun);
	s_options_alwaysrun_box.generic.flags = QMF_SINGLELINE;
	s_options_alwaysrun_box.generic.callback = AlwaysRunFunc;
	s_options_alwaysrun_box.itemnames = yes_no_names;

	Com_sprintf(name_autoweapon, sizeof(name_autoweapon), "\x02%s", m_item_autoweapon->string);
	s_options_autoweapon_box.generic.type = MTYPE_SPINCONTROL;
	s_options_autoweapon_box.generic.x = 0;
	s_options_autoweapon_box.generic.y = 40;
	s_options_autoweapon_box.generic.name = name_autoweapon;
	s_options_autoweapon_box.generic.width = re.BF_Strlen(name_autoweapon);
	s_options_autoweapon_box.generic.flags = QMF_SINGLELINE;
	s_options_autoweapon_box.generic.callback = AutoWeaponFunc;
	s_options_autoweapon_box.itemnames = yes_no_names;
	s_options_autoweapon_box.curvalue = (int)(autoweapon->value != 0.0f);

	Com_sprintf(name_caption, sizeof(name_caption), "\x02%s", m_item_caption->string);
	s_options_captions_box.generic.type = MTYPE_SPINCONTROL;
	s_options_captions_box.generic.x = 0;
	s_options_captions_box.generic.y = 60;
	s_options_captions_box.generic.name = name_caption;
	s_options_captions_box.generic.width = re.BF_Strlen(name_caption);
	s_options_captions_box.generic.flags = QMF_SINGLELINE;
	s_options_captions_box.generic.callback = ShowCaptionsFunc;
	s_options_captions_box.curvalue = (int)(Cvar_VariableValue("cl_showcaptions") != 0.0f); //mxd. Original logic just sets curvalue to cl_showcaptions value.
	s_options_captions_box.itemnames = yes_no_names;

	Com_sprintf(name_yawspeed, sizeof(name_yawspeed), "\x02%s", m_item_yawspeed->string);
	s_options_yawspeed_slider.generic.type = MTYPE_SLIDER;
	s_options_yawspeed_slider.generic.x = 0;
	s_options_yawspeed_slider.generic.y = 80;
	s_options_yawspeed_slider.generic.name = name_yawspeed;
	s_options_yawspeed_slider.generic.width = re.BF_Strlen(name_yawspeed);
	s_options_yawspeed_slider.minvalue = 0.0f;
	s_options_yawspeed_slider.maxvalue = 20.0f;
	s_options_yawspeed_slider.curvalue = cl_yawspeed->value / 15.0f;
	s_options_yawspeed_slider.generic.callback = YawSpeedFunc;

	Com_sprintf(name_violence, sizeof(name_violence), "\x02%s", m_item_violence->string);
	s_options_violence_box.generic.type = MTYPE_SPINCONTROL;
	s_options_violence_box.generic.x = 0;
	s_options_violence_box.generic.y = 120;
	s_options_violence_box.generic.name = name_violence;
	s_options_violence_box.generic.width = re.BF_Strlen(name_violence);
	s_options_violence_box.generic.flags = 0;
	s_options_violence_box.generic.callback = ViolenceFunc;
	s_options_violence_box.curvalue = (int)(Cvar_VariableValue("blood_level"));
	s_options_violence_box.itemnames = violence_refs;

	Com_sprintf(name_console, sizeof(name_console), "\x02%s", m_item_console->string);
	s_options_console_action.generic.type = MTYPE_ACTION;
	s_options_console_action.generic.y = 260;
	s_options_console_action.generic.x = 0;
	s_options_console_action.generic.name = name_console;
	s_options_console_action.generic.width = re.BF_Strlen(name_console);
	s_options_console_action.generic.callback = ConsoleFunc;

	Menu_AddItem(&s_misc_menu, &s_options_crosshair_box);
	Menu_AddItem(&s_misc_menu, &s_options_alwaysrun_box);
	Menu_AddItem(&s_misc_menu, &s_options_autoweapon_box);
	Menu_AddItem(&s_misc_menu, &s_options_captions_box);
	Menu_AddItem(&s_misc_menu, &s_options_yawspeed_slider);
	Menu_AddItem(&s_misc_menu, &s_options_violence_box);
	Menu_AddItem(&s_misc_menu, &s_options_console_action);

	Misc_SetValues();
	Menu_Center(&s_misc_menu);
}

static void Misc_MenuDraw(void) // H2
{
	char title[MAX_QPATH];

	// Draw menu BG.
	Menu_DrawBG("book/back/b_conback8.bk", cls.m_menuscale);

	if (cls.m_menualpha == 0.0f)
		return;

	// Draw menu title.
	Com_sprintf(title, sizeof(title), "\x03%s", m_banner_misc->string);
	const int x = M_GetMenuLabelX(re.BF_Strlen(title));
	const int y = M_GetMenuOffsetY(&s_misc_menu);
	re.DrawBigFont(x, y, title, cls.m_menualpha);

	// Draw menu items.
	s_misc_menu.x = M_GetMenuLabelX(s_misc_menu.width);
	Menu_AdjustCursor(&s_misc_menu, 1);
	Menu_Draw(&s_misc_menu);
}

static const char* Misc_MenuKey(const int key) // H2
{
	return Default_MenuKey(&s_misc_menu, key);
}

void M_Menu_Misc_f(void) // H2
{
	Misc_MenuInit();
	M_PushMenu(Misc_MenuDraw, Misc_MenuKey);
}