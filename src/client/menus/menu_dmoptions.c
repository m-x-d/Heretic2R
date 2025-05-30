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

static menuframework_t s_dmoptions_menu;

static menulist_t s_weapons_stay_box;
static menulist_t s_powerups_box;
static menulist_t s_shrines_box;
static menulist_t s_health_box;
static menulist_t s_offensive_spell_box;
static menulist_t s_defensive_spell_box;
static menulist_t s_infinitemana_box;
static menulist_t s_nonames_box;
static menulist_t s_force_respawn_box;
static menulist_t s_allow_exit_box;
static menulist_t s_samelevel_box;
static menulist_t s_show_leader_box;
static menulist_t s_teamplay_box;
static menulist_t s_friendlyfire_box;
static menulist_t s_allow_dismemberment_box;

static void DMFlagCallback(void* self)
{
	uint bit = 0;
	int remove_flag = 0;
	uint flags = Q_ftol(Cvar_VariableValue("dmflags"));
	const menulist_t* f = self;

	if (f == &s_teamplay_box)
	{
		remove_flag = f->curvalue;
		flags &= ~(DF_SKINTEAMS | DF_MODELTEAMS);

		if (f->curvalue == 1)
			flags |= DF_SKINTEAMS;
		else if (f->curvalue == 2)
			flags |= DF_MODELTEAMS;
	}
	else if (f == &s_friendlyfire_box)
	{
		bit = DF_HURT_FRIENDS;
		remove_flag = 0;
	}
	else if (f == &s_weapons_stay_box)
	{
		bit = DF_WEAPONS_STAY;
		remove_flag = 0;
	}
	else if (f == &s_allow_exit_box)
	{
		bit = DF_ALLOW_EXIT;
		remove_flag = 0;
	}
	else if (f == &s_powerups_box)
	{
		bit = DF_NO_SHRINE;
		remove_flag = 1;
	}
	else if (f == &s_health_box)
	{
		bit = DF_NO_HEALTH;
		remove_flag = 1;
	}
	else if (f == &s_shrines_box)
	{
		bit = DF_SHRINE_CHAOS;
		remove_flag = 0;
	}
	else if (f == &s_samelevel_box)
	{
		bit = DF_SAME_LEVEL;
		remove_flag = 0;
	}
	else if (f == &s_force_respawn_box)
	{
		bit = DF_FORCE_RESPAWN;
		remove_flag = 0;
	}
	else if (f == &s_infinitemana_box)
	{
		bit = DF_INFINITE_MANA;
		remove_flag = 0;
	}
	else if (f == &s_show_leader_box)
	{
		bit = DF_SHOW_LEADER;
		remove_flag = 0;
	}
	else if (f == &s_offensive_spell_box)
	{
		bit = DF_NO_OFFENSIVE_SPELL;
		remove_flag = 0;
	}
	else if (f == &s_defensive_spell_box)
	{
		bit = DF_NO_DEFENSIVE_SPELL;
		remove_flag = 1;
	}
	else if (f == &s_allow_dismemberment_box)
	{
		bit = DF_DISMEMBER;
		remove_flag = 0;
	}
	else if (f == &s_nonames_box)
	{
		bit = DF_NONAMES;
		remove_flag = 0;
	}

	if (f != NULL && f != &s_teamplay_box)
	{
		if (f->curvalue == remove_flag)
			flags &= ~bit;
		else
			flags |= bit;
	}

	Cvar_SetValue("dmflags", (float)flags);
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
	s_weapons_stay_box.curvalue = ((dm_flags & DF_WEAPONS_STAY) != 0);
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
	s_shrines_box.curvalue = ((dm_flags & DF_SHRINE_CHAOS) != 0);
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
	s_offensive_spell_box.curvalue = ((dm_flags & DF_NO_OFFENSIVE_SPELL) != 0);
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
	s_infinitemana_box.curvalue = ((dm_flags & DF_INFINITE_MANA) != 0);
	s_infinitemana_box.itemnames = yes_no_names;

	Com_sprintf(name_nonames, sizeof(name_nonames), "\x02%s", m_item_nonames->string);
	s_nonames_box.generic.type = MTYPE_SPINCONTROL;
	s_nonames_box.generic.x = 0;
	s_nonames_box.generic.y = 140;
	s_nonames_box.generic.name = name_nonames;
	s_nonames_box.generic.width = re.BF_Strlen(name_nonames);
	s_nonames_box.generic.flags = QMF_SINGLELINE;
	s_nonames_box.generic.callback = DMFlagCallback;
	s_nonames_box.curvalue = ((dm_flags & DF_NONAMES) != 0);
	s_nonames_box.itemnames = yes_no_names;

	Com_sprintf(name_forcerespawn, sizeof(name_forcerespawn), "\x02%s", m_item_forcerespawn->string);
	s_force_respawn_box.generic.type = MTYPE_SPINCONTROL;
	s_force_respawn_box.generic.x = 0;
	s_force_respawn_box.generic.y = 160;
	s_force_respawn_box.generic.name = name_forcerespawn;
	s_force_respawn_box.generic.width = re.BF_Strlen(name_forcerespawn);
	s_force_respawn_box.generic.flags = QMF_SINGLELINE;
	s_force_respawn_box.generic.callback = DMFlagCallback;
	s_force_respawn_box.curvalue = ((dm_flags & DF_FORCE_RESPAWN) != 0);
	s_force_respawn_box.itemnames = yes_no_names;

	Com_sprintf(name_allowexit, sizeof(name_allowexit), "\x02%s", m_item_allowexit->string);
	s_allow_exit_box.generic.type = MTYPE_SPINCONTROL;
	s_allow_exit_box.generic.x = 0;
	s_allow_exit_box.generic.y = 180;
	s_allow_exit_box.generic.name = name_allowexit;
	s_allow_exit_box.generic.width = re.BF_Strlen(name_allowexit);
	s_allow_exit_box.generic.flags = QMF_SINGLELINE;
	s_allow_exit_box.generic.callback = DMFlagCallback;
	s_allow_exit_box.curvalue = ((dm_flags & DF_ALLOW_EXIT) != 0);
	s_allow_exit_box.itemnames = yes_no_names;

	Com_sprintf(name_samemap, sizeof(name_samemap), "\x02%s", m_item_samemap->string);
	s_samelevel_box.generic.type = MTYPE_SPINCONTROL;
	s_samelevel_box.generic.x = 0;
	s_samelevel_box.generic.y = 200;
	s_samelevel_box.generic.name = name_samemap;
	s_samelevel_box.generic.width = re.BF_Strlen(name_samemap);
	s_samelevel_box.generic.flags = QMF_SINGLELINE;
	s_samelevel_box.generic.callback = DMFlagCallback;
	s_samelevel_box.curvalue = ((dm_flags & DF_SAME_LEVEL) != 0);
	s_samelevel_box.itemnames = yes_no_names;

	Com_sprintf(name_leader, sizeof(name_leader), "\x02%s", m_item_leader->string);
	s_show_leader_box.generic.type = MTYPE_SPINCONTROL;
	s_show_leader_box.generic.x = 0;
	s_show_leader_box.generic.y = 220;
	s_show_leader_box.generic.name = name_leader;
	s_show_leader_box.generic.width = re.BF_Strlen(name_leader);
	s_show_leader_box.generic.flags = QMF_SINGLELINE;
	s_show_leader_box.generic.callback = DMFlagCallback;
	s_show_leader_box.curvalue = ((dm_flags & DF_SHOW_LEADER) != 0);
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

	//BUGFIX: mxd. Set s_teamplay_box value.
	if (dm_flags & DF_SKINTEAMS)
		s_teamplay_box.curvalue = 1;
	else if (dm_flags & DF_MODELTEAMS)
		s_teamplay_box.curvalue = 2;
	else
		s_teamplay_box.curvalue = 0;

	Com_sprintf(name_friendlyfire, sizeof(name_friendlyfire), "\x02%s", m_item_friendlyfire->string);
	s_friendlyfire_box.generic.type = MTYPE_SPINCONTROL;
	s_friendlyfire_box.generic.x = 0;
	s_friendlyfire_box.generic.y = 260;
	s_friendlyfire_box.generic.name = name_friendlyfire;
	s_friendlyfire_box.generic.width = re.BF_Strlen(name_friendlyfire);
	s_friendlyfire_box.generic.flags = QMF_SINGLELINE;
	s_friendlyfire_box.generic.callback = DMFlagCallback;
	s_friendlyfire_box.curvalue = ((dm_flags & DF_HURT_FRIENDS) != 0);
	s_friendlyfire_box.itemnames = yes_no_names;

	Com_sprintf(name_dismember, sizeof(name_dismember), "\x02%s", m_item_dismember->string);
	s_allow_dismemberment_box.generic.type = MTYPE_SPINCONTROL;
	s_allow_dismemberment_box.generic.x = 0;
	s_allow_dismemberment_box.generic.y = 280;
	s_allow_dismemberment_box.generic.name = name_dismember;
	s_allow_dismemberment_box.generic.width = re.BF_Strlen(name_dismember);
	s_allow_dismemberment_box.generic.flags = QMF_SINGLELINE;
	s_allow_dismemberment_box.generic.callback = DMFlagCallback;
	s_allow_dismemberment_box.curvalue = ((dm_flags & DF_DISMEMBER) != 0);
	s_allow_dismemberment_box.itemnames = yes_no_names;

	Menu_AddItem(&s_dmoptions_menu, &s_weapons_stay_box);
	Menu_AddItem(&s_dmoptions_menu, &s_powerups_box);
	Menu_AddItem(&s_dmoptions_menu, &s_shrines_box);
	Menu_AddItem(&s_dmoptions_menu, &s_health_box);
	Menu_AddItem(&s_dmoptions_menu, &s_offensive_spell_box);
	Menu_AddItem(&s_dmoptions_menu, &s_defensive_spell_box);
	Menu_AddItem(&s_dmoptions_menu, &s_infinitemana_box);
	Menu_AddItem(&s_dmoptions_menu, &s_nonames_box);
	Menu_AddItem(&s_dmoptions_menu, &s_force_respawn_box);
	Menu_AddItem(&s_dmoptions_menu, &s_allow_exit_box);
	Menu_AddItem(&s_dmoptions_menu, &s_samelevel_box);
	Menu_AddItem(&s_dmoptions_menu, &s_show_leader_box);
	Menu_AddItem(&s_dmoptions_menu, &s_teamplay_box);
	Menu_AddItem(&s_dmoptions_menu, &s_friendlyfire_box);
	Menu_AddItem(&s_dmoptions_menu, &s_allow_dismemberment_box);

	Menu_Center(&s_dmoptions_menu);
	//DMFlagCallback(NULL); //mxd. Does not set anything.
}

static void DMOptions_MenuDraw(void)
{
	char title[MAX_QPATH];

	// Draw menu BG.
	re.BookDrawPic(0, 0, "book/back/b_conback8.bk", cls.m_menuscale);

	if (cls.m_menualpha == 0.0f)
		return;

	// Draw menu title.
	Com_sprintf(title, sizeof(title), "\x03%s", m_banner_dmopt->string);
	const int x = M_GetMenuLabelX(re.BF_Strlen(title));
	const int y = M_GetMenuOffsetY(&s_dmoptions_menu);
	re.DrawBigFont(x, y, title, cls.m_menualpha);

	// Draw menu items.
	s_dmoptions_menu.x = M_GetMenuLabelX(s_dmoptions_menu.width);
	Menu_Draw(&s_dmoptions_menu);
}

// Q2 counterpart
static const char* DMOptions_MenuKey(const int key)
{
	return Default_MenuKey(&s_dmoptions_menu, key);
}

// Q2 counterpart
void M_Menu_DMOptions_f(void)
{
	DMOptions_MenuInit();
	M_PushMenu(DMOptions_MenuDraw, DMOptions_MenuKey);
}