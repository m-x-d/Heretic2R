//
// g_items.c
//
// Copyright 1998 Raven Software
//

#include "g_local.h"
#include "p_client.h" //mxd
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

static qboolean Pickup_Puzzle(const edict_t* ent, edict_t* other)
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

qboolean AddWeaponToInventory(gitem_t* weapon, const edict_t* player)
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

static qboolean Pickup_Weapon(const edict_t* ent, edict_t* other)
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

qboolean AddDefenseToInventory(gitem_t* defence, const edict_t* player)
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

static qboolean Pickup_Defense(const edict_t* ent, edict_t* other)
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

static qboolean Pickup_Ammo(const edict_t* ent, edict_t* other)
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
static qboolean Pickup_Mana(const edict_t* ent, edict_t* other)
{
	return Pickup_Ammo(ent, other);
}

#pragma endregion

#pragma region ========================== HEALTH PICKUP LOGIC ==========================

static qboolean Pickup_Health(const edict_t* ent, edict_t* other)
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
		PlayerRepairSkin(other);

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
		gi.RemoveEffects(&ent->s, FX_REMOVE_EFFECTS); // Once picked up, the item is gone forever, so remove it's client effect(s).
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
		ent->think = G_FreeEdict;
		ent->nextthink = level.time + 29.0f;
	}
}

edict_t* Drop_Item(edict_t* ent, gitem_t* item)
{
	edict_t* dropped = G_Spawn();

	dropped->classname = item->classname;
	dropped->item = item;
	dropped->spawnflags = DROPPED_ITEM;
	dropped->s.effects = item->world_model_flags;
	dropped->s.renderfx = RF_GLOW;
	VectorSet(dropped->mins, -15.0f, -15.0f, -15.0f);
	VectorSet(dropped->maxs,  15.0f,  15.0f,  15.0f);
	gi.setmodel(dropped, dropped->item->world_model);
	dropped->solid = SOLID_TRIGGER;
	dropped->movetype = PHYSICSTYPE_NONE;
	dropped->touch = DropItemTempTouch;
	dropped->owner = ent;

	vec3_t forward;
	vec3_t right;

	if (ent->client != NULL)
	{
		AngleVectors(ent->client->v_angle, forward, right, NULL);

		vec3_t offset = { 24.0f, 0.0f, -16.0f };
		G_ProjectSource(ent->s.origin, offset, forward, right, dropped->s.origin);

		trace_t	trace;
		gi.trace(ent->s.origin, dropped->mins, dropped->maxs, dropped->s.origin, ent, CONTENTS_SOLID, &trace);
		VectorCopy(trace.endpos, dropped->s.origin);
	}
	else
	{
		AngleVectors(ent->s.angles, forward, right, NULL);
		VectorCopy(ent->s.origin, dropped->s.origin);
	}

	VectorScale(forward, 100.0f, dropped->velocity);
	dropped->velocity[2] = 300.0f;

	dropped->think = DropItemMakeTouchable;
	dropped->nextthink = level.time + 1.0f;

	gi.linkentity(dropped);

	return dropped;
}

static qboolean IsValidItem(const gitem_t* item) //mxd. Named 'ValidItem' in original version.
{
	if (!DEATHMATCH)
		return true;

	// Some items will be prevented in deathmatch.
	if ((DMFLAGS & DF_NO_DEFENSIVE_SPELL) && (item->flags & IT_DEFENSE))
		return false;

	if ((DMFLAGS & DF_NO_OFFENSIVE_SPELL) && (item->flags & IT_OFFENSE))
		return false;

	if ((DMFLAGS & DF_NO_HEALTH) && (item->flags & IT_HEALTH))
		return false;

	if (item->flags & IT_DEFENSE)
	{
		if (item->tag == ITEM_DEFENSE_TORNADO && (int)no_tornado->value)
			return false;

		if (item->tag == ITEM_DEFENSE_POLYMORPH && (int)no_morph->value)
			return false;

		if (item->tag == ITEM_DEFENSE_SHIELD && (int)no_shield->value)
			return false;

		if (item->tag == ITEM_DEFENSE_TELEPORT && (int)no_teleport->value)
			return false;
	}

	if (item->flags & IT_OFFENSE)
	{
		if (item->tag == ITEM_WEAPON_PHOENIXBOW && (int)no_phoenix->value)
			return false;

		if (item->tag == ITEM_WEAPON_MACEBALLS && (int)no_irondoom->value)
			return false;
	}

	return true;
}

static void SpawnItemEffect(edict_t* ent, const gitem_t* item)
{
	if (!IsValidItem(item))
	{
		G_FreeEdict(ent);
		return;
	}

	assert(ent->PersistantCFX == 0);

	if ((ent->spawnflags & ITEM_COOP_ONLY) && !COOP)
		return;

	if (ent->item->flags & IT_PUZZLE)
		ent->PersistantCFX = gi.CreatePersistantEffect(&ent->s, FX_PICKUP_PUZZLE, CEF_BROADCAST, ent->s.origin, "bv", ent->item->tag, ent->s.angles);
	else if (ent->item->flags & IT_WEAPON)
		ent->PersistantCFX = gi.CreatePersistantEffect(&ent->s, FX_PICKUP_WEAPON, CEF_BROADCAST, ent->s.origin, "b", ent->item->tag);
	else if (ent->item->flags & IT_AMMO)
		ent->PersistantCFX = gi.CreatePersistantEffect(&ent->s, FX_PICKUP_AMMO, CEF_BROADCAST, ent->s.origin, "b", ent->item->tag);
	else if (ent->item->flags & IT_DEFENSE)
		ent->PersistantCFX = gi.CreatePersistantEffect(&ent->s, FX_PICKUP_DEFENSE, CEF_BROADCAST, ent->s.origin, "b", ent->item->tag);
	else if (ent->item->tag != 0)
		ent->PersistantCFX = gi.CreatePersistantEffect(&ent->s, FX_PICKUP_HEALTH, CEF_FLAG6 | CEF_BROADCAST, ent->s.origin, "");
	else
		ent->PersistantCFX = gi.CreatePersistantEffect(&ent->s, FX_PICKUP_HEALTH, CEF_BROADCAST, ent->s.origin, "");
}

static void ItemDropToFloor(edict_t* ent) //mxd. Named 'itemsdroptofloor' in original version.
{
	ent->movetype = PHYSICSTYPE_STATIC;
	ent->solid = SOLID_TRIGGER;
	ent->touch = Touch_Item;
	ent->think = NULL;

	if (!(ent->spawnflags & ITEM_NO_DROP))
	{
		vec3_t dest = { 0.0f, 0.0f, -128.0f };
		Vec3AddAssign(ent->s.origin, dest);

		gi.linkentity(ent);

		trace_t tr;
		gi.trace(ent->s.origin, ent->mins, ent->maxs, dest, ent, MASK_SOLID, &tr);

		if (tr.startsolid)
		{
			gi.dprintf("droptofloor: %s startsolid at %s (inhibited)\n", ent->classname, vtos(ent->s.origin));
			G_FreeEdict(ent);

			return;
		}

		tr.endpos[2] += 24.0f;
		VectorCopy(tr.endpos, ent->s.origin);
	}

	gi.linkentity(ent);

	// If we loading a saved game, the objects will already be out there.
	if (ent->PersistantCFX == 0)
		SpawnItemEffect(ent, ent->item);
}

// Precaches all data needed for a given item.
// This will be called for each item spawned in a level, and for each item in each client's inventory.
static void PrecacheItem(const gitem_t* item)
{
	if (item == NULL)
		return;

	if (item->pickup_sound != NULL)
		gi.soundindex(item->pickup_sound);

	if (item->world_model != NULL)
		gi.modelindex(item->world_model);

	if (item->icon != NULL)
		gi.imageindex(item->icon);

	// Parse everything for its ammo.
	if (item->ammo != NULL && item->ammo[0] != 0)
	{
		const gitem_t* ammo = P_FindItem(item->ammo);

		if (ammo != item)
			PrecacheItem(ammo);
	}
}

// Sets the clipping size and plants the object on the floor.
// Items can't be immediately dropped to the floor because they might be on an entity that hasn't spawned yet.
void SpawnItem(edict_t* ent, gitem_t* item)
{
	if ((ent->spawnflags & ITEM_COOP_ONLY) && !COOP)
		return;

	PrecacheItem(item);

	if (!IsValidItem(item))
	{
		G_FreeEdict(ent);
		return;
	}

	ent->item = item;
	ent->nextthink = level.time + FRAMETIME * 2.0f; // Items start after other solids.
	ent->think = ItemDropToFloor;

	ent->clipmask = MASK_MONSTERSOLID;
	ent->classname = item->classname;

	ent->flags = item->flags;
	if (DEATHMATCH)
		ent->flags |= FL_RESPAWN;

	ent->s.effects = (item->world_model_flags | EF_ALWAYS_ADD_EFFECTS);
	ent->s.renderfx = RF_GLOW;

	if (item->flags & IT_WEAPON)
	{
		if (item->tag == ITEM_WEAPON_MACEBALLS)
			ent->delay = RESPAWN_TIME_MACEBALL; // Maceballs shouldn't come back as fast...
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
	else // Health and anything else.
	{
		ent->delay = RESPAWN_TIME_MISC;
	}

	if (item->flags == IT_PUZZLE)
	{
		VectorCopy(ent->item->mins, ent->mins);
		VectorCopy(ent->item->maxs, ent->maxs);
	}

	// FIXME: Until all objects have bounding boxes, default to these vals.
	if (Vec3IsZero(ent->mins))
	{
		VectorSet(ent->mins, -10.0f, -10.0f, -10.0f);
		VectorSet(ent->maxs,  10.0f,  10.0f,  10.0f);
	}
}

gitem_t* IsItem(const edict_t* ent)
{
	if (ent->classname == NULL)
		return NULL;

	gitem_t* item = &playerExport.p_itemlist[0];
	for (int i = 0; i < game.num_items; i++, item++)
		if (item->classname != NULL && strcmp(item->classname, ent->classname) == 0)
			return item; // Found it.

	return NULL;
}

void G_InitItems(void)
{
	// Setup item function pointers which yield pick-up, use, drop and weaponthink functionality. Leave index 0 empty.

	// weapon_swordstaff. Can't be placed in the editor.
	playerExport.p_itemlist[1].pickup = Pickup_Weapon;
	playerExport.p_itemlist[1].use = P_Weapon_EquipSwordStaff;
	playerExport.p_itemlist[1].weaponthink = WeaponThink_SwordStaff;

	// weapon_flyingfist. Can't be placed in the editor.
	playerExport.p_itemlist[2].pickup = Pickup_Weapon;
	playerExport.p_itemlist[2].use = P_Weapon_EquipSpell;
	playerExport.p_itemlist[2].weaponthink = WeaponThink_FlyingFist;

	// item_weapon_hellstaff. Pickup for the hellstaff weapon.
	playerExport.p_itemlist[3].pickup = Pickup_Weapon;
	playerExport.p_itemlist[3].use = P_Weapon_EquipHellStaff;
	playerExport.p_itemlist[3].weaponthink = WeaponThink_HellStaff;

	// item_weapon_magicmissile. Pickup for the Magic Missile weapon.
	playerExport.p_itemlist[4].pickup = Pickup_Weapon;
	playerExport.p_itemlist[4].use = P_Weapon_EquipSpell;
	playerExport.p_itemlist[4].weaponthink = WeaponThink_MagicMissileSpread;

	// item_weapon_redrain_bow. Pickup for the Red Rain Bow weapon.
	playerExport.p_itemlist[5].pickup = Pickup_Weapon;
	playerExport.p_itemlist[5].use = P_Weapon_EquipBow;
	playerExport.p_itemlist[5].weaponthink = WeaponThink_RedRainBow;

	// item_weapon_firewall. Pickup for the Fire Wall weapon.
	playerExport.p_itemlist[6].pickup = Pickup_Weapon;
	playerExport.p_itemlist[6].use = P_Weapon_EquipSpell;
	playerExport.p_itemlist[6].weaponthink = WeaponThink_Firewall;

	// item_weapon_phoenixbow. Pickup for the Phoenix Bow weapon.
	playerExport.p_itemlist[7].pickup = Pickup_Weapon;
	playerExport.p_itemlist[7].use = P_Weapon_EquipBow;
	playerExport.p_itemlist[7].weaponthink = WeaponThink_PhoenixBow;

	// item_weapon_sphereofannihilation. Pickup for the Sphere Annihilation weapon.
	playerExport.p_itemlist[8].pickup = Pickup_Weapon;
	playerExport.p_itemlist[8].use = P_Weapon_EquipSpell;
	playerExport.p_itemlist[8].weaponthink = WeaponThink_SphereOfAnnihilation;

	// item_weapon_maceballs. Pickup for the Mace Balls weapon.
	playerExport.p_itemlist[9].pickup = Pickup_Weapon;
	playerExport.p_itemlist[9].use = P_Weapon_EquipSpell;
	playerExport.p_itemlist[9].weaponthink = WeaponThink_Maceballs;

	// item_defense_powerup. Can't be placed in the editor.
	playerExport.p_itemlist[10].pickup = Pickup_Defense;
	playerExport.p_itemlist[10].use = Use_Defence;
	playerExport.p_itemlist[10].weaponthink = DefenceThink_Powerup;

	// item_defense_ringofrepulsion. Pickup for the Ring of Repulsion defensive spell.
	playerExport.p_itemlist[11].pickup = Pickup_Defense;
	playerExport.p_itemlist[11].use = Use_Defence;
	playerExport.p_itemlist[11].weaponthink = DefenceThink_RingOfRepulsion;

	// item_defense_shield. Pickup for the Shield defensive spell.
	playerExport.p_itemlist[12].pickup = Pickup_Defense;
	playerExport.p_itemlist[12].use = Use_Defence;
	playerExport.p_itemlist[12].weaponthink = DefenceThink_Shield;

	// item_defense_teleport. Pickup for the Teleport defensive spell.
	playerExport.p_itemlist[13].pickup = Pickup_Defense;
	playerExport.p_itemlist[13].use = Use_Defence;
	playerExport.p_itemlist[13].weaponthink = DefenceThink_Teleport;

	// item_defense_polymorph. Pickup for the Polymorph Barrier defensive spell.
	playerExport.p_itemlist[14].pickup = Pickup_Defense;
	playerExport.p_itemlist[14].use = Use_Defence;
	playerExport.p_itemlist[14].weaponthink = DefenceThink_Morph;

	// item_defense_meteorbarrier. Pickup for the Meteor Barrier defensive spell.
	playerExport.p_itemlist[15].pickup = Pickup_Defense;
	playerExport.p_itemlist[15].use = Use_Defence;
	playerExport.p_itemlist[15].weaponthink = DefenceThink_MeteorBarrier;

	// item_mana_offensive_half. Pickup for the offensive mana (50 points).
	playerExport.p_itemlist[16].pickup = Pickup_Mana;

	// item_mana_offensive_full. Pickup for the offensive mana (100 points).
	playerExport.p_itemlist[17].pickup = Pickup_Mana;

	// item_mana_defensive_half. Pickup for the defensive mana (50 points).
	playerExport.p_itemlist[18].pickup = Pickup_Mana;

	// item_mana_defensive_full. Pickup for the defensive mana (100 points).
	playerExport.p_itemlist[19].pickup = Pickup_Mana;

	// item_mana_combo_quarter. Pickup for both defensive & offensive mana (25 points).
	playerExport.p_itemlist[20].pickup = Pickup_Mana;

	// item_mana_combo_half. Pickup for both defensive & offensive mana (50 points).
	playerExport.p_itemlist[21].pickup = Pickup_Mana;

	// item_ammo_redrain. Pickup ammo for the Red Rain Bow.
	playerExport.p_itemlist[22].pickup = Pickup_Ammo;

	// item_ammo_phoenix. Pickup ammo for the Phoenix Bow.
	playerExport.p_itemlist[23].pickup = Pickup_Ammo;

	// item_ammo_hellstaff. Pickup ammo for the Hellstaff.
	playerExport.p_itemlist[24].pickup = Pickup_Ammo;

	// item_health_half. Pickup health (10 points).
	playerExport.p_itemlist[25].pickup = Pickup_Health;

	// item_health_full. Pickup health (30 points).
	playerExport.p_itemlist[26].pickup = Pickup_Health;

	// item_puzzle_townkey. Key puzzle piece (Town level).
	playerExport.p_itemlist[27].pickup = Pickup_Puzzle;

	// item_puzzle_cog. Cog puzzle piece (Palace level).
	playerExport.p_itemlist[28].pickup = Pickup_Puzzle;

	// item_puzzle_shield. Sithra Shield puzzle item (Healer level).
	playerExport.p_itemlist[29].pickup = Pickup_Puzzle;

	// item_puzzle_potion. Potion puzzle item (Healer level).
	playerExport.p_itemlist[30].pickup = Pickup_Puzzle;

	// item_puzzle_plazacontainer. Container puzzle item (Plaza level).
	playerExport.p_itemlist[31].pickup = Pickup_Puzzle;

	// item_puzzle_slumcontainer. Full Container puzzle item (Slum level).
	playerExport.p_itemlist[32].pickup = Pickup_Puzzle;

	// item_puzzle_crystal. Crystal puzzle item (Academic level).
	playerExport.p_itemlist[33].pickup = Pickup_Puzzle;

	// item_puzzle_canyonkey. Key puzzle item (Canyon level).
	playerExport.p_itemlist[34].pickup = Pickup_Puzzle;

	// item_puzzle_hive2amulet. Amulet puzzle item (Hive 2 level).
	playerExport.p_itemlist[35].pickup = Pickup_Puzzle;

	// item_puzzle_hive2spear. Spear puzzle item (Hive 2 level).
	playerExport.p_itemlist[36].pickup = Pickup_Puzzle;

	// item_puzzle_hive2gem. Gem puzzle item (Hive 2 level).
	playerExport.p_itemlist[37].pickup = Pickup_Puzzle;

	// item_puzzle_minecartwheel. Mine Cart Wheel puzzle item (Mine 1 level).
	playerExport.p_itemlist[38].pickup = Pickup_Puzzle;

	// item_puzzle_ore. Unrefined Ore puzzle item (Mine 2 level).
	playerExport.p_itemlist[39].pickup = Pickup_Puzzle;

	// item_puzzle_refinedore. Refined Ore puzzle item (Mine 2 level).
	playerExport.p_itemlist[40].pickup = Pickup_Puzzle;

	// item_puzzle_dungeonkey. Amulet puzzle item (Dungeon level).
	playerExport.p_itemlist[41].pickup = Pickup_Puzzle;

	// item_puzzle_cloudkey. Key puzzle item (Cloud Quarters 2 level).
	playerExport.p_itemlist[42].pickup = Pickup_Puzzle;

	// item_puzzle_highpriestesskey. Key puzzle item (High Priestess level).
	playerExport.p_itemlist[43].pickup = Pickup_Puzzle;

	// item_puzzle_highpriestesssymbol. Key puzzle item (High Priestess level).
	playerExport.p_itemlist[44].pickup = Pickup_Puzzle;

	// item_puzzle_tome. Tome puzzle piece (2 Cloud levels).
	playerExport.p_itemlist[45].pickup = Pickup_Puzzle;

	// item_puzzle_tavernkey. Key puzzle piece (Ssdocks level).
	playerExport.p_itemlist[46].pickup = Pickup_Puzzle;

	// item_defense_tornado. Pickup for the Tornado defensive spell.
	playerExport.p_itemlist[47].pickup = Pickup_Defense;
	playerExport.p_itemlist[47].use = Use_Defence;
	playerExport.p_itemlist[47].weaponthink = DefenceThink_Tornado;

	// Initialise game variables.
	game.num_items = playerExport.p_num_items;
}

// Called by worldspawn.
void SetItemNames(void)
{
	gitem_t* item = &playerExport.p_itemlist[0];
	for (int i = 0; i < game.num_items; i++, item++)
		gi.configstring(CS_ITEMS + i, item->pickup_name);
}

#pragma endregion