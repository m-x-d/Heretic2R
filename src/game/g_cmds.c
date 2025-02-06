//
// g_cmds.c
//
// Copyright 1998 Raven Software
//

#include "g_cmds.h" //mxd
#include "cl_strings.h"
#include "g_combat.h" //mxd
#include "g_items.h"
#include "g_itemstats.h"
#include "g_playstats.h"
#include "p_anims.h"
#include "p_dll.h" //mxd
#include "p_client.h" //mxd
#include "Random.h"
#include "spl_morph.h" //mxd
#include "Vector.h"

int self_spawn = FALSE; // True when spawned manually using 'spawn' ccmd.

// Flood protection.
static qboolean CheckFlood(const edict_t* ent)
{
	if (!FLOOD_MSGS)
		return false;

	if (level.time < ent->client->flood_locktill)
	{
		gi.msgvar_centerprintf(ent, GM_SHUTUP, (int)(ent->client->flood_locktill - level.time));
		return true;
	}

	int pos = ent->client->flood_whenhead - FLOOD_MSGS + 1;

	if (pos < 0)
		pos += ARRAYSIZE(ent->client->flood_when);

	if (ent->client->flood_when[pos] > 0.0f && (level.time - ent->client->flood_when[pos] < flood_persecond->value))
	{
		ent->client->flood_locktill = level.time + flood_waitdelay->value;
		gi.msgvar_centerprintf(ent, GM_SHUTUP, (int)flood_waitdelay->value);

		return true;
	}

	ent->client->flood_whenhead++;
	ent->client->flood_whenhead %= ARRAYSIZE(ent->client->flood_when);
	ent->client->flood_when[ent->client->flood_whenhead] = level.time;

	return false;
}

static char* ClientSkinTeam(const edict_t* ent)
{
	static char	value[512];

	value[0] = 0;

	if (ent->client == NULL)
		return value;

	strcpy_s(value, sizeof(value), Info_ValueForKey(ent->client->playerinfo.pers.userinfo, "skin")); //mxd. strcpy -> strcpy_s

	char* p = strchr(value, '/');

	return (p != NULL ? ++p : value);
}

static char* ClientModelTeam(const edict_t* ent)
{
	static char	value[512];

	value[0] = 0;

	if (ent->client == NULL)
		return value;

	strcpy_s(value, sizeof(value), Info_ValueForKey(ent->client->playerinfo.pers.userinfo, "skin")); //mxd. strcpy -> strcpy_s

	char* p = strchr(value, '/');

	if (p == NULL)
	{
		// If no slash, then assume male.
		strcpy_s(value, sizeof(value), "male"); //mxd. strcpy -> strcpy_s
		return value;
	}

	*p = 0;
	return value;
}

qboolean OnSameTeam(const edict_t* ent1, const edict_t* ent2)
{
	char ent1_team[512];
	char ent2_team[512];

	if (!(DMFLAGS & (DF_MODELTEAMS | DF_SKINTEAMS)))
		return false;

	if (DMFLAGS & DF_SKINTEAMS)
	{
		strcpy_s(ent1_team, sizeof(ent1_team), ClientSkinTeam(ent1)); //mxd. strcpy -> strcpy_s
		strcpy_s(ent2_team, sizeof(ent2_team), ClientSkinTeam(ent2)); //mxd. strcpy -> strcpy_s
	}
	else
	{
		strcpy_s(ent1_team, sizeof(ent1_team), ClientModelTeam(ent1)); //mxd. strcpy -> strcpy_s
		strcpy_s(ent2_team, sizeof(ent2_team), ClientModelTeam(ent2)); //mxd. strcpy -> strcpy_s
	}

	return (Q_stricmp(ent1_team, ent2_team) == 0); //mxd. stricmp -> Q_stricmp
}

static void SelectNextItem(const edict_t* ent, const int item_flags)
{
	if (SV_CINEMATICFREEZE)
		return;

	gclient_t* cl = ent->client;

	// Scan for the next valid one.
	for (int i = 1; i < MAX_ITEMS; i++) //mxd. Bugfix, kinda: original logic eventually wraps back to selected_item index.
	{
		const int index = (cl->playerinfo.pers.selected_item + i) % MAX_ITEMS;
		if (!cl->playerinfo.pers.inventory.Items[index])
			continue;

		const gitem_t* item = &playerExport.p_itemlist[index];
		if (item->use == NULL || !(item->flags & item_flags))
			continue;

		cl->playerinfo.pers.selected_item = index;

		return;
	}

	cl->playerinfo.pers.selected_item = -1;
}

static void SelectPrevItem(const edict_t* ent, const int item_flags)
{
	if (SV_CINEMATICFREEZE)
		return;

	gclient_t* cl = ent->client;

	// Scan for the previous valid one.
	for (int i = MAX_ITEMS - 1; i > 0; i--) //mxd. Bugfix, kinda: original logic eventually wraps back to selected_item index.
	{
		const int index = (cl->playerinfo.pers.selected_item + i) % MAX_ITEMS;
		if (!cl->playerinfo.pers.inventory.Items[index])
			continue;

		gitem_t* item = &playerExport.p_itemlist[index];
		if (item->use == NULL || !(item->flags & item_flags))
			continue;

		cl->playerinfo.pers.selected_item = index;
		cl->playerinfo.pers.defence = item; //TODO: mxd. Huh? Why is it done here, but not in SelectNextItem()?..

		return;
	}

	cl->playerinfo.pers.selected_item = -1;
}

void ValidateSelectedItem(const edict_t* ent)
{
	const client_persistant_t* pers = &ent->client->playerinfo.pers;
	if (pers->inventory.Items[pers->selected_item] == 0)
		SelectNextItem(ent, -1);
}

#pragma region ========================== Command handlers ==========================

// Give items to a client.
static void Cmd_Give_f(edict_t* ent)
{
	if (DEATHMATCH && !SV_CHEATS)
	{
		gi.gamemsg_centerprintf(ent, GM_NOCHEATS);
		return;
	}

	const char* name = gi.args();

	// FOR TESTING ONLY!
	if (Q_stricmp(name, "level") == 0)
	{
		// Give level-specified weapons.
		if (level.offensive_weapons & 4)
			AddWeaponToInventory(P_FindItem("hell"), ent);

		if (level.offensive_weapons & 8)
			AddWeaponToInventory(P_FindItem("array"), ent);

		if (level.offensive_weapons & 16)
			AddWeaponToInventory(P_FindItem("rain"), ent);

		if (level.offensive_weapons & 32)
			AddWeaponToInventory(P_FindItem("sphere"), ent);

		if (level.offensive_weapons & 64)
			AddWeaponToInventory(P_FindItem("phoen"), ent);

		if (level.offensive_weapons & 128)
			AddWeaponToInventory(P_FindItem("mace"), ent);

		if (level.offensive_weapons & 256)
			AddWeaponToInventory(P_FindItem("fwall"), ent);

		// Give level-specified defenses.
		if (level.defensive_weapons & 1)
			AddDefenseToInventory(P_FindItem("ring"), ent);

		if (level.defensive_weapons & 2)
			AddDefenseToInventory(P_FindItem("lshield"), ent);

		if (level.defensive_weapons & 4)
			AddDefenseToInventory(P_FindItem("tele"), ent);

		if (level.defensive_weapons & 8)
			AddDefenseToInventory(P_FindItem("morph"), ent);

		if (level.defensive_weapons & 16)
			AddDefenseToInventory(P_FindItem("meteor"), ent);

		SetupPlayerinfo_effects(ent);
		P_PlayerUpdateModelAttributes(&ent->client->playerinfo);
		WritePlayerinfo_effects(ent);

		return;
	}

	const qboolean give_all = (Q_stricmp(name, "all") == 0);
	client_persistant_t* pers = &ent->client->playerinfo.pers; //mxd

	if (give_all || Q_stricmp(name, "health") == 0)
	{
		if (gi.argc() == 3)
			ent->health += Q_atoi(gi.argv(2)); //mxd. atoi -> Q_atoi //TODO: shouldn't this be clamped by max_health?
		else
			ent->health = ent->max_health;

		if (give_all || ent->health == ent->max_health)
			ResetPlayerBaseNodes(ent); // Put back all your limbs!

		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "weapons") == 0)
	{
		for (int i = 0; i < game.num_items; i++)
		{
			const gitem_t* item = &playerExport.p_itemlist[i];

			if (item->pickup == NULL || !(item->flags & IT_WEAPON))
				continue;

			pers->inventory.Items[i]++;

			if (item->playeranimseq == ASEQ_WRRBOW_GO || item->playeranimseq == ASEQ_WPHBOW_GO)
			{
				// This is a bow, put the bow on player's back.
				pers->bowtype = ((item->tag == ITEM_WEAPON_PHOENIXBOW) ? BOW_TYPE_PHOENIX : BOW_TYPE_REDRAIN);

				SetupPlayerinfo_effects(ent);
				P_PlayerUpdateModelAttributes(&ent->client->playerinfo);
				WritePlayerinfo_effects(ent);
			}
		}

		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "defences") == 0)
	{
		for (int i = 0; i < game.num_items; i++)
		{
			const gitem_t* item = &playerExport.p_itemlist[i];
			if (item->pickup != NULL && (item->flags & IT_DEFENSE))
				pers->inventory.Items[i]++;
		}

		// If we don't already have a defence item, make the ring default.
		if (pers->defence == NULL)
			pers->defence = P_FindItem("ring");

		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "mana") == 0)
	{
		for (int i = 0; i < game.num_items; i++)
		{
			const gitem_t* item = &playerExport.p_itemlist[i];
			if (item->pickup != NULL && (item->flags & IT_AMMO))
				Add_Ammo(ent, item, 1000);
		}

		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "armor") == 0)
	{
		if (pers->armortype == ARMOR_TYPE_NONE)
		{
			pers->armor_count = silver_armor_info.max_armor;
			pers->armortype = ARMOR_TYPE_SILVER;
		}
		else // We'll assume there's armor, so load up with gold.
		{
			pers->armor_count = gold_armor_info.max_armor;
			pers->armortype = ARMOR_TYPE_GOLD;
		}

		SetupPlayerinfo_effects(ent);
		P_PlayerUpdateModelAttributes(&ent->client->playerinfo);
		WritePlayerinfo_effects(ent);

		if (!give_all)
			return;
	}

	// Give all does not give staff powerup.
	if (Q_stricmp(name, "staff") == 0)
	{
		if (pers->stafflevel < STAFF_LEVEL_MAX - 1)
			pers->stafflevel++;
		else
			pers->stafflevel = STAFF_LEVEL_BASIC;

		gi.dprintf("Setting staff level to %d\n", pers->stafflevel);

		SetupPlayerinfo_effects(ent);
		P_PlayerUpdateModelAttributes(&ent->client->playerinfo);
		WritePlayerinfo_effects(ent);

		return;
	}

	// Give all does not give lungs.
	if (Q_stricmp(name, "lungs") == 0)
	{
		ent->client->playerinfo.lungs_timer = LUNGS_DURATION;
		ent->air_finished = level.time + HOLD_BREATH_TIME;

		return;
	}

	// Give all does not give powerups.
	if (Q_stricmp(name, "powerup") == 0)
	{
		ent->client->playerinfo.powerup_timer = level.time + POWERUP_DURATION;
		return;
	}

	// Give all does not give reflection.
	if (Q_stricmp(name, "reflection") == 0)
	{
		ent->client->playerinfo.reflect_timer = level.time + REFLECT_DURATION_SINGLE;
		ent->s.renderfx |= RF_REFLECTION; // Turn on reflection client effect.

		return;
	}

	// Give all does not give ghost.
	if (Q_stricmp(name, "ghost") == 0)
	{
		ent->client->playerinfo.ghost_timer = level.time + GHOST_DURATION;
		ent->s.renderfx |= RF_TRANS_GHOST; // Turn on ghosting client effect.

		return;
	}

	// Give all does not give chicken.
	if (Q_stricmp(name, "chicken") == 0)
	{
		MorphPlayerToChicken(ent, ent);
		return;
	}

	// Give all does not give plague.
	if (Q_stricmp(name, "plague") == 0)
	{
		if (ent->client->playerinfo.plaguelevel < PLAGUE_NUM_LEVELS - 1)
			ent->client->playerinfo.plaguelevel++;
		else
			ent->client->playerinfo.plaguelevel = 0;

		gi.dprintf("Setting plague level to %d\n", ent->client->playerinfo.plaguelevel);

		char userinfo[MAX_INFO_STRING];
		memcpy(userinfo, pers->userinfo, sizeof(userinfo));
		ClientUserinfoChanged(ent, userinfo);

		SetupPlayerinfo_effects(ent);
		P_PlayerUpdateModelAttributes(&ent->client->playerinfo);
		WritePlayerinfo_effects(ent);

		return;
	}

	// Give puzzle items (?).
	if (give_all)
	{
		for (int i = 0; i < game.num_items; i++)
		{
			const gitem_t* item = &playerExport.p_itemlist[i];

			if (item->pickup == NULL && !(item->flags & IT_PUZZLE))
				continue;

			if (item->flags & (IT_ARMOR | IT_WEAPON | IT_AMMO | IT_DEFENSE)) //TODO: should skip IT_HEALTH items too?
				continue;

			pers->inventory.Items[i] = 1;
		}

		return;
	}

	// Give specific item by name, with optional count.
	const char* item_name = gi.argv(1); //mxd
	gitem_t* item = P_FindItem(item_name);
	if (item == NULL)
	{
		gi.dprintf("Unknown item\n");
		return;
	}

	if (item->pickup == NULL)
	{
		gi.dprintf("Non-pickup item\n");
		return;
	}

	const int index = ITEM_INDEX(item);

	if (item->flags & IT_AMMO)
		pers->inventory.Items[index] += ((gi.argc() == 3) ? Q_atoi(gi.argv(2)) : item->quantity); //mxd. atoi -> Q_atoi
	else
		pers->inventory.Items[index]++;

	// If we don't already have a defence item, make this defence item default.
	if (pers->defence == NULL && item->flags & IT_DEFENSE)
		pers->defence = item;
}

/*
==================
Cmd_God_f

Sets client to godmode

argv(0) god
==================
*/
void Cmd_God_f (edict_t *ent)
{
	char	*msg;

	if (deathmatch->value && !sv_cheats->value)
	{
		gi.gamemsg_centerprintf (ent, GM_NOCHEATS);
		return;
	}

	ent->flags ^= FL_GODMODE;
	if (!(ent->flags & FL_GODMODE) )
		msg = "godmode OFF\n";
	else
		msg = "godmode ON\n";

	gi.cprintf (ent, PRINT_HIGH, msg);
}


/*
==================
Cmd_Notarget_f

Sets client to notarget

argv(0) notarget
==================
*/
void Cmd_Notarget_f (edict_t *ent)
{
	char	*msg;

	if (deathmatch->value && !sv_cheats->value)
	{
		gi.gamemsg_centerprintf (ent, GM_NOCHEATS);
		return;
	}

	ent->flags ^= FL_NOTARGET;
	if (!(ent->flags & FL_NOTARGET) )
		msg = "notarget OFF\n";
	else
		msg = "notarget ON\n";

	gi.cprintf (ent, PRINT_HIGH, msg);
}

/*
==================
Cmd_Noclip_f

argv(0) noclip
==================
*/
void Cmd_Noclip_f (edict_t *ent)
{
	char	*msg;

	if (deathmatch->value && !sv_cheats->value)
	{
		gi.gamemsg_centerprintf (ent, GM_NOCHEATS);
		return;
	}

	if (ent->movetype == PHYSICSTYPE_NOCLIP)
	{
		ent->movetype = PHYSICSTYPE_STEP;
		msg = "noclip OFF\n";
	}
	else
	{
		ent->movetype = PHYSICSTYPE_NOCLIP;
		msg = "noclip ON\n";
	}

	gi.cprintf (ent, PRINT_HIGH, msg);
}

/*
==================
Cmd_Powerup_f

argv(0) powerup
==================
*/
void Cmd_Powerup_f (edict_t *ent)
{
	char	*msg;

	if (deathmatch->value && !sv_cheats->value)
	{
		gi.gamemsg_centerprintf (ent, GM_NOCHEATS);
		return;
	}

	assert(ent->client);
	if (ent->client->playerinfo.powerup_timer > level.time)
	{	// Turn OFF powerup
		ent->client->playerinfo.powerup_timer = level.time-0.1;
		msg = "Powerup OFF\n";
	}
	else
	{	// Turn ON powerup
		ent->client->playerinfo.powerup_timer = level.time + (60*60*24);	// One full day
		msg = "Powerup ON\n";
	}

	gi.cprintf (ent, PRINT_HIGH, msg);
}


/*
==================
Cmd_Use_f

Use an inventory item
==================
*/
void Cmd_Use_f (edict_t *ent, char *s)
{
	int			index;
	gitem_t		*it;
	qboolean	castme;
	playerinfo_t	*playerinfo;

	assert(ent);
	assert(ent->client);
//	assert(ent->client->playerinfo);
	playerinfo = &(ent->client->playerinfo);

	if (s[0] == '*')
	{	// Cast automatically with asterisk before name.  THIS ONLY WORKS WITH DEFENSIVE ITEMS.
		castme=true;
		s++;
	}
	else
	{
		castme=false;
	}

	it = P_FindItem (s);

	if(sv_cinematicfreeze->value)
		return;

	if (!it)
	{
		gi.cprintf (ent, PRINT_HIGH, "unknown item: %s\n", s);
		return;
	}
	if (!it->use)
	{
		gi.gamemsg_centerprintf (ent, GM_NOTUSABLE);
		return;
	}
	index = ITEM_INDEX(it);

	if (!playerinfo->pers.inventory.Items[index])
	{
		if (it->flags & (IT_WEAPON|IT_DEFENSE))
			// index is two off, since we can never run out of the staff or the flying fist
			gi.gamemsg_centerprintf (ent, it->msg_nouse);
		else
			gi.gamemsg_centerprintf (ent, GM_NOITEM);
		return;
	}

	if (castme && (it->flags & IT_DEFENSE) && 
			it->weaponthink && 
			ent->deadflag!=DEAD_DEAD && playerinfo->deadflag!=DEAD_DYING)
	{
		if (playerinfo->leveltime > playerinfo->defensive_debounce)
		{	// Do something only if the debounce is okay.
			playerinfo->pers.lastdefence = playerinfo->pers.defence;	
			playerinfo->pers.defence=it;

			if (P_Defence_CurrentShotsLeft(playerinfo, 1) > 0)
			{	// Only if there is ammo
				it->weaponthink(ent,"");

				if(playerinfo->pers.defence&&playerinfo->pers.defence->ammo)
					playerinfo->def_ammo_index=ITEM_INDEX(P_FindItem(playerinfo->pers.defence->ammo));
				else
					playerinfo->def_ammo_index=0;

				playerinfo->defensive_debounce = playerinfo->leveltime + DEFENSE_DEBOUNCE;
			}
			else
			{	//Play a sound to tell the player they're out of mana
				gi.sound(ent, CHAN_VOICE, gi.soundindex("*nomana.wav"), 0.75, ATTN_NORM, 0);
			}

			// Put the ammo back.
			playerinfo->pers.defence = playerinfo->pers.lastdefence;
			playerinfo->pers.lastdefence = it;
		}
	}
	else
	{
		it->use(&ent->client->playerinfo,it);
	}
}

/*
=================
Cmd_WeapPrev_f
=================
*/
void Cmd_WeapPrev_f (edict_t *ent)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;
	int			selected_weapon;

	cl = ent->client;

	if (!cl->playerinfo.pers.weapon || sv_cinematicfreeze->value)
		return;

	selected_weapon = ITEM_INDEX(cl->playerinfo.pers.weapon);

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (selected_weapon + (MAX_ITEMS -i))%MAX_ITEMS;

		if (!cl->playerinfo.pers.inventory.Items[index])
			continue;

		it = &playerExport.p_itemlist[index];
		if (!it->use)
			continue;
		if (! (it->flags & IT_WEAPON) )
			continue;

		// if we are in water, don't select any weapon that requires ammo
		if ((ent->waterlevel >= 2) &&
			((it->tag == ITEM_WEAPON_HELLSTAFF) ||
			 (it->tag == ITEM_WEAPON_REDRAINBOW) ||
			 (it->tag == ITEM_WEAPON_PHOENIXBOW)))
			continue;

		it->use(&ent->client->playerinfo,it);
		if (ent->client->playerinfo.pers.newweapon == it)
			return;	// successful
	}
}

/*
=================
Cmd_WeapNext_f
=================
*/
void Cmd_WeapNext_f (edict_t *ent)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;
	int			selected_weapon;

	cl = ent->client;

	if (!cl->playerinfo.pers.weapon || sv_cinematicfreeze->value)
		return;

	selected_weapon = ITEM_INDEX(cl->playerinfo.pers.weapon);

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (selected_weapon + i)%MAX_ITEMS;

		if (!cl->playerinfo.pers.inventory.Items[index])
			continue;

		it = &playerExport.p_itemlist[index];
		if (!it->use)
			continue;
		if (! (it->flags & IT_WEAPON) )
			continue;

		// if we are in water, don't select any weapon that requires ammo
		if ((ent->waterlevel >= 2) &&
			((it->tag == ITEM_WEAPON_HELLSTAFF) ||
			(it->tag == ITEM_WEAPON_REDRAINBOW) ||
			(it->tag == ITEM_WEAPON_PHOENIXBOW)))
			continue;

		it->use(&ent->client->playerinfo,it);
		if (ent->client->playerinfo.pers.newweapon == it)
			return;	// successful
	}
}
/*
=================
Cmd_DefPrev_f
=================
*/
void Cmd_DefPrev_f (edict_t *ent)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;
	int			selected_defence;
	int			start_defence;

	if(sv_cinematicfreeze->value)
		return;

	cl = ent->client;

	if (!cl->playerinfo.pers.defence)
		selected_defence = 1;
	else
		selected_defence = ITEM_INDEX(cl->playerinfo.pers.defence);
	start_defence = selected_defence;
	
	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (selected_defence + (MAX_ITEMS-i))%MAX_ITEMS;

		if (!cl->playerinfo.pers.inventory.Items[index])
			continue;

		it = &playerExport.p_itemlist[index];
		if (!it->use)
			continue;
		if (! (it->flags & IT_DEFENSE) )
			continue;

		it->use(&ent->client->playerinfo,it);
		if (cl->playerinfo.pers.defence == it)
		{
			selected_defence = index;
			break;	// successful
		}
	}

	if ((selected_defence != 1) && (start_defence != selected_defence))
		cl->playerinfo.G_Sound(SND_PRED_NULL,
							   level.time,
							   ent,
							   CHAN_AUTO,
							   cl->playerinfo.G_SoundIndex("Weapons/DefenseSelect.wav"),
							   1,
							   ATTN_NORM,
							   0);
}

/*
=================
Cmd_DefNext_f
=================
*/
void Cmd_DefNext_f (edict_t *ent)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;
	int			selected_defence;
	int			start_defence;

	if(sv_cinematicfreeze->value)
		return;

	cl = ent->client;

	if (!cl->playerinfo.pers.defence)
		selected_defence = 1;
	else
		selected_defence = ITEM_INDEX(cl->playerinfo.pers.defence);
	start_defence = selected_defence;

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (selected_defence + i)%MAX_ITEMS;
		
		if (!cl->playerinfo.pers.inventory.Items[index])
			continue;
		it = &playerExport.p_itemlist[index];
		if (!it->use)
			continue;
		if (! (it->flags & IT_DEFENSE) )
			continue;

		it->use(&ent->client->playerinfo,it);
		if (cl->playerinfo.pers.defence == it)
		{
			selected_defence = index;
			break;	// successful
		}
	}

	if ((selected_defence != 1) && (start_defence != selected_defence))
		cl->playerinfo.G_Sound(SND_PRED_NULL,
							   level.time,
							   ent,
							   CHAN_AUTO,
							   cl->playerinfo.G_SoundIndex("Weapons/DefenseSelect.wav"),
							   1,
							   ATTN_NORM,
							   0);
}

/*
=================
Cmd_WeapLast_f
=================
*/
void Cmd_WeapLast_f (edict_t *ent)
{
	gclient_t	*cl;
	int			index;
	gitem_t		*it;

	if(sv_cinematicfreeze->value)
		return;

	cl = ent->client;

	if (!cl->playerinfo.pers.weapon || !cl->playerinfo.pers.lastweapon)
		return;

	index = ITEM_INDEX(cl->playerinfo.pers.lastweapon);
	if (!cl->playerinfo.pers.inventory.Items[index])
		return;

	it = &playerExport.p_itemlist[index];
	if (!it->use)
		return;
	if (! (it->flags & IT_WEAPON) )
		return;

	it->use(&ent->client->playerinfo,it);
}

/*
=================
Cmd_Kill_f
=================
*/
void Cmd_Kill_f (edict_t *ent)
{
	if(ent->client->flood_nextkill > level.time)
	{
		gi.msgvar_centerprintf(ent, GM_NOKILL, (int)(ent->client->flood_nextkill - level.time) + 1);
		return;
	}
	ent->flags &= ~FL_GODMODE;
	
	if(ent->health > -1)
	{
		// Make sure we gib as we don't want bodies lying around everywhere.

		ent->health = -100000;
		ent->client->meansofdeath = MOD_SUICIDE;
		player_die (ent, ent, ent, 100000, vec3_origin);

		// Don't even bother waiting for death frames.

		ent->deadflag = DEAD_DEAD;

		// Put us back in the game
		
		respawn(ent);

		// Set up the next valid suicide time.

		ent->client->flood_nextkill = level.time + flood_killdelay->value;
	}
}


int PlayerSort (void const *a, void const *b)
{
	int		anum, bnum;

	anum = *(int *)a;
	bnum = *(int *)b;

	anum = game.clients[anum].ps.stats[STAT_FRAGS];
	bnum = game.clients[bnum].ps.stats[STAT_FRAGS];

	if (anum < bnum)
		return -1;
	if (anum > bnum)
		return 1;
	return 0;
}

/*
=================
Cmd_Players_f
=================
*/
void Cmd_Players_f (edict_t *ent)
{
	int		i;
	int		count;
	char	qsmall[64];
	char	large[1280];
	int		index[256];

	count = 0;
	for (i = 0 ; i < maxclients->value ; i++)
		
		if (game.clients[i].playerinfo.pers.connected)
		{
			index[count] = i;
			count++;
		}

	// sort by frags
	qsort (index, count, sizeof(index[0]), PlayerSort);

	// print information
	large[0] = 0;

	for (i = 0 ; i < count ; i++)
	{
		Com_sprintf (qsmall, sizeof(qsmall), "%3i %s\n",

		game.clients[index[i]].ps.stats[STAT_FRAGS],

		game.clients[index[i]].playerinfo.pers.netname);

		if (strlen (qsmall) + strlen(large) > sizeof(large) - 100 )
		{	// can't print all of them in one packet
			strcat (large, "...\n");
			break;
		}
		strcat (large, qsmall);
	}

	gi.cprintf (ent, PRINT_HIGH, "%s\n%i players\n", large, count);
}

/*
===================
Spawn an item
===================
*/

void Cmd_SpawnEntity_f(edict_t *ent)
{
	vec3_t	forward;
	edict_t	*newent;

	if (deathmatch->value && !sv_cheats->value)
	{
		gi.gamemsg_centerprintf (ent, GM_NOCHEATS);
		return;
	}
	gi.cprintf(ent, PRINT_HIGH, "Spawning : %s\n", gi.argv(1));
	self_spawn = TRUE;

	newent = G_Spawn();
	newent->classname = ED_NewString(gi.argv(1));
	AngleVectors(ent->s.angles, forward, NULL, NULL);
	VectorScale(forward, 100, forward);
	VectorAdd(ent->s.origin, forward, newent->s.origin);
	VectorCopy(ent->s.angles, newent->s.angles);
	ED_CallSpawn(newent);
	self_spawn = FALSE;
}

/*
===================
Toggle the Inventory Console
===================
*/

void Cmd_ToggleInventory_f(edict_t *ent)
{
	gclient_t	*cl;

	cl = ent->client;

	if (cl->playerinfo.showpuzzleinventory)
		cl->playerinfo.showpuzzleinventory = false;
	else
		cl->playerinfo.showpuzzleinventory = true;

}


/*
===================
Kill all monsters on a level
===================
*/

extern void Killed (edict_t *targ, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point, int mod);
void Cmd_KillMonsters_f(edict_t *ent)
{
	edict_t *searchent;

	gi.cprintf(ent, PRINT_HIGH, "Killing all non-boss level monsters\n");

	for (searchent = g_edicts; searchent < &g_edicts[globals.num_edicts]; searchent++)
	{
		if (!searchent->inuse)
			continue;
		if ((searchent->svflags & SVF_MONSTER) && !(searchent->monsterinfo.c_mode) && !(searchent->svflags & SVF_BOSS))
		{
			gi.dprintf("Killing monster %s\n", searchent->classname);
			Killed (searchent, ent, ent, 100000, searchent->s.origin, MOD_UNKNOWN);
			searchent->health = 0;
		}
	}
}

void Cmd_CrazyMonsters_f(edict_t *ent)
{
	edict_t *searchent;
	edict_t *enemy_ent;

	gi.cprintf(ent, PRINT_HIGH, "Berzerking all level monsters\n");
	ANARCHY = true;
	for (searchent = g_edicts; searchent < &g_edicts[globals.num_edicts]; searchent++)
	{
		if (!searchent->inuse)
			continue;
		if (searchent->svflags & SVF_MONSTER)
		{
			enemy_ent = NULL;
			while(!enemy_ent || !enemy_ent->inuse || !(enemy_ent->svflags & SVF_MONSTER)||enemy_ent->health<0||enemy_ent == searchent)
			{
				enemy_ent = &g_edicts[irand(0, globals.num_edicts)];
			}
			searchent->enemy = enemy_ent;
			FoundTarget(searchent, false);
		}
	}
}

void Cmd_AngerMonsters_f(edict_t *ent)
{
	edict_t *searchent;

	gi.cprintf(ent, PRINT_HIGH, "Angering all level monsters\n");
	for (searchent = g_edicts; searchent < &g_edicts[globals.num_edicts]; searchent++)
	{
		if (!searchent->inuse)
			continue;
		if (searchent->svflags & SVF_MONSTER)
		{
			searchent->enemy = ent;
			FoundTarget(searchent, false);
		}
	}
}
/*
===================
Go to next monster frame for frozen monsters
===================
*/

extern qboolean MonsterAdvanceFrame;
void Cmd_NextMonsterFrame_f(edict_t *ent)
{
	MonsterAdvanceFrame = true;
}



/*
==================
Cmd_Say_f
==================
*/
void Cmd_Say_f (edict_t *ent, qboolean team, qboolean arg0)
{
	int		j;
	edict_t	*other;
	char	*p;
	char	text[2048];
	char	text2[2048];
	int		color;

	if (gi.argc () < 2 && !arg0)
		return;

	if (!((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS)))
		team = false;

	// delimit the string if we are normal play talking, so we don't send the name with it
	Com_sprintf (text2, sizeof(text), "%s: ", ent->client->playerinfo.pers.netname);
	text[0] = 0;

	if (arg0)
	{
		strcat (text, gi.argv(0));
		strcat (text, " ");
		strcat (text, gi.args());

		strcat (text2, gi.argv(0));
		strcat (text2, " ");
		strcat (text2, gi.args());
	}
	else
	{
		p = gi.args();

		if (*p == '"')
		{
			p++;
			p[strlen(p)-1] = 0;
		}
		strcat(text, p);

		p = gi.args();

		if (*p == '"')
		{
			p++;
			p[strlen(p)-1] = 0;
		}
		strcat(text2, p);

	}

	// don't let text be too long for malicious reasons
	if (strlen(text) > 150)
		text[150] = 0;

	if (strlen(text2) > 150)
		text2[150] = 0;


	strcat(text, "\n");
	strcat(text2, "\n");

	if (CheckFlood(ent))
		return;

	if (dedicated->value)
		gi.cprintf(NULL, PRINT_CHAT, "%s", text2);

	for (j = 1; j <= game.maxclients; j++)
	{
		color = 0;
		other = &g_edicts[j];
		if (!other->inuse)
			continue;
		if (!other->client)
			continue;
		if (team)
		{
			color = 1;
			if (!OnSameTeam(ent, other))
				continue;
		}

		gi.clprintf(other,ent, color, "%s", text);	
	}
}


void Cmd_ShowCoords_f (edict_t *ent)
{
	assert(ent->client);

	Com_Printf("Player Location:  (%d, %d, %d)\n", 
			(int)(ent->s.origin[0]), (int)(ent->s.origin[1]), (int)(ent->s.origin[2]));
	Com_Printf("       Angle:  Facing=%2.2f, Pitch=%2.2f\n", ent->client->aimangles[YAW], -ent->client->aimangles[PITCH]);
}


void Cmd_TestFX_f (edict_t *ent)
{
	int i;

	if (ent->client == NULL)
		return;

	i = irand(0, 15);
	gi.dprintf("Setting pain skin number %d\n", i);
	ent->client->playerinfo.pers.altparts |= 1<<i;

	SetupPlayerinfo_effects(ent);
	P_PlayerUpdateModelAttributes(&ent->client->playerinfo);
	WritePlayerinfo_effects(ent);
}

#pragma endregion

/*
=================
ClientCommand
=================
*/
void ClientCommand (edict_t *ent)
{
	char	*cmd;

	if (!ent->client)
		return;		// not fully in game yet

	cmd = gi.argv(0);

	if (Q_stricmp (cmd, "players") == 0)
	{
		Cmd_Players_f (ent);
		return;
	}
	if (Q_stricmp (cmd, "say") == 0)
	{
		Cmd_Say_f (ent, false, false);
		return;
	}
	if (Q_stricmp (cmd, "say_team") == 0)
	{
		Cmd_Say_f (ent, true, false);
		return;
	}
	if (Q_stricmp (cmd, "score") == 0)
	{
		Cmd_Score_f (ent);
		return;
	}

	if (level.intermissiontime)
		return;

	else if (Q_stricmp (cmd, "use") == 0)
		Cmd_Use_f (ent, gi.args());
	else if (Q_stricmp (cmd, "toggleinventory") == 0)
		Cmd_ToggleInventory_f (ent);
	else if (Q_stricmp (cmd, "invnextw") == 0)
		SelectNextItem (ent, IT_WEAPON);
	else if (Q_stricmp (cmd, "invprevw") == 0)
		SelectPrevItem (ent, IT_WEAPON);
	else if (Q_stricmp (cmd, "invnextp") == 0)
		SelectNextItem (ent, IT_DEFENSE);
	else if (Q_stricmp (cmd, "invprevp") == 0)
		SelectPrevItem (ent, IT_DEFENSE);
	else if (Q_stricmp (cmd, "weapprev") == 0)
		Cmd_WeapPrev_f (ent);
	else if (Q_stricmp (cmd, "weapnext") == 0)
		Cmd_WeapNext_f (ent);
	else if (Q_stricmp (cmd, "defprev") == 0)
		Cmd_DefPrev_f (ent);
	else if (Q_stricmp (cmd, "defnext") == 0)
		Cmd_DefNext_f (ent);
	else if (Q_stricmp (cmd, "weaplast") == 0)
		Cmd_WeapLast_f (ent);
	else if (Q_stricmp (cmd, "kill") == 0)
		Cmd_Kill_f (ent);
	else if (Q_stricmp (cmd, "spawn") == 0)
		Cmd_SpawnEntity_f (ent);
	else if (Q_stricmp (cmd, "nextmonsterframe") == 0)
		Cmd_NextMonsterFrame_f(ent);
	else if (Q_stricmp (cmd, "crazymonsters") == 0)
		Cmd_CrazyMonsters_f (ent);
	else if (Q_stricmp (cmd, "angermonsters") == 0)
		Cmd_AngerMonsters_f (ent);
	else if (Q_stricmp (cmd, "showcoords") == 0)
		Cmd_ShowCoords_f (ent);
	else if (Q_stricmp (cmd, "testfx") == 0)
		Cmd_TestFX_f(ent);
	else if (Q_stricmp (cmd, "gameversion") == 0)
	{
		gi.cprintf (ent, PRINT_HIGH, "%s : %s\n", GAMEVERSION, __DATE__);
	}

	else if (Q_stricmp (cmd, "playbetter") == 0)	//	else if (Q_stricmp (cmd, "god") == 0)
		Cmd_God_f (ent);
	else if (Q_stricmp (cmd, "kiwi") == 0)			//	else if (Q_stricmp (cmd, "noclip") == 0)
		Cmd_Noclip_f (ent);
	else if (Q_stricmp (cmd, "victor") == 0)		//	else if (Q_stricmp (cmd, "notarget") == 0)
		Cmd_Notarget_f (ent);
	else if (Q_stricmp (cmd, "suckitdown") == 0)	//	else if (Q_stricmp (cmd, "give") == 0)
		Cmd_Give_f (ent);
	else if (Q_stricmp (cmd, "twoweeks") == 0)		//	else if (Q_stricmp (cmd, "powerup") == 0)
		Cmd_Powerup_f (ent);
	else if (Q_stricmp (cmd, "meatwagon") == 0)		//	else if (Q_stricmp (cmd, "killmonsters") == 0)
		Cmd_KillMonsters_f (ent);

	else if (Q_stricmp (cmd, "fov") == 0)
	{
		ent->client->ps.fov = atoi(gi.argv(1));
		if (ent->client->ps.fov < 1)
			ent->client->ps.fov = 90;
		else if (ent->client->ps.fov > 160)
			ent->client->ps.fov = 160;
	}
	else	// anything that doesn't match a command will be a chat
		Cmd_Say_f (ent, false, true);
}