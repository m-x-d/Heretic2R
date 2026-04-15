//
// menu_sound.c
//
// Copyright 1998 Raven Software
//

#include <ctype.h>
#include "menu_sound.h"
#include "client/client.h"
#include "win32/dll_io/snd_dll.h"

cvar_t* m_banner_sound;

cvar_t* m_item_sndbackend; //mxd
cvar_t* m_item_effectsvol;
cvar_t* m_item_musicvol; //mxd
cvar_t* m_item_soundquality;
cvar_t* m_item_menutrack; //mxd
cvar_t* m_item_menutrack_none; //mxd

static menuframework_t s_sound_menu;

static menulist_t s_sound_backend_list; //mxd
static menuslider_t s_sound_sfxvolume_slider;
static menuslider_t s_sound_musicvolume_slider; //mxd
static menulist_t s_sound_quality_list;
static menulist_t s_menu_track_list; //mxd

static const char* snd_list_titles[MAX_SNDLIBS]; //mxd
static int initial_sndlib_index; //mxd. snd_lib index when entering menu.

//mxd. Track names for "Menu Music" item. Track 0 is for "None" menu item.
#define MAX_MUSIC_TRACKS	32
static const char* track_list_names[MAX_MUSIC_TRACKS + 1]; // Extra slot for NULL-terminator.
static int track_numbers[MAX_MUSIC_TRACKS];

//mxd. Delay before switching to new music track.
#define MUSIC_TRACK_SWITCH_DELAY	100
static int track_last_update_time;

//mxd. Music playback state when entering the menu.
static int initial_track;
static uint initial_track_pos;
static qboolean initial_track_looping;

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
		restart_sound = !Cvar_IsSet("s_loadas8bit");
		Cvar_SetValue("s_khz", 22.0f); // H2_1.07: 11.0f -> 22.0f.
		Cvar_SetValue("s_loadas8bit", 1);
	}
	else
	{
		restart_sound = Cvar_IsSet("s_loadas8bit");
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

static void UpdateMenuMusicTrackFunc(void* self) //mxd
{
	Cvar_SetValue("m_music_track", (float)track_numbers[s_menu_track_list.curvalue]);
	track_last_update_time = curtime;
}

static int Sound_InitMusicTrackNames(void) //mxd
{
#define TRACK_FIND_FLAGS (SFF_SUBDIR | SFF_HIDDEN | SFF_SYSTEM)

	static char track_names[MAX_MUSIC_TRACKS][MAX_QPATH];

	track_list_names[0] = m_item_menutrack_none->string;
	track_numbers[0] = 0;
	int num_items = 1;

	const char* track_filename = Sys_FindFirst(va("%s/music/track??.ogg", FS_Gamedir()), 0, TRACK_FIND_FLAGS); // "music/track??.ogg"
	while (track_filename != NULL && num_items < MAX_MUSIC_TRACKS)
	{
		// Check track number.
		char* track_name = track_names[num_items];
		COM_FileBase(track_filename, track_name);
		COM_StripExtension(track_name, track_name);

		// Expect "trackXX" filename.
		if (isdigit(track_name[5]) && isdigit(track_name[6]))
		{
			track_numbers[num_items] = Q_atoi(&track_name[5]);
			track_list_names[num_items] = track_name;

			num_items++;
		}

		// Advance to next file.
		track_filename = Sys_FindNext(0, TRACK_FIND_FLAGS);
	}

	Sys_FindClose();
	track_list_names[num_items + 1] = NULL;

	return num_items;
}

static void Sound_MenuInit(void) // H2
{
	static const char* lowhigh_names[] = { m_text_low, m_text_high, NULL };

	static char name_sndbackend[MAX_QPATH]; //mxd
	static char name_effectsvol[MAX_QPATH];
	static char name_musicvol[MAX_QPATH]; //mxd
	static char name_soundquality[MAX_QPATH];
	static char name_menutrack[MAX_QPATH]; //mxd

	s_sound_menu.nitems = 0;

	snd_dll = Cvar_Get("snd_dll", DEFAULT_SOUND_LIBRARY_NAME, CVAR_ARCHIVE); //mxd. Use DEFAULT_SOUND_LIBRARY_NAME instead of "", make archiveable.

	//mxd. Sound backend library titles.
	for (int i = 0; i < num_sndlib_infos; i++)
	{
		snd_list_titles[i] = sndlib_infos[i].title;

		if (Q_stricmp(snd_dll->string, sndlib_infos[i].id) == 0)
		{
			initial_sndlib_index = i;
			s_sound_backend_list.curvalue = i;
		}
	}

	snd_list_titles[num_sndlib_infos] = NULL;

	//mxd
	Com_sprintf(name_sndbackend, sizeof(name_sndbackend), "\x02%s", m_item_sndbackend->string);
	s_sound_backend_list.generic.type = MTYPE_SPINCONTROL;
	s_sound_backend_list.generic.x = 0;
	s_sound_backend_list.generic.y = 0;
	s_sound_backend_list.generic.name = name_sndbackend;
	s_sound_backend_list.generic.width = re.BF_Strlen(name_sndbackend);
	s_sound_backend_list.curvalue = initial_sndlib_index;
	s_sound_backend_list.itemnames = snd_list_titles;

	Com_sprintf(name_effectsvol, sizeof(name_effectsvol), "\x02%s", m_item_effectsvol->string);
	s_sound_sfxvolume_slider.generic.type = MTYPE_SLIDER;
	s_sound_sfxvolume_slider.generic.x = 0;
	s_sound_sfxvolume_slider.generic.y = 40; // H2: 60
	s_sound_sfxvolume_slider.generic.name = name_effectsvol;
	s_sound_sfxvolume_slider.generic.width = re.BF_Strlen(name_effectsvol);
	s_sound_sfxvolume_slider.generic.callback = UpdateSoundVolumeFunc;
	s_sound_sfxvolume_slider.minvalue = 0.0f;
	s_sound_sfxvolume_slider.maxvalue = 10.0f;
	s_sound_sfxvolume_slider.curvalue = Cvar_VariableValue("s_volume") * 10.0f;

	//mxd
	Com_sprintf(name_musicvol, sizeof(name_musicvol), "\x02%s", m_item_musicvol->string);
	s_sound_musicvolume_slider.generic.type = MTYPE_SLIDER;
	s_sound_musicvolume_slider.generic.x = 0;
	s_sound_musicvolume_slider.generic.y = 80;
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
	s_sound_quality_list.generic.y = 120;
	s_sound_quality_list.generic.name = name_soundquality;
	s_sound_quality_list.generic.width = re.BF_Strlen(name_soundquality);
	s_sound_quality_list.generic.flags = QMF_SINGLELINE;
	s_sound_quality_list.generic.callback = UpdateSoundQualityFunc;
	s_sound_quality_list.itemnames = lowhigh_names;
	s_sound_quality_list.curvalue = !Cvar_IsSet("s_loadas8bit");

	//mxd
	Com_sprintf(name_menutrack, sizeof(name_menutrack), "\x02%s", m_item_menutrack->string);
	s_menu_track_list.generic.type = MTYPE_SPINCONTROL;
	s_menu_track_list.generic.x = 0;
	s_menu_track_list.generic.y = 140;
	s_menu_track_list.generic.name = name_menutrack;
	s_menu_track_list.generic.width = re.BF_Strlen(name_menutrack);
	s_menu_track_list.generic.flags = QMF_SINGLELINE;
	s_menu_track_list.generic.callback = UpdateMenuMusicTrackFunc;
	s_menu_track_list.itemnames = track_list_names;
	s_menu_track_list.curvalue = 0;

	track_last_update_time = -1;
	const int num_items = Sound_InitMusicTrackNames();

	if (num_items > 1) // 1-st item is the "None" item.
	{
		for (int i = 1; i < num_items; i++)
		{
			if (track_numbers[i] == (int)m_music_track->value)
			{
				s_menu_track_list.curvalue = i;
				break;
			}
		}
	}
	else
	{
		s_menu_track_list.generic.flags |= QMF_GRAYED;
	}

	//mxd. When ingame, store current music state.
	if (cls.state == ca_active)
		se.MusicGetCurrentTrackInfo(&initial_track, &initial_track_pos, &initial_track_looping);

	Menu_AddItem(&s_sound_menu, &s_sound_backend_list); //mxd
	Menu_AddItem(&s_sound_menu, &s_sound_sfxvolume_slider);
	Menu_AddItem(&s_sound_menu, &s_sound_musicvolume_slider); //mxd
	Menu_AddItem(&s_sound_menu, &s_sound_quality_list);
	Menu_AddItem(&s_sound_menu, &s_menu_track_list); //mxd
	Menu_Center(&s_sound_menu);
}

static void Sound_MenuDraw(void) // H2
{
	char title[MAX_QPATH];

	// Draw menu BG.
	Menu_DrawBG("book/back/b_conback8.bk", cls.m_menuscale);

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

	//mxd. Time to switch menu music track?
	if (track_last_update_time != -1 && curtime - track_last_update_time > MUSIC_TRACK_SWITCH_DELAY)
	{
		se.MusicPlay((int)m_music_track->value, 0, true);
		track_last_update_time = -1;
	}
}

static void Sound_PopMenu(void) //mxd
{
	// Handle sound backend switching.
	if (initial_sndlib_index != s_sound_backend_list.curvalue)
	{
		Cvar_Set("snd_dll", sndlib_infos[s_sound_backend_list.curvalue].id);
		initial_sndlib_index = s_sound_backend_list.curvalue;
		CL_Snd_Restart_f();
	}

	// When ingame, restore current music state.
	if (cls.state == ca_active)
		se.MusicPlay(initial_track, initial_track_pos, initial_track_looping);

	M_PopMenu();
}

static char* Sound_HandleMenuKey(menuframework_t* menu, const int key) // H2
{
	if (cls.m_menustate != MS_OPENED)
		return NULL;

	//mxd. Removed null menu checks.
	switch (key)
	{
		case K_TAB:
		case K_DOWNARROW:
		case K_KP_DOWNARROW:
			menu->cursor++;
			Menu_AdjustCursor(menu, 1);
			return SND_MENU_SELECT;

		case K_UPARROW:
		case K_KP_UPARROW:
			menu->cursor--;
			Menu_AdjustCursor(menu, -1);
			return SND_MENU_SELECT;

		case K_LEFTARROW:
		case K_KP_LEFTARROW:
			return (Menu_SlideItem(menu, -1) ? SND_MENU_TOGGLE : NULL);

		case K_RIGHTARROW:
		case K_KP_RIGHTARROW:
			return (Menu_SlideItem(menu, 1) ? SND_MENU_TOGGLE : NULL);

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
			Sound_PopMenu();
			return SND_MENU_CLOSE; //mxd. Added close menu sound.

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