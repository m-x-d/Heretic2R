//
// g_shrine.c
//
// Copyright 1998 Raven Software
//

#include "g_Shrine.h" //mxd
#include "g_ShrineLocal.h" //mxd
#include "cl_strings.h"
#include "g_combat.h" //mxd
#include "g_itemstats.h"
#include "p_anims.h"
#include "p_main.h"
#include "FX.h"
#include "Random.h"
#include "Vector.h"

#define INVUNERABILITY_TIME 2.0f
#define SF_PERMANENT		1 //mxd

typedef enum ShrineType_s //mxd. Made typed.
{
	SHRINE_MANA,
	SHRINE_LUNGS,
	SHRINE_ARMOR_SILVER,
	SHRINE_ARMOR_GOLD,
	SHRINE_LIGHT,
	SHRINE_SPEED,
	SHRINE_HEAL,
	SHRINE_STAFF,
	SHRINE_GHOST,
	SHRINE_REFLECT,
	SHRINE_POWERUP,
	SHRINE_RANDOM
} ShrineType_t;

// Set up those shrines that are randomly selectable.
static char delay_text[] = "shrine respawn delay";
static char chaos_text[] = "chaos shrine touch";
static char health_text[] = "health shrine touch";
static char mana_text[] = "mana shrine touch";
static char light_text[] = "light shrine touch";
static char lungs_text[] = "lungs shrine touch";
static char run_text[] = "run shrine touch";
static char staff_text[] = "staff shrine touch";
static char powerup_text[] = "powerup shrine touch";
static char ghost_text[] = "ghost shrine touch";
static char reflect_text[] = "reflect shrine touch";
static char armor_gold_text[] = "armor gold shrine touch";
static char armor_silver_text[] = "armor silver shrine touch";

// Remove all shrine effects associated with a player. Used when he's turned into a chicken.
void PlayerKillShrineFX(edict_t* self)
{
	playerinfo_t* info = &self->client->playerinfo;

	assert(info);

	// Turn off the reflection powerup.
	info->reflect_timer = level.time - 1.0f;
	self->s.renderfx &= ~RF_REFLECTION;

	// Turn off the invisibility powerup.
	info->ghost_timer = level.time - 1.0f;
	self->s.renderfx &= ~RF_TRANS_GHOST;

	// Turn off the light powerup.
	info->light_timer = level.time - 1.0f;
	self->s.effects &= ~EF_LIGHT_ENABLED;

	// Turn off the run shrine effect.
	self->s.effects &= ~EF_SPEED_ACTIVE;
	gi.RemoveEffects(&self->s, FX_FOOT_TRAIL);

	// Kill any lights that may already be out there for this player.
	gi.RemoveEffects(&self->s, FX_PLAYER_TORCH);

	// Kill lungs.
	info->lungs_timer = 0.0f;

	// Remove Armor.
	info->pers.armor_count = 0.0f;
	info->pers.armortype = ARMOR_TYPE_NONE; //mxd. ARMOR_NONE in original version.

	SetupPlayerinfo_effects(self);
	P_PlayerUpdateModelAttributes(info);
	WritePlayerinfo_effects(self);

	// Remove Staff powerup.
	info->pers.stafflevel = STAFF_LEVEL_BASIC;

	// Remove Weapons powerup.
	info->powerup_timer = level.time - 1.0f;

	// Kill any tomes that may already be out there for this player.
	gi.RemoveEffects(&self->s, FX_TOME_OF_POWER);

	// Turn off the tome at the client effect end through client flags that are passed down.
	self->s.effects &= ~EF_POWERUP_ENABLED;
}

// This is the routine that restarts any client effects that need to be running.
// For instance, recovery of a saved game, where for example, the torch is active.
void PlayerRestartShrineFX(edict_t* self)
{
	// If we have a light, restart it's sfx.
	if (self->s.effects & EF_LIGHT_ENABLED)
	{
		gi.RemoveEffects(&self->s, FX_PLAYER_TORCH);
		gi.CreateEffect(&self->s, FX_PLAYER_TORCH, CEF_OWNERS_ORIGIN, NULL, "");
	}

	// If we have Tome of Power, restart it's sfx.
	if (self->s.effects & EF_POWERUP_ENABLED)
	{
		gi.RemoveEffects(&self->s, FX_TOME_OF_POWER);
		gi.CreateEffect(&self->s, FX_TOME_OF_POWER, CEF_OWNERS_ORIGIN, NULL, "");
	}

	// If we have a Speed powerup, restart it's sfx.
	if (self->s.effects & EF_SPEED_ACTIVE)
	{
		gi.RemoveEffects(&self->s, FX_FOOT_TRAIL);
		gi.CreateEffect(&self->s, FX_FOOT_TRAIL, CEF_OWNERS_ORIGIN, NULL, "");
	}
}

// Called from the random Shrine - which one do we want to do?
static void PlayerRandomShrineEffect(edict_t* self, const ShrineType_t value)
{
	switch (value)
	{
		case SHRINE_ARMOR_SILVER:
			PlayerShrineArmorSilverEffect(self);
			break;

		case SHRINE_ARMOR_GOLD:
			PlayerShrineArmorGoldEffect(self);
			break;

		case SHRINE_LIGHT:
			PlayerShrineLightEffect(self);
			break;

		case SHRINE_HEAL:
			PlayerShrineHealthEffect(self);
			break;

		case SHRINE_STAFF:
			PlayerShrineStaffEffect(self);
			break;

		case SHRINE_LUNGS:
			PlayerShrineLungsEffect(self);
			break;

		case SHRINE_GHOST:
			PlayerShrineGhostEffect(self);
			break;

		case SHRINE_REFLECT:
			PlayerShrineReflectEffect(self);
			break;

		case SHRINE_POWERUP:
			PlayerShrinePowerupEffect(self);
			break;

		case SHRINE_MANA:
			PlayerShrineManaEffect(self);
			break;

		case SHRINE_SPEED:
			PlayerShrineSpeedEffect(self);
			break;

		default:
			PlayerShrinePowerupEffect(self); //mxd. G_PlayerActionShrineEffect() uses player_shrine_mana_effect() here in original version.
			break;
	}
}

void G_PlayerActionShrineEffect(const playerinfo_t* playerinfo)
{
	edict_t* self = playerinfo->self;
	PlayerRandomShrineEffect(self, self->shrine_type); //mxd. Reduce code duplication. 
}

// Wait till we can use this shrine again.
static void DelayThink(edict_t* self)
{
	// Handle changing shrine types in deathmatch.
	if (DEATHMATCH)
	{
		if (self->oldtouch == ShrineArmorGoldTouch)
			self->owner->touch = ShrineArmorSilverTouch; // If we were gold in deathmatch, we won't be again.
		else if (self->oldtouch == ShrineArmorSilverTouch && irand(0, 8) == 0)
			self->owner->touch = ShrineArmorGoldTouch; // 1 in 9 chance in deathmatch an armor shrine turns gold.
	}
	else
	{
		self->owner->touch = self->oldtouch; // Restore the touch pad.
	}

	// Setup the destination entity of the teleport.
	edict_t* dest = G_Find(NULL, FOFS(targetname), self->owner->target);

	if (dest != NULL)
	{
		// Make the ball appear in the middle.
		if (self->owner->touch == ShrineArmorGoldTouch)
			dest->style = 7;
		else if (self->owner->touch == ShrineArmorSilverTouch)
			dest->style = 6;

		vec3_t angles;
		VectorScale(dest->s.angles, ANGLE_TO_RAD, angles);

		vec3_t direction;
		DirFromAngles(angles, direction);

		dest->PersistantCFX = gi.CreatePersistantEffect(&dest->s, FX_SHRINE_BALL, CEF_BROADCAST, dest->s.origin, "db", direction, (byte)(dest->style - 1));
	}

	G_SetToFree(self);
}

// Either kill or set this shrine node to unusable for a while.
static void UpdateShrineNode(edict_t* self) //mxd. Named 'deal_with_shrine_node' in original version.
{
	// Set up a delay so we can't use this shrine for a while.
	if (DEATHMATCH || (self->spawnflags & SF_PERMANENT))
	{
		edict_t* delay = G_Spawn();

		delay->svflags |= SVF_NOCLIENT;
		delay->movetype = PHYSICSTYPE_NONE;
		delay->solid = SOLID_NOT;
		delay->think = DelayThink;
		delay->owner = self;
		delay->oldtouch = self->touch;
		delay->classname = delay_text; //TODO: is this ever used?..

		float delay_time = SHRINE_DELAY;

		if (DEATHMATCH)
		{
			const float clients = max(2.0f, (float)game.num_clients);
			delay_time *= sqrtf(2.0f / clients); // Spawn more frequently when more players.
			delay_time = max(5.0f, delay_time); // Sanity check.
		}

		delay->nextthink = level.time + delay_time;

		gi.linkentity(delay);
	}

	// Turn off the touch for this shrine.
	self->touch = NULL;

	// Setup the destination entity of the teleport.
	edict_t* dest = G_Find(NULL, FOFS(targetname), self->target);

	if (dest == NULL)
		return;

	// But kill the shrine ball that's out there for this shrine.
	gi.RemoveEffects(&dest->s, FX_SHRINE_BALL);

	// Kill the glowing ball in the middle.
	if (dest->PersistantCFX > 0)
	{
		gi.RemovePersistantEffect(dest->PersistantCFX, REMOVE_SHRINE);
		dest->PersistantCFX = 0;
	}

	// Make the shrine ball explode.
	vec3_t angles;
	VectorScale(dest->s.angles, ANGLE_TO_RAD, angles);

	vec3_t direction;
	DirFromAngles(angles, direction);

	gi.CreateEffect(&dest->s, FX_SHRINE_BALL_EXPLODE, CEF_OWNERS_ORIGIN, dest->s.origin, "db", direction, (byte)(dest->style - 1));
}

static void ShrineRestorePlayer(edict_t* other) //mxd. Named 'ShrineRestorePlayer' in original version.
{
	// Stop us from being on fire.
	if (other->fire_damage_time > level.time)
	{
		other->fire_damage_time = 0.0f;
		other->s.effects &= ~EF_ON_FIRE; // Turn off CFX too.
	}

	// Stop bleeding and Restore limbs. //FIXME: maybe do some cool temp effect on these nodes to show they respawned?
	ResetPlayerBaseNodes(other); //mxd. Removes PLAYER_FLAG_BLEED, PLAYER_FLAG_NO_LARM and PLAYER_FLAG_NO_RARM flags, among other things.
}

//mxd. Added to reduce code duplication.
static void PlayerShrineStartUseAnimation(edict_t* player, const ShrineType_t shrine_type)
{
	player->shrine_type = shrine_type; // Tell us what sort of shrine we just hit.
	P_PlayerAnimSetLowerSeq(&player->client->playerinfo, ASEQ_SHRINE); // Initialise the shrine animation.
	player->client->shrine_framenum = level.time + INVUNERABILITY_TIME; // Make us invulnerable for a couple of seconds.
}

#pragma region ========================== HEALTH SHRINE ==========================

// Fire off the health shrine effect.
static void PlayerShrineHealthEffect(edict_t* self) //mxd. Named 'player_shrine_health_effect' in original version.
{
	gi.CreateEffect(&self->s, FX_SHRINE_HEALTH, CEF_OWNERS_ORIGIN, NULL, "");
	gi.sound(self, CHAN_ITEM, gi.soundindex("items/shrine4.wav"), 1.0f, ATTN_NORM, 0.0f);
}

static void ShrineHealCore(edict_t* other) //mxd. Named 'shrine_heal_core' in original version.
{
	if (other->deadflag != DEAD_NO)
		return;

	// If we are a chicken, lets make us a player again. Don't give him anything else.
	if (other->flags & FL_CHICKEN)
	{
		other->morph_timer = (int)level.time - 1; //BUGFIX: mxd. 'level.time - 0.1f' in original version. 
		return;
	}

	// Give us some health.
	other->health = min(SHRINE_MAX_HEALTH, other->health + SHRINE_HEALTH);

	// Restore dismemberment, and stop us being on fire.
	ShrineRestorePlayer(other);
}

// Fire off a heal effect and give us some health.
static void ShrineHealTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf) //mxd. Named 'shrine_heal_touch' in original version.
{
	// If we aren't a player, forget it.
	if (other->client == NULL)
		return;

	ShrineHealCore(other);
	gi.gamemsg_centerprintf(other, GM_S_HEALTH);

	// If we are in deathmatch, a chicken or underwater (or underwater chicken. In deathmatch!), don't make us go through the shrine anim,
	// just start the effect, give us whatever, and leave it at that.
	if (DEATHMATCH || (other->flags & FL_CHICKEN) || (other->client->playerinfo.flags & PLAYER_FLAG_WATER))
		PlayerShrineHealthEffect(other);
	else
		PlayerShrineStartUseAnimation(other, SHRINE_HEAL); //mxd

	// Decide whether to delete this shrine or disable it for a while.
	UpdateShrineNode(self);
}

// QUAKED shrine_heal (.5 .3 .5) ? PERMANENT
void SP_shrine_heal_trigger(edict_t* ent) //mxd. Named 'shrine_heal' in original version.
{
	ent->movetype = PHYSICSTYPE_NONE;
	ent->svflags |= SVF_NOCLIENT;
	ent->solid = SOLID_TRIGGER;
	ent->shrine_type = SHRINE_HEAL;
	ent->classname = health_text;

	if (!DEATHMATCH || (DEATHMATCH && !(DMFLAGS & DF_NO_SHRINE)))
		ent->touch = ShrineHealTouch;

	if (DEATHMATCH && (DMFLAGS & DF_SHRINE_CHAOS) && !(DMFLAGS & DF_NO_SHRINE))
	{
		ent->shrine_type = SHRINE_RANDOM;
		ent->touch = ShrineRandomTouch;
	}

	gi.setmodel(ent, ent->model);
	gi.linkentity(ent);
}

#pragma endregion

#pragma region ========================== SILVER ARMOR SHRINE ==========================

// Fire off the armor shrine effect.
static void PlayerShrineArmorSilverEffect(edict_t* self) //mxd. Named 'player_shrine_armor_silver_effect' in original version.
{
	gi.CreateEffect(&self->s, FX_SHRINE_ARMOR, CEF_OWNERS_ORIGIN, NULL, "");
	gi.sound(self, CHAN_ITEM, gi.soundindex("items/shrine2.wav"), 1.0f, ATTN_NORM, 0.0f);
}

static void ShrineArmorSilverCore(edict_t* other) //mxd. Named 'shrine_armor_silver_core' in original version.
{
	if (other->deadflag != DEAD_NO)
		return;

	// If we are a chicken, lets make us a player again. Don't give him anything else.
	if (other->flags & FL_CHICKEN)
	{
		other->morph_timer = (int)level.time - 1; //BUGFIX: mxd. 'level.time - 0.1f' in original version. 
		return;
	}

	// Add armor to player.
	client_persistant_t* pers = &other->client->playerinfo.pers; //mxd
	if (pers->armortype == ARMOR_TYPE_GOLD && pers->armor_count >= gold_armor_info.max_armor / 2.0f)
	{
		pers->armor_count = gold_armor_info.max_armor;
	}
	else
	{
		pers->armortype = ARMOR_TYPE_SILVER;
		pers->armor_count = silver_armor_info.max_armor;
	}

	SetupPlayerinfo_effects(other);
	P_PlayerUpdateModelAttributes(&other->client->playerinfo);
	WritePlayerinfo_effects(other);

	// Restore dismemberment, and stop us being on fire.
	ShrineRestorePlayer(other);
}

// Fire off an effect and give us some armor.
static void ShrineArmorSilverTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf) //mxd. Named 'shrine_armor_silver_touch' in original version.
{
	// If we aren't a player, forget it.
	if (other->client == NULL)
		return;

	ShrineArmorSilverCore(other);
	gi.gamemsg_centerprintf(other, GM_S_SILVER);

	// If we are in deathmatch, don't make us go through the shrine anim, just start the effect, give us whatever, and leave it at that.
	if (DEATHMATCH || (other->flags & FL_CHICKEN) || (other->client->playerinfo.flags & PLAYER_FLAG_WATER))
	{
		if (other->client->playerinfo.pers.armortype == ARMOR_TYPE_SILVER)
			PlayerShrineArmorSilverEffect(other);
		else
			PlayerShrineArmorGoldEffect(other);
	}
	else
	{
		PlayerShrineStartUseAnimation(other, SHRINE_ARMOR_SILVER); //mxd
	}

	// Decide whether to delete this shrine or disable it for a while.
	UpdateShrineNode(self);
}

// QUAKED shrine_armor (.5 .3 .5) ? PERMANENT
void SP_shrine_armor_silver_trigger(edict_t* ent) //mxd. Named 'shrine_armor' in original version.
{
	ent->movetype = PHYSICSTYPE_NONE;
	ent->svflags |= SVF_NOCLIENT;
	ent->solid = SOLID_TRIGGER;
	ent->shrine_type = SHRINE_ARMOR_SILVER;
	ent->classname = armor_silver_text;

	if (!DEATHMATCH || (DEATHMATCH && !(DMFLAGS & DF_NO_SHRINE)))
		ent->touch = ShrineArmorSilverTouch;

	if (DEATHMATCH && (DMFLAGS & DF_SHRINE_CHAOS) && !(DMFLAGS & DF_NO_SHRINE))
	{
		ent->shrine_type = SHRINE_RANDOM;
		ent->touch = ShrineRandomTouch;
	}

	gi.setmodel(ent, ent->model);
	gi.linkentity(ent);
}

#pragma endregion

#pragma region ========================== GOLD ARMOR SHRINE ==========================

// Fire off the gold armor shrine effect.
static void PlayerShrineArmorGoldEffect(edict_t* self) //mxd. Named 'player_shrine_armor_gold_effect' in original version.
{
	gi.CreateEffect(&self->s, FX_SHRINE_ARMOR, CEF_OWNERS_ORIGIN | CEF_FLAG6, NULL, "");
	gi.sound(self, CHAN_ITEM, gi.soundindex("items/shrine2.wav"), 1.0f, ATTN_NORM, 0.0f);
}

static void ShrineArmorGoldCore(edict_t* other) //mxd. Named 'shrine_armor_gold_core' in original version.
{
	if (other->deadflag != DEAD_NO)
		return;

	// If we are a chicken, lets make us a player again. Don't give him anything else.
	if (other->flags & FL_CHICKEN)
	{
		other->morph_timer = (int)level.time - 1; //BUGFIX: mxd. 'level.time - 0.1f' in original version. 
		return;
	}

	// Add gold armor to player.
	other->client->playerinfo.pers.armortype = ARMOR_TYPE_GOLD;
	other->client->playerinfo.pers.armor_count = gold_armor_info.max_armor;

	SetupPlayerinfo_effects(other);
	P_PlayerUpdateModelAttributes(&other->client->playerinfo);
	WritePlayerinfo_effects(other);

	// Restore dismemberment, and stop us being on fire.
	ShrineRestorePlayer(other);
}

// Fire off an effect and give us some armor.
static void ShrineArmorGoldTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf) //mxd. Named 'shrine_armor_gold_touch' in original version.
{
	//  If we aren't a player, forget it.
	if (other->client == NULL)
		return;

	ShrineArmorGoldCore(other);
	gi.gamemsg_centerprintf(other, GM_S_GOLD);

	// If we are in deathmatch, don't make us go through the shrine anim, just start the effect, give us whatever, and leave it at that.
	if (DEATHMATCH || (other->flags & FL_CHICKEN) || (other->client->playerinfo.flags & PLAYER_FLAG_WATER))
		PlayerShrineArmorGoldEffect(other);
	else
		PlayerShrineStartUseAnimation(other, SHRINE_ARMOR_GOLD); //mxd

	// Decide whether to delete this shrine or disable it for a while.
	UpdateShrineNode(self);
}

// QUAKED shrine_armor_gold (.5 .3 .5) ? PERMANENT
void SP_shrine_armor_gold_trigger(edict_t* ent) //mxd. Named 'shrine_armor_gold' in original version.
{
	ent->movetype = PHYSICSTYPE_NONE;
	ent->svflags |= SVF_NOCLIENT;
	ent->solid = SOLID_TRIGGER;
	ent->shrine_type = SHRINE_ARMOR_GOLD;
	ent->classname = armor_gold_text;

	// No touch if flags say so.
	if (!DEATHMATCH || (DEATHMATCH && !(DMFLAGS & DF_NO_SHRINE)))
		ent->touch = ShrineArmorGoldTouch;

	if (DEATHMATCH && (DMFLAGS & DF_SHRINE_CHAOS) && !(DMFLAGS & DF_NO_SHRINE))
	{
		ent->shrine_type = SHRINE_RANDOM;
		ent->touch = ShrineRandomTouch;
	}

	gi.setmodel(ent, ent->model);
	gi.linkentity(ent);
}

#pragma endregion

#pragma region ========================== STAFF POWERUP SHRINE ==========================

// Fire off the staff shrine effect.
static void PlayerShrineStaffEffect(edict_t* self) //mxd. Named 'player_shrine_staff_effect' in original version.
{
	int	flags = CEF_OWNERS_ORIGIN;
	char* snd_name; //mxd

	// Do the SHRINE sound.
	if (self->client->playerinfo.pers.stafflevel == STAFF_LEVEL_POWER2)
	{
		flags |= CEF_FLAG6;
		snd_name = "weapons/FirewallPowerCast.wav";
	}
	else
	{
		snd_name = "items/shrine7.wav";
	}

	gi.sound(self, CHAN_ITEM, gi.soundindex(snd_name), 1.0f, ATTN_NORM, 0.0f);

	// Start up the shrine staff effect.
	gi.CreateEffect(&self->s, FX_SHRINE_STAFF, flags, NULL, "");
}

static void ShrineStaffCore(edict_t* other) //mxd. Named 'shrine_staff_core' in original version.
{
	if (other->deadflag != DEAD_NO)
		return;

	// If we are a chicken, lets make us a player again. Don't give him anything else.
	if (other->flags & FL_CHICKEN)
	{
		other->morph_timer = (int)level.time - 1; //BUGFIX: mxd. 'level.time - 0.1f' in original version. 
		return;
	}

	// Add onto his staff.
	if (other->client->playerinfo.pers.stafflevel < STAFF_LEVEL_MAX - 1)
	{
		other->client->playerinfo.pers.stafflevel++;

		SetupPlayerinfo_effects(other);
		P_PlayerUpdateModelAttributes(&other->client->playerinfo);
		WritePlayerinfo_effects(other);
	}

	// Restore dismemberment, and stop us being on fire.
	ShrineRestorePlayer(other);
}

// Fire off an effect and give us a staff powerup.
static void ShrineStaffTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf) //mxd. Named 'shrine_staff_touch' in original version.
{
	// If we aren't a player, forget it.
	if (other->client == NULL)
		return;

	ShrineStaffCore(other);
	gi.gamemsg_centerprintf(other, GM_S_BLADE);

	// If we are in deathmatch, don't make us go through the shrine anim, just start the effect, give us whatever, and leave it at that.
	if (DEATHMATCH || (other->flags & FL_CHICKEN) || (other->client->playerinfo.flags & PLAYER_FLAG_WATER))
		PlayerShrineStaffEffect(other);
	else
		PlayerShrineStartUseAnimation(other, SHRINE_STAFF); //mxd

	// Decide whether to delete this shrine or disable it for a while.
	UpdateShrineNode(self);
}

// QUAKED shrine_staff (.5 .3 .5) ? PERMANENT
void SP_shrine_staff_trigger(edict_t* ent) //mxd. Named 'shrine_staff' in original version.
{
	ent->movetype = PHYSICSTYPE_NONE;
	ent->svflags |= SVF_NOCLIENT;
	ent->solid = SOLID_TRIGGER;
	ent->shrine_type = SHRINE_STAFF;
	ent->classname = staff_text;

	if (!DEATHMATCH || (DEATHMATCH && !(DMFLAGS & DF_NO_SHRINE)))
		ent->touch = ShrineStaffTouch;

	if (DEATHMATCH && (DMFLAGS & DF_SHRINE_CHAOS) && !(DMFLAGS & DF_NO_SHRINE))
	{
		ent->shrine_type = SHRINE_RANDOM;
		ent->touch = ShrineRandomTouch;
	}

	gi.setmodel(ent, ent->model);
	gi.linkentity(ent);
}

#pragma endregion

#pragma region ========================== LUNGS SHRINE ==========================

// Fire off the lungs shrine effect.
static void PlayerShrineLungsEffect(edict_t* self) //mxd. Named 'player_shrine_lungs_effect' in original version.
{
	gi.CreateEffect(&self->s, FX_SHRINE_LUNGS, CEF_OWNERS_ORIGIN, NULL, "");
	gi.sound(self, CHAN_ITEM, gi.soundindex("items/shrine9.wav"), 1.0f, ATTN_NORM, 0.0f);
}

static void ShrineLungsCore(edict_t* other) //mxd. Named 'shrine_lung_core' in original version.
{
	if (other->deadflag != DEAD_NO)
		return;

	// If we are a chicken, lets make us a player again. Don't give him anything else.
	if (other->flags & FL_CHICKEN)
	{
		other->morph_timer = (int)level.time - 1; //BUGFIX: mxd. 'level.time - 0.1f' in original version.
		return;
	}

	// Add some time in on the timer for the lungs.
	other->client->playerinfo.lungs_timer = LUNGS_DURATION;

	// Restore dismemberment, and stop us being on fire.
	ShrineRestorePlayer(other);
}

// Fire off an effect and give us lung power.

void shrine_lung_touch	(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	// If we aren't a player, forget it.

	if (!other->client)
		return;

	ShrineLungsCore(other);

	gi.gamemsg_centerprintf(other, GM_S_LUNGS);

	// If we are in death match, don't make us go through the shrine anim, just start the effect,
	// give us whatever, and leave it at that.

	if (deathmatch->value || (other->flags & FL_CHICKEN) || (other->client->playerinfo.flags & PLAYER_FLAG_WATER))
	{
		PlayerShrineLungsEffect(other);
	}
	else
	{
		// Tell us what sort of shrine we just hit.

		other->shrine_type = SHRINE_LUNGS;

		// Initialise the shrine animation.

		P_PlayerAnimSetLowerSeq(&other->client->playerinfo, ASEQ_SHRINE);

		// Make us invulnerable for a couple of seconds.

		other->client->shrine_framenum = level.time + INVUNERABILITY_TIME;
	}

	// Decide whether to delete this shrine or disable it for a while.

	UpdateShrineNode(self);
}

/*QUAKED shrine_lung (.5 .3 .5) ? PERMANENT
*/
void SP_shrine_lungs_trigger (edict_t *ent)
{
	ent->movetype = PHYSICSTYPE_NONE;
	ent->svflags |= SVF_NOCLIENT;
	ent->solid = SOLID_TRIGGER;
	ent->shrine_type = SHRINE_LUNGS;
	ent->classname = lungs_text;

	if(!deathmatch->value || (deathmatch->value && !((int)dmflags->value & DF_NO_SHRINE)))
		ent->touch = shrine_lung_touch;

	if(deathmatch->value && ((int)dmflags->value & DF_SHRINE_CHAOS) && !((int)dmflags->value & DF_NO_SHRINE))
	{
		ent->shrine_type = SHRINE_RANDOM;
		ent->touch = ShrineRandomTouch;
	}

	gi.setmodel(ent, ent->model);
	gi.linkentity (ent);
}

#pragma endregion

// ************************************************************************************************
// Light Shrine
// ************************************************************************************************

// Fire off the shrine light effect .

void PlayerShrineLightEffect(edict_t *self)
{
	assert(self->client);

	// Kill any lights that may already be out there for this player.

	gi.RemoveEffects(&self->s, FX_PLAYER_TORCH);

	// Create the light and the tome of power.

	gi.CreateEffect(&self->s, FX_PLAYER_TORCH, CEF_OWNERS_ORIGIN, NULL, "");

	// Start up the shrine light effect.

	gi.CreateEffect(&self->s, FX_SHRINE_LIGHT, CEF_OWNERS_ORIGIN, NULL, "");

	// Do the SHRINE sound.

	gi.sound(self,CHAN_ITEM,gi.soundindex("items/shrine8.wav"),1,ATTN_NORM,0);

}

void shrine_light_core(edict_t *self, edict_t *other)
{
	if (other->deadflag != DEAD_NO)
		return;

	// If we are a chicken, lets make us a player again.  Don't give him anything else.
	if (other->flags & FL_CHICKEN)
	{
		other->morph_timer = level.time - 0.1;
		return;
	}

	// Add some time in on the timer for the light.

	other->client->playerinfo.light_timer = level.time + LIGHT_DURATION;

	// Turn on the light.

	other->s.effects |= EF_LIGHT_ENABLED;

	// restore dismemberment, and stop us being on fire
	ShrineRestorePlayer(other);

}

// Fire off an effect and give us some light.

void shrine_light_touch	(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	// If we aren't a player, forget it.

	if (!other->client)
		return;

	shrine_light_core(self,other);

	gi.gamemsg_centerprintf(other, GM_S_LIGHT);

	// If we are in death match, don't make us go through the shrine anim, just start the effect,
	// give us whatever, and leave it at that.

	if (deathmatch->value || (other->flags & FL_CHICKEN) || (other->client->playerinfo.flags & PLAYER_FLAG_WATER))
	{
		PlayerShrineLightEffect(other);
	}
	else
	{
		// Tell us what sort of shrine we just hit.

		other->shrine_type = SHRINE_LIGHT;

		// Initialise the shrine animation.

		P_PlayerAnimSetLowerSeq(&other->client->playerinfo, ASEQ_SHRINE);

		// Make us invunerable for a couple of seconds.

		other->client->shrine_framenum = level.time + INVUNERABILITY_TIME;
	}

	// Decide whether to delete this shrine or disable it for a while.

	UpdateShrineNode(self);
}

/*QUAKED shrine_light (.5 .3 .5) ? PERMANENT
*/

void SP_shrine_light_trigger (edict_t *ent)
{
	ent->movetype = PHYSICSTYPE_NONE;
	ent->svflags |= SVF_NOCLIENT;
	ent->solid = SOLID_TRIGGER;
	ent->shrine_type = SHRINE_LIGHT;
	ent->classname = light_text;

	if (!deathmatch->value || (deathmatch->value && !((int)dmflags->value & DF_NO_SHRINE)))
		ent->touch = shrine_light_touch;

	if(deathmatch->value && ((int)dmflags->value & DF_SHRINE_CHAOS) && !((int)dmflags->value & DF_NO_SHRINE))
	{
		ent->shrine_type = SHRINE_RANDOM;
		ent->touch = ShrineRandomTouch;
	}

	gi.setmodel(ent, ent->model);
	gi.linkentity (ent);
}

// ************************************************************************************************
// Mana Shrine
// ************************************************************************************************

// Fire off the shrine mana effect.

void PlayerShrineManaEffect(edict_t *self)
{
 	// Start up the shrine mana effect.
	
	gi.CreateEffect(&self->s, FX_SHRINE_MANA, CEF_OWNERS_ORIGIN, NULL, "");

	// Do the SHRINE sound.

	gi.sound(self,CHAN_ITEM,gi.soundindex("items/shrine1.wav"),1,ATTN_NORM,0);
}

void shrine_mana_core(edict_t *self, edict_t *other)
{
	if (other->deadflag != DEAD_NO)
		return;

	// If we are a chicken, lets make us a player again.  Don't give him anything else.
	if (other->flags & FL_CHICKEN)
	{
		other->morph_timer = level.time - 0.1;
		return;
	}

	// Add mana.

	other->client->playerinfo.pers.inventory.Items[ITEM_INDEX(P_FindItem("Off-mana"))] = 100;
    other->client->playerinfo.pers.inventory.Items[ITEM_INDEX(P_FindItem("Def-mana"))] = 100;

	// restore dismemberment, and stop us being on fire
	ShrineRestorePlayer(other);

}

// We hit the mana shrine pad, give us some manna, do the animation.

void shrine_mana_touch	(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	// If we aren't a player, forget it.

	if (!other->client)
		return;

	shrine_mana_core(self,other);

	gi.gamemsg_centerprintf(other, GM_S_MANA);

	// If we are in death match, don't make us go through the shrine anim, just start the effect,
	// give us whatever, and leave it at that.

	if (deathmatch->value || (other->flags & FL_CHICKEN) || (other->client->playerinfo.flags & PLAYER_FLAG_WATER))
	{
		PlayerShrineManaEffect(other);
	}
	else
	{
		// Tell us what sort of shrine we just hit.

		other->shrine_type = SHRINE_MANA;

		// Initialise the shrine animation.

		P_PlayerAnimSetLowerSeq(&other->client->playerinfo, ASEQ_SHRINE);

		// Make us invunerable for a couple of seconds.

		other->client->shrine_framenum = level.time + INVUNERABILITY_TIME;
	}

	// Decide whether to delete this shrine or disable it for a while.

	UpdateShrineNode(self);
}

/*QUAKED shrine_mana (.5 .3 .5) ? PERMANENT
*/

void SP_shrine_mana_trigger (edict_t *ent)
{
	ent->movetype = PHYSICSTYPE_NONE;
	ent->svflags |= SVF_NOCLIENT;
	ent->solid = SOLID_TRIGGER;
	ent->shrine_type = SHRINE_MANA;
	ent->classname = mana_text;

	if (!deathmatch->value || (deathmatch->value && !((int)dmflags->value & DF_NO_SHRINE)))
		ent->touch = shrine_mana_touch;

	if(deathmatch->value && ((int)dmflags->value & DF_SHRINE_CHAOS) && !((int)dmflags->value & DF_NO_SHRINE))
	{
		ent->shrine_type = SHRINE_RANDOM;
		ent->touch = ShrineRandomTouch;
	}

	gi.setmodel(ent, ent->model);
	gi.linkentity (ent);
}

// ************************************************************************************************
// Ghost (invisibilty) shrine.
// ************************************************************************************************

// Fire off the ghost shrine effect.

void PlayerShrineGhostEffect(edict_t *self)
{
	assert(self->client);
	
	// Start up the shrine ghost effect.

	gi.CreateEffect(&self->s, FX_SHRINE_GHOST, CEF_OWNERS_ORIGIN, NULL, "");

	// Do the SHRINE sound.

	gi.sound(self,CHAN_ITEM,gi.soundindex("items/shrine6.wav"),1,ATTN_NORM,0);
}

void shrine_ghost_core(edict_t *self,edict_t *other)
{
	if (other->deadflag != DEAD_NO)
		return;

	// If we are a chicken, lets make us a player again.  Don't give him anything else.
	if (other->flags & FL_CHICKEN)
	{
		other->morph_timer = level.time - 0.1;
		return;
	}

	// Add some time in on the timer for the ghost effect.

	other->client->playerinfo.ghost_timer = level.time + GHOST_DURATION;

	// Update the model attributes for ghosting.
	
	SetupPlayerinfo_effects(other);
	P_PlayerUpdateModelAttributes(&other->client->playerinfo);
	WritePlayerinfo_effects(other);

	// restore dismemberment, and stop us being on fire
	ShrineRestorePlayer(other);

}

// Fire off an effect and give us a ghosting for a while powerup.

void shrine_ghost_touch	(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	// If we aren't a player, forget it.

	if (!other->client)
		return;

	shrine_ghost_core(self,other);

	gi.gamemsg_centerprintf(other, GM_S_GHOST);

	// If we are in death match, don't make us go through the shrine anim, just start the effect,
	// give us whatever, and leave it at that.

	if (deathmatch->value || (other->flags & FL_CHICKEN) || (other->client->playerinfo.flags & PLAYER_FLAG_WATER))
	{
		PlayerShrineGhostEffect(other);
	}
	else
	{
		// Tell us what sort of shrine we just hit.

		other->shrine_type = SHRINE_GHOST;

		// Initialise the shrine animation.

		P_PlayerAnimSetLowerSeq(&other->client->playerinfo, ASEQ_SHRINE);

		// Make us invulnerable for a couple of seconds.

		other->client->shrine_framenum = level.time + INVUNERABILITY_TIME;
	}

	// Decide whether to delete this shrine or disable it for a while.
	
	UpdateShrineNode(self);
}

/*QUAKED shrine_ghost (.5 .3 .5) ? PERMANENT
*/

void SP_shrine_ghost_trigger (edict_t *ent)
{
	ent->movetype = PHYSICSTYPE_NONE;
	ent->svflags |= SVF_NOCLIENT;
	ent->solid = SOLID_TRIGGER;
	ent->shrine_type = SHRINE_GHOST;
	ent->classname = ghost_text;

	if (!deathmatch->value || (deathmatch->value && !((int)dmflags->value & DF_NO_SHRINE)))
		ent->touch = shrine_ghost_touch;

	if(deathmatch->value && ((int)dmflags->value & DF_SHRINE_CHAOS) && !((int)dmflags->value & DF_NO_SHRINE))
	{
		ent->shrine_type = SHRINE_RANDOM;
		ent->touch = ShrineRandomTouch;
	}

	gi.setmodel(ent, ent->model);
	gi.linkentity (ent);
}

// ************************************************************************************************
// Spell reflecting shrine.
// ************************************************************************************************

// Fire off the reflect shrine effect.

void PlayerShrineReflectEffect(edict_t *self)
{
	assert(self->client);

	// Start up the shrine staff effect.

	gi.CreateEffect(&self->s, FX_SHRINE_REFLECT, CEF_OWNERS_ORIGIN, NULL, "");

	// Do the SHRINE sound.

	gi.sound(self,CHAN_ITEM,gi.soundindex("items/shrine3.wav"),1,ATTN_NORM,0);
}

void shrine_reflect_core(edict_t *self,edict_t *other)
{
	if (other->deadflag != DEAD_NO)
		return;

	// If we are a chicken, lets make us a player again.  Don't give him anything else.
	if (other->flags & FL_CHICKEN)
	{
		other->morph_timer = level.time - 0.1;
		return;
	}

	// Add some time in on the timer for the reflectivity.

	if (deathmatch->value)
		other->client->playerinfo.reflect_timer = level.time + REFLECT_DURATION_DEATHMATCH;
	else
		other->client->playerinfo.reflect_timer = level.time + REFLECT_DURATION_SINGLE;

	// Update the model attributes for the reflection skin.
	
	SetupPlayerinfo_effects(other);
	P_PlayerUpdateModelAttributes(&other->client->playerinfo);
	WritePlayerinfo_effects(other);

	// restore dismemberment, and stop us being on fire
	ShrineRestorePlayer(other);

}

// Fire off an effect and give us a reflecting for a while powerup.

void shrine_reflect_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	// If we aren't a player, forget it.

	if (!other->client)
		return;

	shrine_reflect_core(self,other);

	gi.gamemsg_centerprintf(other, GM_S_REFLECT);

	// If we are in death match, don't make us go through the shrine anim, just start the effect,
	// give us whatever, and leave it at that.

	if (deathmatch->value || (other->flags & FL_CHICKEN) || (other->client->playerinfo.flags & PLAYER_FLAG_WATER))
	{
		PlayerShrineReflectEffect(other);
	}
	else
	{
		// Tell us what sort of shrine we just hit.

		other->shrine_type = SHRINE_REFLECT;

		// Initialise the shrine animation.

		P_PlayerAnimSetLowerSeq(&other->client->playerinfo, ASEQ_SHRINE);

		// Make us invunerable for a couple of seconds.

		other->client->shrine_framenum = level.time + INVUNERABILITY_TIME;
	}

	// Decide whether to delete this shrine or disable it for a while.

	UpdateShrineNode(self);
}

/*QUAKED shrine_reflect (.5 .3 .5) ? PERMANENT
*/

void SP_shrine_reflect_trigger (edict_t *ent)
{
	ent->movetype = PHYSICSTYPE_NONE;
	ent->svflags |= SVF_NOCLIENT;
	ent->solid = SOLID_TRIGGER;
	ent->shrine_type = SHRINE_REFLECT;
	ent->classname = reflect_text;

	if (!deathmatch->value || (deathmatch->value && !((int)dmflags->value & DF_NO_SHRINE)))
		ent->touch = shrine_reflect_touch;

	if(deathmatch->value && ((int)dmflags->value & DF_SHRINE_CHAOS) && !((int)dmflags->value & DF_NO_SHRINE))
	{
		ent->shrine_type = SHRINE_RANDOM;
		ent->touch = ShrineRandomTouch;
	}

	gi.setmodel(ent, ent->model);
	gi.linkentity (ent);
}

// ************************************************************************************************
// Spell powerup Shrine
// ************************************************************************************************

// Fire off the powerup shrine effect.

void PlayerShrinePowerupEffect(edict_t *self)
{
	assert(self->client);

	// Kill any tomes that may already be out there for this player.

	gi.RemoveEffects(&self->s, FX_TOME_OF_POWER);

	// Create the tome of power.

	gi.CreateEffect(&self->s, FX_TOME_OF_POWER, CEF_OWNERS_ORIGIN, NULL, "");

	// Start up the shrine powerup effect.

	gi.CreateEffect(&self->s, FX_SHRINE_POWERUP, CEF_OWNERS_ORIGIN, NULL, "");
	
	// Do the SHRINE sound.

	gi.sound(self,CHAN_ITEM,gi.soundindex("items/shrine5.wav"),1,ATTN_NORM,0);
}

// Fire off an effect and give us a powerup for a while.

void shrine_powerup_core (edict_t *self, edict_t *other)
{
	if (other->deadflag != DEAD_NO)
		return;

	// If we are a chicken, lets make us a player again.  Don't give him anything else.
	if (other->flags & FL_CHICKEN)
	{
		other->morph_timer = level.time - 0.1;
		return;
	}

	// Add some time in on the timer for the reflectivity.

	other->client->playerinfo.powerup_timer = level.time + POWERUP_DURATION;

	// Turn on the light at the client end through client flags that are passed to the client.

	other->s.effects |= EF_POWERUP_ENABLED;

	// restore dismemberment, and stop us being on fire
	ShrineRestorePlayer(other);

}

void shrine_powerup_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	// If we aren't a player, forget it.

	if (!other->client)
		return;

	shrine_powerup_core(self,other);

	gi.gamemsg_centerprintf(other, GM_S_POWERUP);

	// If we are in death match, don't make us go through the shrine anim, just/ start the effect,
	// give us whatever, and leave it at that.

	if (deathmatch->value || (other->flags & FL_CHICKEN) || (other->client->playerinfo.flags & PLAYER_FLAG_WATER))
	{
		PlayerShrinePowerupEffect(other);
	}
	else
	{
		// Tell us what sort of shrine we just hit.

		other->shrine_type = SHRINE_POWERUP;

		// Initialise the shrine animation.

		P_PlayerAnimSetLowerSeq(&other->client->playerinfo, ASEQ_SHRINE);

		// Make us invunerable for a couple of seconds.

		other->client->shrine_framenum = level.time + INVUNERABILITY_TIME;
	}

	// Decide whether to delete this shrine or disable it for a while.

	UpdateShrineNode(self);
}

/*QUAKED shrine_powerup (.5 .3 .5) ? PERMANENT
*/

void SP_shrine_powerup_trigger (edict_t *ent)
{	
	ent->movetype = PHYSICSTYPE_NONE;
	ent->svflags |= SVF_NOCLIENT;
	ent->solid = SOLID_TRIGGER;
	ent->shrine_type = SHRINE_POWERUP;
	ent->classname = powerup_text;

	if (!deathmatch->value || (deathmatch->value && !((int)dmflags->value & DF_NO_SHRINE)))
		ent->touch = shrine_powerup_touch;

	if(deathmatch->value && ((int)dmflags->value & DF_SHRINE_CHAOS) && !((int)dmflags->value & DF_NO_SHRINE))
	{
		ent->shrine_type = SHRINE_RANDOM;
		ent->touch = ShrineRandomTouch;
	}

	gi.setmodel(ent, ent->model);
	gi.linkentity (ent);
}

// ************************************************************************************************
// Speed Shrine
// ************************************************************************************************

// Fire off the powerup shrine effect.

void PlayerShrineSpeedEffect(edict_t *self)
{
	assert(self->client);

	// Start up the shrine powerup effect.

	gi.CreateEffect(&self->s, FX_SHRINE_SPEED, CEF_OWNERS_ORIGIN, NULL, "");
	
	// Do the SHRINE sound.

	gi.sound(self,CHAN_ITEM,gi.soundindex("items/shrine10.wav"),1,ATTN_NORM,0);
}

// Fire off an effect and give us double speed for a while

void shrine_speed_core (edict_t *self, edict_t *other)
{
	if (other->deadflag != DEAD_NO)
		return;

	// If we are a chicken, lets make us a player again.  Don't give him anything else.
	if (other->flags & FL_CHICKEN)
	{
		other->morph_timer = level.time - 0.1;
		return;
	}

	// Add some time in on the timer for speeding

	other->client->playerinfo.speed_timer = level.time + SPEED_DURATION;

	// Turn on the speed at the client level.
	other->s.effects |= EF_SPEED_ACTIVE;

	// Kill any tomes that may already be out there for this player.

	gi.RemoveEffects(&other->s, FX_FOOT_TRAIL);

	// Create the tome of power.

	gi.CreateEffect(&other->s, FX_FOOT_TRAIL, CEF_OWNERS_ORIGIN, NULL, "");

	// restore dismemberment, and stop us being on fire
	ShrineRestorePlayer(other);

}

void shrine_speed_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	// If we aren't a player, forget it.

	if (!other->client)
		return;

	shrine_speed_core(self,other);

	gi.gamemsg_centerprintf(other, GM_S_SPEED);

	// If we are in death match, don't make us go through the shrine anim, just/ start the effect,
	// give us whatever, and leave it at that.

	if (deathmatch->value || (other->flags & FL_CHICKEN) || (other->client->playerinfo.flags & PLAYER_FLAG_WATER))
	{
		PlayerShrineSpeedEffect(other);
	}
	else
	{
		// Tell us what sort of shrine we just hit.

		other->shrine_type = SHRINE_SPEED;

		// Initialise the shrine animation.

		P_PlayerAnimSetLowerSeq(&other->client->playerinfo, ASEQ_SHRINE);

		// Make us invunerable for a couple of seconds.

		other->client->shrine_framenum = level.time + INVUNERABILITY_TIME;
	}

	// Decide whether to delete this shrine or disable it for a while.

	UpdateShrineNode(self);
}

/*QUAKED shrine_speed (.5 .3 .5) ? PERMANENT
*/
void SP_shrine_speed_trigger (edict_t *ent)
{	
	ent->movetype = PHYSICSTYPE_NONE;
	ent->svflags |= SVF_NOCLIENT;
	ent->solid = SOLID_TRIGGER;
	ent->shrine_type = SHRINE_SPEED;
	ent->classname = run_text;

	if (no_runshrine->value)
		return;

	if (!deathmatch->value || (deathmatch->value && !((int)dmflags->value & DF_NO_SHRINE)))
		ent->touch = shrine_speed_touch;

	if(deathmatch->value && ((int)dmflags->value & DF_SHRINE_CHAOS) && !((int)dmflags->value & DF_NO_SHRINE))
	{
		ent->shrine_type = SHRINE_RANDOM;
		ent->touch = ShrineRandomTouch;
	}

	gi.setmodel(ent, ent->model);
	gi.linkentity (ent);
}

// ************************************************************************************************
// Random shrine.
// ************************************************************************************************

#define POSSIBLE_RANDOM_SHRINES 9

int	possible_shrines[POSSIBLE_RANDOM_SHRINES] =
{
	SHRINE_MANA,
	SHRINE_STAFF,
	SHRINE_ARMOR_SILVER,
	SHRINE_ARMOR_GOLD,
};


// Fire off an effect and give us a powerup for a while powerup.
void ShrineRandomTouch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	int		random_shrine_num;
	int		total_rand_count = 0;
	int		possible_shrines[10];

	// If we aren't a player, forget it!

	if (!other->client)
		return;

	if(other->client->playerinfo.flags&PLAYER_FLAG_BLEED||
	   other->client->playerinfo.flags&PLAYER_FLAG_NO_LARM||
	   other->client->playerinfo.flags&PLAYER_FLAG_NO_RARM)
	{
		// Always heal if they're missing a limb or bleeding to death - should it give full health
		// too though?

		random_shrine_num = SHRINE_HEAL;
	}
	else
	{
		
		// here's where we make the shrines clever. If we already have a shrine option, lets remove it from
		// the possible shrine list
		if (other->client->playerinfo.speed_timer < level.time)
		{
			if (!no_runshrine->value)
			{
				possible_shrines[total_rand_count] = SHRINE_SPEED;
				total_rand_count++;
			}
		}
		if (other->health < SHRINE_MAX_HEALTH)
		{
			possible_shrines[total_rand_count] = SHRINE_HEAL;
			total_rand_count++;
		}
		if (other->client->playerinfo.powerup_timer < level.time)
		{
			possible_shrines[total_rand_count] = SHRINE_POWERUP;
			total_rand_count++;
		}
		if (other->client->playerinfo.ghost_timer < level.time)
		{
			possible_shrines[total_rand_count] = SHRINE_GHOST;
			total_rand_count++;
		}
		if (other->client->playerinfo.reflect_timer < level.time)
		{
			if (!irand(0,1))
			{	// Reflection shrines appear 50% as often as other shrines.
				possible_shrines[total_rand_count] = SHRINE_REFLECT;
				total_rand_count++;
			}
		}
		if ((other->client->playerinfo.pers.armortype != ARMOR_TYPE_GOLD) ||
			(!other->client->playerinfo.pers.armor_count))
		{
			possible_shrines[total_rand_count] = SHRINE_ARMOR_GOLD;
			total_rand_count++;
		}
		if ((other->client->playerinfo.pers.inventory.Items[ITEM_INDEX(P_FindItem("Off-mana"))] < 100) ||
		    (other->client->playerinfo.pers.inventory.Items[ITEM_INDEX(P_FindItem("Def-mana"))] < 100))
		{
			possible_shrines[total_rand_count] = SHRINE_MANA;
			total_rand_count++;
		}
		if (other->client->playerinfo.pers.stafflevel < STAFF_LEVEL_MAX-1)
		{
			possible_shrines[total_rand_count] = SHRINE_STAFF;
			total_rand_count++;
		}
		if (((other->client->playerinfo.pers.armortype != ARMOR_TYPE_GOLD) &&
			(other->client->playerinfo.pers.armortype != ARMOR_TYPE_SILVER)) ||
			(!other->client->playerinfo.pers.armor_count))
		{
			possible_shrines[total_rand_count] = SHRINE_ARMOR_SILVER;
			total_rand_count++;
		}

		// if we have everything, give us a powerup. thats always helpful
		if (!total_rand_count)
			random_shrine_num = SHRINE_POWERUP;
		else
			random_shrine_num = possible_shrines[irand(0,total_rand_count)];
	}

	// Give us whatever we should have from this shrine.

	switch(random_shrine_num)
	{
		case SHRINE_HEAL:

			ShrineHealCore(other);
			gi.gamemsg_centerprintf(other, GM_CS_HEALTH);

			break;

		case SHRINE_ARMOR_SILVER:
			
			ShrineArmorSilverCore(other);
			gi.gamemsg_centerprintf(other, GM_CS_SILVER);

			break;

		case SHRINE_ARMOR_GOLD:

			ShrineArmorGoldCore(other);
			gi.gamemsg_centerprintf(other, GM_CS_GOLD);

			break;

		case SHRINE_MANA:

			shrine_mana_core(self,other);
			gi.gamemsg_centerprintf(other, GM_CS_MANA);

			break;

		case SHRINE_STAFF:

			ShrineStaffCore(other);
			gi.gamemsg_centerprintf(other, GM_CS_BLADE);

			break;

		case SHRINE_GHOST:

			shrine_ghost_core(self,other);
			gi.gamemsg_centerprintf(other, GM_CS_GHOST);

			break;

		case SHRINE_REFLECT:

			shrine_reflect_core(self,other);
			gi.gamemsg_centerprintf(other, GM_CS_REFLECT);
			
			break;

		case SHRINE_POWERUP:
		
			shrine_powerup_core(self,other);
			gi.gamemsg_centerprintf(other, GM_CS_POWERUP);
			
			break;

		case SHRINE_SPEED:
			shrine_speed_core(self,other);
			gi.gamemsg_centerprintf(other, GM_CS_SPEED);

			break;

		default:

			shrine_powerup_core(self,other);
			gi.gamemsg_centerprintf(other, GM_CS_POWERUP);
			
			break;
	}

	// If we are in death match, don't make us go through the shrine anim, just start the effect,
	// give us whatever, and leave it at that.

	if (deathmatch->value || (other->flags & FL_CHICKEN) || (other->client->playerinfo.flags & PLAYER_FLAG_WATER))
	{
		PlayerRandomShrineEffect(other, random_shrine_num);
	}
	else
	{
		// Tell us what sort of shrine we just hit.

		other->shrine_type = random_shrine_num;

		// Initialise the shrine animation.

		P_PlayerAnimSetLowerSeq(&other->client->playerinfo, ASEQ_SHRINE);

		// Make us invulnerable for a couple of seconds.

		other->client->shrine_framenum = level.time + INVUNERABILITY_TIME;
	}

	// Decide whether to delete this shrine or disable it for a while.

	UpdateShrineNode(self);
}

/*QUAKED shrine_random (.5 .3 .5) ? PERMANENT
*/

void SP_shrine_random_trigger(edict_t *ent)
{	
	ent->movetype = PHYSICSTYPE_NONE;
	ent->svflags |= SVF_NOCLIENT;
	ent->solid = SOLID_TRIGGER;
	ent->shrine_type = SHRINE_RANDOM;
	ent->classname = chaos_text;

	if(!deathmatch->value || (deathmatch->value && !((int)dmflags->value & DF_NO_SHRINE)))
		ent->touch = ShrineRandomTouch;

	gi.setmodel(ent, ent->model);
	gi.linkentity (ent);
}

