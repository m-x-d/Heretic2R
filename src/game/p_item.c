//
// p_item.c
//
// Copyright 1998 Raven Software
//

#include "p_items.h" //mxd
#include "spl_BlueRing.h" //mxd
#include "spl_meteorbarrier.h" //mxd
#include "spl_morph.h" //mxd
#include "spl_powerup.h" //mxd
#include "spl_shield.h" //mxd
#include "g_teleport.h" //mxd
#include "spl_tornado.h" //mxd
#include "g_local.h"

void Use_Defence(playerinfo_t* info, gitem_t* defence)
{
	info->pers.lastdefence = info->pers.defence;
	info->pers.defence = defence;

	if (info->pers.defence != NULL && info->pers.defence->ammo != NULL)
		info->def_ammo_index = ITEM_INDEX(P_FindItem(info->pers.defence->ammo));
	else
		info->def_ammo_index = 0;
}

static void TakeMana(const edict_t* caster) //mxd. Added to reduce code duplication.
{
	if (!DEATHMATCH || !(DMFLAGS & DF_INFINITE_MANA))
	{
		playerinfo_t* info = &caster->client->playerinfo;
		assert(info->def_ammo_index);
		info->pers.inventory.Items[info->def_ammo_index] -= info->pers.defence->quantity;
	}
}

void DefenceThink_Tornado(edict_t* caster, char* format, ...)
{
	SpellCastDropTornado(caster, caster->s.origin, caster->client->aimangles, NULL, 0.0f);
	TakeMana(caster); //mxd
}

void DefenceThink_Powerup(edict_t* caster, char* format, ...)
{
	SpellCastPowerup(caster, caster->s.origin, NULL, NULL, 0.0f);
	TakeMana(caster); //mxd
}

void DefenceThink_RingOfRepulsion(edict_t* caster, char* format, ...)
{
	SpellCastBlueRing(caster, caster->s.origin, NULL, NULL, 0.0f);
	TakeMana(caster); //mxd
}

// ************************************************************************************************

void DefenceThink_MeteorBarrier(edict_t *Caster, char *Format,...)
{
	playerinfo_t *playerinfo;

	playerinfo = &Caster->client->playerinfo;

	SpellCastMeteorBarrier(Caster, Caster->s.origin, NULL, NULL, 0.0F);

	assert(playerinfo->def_ammo_index);

}


// ************************************************************************************************

void DefenceThink_Morph(edict_t *Caster, char *Format,...)
{
	playerinfo_t *playerinfo;
	playerinfo = &Caster->client->playerinfo;

	// Set up the Meteor-barrier's aiming angles and starting position then cast the spell.

	SpellCastMorph(Caster, Caster->s.origin, Caster->client->aimangles, NULL, 0.0F);

	assert(playerinfo->def_ammo_index);
	if (!deathmatch->value || (deathmatch->value && !((int)dmflags->value & DF_INFINITE_MANA)))
		playerinfo->pers.inventory.Items[playerinfo->def_ammo_index] -= playerinfo->pers.defence->quantity;
}

// ************************************************************************************************
void DefenceThink_Teleport(edict_t *Caster, char *Format,...)
{
	playerinfo_t *playerinfo;
	playerinfo = &Caster->client->playerinfo;

	// Set up the teleport and then do it

	SpellCastTeleport(Caster, Caster->s.origin, NULL, NULL, 0.0F);

	assert(playerinfo->def_ammo_index);
	if (!deathmatch->value || (deathmatch->value && !((int)dmflags->value & DF_INFINITE_MANA)))
		playerinfo->pers.inventory.Items[playerinfo->def_ammo_index] -= playerinfo->pers.defence->quantity;
}

// ************************************************************************************************
void DefenceThink_Shield(edict_t *Caster, char *Format,...)
{
	playerinfo_t *playerinfo;
	assert(Caster->client);
	playerinfo = &Caster->client->playerinfo;

	// Make sure that there isn't already a shield in place.
	if (playerinfo->shield_timer < playerinfo->leveltime)
	{	// Don't do anything if there is already a shield in place.
		// Set up the shield and then do it

		SpellCastShield(Caster, Caster->s.origin, NULL, NULL, 0.0F);

		assert(playerinfo->def_ammo_index);
		if (!deathmatch->value || (deathmatch->value && !((int)dmflags->value & DF_INFINITE_MANA)))
			playerinfo->pers.inventory.Items[playerinfo->def_ammo_index] -= playerinfo->pers.defence->quantity;
	}
}
