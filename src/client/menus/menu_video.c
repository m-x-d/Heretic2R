//
// menu_video.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "vid_dll.h"
#include "menu_video.h"

#define SOFTWARE_MENU	0
#define OPENGL_MENU		1

cvar_t* m_banner_video;

cvar_t* m_item_driver;
cvar_t* m_item_vidmode;
cvar_t* m_item_screensize;
cvar_t* m_item_gamma;
cvar_t* m_item_brightness;
cvar_t* m_item_contrast;
cvar_t* m_item_detail;
cvar_t* m_item_fullscreen;

static float m_gamma;
static float m_brightness;
static float m_contrast;
static float m_detail;
static float m_picmip;
static float m_skinmip;

static menuframework_t* s_current_menu;
static menuframework_t s_opengl_menu;
static menuframework_t s_software_menu;

static menulist_t s_ref_list[2];
static menulist_t s_mode_list[2];
static menuslider_t s_screensize_slider[2];
static menuslider_t s_gamma_slider[2];
static menuslider_t s_brightness_slider[2];
static menuslider_t s_contrast_slider[2];
static menuslider_t s_detail_slider[2];
static menulist_t s_fs_box[2];
//static menulist_s s_paletted_texture_box; //mxd. Disabled

static int s_current_menu_index;

#define NUM_DRIVERNAMES			16
#define MAX_DRIVERNAME_LENGTH	20

static int gl_drivers_count;
static char gl_drivernames[NUM_DRIVERNAMES - 2][MAX_DRIVERNAME_LENGTH];
static char* gl_drivername_labels[NUM_DRIVERNAMES];

#pragma region ========================== MENU ITEM CALLBACKS ==========================

static void RendererCallback(void* self)
{
	s_ref_list[!s_current_menu_index].curvalue = s_ref_list[s_current_menu_index].curvalue;

	if (s_ref_list[s_current_menu_index].curvalue == 0)
	{
		s_current_menu = &s_software_menu;
		s_current_menu_index = SOFTWARE_MENU;
		s_detail_slider[SOFTWARE_MENU].curvalue = 1.0f; // H2
		s_mode_list[SOFTWARE_MENU].curvalue = 0; // H2
	}
	else
	{
		s_current_menu = &s_opengl_menu;
		s_current_menu_index = OPENGL_MENU;
		s_detail_slider[OPENGL_MENU].curvalue = 2.0f; // H2
	}
}

// Q2 counterpart
static void ScreenSizeCallback(void* self)
{
	const menuslider_t* slider = (menuslider_t*)self;
	Cvar_SetValue("viewsize", slider->curvalue * 10.0f);
}

static void GammaCallback(void* self) // H2
{
	const menuslider_t* slider = (menuslider_t*)self;

	if (s_current_menu_index == SOFTWARE_MENU)
		s_gamma_slider[OPENGL_MENU].curvalue = s_gamma_slider[SOFTWARE_MENU].curvalue;
	else
		s_gamma_slider[SOFTWARE_MENU].curvalue = s_gamma_slider[OPENGL_MENU].curvalue;

	Cvar_SetValue("vid_gamma", (16.0f - slider->curvalue) / 16.0f);
}

static void BrightnessCallback(void* self)
{
	const menuslider_t* slider = (menuslider_t*)self;

	if (s_current_menu_index == SOFTWARE_MENU)
		s_brightness_slider[OPENGL_MENU].curvalue = s_brightness_slider[SOFTWARE_MENU].curvalue;
	else
		s_brightness_slider[SOFTWARE_MENU].curvalue = s_brightness_slider[OPENGL_MENU].curvalue;

	Cvar_SetValue("vid_brightness", slider->curvalue / 16.0f);
}

static void ContrastCallback(void* self) // H2
{
	const menuslider_t* slider = (menuslider_t*)self;

	if (s_current_menu_index == SOFTWARE_MENU)
		s_contrast_slider[OPENGL_MENU].curvalue = s_contrast_slider[SOFTWARE_MENU].curvalue;
	else
		s_contrast_slider[SOFTWARE_MENU].curvalue = s_contrast_slider[OPENGL_MENU].curvalue;

	Cvar_SetValue("vid_contrast", slider->curvalue / 16.0f);
}

static void DetailCallback(void* self) // H2
{
	const menuslider_t* slider = (menuslider_t*)self;

	if (s_current_menu_index == SOFTWARE_MENU)
		s_detail_slider[OPENGL_MENU].curvalue = s_detail_slider[SOFTWARE_MENU].curvalue;
	else
		s_detail_slider[SOFTWARE_MENU].curvalue = s_detail_slider[OPENGL_MENU].curvalue;

	Cvar_SetValue("r_detail", slider->curvalue);
	VID_MenuSetDetail(Q_ftol(slider->curvalue));
}

static void ApplyChanges(void)
{
	if (Cvar_VariableValue("gl_picmip") != m_picmip || Cvar_VariableValue("gl_skinmip") != m_skinmip)
		vid_restart_required = true;

	// Make values consistent.
	s_fs_box[!s_current_menu_index].curvalue = s_fs_box[s_current_menu_index].curvalue;
	s_brightness_slider[!s_current_menu_index].curvalue = s_brightness_slider[s_current_menu_index].curvalue;
	s_ref_list[!s_current_menu_index].curvalue = s_ref_list[s_current_menu_index].curvalue;

	if ((int)vid_fullscreen->value != s_fs_box[s_current_menu_index].curvalue)
	{
		Cvar_SetValue("vid_fullscreen", (float)s_ref_list[s_current_menu_index].curvalue);

		if (s_fs_box[s_current_menu_index].curvalue != 0)
		{
			Cvar_SetValue("vid_xpos", 0);
			Cvar_SetValue("vid_ypos", 0);
		}

		vid_restart_required = true;
	}

	//mxd. Disabled
	/*if ((int)gl_ext_palettedtexture->value != s_paletted_texture_box.curvalue)
	{
		Cvar_SetValue("gl_ext_palettedtexture", (float)s_paletted_texture_box.curvalue);
		vid_restart_required = true;
	}*/

	if (s_ref_list[s_current_menu_index].curvalue == SOFTWARE_MENU)
	{
		Cvar_SetValue("vid_gamma", (16.0f - s_gamma_slider[SOFTWARE_MENU].curvalue) / 16.0f);
		Cvar_SetValue("vid_brightness", s_brightness_slider[SOFTWARE_MENU].curvalue / 16.0f);
		Cvar_SetValue("vid_contrast", s_contrast_slider[SOFTWARE_MENU].curvalue / 16.0f);

		if ((int)m_origmode->value != s_mode_list[SOFTWARE_MENU].curvalue)
		{
			Cvar_SetValue("m_origmode", (float)s_mode_list[SOFTWARE_MENU].curvalue);
			vid_restart_required = true;
		}

		if (Q_stricmp(vid_ref->string, "soft") != 0) // H2_1.07: "soft" -> "gl"
		{
			Cvar_Set("vid_ref", "soft"); // H2_1.07: "soft" -> "gl"
			vid_restart_required = true;
		}
	}
	else
	{
		Cvar_SetValue("vid_gamma", (16.0f - s_gamma_slider[OPENGL_MENU].curvalue) / 16.0f);
		Cvar_SetValue("vid_brightness", s_brightness_slider[OPENGL_MENU].curvalue / 16.0f);
		Cvar_SetValue("vid_contrast", s_contrast_slider[OPENGL_MENU].curvalue / 16.0f);

		if ((int)m_origmode->value != s_mode_list[OPENGL_MENU].curvalue)
		{
			Cvar_SetValue("m_origmode", (float)s_mode_list[OPENGL_MENU].curvalue);
			Cvar_SetValue("vid_mode", (float)s_mode_list[OPENGL_MENU].curvalue);
			vid_restart_required = true;
		}

		//mxd. Skip extra opengl drivers selection logic.
		if (Q_stricmp(vid_ref->string, "gl") != 0)
		{
			Cvar_Set("vid_ref", "gl");
			Cvar_Set("gl_driver", "opengl32");
			vid_restart_required = true;
		}
	}

	if (Cvar_VariableValue("vid_gamma") != m_gamma ||
		Cvar_VariableValue("vid_brightness") != m_brightness ||
		Cvar_VariableValue("vid_contrast") != m_contrast)
	{
		vid_restart_required = true;
	}

	//TODO: don't drop to console when vid restart is not needed.
	M_ForceMenuOff();
	M_UpdateOrigMode(); // H2
}

#pragma endregion

void VID_MenuSetDetail(const int detail)
{
	switch (detail)
	{
		case 0:
			Cvar_SetValue("gl_picmip", 1.0f);
			Cvar_SetValue("gl_skinmip", 2.0f);
			break;

		case 1:
			Cvar_SetValue("gl_picmip", 0.0f);
			Cvar_SetValue("gl_skinmip", 1.0f);
			break;

		default: //mxd (cases 2 and 3 in original logic)
			Cvar_SetValue("gl_picmip", 0.0f);
			Cvar_SetValue("gl_skinmip", 0.0f);
			break;
	}
}

//TODO: ancient bundled drivers lookup logic. Remove?
static void GetDriverNames(void)
{
	char buffer[MAX_QPATH];

	gl_drivername_labels[0] = "Software";
	gl_drivername_labels[1] = "OpenGL32";
	gl_drivers_count = 2;

	for (int i = 0; i < NUM_DRIVERNAMES - gl_drivers_count; i++)
	{
		Com_sprintf(buffer, sizeof(buffer), "gl_driver%d", i);
		const cvar_t* var = Cvar_Get(buffer, "", CVAR_ARCHIVE);
		const uint len = strlen(var->string);

		if (len == 0)
			break;

		if (len > MAX_DRIVERNAME_LENGTH - 1)
		{
			Com_Printf("*** ERROR : Invalid driver name %s\n", var->string);
			strcpy_s(var->string, len, "Invalid Driver");
		}

		strncpy_s(gl_drivernames[i], sizeof(gl_drivernames[i]), var->string, MAX_DRIVERNAME_LENGTH - 1);

		gl_drivername_labels[gl_drivers_count] = gl_drivernames[i];
		gl_drivers_count++;
	}

	gl_drivername_labels[gl_drivers_count] = NULL;
}

void VID_PreMenuInit(void) //TODO: this needs to be updated because of reflib interface changes...
{
	static cvar_t* gl_driver;

	GetDriverNames();

	if (gl_driver == NULL)
	{
		gl_driver = Cvar_Get("gl_driver", "opengl32", 0);
		vid_restart_required = true;
	}

	if (vid_mode == NULL)
	{
		if (Q_stricmp(vid_ref->string, "soft") == 0) // H2_1.07: "soft" -> "gl"
			vid_mode = Cvar_Get("vid_mode", "0", 0);
		else
			vid_mode = Cvar_Get("vid_mode", "3", 0);

		vid_restart_required = true;
	}

	if (vid_menu_mode == NULL)
		vid_menu_mode = Cvar_Get("vid_menu_mode", "3", 0);

	//mxd. Skip gl_ext_palettedtexture logic

	if (scr_viewsize == NULL)
		scr_viewsize = Cvar_Get("viewsize", "100", CVAR_ARCHIVE);

	if (Q_stricmp(vid_ref->string, "soft") == 0)
	{
		s_current_menu_index = SOFTWARE_MENU;
		s_ref_list[0].curvalue = 0;
		s_ref_list[1].curvalue = 0;
	}
	else //mxd. Original code does 'Q_stricmp(vid_ref->string, "gl") == 0' check here.
	{
		s_current_menu_index = OPENGL_MENU;
		s_ref_list[1].curvalue = 1;

		//TODO: remove?
		for (int i = 2; i < gl_drivers_count; i++)
		{
			if (Q_stricmp(gl_driver->string, gl_drivernames[i]) == 0)
			{
				s_ref_list[s_current_menu_index].curvalue = i;
				break;
			}
		}
	}

	if ((int)vid_fullscreen->value)
	{
		Cvar_SetValue("vid_xpos", 0.0f);
		Cvar_SetValue("vid_ypos", 0.0f);
	}

	const float detail = Cvar_VariableValue("r_detail");
	VID_MenuSetDetail((int)detail);
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
		s_ref_list[i].generic.callback = RendererCallback;
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
	s_current_menu = (s_current_menu_index ? &s_opengl_menu : &s_software_menu);

	// Draw menu BG.
	re.BookDrawPic(0, 0, "book/back/b_conback8.bk", cls.m_menuscale);

	if (cls.m_menualpha == 0.0f)
		return;

	// Draw menu title.
	Com_sprintf(title, sizeof(title), "\x03%s", m_banner_video->string);
	const int x = M_GetMenuLabelX(re.BF_Strlen(title));
	const int y = M_GetMenuOffsetY(s_current_menu);
	re.DrawBigFont(x, y, title, cls.m_menualpha);

	s_software_menu.x = M_GetMenuLabelX(s_software_menu.width);
	s_opengl_menu.x = M_GetMenuLabelX(s_opengl_menu.width);
	Menu_AdjustCursor(s_current_menu, 1);
	Menu_Draw(s_current_menu);
}

static const char* VID_MenuKey(const int key)
{
	if (cls.m_menustate != 2)
		return NULL;

	switch (key)
	{
		case K_ENTER:
		case K_KP_ENTER:
		case K_ESCAPE:
			if (!Menu_SelectItem(s_current_menu))
			{
				se.StopAllSounds_Sounding();
				ApplyChanges();
			}
			break;

		case K_UPARROW:
		case K_KP_UPARROW:
			s_current_menu->cursor--;
			Menu_AdjustCursor(s_current_menu, -1);
			return SND_MENU2;

		case K_DOWNARROW:
		case K_KP_DOWNARROW:
			s_current_menu->cursor++;
			Menu_AdjustCursor(s_current_menu, 1);
			return SND_MENU2;

		case K_LEFTARROW:
		case K_KP_LEFTARROW:
			se.StopAllSounds_Sounding();
			Menu_SlideItem(s_current_menu, -1);
			break;

		case K_RIGHTARROW:
		case K_KP_RIGHTARROW:
			se.StopAllSounds_Sounding();
			Menu_SlideItem(s_current_menu, 1);
			break;

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