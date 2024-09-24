//
// menu_dmoptions.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "menu_dmoptions.h"

cvar_t* m_banner_dmopt;

cvar_t* m_item_weaponsstay;
cvar_t* m_item_allowpowerup;
cvar_t* m_item_shrinechaos;
cvar_t* m_item_allowhealth;
cvar_t* m_item_leader;
cvar_t* m_item_offensive_spell;
cvar_t* m_item_defensive_spell;
cvar_t* m_item_samemap;
cvar_t* m_item_forcerespawn;
cvar_t* m_item_teamplay;
cvar_t* m_item_allowexit;
cvar_t* m_item_infinitemana;
cvar_t* m_item_friendlyfire;
cvar_t* m_item_dismember;
cvar_t* m_item_nonames;

static menuframework_s s_dmoptions_menu;

static menulist_s s_weapons_stay_box;
static menulist_s s_powerups_box;
static menulist_s s_shrines_box;
static menulist_s s_health_box;
static menulist_s s_offensive_spell_box;
static menulist_s s_defensive_spell_box;
static menulist_s s_infinitemana_box;
static menulist_s s_nonames_box;
static menulist_s s_force_respawn_box;
static menulist_s s_allow_exit_box;
static menulist_s s_samelevel_box;
static menulist_s s_show_leader_box;
static menulist_s s_teamplay_box;
static menulist_s s_friendlyfire_box;
static menulist_s s_allow_dismemberment_box;

static void DMFlagCallback(void* self)
{
	NOT_IMPLEMENTED
}

static void DMOptions_MenuInit(void)
{
	static char* teamplay_names[] = { "disabled", "by skin", "by model", 0 };

	static char name_weaponsstay[MAX_QPATH];
	static char name_allowpowerup[MAX_QPATH];
	static char name_shrinechaos[MAX_QPATH];
	static char name_allowhealth[MAX_QPATH];
	static char name_offensive_spell[MAX_QPATH];
	static char name_defensive_spell[MAX_QPATH];
	static char name_infinitemana[MAX_QPATH];
	static char name_nonames[MAX_QPATH];
	static char name_forcerespawn[MAX_QPATH];
	static char name_allowexit[MAX_QPATH];
	static char name_samemap[MAX_QPATH];
	static char name_leader[MAX_QPATH];
	static char name_teamplay[MAX_QPATH];
	static char name_friendlyfire[MAX_QPATH];
	static char name_dismember[MAX_QPATH];

	const int dm_flags = Q_ftol(Cvar_VariableValue("dmflags"));
	s_dmoptions_menu.nitems = 0;

	Com_sprintf(name_weaponsstay, sizeof(name_weaponsstay), "\x02%s", m_item_weaponsstay->string);
	s_weapons_stay_box.generic.type = MTYPE_SPINCONTROL;
	s_weapons_stay_box.generic.x = 0;
	s_weapons_stay_box.generic.y = 0;
	s_weapons_stay_box.generic.name = name_weaponsstay;
	s_weapons_stay_box.generic.width = re.BF_Strlen(name_weaponsstay);
	s_weapons_stay_box.generic.flags = QMF_SINGLELINE;
	s_weapons_stay_box.generic.callback = DMFlagCallback;
	s_weapons_stay_box.curvalue = (dm_flags & DF_WEAPONS_STAY);
	s_weapons_stay_box.itemnames = yes_no_names;

	Com_sprintf(name_allowpowerup, sizeof(name_allowpowerup), "\x02%s", m_item_allowpowerup->string);
	s_powerups_box.generic.type = MTYPE_SPINCONTROL;
	s_powerups_box.generic.x = 0;
	s_powerups_box.generic.y = 20;
	s_powerups_box.generic.name = name_allowpowerup;
	s_powerups_box.generic.width = re.BF_Strlen(name_allowpowerup);
	s_powerups_box.generic.callback = DMFlagCallback;
	s_powerups_box.generic.flags = QMF_SINGLELINE;
	s_powerups_box.curvalue = ((dm_flags & DF_NO_SHRINE) == 0); //mxd. Inverted value!
	s_powerups_box.itemnames = yes_no_names;

	Com_sprintf(name_shrinechaos, sizeof(name_shrinechaos), "\x02%s", m_item_shrinechaos->string);
	s_shrines_box.generic.type = MTYPE_SPINCONTROL;
	s_shrines_box.generic.x = 0;
	s_shrines_box.generic.y = 40;
	s_shrines_box.generic.name = name_shrinechaos;
	s_shrines_box.generic.width = re.BF_Strlen(name_shrinechaos);
	s_shrines_box.generic.flags = QMF_SINGLELINE;
	s_shrines_box.generic.callback = DMFlagCallback;
	s_shrines_box.curvalue = (dm_flags & DF_SHRINE_CHAOS);
	s_shrines_box.itemnames = yes_no_names;

	Com_sprintf(name_allowhealth, sizeof(name_allowhealth), "\x02%s", m_item_allowhealth->string);
	s_health_box.generic.type = MTYPE_SPINCONTROL;
	s_health_box.generic.x = 0;
	s_health_box.generic.y = 60;
	s_health_box.generic.callback = DMFlagCallback;
	s_health_box.generic.name = name_allowhealth;
	s_health_box.generic.width = re.BF_Strlen(name_allowhealth);
	s_health_box.generic.flags = QMF_SINGLELINE;
	s_health_box.curvalue = ((dm_flags & DF_NO_HEALTH) == 0); //mxd. Inverted value!
	s_health_box.itemnames = yes_no_names;

	Com_sprintf(name_offensive_spell, sizeof(name_offensive_spell), "\x02%s", m_item_offensive_spell->string);
	s_offensive_spell_box.generic.type = MTYPE_SPINCONTROL;
	s_offensive_spell_box.generic.x = 0;
	s_offensive_spell_box.generic.y = 80;
	s_offensive_spell_box.generic.name = name_offensive_spell;
	s_offensive_spell_box.generic.width = re.BF_Strlen(name_offensive_spell);
	s_offensive_spell_box.generic.flags = QMF_SINGLELINE;
	s_offensive_spell_box.generic.callback = DMFlagCallback;
	s_offensive_spell_box.curvalue = (dm_flags & DF_NO_OFFENSIVE_SPELL);
	s_offensive_spell_box.itemnames = yes_no_names;

	Com_sprintf(name_defensive_spell, sizeof(name_defensive_spell), "\x02%s", m_item_defensive_spell->string);
	s_defensive_spell_box.generic.type = MTYPE_SPINCONTROL;
	s_defensive_spell_box.generic.x = 0;
	s_defensive_spell_box.generic.y = 100;
	s_defensive_spell_box.generic.name = name_defensive_spell;
	s_defensive_spell_box.generic.width = re.BF_Strlen(name_defensive_spell);
	s_defensive_spell_box.generic.flags = QMF_SINGLELINE;
	s_defensive_spell_box.generic.callback = DMFlagCallback;
	s_defensive_spell_box.curvalue = ((dm_flags & DF_NO_DEFENSIVE_SPELL) == 0); //mxd. Inverted value!
	s_defensive_spell_box.itemnames = yes_no_names;

	Com_sprintf(name_infinitemana, sizeof(name_infinitemana), "\x02%s", m_item_infinitemana->string);
	s_infinitemana_box.generic.type = MTYPE_SPINCONTROL;
	s_infinitemana_box.generic.x = 0;
	s_infinitemana_box.generic.y = 120;
	s_infinitemana_box.generic.name = name_infinitemana;
	s_infinitemana_box.generic.width = re.BF_Strlen(name_infinitemana);
	s_infinitemana_box.generic.flags = QMF_SINGLELINE;
	s_infinitemana_box.generic.callback = DMFlagCallback;
	s_infinitemana_box.curvalue = (dm_flags & DF_INFINITE_MANA);
	s_infinitemana_box.itemnames = yes_no_names;

	Com_sprintf(name_nonames, sizeof(name_nonames), "\x02%s", m_item_nonames->string);
	s_nonames_box.generic.type = MTYPE_SPINCONTROL;
	s_nonames_box.generic.x = 0;
	s_nonames_box.generic.y = 140;
	s_nonames_box.generic.name = name_nonames;
	s_nonames_box.generic.width = re.BF_Strlen(name_nonames);
	s_nonames_box.generic.flags = QMF_SINGLELINE;
	s_nonames_box.generic.callback = DMFlagCallback;
	s_nonames_box.curvalue = (dm_flags & DF_NONAMES);
	s_nonames_box.itemnames = yes_no_names;

	Com_sprintf(name_forcerespawn, sizeof(name_forcerespawn), "\x02%s", m_item_forcerespawn->string);
	s_force_respawn_box.generic.type = MTYPE_SPINCONTROL;
	s_force_respawn_box.generic.x = 0;
	s_force_respawn_box.generic.y = 160;
	s_force_respawn_box.generic.name = name_forcerespawn;
	s_force_respawn_box.generic.width = re.BF_Strlen(name_forcerespawn);
	s_force_respawn_box.generic.flags = QMF_SINGLELINE;
	s_force_respawn_box.generic.callback = DMFlagCallback;
	s_force_respawn_box.curvalue = (dm_flags & DF_FORCE_RESPAWN);
	s_force_respawn_box.itemnames = yes_no_names;

	Com_sprintf(name_allowexit, sizeof(name_allowexit), "\x02%s", m_item_allowexit->string);
	s_allow_exit_box.generic.type = MTYPE_SPINCONTROL;
	s_allow_exit_box.generic.x = 0;
	s_allow_exit_box.generic.y = 180;
	s_allow_exit_box.generic.name = name_allowexit;
	s_allow_exit_box.generic.width = re.BF_Strlen(name_allowexit);
	s_allow_exit_box.generic.flags = QMF_SINGLELINE;
	s_allow_exit_box.generic.callback = DMFlagCallback;
	s_allow_exit_box.curvalue = (dm_flags & DF_ALLOW_EXIT);
	s_allow_exit_box.itemnames = yes_no_names;

	Com_sprintf(name_samemap, sizeof(name_samemap), "\x02%s", m_item_samemap->string);
	s_samelevel_box.generic.type = MTYPE_SPINCONTROL;
	s_samelevel_box.generic.x = 0;
	s_samelevel_box.generic.y = 200;
	s_samelevel_box.generic.name = name_samemap;
	s_samelevel_box.generic.width = re.BF_Strlen(name_samemap);
	s_samelevel_box.generic.flags = QMF_SINGLELINE;
	s_samelevel_box.generic.callback = DMFlagCallback;
	s_samelevel_box.curvalue = (dm_flags & DF_SAME_LEVEL);
	s_samelevel_box.itemnames = yes_no_names;

	Com_sprintf(name_leader, sizeof(name_leader), "\x02%s", m_item_leader->string);
	s_show_leader_box.generic.type = MTYPE_SPINCONTROL;
	s_show_leader_box.generic.x = 0;
	s_show_leader_box.generic.y = 220;
	s_show_leader_box.generic.name = name_leader;
	s_show_leader_box.generic.width = re.BF_Strlen(name_leader);
	s_show_leader_box.generic.flags = QMF_SINGLELINE;
	s_show_leader_box.generic.callback = DMFlagCallback;
	s_show_leader_box.curvalue = (dm_flags & DF_SHOW_LEADER);
	s_show_leader_box.itemnames = yes_no_names;

	Com_sprintf(name_teamplay, sizeof(name_teamplay), "\x02%s", m_item_teamplay->string);
	s_teamplay_box.generic.type = MTYPE_SPINCONTROL;
	s_teamplay_box.generic.x = 0;
	s_teamplay_box.generic.y = 240;
	s_teamplay_box.generic.name = name_teamplay;
	s_teamplay_box.generic.width = re.BF_Strlen(name_teamplay);
	s_teamplay_box.generic.flags = QMF_SINGLELINE;
	s_teamplay_box.generic.callback = DMFlagCallback;
	s_teamplay_box.itemnames = teamplay_names;

	Com_sprintf(name_friendlyfire, sizeof(name_friendlyfire), "\x02%s", m_item_friendlyfire->string);
	s_friendlyfire_box.generic.type = MTYPE_SPINCONTROL;
	s_friendlyfire_box.generic.x = 0;
	s_friendlyfire_box.generic.y = 260;
	s_friendlyfire_box.generic.name = name_friendlyfire;
	s_friendlyfire_box.generic.width = re.BF_Strlen(name_friendlyfire);
	s_friendlyfire_box.generic.flags = QMF_SINGLELINE;
	s_friendlyfire_box.generic.callback = DMFlagCallback;
	s_friendlyfire_box.curvalue = (dm_flags & DF_HURT_FRIENDS);
	s_friendlyfire_box.itemnames = yes_no_names;

	Com_sprintf(name_dismember, sizeof(name_dismember), "\x02%s", m_item_dismember->string);
	s_allow_dismemberment_box.generic.type = MTYPE_SPINCONTROL;
	s_allow_dismemberment_box.generic.x = 0;
	s_allow_dismemberment_box.generic.y = 280;
	s_allow_dismemberment_box.generic.name = name_dismember;
	s_allow_dismemberment_box.generic.width = re.BF_Strlen(name_dismember);
	s_allow_dismemberment_box.generic.flags = QMF_SINGLELINE;
	s_allow_dismemberment_box.generic.callback = DMFlagCallback;
	s_allow_dismemberment_box.curvalue = (dm_flags & DF_DISMEMBER);
	s_allow_dismemberment_box.itemnames = yes_no_names;

	Menu_AddItem(&s_dmoptions_menu, &s_weapons_stay_box.generic);
	Menu_AddItem(&s_dmoptions_menu, &s_powerups_box.generic);
	Menu_AddItem(&s_dmoptions_menu, &s_shrines_box.generic);
	Menu_AddItem(&s_dmoptions_menu, &s_health_box.generic);
	Menu_AddItem(&s_dmoptions_menu, &s_offensive_spell_box.generic);
	Menu_AddItem(&s_dmoptions_menu, &s_defensive_spell_box.generic);
	Menu_AddItem(&s_dmoptions_menu, &s_infinitemana_box.generic);
	Menu_AddItem(&s_dmoptions_menu, &s_nonames_box.generic);
	Menu_AddItem(&s_dmoptions_menu, &s_force_respawn_box.generic);
	Menu_AddItem(&s_dmoptions_menu, &s_allow_exit_box.generic);
	Menu_AddItem(&s_dmoptions_menu, &s_samelevel_box.generic);
	Menu_AddItem(&s_dmoptions_menu, &s_show_leader_box.generic);
	Menu_AddItem(&s_dmoptions_menu, &s_teamplay_box.generic);
	Menu_AddItem(&s_dmoptions_menu, &s_friendlyfire_box.generic);
	Menu_AddItem(&s_dmoptions_menu, &s_allow_dismemberment_box.generic);

	Menu_Center(&s_dmoptions_menu);

	DMFlagCallback(NULL);
}

static void DMOptions_MenuDraw(void)
{
	NOT_IMPLEMENTED
}

static const char* DMOptions_MenuKey(int key)
{
	NOT_IMPLEMENTED
	return NULL;
}

// Q2 counterpart
void M_Menu_DMOptions_f(void)
{
	DMOptions_MenuInit();
	M_PushMenu(DMOptions_MenuDraw, DMOptions_MenuKey);
}