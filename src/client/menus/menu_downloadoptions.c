//
// menu_downloadoptions.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "menu_downloadoptions.h"

cvar_t* m_banner_download;

cvar_t* m_item_allowdownload;
cvar_t* m_item_allowdownloadmap;
cvar_t* m_item_allowdownloadsound;
cvar_t* m_item_allowdownloadplayer;
cvar_t* m_item_allowdownloadmodel;

static menuframework_t s_downloadoptions_menu;

static menulist_t s_allow_download_box;
static menulist_t s_allow_download_maps_box;
static menulist_t s_allow_download_players_box;
static menulist_t s_allow_download_models_box;
static menulist_t s_allow_download_sounds_box;

// Q2 counterpart
static void DownloadCallback(const void* self)
{
	const menulist_t* f = self;

	if (f == &s_allow_download_box)
		Cvar_SetValue("allow_download", (float)f->curvalue);
	else if (f == &s_allow_download_maps_box)
		Cvar_SetValue("allow_download_maps", (float)f->curvalue);
	else if (f == &s_allow_download_models_box)
		Cvar_SetValue("allow_download_models", (float)f->curvalue);
	else if (f == &s_allow_download_players_box)
		Cvar_SetValue("allow_download_players", (float)f->curvalue);
	else if (f == &s_allow_download_sounds_box)
		Cvar_SetValue("allow_download_sounds", (float)f->curvalue);
}

static void DownloadOptions_SetValues(void) // H2
{
	Cvar_SetValue("allow_download", Clamp(allow_download->value, 0, 1));
	s_allow_download_box.curvalue = Q_ftol(allow_download->value);

	Cvar_SetValue("allow_download_maps", Clamp(allow_download_maps->value, 0, 1));
	s_allow_download_maps_box.curvalue = Q_ftol(allow_download_maps->value);

	Cvar_SetValue("allow_download_players", Clamp(allow_download_players->value, 0, 1));
	s_allow_download_players_box.curvalue = Q_ftol(allow_download_players->value);

	Cvar_SetValue("allow_download_models", Clamp(allow_download_models->value, 0, 1));
	s_allow_download_models_box.curvalue = Q_ftol(allow_download_models->value);

	Cvar_SetValue("allow_download_sounds", Clamp(allow_download_sounds->value, 0, 1));
	s_allow_download_sounds_box.curvalue = Q_ftol(allow_download_sounds->value);
}

static void DownloadOptions_MenuInit(void)
{
	static char name_allowdownload[MAX_QPATH];
	static char name_allowdownloadmap[MAX_QPATH];
	static char name_allowdownloadplayer[MAX_QPATH];
	static char name_allowdownloadmodel[MAX_QPATH];
	static char name_allowdownloadsound[MAX_QPATH];

	s_downloadoptions_menu.nitems = 0;
	s_downloadoptions_menu.x = viddef.width / 2;

	Com_sprintf(name_allowdownload, sizeof(name_allowdownload), "\x02%s", m_item_allowdownload->string);
	s_allow_download_box.generic.type = MTYPE_SPINCONTROL;
	s_allow_download_box.generic.x = 0;
	s_allow_download_box.generic.y = 0;
	s_allow_download_box.generic.name = name_allowdownload;
	s_allow_download_box.generic.width = re.BF_Strlen(name_allowdownload);
	s_allow_download_box.generic.flags = QMF_SINGLELINE;
	s_allow_download_box.generic.callback = DownloadCallback;
	s_allow_download_box.itemnames = yes_no_names;

	Com_sprintf(name_allowdownloadmap, sizeof(name_allowdownloadmap), "\x02%s", m_item_allowdownloadmap->string);
	s_allow_download_maps_box.generic.type = MTYPE_SPINCONTROL;
	s_allow_download_maps_box.generic.x = 0;
	s_allow_download_maps_box.generic.y = 20;
	s_allow_download_maps_box.generic.name = name_allowdownloadmap;
	s_allow_download_maps_box.generic.width = re.BF_Strlen(name_allowdownloadmap);
	s_allow_download_maps_box.generic.flags = QMF_SINGLELINE;
	s_allow_download_maps_box.generic.callback = DownloadCallback;
	s_allow_download_maps_box.itemnames = yes_no_names;

	Com_sprintf(name_allowdownloadplayer, sizeof(name_allowdownloadplayer), "\x02%s", m_item_allowdownloadplayer->string);
	s_allow_download_players_box.generic.type = MTYPE_SPINCONTROL;
	s_allow_download_players_box.generic.x = 0;
	s_allow_download_players_box.generic.y = 40;
	s_allow_download_players_box.generic.name = name_allowdownloadplayer;
	s_allow_download_players_box.generic.width = re.BF_Strlen(name_allowdownloadplayer);
	s_allow_download_players_box.generic.flags = QMF_SINGLELINE;
	s_allow_download_players_box.generic.callback = DownloadCallback;
	s_allow_download_players_box.itemnames = yes_no_names;

	Com_sprintf(name_allowdownloadmodel, sizeof(name_allowdownloadmodel), "\x02%s", m_item_allowdownloadmodel->string);
	s_allow_download_models_box.generic.type = MTYPE_SPINCONTROL;
	s_allow_download_models_box.generic.x = 0;
	s_allow_download_models_box.generic.y = 60;
	s_allow_download_models_box.generic.name = name_allowdownloadmodel;
	s_allow_download_models_box.generic.width = re.BF_Strlen(name_allowdownloadmodel);
	s_allow_download_models_box.generic.flags = QMF_SINGLELINE;
	s_allow_download_models_box.generic.callback = DownloadCallback;
	s_allow_download_models_box.itemnames = yes_no_names;

	Com_sprintf(name_allowdownloadsound, sizeof(name_allowdownloadsound), "\x02%s", m_item_allowdownloadsound->string);
	s_allow_download_sounds_box.generic.type = MTYPE_SPINCONTROL;
	s_allow_download_sounds_box.generic.x = 0;
	s_allow_download_sounds_box.generic.y = 80;
	s_allow_download_sounds_box.generic.name = name_allowdownloadsound;
	s_allow_download_sounds_box.generic.width = re.BF_Strlen(name_allowdownloadsound);
	s_allow_download_sounds_box.generic.flags = QMF_SINGLELINE;
	s_allow_download_sounds_box.generic.callback = DownloadCallback;
	s_allow_download_sounds_box.itemnames = yes_no_names;

	Menu_AddItem(&s_downloadoptions_menu, &s_allow_download_box);
	Menu_AddItem(&s_downloadoptions_menu, &s_allow_download_maps_box);
	Menu_AddItem(&s_downloadoptions_menu, &s_allow_download_players_box);
	Menu_AddItem(&s_downloadoptions_menu, &s_allow_download_models_box);
	Menu_AddItem(&s_downloadoptions_menu, &s_allow_download_sounds_box);

	//Menu_Center(&s_downloadoptions_menu); //mxd. Not needed?
	DownloadOptions_SetValues(); // H2
	Menu_Center(&s_downloadoptions_menu);
}

static void DownloadOptions_MenuDraw(void)
{
	char title[MAX_QPATH];

	// Draw menu BG.
	re.BookDrawPic("book/back/b_conback8.bk", cls.m_menuscale);

	if (cls.m_menualpha == 0.0f)
		return;

	// Draw menu title.
	Com_sprintf(title, sizeof(title), "\x03%s", m_banner_download->string);
	const int x = M_GetMenuLabelX(re.BF_Strlen(title));
	const int y = M_GetMenuOffsetY(&s_downloadoptions_menu);
	re.DrawBigFont(x, y, title, cls.m_menualpha);

	// Draw menu items.
	s_downloadoptions_menu.x = M_GetMenuLabelX(s_downloadoptions_menu.width);
	Menu_AdjustCursor(&s_downloadoptions_menu, 1);
	Menu_Draw(&s_downloadoptions_menu);
}

// Q2 counterpart
static const char* DownloadOptions_MenuKey(const int key)
{
	return Default_MenuKey(&s_downloadoptions_menu, key);
}

// Q2 counterpart
void M_Menu_DownloadOptions_f(void)
{
	DownloadOptions_MenuInit();
	M_PushMenu(DownloadOptions_MenuDraw, DownloadOptions_MenuKey);
}