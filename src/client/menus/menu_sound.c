//
// menu_sound.c
//
// Copyright 1998 Raven Software
//

#include "menu_sound.h"
#include "client.h"
#include "qcommon.h"
#include "snd_dll.h"

cvar_t* m_banner_sound;

cvar_t* m_item_effectsvol;
cvar_t* m_item_musicvol; //mxd
cvar_t* m_item_soundquality;

static menuframework_t s_sound_menu;

static menuslider_t s_sound_sfxvolume_slider;
static menuslider_t s_sound_musicvolume_slider; //mxd
static menulist_t s_sound_quality_list;

static char snd_dll_name[16];

// Q2 counterpart
static void UpdateSoundVolumeFunc(void* self)
{
	Cvar_SetValue("s_volume", s_sound_sfxvolume_slider.curvalue * 0.1f);
}

static void UpdateMusicVolumeFunc(void* self) //mxd
{
	Cvar_SetValue("m_volume", s_sound_musicvolume_slider.curvalue * 0.1f);
}

static void UpdateSoundQualityFunc(void* self)
{
	qboolean restart_sound;

	if (s_sound_quality_list.curvalue == 0)
	{
		restart_sound = (Cvar_VariableValue("s_loadas8bit") == 0.0f);
		Cvar_SetValue("s_khz", 22.0f); // H2_1.07: 11.0f -> 22.0f.
		Cvar_SetValue("s_loadas8bit", 1);
	}
	else
	{
		restart_sound = (Cvar_VariableValue("s_loadas8bit") != 0.0f);
		Cvar_SetValue("s_khz", 44.0f); // H2_1.07: 22.0f -> 44.0f.
		Cvar_SetValue("s_loadas8bit", 0);
	}

	if (restart_sound)
	{
		//mxd. Removed GM_CH_SOUND textbox drawing and 500 ms. sleep.
		se.StopAllSounds_Sounding();
		CL_Snd_Restart_f();
	}
}

static void Sound_MenuInit(void) // H2
{
	static char* lowhigh_names[] = { m_text_low, m_text_high, 0 };

	static char name_effectsvol[MAX_QPATH];
	static char name_musicvol[MAX_QPATH]; //mxd
	static char name_soundquality[MAX_QPATH];

	s_sound_menu.nitems = 0;

	snd_dll = Cvar_Get("snd_dll", DEFAULT_SOUND_LIBRARY_NAME, CVAR_ARCHIVE); //mxd. Use DEFAULT_SOUND_LIBRARY_NAME instead of "", make archiveable.
	strcpy_s(snd_dll_name, sizeof(snd_dll_name), snd_dll->string); //mxd. strcpy -> strcpy_s

	Com_sprintf(name_effectsvol, sizeof(name_effectsvol), "\x02%s", m_item_effectsvol->string);
	s_sound_sfxvolume_slider.generic.type = MTYPE_SLIDER;
	s_sound_sfxvolume_slider.generic.x = 0;
	s_sound_sfxvolume_slider.generic.y = 0; // H2: 60
	s_sound_sfxvolume_slider.generic.name = name_effectsvol;
	s_sound_sfxvolume_slider.generic.width = re.BF_Strlen(name_effectsvol);
	s_sound_sfxvolume_slider.generic.callback = UpdateSoundVolumeFunc;
	s_sound_sfxvolume_slider.minvalue = 0.0f;
	s_sound_sfxvolume_slider.maxvalue = 10.0f;
	s_sound_sfxvolume_slider.curvalue = Cvar_VariableValue("s_volume") * 10.0f;

	//mxd
	Com_sprintf(name_musicvol, sizeof(name_effectsvol), "\x02%s", m_item_musicvol->string);
	s_sound_musicvolume_slider.generic.type = MTYPE_SLIDER;
	s_sound_musicvolume_slider.generic.x = 0;
	s_sound_musicvolume_slider.generic.y = 40;
	s_sound_musicvolume_slider.generic.name = name_musicvol;
	s_sound_musicvolume_slider.generic.width = re.BF_Strlen(name_musicvol);
	s_sound_musicvolume_slider.generic.callback = UpdateMusicVolumeFunc;
	s_sound_musicvolume_slider.minvalue = 0.0f;
	s_sound_musicvolume_slider.maxvalue = 10.0f;
	s_sound_musicvolume_slider.curvalue = Cvar_VariableValue("m_volume") * 10.0f;

	//TODO: remove, always use 44 Khz sounds?
	Com_sprintf(name_soundquality, sizeof(name_soundquality), "\x02%s", m_item_soundquality->string);
	s_sound_quality_list.generic.type = MTYPE_SPINCONTROL;
	s_sound_quality_list.generic.x = 0;
	s_sound_quality_list.generic.y = 80;
	s_sound_quality_list.generic.name = name_soundquality;
	s_sound_quality_list.generic.width = re.BF_Strlen(name_soundquality);
	s_sound_quality_list.generic.flags = QMF_SINGLELINE;
	s_sound_quality_list.generic.callback = UpdateSoundQualityFunc;
	s_sound_quality_list.itemnames = lowhigh_names;
	s_sound_quality_list.curvalue = (int)(Cvar_VariableValue("s_loadas8bit") == 0.0f);

	Menu_AddItem(&s_sound_menu, &s_sound_sfxvolume_slider);
	Menu_AddItem(&s_sound_menu, &s_sound_musicvolume_slider); //mxd
	Menu_AddItem(&s_sound_menu, &s_sound_quality_list);
	Menu_Center(&s_sound_menu);
}

static void Sound_MenuDraw(void) // H2
{
	char title[MAX_QPATH];

	// Draw menu BG.
	re.BookDrawPic("book/back/b_conback8.bk", cls.m_menuscale);

	if (cls.m_menualpha == 0.0f)
		return;

	// Draw menu title.
	Com_sprintf(title, sizeof(title), "\x03%s", m_banner_sound->string);
	const int x = M_GetMenuLabelX(re.BF_Strlen(title));
	const int y = M_GetMenuOffsetY(&s_sound_menu);
	re.DrawBigFont(x, y, title, cls.m_menualpha);

	// Draw menu items.
	s_sound_menu.x = M_GetMenuLabelX(s_sound_menu.width);
	Menu_AdjustCursor(&s_sound_menu, 1);
	Menu_Draw(&s_sound_menu);
}

static char* Sound_HandleMenuKey(menuframework_t* menu, const int key) // H2
{
	if (cls.m_menustate != 2)
		return NULL;

	//mxd. Removed null menu checks.
	switch (key)
	{
		case K_TAB:
		case K_DOWNARROW:
		case K_KP_DOWNARROW:
			menu->cursor++;
			Menu_AdjustCursor(menu, 1);
			return SND_MENU2;

		case K_UPARROW:
		case K_KP_UPARROW:
			menu->cursor--;
			Menu_AdjustCursor(menu, -1);
			return SND_MENU2;

		case K_LEFTARROW:
		case K_KP_LEFTARROW:
			Menu_SlideItem(menu, -1);
			return SND_MENU4;

		case K_RIGHTARROW:
		case K_KP_RIGHTARROW:
			Menu_SlideItem(menu, 1);
			return SND_MENU4;

		case K_ENTER:
		case K_ESCAPE:
		case K_KP_ENTER:
		case K_MOUSE1:
		case K_MOUSE2:
		case K_MOUSE3:
		case K_JOY1:
		case K_JOY2:
		case K_JOY3:
		case K_JOY4:
		case K_AUX1:
		case K_AUX2:
		case K_AUX3:
		case K_AUX4:
		case K_AUX5:
		case K_AUX6:
		case K_AUX7:
		case K_AUX8:
		case K_AUX9:
		case K_AUX10:
		case K_AUX11:
		case K_AUX12:
		case K_AUX13:
		case K_AUX14:
		case K_AUX15:
		case K_AUX16:
		case K_AUX17:
		case K_AUX18:
		case K_AUX19:
		case K_AUX20:
		case K_AUX21:
		case K_AUX22:
		case K_AUX23:
		case K_AUX24:
		case K_AUX25:
		case K_AUX26:
		case K_AUX27:
		case K_AUX28:
		case K_AUX29:
		case K_AUX30:
		case K_AUX31:
		case K_AUX32:
			if (Q_stricmp(snd_dll_name, snd_dll->string) != 0)
				CL_Snd_Restart_f();
			M_PopMenu();
			return SND_MENU3; //mxd. Added close menu sound.

		default:
			break;
	}

	return NULL;
}

static const char* Sound_MenuKey(const int key) // H2
{
	return Sound_HandleMenuKey(&s_sound_menu, key);
}

void M_Menu_Sound_f(void) // H2
{
	Sound_MenuInit();
	M_PushMenu(Sound_MenuDraw, Sound_MenuKey);
}