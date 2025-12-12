//
// menu_startserver.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "menu_startserver.h"
#include "menu_dmoptions.h"

cvar_t* m_banner_startserver;

cvar_t* m_item_begin;
cvar_t* m_item_startmap;
cvar_t* m_item_rules;
cvar_t* m_item_timelimit;
cvar_t* m_item_fraglimit;
cvar_t* m_item_maxplayers;
cvar_t* m_item_hostname;
cvar_t* m_item_deathmatch;
cvar_t* m_item_coop;

static menuframework_t s_startserver_menu;

static menulist_t s_startmap_list;
static menulist_t s_rules_box;
static menufield_t s_timelimit_field;
static menufield_t s_fraglimit_field;
static menufield_t s_maxclients_field;
static menufield_t s_hostname_field;
static menuaction_t s_startserver_dmoptions_action;
static menuaction_t s_startserver_start_action;

#define NUM_MAPNAMES	128
static char* mapnames[NUM_MAPNAMES];

//mxd. Returns the number of map names loaded.
static int LoadMapnames(const qboolean is_coop)
{
	static char mapnames_buffer[NUM_MAPNAMES][MAX_QPATH];

	for (int i = 0; i < NUM_MAPNAMES; i++)
		mapnames[i] = mapnames_buffer[i];

	char* buffer;
	const cvar_t* cv_maplist = (is_coop ? m_cooplist : m_dmlist);
	const int size = FS_LoadFile(va("%s.lst", cv_maplist->string), (void**)&buffer);
	if (size == -1)
		Com_Error(ERR_DROP, "*************************\n\t\t\t\t\t\t\t Could not open %s.lst\n\t\t\t\t\t\t\t *************************\n", cv_maplist->string);

	//mxd. We need trailing 0...
	char* maplist = Z_Malloc(size + 1);
	memcpy(maplist, buffer, size);
	maplist[size] = 0;

	FS_FreeFile(buffer);

	int num_maps = 0;
	char* s = &maplist[0];

	while (s != NULL)
	{
		char map_name[MAX_TOKEN_CHARS];
		strcpy_s(map_name, sizeof(map_name), COM_Parse(&s)); //mxd. strcpy -> strcpy_s

		char map_title[MAX_TOKEN_CHARS];
		strcpy_s(map_title, sizeof(map_title), COM_Parse(&s)); //mxd. strcpy -> strcpy_s

		if (strlen(map_name) == 0 || strlen(map_title) == 0)
			continue;

		FILE* file;
		FS_FOpenFile(va("maps/%s.bsp", map_name), &file);

		if (file == NULL)
		{
			Com_Printf("WARNING: could not find map '%s.bsp'!\n", map_name);
			continue;
		}

		FS_FCloseFile(file);
		Com_sprintf(mapnames_buffer[num_maps], sizeof(mapnames_buffer[num_maps]), "%s\n%s", map_title, map_name);
		num_maps++;

		//mxd. Add max. maps sanity check.
		if (num_maps >= NUM_MAPNAMES - 1)
		{
			Com_Printf("WARNING: maximum number of maps (%i) exceeded in %s.lst!\n", NUM_MAPNAMES - 1, cv_maplist->string);
			break;
		}
	}

	mapnames[num_maps] = NULL;
	Z_Free(maplist);

	return num_maps;
}

static void RulesChangeFunc(void* self)
{
	// When no maps, flip game mode.
	if (LoadMapnames(s_rules_box.curvalue) == 0)
		s_rules_box.curvalue ^= 1;

	s_startmap_list.curvalue = 0;

	// When coop, limit max players to 4.
	if (s_rules_box.curvalue == 1 && Q_atoi(s_maxclients_field.buffer) > 4)
		strcpy_s(s_maxclients_field.buffer, sizeof(s_maxclients_field.buffer), "4");
}

// Q2 counterpart
static void DMOptionsFunc(void* self)
{
	if (s_rules_box.curvalue != 1) //mxd. Don't show when coop.
		M_Menu_DMOptions_f();
}

static void StartServerActionFunc(void* self)
{
	char startmap[1024];
	strcpy_s(startmap, sizeof(startmap), strchr(mapnames[s_startmap_list.curvalue], '\n') + 1); //mxd. strcpy -> strcpy_s

	const int maxclients = Q_atoi(s_maxclients_field.buffer);
	const int timelimit = Q_atoi(s_timelimit_field.buffer);
	const int fraglimit = Q_atoi(s_fraglimit_field.buffer);

	Cvar_SetValue("maxclients", (float)max(0, maxclients));
	Cvar_SetValue("timelimit", (float)max(0, timelimit));
	Cvar_SetValue("fraglimit", (float)max(0, fraglimit));
	Cvar_Set("hostname", s_hostname_field.buffer);
	Cvar_SetValue("deathmatch", (float)(s_rules_box.curvalue == 0)); // H2
	Cvar_SetValue("coop", (float)(s_rules_box.curvalue == 1)); // H2
	Cvar_SetValue("paused", 0); // H2

	char* spot = NULL;

	if (s_rules_box.curvalue == 1) //mxd. When coop.
	{
		if (Q_stricmp(startmap, "dmireswamp") == 0)
			spot = "sspalace";
		else if (Q_stricmp(startmap, "andhealer") == 0)
			spot = "dmireswamp";
		else if (Q_stricmp(startmap, "kellcaves") == 0)
			spot = "andhealer";
		else if (Q_stricmp(startmap, "canyon") == 0)
			spot = "kellcaves";
		else if (Q_stricmp(startmap, "hive1") == 0)
			spot = "canyon";
		else if (Q_stricmp(startmap, "oglemine1") == 0)
			spot = "hivepriestess";
		else if (Q_stricmp(startmap, "dungeon") == 0)
			spot = "oglemine2";
		else if (Q_stricmp(startmap, "cloudhub") == 0)
			spot = "dungeon";
	}

	if (spot != NULL)
	{
		if (Com_ServerState())
			Cbuf_AddText("disconnect\n");

		Cbuf_AddText(va("gamemap \"*%s$%s\"\n", startmap, spot));
	}
	else
	{
		Cbuf_AddText(va("map %s\n", startmap));
	}

	M_ForceMenuOff();
}

static qboolean StartServer_MenuInit(void)
{
	static char name_deathmatch[MAX_QPATH];
	static char name_coop[MAX_QPATH];
	static char name_startmap[MAX_QPATH];
	static char name_rules[MAX_QPATH];
	static char name_timelimit[MAX_QPATH];
	static char name_fraglimit[MAX_QPATH];
	static char name_maxplayers[MAX_QPATH];
	static char name_hostname[MAX_QPATH];
	static char name_dmopt[MAX_QPATH];
	static char name_begin[MAX_QPATH];

	static char* dm_coop_names[] = { name_deathmatch, name_coop, 0 };

	if (LoadMapnames(Cvar_VariableValue("coop")) == 0)
		return false;

	Com_sprintf(name_deathmatch, sizeof(name_deathmatch), "\x02%s", m_item_deathmatch->string);
	Com_sprintf(name_coop, sizeof(name_coop), "\x02%s", m_item_coop->string);
	s_startserver_menu.nitems = 0;

	// Initialize the menu stuff.
	Com_sprintf(name_startmap, sizeof(name_startmap), "\x02%s", m_item_startmap->string);
	s_startmap_list.generic.type = MTYPE_SPINCONTROL;
	s_startmap_list.generic.x = 0;
	s_startmap_list.generic.y = 0;
	s_startmap_list.generic.name = name_startmap;
	s_startmap_list.generic.width = re.BF_Strlen(name_startmap);
	s_startmap_list.itemnames = mapnames;

	Com_sprintf(name_rules, sizeof(name_rules), "\x02%s", m_item_rules->string);
	s_rules_box.generic.type = MTYPE_SPINCONTROL;
	s_rules_box.generic.x = 0;
	s_rules_box.generic.y = 80;
	s_rules_box.generic.name = name_rules;
	s_rules_box.generic.width = re.BF_Strlen(name_rules);
	s_rules_box.itemnames = dm_coop_names;
	s_rules_box.curvalue = (int)(Cvar_VariableValue("coop") != 0.0f);
	s_rules_box.generic.callback = RulesChangeFunc;

	Com_sprintf(name_timelimit, sizeof(name_timelimit), "\x02%s", m_item_timelimit->string);
	s_timelimit_field.generic.type = MTYPE_FIELD;
	s_timelimit_field.generic.name = name_timelimit;
	s_timelimit_field.generic.width = re.BF_Strlen(name_timelimit);
	s_timelimit_field.generic.flags = QMF_NUMBERSONLY;
	s_timelimit_field.generic.x = 0;
	s_timelimit_field.generic.y = 120;
	s_timelimit_field.length = 4;
	s_timelimit_field.visible_length = 4;
	strcpy_s(s_timelimit_field.buffer, sizeof(s_timelimit_field.buffer), Cvar_VariableString("timelimit")); //mxd. strcpy -> strcpy_s
	s_timelimit_field.cursor = (int)strlen(s_timelimit_field.buffer);

	Com_sprintf(name_fraglimit, sizeof(name_fraglimit), "\x02%s", m_item_fraglimit->string);
	s_fraglimit_field.generic.type = MTYPE_FIELD;
	s_fraglimit_field.generic.name = name_fraglimit;
	s_fraglimit_field.generic.width = re.BF_Strlen(name_fraglimit);
	s_fraglimit_field.generic.flags = QMF_NUMBERSONLY;
	s_fraglimit_field.generic.x = 0;
	s_fraglimit_field.generic.y = 160;
	s_fraglimit_field.length = 4;
	s_fraglimit_field.visible_length = 4;
	strcpy_s(s_fraglimit_field.buffer, sizeof(s_fraglimit_field.buffer), Cvar_VariableString("fraglimit")); //mxd. strcpy -> strcpy_s
	s_fraglimit_field.cursor = (int)strlen(s_fraglimit_field.buffer);

	// maxclients determines the maximum number of players that can join the game.
	// If maxclients is only "1" then we should default the menu option to 8 players, otherwise use whatever its current value is.
	// Clamping will be done when the server is actually started.
	Com_sprintf(name_maxplayers, sizeof(name_maxplayers), "\x02%s", m_item_maxplayers->string);
	s_maxclients_field.generic.type = MTYPE_FIELD;
	s_maxclients_field.generic.name = name_maxplayers;
	s_maxclients_field.generic.width = re.BF_Strlen(name_maxplayers);
	s_maxclients_field.generic.flags = QMF_NUMBERSONLY;
	s_maxclients_field.generic.x = 0;
	s_maxclients_field.generic.y = 200;
	s_maxclients_field.length = 3;
	s_maxclients_field.visible_length = 3;
	const char* val = ((int)(Cvar_VariableValue("maxclients")) == 1 ? "8" : Cvar_VariableString("maxclients"));
	strcpy_s(s_maxclients_field.buffer, sizeof(s_maxclients_field.buffer), val); //mxd. strcpy -> strcpy_s
	s_maxclients_field.cursor = (int)strlen(s_maxclients_field.buffer);

	Com_sprintf(name_hostname, sizeof(name_hostname), "\x02%s", m_item_hostname->string);
	s_hostname_field.generic.type = MTYPE_FIELD;
	s_hostname_field.generic.name = name_hostname;
	s_hostname_field.generic.width = re.BF_Strlen(name_hostname);
	s_hostname_field.generic.flags = 0;
	s_hostname_field.generic.x = 0;
	s_hostname_field.generic.y = 240;
	s_hostname_field.length = 20;
	s_hostname_field.visible_length = 20;
	strcpy_s(s_hostname_field.buffer, sizeof(s_hostname_field.buffer), Cvar_VariableString("hostname")); //mxd. strcpy -> strcpy_s
	s_hostname_field.cursor = (int)strlen(s_hostname_field.buffer);

	Com_sprintf(name_dmopt, sizeof(name_dmopt), "\x02%s", m_banner_dmopt->string);
	s_startserver_dmoptions_action.generic.type = MTYPE_ACTION;
	s_startserver_dmoptions_action.generic.name = name_dmopt;
	s_startserver_dmoptions_action.generic.width = re.BF_Strlen(name_dmopt);
	s_startserver_dmoptions_action.generic.flags = QMF_LEFT_JUSTIFY | QMF_SELECT_SOUND; //mxd: +QMF_SELECT_SOUND.
	s_startserver_dmoptions_action.generic.x = 24;
	s_startserver_dmoptions_action.generic.y = 300;
	s_startserver_dmoptions_action.generic.callback = DMOptionsFunc;

	Com_sprintf(name_begin, sizeof(name_begin), "\x02%s", m_item_begin->string);
	s_startserver_start_action.generic.type = MTYPE_ACTION;
	s_startserver_start_action.generic.name = name_begin;
	s_startserver_start_action.generic.width = re.BF_Strlen(name_begin);
	s_startserver_start_action.generic.flags = QMF_LEFT_JUSTIFY | QMF_SELECT_SOUND;
	s_startserver_start_action.generic.x = 24;
	s_startserver_start_action.generic.y = 320;
	s_startserver_start_action.generic.callback = StartServerActionFunc;

	Menu_AddItem(&s_startserver_menu, &s_startmap_list);
	Menu_AddItem(&s_startserver_menu, &s_rules_box);
	Menu_AddItem(&s_startserver_menu, &s_timelimit_field);
	Menu_AddItem(&s_startserver_menu, &s_fraglimit_field);
	Menu_AddItem(&s_startserver_menu, &s_maxclients_field);
	Menu_AddItem(&s_startserver_menu, &s_hostname_field);
	Menu_AddItem(&s_startserver_menu, &s_startserver_dmoptions_action);
	Menu_AddItem(&s_startserver_menu, &s_startserver_start_action);

	Menu_Center(&s_startserver_menu);

	// Call this now to set proper initial state.
	RulesChangeFunc(NULL);

	return true;
}

static void StartServer_MenuDraw(void)
{
	char title[MAX_QPATH];

	// Draw menu BG.
	Menu_DrawBG("book/back/b_conback8.bk", cls.m_menuscale);

	if (cls.m_menualpha == 0.0f)
		return;

	// Draw menu title.
	Com_sprintf(title, sizeof(title), "\x03%s", m_banner_startserver->string);
	const int x = M_GetMenuLabelX(re.BF_Strlen(title));
	const int y = M_GetMenuOffsetY(&s_startserver_menu);
	re.DrawBigFont(x, y, title, cls.m_menualpha);

	// Draw menu items.
	s_startserver_menu.x = M_GetMenuLabelX(s_startserver_menu.width);
	Menu_Draw(&s_startserver_menu);
}

static const char* StartServer_MenuKey(const int key)
{
	if (cls.m_menustate != MS_OPENED)
		return NULL;

	return Default_MenuKey(&s_startserver_menu, key);
}

void M_Menu_StartServer_f(void)
{
	if (StartServer_MenuInit()) // H2
		M_PushMenu(StartServer_MenuDraw, StartServer_MenuKey);
}