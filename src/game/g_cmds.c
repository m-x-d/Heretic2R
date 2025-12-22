//
// g_cmds.c
//
// Copyright 1998 Raven Software
//

#include "g_cmds.h" //mxd
#include "cl_strings.h"
#include "g_ai.h" //mxd
#include "g_combat.h" //mxd
#include "g_items.h"
#include "g_itemstats.h"
#include "g_playstats.h"
#include "p_anims.h"
#include "p_dll.h" //mxd
#include "p_client.h" //mxd
#include "p_hud.h" //mxd
#include "p_morph.h" //mxd
#include "p_view.h" //mxd
#include "Random.h"
#include "Vector.h"
#include "g_local.h"

qboolean self_spawn; // True when spawned manually using 'spawn' ccmd.

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
		pos += ARRAY_SIZE(ent->client->flood_when);

	if (ent->client->flood_when[pos] > 0.0f && (level.time - ent->client->flood_when[pos] < flood_persecond->value))
	{
		ent->client->flood_locktill = level.time + flood_waitdelay->value;
		gi.msgvar_centerprintf(ent, GM_SHUTUP, (int)flood_waitdelay->value);

		return true;
	}

	ent->client->flood_whenhead++;
	ent->client->flood_whenhead %= ARRAY_SIZE(ent->client->flood_when);
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

	const char* name = gi.argv(1); //H2_BUGFIX: mxd. gi.args() in original logic (so "give health NNN" didn't work).

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

		Player_UpdateModelAttributes(ent); //mxd

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

		if (give_all || ent->health >= ent->max_health) //mxd. 'ent->health == ent->max_health' in original logic.
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
				Player_UpdateModelAttributes(ent); //mxd
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

		Player_UpdateModelAttributes(ent); //mxd

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
		Player_UpdateModelAttributes(ent); //mxd

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
		MorphPlayerToChickenStart(ent);
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
		Player_UpdateModelAttributes(ent); //mxd

		return;
	}

	// Give puzzle items.
	if (give_all || Q_stricmp(name, "puzzle") == 0) //mxd. +give puzzle option.
	{
		for (int i = 0; i < game.num_items; i++)
		{
			const gitem_t* item = &playerExport.p_itemlist[i];
			if (item->pickup != NULL && (item->flags & IT_PUZZLE))
				pers->inventory.Items[i] = 1;
		}

		return;
	}

	// Give specific item by name, with optional count.
	gitem_t* item = P_FindItem(name);
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

// Sets client to godmode.
static void Cmd_God_f(edict_t* ent)
{
	if (DEATHMATCH && !SV_CHEATS)
	{
		gi.gamemsg_centerprintf(ent, GM_NOCHEATS);
	}
	else
	{
		ent->flags ^= FL_GODMODE;
		gi.cprintf(ent, PRINT_HIGH, ((ent->flags & FL_GODMODE) ? "godmode ON\n" : "godmode OFF\n"));
	}
}

// Sets client to notarget.
static void Cmd_Notarget_f(edict_t* ent)
{
	if (DEATHMATCH && !SV_CHEATS)
	{
		gi.gamemsg_centerprintf(ent, GM_NOCHEATS);
	}
	else
	{
		ent->flags ^= FL_NOTARGET;
		gi.cprintf(ent, PRINT_HIGH, ((ent->flags & FL_NOTARGET) ? "notarget ON\n" : "notarget OFF\n"));
	}
}

static void Cmd_Noclip_f(edict_t* ent)
{
	if (DEATHMATCH && !SV_CHEATS)
	{
		gi.gamemsg_centerprintf(ent, GM_NOCHEATS);
	}
	else
	{
		ent->movetype = ((ent->movetype == PHYSICSTYPE_NOCLIP) ? PHYSICSTYPE_STEP : PHYSICSTYPE_NOCLIP);
		gi.cprintf(ent, PRINT_HIGH, ((ent->movetype == PHYSICSTYPE_NOCLIP) ? "noclip ON\n" : "noclip OFF\n"));
	}
}

static void Cmd_Powerup_f(const edict_t* ent)
{
	if (DEATHMATCH && !SV_CHEATS)
	{
		gi.gamemsg_centerprintf(ent, GM_NOCHEATS);
		return;
	}

	assert(ent->client);

	if (ent->client->playerinfo.powerup_timer > level.time)
	{
		ent->client->playerinfo.powerup_timer = level.time - 0.1f;
		gi.cprintf(ent, PRINT_HIGH, "Powerup OFF\n");
	}
	else
	{
		ent->client->playerinfo.powerup_timer = level.time + (60.0f * 60.0f * 24.0f); // One full day.
		gi.cprintf(ent, PRINT_HIGH, "Powerup ON\n");
	}
}

// Use an inventory item.
void Cmd_Use_f(edict_t* ent, char* name)
{
	qboolean cast_me;

	if (SV_CINEMATICFREEZE)
		return;

	assert(ent != NULL && ent->client != NULL);
	playerinfo_t* info = &ent->client->playerinfo;

	if (name[0] == '*')
	{
		// Cast automatically with asterisk before name. THIS ONLY WORKS WITH DEFENSIVE ITEMS.
		cast_me = true;
		name++;
	}
	else
	{
		cast_me = false;
	}

	gitem_t* item = P_FindItem(name);

	if (item == NULL)
	{
		gi.cprintf(ent, PRINT_HIGH, "Unknown item: %s\n", name);
		return;
	}

	if (item->use == NULL)
	{
		gi.gamemsg_centerprintf(ent, GM_NOTUSABLE);
		return;
	}

	const int index = ITEM_INDEX(item);

	if (info->pers.inventory.Items[index] == 0)
	{
		// Index is two off for weapons, since we can never run out of the staff or the flying fist.
		const short msg_id = (short)((item->flags & (IT_WEAPON | IT_DEFENSE)) ? item->msg_nouse : GM_NOITEM); //mxd
		gi.gamemsg_centerprintf(ent, msg_id);

		return;
	}

	if (cast_me && (item->flags & IT_DEFENSE) && item->weaponthink != NULL && ent->dead_state != DEAD_DEAD && info->deadflag != DEAD_DYING)
	{
		if (info->leveltime > info->defensive_debounce)
		{
			// Do something only if the debounce is okay.
			info->pers.lastdefence = info->pers.defence;
			info->pers.defence = item;

			if (P_Defence_CurrentShotsLeft(info, 1) > 0)
			{
				// Only if there is ammo.
				item->weaponthink(ent, "");

				if (info->pers.defence != NULL && info->pers.defence->ammo != NULL)
					info->def_ammo_index = ITEM_INDEX(P_FindItem(info->pers.defence->ammo));
				else
					info->def_ammo_index = 0;

				info->defensive_debounce = info->leveltime + DEFENSE_DEBOUNCE;
			}
			else
			{
				// Play a sound to tell the player they're out of mana (also done in PlayerUpdate() --mxd).
				char* snd_name; //mxd

				if (info->edictflags & FL_AVERAGE_CHICKEN)
					snd_name = va("Monsters/chicken/pain%i.wav", irand(1, 2)); // Sounds unused in original logic --mxd.
				else if (info->edictflags & FL_SUPER_CHICKEN)
					snd_name = va("Monsters/superchicken/pain%i.wav", irand(1, 2)); // Sounds unused in original logic --mxd.
				else
					snd_name = "*nomana.wav";

				gi.sound(ent, CHAN_VOICE, gi.soundindex(snd_name), 0.75f, ATTN_NORM, 0.0f);
			}

			// Put the ammo back.
			info->pers.defence = info->pers.lastdefence;
			info->pers.lastdefence = item;
		}
	}
	else
	{
		item->use(info, item);
	}
}

void Cmd_WeapPrev_f(const edict_t* ent)
{
	gclient_t* cl = ent->client;

	if (cl->playerinfo.pers.weapon == NULL || SV_CINEMATICFREEZE)
		return;

	const int selected_weapon = ITEM_INDEX(cl->playerinfo.pers.weapon);

	// Scan for the previous valid one.
	for (int i = MAX_ITEMS - 1; i > 0; i--) //mxd. Bugfix, kinda: original logic eventually wraps back to selected_weapon index.
	{
		const int index = (selected_weapon + i) % MAX_ITEMS;

		if (cl->playerinfo.pers.inventory.Items[index] == 0)
			continue;

		gitem_t* wpn = &playerExport.p_itemlist[index];

		if (wpn->use == NULL || !(wpn->flags & IT_WEAPON))
			continue;

		// If we are in water, skip weapons that require ammo.
		if (ent->waterlevel > 1 && (wpn->tag == ITEM_WEAPON_HELLSTAFF || wpn->tag == ITEM_WEAPON_REDRAINBOW || wpn->tag == ITEM_WEAPON_PHOENIXBOW))
			continue;

		wpn->use(&cl->playerinfo, wpn);

		if (cl->playerinfo.pers.newweapon == wpn)
			return;	// Successful.
	}
}

static void Cmd_WeapNext_f(const edict_t* ent)
{
	gclient_t* cl = ent->client;

	if (!cl->playerinfo.pers.weapon || SV_CINEMATICFREEZE)
		return;

	const int selected_weapon = ITEM_INDEX(cl->playerinfo.pers.weapon);

	// Scan for the next valid one.
	for (int i = 1; i < MAX_ITEMS; i++) //mxd. Bugfix, kinda: original logic eventually wraps back to selected_weapon index.
	{
		const int index = (selected_weapon + i) % MAX_ITEMS;

		if (cl->playerinfo.pers.inventory.Items[index] == 0)
			continue;

		gitem_t* wpn = &playerExport.p_itemlist[index];

		if (wpn->use == NULL || !(wpn->flags & IT_WEAPON))
			continue;

		// If we are in water, skip weapons that require ammo.
		if (ent->waterlevel > 1 && (wpn->tag == ITEM_WEAPON_HELLSTAFF || wpn->tag == ITEM_WEAPON_REDRAINBOW || wpn->tag == ITEM_WEAPON_PHOENIXBOW))
			continue;

		wpn->use(&cl->playerinfo, wpn);

		if (cl->playerinfo.pers.newweapon == wpn)
			return;	// Successful.
	}
}

static void Cmd_WeapLast_f(const edict_t* ent)
{
	if (SV_CINEMATICFREEZE)
		return;

	gclient_t* cl = ent->client;
	const client_persistant_t* pers = &cl->playerinfo.pers; //mxd

	if (pers->weapon == NULL || pers->lastweapon == NULL)
		return;

	const int index = ITEM_INDEX(pers->lastweapon);

	if (pers->inventory.Items[index] == 0)
		return;

	gitem_t* wpn = &playerExport.p_itemlist[index];

	if (wpn->use != NULL && (wpn->flags & IT_WEAPON))
		wpn->use(&cl->playerinfo, wpn);
}

static void Cmd_DefPrev_f(const edict_t* ent)
{
	if (SV_CINEMATICFREEZE)
		return;

	gclient_t* cl = ent->client;
	const client_persistant_t* pers = &cl->playerinfo.pers; //mxd
	const int selected_defence = ((pers->defence != NULL) ? ITEM_INDEX(pers->defence) : 1);

	// Scan for the previous valid one.
	for (int i = MAX_ITEMS - 1; i > 0; i--) //mxd. Bugfix, kinda: original logic eventually wraps back to selected_defence index.
	{
		const int index = (selected_defence + i) % MAX_ITEMS;

		if (pers->inventory.Items[index] == 0)
			continue;

		gitem_t* def = &playerExport.p_itemlist[index];

		if (def->use == NULL || !(def->flags & IT_DEFENSE))
			continue;

		def->use(&cl->playerinfo, def);

		if (pers->defence == def)
		{
			//TODO: shouldn't defence item itself play select sound?
			//TODO: weapon select sounds use non-null SND_PRED ids. Should use here too?..
			cl->playerinfo.G_Sound((byte)SND_PRED_NULL, level.time, ent, CHAN_AUTO, cl->playerinfo.G_SoundIndex("Weapons/DefenseSelect.wav"), 1.0f, ATTN_NORM, 0.0f);
			return; // Successful.
		}
	}
}

static void Cmd_DefNext_f(const edict_t* ent)
{
	if (SV_CINEMATICFREEZE)
		return;

	gclient_t* cl = ent->client;
	const client_persistant_t* pers = &cl->playerinfo.pers; //mxd
	const int selected_defence = ((pers->defence != NULL) ? ITEM_INDEX(pers->defence) : 1);

	// Scan for the next valid one.
	for (int i = 1; i < MAX_ITEMS; i++) //mxd. Bugfix, kinda: original logic eventually wraps back to selected_defence index.
	{
		const int index = (selected_defence + i) % MAX_ITEMS;

		if (pers->inventory.Items[index] == 0)
			continue;

		gitem_t* def = &playerExport.p_itemlist[index];

		if (def->use == NULL || !(def->flags & IT_DEFENSE))
			continue;

		def->use(&cl->playerinfo, def);

		if (pers->defence == def)
		{
			//TODO: shouldn't defence item itself play select sound?
			//TODO: weapon select sounds use non-null SND_PRED ids. Should use here too?..
			cl->playerinfo.G_Sound((byte)SND_PRED_NULL, level.time, ent, CHAN_AUTO, cl->playerinfo.G_SoundIndex("Weapons/DefenseSelect.wav"), 1.0f, ATTN_NORM, 0.0f);
			return; // Successful.
		}
	}
}

static void Cmd_Kill_f(edict_t* ent)
{
	if (ent->client->flood_nextkill > level.time)
	{
		gi.msgvar_centerprintf(ent, GM_NOKILL, (int)(ent->client->flood_nextkill - level.time) + 1);
		return;
	}

	ent->flags &= ~FL_GODMODE;

	if (ent->health < 0)
		return;

	// Make sure we gib as we don't want bodies lying around everywhere.
	ent->health = -100000;
	ent->client->meansofdeath = MOD_SUICIDE;
	PlayerDie(ent, ent, ent, 100000, vec3_origin);

	// Don't even bother waiting for death frames.
	ent->dead_state = DEAD_DEAD;

	// Put us back in the game.
	ClientRespawn(ent);

	// Set up the next valid suicide time.
	ent->client->flood_nextkill = level.time + flood_killdelay->value;
}

static int PlayerSort(const void* a, const void* b)
{
	const int anum = game.clients[*(const int*)a].ps.stats[STAT_FRAGS];
	const int bnum = game.clients[*(const int*)b].ps.stats[STAT_FRAGS];

	if (anum < bnum)
		return -1;

	if (anum > bnum)
		return 1;

	return 0;
}

static void Cmd_Players_f(const edict_t* ent)
{
	int client_indices[256];

	int clients_count = 0;
	for (int i = 0; i < MAXCLIENTS; i++)
	{
		if (game.clients[i].playerinfo.pers.connected)
		{
			client_indices[clients_count] = i;
			clients_count++;
		}
	}

	// Sort by frags.
	qsort(client_indices, clients_count, sizeof(client_indices[0]), PlayerSort);

	// Print information.
	char message[1024] = { 0 }; //BUGFIX: mxd. 1280 in original logic (exceeds buffer size in PF_cprintf).
	const char* fmt = "%s\n%i player(s)\n"; //mxd
	const int fmt_len = (int)strlen(fmt) + 1; //mxd

	for (int i = 0; i < clients_count; i++)
	{
		char line[64];
		Com_sprintf(line, sizeof(line), "%3i %s\n", game.clients[client_indices[i]].ps.stats[STAT_FRAGS], game.clients[client_indices[i]].playerinfo.pers.netname);

		if (strlen(line) + strlen(message) > sizeof(message) - fmt_len) //mxd. sizeof(large) - 100 in original logic.
		{
			// Can't print all of them in one packet.
			strcat_s(message, sizeof(message), "...\n"); //mxd. strcat -> strcat_s
			break;
		}

		strcat_s(message, sizeof(message), line); //mxd. strcat -> strcat_s
	}

	gi.cprintf(ent, PRINT_HIGH, fmt, message, clients_count);
}

// Spawn an item.
static void Cmd_SpawnEntity_f(const edict_t* ent)
{
	if (DEATHMATCH && !SV_CHEATS)
	{
		gi.gamemsg_centerprintf(ent, GM_NOCHEATS);
		return;
	}

	gi.cprintf(ent, PRINT_HIGH, "Spawning %s\n", gi.argv(1));

	self_spawn = true;

	edict_t* new_ent = G_Spawn();
	new_ent->classname = ED_NewString(gi.argv(1));

	vec3_t forward;
	AngleVectors(ent->s.angles, forward, NULL, NULL);
	Vec3ScaleAssign(100.0f, forward);
	VectorAdd(ent->s.origin, forward, new_ent->s.origin);

	VectorCopy(ent->s.angles, new_ent->s.angles);
	ED_CallSpawn(new_ent);

	self_spawn = false;
}

// Toggle the Inventory Console
static void Cmd_ToggleInventory_f(const edict_t* ent)
{
	ent->client->playerinfo.showpuzzleinventory = !ent->client->playerinfo.showpuzzleinventory;
}

// Kill all non-boss level monsters on a level.
static void Cmd_KillMonsters_f(edict_t* ent)
{
	gi.cprintf(ent, PRINT_HIGH, "Killing all non-boss level monsters\n");

	for (edict_t* e = &g_edicts[0]; e < &g_edicts[globals.num_edicts]; e++)
	{
		if (e->inuse && !e->monsterinfo.c_mode && (e->svflags & SVF_MONSTER) && !(e->svflags & SVF_BOSS))
		{
			gi.dprintf("Killing monster %s\n", e->classname);
			Killed(e, ent, ent, 100000, e->s.origin, MOD_UNKNOWN);
			e->health = 0; //TODO: is this necessary?
		}
	}
}

static void Cmd_CrazyMonsters_f(const edict_t* ent)
{
	gi.cprintf(ent, PRINT_HIGH, "Berzerking all level monsters\n");
	ANARCHY = true;

	for (edict_t* e = &g_edicts[0]; e < &g_edicts[globals.num_edicts]; e++)
	{
		if (e->inuse && e->svflags & SVF_MONSTER)
		{
			// Pick random enemy.
			edict_t* enemy = NULL;
			while (enemy == NULL || !enemy->inuse || !(enemy->svflags & SVF_MONSTER) || enemy->health < 0 || enemy == e)
				enemy = &g_edicts[irand(0, globals.num_edicts - 1)]; //BUGFIX: mxd. irand(0, globals.num_edicts) in original logic, which can potentially cause array overrun.

			e->enemy = enemy;
			AI_FoundTarget(e, false);
		}
	}
}

static void Cmd_AngerMonsters_f(edict_t* ent)
{
	gi.cprintf(ent, PRINT_HIGH, "Angering all level monsters\n");

	for (edict_t* e = &g_edicts[0]; e < &g_edicts[globals.num_edicts]; e++)
	{
		if (e->inuse && e->svflags & SVF_MONSTER)
		{
			e->enemy = ent;
			AI_FoundTarget(e, false);
		}
	}
}

static void Cmd_Say_f(const edict_t* ent, qboolean team, const qboolean arg0) //mxd. Rewritten to use single text buffer.
{
	char text[2048] = { 0 };

	if (CheckFlood(ent) || (gi.argc() < 2 && !arg0))
		return;

	if (!(DMFLAGS & (DF_MODELTEAMS | DF_SKINTEAMS)))
		team = false;

	if (arg0)
	{
		strcat_s(text, sizeof(text), va("%s %s", gi.argv(0), gi.args())); //mxd. strcat -> strcat_s; use va().
	}
	else
	{
		char* p = gi.args();

		if (*p == '"') // Strip quotes.
		{
			p++;
			p[strlen(p) - 1] = 0;
		}

		strcat_s(text, sizeof(text), p); //mxd. strcat -> strcat_s
	}

	// Don't let text be too long for malicious reasons.
	if (strlen(text) > 150)
		text[150] = 0;

	strcat_s(text, sizeof(text), "\n"); //mxd. strcat -> strcat_s

	if (DEDICATED)
		gi.cprintf(NULL, PRINT_CHAT, "%s: %s", ent->client->playerinfo.pers.netname, text);

	const int color = (team ? 1 : 0);
	for (int i = 1; i <= game.maxclients; i++)
	{
		const edict_t* other = &g_edicts[i];

		if (!other->inuse || other->client == NULL || (team && !OnSameTeam(ent, other)))
			continue;

		gi.clprintf(other, ent, color, "%s", text);
	}
}

static void Cmd_ShowCoords_f(const edict_t* ent)
{
	assert(ent->client);

	Com_Printf("Player location: %i %i %i\n", (int)ent->s.origin[0], (int)ent->s.origin[1], (int)ent->s.origin[2]);
	Com_Printf("       facing:   %2.2f\n", ent->client->aimangles[YAW]);
	Com_Printf("       pitch:    %2.2f\n", -ent->client->aimangles[PITCH]);
}

#pragma endregion

void ClientCommand(edict_t* ent)
{
	if (ent->client == NULL)
		return; // Not fully in game yet.

	const char* cmd = gi.argv(0);

	if (Q_stricmp(cmd, "players") == 0)
	{
		Cmd_Players_f(ent);
		return;
	}

	if (Q_stricmp(cmd, "say") == 0)
	{
		Cmd_Say_f(ent, false, false);
		return;
	}

	if (Q_stricmp(cmd, "say_team") == 0)
	{
		Cmd_Say_f(ent, true, false);
		return;
	}

	if (Q_stricmp(cmd, "score") == 0)
	{
		Cmd_Score_f(ent);
		return;
	}

	if (level.intermissiontime > 0.0f)
		return;

	if (Q_stricmp(cmd, "use") == 0)
	{
		Cmd_Use_f(ent, gi.args());
		return;
	}

	if (Q_stricmp(cmd, "toggleinventory") == 0)
	{
		Cmd_ToggleInventory_f(ent);
		return;
	}

	if (Q_stricmp(cmd, "invnextw") == 0)
	{
		SelectNextItem(ent, IT_WEAPON);
		return;
	}

	if (Q_stricmp(cmd, "invprevw") == 0)
	{
		SelectPrevItem(ent, IT_WEAPON);
		return;
	}

	if (Q_stricmp(cmd, "invnextp") == 0)
	{
		SelectNextItem(ent, IT_DEFENSE);
		return;
	}

	if (Q_stricmp(cmd, "invprevp") == 0)
	{
		SelectPrevItem(ent, IT_DEFENSE);
		return;
	}

	if (Q_stricmp(cmd, "weapprev") == 0)
	{
		Cmd_WeapPrev_f(ent);
		return;
	}

	if (Q_stricmp(cmd, "weapnext") == 0)
	{
		Cmd_WeapNext_f(ent);
		return;
	}

	if (Q_stricmp(cmd, "defprev") == 0)
	{
		Cmd_DefPrev_f(ent);
		return;
	}

	if (Q_stricmp(cmd, "defnext") == 0)
	{
		Cmd_DefNext_f(ent);
		return;
	}

	if (Q_stricmp(cmd, "weaplast") == 0)
	{
		Cmd_WeapLast_f(ent);
		return;
	}

	if (Q_stricmp(cmd, "kill") == 0)
	{
		Cmd_Kill_f(ent);
		return;
	}

	if (Q_stricmp(cmd, "spawn") == 0)
	{
		Cmd_SpawnEntity_f(ent);
		return;
	}

	if (Q_stricmp(cmd, "crazymonsters") == 0)
	{
		Cmd_CrazyMonsters_f(ent);
		return;
	}

	if (Q_stricmp(cmd, "angermonsters") == 0)
	{
		Cmd_AngerMonsters_f(ent);
		return;
	}

	if (Q_stricmp(cmd, "showcoords") == 0)
	{
		Cmd_ShowCoords_f(ent);
		return;
	}

	if (Q_stricmp(cmd, "gameversion") == 0)
	{
		gi.cprintf(ent, PRINT_HIGH, "%s : %s\n", GAMEVERSION, __DATE__);
		return;
	}

	if (Q_stricmp(cmd, "playbetter") == 0 || Q_stricmp(cmd, "god") == 0) //mxd. Re-enable classic cheats.
	{
		Cmd_God_f(ent);
		return;
	}

	if (Q_stricmp(cmd, "kiwi") == 0 || Q_stricmp(cmd, "noclip") == 0) //mxd. Re-enable classic cheats.
	{
		Cmd_Noclip_f(ent);
		return;
	}

	if (Q_stricmp(cmd, "victor") == 0 || Q_stricmp(cmd, "notarget") == 0) //mxd. Re-enable classic cheats.
	{
		Cmd_Notarget_f(ent);
		return;
	}

	if (Q_stricmp(cmd, "suckitdown") == 0 || Q_stricmp(cmd, "give") == 0) //mxd. Re-enable classic cheats.
	{
		Cmd_Give_f(ent);
		return;
	}

	if (Q_stricmp(cmd, "twoweeks") == 0 || Q_stricmp(cmd, "powerup") == 0) //mxd. Re-enable classic cheats.
	{
		Cmd_Powerup_f(ent);
		return;
	}

	if (Q_stricmp(cmd, "meatwagon") == 0 || Q_stricmp(cmd, "killmonsters") == 0) //mxd. Re-enable classic cheats.
	{
		Cmd_KillMonsters_f(ent);
		return;
	}

	// Anything that doesn't match a command will be a chat.
	Cmd_Say_f(ent, false, true);
}