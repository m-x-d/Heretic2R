//
// menu_video.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "menu_video.h"

#include "vid_dll.h"

#define SOFTWARE_MENU	0
#define OPENGL_MENU		1

cvar_t* m_banner_video;

static float m_gamma;
static float m_brightness;
static float m_contrast;
static float m_detail;
static float m_picmip;
static float m_skinmip;

static menuframework_s* s_vid_menu;
static menuframework_s s_opengl_menu;
static menuframework_s s_software_menu;

static menulist_s s_ref_list[2];
static menulist_s s_mode_list[2];
static menuslider_s s_screensize_slider[2];
static menuslider_s s_gamma_slider[2];
static menuslider_s s_brightness_slider[2];
static menuslider_s s_contrast_slider[2];
static menuslider_s s_detail_slider[2];
static menulist_s s_fs_box[2];
//static menulist_s s_paletted_texture_box; //mxd. Disabled

static void DriverCallback(void* self)
{
	NOT_IMPLEMENTED
}

static void ScreenSizeCallback(void* self)
{
	NOT_IMPLEMENTED
}

static void GammaCallback(void* self)
{
	NOT_IMPLEMENTED
}

static void BrightnessCallback(void* self)
{
	NOT_IMPLEMENTED
}

static void ContrastCallback(void* self)
{
	NOT_IMPLEMENTED
}

static void DetailCallback(void* self)
{
	NOT_IMPLEMENTED
}

void VID_MenuInit(void)
{
	static char* resolutions[] =
	{
		"320 240  ",
		"400 300  ",
		"512 384  ",
		"640 480  ",
		"800 600  ",
		"960 720  ",
		"1024 768 ",
		"1152 864 ",
		"1280 960 ",
		"1600 1200", //mxd. 2048x1536 resolution is missing in H2.
		0
	};

	static char name_driver[MAX_QPATH];
	static char name_vidmode[MAX_QPATH];
	static char name_screensize[MAX_QPATH];
	static char name_gamma[MAX_QPATH];
	static char name_brightness[MAX_QPATH];
	static char name_contrast[MAX_QPATH];
	static char name_detail[MAX_QPATH];
	static char name_fullscreen[MAX_QPATH];
	//static char name_palettedtextures[MAX_QPATH]; //mxd. Disabled
	static char name_defaults[MAX_QPATH];

	VID_PreMenuInit();

	m_gamma = Cvar_VariableValue("vid_gamma");
	m_brightness = Cvar_VariableValue("vid_brightness");
	m_contrast = Cvar_VariableValue("vid_contrast");
	m_detail = Cvar_VariableValue("r_detail");
	m_picmip = Cvar_VariableValue("gl_picmip");
	m_skinmip = Cvar_VariableValue("gl_skinmip");

	s_software_menu.nitems = 0;
	s_opengl_menu.nitems = 0;

	Cvar_SetValue("r_detail", Clamp(r_detail->value, 0, 3));

	Com_sprintf(name_driver, sizeof(name_driver), "\x02%s", m_item_driver->string);
	Com_sprintf(name_vidmode, sizeof(name_vidmode), "\x02%s", m_item_vidmode->string);
	Com_sprintf(name_screensize, sizeof(name_screensize), "\x02%s", m_item_screensize->string);
	Com_sprintf(name_gamma, sizeof(name_gamma), "\x02%s", m_item_gamma->string);
	Com_sprintf(name_brightness, sizeof(name_brightness), "\x02%s", m_item_brightness->string);
	Com_sprintf(name_contrast, sizeof(name_contrast), "\x02%s", m_item_contrast->string);
	Com_sprintf(name_detail, sizeof(name_detail), "\x02%s", m_item_detail->string);
	Com_sprintf(name_fullscreen, sizeof(name_fullscreen), "\x02%s", m_item_fullscreen->string);
	//Com_sprintf(name_palettedtextures, sizeof(name_palettedtextures), "\x02%s", m_item_palettedtextures->string); //mxd. Disabled
	Com_sprintf(name_defaults, sizeof(name_defaults), "\x02%s", m_item_defaults->string);

	for (int i = 0; i < 2; i++)
	{
		s_ref_list[i].generic.type = MTYPE_SPINCONTROL;
		s_ref_list[i].generic.x = 0;
		s_ref_list[i].generic.y = 0;
		s_ref_list[i].generic.name = name_driver;
		s_ref_list[i].generic.width = re.BF_Strlen(name_driver);
		s_ref_list[i].generic.callback = DriverCallback;
		s_ref_list[i].itemnames = gl_drivername_labels;

		s_mode_list[i].generic.type = MTYPE_SPINCONTROL;
		s_mode_list[i].generic.x = 0;
		s_mode_list[i].generic.y = 40;
		s_mode_list[i].generic.name = name_vidmode;
		s_mode_list[i].generic.width = re.BF_Strlen(name_vidmode);
		s_mode_list[i].curvalue = Q_ftol(m_origmode->value);
		s_mode_list[i].itemnames = resolutions;

		s_screensize_slider[i].generic.type = MTYPE_SLIDER;
		s_screensize_slider[i].generic.x = 0;
		s_screensize_slider[i].generic.y = 80;
		s_screensize_slider[i].generic.name = name_screensize;
		s_screensize_slider[i].generic.width = re.BF_Strlen(name_screensize);
		s_screensize_slider[i].generic.callback = ScreenSizeCallback;
		s_screensize_slider[i].minvalue = 3.0f;
		s_screensize_slider[i].maxvalue = 12.0f;
		s_screensize_slider[i].curvalue = scr_viewsize->value * 0.1f;

		s_gamma_slider[i].generic.type = MTYPE_SLIDER;
		s_gamma_slider[i].generic.flags = QMF_SELECT_SOUND;
		s_gamma_slider[i].generic.x = 0;
		s_gamma_slider[i].generic.y = 120;
		s_gamma_slider[i].generic.name = name_gamma;
		s_gamma_slider[i].generic.width = re.BF_Strlen(name_gamma);
		s_gamma_slider[i].generic.callback = GammaCallback;
		s_gamma_slider[i].minvalue = 0.0f;
		s_gamma_slider[i].maxvalue = 16.0f;
		s_gamma_slider[i].curvalue = 16.0f - m_gamma * 16.0f; //mxd. Original version used vid_gamma cvar here.

		s_brightness_slider[i].generic.type = MTYPE_SLIDER;
		s_brightness_slider[i].generic.flags = QMF_SELECT_SOUND;
		s_brightness_slider[i].generic.x = 0;
		s_brightness_slider[i].generic.y = 160;
		s_brightness_slider[i].generic.name = name_brightness;
		s_brightness_slider[i].generic.width = re.BF_Strlen(name_brightness);
		s_brightness_slider[i].generic.callback = BrightnessCallback;
		s_brightness_slider[i].minvalue = 0.0f;
		s_brightness_slider[i].maxvalue = 16.0f;
		s_brightness_slider[i].curvalue = m_brightness * 16.0f; //mxd. Original version used vid_brightness cvar here.

		s_contrast_slider[i].generic.type = MTYPE_SLIDER;
		s_contrast_slider[i].generic.flags = QMF_SELECT_SOUND;
		s_contrast_slider[i].generic.x = 0;
		s_contrast_slider[i].generic.y = 200;
		s_contrast_slider[i].generic.name = name_contrast;
		s_contrast_slider[i].generic.width = re.BF_Strlen(name_contrast);
		s_contrast_slider[i].generic.callback = ContrastCallback;
		s_contrast_slider[i].minvalue = 1.6f;
		s_contrast_slider[i].maxvalue = 14.4f;
		s_contrast_slider[i].curvalue = m_contrast * 16.0f; //mxd. Original version used vid_contrast cvar here.

		s_detail_slider[i].generic.type = MTYPE_SLIDER;
		s_detail_slider[i].generic.flags = QMF_SELECT_SOUND; //mxd. QMF_SELECT_SOUND flag was missing in original version.
		s_detail_slider[i].generic.x = 0;
		s_detail_slider[i].generic.y = 240;
		s_detail_slider[i].generic.name = name_detail;
		s_detail_slider[i].generic.width = re.BF_Strlen(name_detail);
		s_detail_slider[i].generic.callback = DetailCallback;
		s_detail_slider[i].minvalue = 0.0f;
		s_detail_slider[i].maxvalue = 3.0f;
		s_detail_slider[i].curvalue = m_detail; //mxd. Original version used Cvar_VariableValue("r_detail") here.

		s_fs_box[i].generic.type = MTYPE_SPINCONTROL;
		s_fs_box[i].generic.flags = QMF_SINGLELINE;
		s_fs_box[i].generic.x = 0;
		s_fs_box[i].generic.y = 280; // 300 in original version.
		s_fs_box[i].generic.name = name_fullscreen;
		s_fs_box[i].generic.width = re.BF_Strlen(name_fullscreen);
		s_fs_box[i].curvalue = Q_ftol(vid_fullscreen->value);
		s_fs_box[i].itemnames = yes_no_names;
	}

	//mxd. Disabled.
	/*s_paletted_texture_box.generic.type = MTYPE_SPINCONTROL;
	s_paletted_texture_box.generic.x = 0;
	s_paletted_texture_box.generic.y = 280;
	s_paletted_texture_box.generic.name = name_palettedtextures;
	s_paletted_texture_box.generic.width = re.BF_Strlen(name_palettedtextures);
	s_paletted_texture_box.generic.flags = QMF_SINGLELINE;
	s_paletted_texture_box.itemnames = yes_no_names;
	s_paletted_texture_box.curvalue = Q_ftol(gl_ext_palettedtexture->value);*/

	Menu_AddItem(&s_software_menu, &s_ref_list[SOFTWARE_MENU]);
	Menu_AddItem(&s_software_menu, &s_mode_list[SOFTWARE_MENU]);
	Menu_AddItem(&s_software_menu, &s_screensize_slider[SOFTWARE_MENU]);
	Menu_AddItem(&s_software_menu, &s_gamma_slider[SOFTWARE_MENU]);
	Menu_AddItem(&s_software_menu, &s_brightness_slider[SOFTWARE_MENU]);
	Menu_AddItem(&s_software_menu, &s_contrast_slider[SOFTWARE_MENU]);
	Menu_AddItem(&s_software_menu, &s_detail_slider[SOFTWARE_MENU]);
	Menu_AddItem(&s_software_menu, &s_fs_box[SOFTWARE_MENU]);

	Menu_AddItem(&s_opengl_menu, &s_ref_list[OPENGL_MENU]);
	Menu_AddItem(&s_opengl_menu, &s_mode_list[OPENGL_MENU]);
	Menu_AddItem(&s_opengl_menu, &s_screensize_slider[OPENGL_MENU]);
	Menu_AddItem(&s_opengl_menu, &s_gamma_slider[OPENGL_MENU]);
	Menu_AddItem(&s_opengl_menu, &s_brightness_slider[OPENGL_MENU]);
	Menu_AddItem(&s_opengl_menu, &s_contrast_slider[OPENGL_MENU]);
	Menu_AddItem(&s_opengl_menu, &s_detail_slider[OPENGL_MENU]);
	//Menu_AddItem(&s_opengl_menu, &s_paletted_texture_box); //mxd. Disabled.
	Menu_AddItem(&s_opengl_menu, &s_fs_box[OPENGL_MENU]);

	Menu_Center(&s_software_menu);
	Menu_Center(&s_opengl_menu);
}

static void VID_MenuDraw(void)
{
	char title[MAX_QPATH];

	// Set pointer to current menu.
	s_vid_menu = (s_current_menu_index ? &s_opengl_menu : &s_software_menu);

	// Draw menu BG.
	re.BookDrawPic(0, 0, "book/back/b_conback8.bk", cls.m_menuscale);

	if (cls.m_menualpha == 0.0f)
		return;

	// Draw menu title.
	Com_sprintf(title, sizeof(title), "\x03%s", m_banner_video->string);
	const int x = M_GetMenuLabelX(re.BF_Strlen(title));
	const int y = M_GetMenuOffsetY(s_vid_menu);
	re.DrawBigFont(x, y, title, cls.m_menualpha);

	s_software_menu.x = M_GetMenuLabelX(s_software_menu.width);
	s_opengl_menu.x = M_GetMenuLabelX(s_opengl_menu.width);
	Menu_AdjustCursor(s_vid_menu, 1);
	Menu_Draw(s_vid_menu);
}

static const char* VID_MenuKey(int key)
{
	NOT_IMPLEMENTED
	return NULL;
}

// Q2 counterpart
void M_Menu_Video_f(void)
{
	VID_MenuInit();
	M_PushMenu(VID_MenuDraw, VID_MenuKey);
}