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

//TODO: is this layer of indirection needed? Can't we rewrite spell-casting functions (SpellCastDropTornado(), SpellCastPowerup() etc.) to take mana and work as weaponthink callbacks?
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
	SpellCastPowerup(caster);
	TakeMana(caster); //mxd
}

void DefenceThink_RingOfRepulsion(edict_t* caster, char* format, ...)
{
	SpellCastBlueRing(caster);
	TakeMana(caster); //mxd
}

void DefenceThink_MeteorBarrier(edict_t* caster, char* format, ...)
{
	SpellCastMeteorBarrier(caster, caster->s.origin);
	//mxd. Mana decrement is handled per-meteor in SpellCastMeteorBarrier(). 
}

void DefenceThink_Morph(edict_t* caster, char* format, ...)
{
	SpellCastMorph(caster, caster->s.origin, caster->client->aimangles);
	TakeMana(caster); //mxd
}

void DefenceThink_Teleport(edict_t* caster, char* format, ...)
{
	SpellCastTeleport(caster, caster->s.origin, NULL, NULL, 0.0f);
	TakeMana(caster); //mxd
}

void DefenceThink_Shield(edict_t* caster, char* format, ...)
{
	assert(caster->client != NULL);
	const playerinfo_t* info = &caster->client->playerinfo;

	// Make sure that there isn't already a shield in place.
	if (info->shield_timer < info->leveltime)
	{
		SpellCastShield(caster, caster->s.origin, NULL, NULL, 0.0f);
		TakeMana(caster); //mxd
	}
}