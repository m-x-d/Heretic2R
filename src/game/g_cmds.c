#include "g_local.h"
#include "m_player.h"
#include "vector.h"
#include "g_items.h"
#include "random.h"
#include "g_playstats.h"
#include "p_actions.h"
#include "p_anims.h"
#include "p_main.h"
#include "p_funcs.h"
#include "g_itemstats.h"
#include "cl_strings.h"


extern gitem_armor_t silver_armor_info;
extern gitem_armor_t gold_armor_info;
extern void MorphPlayerToChicken(edict_t *self, edict_t *caster);
extern qboolean AddWeaponToInventory(gitem_t *it,edict_t *player);
extern void AddDefenseToInventory(gitem_t *it,edict_t *player);
extern player_export_t	playerExport;	// interface to player DLL.
extern void ClientUserinfoChanged (edict_t *ent, char *userinfo);

qboolean CheckFlood(edict_t *ent);
void ED_CallSpawn (edict_t *ent);
void MorphPlayerToChicken(edict_t *self, edict_t *caster);

int		self_spawn = FALSE;


char *ClientSkinTeam (edict_t *ent)
{
	char		*p;
	static char	value[512];

	value[0] = 0;

	if (!ent->client)
		return value;

	strcpy(value, Info_ValueForKey (ent->client->playerinfo.pers.userinfo, "skin"));

	p = strchr(value, '/');
	if (!p)
		return value;

	// if ((int)(dmflags->value) & DF_SKINTEAMS)
	return ++p;
}


char *ClientModelTeam (edict_t *ent)
{
	char		*p;
	static char	value[512];

	value[0] = 0;

	if (!ent->client)
		return value;

	strcpy(value, Info_ValueForKey (ent->client->playerinfo.pers.userinfo, "skin"));

	p = strchr(value, '/');
	if (!p)
	{	// If no slash, then assume male
		strcpy(value, "male");
		return value;
	}

	*p = 0;

	// if ((int)(dmflags->value) & DF_SKINTEAMS)
	return value;
}


qboolean OnSameTeam (edict_t *ent1, edict_t *ent2)
{
	char	ent1Team [512];
	char	ent2Team [512];

	if (!((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS)))
		return false;

	if ((int)(dmflags->value) & DF_SKINTEAMS)
	{
		strcpy (ent1Team, ClientSkinTeam (ent1));
		strcpy (ent2Team, ClientSkinTeam (ent2));

		if (stricmp(ent1Team, ent2Team) == 0)
			return true;
	}
	else
	{
		strcpy (ent1Team, ClientModelTeam (ent1));
		strcpy (ent2Team, ClientModelTeam (ent2));

		if (stricmp(ent1Team, ent2Team) == 0)
			return true;
	}

	return false;
}

void SelectNextItem (edict_t *ent, int itflags)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;

	if(sv_cinematicfreeze->value)
		return;

	cl = ent->client;

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (cl->playerinfo.pers.selected_item + i)%MAX_ITEMS;
		if (!cl->playerinfo.pers.inventory.Items[index])
			continue;

		it = &playerExport.p_itemlist[index];
		if (!it->use)
			continue;
		if (!(it->flags & itflags))
			continue;

		cl->playerinfo.pers.selected_item = index;

		return;
	}

	cl->playerinfo.pers.selected_item = -1;
}

void SelectPrevItem (edict_t *ent, int itflags)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;

	if(sv_cinematicfreeze->value)
		return;

	cl = ent->client;

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (cl->playerinfo.pers.selected_item + MAX_ITEMS - i) % MAX_ITEMS;
		if (!cl->playerinfo.pers.inventory.Items[index])
			continue;

		it = &playerExport.p_itemlist[index];
		if (!it->use)
			continue;
		if (!(it->flags & itflags))
			continue;

		cl->playerinfo.pers.selected_item = index;
		cl->playerinfo.pers.defence = it;

		return;
	}

	cl->playerinfo.pers.selected_item = -1;
}

void ValidateSelectedItem (edict_t *ent)
{
	gclient_t	*cl;

	cl = ent->client;

	if (cl->playerinfo.pers.inventory.Items[cl->playerinfo.pers.selected_item])
		return;		// valid

	SelectNextItem (ent, -1);
}

/*
==================
Cmd_Give_f

Give items to a client
==================
*/
void Cmd_Give_f (edict_t *ent)
{
	char		*name;
	gitem_t		*it;
	int			index;
	int			i;
	qboolean	give_all;

	if (deathmatch->value && !sv_cheats->value)
	{
		gi.gamemsg_centerprintf (ent, GM_NOCHEATS);
		return;
	}

	name = gi.args();
	// FOR TESTING ONLY !

	if ((Q_stricmp(name, "level") == 0))
	{

		if(level.offensive_weapons&4)
		{	
			it=P_FindItem("hell");
			AddWeaponToInventory(it,ent);
		}

		if(level.offensive_weapons&8)
		{	
			it=P_FindItem("array");
			AddWeaponToInventory(it,ent);
		}

		if(level.offensive_weapons&16)
		{	
			it=P_FindItem("rain");
			AddWeaponToInventory(it,ent);
		}

		if(level.offensive_weapons&32)
		{	
			it=P_FindItem("sphere");
			AddWeaponToInventory(it,ent);
		}

		if(level.offensive_weapons&64)
		{	
			it=P_FindItem("phoen");
			AddWeaponToInventory(it,ent);
		}

		if(level.offensive_weapons&128)
		{	
			it=P_FindItem("mace");
			AddWeaponToInventory(it,ent);
		}

		if(level.offensive_weapons&256)
		{	
			it=P_FindItem("fwall");
			AddWeaponToInventory(it,ent);
		}

		if(level.defensive_weapons&1)
		{	
			it=P_FindItem("ring");
			AddDefenseToInventory(it,ent);
		}

		if(level.defensive_weapons&2)
		{
			it=P_FindItem("lshield");
			AddDefenseToInventory(it,ent);
		}

		if(level.defensive_weapons&4)
		{	
			it=P_FindItem("tele");
			AddDefenseToInventory(it,ent);
		}

		if(level.defensive_weapons&8)
		{	
			it=P_FindItem("morph");
			AddDefenseToInventory(it,ent);
		}

		if(level.defensive_weapons&16)
		{	
			it=P_FindItem("meteor");
			AddDefenseToInventory(it,ent);	
		}

		SetupPlayerinfo_effects(ent);
		P_PlayerUpdateModelAttributes(&ent->client->playerinfo);
		WritePlayerinfo_effects(ent);
		return;
	}

	if (Q_stricmp(name, "all") == 0)
		give_all = true;
	else
		give_all = false;

	if (give_all || Q_stricmp(name, "health") == 0)
	{
		if (gi.argc() == 3)
			ent->health += atoi(gi.argv(2));
		else
			ent->health = ent->max_health;
		
		if(give_all || ent->health == ent->max_health)
			ResetPlayerBaseNodes (ent);//put back all your limbs!

		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "weapons") == 0)
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = playerExport.p_itemlist + i;
			if (!it->pickup)
				continue;
			if (!(it->flags & IT_WEAPON))
				continue;

#if DEMO_CODE
			// don't allow certain things for demo
			if ((int)it->info)
				continue;
#endif


			ent->client->playerinfo.pers.inventory.Items[i] += 1;

			if((it->playeranimseq == ASEQ_WRRBOW_GO)||(it->playeranimseq == ASEQ_WPHBOW_GO))
			{
				// This is a bow, put the bow on his back.

				if (it->tag == ITEM_WEAPON_PHOENIXBOW)
					ent->client->playerinfo.pers.bowtype = BOW_TYPE_PHOENIX;
				else
					ent->client->playerinfo.pers.bowtype = BOW_TYPE_REDRAIN;

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
		for (i=0 ; i<game.num_items ; i++)
		{
			it = playerExport.p_itemlist + i;
			if (!it->pickup)
				continue;
			if (!(it->flags & IT_DEFENSE))
				continue;

#if DEMO_CODE
			// don't allow certain things for demo
			if ((int)it->info)
				continue;
#endif


			ent->client->playerinfo.pers.inventory.Items[i] += 1;
		}

		// if we don't already have a defence item, make the ring default
		if (ent->client->playerinfo.pers.defence == NULL)
			ent->client->playerinfo.pers.defence=P_FindItem("ring");

		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "mana") == 0)
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = playerExport.p_itemlist + i;
			if (!it->pickup)
				continue;
			if (!(it->flags & IT_AMMO))
				continue;
			Add_Ammo (ent, it, 1000);
		}
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "armor") == 0)
	{
		if (ent->client->playerinfo.pers.armortype == ARMOR_TYPE_NONE)
		{
			ent->client->playerinfo.pers.armor_count = silver_armor_info.max_armor;
			ent->client->playerinfo.pers.armortype = ARMOR_TYPE_SILVER;
		}
		else	// We'll assume there's armor, so load up with gold.
		{
			ent->client->playerinfo.pers.armor_count = gold_armor_info.max_armor;
			ent->client->playerinfo.pers.armortype = ARMOR_TYPE_GOLD;

		}

		SetupPlayerinfo_effects(ent);
		P_PlayerUpdateModelAttributes(&ent->client->playerinfo);
		WritePlayerinfo_effects(ent);

		if (!give_all)
			return;
	}

	// Give all does not give staff powerup
	if (Q_stricmp(name, "staff") == 0)
	{
		if (ent->client->playerinfo.pers.stafflevel < (STAFF_LEVEL_MAX-1))
			ent->client->playerinfo.pers.stafflevel++;
		else
			ent->client->playerinfo.pers.stafflevel = STAFF_LEVEL_BASIC;

		gi.dprintf("Setting staff level to %d\n", ent->client->playerinfo.pers.stafflevel);

		SetupPlayerinfo_effects(ent);
		P_PlayerUpdateModelAttributes(&ent->client->playerinfo);
		WritePlayerinfo_effects(ent);
		
		return;
	}

	// Give all does not give lungs
	if (Q_stricmp(name, "lungs") == 0)
	{

		// add gam time + 30 secs to lungs timer

		ent->client->playerinfo.lungs_timer = LUNGS_DURATION;
		ent->air_finished = level.time + HOLD_BREATH_TIME;

		return;
	}

	// Give all does not give powerups
	if (Q_stricmp(name, "powerup") == 0)
	{

		// add gam time + 30 secs to powerup timer

		ent->client->playerinfo.powerup_timer = POWERUP_DURATION + level.time;

		return;
	}

	// Give all does not give reflection.
	if (Q_stricmp(name, "reflection") == 0)
	{

		// add gam time + 30 secs to reflect timer

		// add some time in on the timer for the reflectivity
		ent->client->playerinfo.reflect_timer = level.time + REFLECT_DURATION_SINGLE;
	
		// turn on the relection at the client effect end through client flags that are passed down
		ent->s.renderfx |= RF_REFLECTION;
	
		return;
	}

	// Give all does not give ghost
	if (Q_stricmp(name, "ghost") == 0)
	{
		// add some time in on the timer for the ghost effect
		ent->client->playerinfo.ghost_timer = level.time + GHOST_DURATION;

		// turn on the ghosting at the client effect end through client flags that are passed down
		ent->s.renderfx |= RF_TRANS_GHOST;
		return;
	}

	// Give all does not give chicken.
	if (Q_stricmp(name, "chicken") == 0)
	{
		MorphPlayerToChicken(ent, ent);
	
		return;
	}

	// Give all does not give plague
	if (Q_stricmp(name, "plague") == 0)
	{
		char	userinfo[MAX_INFO_STRING];
		
		if (ent->client->playerinfo.plaguelevel < PLAGUE_NUM_LEVELS-1)
			ent->client->playerinfo.plaguelevel++;
		else
			ent->client->playerinfo.plaguelevel=0;

		gi.dprintf("Setting plague level to %d\n", ent->client->playerinfo.plaguelevel);

		memcpy (userinfo, ent->client->playerinfo.pers.userinfo, sizeof(userinfo));
		ClientUserinfoChanged (ent, userinfo);

		SetupPlayerinfo_effects(ent);
		P_PlayerUpdateModelAttributes(&ent->client->playerinfo);
		WritePlayerinfo_effects(ent);
	
		return;
	}

	if (give_all)
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = playerExport.p_itemlist + i;
			if ((!it->pickup) && !(it->flags & IT_PUZZLE))
				continue;
			if (it->flags & (IT_ARMOR|IT_WEAPON|IT_AMMO|IT_DEFENSE))
				continue;

			ent->client->playerinfo.pers.inventory.Items[i] = 1;
		}
		return;
	}

	it = P_FindItem (name);
	if (!it)
	{
		name = gi.argv(1);
		it = P_FindItem (name);
		if (!it)
		{
			gi.dprintf ("unknown item\n");
			return;
		}
	}

	if (!it->pickup)
	{
		gi.dprintf ("non-pickup item\n");
		return;
	}

#if DEMO_CODE
			// don't allow certain things for demo
	if ((int)it->info)
	{
		gi.cprintf (ent, PRINT_HIGH,"Non Selectable Item in Demo\n");
		return;
	}
#endif

	index = ITEM_INDEX(it);

	if (it->flags & IT_WEAPON)
	{
		ent->client->playerinfo.pers.inventory.Items[index] += 1;
	}
	else if (it->flags & IT_AMMO)
	{
		if (gi.argc() == 3)
			ent->client->playerinfo.pers.inventory.Items[index] += atoi(gi.argv(2));
		else
			ent->client->playerinfo.pers.inventory.Items[index] += it->quantity;
	}
	else
	{
		ent->client->playerinfo.pers.inventory.Items[index] += 1;
	}

 	// if we don't already have a defence item, make this defence item default
	if ((ent->client->playerinfo.pers.defence == NULL) && (it->flags & IT_DEFENSE))
			ent->client->playerinfo.pers.defence=it;


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
#if DEMO_CODE
	gi.cprintf(ent, PRINT_HIGH, "Cannot spawn items in the Heretic II Demo.\n");
#else
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
#endif
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

// Flood protection

qboolean CheckFlood(edict_t *ent)
{
	int					i;

	if (flood_msgs->value)
	{
        if (level.time < ent->client->flood_locktill)
		{
			gi.msgvar_centerprintf(ent, GM_SHUTUP, (int)(ent->client->flood_locktill - level.time));
            return true;
        }
        i = ent->client->flood_whenhead - flood_msgs->value + 1;
        if (i < 0)
		{
            i = (sizeof(ent->client->flood_when) / sizeof(ent->client->flood_when[0])) + i;
		}
		if (ent->client->flood_when[i] && (level.time - ent->client->flood_when[i] < flood_persecond->value))
		{
			ent->client->flood_locktill = level.time + flood_waitdelay->value;
			gi.msgvar_centerprintf(ent, GM_SHUTUP, (int)flood_waitdelay->value);
            return true;
        }
		ent->client->flood_whenhead = (ent->client->flood_whenhead + 1) % (sizeof(ent->client->flood_when) / sizeof(ent->client->flood_when[0]));
		ent->client->flood_when[ent->client->flood_whenhead] = level.time;
	}
	return false;
}

// end
