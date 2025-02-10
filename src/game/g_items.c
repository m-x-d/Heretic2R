//
// g_items.c
//
// Copyright 1998 Raven Software
//

#include "g_local.h"
#include "p_dll.h" //mxd
#include "p_item.h"
#include "g_items.h"
#include "g_itemstats.h"
#include "g_weapon.h"
#include "FX.h"
#include "Random.h"
#include "Vector.h"

#define HEALTH_IGNORE_MAX	1

#define ITEM_COOP_ONLY		1
#define ITEM_NO_DROP		2

#pragma region ========================== RESPAWN LOGIC ==========================

static void RespawnedThink(edict_t* ent) //TODO: can't we just clear ent->think in place instead of doing this?
{
	ent->think = NULL;
}

static void DoRespawn(edict_t* ent)
{
	// For team, respawn random member of the team (?)
	if (ent->team != NULL)
	{
		int count;
		edict_t* master = ent->teammaster;

		ent = master;
		for (count = 0; ent != NULL; count++)
			ent = ent->chain;

		const int choice = irand(0, count - 1);

		ent = master;
		for (int i = 0; i < choice; i++)
			ent = ent->chain;
	}

	ent->solid = SOLID_TRIGGER;
	gi.linkentity(ent);

	// Re-enable the persistent effect.
	ent->s.effects &= ~EF_DISABLE_ALL_CFX;

	// So it'll get displayed again.
	ent->s.effects |= EF_ALWAYS_ADD_EFFECTS;

	// And the rest.
	ent->think = RespawnedThink;
	ent->nextthink = level.time + FRAMETIME;
}

static void PreRespawnThink(edict_t* ent)
{
	float delay = ent->delay;

	if (DEATHMATCH)
	{
		const float num_clients = max(2.0f, (float)game.num_clients);
		delay *= sqrtf(2.0f / num_clients); // Spawn more frequently when more players.
	}

	ent->nextthink = level.time + delay - FRAMETIME;
	ent->think = DoRespawn;
}

static void SetRespawn(edict_t* ent)
{
	ent->s.effects |= EF_DISABLE_ALL_CFX; // Disables all the effects on the entity.
	ent->s.effects &= ~EF_ALWAYS_ADD_EFFECTS; // Take off the EF_ALWAYS_ADD_EFFECTS or EF_DISABLE_ALL_CFX won't have an effect.
	ent->solid = SOLID_NOT;

	float delay = ent->delay; //mxd

	if (DEATHMATCH && game.num_clients > 2)
	{
		// No less than 1/4th the delay. //BUGFIX: logic skipped in original version. 
		const float num_clients = min(8.0f, (float)game.num_clients); //mxd

		// So things respawn faster with a lot of players.
		delay /= num_clients * 0.5f;
	}

	ent->think = PreRespawnThink;
	ent->nextthink = level.time + delay;
}

#pragma endregion

#pragma region ========================== PUZZLE ITEM PICKUP LOGIC ==========================

static qboolean Pickup_Puzzle(edict_t* ent, edict_t* other)
{
	if (other->flags & FL_CHICKEN) // Chicken can't into puzzles...
		return false;

	const int item_index = ITEM_INDEX(ent->item); //mxd
	if (other->client->playerinfo.pers.inventory.Items[item_index] == 0)
	{
		other->client->playerinfo.pers.inventory.Items[item_index] = 1;
		gi.gamemsg_centerprintf(other, ent->item->msg_pickup);

		return true;
	}

	return false;
}

#pragma endregion

#pragma region ========================== WEAPON PICKUP LOGIC ==========================

qboolean AddWeaponToInventory(gitem_t* weapon, edict_t* player)
{
	client_persistant_t* pers = &player->client->playerinfo.pers; //mxd
	const int wpn_index = ITEM_INDEX(weapon); //mxd

	// Do we already have this weapon?
	if (pers->inventory.Items[wpn_index] == 0)
	{
		// We don't already have it, so get the weapon and some ammo.
		pers->inventory.Items[wpn_index] = 1;

		// Add ammo.
		int count;
		switch (weapon->tag)
		{
			case ITEM_WEAPON_SWORDSTAFF:
				count = 0;
				break;

			case ITEM_WEAPON_HELLSTAFF:
				count = AMMO_COUNT_HELLSTAFF;
				break;

			case ITEM_WEAPON_REDRAINBOW:
				pers->bowtype = BOW_TYPE_REDRAIN;
				count = AMMO_COUNT_REDRAINBOW;
				break;

			case ITEM_WEAPON_PHOENIXBOW:
				pers->bowtype = BOW_TYPE_PHOENIX;
				count = AMMO_COUNT_PHOENIXBOW;
				break;

			default:
				count = AMMO_COUNT_MOST;
				break;
		}

		if (count > 0)
			Add_Ammo(player, P_FindItem(weapon->ammo), count);

		// If new weapon is a higher value than the one we currently have, swap the current weapon for the new one.
		if (pers->autoweapon && wpn_index > ITEM_INDEX(pers->weapon))
			weapon->use(&player->client->playerinfo, weapon);

		return true;
	}

	// We already have it...
	if (COOP || (DEATHMATCH && (DMFLAGS & DF_WEAPONS_STAY)))
		return false;

	// ...and DF_WEPONS_STAY is off and we're not in coop, so just try to up the ammo counts.
	gitem_t* ammo;
	int count;

	switch (weapon->tag)
	{
		case ITEM_WEAPON_HELLSTAFF:
			ammo = P_FindItemByClassname("item_ammo_hellstaff");
			count = AMMO_COUNT_HELLSTAFF;
			break;

		case ITEM_WEAPON_REDRAINBOW:
			ammo = P_FindItemByClassname("item_ammo_redrain");
			count = AMMO_COUNT_REDRAINBOW;
			break;

		case ITEM_WEAPON_PHOENIXBOW:
			ammo = P_FindItemByClassname("item_ammo_phoenix");
			count = AMMO_COUNT_PHOENIXBOW;
			break;

		default:
			ammo = P_FindItemByClassname("item_mana_offensive_half");
			count = AMMO_COUNT_MOST;
			break;
	}

	return Add_Ammo(player, ammo, count); // Count as added if ammo was added.
}

static qboolean Pickup_Weapon(edict_t* ent, edict_t* other)
{
	if (other->flags & FL_CHICKEN) // Chicken can't into weapons...
		return false;

	if (AddWeaponToInventory(ent->item, other))
	{
		gi.gamemsg_centerprintf(other, ent->item->msg_pickup);
		return true;
	}

	return false; // We already have it.
}

#pragma endregion

#pragma region ========================== DEFENSE PICKUP LOGIC (OR IS IT DEFENCE?..) ==========================

qboolean AddDefenseToInventory(gitem_t* defence, edict_t* player)
{
	client_persistant_t* pers = &player->client->playerinfo.pers; //mxd
	const int def_index = ITEM_INDEX(defence); //mxd

	if (pers->inventory.Items[def_index] == 0)
	{
		pers->inventory.Items[def_index] = 1;

		// Now decide if we want to swap defenses or not.
		if (pers->autoweapon)
			defence->use(&player->client->playerinfo, defence);

		return true;
	}

	return false; // We already have it...
}

static qboolean Pickup_Defense(edict_t* ent, edict_t* other)
{
	if (other->flags & FL_CHICKEN) // Chicken can't into defence...
		return false;

	if (AddDefenseToInventory(ent->item, other))
	{
		gi.gamemsg_centerprintf(other, ent->item->msg_pickup);
		return true;
	}

	return false;
}

#pragma endregion

#pragma region ========================== AMMO PICKUP LOGIC ==========================

static qboolean Add_AmmoToInventory(const edict_t* ent, const gitem_t* ammo, const int count, const int max)
{
	inventory_t* inventory = &ent->client->playerinfo.pers.inventory; //mxd
	const int ammo_index = ITEM_INDEX(ammo);

	if (inventory->Items[ammo_index] < max)
	{
		inventory->Items[ammo_index] = min(max, inventory->Items[ammo_index] + count);
		return true;
	}

	return false;
}

qboolean Add_Ammo(const edict_t* ent, const gitem_t* ammo, const int count)
{
	if (ent->client == NULL)
		return false;

	const client_persistant_t* pers = &ent->client->playerinfo.pers; //mxd

	switch (ammo->tag)
	{
		case ITEM_AMMO_MANA_OFFENSIVE_HALF:
		case ITEM_AMMO_MANA_OFFENSIVE_FULL:
			return Add_AmmoToInventory(ent, P_FindItemByClassname("item_mana_offensive_half"), count, pers->max_offmana);

		case ITEM_AMMO_MANA_DEFENSIVE_HALF:
		case ITEM_AMMO_MANA_DEFENSIVE_FULL:
			return Add_AmmoToInventory(ent, P_FindItemByClassname("item_mana_defensive_half"), count, pers->max_defmana);

		case ITEM_AMMO_MANA_COMBO_QUARTER:
		case ITEM_AMMO_MANA_COMBO_HALF:
			return (Add_AmmoToInventory(ent, P_FindItemByClassname("item_mana_offensive_half"), count, pers->max_offmana) |
				Add_AmmoToInventory(ent, P_FindItemByClassname("item_mana_defensive_half"), count, pers->max_defmana));

		case ITEM_AMMO_REDRAIN:
			return Add_AmmoToInventory(ent, ammo, count, pers->max_redarrow);

		case ITEM_AMMO_PHOENIX:
			return Add_AmmoToInventory(ent, ammo, count, pers->max_phoenarr);

		case ITEM_AMMO_HELLSTAFF:
			return Add_AmmoToInventory(ent, ammo, count, pers->max_hellstaff);

		default:
			return false;
	}
}

static qboolean Pickup_Ammo(edict_t* ent, edict_t* other)
{
	if (other->flags & FL_CHICKEN) // Chicken can't into ammo...
		return false;

	const int count = (ent->count > 0 ? ent->count : ent->item->quantity); //mxd
	if (Add_Ammo(other, ent->item, count))
	{
		gi.gamemsg_centerprintf(other, ent->item->msg_pickup);
		return true;
	}

	return false;
}

#pragma endregion

#pragma region ========================== MANA PICKUP LOGIC ==========================

// Separate routine so we can distinguish between ammo and mana.
static qboolean Pickup_Mana(edict_t* ent, edict_t* other)
{
	return Pickup_Ammo(ent, other);
}

#pragma endregion

#pragma region ========================== HEALTH PICKUP LOGIC ==========================

static qboolean Pickup_Health(edict_t* ent, edict_t* other)
{
	if (other->flags & FL_CHICKEN) // Chicken is not very healthy...
		return false;

	if (!(ent->style & HEALTH_IGNORE_MAX) && other->health >= other->max_health)
		return false;

	other->health += ent->item->quantity;

	if (!(ent->style & HEALTH_IGNORE_MAX) && other->health > other->max_health)
		other->health = other->max_health;

	// Use health pickup to douse fire.
	if (other->fire_damage_time > level.time)
	{
		other->fire_damage_time -= (float)ent->item->quantity / 10.0f;

		if (other->fire_damage_time <= 0.0f)
		{
			other->fire_damage_time = 0.0f;
			other->s.effects |= EF_MARCUS_FLAG1; // Notify the effect to turn itself off.
		}
	}

	if (other->client != NULL)
		player_repair_skin(other);

	gi.gamemsg_centerprintf(other, ent->item->msg_pickup);

	return true;
}

#pragma endregion

#pragma region ========================== GENERIC PICKUP LOGIC ==========================

static void Touch_Item(edict_t* ent, edict_t* other, cplane_t* plane, csurface_t* surf)
{
	// Only alive players can touch items.
	if (strcmp(other->classname, "player") != 0 || other->health <= 0)
		return;

	// Not a grabbable item or player can't hold it.
	if (ent->item->pickup == NULL || !ent->item->pickup(ent, other))
		return;

	//mxd. Skip Pickup_Health() chicken check. Pickup_Health() already checks for chicken.

	gi.sound(other, CHAN_ITEM, gi.soundindex(ent->item->pickup_sound), 1.0f, ATTN_NORM, 0.0f);
	gi.CreateEffect(NULL, FX_PICKUP, 0, ent->s.origin, "");

	G_UseTargets(ent, other);

	// Don't remove weapon/defence/puzzle pickup when coop or dm with DF_WEAPONS_STAY flag.
	if ((ent->item->pickup == Pickup_Weapon || ent->item->pickup == Pickup_Defense || ent->item->pickup == Pickup_Puzzle) && (COOP || (DEATHMATCH && (DMFLAGS & DF_WEAPONS_STAY))))
		return;

	if (ent->flags & FL_RESPAWN)
	{
		SetRespawn(ent); // The item should respawn.
	}
	else
	{
		ent->solid = SOLID_NOT; // Going away for good, so make it noclipping.
		gi.RemoveEffects(&ent->s, 0); // Once picked up, the item is gone forever, so remove it's client effect(s).
		G_SetToFree(ent); // The persistent part is removed from the server here.
	}
}

static void DropItemTempTouch(edict_t* ent, edict_t* other, cplane_t* plane, csurface_t* surf) //mxd. Named 'drop_temp_touch' in original logic.
{
	if (other != ent->owner)
		Touch_Item(ent, other, plane, surf);
}

static void DropItemMakeTouchable(edict_t* ent) //mxd. Named 'drop_make_touchable' in original logic.
{
	ent->touch = Touch_Item;

	if (DEATHMATCH)
	{
		ent->nextthink = level.time + 29;
		ent->think = G_FreeEdict;
	}
}

/*
================
Drop_Item
================
*/

edict_t *Drop_Item (edict_t *ent, gitem_t *item)
{
	edict_t	*dropped;
	vec3_t	forward, right;
	vec3_t	offset;

	dropped = G_Spawn();

	dropped->classname = item->classname;
	dropped->item = item;
	dropped->spawnflags = DROPPED_ITEM;
	dropped->s.effects = item->world_model_flags;
	dropped->s.renderfx = RF_GLOW;
	VectorSet (dropped->mins, -15, -15, -15);
	VectorSet (dropped->maxs, 15, 15, 15);
	gi.setmodel (dropped, dropped->item->world_model);
	dropped->solid = SOLID_TRIGGER;
	dropped->movetype = PHYSICSTYPE_NONE;  
	dropped->touch = DropItemTempTouch;
	dropped->owner = ent;

	if (ent->client)
	{
		trace_t	trace;

		AngleVectors (ent->client->v_angle, forward, right, NULL);
		VectorSet(offset, 24, 0, -16);
		G_ProjectSource (ent->s.origin, offset, forward, right, dropped->s.origin);
		gi.trace (ent->s.origin, dropped->mins, dropped->maxs,
			dropped->s.origin, ent, CONTENTS_SOLID,&trace);
		VectorCopy (trace.endpos, dropped->s.origin);
	}
	else
	{
		AngleVectors (ent->s.angles, forward, right, NULL);
		VectorCopy (ent->s.origin, dropped->s.origin);
	}

	VectorScale (forward, 100, dropped->velocity);
	dropped->velocity[2] = 300;

	dropped->think = DropItemMakeTouchable;
	dropped->nextthink = level.time + 1;

	gi.linkentity (dropped);

	return dropped;
}

/*
================
Use_Item
================
*/
/*
void Use_Item (edict_t *ent, edict_t *other, edict_t *activator)
{
//	ent->svflags &= ~SVF_NOCLIENT;
	ent->use = NULL;

	if (ent->spawnflags & 2)	// NO_TOUCH
	{
		ent->solid = SOLID_BBOX;
		ent->touch = NULL;
	}
	else
	{
		ent->solid = SOLID_TRIGGER;
		ent->touch = Touch_Item;
	}

	gi.linkentity (ent);
}
*/

// ************************************************************************************************
// ValidItem
// ---------
// ************************************************************************************************

qboolean ValidItem(gitem_t *item)
{
	// Some items will be prevented in deathmatch.

	if(deathmatch->value)
	{
		if ( (int)dmflags->value & DF_NO_DEFENSIVE_SPELL )
		{
			if (item->flags & IT_DEFENSE)
			{
				return false;
			}
		}
		if ( (int)dmflags->value & DF_NO_OFFENSIVE_SPELL )
		{
			if (item->flags & IT_OFFENSE)
			{
				return false;
			}
		}
		if ( (int)dmflags->value & DF_NO_HEALTH )
		{
			if (item->flags & IT_HEALTH)
			{
				return false;
			}
		}

		if ((item->flags & IT_DEFENSE) && (item->tag == ITEM_DEFENSE_TORNADO) && (no_tornado->value))
			return false;
		else if ((item->flags & IT_DEFENSE) && (item->tag == ITEM_DEFENSE_POLYMORPH) && (no_morph->value))
			return false;
		else if ((item->flags & IT_DEFENSE) && (item->tag == ITEM_DEFENSE_SHIELD) && (no_shield->value))
			return false;
		else if ((item->flags & IT_DEFENSE) && (item->tag == ITEM_DEFENSE_TELEPORT) && (no_teleport->value))
			return false;
		else if ((item->flags & IT_OFFENSE) && (item->tag == ITEM_WEAPON_PHOENIXBOW) && (no_phoenix->value))
			return false;
		else if ((item->flags & IT_OFFENSE) && (item->tag == ITEM_WEAPON_MACEBALLS) && (no_irondoom->value))
			return false;

	}

	return true;
}

// ************************************************************************************************
// SpawnItemEffect
// ---------------
// ************************************************************************************************

void SpawnItemEffect(edict_t *ent, gitem_t *item)
{

	if(!ValidItem(item))
	{
		G_FreeEdict (ent);
		return;
	}

	assert(!ent->PersistantCFX);

	if ((ent->spawnflags & ITEM_COOP_ONLY) && (!coop->value))
		return;

	if(ent->item->flags & IT_PUZZLE)
	{
		ent->PersistantCFX = gi.CreatePersistantEffect(&ent->s, FX_PICKUP_PUZZLE, CEF_BROADCAST, ent->s.origin, "bv", ent->item->tag,ent->s.angles);
	}
	else if(ent->item->flags & IT_WEAPON)
	{
		ent->PersistantCFX = gi.CreatePersistantEffect(&ent->s, FX_PICKUP_WEAPON, CEF_BROADCAST, ent->s.origin, "b", ent->item->tag);
	}
	else if(ent->item->flags & IT_AMMO)
	{
		ent->PersistantCFX = gi.CreatePersistantEffect(&ent->s, FX_PICKUP_AMMO, CEF_BROADCAST, ent->s.origin, "b", ent->item->tag);
	}
	else if(ent->item->flags & IT_DEFENSE)
	{
		ent->PersistantCFX = gi.CreatePersistantEffect(&ent->s, FX_PICKUP_DEFENSE, CEF_BROADCAST, ent->s.origin, "b", ent->item->tag);
	}
	else 
	{
		if (ent->item->tag)
			ent->PersistantCFX = gi.CreatePersistantEffect(&ent->s, FX_PICKUP_HEALTH, CEF_FLAG6|CEF_BROADCAST, ent->s.origin, "");
		else
			ent->PersistantCFX = gi.CreatePersistantEffect(&ent->s, FX_PICKUP_HEALTH, CEF_BROADCAST, ent->s.origin, "");
	}
}


/*
================
droptofloor
================
*/

void itemsdroptofloor (edict_t *ent)
{
	trace_t		tr;
	vec3_t		dest;

	ent->movetype = PHYSICSTYPE_STATIC;  
	ent->solid = SOLID_TRIGGER;
	ent->touch = Touch_Item;
	ent->think = NULL;

	if (!(ent->spawnflags & ITEM_NO_DROP))
	{
		VectorSet(dest, 0.0, 0.0, -128.0);
		Vec3AddAssign (ent->s.origin, dest);

		gi.linkentity (ent);

		gi.trace (ent->s.origin, ent->mins, ent->maxs, dest, ent, MASK_SOLID, &tr);
		if (tr.startsolid)
		{
			gi.dprintf ("droptofloor: %s startsolid at %s (inhibited)\n", ent->classname, vtos(ent->s.origin));
			G_FreeEdict (ent);
			return;
		}

		tr.endpos[2] += 24;
		VectorCopy(tr.endpos,ent->s.origin);
	}


	gi.linkentity (ent);

	// if we loading a saved game - the objects will already be out there
	if (!ent->PersistantCFX)
		SpawnItemEffect(ent, ent->item);
}


/*
===============
PrecacheItem

Precaches all data needed for a given item. This will be called for each item
spawned in a level, and for each item in each client's inventory.
===============
*/

void PrecacheItem (gitem_t *it)
{
	gitem_t	*ammo;

	if (!it)
		return;

	if (it->pickup_sound)
		gi.soundindex (it->pickup_sound);
	if (it->world_model)
		gi.modelindex (it->world_model);
	if (it->icon)
		gi.imageindex (it->icon);

	// parse everything for its ammo
	if (it->ammo && it->ammo[0])
	{
		ammo = P_FindItem (it->ammo);
		if (ammo != it)
			PrecacheItem (ammo);
	}
}
// ************************************************************************************************
// SpawnItem
// ---------
// Sets the clipping size and plants the object on the floor. Items can't be immediately dropped to
// the floor because they might be on an entity that hasn't spawned yet.
// ************************************************************************************************

void SpawnItem (edict_t *ent, gitem_t *item)
{
	if ((ent->spawnflags & ITEM_COOP_ONLY) && (!coop->value))
		return;

	PrecacheItem(item);

	if(!ValidItem(item))
	{
		G_FreeEdict (ent);
		return;
	}

	ent->item = item;
	ent->nextthink = level.time + (2 * FRAMETIME);    // items start after other solids
	ent->think = itemsdroptofloor;

	ent->s.effects = item->world_model_flags;
	ent->s.renderfx = RF_GLOW;
//	ent->svflags |= SVF_ALWAYS_SEND;				// make sure SVF_NOCLIENT gets set in think
	ent->s.effects |= EF_ALWAYS_ADD_EFFECTS;

	if (item->flags & IT_WEAPON)
	{
		if (item->tag == ITEM_WEAPON_MACEBALLS)
			ent->delay = RESPAWN_TIME_MACEBALL;		// Maceballs shouldn't come back as fast...
		else
			ent->delay = RESPAWN_TIME_WEAPON;
	}
	else if (item->flags & IT_DEFENSE)
	{
		if (item->tag == ITEM_DEFENSE_REPULSION)
			ent->delay = RESPAWN_TIME_RING;
		else if (item->tag == ITEM_DEFENSE_TELEPORT)
			ent->delay = RESPAWN_TIME_TELEPORT;
		else if (item->tag == ITEM_DEFENSE_POLYMORPH)
			ent->delay = RESPAWN_TIME_MORPH;
		else
			ent->delay = RESPAWN_TIME_DEFENSE;
	}
	else if (item->flags & IT_AMMO)
	{
		if (item->tag == ITEM_AMMO_REDRAIN || item->tag == ITEM_AMMO_PHOENIX)
			ent->delay = RESPAWN_TIME_ARROWS;
		else
			ent->delay = RESPAWN_TIME_AMMO;
	}
	else // Health and anything else
	{
		ent->delay = RESPAWN_TIME_MISC;
	}

	ent->flags = item->flags;
	ent->clipmask = MASK_MONSTERSOLID;

	if (item->flags == IT_PUZZLE)
	{
		VectorCopy(ent->item->mins, ent->mins);
		VectorCopy(ent->item->maxs, ent->maxs);
	}

	// FIXME: Until all objects have bounding boxes, default to these vals.
	if (Vec3IsZero(ent->mins))
	{
		VectorSet (ent->mins, -10.0, -10.0, -10.0);
		VectorSet (ent->maxs, 10.0, 10.0, 10.0);
	}

	ent->classname = item->classname;
	if(deathmatch->value)
	{
		ent->flags |= FL_RESPAWN;
	}
}
// ************************************************************************************************
// IsItem
// ------
// ************************************************************************************************

gitem_t	*IsItem(edict_t *ent)
{
	gitem_t	*item;
	int i;

	if(!ent->classname)
	{
		return NULL;
	}

	for(i = 0, item = playerExport.p_itemlist; i < game.num_items; ++i, ++item)
	{
		if(!item->classname)
		{
			continue;
		}

		if(!strcmp(item->classname, ent->classname))
		{	
			// Found it.

			return item;
		}
	}

	return NULL;
}

// ************************************************************************************************
// G_InitItems
// -----------
// ************************************************************************************************

void G_InitItems(void)
{
	// ********************************************************************************************
	// Setup item function pointers which yield pick-up, use, drop and weaponthink functionality.
	// ********************************************************************************************

	// Leave index 0 empty.

	// weapon_swordstaff
	// This can't be placed in the editor

	playerExport.p_itemlist[1].pickup=Pickup_Weapon;
	playerExport.p_itemlist[1].use=P_Weapon_EquipSwordStaff;
	playerExport.p_itemlist[1].weaponthink=WeaponThink_SwordStaff;

	// weapon_flyingfist
	// This can't be placed in the editor

	playerExport.p_itemlist[2].pickup=Pickup_Weapon;
	playerExport.p_itemlist[2].use=P_Weapon_EquipSpell;
	playerExport.p_itemlist[2].weaponthink=WeaponThink_FlyingFist;

	// item_weapon_hellstaff
/*QUAKED item_weapon_hellstaff (.3 .3 1) (-16 -16 -16) (16 16 16) COOP_ONLY
Pickup for the hellstaff weapon.
*/

	playerExport.p_itemlist[3].pickup=Pickup_Weapon;
	playerExport.p_itemlist[3].use=P_Weapon_EquipHellStaff;
	playerExport.p_itemlist[3].weaponthink=WeaponThink_HellStaff;

	// item_weapon_magicmissile
/*QUAKED item_weapon_magicmissile (.3 .3 1) (-16 -16 -16) (16 16 16) COOP_ONLY
Pickup for the Magic Missile weapon.
*/

	playerExport.p_itemlist[4].pickup=Pickup_Weapon;
	playerExport.p_itemlist[4].use=P_Weapon_EquipSpell;
	playerExport.p_itemlist[4].weaponthink=WeaponThink_MagicMissileSpread;

	// item_weapon_redrain_bow
/*QUAKED item_weapon_redrain_bow (.3 .3 1) (-16 -16 -16) (16 16 16)  COOP_ONLY
Pickup for the Red Rain Bow weapon.
*/

	playerExport.p_itemlist[5].pickup=Pickup_Weapon;
	playerExport.p_itemlist[5].use=P_Weapon_EquipBow;
	playerExport.p_itemlist[5].weaponthink=WeaponThink_RedRainBow;

	// item_weapon_firewall
/*QUAKED item_weapon_firewall (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
Pickup for the Fire Wall weapon.
*/

	playerExport.p_itemlist[6].pickup=Pickup_Weapon;
	playerExport.p_itemlist[6].use=P_Weapon_EquipSpell;
	playerExport.p_itemlist[6].weaponthink=WeaponThink_Firewall;

	// item_weapon_phoenixbow
/*QUAKED item_weapon_phoenixbow (.3 .3 1) (-16 -16 -16) (16 16 16)  COOP_ONLY
Pickup for the Phoenix Bow weapon.
*/

	playerExport.p_itemlist[7].pickup=Pickup_Weapon;
	playerExport.p_itemlist[7].use=P_Weapon_EquipBow;
	playerExport.p_itemlist[7].weaponthink=WeaponThink_PhoenixBow;

	// item_weapon_sphereofannihilation
/*QUAKED item_weapon_sphereofannihilation (.3 .3 1) (-16 -16 -16) (16 16 16)  COOP_ONLY
Pickup for the Sphere Annihilation weapon.
*/

	playerExport.p_itemlist[8].pickup=Pickup_Weapon;
	playerExport.p_itemlist[8].use=P_Weapon_EquipSpell;
	playerExport.p_itemlist[8].weaponthink=WeaponThink_SphereOfAnnihilation;

	// item_weapon_maceballs
/*QUAKED item_weapon_maceballs (.3 .3 1) (-16 -16 -16) (16 16 16)  COOP_ONLY
Pickup for the Mace Balls weapon.
*/

	playerExport.p_itemlist[9].pickup=Pickup_Weapon;
	playerExport.p_itemlist[9].use=P_Weapon_EquipSpell;
	playerExport.p_itemlist[9].weaponthink=WeaponThink_Maceballs;
	
	// item_defense_powerup
	// This can't be placed in the editor

	playerExport.p_itemlist[10].pickup=Pickup_Defense;
	playerExport.p_itemlist[10].use=Use_Defence;
	playerExport.p_itemlist[10].weaponthink=DefenceThink_Powerup;

	// item_defense_ringofrepulsion
/*QUAKED item_defense_ringofrepulsion (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
Pickup for the Ring of Repulsion defensive spell.
*/

	playerExport.p_itemlist[11].pickup=Pickup_Defense;
	playerExport.p_itemlist[11].use=Use_Defence;
	playerExport.p_itemlist[11].weaponthink=DefenceThink_RingOfRepulsion;
	
	// item_defense_shield
/*QUAKED item_defense_shield (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
Pickup for the Shield defensive spell.
*/

	playerExport.p_itemlist[12].pickup=Pickup_Defense;
	playerExport.p_itemlist[12].use=Use_Defence;
	playerExport.p_itemlist[12].weaponthink=DefenceThink_Shield;

	// item_defense_teleport
/*QUAKED item_defense_teleport (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
Pickup for the Teleport defensive spell.
*/

	playerExport.p_itemlist[13].pickup=Pickup_Defense;
	playerExport.p_itemlist[13].use=Use_Defence;
	playerExport.p_itemlist[13].weaponthink=DefenceThink_Teleport;

	// item_defense_polymorph
/*QUAKED item_defense_polymorph (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
Pickup for the Polymorph Barrier defensive spell.
*/

	playerExport.p_itemlist[14].pickup=Pickup_Defense;
	playerExport.p_itemlist[14].use=Use_Defence;
	playerExport.p_itemlist[14].weaponthink=DefenceThink_Morph;

	// item_defense_meteorbarrier
/*QUAKED item_defense_meteorbarrier (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
Pickup for the Meteor Barrier defensive spell.
*/

	playerExport.p_itemlist[15].pickup=Pickup_Defense;
	playerExport.p_itemlist[15].use=Use_Defence;
	playerExport.p_itemlist[15].weaponthink=DefenceThink_MeteorBarrier;

	// item_mana_offensive_half
/*QUAKED item_mana_offensive_half (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
Pickup for the offensive mana (50 points).
*/
	playerExport.p_itemlist[16].pickup=Pickup_Mana;
	
	// item_mana_offensive_full
/*QUAKED item_mana_offensive_full (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
Pickup for the offensive mana (100 points).
*/

	playerExport.p_itemlist[17].pickup=Pickup_Mana;

	// item_mana_defensive_half
/*QUAKED item_mana_defensive_half (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
Pickup for the defensive mana (50 points).
*/

	playerExport.p_itemlist[18].pickup=Pickup_Mana;

	// item_mana_defensive_full
/*QUAKED item_mana_defensive_full (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
Pickup for the defensive mana (100 points).
*/

	playerExport.p_itemlist[19].pickup=Pickup_Mana;

	// item_mana_combo_quarter
/*QUAK-ED item_mana_combo_quarter (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
Pickup for both defensive & offensive mana (25 points).
*/

	playerExport.p_itemlist[20].pickup=Pickup_Mana;

	// item_mana_combo_half
/*QUAKED item_mana_combo_half (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
Pickup for both defensive & offensive mana (50 points).
*/

	playerExport.p_itemlist[21].pickup=Pickup_Mana;

	// item_ammo_redrain
/*QUAKED item_ammo_redrain (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
Pickup ammo for the Red Rain Bow
*/

	playerExport.p_itemlist[22].pickup=Pickup_Ammo;

	// item_ammo_phoenix
/*QUAKED item_ammo_phoenix (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
Pickup ammo for the Phoenix Bow
*/

	playerExport.p_itemlist[23].pickup=Pickup_Ammo;

	// item_ammo_hellstaff
/*QUAKED item_ammo_hellstaff (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
Pickup ammo for the Hellstaff
*/

	playerExport.p_itemlist[24].pickup=Pickup_Ammo;

	// item_health_half
/*QUAKED item_health_half (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
Pickup health (10 points)
*/

	playerExport.p_itemlist[25].pickup=Pickup_Health;

	// item_health_full
/*QUAKED item_health_full (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
Pickup health (30 points)
*/

	playerExport.p_itemlist[26].pickup=Pickup_Health;

/*QUAKED item_puzzle_townkey (.3 .3 1) (-8 -8 -4) (8 8 4)  x NO_DROP
Key puzzle piece
Town Level
NO_DROP - won't drop to ground

*/
	playerExport.p_itemlist[27].pickup = Pickup_Puzzle;

/*QUAKED item_puzzle_cog (.3 .3 1) (-10 -10 -24) (10 10 20)  x  NO_DROP
Cog puzzle piece
Palace level
NO_DROP - won't drop to ground
*/
	playerExport.p_itemlist[28].pickup = Pickup_Puzzle;

/*QUAKED item_puzzle_shield (.3 .3 1) (-2 -6 -12) (2 6 12)  x  NO_DROP
Sithra Shield puzzle item
Healer Level
NO_DROP - won't drop to ground
*/
	playerExport.p_itemlist[29].pickup = Pickup_Puzzle;

/*QUAKED item_puzzle_potion (.3 .3 1) (-3 -3 -10) (3 3 10)  x  NO_DROP
Potion puzzle item
Healer Level
NO_DROP - won't drop to ground
*/
	playerExport.p_itemlist[30].pickup = Pickup_Puzzle;

/*QUAKED item_puzzle_plazacontainer (.3 .3 1) (-6 -6 -8) (6 6 6)  x  NO_DROP
Container puzzle item
Plaza Level
NO_DROP - won't drop to ground
*/
	playerExport.p_itemlist[31].pickup = Pickup_Puzzle;

/*QUAKED item_puzzle_slumcontainer (.3 .3 1) (-6 -6 -8) (6 6 6)  x  NO_DROP
Full Container puzzle item
Slum Level
NO_DROP - won't drop to ground
*/
	playerExport.p_itemlist[32].pickup = Pickup_Puzzle;

/*QUAKED item_puzzle_crystal (.3 .3 1) (-16 -16 -16) (16 16 16)  x  NO_DROP
Crystal puzzle item
Academic Level
NO_DROP - won't drop to ground
*/
	playerExport.p_itemlist[33].pickup = Pickup_Puzzle;

/*QUAKED item_puzzle_canyonkey (.3 .3 1) (-16 -16 -16) (16 16 16)  x  NO_DROP
Key puzzle item
Canyon Level
NO_DROP - won't drop to ground
*/
	playerExport.p_itemlist[34].pickup = Pickup_Puzzle;

/*QUAKED item_puzzle_hive2amulet (.3 .3 1) (-16 -16 -16) (16 16 16)  x  NO_DROP
Amulet puzzle item
Hive 2 Level
NO_DROP - won't drop to ground
*/
	playerExport.p_itemlist[35].pickup = Pickup_Puzzle;

/*QUAKED item_puzzle_hive2spear (.3 .3 1) (-16 -16 -16) (16 16 16)  x  NO_DROP
Spear puzzle item
Hive 2 Level
NO_DROP - won't drop to ground
*/
	playerExport.p_itemlist[36].pickup = Pickup_Puzzle;

/*QUAKED item_puzzle_hive2gem (.3 .3 1) (-16 -16 -16) (16 16 16)  x  NO_DROP
Gem puzzle item
Hive 2 Level
NO_DROP - won't drop to ground
*/
	playerExport.p_itemlist[37].pickup = Pickup_Puzzle;

/*QUAKED item_puzzle_minecartwheel (.3 .3 1) (-1 -6 -6) (1 6 6)  x  NO_DROP
Mine Cart Wheel puzzle item
Mine 1 Level
NO_DROP - won't drop to ground
*/
	playerExport.p_itemlist[38].pickup = Pickup_Puzzle;

/*QUAKED item_puzzle_ore (.3 .3 1) (-10 -10 -8) (10 10 8)  x  NO_DROP
Unrefined Ore puzzle item
Mine 2 Level
NO_DROP - won't drop to ground
*/
	playerExport.p_itemlist[39].pickup = Pickup_Puzzle;

/*QUAKED item_puzzle_refinedore (.3 .3 1) (-3 -12 -2) (3 12 2) x   NO_DROP
Refined Ore puzzle item
Mine 2 Level
NO_DROP - won't drop to ground
*/
	playerExport.p_itemlist[40].pickup = Pickup_Puzzle;

/*QUAKED item_puzzle_dungeonkey (.3 .3 1) (-1 -18 -9) (1 18 9)  x  NO_DROP
Amulet puzzle item
Dungeon Level
NO_DROP - won't drop to ground
*/
	playerExport.p_itemlist[41].pickup = Pickup_Puzzle;

/*QUAKED item_puzzle_cloudkey (.3 .3 1) (-8 -8 -3) (8 8 6)  x  NO_DROP
Key puzzle item
Cloud Quarters 2 Level
NO_DROP - won't drop to ground
*/
	playerExport.p_itemlist[42].pickup = Pickup_Puzzle;

/*QUAKED item_puzzle_highpriestesskey (.3 .3 1) (-12 -12 -6) (12 12 6) x   NO_DROP
Key puzzle item
High Priestess Level
NO_DROP - won't drop to ground
*/
	playerExport.p_itemlist[43].pickup = Pickup_Puzzle;

/*QUAKED item_puzzle_highpriestesssymbol (.3 .3 1) (-12 -12 -4) (12 12 4) x   NO_DROP
Key puzzle item
High Priestess Level
NO_DROP - won't drop to ground
*/
	playerExport.p_itemlist[44].pickup = Pickup_Puzzle;

/*QUAKED item_puzzle_tome (.3 .3 1) (-12 -12 -4) (12 12 4)  x  NO_DROP
Tome puzzle piece
2 Cloud Levels
NO_DROP - won't drop to ground
*/
	playerExport.p_itemlist[45].pickup = Pickup_Puzzle;

/*QUAKED item_puzzle_tavernkey (.3 .3 1) (-8 -8 -4) (8 8 4)    x   NO_DROP
Key puzzle piece
Ssdocks Level
NO_DROP - won't drop to ground
*/
	playerExport.p_itemlist[46].pickup = Pickup_Puzzle;

	// item_defense_tornado
/*QUAKED item_defense_tornado (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
Pickup for the Tornado defensive spell.
*/
	playerExport.p_itemlist[47].pickup=Pickup_Defense;
	playerExport.p_itemlist[47].use=Use_Defence;
	playerExport.p_itemlist[47].weaponthink=DefenceThink_Tornado;

	// ********************************************************************************************
	// Initialise game variables.
	// ********************************************************************************************

	game.num_items=playerExport.p_num_items;
}

/*
===============
SetItemNames - called by worldspawn.
===============
*/

void SetItemNames(void)
{
	int		i;
	gitem_t	*it;

	for (i=0 ; i<game.num_items ; i++)
	{
		it = &playerExport.p_itemlist[i];
		gi.configstring (CS_ITEMS+i, it->pickup_name);
	}
}
