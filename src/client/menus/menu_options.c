//
// menu_options.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "menu_options.h"
#include "menu_actionkeys.h"
#include "menu_cameracfg.h"
#include "menu_doubletapkeys.h"
#include "menu_loadcfg.h"
#include "menu_misc.h"
#include "menu_mousecfg.h"
#include "menu_movekeys.h"
#include "menu_shortkeys.h"

cvar_t* m_banner_options;
cvar_t* m_banner_savecfg;

static menuframework_t s_options_menu;

static menuaction_t s_action_keys_action;
static menuaction_t s_move_keys_action;
static menuaction_t s_short_keys_action;
static menuaction_t s_dt_keys_action;
static menuaction_t s_mousecfg_action;
static menuaction_t s_cameracfg_action;
static menuaction_t s_misc_action;
static menuaction_t s_loadcfg_action;
static menuaction_t s_savecfg_action;
static menuaction_t s_defaults_action;

#define SAVECFG_DISPLAY_TIME	4000 //mxd. Display for 4 seconds.
#define SAVECFG_FADEOUT_TIME	1000.0f //mxd. Fade-out over 1 second.

static int savecfg_msg_display_time; //mxd
static char savecfg_path[256]; //mxd

static void ActionKeysFunc(void* self)
{
	M_Menu_ActionKeys_f();
}

static void MoveKeysFunc(void* self)
{
	M_Menu_MoveKeys_f();
}

static void ShortKeysFunc(void* self)
{
	M_Menu_ShortKeys_f();
}

static void DoubletapKeysFunc(void* self)
{
	M_Menu_DoubletapKeys_f();
}

static void MouseConfigFunc(void* self)
{
	M_Menu_MouseCfg_f();
}

static void CameraConfigFunc(void* self)
{
	M_Menu_CameraCfg_f();
}

static void MiscFunc(void* self)
{
	M_Menu_Misc_f();
}

static void LoadConfigFunc(void* self)
{
	M_Menu_LoadCfg_f();
}

static void SaveConfigFunc(void* self) // H2
{
	CL_SaveConfig_f();

	//mxd. Display save config message in non-blocking fashion.
	savecfg_msg_display_time = curtime + SAVECFG_DISPLAY_TIME;
	sprintf_s(savecfg_path, sizeof(savecfg_path), "'%s'.", CL_GetConfigPath());
}

static void ControlsResetToDefaultsFunc(void* self)
{
	Cbuf_AddText("exec default.cfg\n");
	Cbuf_Execute();
}

static void Options_MenuInit(void)
{
	static char name_action_keys[MAX_QPATH];
	static char name_move_keys[MAX_QPATH];
	static char name_short_keys[MAX_QPATH];
	static char name_dt_keys[MAX_QPATH];
	static char name_mousecfg[MAX_QPATH];
	static char name_cameracfg[MAX_QPATH];
	static char name_misc[MAX_QPATH];
	static char name_loadcfg[MAX_QPATH];
	static char name_savecfg[MAX_QPATH];
	static char name_defaults[MAX_QPATH];

	s_options_menu.nitems = 0;

	Com_sprintf(name_action_keys, sizeof(name_action_keys), "\x02%s", m_banner_action_keys->string);
	s_action_keys_action.generic.type = MTYPE_ACTION;
	s_action_keys_action.generic.flags = QMF_SELECT_SOUND; //mxd
	s_action_keys_action.generic.x = 0;
	s_action_keys_action.generic.y = 0;
	s_action_keys_action.generic.name = name_action_keys;
	s_action_keys_action.generic.width = re.BF_Strlen(name_action_keys);
	s_action_keys_action.generic.callback = ActionKeysFunc;

	Com_sprintf(name_move_keys, sizeof(name_move_keys), "\x02%s", m_banner_move_keys->string);
	s_move_keys_action.generic.type = MTYPE_ACTION;
	s_move_keys_action.generic.flags = QMF_SELECT_SOUND; //mxd
	s_move_keys_action.generic.x = 0;
	s_move_keys_action.generic.y = 20;
	s_move_keys_action.generic.name = name_move_keys;
	s_move_keys_action.generic.width = re.BF_Strlen(name_move_keys);
	s_move_keys_action.generic.callback = MoveKeysFunc;

	Com_sprintf(name_short_keys, sizeof(name_short_keys), "\x02%s", m_banner_short_keys->string);
	s_short_keys_action.generic.type = MTYPE_ACTION;
	s_short_keys_action.generic.flags = QMF_SELECT_SOUND; //mxd
	s_short_keys_action.generic.x = 0;
	s_short_keys_action.generic.y = 40;
	s_short_keys_action.generic.name = name_short_keys;
	s_short_keys_action.generic.width = re.BF_Strlen(name_short_keys);
	s_short_keys_action.generic.callback = ShortKeysFunc;

	Com_sprintf(name_dt_keys, sizeof(name_dt_keys), "\x02%s", m_banner_dt_keys->string);
	s_dt_keys_action.generic.type = MTYPE_ACTION;
	s_dt_keys_action.generic.flags = QMF_SELECT_SOUND; //mxd
	s_dt_keys_action.generic.x = 0;
	s_dt_keys_action.generic.y = 60;
	s_dt_keys_action.generic.name = name_dt_keys;
	s_dt_keys_action.generic.width = re.BF_Strlen(name_dt_keys);
	s_dt_keys_action.generic.callback = DoubletapKeysFunc;

	Com_sprintf(name_mousecfg, sizeof(name_mousecfg), "\x02%s", m_banner_mousecfg->string);
	s_mousecfg_action.generic.type = MTYPE_ACTION;
	s_mousecfg_action.generic.flags = QMF_SELECT_SOUND; //mxd
	s_mousecfg_action.generic.x = 0;
	s_mousecfg_action.generic.y = 80;
	s_mousecfg_action.generic.name = name_mousecfg;
	s_mousecfg_action.generic.width = re.BF_Strlen(name_mousecfg);
	s_mousecfg_action.generic.callback = MouseConfigFunc;

	Com_sprintf(name_cameracfg, sizeof(name_cameracfg), "\x02%s", m_banner_cameracfg->string);
	s_cameracfg_action.generic.type = MTYPE_ACTION;
	s_cameracfg_action.generic.flags = QMF_SELECT_SOUND; //mxd
	s_cameracfg_action.generic.x = 0;
	s_cameracfg_action.generic.y = 100;
	s_cameracfg_action.generic.name = name_cameracfg;
	s_cameracfg_action.generic.width = re.BF_Strlen(name_cameracfg);
	s_cameracfg_action.generic.callback = CameraConfigFunc;

	Com_sprintf(name_misc, sizeof(name_misc), "\x02%s", m_banner_misc->string);
	s_misc_action.generic.type = MTYPE_ACTION;
	s_misc_action.generic.flags = QMF_SELECT_SOUND; //mxd
	s_misc_action.generic.x = 0;
	s_misc_action.generic.y = 120;
	s_misc_action.generic.name = name_misc;
	s_misc_action.generic.width = re.BF_Strlen(name_misc);
	s_misc_action.generic.callback = MiscFunc;

	Com_sprintf(name_loadcfg, sizeof(name_loadcfg), "\x02%s", m_banner_loadcfg->string);
	s_loadcfg_action.generic.type = MTYPE_ACTION;
	s_loadcfg_action.generic.flags = QMF_SELECT_SOUND; //mxd
	s_loadcfg_action.generic.x = 0;
	s_loadcfg_action.generic.y = 180;
	s_loadcfg_action.generic.name = name_loadcfg;
	s_loadcfg_action.generic.width = re.BF_Strlen(name_loadcfg);
	s_loadcfg_action.generic.callback = LoadConfigFunc;

	Com_sprintf(name_savecfg, sizeof(name_savecfg), "\x02%s", m_banner_savecfg->string);
	s_savecfg_action.generic.type = MTYPE_ACTION;
	s_savecfg_action.generic.flags = QMF_SELECT_SOUND; //mxd
	s_savecfg_action.generic.x = 0;
	s_savecfg_action.generic.y = 200;
	s_savecfg_action.generic.name = name_savecfg;
	s_savecfg_action.generic.width = re.BF_Strlen(name_savecfg);
	s_savecfg_action.generic.callback = SaveConfigFunc;

	Com_sprintf(name_defaults, sizeof(name_defaults), "\x02%s", m_item_defaults->string);
	s_defaults_action.generic.type = MTYPE_ACTION;
	s_defaults_action.generic.flags = QMF_SELECT_SOUND; //mxd
	s_defaults_action.generic.x = 0;
	s_defaults_action.generic.y = 220;
	s_defaults_action.generic.name = name_defaults;
	s_defaults_action.generic.width = re.BF_Strlen(name_defaults);
	s_defaults_action.generic.callback = ControlsResetToDefaultsFunc;

	Menu_AddItem(&s_options_menu, &s_action_keys_action);
	Menu_AddItem(&s_options_menu, &s_move_keys_action);
	Menu_AddItem(&s_options_menu, &s_short_keys_action);
	Menu_AddItem(&s_options_menu, &s_dt_keys_action);
	Menu_AddItem(&s_options_menu, &s_mousecfg_action);
	Menu_AddItem(&s_options_menu, &s_cameracfg_action);
	Menu_AddItem(&s_options_menu, &s_misc_action);
	Menu_AddItem(&s_options_menu, &s_loadcfg_action);
	Menu_AddItem(&s_options_menu, &s_savecfg_action);
	Menu_AddItem(&s_options_menu, &s_defaults_action);

	Menu_Center(&s_options_menu);
}

static void Options_MenuDraw(void)
{
	char title[MAX_QPATH];

	// Draw menu BG.
	Menu_DrawBG("book/back/b_conback8.bk", cls.m_menuscale);

	if (cls.m_menualpha == 0.0f)
		return;

	// Draw menu title.
	Com_sprintf(title, sizeof(title), "\x03%s", m_banner_options->string);
	const int x = M_GetMenuLabelX(re.BF_Strlen(title));
	const int y = M_GetMenuOffsetY(&s_options_menu);
	re.DrawBigFont(x, y, title, cls.m_menualpha);

	// Draw menu items.
	s_options_menu.x = M_GetMenuLabelX(s_options_menu.width);
	Menu_AdjustCursor(&s_options_menu, 1);
	Menu_Draw(&s_options_menu);

	//mxd. Draw "saved config" message?
	if (savecfg_msg_display_time > curtime)
	{
		const float delta = 1.0f - min(SAVECFG_FADEOUT_TIME, (float)(savecfg_msg_display_time - curtime)) / SAVECFG_FADEOUT_TIME;

		paletteRGBA_t color = TextPalette[P_YELLOW];
		color.a = (byte)(sinf((M_PI + M_PI * delta) * 0.5f) * 255.0f); // Ease-in fade-out over last second.

		const char* info = "Saved configuration to";
		int msg_len = (int)strlen(info);
		DrawString((viddef.width - msg_len * ui_char_size) / 2, viddef.height - ui_line_height * 9, info, color, msg_len);

		msg_len = (int)strlen(savecfg_path);
		DrawString((viddef.width - msg_len * ui_char_size) / 2, viddef.height - ui_line_height * 8, savecfg_path, color, msg_len);
	}
}

// Q2 counterpart
static const char* Options_MenuKey(const int key)
{
	return Default_MenuKey(&s_options_menu, key);
}

// Q2 counterpart
void M_Menu_Options_f(void)
{
	Options_MenuInit();
	M_PushMenu(Options_MenuDraw, Options_MenuKey);
}