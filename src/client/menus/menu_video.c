//
// menu_video.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "vid_dll.h"
#include "menu_video.h"

cvar_t* m_banner_video;

cvar_t* m_item_driver; // "Renderer"
cvar_t* m_item_vidmode; // "Video resolution"
cvar_t* m_item_gamma;
cvar_t* m_item_brightness;
cvar_t* m_item_contrast;
cvar_t* m_item_detail;

static float m_gamma;
static float m_brightness;
static float m_contrast;

static menuframework_t s_video_menu;

static menulist_t s_ref_list;
static menulist_t s_mode_list;
static menuslider_t s_gamma_slider;
static menuslider_t s_brightness_slider;
static menuslider_t s_contrast_slider;
static menuslider_t s_detail_slider;

static char* ref_list_titles[MAX_REFLIBS];
static int initial_reflib_index; // vid_ref index when entering menu.

#define MAX_DISPLAYED_VIDMODES	64 //mxd. This is kinda ugly, since vid_modes array itself is dynamically allocated... 
static char* vid_mode_titles[MAX_DISPLAYED_VIDMODES];
static int initial_vid_mode; // vid_mode when entering menu.

#pragma region ========================== MENU ITEM CALLBACKS ==========================

static void GammaCallback(void* self) // H2
{
	const menuslider_t* slider = (menuslider_t*)self;
	Cvar_SetValue("vid_gamma", (16.0f - slider->curvalue) / 16.0f);
}

static void BrightnessCallback(void* self)
{
	const menuslider_t* slider = (menuslider_t*)self;
	Cvar_SetValue("vid_brightness", slider->curvalue / 16.0f);
}

static void ContrastCallback(void* self) // H2
{
	const menuslider_t* slider = (menuslider_t*)self;
	Cvar_SetValue("vid_contrast", slider->curvalue / 16.0f);
}

static void DetailCallback(void* self) // H2
{
	const menuslider_t* slider = (menuslider_t*)self;
	Cvar_SetValue("r_detail", slider->curvalue);
}

static void ApplyChanges(const qboolean close_menu) //mxd. +close_menu arg.
{
	if (initial_vid_mode != s_mode_list.curvalue)
	{
		Cvar_SetValue("vid_mode", (float)s_mode_list.curvalue);
		initial_vid_mode = s_mode_list.curvalue;
		vid_restart_required = true;
	}

	if (initial_reflib_index != s_ref_list.curvalue)
	{
		Cvar_Set("vid_ref", reflib_infos[s_ref_list.curvalue].id);
		initial_reflib_index = s_ref_list.curvalue;
		vid_restart_required = true;
	}

	if (vid_restart_required)
	{
		M_ForceMenuOff();
		return;
	}

	if (close_menu)
	{
		//mxd. These don't require vid_restart, but we still need to update ALL textures in RI_BeginFrame() AFTER menu is closed.
		// (only it_pic/it_sky textures are updated by R_GammaAffect() when menus are open (for performance reasons)).
		if (m_gamma != vid_gamma->value || m_brightness != vid_brightness->value || m_contrast != vid_contrast->value)
			Cvar_SetValue("vid_textures_refresh_required", 1.0f);

		M_PopMenu();
	}
}

#pragma endregion

void VID_PreMenuInit(void)
{
	//mxd. Refresher library titles.
	for (int i = 0; i < num_reflib_infos; i++)
	{
		ref_list_titles[i] = reflib_infos[i].title;

		if (Q_stricmp(vid_ref->string, reflib_infos[i].id) == 0)
		{
			initial_reflib_index = i;
			s_ref_list.curvalue = i;
		}
	}

	ref_list_titles[num_reflib_infos] = NULL;

	//mxd. Window resolution labels.
	for (int i = 0; i < min(MAX_DISPLAYED_VIDMODES, num_vid_modes); i++)
		vid_mode_titles[i] = vid_modes[i].description;

	vid_mode_titles[num_vid_modes] = NULL;

	if (vid_mode == NULL)
	{
		vid_mode = Cvar_Get("vid_mode", "0", 0);
		vid_restart_required = true;
	}

	initial_vid_mode = (int)vid_mode->value; //mxd

	if (scr_viewsize == NULL)
		scr_viewsize = Cvar_Get("viewsize", "100", CVAR_ARCHIVE);
}

static void VID_MenuInit(void)
{
	static char name_driver[MAX_QPATH];
	static char name_vidmode[MAX_QPATH];
	static char name_gamma[MAX_QPATH];
	static char name_brightness[MAX_QPATH];
	static char name_contrast[MAX_QPATH];
	static char name_detail[MAX_QPATH];
	static char name_defaults[MAX_QPATH];

	VID_PreMenuInit();

	m_gamma = Cvar_VariableValue("vid_gamma");
	m_brightness = Cvar_VariableValue("vid_brightness");
	m_contrast = Cvar_VariableValue("vid_contrast");

	s_video_menu.nitems = 0;

	Cvar_SetValue("r_detail", Clamp(r_detail->value, 0.0f, 3.0f));

	Com_sprintf(name_driver, sizeof(name_driver), "\x02%s", m_item_driver->string);
	Com_sprintf(name_vidmode, sizeof(name_vidmode), "\x02%s", m_item_vidmode->string);
	Com_sprintf(name_gamma, sizeof(name_gamma), "\x02%s", m_item_gamma->string);
	Com_sprintf(name_brightness, sizeof(name_brightness), "\x02%s", m_item_brightness->string);
	Com_sprintf(name_contrast, sizeof(name_contrast), "\x02%s", m_item_contrast->string);
	Com_sprintf(name_detail, sizeof(name_detail), "\x02%s", m_item_detail->string);
	Com_sprintf(name_defaults, sizeof(name_defaults), "\x02%s", m_item_defaults->string);

	s_ref_list.generic.type = MTYPE_SPINCONTROL;
	s_ref_list.generic.x = 0;
	s_ref_list.generic.y = 0;
	s_ref_list.generic.name = name_driver;
	s_ref_list.generic.width = re.BF_Strlen(name_driver);
	s_ref_list.curvalue = initial_reflib_index;
	s_ref_list.itemnames = ref_list_titles;

	s_mode_list.generic.type = MTYPE_SPINCONTROL;
	s_mode_list.generic.x = 0;
	s_mode_list.generic.y = 40;
	s_mode_list.generic.name = name_vidmode;
	s_mode_list.generic.width = re.BF_Strlen(name_vidmode);
	s_mode_list.curvalue = initial_vid_mode;
	s_mode_list.itemnames = vid_mode_titles;

	s_gamma_slider.generic.type = MTYPE_SLIDER;
	s_gamma_slider.generic.flags = QMF_SELECT_SOUND;
	s_gamma_slider.generic.x = 0;
	s_gamma_slider.generic.y = 80;
	s_gamma_slider.generic.name = name_gamma;
	s_gamma_slider.generic.width = re.BF_Strlen(name_gamma);
	s_gamma_slider.generic.callback = GammaCallback;
	s_gamma_slider.minvalue = 0.0f;
	s_gamma_slider.maxvalue = 16.0f;
	s_gamma_slider.curvalue = 16.0f - vid_gamma->value * 16.0f;

	s_brightness_slider.generic.type = MTYPE_SLIDER;
	s_brightness_slider.generic.flags = QMF_SELECT_SOUND;
	s_brightness_slider.generic.x = 0;
	s_brightness_slider.generic.y = 120;
	s_brightness_slider.generic.name = name_brightness;
	s_brightness_slider.generic.width = re.BF_Strlen(name_brightness);
	s_brightness_slider.generic.callback = BrightnessCallback;
	s_brightness_slider.minvalue = 0.0f;
	s_brightness_slider.maxvalue = 16.0f;
	s_brightness_slider.curvalue = vid_brightness->value * 16.0f;

	s_contrast_slider.generic.type = MTYPE_SLIDER;
	s_contrast_slider.generic.flags = QMF_SELECT_SOUND;
	s_contrast_slider.generic.x = 0;
	s_contrast_slider.generic.y = 160;
	s_contrast_slider.generic.name = name_contrast;
	s_contrast_slider.generic.width = re.BF_Strlen(name_contrast);
	s_contrast_slider.generic.callback = ContrastCallback;
	s_contrast_slider.minvalue = 1.6f;
	s_contrast_slider.maxvalue = 14.4f;
	s_contrast_slider.curvalue = vid_contrast->value * 16.0f;

	s_detail_slider.generic.type = MTYPE_SLIDER;
	s_detail_slider.generic.flags = QMF_SELECT_SOUND; //mxd. QMF_SELECT_SOUND flag was missing in original version.
	s_detail_slider.generic.x = 0;
	s_detail_slider.generic.y = 200;
	s_detail_slider.generic.name = name_detail;
	s_detail_slider.generic.width = re.BF_Strlen(name_detail);
	s_detail_slider.generic.callback = DetailCallback;
	s_detail_slider.minvalue = 0.0f;
	s_detail_slider.maxvalue = 3.0f;
	s_detail_slider.curvalue = r_detail->value; //mxd. Original version used Cvar_VariableValue("r_detail") here.

	Menu_AddItem(&s_video_menu, &s_ref_list);
	Menu_AddItem(&s_video_menu, &s_mode_list);
	Menu_AddItem(&s_video_menu, &s_gamma_slider);
	Menu_AddItem(&s_video_menu, &s_brightness_slider);
	Menu_AddItem(&s_video_menu, &s_contrast_slider);
	Menu_AddItem(&s_video_menu, &s_detail_slider);

	Menu_Center(&s_video_menu);
}

static void VID_MenuDraw(void)
{
	char title[MAX_QPATH];

	// Draw menu BG.
	re.BookDrawPic("book/back/b_conback8.bk", cls.m_menuscale);

	if (cls.m_menualpha == 0.0f)
		return;

	// Draw menu title.
	Com_sprintf(title, sizeof(title), "\x03%s", m_banner_video->string);
	const int x = M_GetMenuLabelX(re.BF_Strlen(title));
	const int y = M_GetMenuOffsetY(&s_video_menu);
	re.DrawBigFont(x, y, title, cls.m_menualpha);

	s_video_menu.x = M_GetMenuLabelX(s_video_menu.width);
	Menu_AdjustCursor(&s_video_menu, 1);
	Menu_Draw(&s_video_menu);
}

static const char* VID_MenuKey(const int key)
{
	if (cls.m_menustate != 2)
		return NULL;

	switch (key)
	{
		case K_ENTER:
		case K_KP_ENTER:
			ApplyChanges(false);
			return SND_MENU1;

		case K_ESCAPE:
			ApplyChanges(true);
			return SND_MENU3;

		case K_UPARROW:
		case K_KP_UPARROW:
			s_video_menu.cursor--;
			Menu_AdjustCursor(&s_video_menu, -1);
			return SND_MENU2;

		case K_DOWNARROW:
		case K_KP_DOWNARROW:
			s_video_menu.cursor++;
			Menu_AdjustCursor(&s_video_menu, 1);
			return SND_MENU2;

		case K_LEFTARROW:
		case K_KP_LEFTARROW:
			se.StopAllSounds_Sounding();
			Menu_SlideItem(&s_video_menu, -1);
			return SND_MENU4; //mxd. Add sound.

		case K_RIGHTARROW:
		case K_KP_RIGHTARROW:
			se.StopAllSounds_Sounding();
			Menu_SlideItem(&s_video_menu, 1);
			return SND_MENU4; //mxd. Add sound.

		default:
			break;
	}

	return NULL;
}

// Q2 counterpart
void M_Menu_Video_f(void)
{
	VID_MenuInit();
	M_PushMenu(VID_MenuDraw, VID_MenuKey);
}