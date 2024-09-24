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

static menuframework_s s_startserver_menu;

static menulist_s s_startmap_list;
static menulist_s s_rules_box;
static menufield_s s_timelimit_field;
static menufield_s s_fraglimit_field;
static menufield_s s_maxclients_field;
static menufield_s s_hostname_field;
static menuaction_s s_startserver_dmoptions_action;
static menuaction_s s_startserver_start_action;

static char** mapnames;

static int LoadMapList(qboolean is_coop)
{
	NOT_IMPLEMENTED
	return 0;
}

static void RulesChangeFunc(void* self)
{
	NOT_IMPLEMENTED
}

static void DMOptionsFunc(void* self)
{
	NOT_IMPLEMENTED
}

static void StartServerActionFunc(void* self)
{
	NOT_IMPLEMENTED
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

	static char* dm_coop_names[] =
	{
		name_deathmatch,
		name_coop,
		0
	};

	if (LoadMapList(Cvar_VariableValue("coop")) == 0)
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

	if ((int)Cvar_VariableValue("maxclients") == 1)
		strcpy_s(s_maxclients_field.buffer, sizeof(s_maxclients_field.buffer), "8"); //mxd. strcpy -> strcpy_s
	else
		strcpy_s(s_maxclients_field.buffer, sizeof(s_maxclients_field.buffer), Cvar_VariableString("maxclients")); //mxd. strcpy -> strcpy_s

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
	s_startserver_dmoptions_action.generic.flags = QMF_LEFT_JUSTIFY;
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
	NOT_IMPLEMENTED
}

static const char* StartServer_MenuKey(int key)
{
	NOT_IMPLEMENTED
	return NULL;
}

void M_Menu_StartServer_f(void)
{
	if (StartServer_MenuInit()) // H2
		M_PushMenu(StartServer_MenuDraw, StartServer_MenuKey);
}