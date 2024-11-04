//
// Heretic II
// Copyright 1998 Raven Software
//
// Created by JJS

//#include "p_types.h"
#include "player.h"
#include "g_local.h"
#include "p_weapon.h"
#include "m_player.h"
#include "fx.h"
#include "matrix.h"
#include "vector.h"
#include "g_skeletons.h"
#include "angles.h"

void SpellCastPowerup(edict_t *Caster, vec3_t StartPos, vec3_t AimAngles, vec3_t AimDir, float value);
void SpellCastBlueRing(edict_t *Caster, vec3_t StartPos, vec3_t AimAngles, vec3_t AimDir, float value);
void SpellCastMeteorBarrier(edict_t *Caster,vec3_t StartPos,vec3_t AimAngles,vec3_t AimDir,float Value);
void SpellCastTeleport(edict_t *Caster,vec3_t StartPos,vec3_t AimAngles,vec3_t AimDir,float Value);
void SpellCastMorph(edict_t *Caster,vec3_t StartPos,vec3_t AimAngles,vec3_t AimDir,float Value);
void SpellCastShield(edict_t *caster,vec3_t startpos,vec3_t aimangles,vec3_t aimdir,float Value);
void SpellCastDropTornado(edict_t *Caster,vec3_t StartPos,vec3_t AimAngles,vec3_t AimDir,float Value);

void Use_Defence(playerinfo_t *playerinfo, gitem_t *defence)
{
	playerinfo->pers.lastdefence = playerinfo->pers.defence;	
	playerinfo->pers.defence=defence;

	if(playerinfo->pers.defence&&playerinfo->pers.defence->ammo)
		playerinfo->def_ammo_index=ITEM_INDEX(P_FindItem(playerinfo->pers.defence->ammo));
	else
		playerinfo->def_ammo_index=0;
}

// ************************************************************************************************
// DefenceThink_DropTornado
// ----------------------
// ************************************************************************************************

void DefenceThink_Tornado(edict_t *Caster,char *Format,...)
{

	// Set up the Tornado's starting position and aiming angles then cast the spell.

	SpellCastDropTornado(Caster,Caster->s.origin,Caster->client->aimangles,NULL,0.0);

	// Take off mana
	if (!deathmatch->value || (deathmatch->value && !((int)dmflags->value & DF_INFINITE_MANA)))
		Caster->client->playerinfo.pers.inventory.Items[Caster->client->playerinfo.def_ammo_index]-= Caster->client->playerinfo.pers.defence->quantity;
}


// ************************************************************************************************

void DefenceThink_Powerup(edict_t *Caster, char *Format,...)
{
	playerinfo_t *playerinfo;
	playerinfo = &Caster->client->playerinfo;

	SpellCastPowerup(Caster, Caster->s.origin, NULL,NULL, 0.0F); 

	assert(playerinfo->def_ammo_index);

	if (!deathmatch->value || (deathmatch->value && !((int)dmflags->value & DF_INFINITE_MANA)))
		playerinfo->pers.inventory.Items[playerinfo->def_ammo_index] -= playerinfo->pers.defence->quantity;
}

// ************************************************************************************************

void DefenceThink_RingOfRepulsion(edict_t *Caster, char *Format,...)
{
	playerinfo_t *playerinfo;
	playerinfo = &Caster->client->playerinfo;

	SpellCastBlueRing(Caster, Caster->s.origin, NULL,NULL, 0.0F); 

	assert(playerinfo->def_ammo_index);

	if (!deathmatch->value || (deathmatch->value && !((int)dmflags->value & DF_INFINITE_MANA)))
		playerinfo->pers.inventory.Items[playerinfo->def_ammo_index] -= playerinfo->pers.defence->quantity;
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
