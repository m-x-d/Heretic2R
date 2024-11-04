// g_shrine.c
//
// Heretic II - Raven software
//

#include "FX.h"
#include "g_local.h"
#include "g_itemstats.h"
#include "random.h"
#include "vector.h"
#include "p_actions.h"
#include "p_anims.h"
#include "p_main.h"
#include "m_player.h"
#include "p_funcs.h"
#include "cl_strings.h"

// Set up those shrines that are randomly selectable.

char	delay_text[] = "shrine respawn delay";
char	chaos_text[] = "chaos shrine touch";
char	health_text[] = "health shrine touch";
char	mana_text[] = "mana shrine touch";
char	light_text[] = "light shrine touch";
char	lungs_text[] = "lungs shrine touch";
char	run_text[] = "run shrine touch";
char	staff_text[] = "staff shrine touch";
char	powerup_text[] = "powerup shrine touch";
char	ghost_text[] = "ghost shrine touch";
char	reflect_text[] = "reflect shrine touch";
char	armor_gold_text[] = "armor gold shrine touch";
char	armor_silver_text[] = "armor silver shrine touch";

void player_shrine_health_effect(edict_t *self);
void player_shrine_armor_silver_effect(edict_t *self);
void player_shrine_armor_gold_effect(edict_t *self);
void player_shrine_lungs_effect(edict_t *self);
void player_shrine_light_effect(edict_t *self);
void player_shrine_staff_effect(edict_t *self);
void player_shrine_mana_effect(edict_t *self);
void player_shrine_ghost_effect(edict_t *self);
void player_shrine_reflect_effect(edict_t *self);
void player_shrine_powerup_effect(edict_t *self);
void player_shrine_speed_effect(edict_t *self);

void shrine_armor_silver_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
void shrine_armor_gold_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
void shrine_random_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);

extern gitem_armor_t silver_armor_info;
extern gitem_armor_t gold_armor_info;

#define INVUN_TIME 2.0

// ************************************************************************************************
// PlayerKillShrineFX
// ------------------
// Remove all shrine effects associated with a player. Used when he's turned into a chicken.
// ************************************************************************************************

void PlayerKillShrineFX(edict_t *self)
{
	playerinfo_t *playerinfo;
	
	playerinfo=&self->client->playerinfo;
	
	assert(playerinfo);

	// --- Remove Reflection
	
	// Remove time on the timer for the reflectivity.

	self->client->playerinfo.reflect_timer = level.time - 1.0;

	// Turn off the relection at the client effect end through client flags that are passed down.

	self->s.renderfx &= ~RF_REFLECTION;

	// --- Remove Ghosting.

	// Remove time on the timer for the ghosting.

	self->client->playerinfo.ghost_timer = level.time - 1.0;

	// Turn off the ghosting at the client effect end through client flags that are passed down.

	self->s.renderfx &= ~RF_TRANS_GHOST;

	// --- Remove the light.

	// Remove time on the timer for the light.

	self->client->playerinfo.light_timer = level.time - 1.0;

	// Turn off the light at the client effect end through client flags that are passed down.

	self->s.effects &= ~EF_LIGHT_ENABLED;

	// turn off the run shrine should we need to

	self->s.effects &= ~EF_SPEED_ACTIVE;
	gi.RemoveEffects(&self->s, FX_FOOT_TRAIL);

	// Kill any lights that may already be out there for this player.

	gi.RemoveEffects(&self->s, FX_PLAYER_TORCH);

	// Kill lungs.

	self->client->playerinfo.lungs_timer = 0.0;

	// Remove Armor.

	self->client->playerinfo.pers.armor_count = 0;

	// Turn off the armor at the model level.

	playerinfo->pers.armortype = ARMOR_NONE;

	SetupPlayerinfo_effects(self);
	P_PlayerUpdateModelAttributes(&self->client->playerinfo);
	WritePlayerinfo_effects(self);

	// Remove Staff powerup.

	self->client->playerinfo.pers.stafflevel = STAFF_LEVEL_BASIC;

	// Remove Weapons powerup.

	self->client->playerinfo.powerup_timer = level.time - 1.0;

	// Kill any tomes that may already be out there for this player.

	gi.RemoveEffects(&self->s, FX_TOME_OF_POWER);

	// Turn off the tome at the client effect end through client flags that are passed down.

	self->s.effects &= ~EF_POWERUP_ENABLED;
}

// ************************************************************************************************
// PlayerRestartShrineFX
// ---------------------
// This is the routine that re-starts any client effects that need to be running. For instance,
// recovery of a saved game, where for example, the torch is active.
// ************************************************************************************************

void PlayerRestartShrineFX(edict_t *self)
{
	// If we have a light, turn it on.

	if (self->s.effects & EF_LIGHT_ENABLED)
	{
		// Kill any lights that may already be out there for this player.

		gi.RemoveEffects(&self->s, FX_PLAYER_TORCH);

		// Create the light and the tome of power.

		gi.CreateEffect(&self->s, FX_PLAYER_TORCH, CEF_OWNERS_ORIGIN, NULL, "");
	}

	// If we have a powerup, turn it on.

	if (self->s.effects & EF_POWERUP_ENABLED)
	{
		// Kill any lights that may already be out there for this player.

		gi.RemoveEffects(&self->s, FX_TOME_OF_POWER);

		// Create the light and the tome of power.
		
		gi.CreateEffect(&self->s, FX_TOME_OF_POWER, CEF_OWNERS_ORIGIN, NULL, "");
	}

	// If we have a powerup, turn it on.

	if (self->s.effects & EF_SPEED_ACTIVE)
	{
		// Kill any lights that may already be out there for this player.

		gi.RemoveEffects(&self->s, FX_FOOT_TRAIL);

		// Create the light and the tome of power.
		
		gi.CreateEffect(&self->s, FX_FOOT_TRAIL, CEF_OWNERS_ORIGIN, NULL, "");
	}

}

// ************************************************************************************************
// G_PlayerActionShrineEffect
// --------------------------
// ************************************************************************************************

void G_PlayerActionShrineEffect(playerinfo_t *playerinfo)
{
	edict_t *self;

	self=(edict_t *)playerinfo->self;

	switch(self->shrine_type)
	{
		case SHRINE_ARMOR_SILVER:
			player_shrine_armor_silver_effect(self);
			break;

		case SHRINE_ARMOR_GOLD:
			player_shrine_armor_gold_effect(self);
			break;

		case SHRINE_LIGHT:
			player_shrine_light_effect(self);
			break;

		case SHRINE_HEAL:
			player_shrine_health_effect(self);
			break;

		case SHRINE_STAFF:
			player_shrine_staff_effect(self);
			break;

		case SHRINE_LUNGS:
			player_shrine_lungs_effect(self);
			break;

		case SHRINE_GHOST:
			player_shrine_ghost_effect(self);
			break;

		case SHRINE_REFLECT:
			player_shrine_reflect_effect(self);
			break;

		case SHRINE_POWERUP:
			player_shrine_powerup_effect(self);
			break;

		case SHRINE_MANA:
			player_shrine_mana_effect(self);
			break;
		
		case SHRINE_SPEED:
			player_shrine_speed_effect(self);
			break;
		
		default:
			player_shrine_mana_effect(self);
			break;		
	}
}

// ************************************************************************************************
// PlayerRandomShrineEffect
// ------------------------
// Called from the random Shrine - which one do we want to do?
// ************************************************************************************************

void PlayerRandomShrineEffect(edict_t *self, int value)
{
	switch(value)
	{
		case SHRINE_ARMOR_SILVER:
			player_shrine_armor_silver_effect(self);
			break;

		case SHRINE_ARMOR_GOLD:
			player_shrine_armor_gold_effect(self);
			break;

		case SHRINE_LIGHT:
			player_shrine_light_effect(self);
			break;

		case SHRINE_HEAL:
			player_shrine_health_effect(self);
			break;

		case SHRINE_STAFF:
			player_shrine_staff_effect(self);
			break;

		case SHRINE_LUNGS:
			player_shrine_lungs_effect(self);
			break;

		case SHRINE_GHOST:
			player_shrine_ghost_effect(self);
			break;

		case SHRINE_REFLECT:
			player_shrine_reflect_effect(self);
			break;

		case SHRINE_POWERUP:
			player_shrine_powerup_effect(self);
			break;

		case SHRINE_MANA:
			player_shrine_mana_effect(self);
			break;

		case SHRINE_SPEED:
			player_shrine_speed_effect(self);
			break;

		default:
			player_shrine_powerup_effect(self);
			break;
	}
}

// ************************************************************************************************
// DelayThink
// ----------
// Wait till we can use this shrine again.
// ************************************************************************************************

void DelayThink(edict_t *self)
{
	edict_t		*dest;
	vec3_t		offset;
	vec3_t		offset2;

	// Handle changing shrine types in deathmatch.

	if (deathmatch->value && (self->oldtouch == shrine_armor_gold_touch))
	{
		// If we were gold in death match, we won't be again.

		self->owner->touch = shrine_armor_silver_touch;
	}	
	else if (deathmatch->value && (self->oldtouch == shrine_armor_silver_touch) && !(irand(0,8)))
	{
		// 1 in 9 chance in death match an armor shrine turns gold.

		self->owner->touch = shrine_armor_gold_touch;
	}
	else
	{
		// Restore the touch pad.

		self->owner->touch = self->oldtouch;
	}

	// Make the ball appear in the middle.

	// Setup in playerinfo the destination entity of the teleport.

	dest = G_Find (NULL, FOFS(targetname), self->owner->target);

	if (!dest)
	{
#ifdef _DEVEL
		gi.dprintf ("Shrine Trigger couldn't find shrine model\n");
#endif
		G_SetToFree (self);
		return;
	}

	if (self->owner->touch == shrine_armor_gold_touch)
		dest->style = 7;
	else
	if (self->owner->touch == shrine_armor_silver_touch)
		dest->style = 6;
		
	VectorScale(dest->s.angles, ANGLE_TO_RAD, offset);
	DirFromAngles(offset, offset2);
	dest->PersistantCFX=gi.CreatePersistantEffect(&dest->s,
												  FX_SHRINE_BALL,
												  CEF_BROADCAST,
												  dest->s.origin, 
												  "db",
												  offset2,
												  (byte)(dest->style-1));

	G_SetToFree (self);
}

// ************************************************************************************************
// deal_with_shrine_node
// ---------------------
// Either kill or set this shrine node to unuseable for a while.
// ************************************************************************************************

void deal_with_shrine_node(edict_t *self)
{
	edict_t	*delay,*dest;
	vec3_t	offset,offset2;
	int		time;
	float	clients;

	// Set up a delay so we can't use this shrine for a while.

	if (deathmatch->value || (self->spawnflags & 1))
	{
		delay = G_Spawn ();
		delay->svflags |= SVF_NOCLIENT;
		delay->movetype = PHYSICSTYPE_NONE;
		delay->solid = SOLID_NOT;
		delay->think = DelayThink;
		delay->owner = self;
		delay->classname = delay_text;
		if (deathmatch->value)
		// The equation for respawn:  
		//		--The respawn times should be normal for 8 players.
		//		--For 32 players the respawn should be halved
		//		--For 2 players the respawn should be doubled.
		{
/*
			time = SHRINE_DELAY * sqrt((float)game.num_clients/8.0);		// This makes it a nice curve.  Clever, no?
			// Lemme see here:  sqrt(2/8) = sqrt(1/4) = 1/2
			//					sqrt(8/8) = sqrt(1) = 1
			//					sqrt(32/8) = sqrt(4) = 2
*/
			clients=(float)game.num_clients;
			if (clients<2.0)
				clients=2.0;
			time = SHRINE_DELAY * sqrt(2.0/clients);		// Spawn more frequently when more players.
			// Lemme see here:  sqrt(2/2) = sqrt(1) = 1
			//					sqrt(2/8) = sqrt(1/4) = 1/2
			//					sqrt(2/32) = sqrt(1/16) = 1/4
		}
		else
		{
			time = SHRINE_DELAY;
		}

		// sanity check
		if (time < 5)
			time = 5;

		delay->nextthink = level.time + time;
		delay->oldtouch = self->touch;
		gi.linkentity (delay);
	}

	// Turn off the touch for this shrine.

	self->touch = NULL;

	// Setup in playerinfo, the destination entity of the teleport.

	dest = G_Find (NULL, FOFS(targetname), self->target);

	if (!dest)
	{
#ifdef _DEVEL
		gi.dprintf ("Shrine Trigger couldn't find shrine model\n");
#endif
		return;
	}

	// But kill the shrine ball thats out there for this shrine.

	gi.RemoveEffects(&dest->s, FX_SHRINE_BALL);

	// Kill the glowing ball in the middle.
	if (dest->PersistantCFX)
	{
		gi.RemovePersistantEffect(dest->PersistantCFX, REMOVE_SHRINE);
		dest->PersistantCFX = 0;
	}

	// Make the shrine ball explode.
	VectorScale(dest->s.angles, ANGLE_TO_RAD, offset);
	DirFromAngles(offset, offset2);
	gi.CreateEffect(&dest->s,FX_SHRINE_BALL_EXPLODE,CEF_OWNERS_ORIGIN,dest->s.origin,"db",offset2,(byte)(dest->style-1));
}

void shrine_restore_player(edict_t *other)
{
	// Stop us from being on fire.

	if(other->fire_damage_time>level.time)
	{
		other->fire_damage_time = 0;

		// Turn off CFX too.
		other->s.effects &= ~EF_ON_FIRE;
	}
	
	// Stop bleeding.

	other->client->playerinfo.flags &= ~PLAYER_FLAG_BLEED;

	// Restore limbs!
	// FIXME: maybe do some cool temp effect on these nodes to show they respawned?

	ResetPlayerBaseNodes(other);

	other->client->playerinfo.flags &= ~PLAYER_FLAG_NO_LARM;
	other->client->playerinfo.flags &= ~PLAYER_FLAG_NO_RARM;
}

// ************************************************************************************************
// Health Shrine
// ************************************************************************************************

// Fire off the health shrine effect.

void player_shrine_health_effect(edict_t *self)
{
	// Start up the shrine heal effect.

	gi.CreateEffect(&self->s, FX_SHRINE_HEALTH, CEF_OWNERS_ORIGIN, NULL, "");

	// Do the SHRINE sound.

	gi.sound(self,CHAN_ITEM,gi.soundindex("items/shrine4.wav"),1,ATTN_NORM,0);
}

void shrine_heal_core(edict_t *self,edict_t *other)
{
	if (other->deadflag != DEAD_NO)
		return;

	// If we are a chicken, lets make us a player again.  Don't give him anything else.
	if (other->flags & FL_CHICKEN)
	{
		other->morph_timer = level.time - 0.1;
		return;
	}

	// Give us some health.

	if (other->health < (SHRINE_MAX_HEALTH - SHRINE_HEALTH))
		other->health += SHRINE_HEALTH;
	else if (other->health < SHRINE_MAX_HEALTH)
		other->health = SHRINE_MAX_HEALTH;

	// restore dismemberment, and stop us being on fire
	shrine_restore_player(other);
}

// ************************************************************************************************
// shrine_heal_touch
// -----------------
// Fire off a heal effect and give us some health.
// ************************************************************************************************

void shrine_heal_touch	(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	// If we aren't a player, forget it.

	if (!other->client)
		return;

	shrine_heal_core(self,other);

	gi.gamemsg_centerprintf(other, GM_S_HEALTH);

	// If we are in death match, don't make us go through the shrine anim, just start the effect,
	// give us whatever, and leave it at that.

	if (deathmatch->value || (other->flags & FL_CHICKEN) || (other->client->playerinfo.flags & PLAYER_FLAG_WATER))
	{
		player_shrine_health_effect(other);
	}
	else
	{
		// Tell us what sort of shrine we just hit.

		other->shrine_type = SHRINE_HEAL;

		// Initialise the shrine animation.

		P_PlayerAnimSetLowerSeq(&other->client->playerinfo,ASEQ_SHRINE);

		// Make us invunerable for a couple of seconds.

		other->client->shrine_framenum = level.time + INVUN_TIME;
	}

	// Decide whether to delete this shrine or disable it for a while.
	
	deal_with_shrine_node(self);
}

/*QUAKED shrine_heal (.5 .3 .5) ? PERMANENT
*/

void shrine_heal(edict_t *ent)
{
	ent->movetype = PHYSICSTYPE_NONE;
	ent->svflags |= SVF_NOCLIENT;
	ent->solid = SOLID_TRIGGER;
	ent->shrine_type = SHRINE_HEAL;
	ent->classname = health_text;

	if(!deathmatch->value || (deathmatch->value && !((int)dmflags->value & DF_NO_SHRINE)))
		ent->touch = shrine_heal_touch;

	if(deathmatch->value && ((int)dmflags->value & DF_SHRINE_CHAOS) && !((int)dmflags->value & DF_NO_SHRINE))
	{
		ent->shrine_type = SHRINE_RANDOM;
		ent->touch = shrine_random_touch;
	}

	gi.setmodel(ent, ent->model);
	gi.linkentity(ent);
}

// ************************************************************************************************
// Silver armor shrine.
// ************************************************************************************************

// Fire off the armor shrine effect.

void player_shrine_armor_silver_effect(edict_t *self)
{
	// Start up the shrine armor effect.

	gi.CreateEffect(&self->s, FX_SHRINE_ARMOR, CEF_OWNERS_ORIGIN, NULL, "");

	// Do the SHRINE sound.

	gi.sound(self,CHAN_ITEM,gi.soundindex("items/shrine2.wav"),1,ATTN_NORM,0);
}

void shrine_armor_silver_core(edict_t *self,edict_t *other)
{
	if (other->deadflag != DEAD_NO)
		return;

	// If we are a chicken, lets make us a player again.  Don't give him anything else.
	if (other->flags & FL_CHICKEN)
	{
		other->morph_timer = level.time - 0.1;
		return;
	}

	// Add armor to player.
	if ((other->client->playerinfo.pers.armortype == ARMOR_TYPE_GOLD) &&
		(other->client->playerinfo.pers.armor_count >= gold_armor_info.max_armor / 2))
		other->client->playerinfo.pers.armor_count = gold_armor_info.max_armor;
	else
	{
		other->client->playerinfo.pers.armortype = ARMOR_TYPE_SILVER;
		other->client->playerinfo.pers.armor_count = silver_armor_info.max_armor;
	}

	SetupPlayerinfo_effects(other);
	P_PlayerUpdateModelAttributes(&other->client->playerinfo);
	WritePlayerinfo_effects(other);


	// restore dismemberment, and stop us being on fire
	shrine_restore_player(other);
}

// Fire off an effect and give us some armor.

void shrine_armor_silver_touch	(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	// If we aren't a player, forget it.

	if (!other->client)
		return;

	shrine_armor_silver_core(self,other);

	gi.gamemsg_centerprintf(other, GM_S_SILVER);

	// If we are in death match, don't make us go through the shrine anim, just start the effect,
	// give us whatever, and leave it at that.

	if (deathmatch->value || (other->flags & FL_CHICKEN) || (other->client->playerinfo.flags & PLAYER_FLAG_WATER))
	{
		if (other->client->playerinfo.pers.armortype == ARMOR_TYPE_SILVER)
			player_shrine_armor_silver_effect(other);
		else
			player_shrine_armor_gold_effect(other);
	}
	else
	{
		// Tell us what sort of shrine we just hit.

		other->shrine_type = SHRINE_ARMOR_SILVER;

		// Initialise the shrine animation.

		P_PlayerAnimSetLowerSeq(&other->client->playerinfo, ASEQ_SHRINE);

		// Make us invunerable for a couple of seconds.

		other->client->shrine_framenum = level.time + INVUN_TIME;
	}

	// Decide whether to delete this shrine or disable it for a while.

	deal_with_shrine_node(self);
}

/*QUAKED shrine_armor (.5 .3 .5) ? PERMANENT
*/

void shrine_armor (edict_t *ent)
{
	ent->movetype = PHYSICSTYPE_NONE;
	ent->svflags |= SVF_NOCLIENT;
	ent->solid = SOLID_TRIGGER;
	ent->shrine_type = SHRINE_ARMOR_SILVER;
	ent->classname = armor_silver_text;

	if (!deathmatch->value || (deathmatch->value && !((int)dmflags->value & DF_NO_SHRINE)))
		ent->touch = shrine_armor_silver_touch;

	if(deathmatch->value && ((int)dmflags->value & DF_SHRINE_CHAOS) && !((int)dmflags->value & DF_NO_SHRINE))
	{
		ent->shrine_type = SHRINE_RANDOM;
		ent->touch = shrine_random_touch;
	}

	gi.setmodel(ent, ent->model);
	gi.linkentity (ent);
}

// ************************************************************************************************
// Armor shrine - gold.
// ************************************************************************************************

// Fire off the gold armor shrine effect.

void player_shrine_armor_gold_effect(edict_t *self)
{
	// Start up the shrine armor effect.

	gi.CreateEffect(&self->s, FX_SHRINE_ARMOR, CEF_OWNERS_ORIGIN|CEF_FLAG6, NULL, "");

	// Do the SHRINE sound.

	gi.sound(self,CHAN_ITEM,gi.soundindex("items/shrine2.wav"),1,ATTN_NORM,0);
}

void shrine_armor_gold_core(edict_t *self,edict_t *other)
{
	if (other->deadflag != DEAD_NO)
		return;

	// If we are a chicken, lets make us a player again.  Don't give him anything else.
	if (other->flags & FL_CHICKEN)
	{
		other->morph_timer = level.time - 0.1;
		return;
	}

	// Add gold armor to player.

	other->client->playerinfo.pers.armortype = ARMOR_TYPE_GOLD;
	other->client->playerinfo.pers.armor_count = gold_armor_info.max_armor;

	SetupPlayerinfo_effects(other);
	P_PlayerUpdateModelAttributes(&other->client->playerinfo);
	WritePlayerinfo_effects(other);

	// restore dismemberment, and stop us being on fire
	shrine_restore_player(other);

}

// Fire off an effect and give us some armor.

void shrine_armor_gold_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	//  If we aren't a player, forget it.

	if (!other->client)
		return;

	shrine_armor_gold_core(self,other);

	gi.gamemsg_centerprintf(other, GM_S_GOLD);

	// If we are in death match, don't make us go through the shrine anim, just start the effect,
	// give us whatever, and leave it at that.

	if (deathmatch->value || (other->flags & FL_CHICKEN) || (other->client->playerinfo.flags & PLAYER_FLAG_WATER))
	{
		player_shrine_armor_gold_effect(other);
	}
	else
	{
		// Tell us what sort of shrine we just hit.

		other->shrine_type = SHRINE_ARMOR_GOLD;

		// Initialise the shrine animation.

		P_PlayerAnimSetLowerSeq(&other->client->playerinfo, ASEQ_SHRINE);

		// Make us invunerable for a couple of seconds.

		other->client->shrine_framenum = level.time + INVUN_TIME;
	}

	// Decide whether to delete this shrine or disable it for a while.

	deal_with_shrine_node(self);
}

/*QUAKED shrine_armor_gold (.5 .3 .5) ? PERMANENT
*/

void shrine_armor_gold (edict_t *ent)
{	
	ent->movetype = PHYSICSTYPE_NONE;
	ent->svflags |= SVF_NOCLIENT;
	ent->solid = SOLID_TRIGGER;
	ent->shrine_type = SHRINE_ARMOR_GOLD;
	ent->classname = armor_gold_text;

	// No touch if flags say so.

	if(!deathmatch->value || (deathmatch->value && !((int)dmflags->value & DF_NO_SHRINE)))
		ent->touch = shrine_armor_gold_touch;

	if(deathmatch->value && ((int)dmflags->value & DF_SHRINE_CHAOS) && !((int)dmflags->value & DF_NO_SHRINE))
	{
		ent->shrine_type = SHRINE_RANDOM;
		ent->touch = shrine_random_touch;
	}

	gi.setmodel(ent, ent->model);
	gi.linkentity (ent);
}

// ************************************************************************************************
// Staff powerup shrine.
// ************************************************************************************************

// Fire off the staff shrine effect.

void player_shrine_staff_effect(edict_t *self)
{
	int	flags = CEF_OWNERS_ORIGIN;
	// Start up the shrine staff effect.

	if (self->client->playerinfo.pers.stafflevel == STAFF_LEVEL_POWER2)
	{
		flags |= CEF_FLAG6;
		gi.sound(self,CHAN_ITEM,gi.soundindex("weapons/FirewallPowerCast.wav"),1,ATTN_NORM,0);
	}
	else
		gi.sound(self,CHAN_ITEM,gi.soundindex("items/shrine7.wav"),1,ATTN_NORM,0);

	gi.CreateEffect(&self->s, FX_SHRINE_STAFF, flags, NULL, "");

	// Do the SHRINE sound.

}

void shrine_staff_core(edict_t *self,edict_t *other)
{
	if (other->deadflag != DEAD_NO)
		return;

	// If we are a chicken, lets make us a player again.  Don't give him anything else.
	if (other->flags & FL_CHICKEN)
	{
		other->morph_timer = level.time - 0.1;
		return;
	}

	// Add onto his staff.

	if (other->client->playerinfo.pers.stafflevel < STAFF_LEVEL_MAX-1)
	{
		other->client->playerinfo.pers.stafflevel++;

		SetupPlayerinfo_effects(other);
		P_PlayerUpdateModelAttributes(&other->client->playerinfo);
		WritePlayerinfo_effects(other);
	}

	// restore dismemberment, and stop us being on fire
	shrine_restore_player(other);
}

// Fire off an effect and give us a staff powerup.

void shrine_staff_touch	(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	// If we aren't a player, forget it.

	if (!other->client)
		return;

	shrine_staff_core(self,other);

	gi.gamemsg_centerprintf(other, GM_S_BLADE);

	// If we are in death match, don't make us go through the shrine anim, just start the effect,
	// give us whatever, and leave it at that.

	if (deathmatch->value || (other->flags & FL_CHICKEN) || (other->client->playerinfo.flags & PLAYER_FLAG_WATER))
	{
		player_shrine_staff_effect(other);
	}
	else
	{
		// Tell us what sort of shrine we just hit.

		other->shrine_type = SHRINE_STAFF;

		// Initialise the shrine animation.

		P_PlayerAnimSetLowerSeq(&other->client->playerinfo, ASEQ_SHRINE);

		// Make us invunerable for a couple of seconds.

		other->client->shrine_framenum = level.time + INVUN_TIME;
	}

	// Decide whether to delete this shrine or disable it for a while.

	deal_with_shrine_node(self);
}

/*QUAKED shrine_staff (.5 .3 .5) ? PERMANENT
*/

void shrine_staff (edict_t *ent)
{	
	ent->movetype = PHYSICSTYPE_NONE;
	ent->svflags |= SVF_NOCLIENT;
	ent->solid = SOLID_TRIGGER;
	ent->shrine_type = SHRINE_STAFF;
	ent->classname = staff_text;

	if(!deathmatch->value || (deathmatch->value && !((int)dmflags->value & DF_NO_SHRINE)))
		ent->touch = shrine_staff_touch;

	if(deathmatch->value && ((int)dmflags->value & DF_SHRINE_CHAOS) && !((int)dmflags->value & DF_NO_SHRINE))
	{
		ent->shrine_type = SHRINE_RANDOM;
		ent->touch = shrine_random_touch;
	}

	gi.setmodel(ent, ent->model);
	gi.linkentity (ent);
}

// ************************************************************************************************
// Lungs Shrine
// ************************************************************************************************

// Fire off the lungs shrine effect.

void player_shrine_lungs_effect(edict_t *self)
{
	// Start up the shrine lung effect.

	gi.CreateEffect(&self->s, FX_SHRINE_LUNGS, CEF_OWNERS_ORIGIN, NULL, "");

	// Do the SHRINE sound.

	gi.sound(self,CHAN_ITEM,gi.soundindex("items/shrine9.wav"),1,ATTN_NORM,0);
}

void shrine_lung_core(edict_t *self, edict_t *other)
{
	if (other->deadflag != DEAD_NO)
		return;

	// If we are a chicken, lets make us a player again.  Don't give him anything else.
	if (other->flags & FL_CHICKEN)
	{
		other->morph_timer = level.time - 0.1;
		return;
	}

	// Add some time in on the timer for the lungs.

	other->client->playerinfo.lungs_timer = LUNGS_DURATION;

	// restore dismemberment, and stop us being on fire
	shrine_restore_player(other);

}

// Fire off an effect and give us lung power.

void shrine_lung_touch	(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	// If we aren't a player, forget it.

	if (!other->client)
		return;

	shrine_lung_core(self,other);

	gi.gamemsg_centerprintf(other, GM_S_LUNGS);

	// If we are in death match, don't make us go through the shrine anim, just start the effect,
	// give us whatever, and leave it at that.

	if (deathmatch->value || (other->flags & FL_CHICKEN) || (other->client->playerinfo.flags & PLAYER_FLAG_WATER))
	{
		player_shrine_lungs_effect(other);
	}
	else
	{
		// Tell us what sort of shrine we just hit.

		other->shrine_type = SHRINE_LUNGS;

		// Initialise the shrine animation.

		P_PlayerAnimSetLowerSeq(&other->client->playerinfo, ASEQ_SHRINE);

		// Make us invulnerable for a couple of seconds.

		other->client->shrine_framenum = level.time + INVUN_TIME;
	}

	// Decide whether to delete this shrine or disable it for a while.

	deal_with_shrine_node(self);
}

/*QUAKED shrine_lung (.5 .3 .5) ? PERMANENT
*/
void shrine_lung (edict_t *ent)
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
		ent->touch = shrine_random_touch;
	}

	gi.setmodel(ent, ent->model);
	gi.linkentity (ent);
}

// ************************************************************************************************
// Light Shrine
// ************************************************************************************************

// Fire off the shrine light effect .

void player_shrine_light_effect(edict_t *self)
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
	shrine_restore_player(other);

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
		player_shrine_light_effect(other);
	}
	else
	{
		// Tell us what sort of shrine we just hit.

		other->shrine_type = SHRINE_LIGHT;

		// Initialise the shrine animation.

		P_PlayerAnimSetLowerSeq(&other->client->playerinfo, ASEQ_SHRINE);

		// Make us invunerable for a couple of seconds.

		other->client->shrine_framenum = level.time + INVUN_TIME;
	}

	// Decide whether to delete this shrine or disable it for a while.

	deal_with_shrine_node(self);
}

/*QUAKED shrine_light (.5 .3 .5) ? PERMANENT
*/

void shrine_light (edict_t *ent)
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
		ent->touch = shrine_random_touch;
	}

	gi.setmodel(ent, ent->model);
	gi.linkentity (ent);
}

// ************************************************************************************************
// Mana Shrine
// ************************************************************************************************

// Fire off the shrine mana effect.

void player_shrine_mana_effect(edict_t *self)
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
	shrine_restore_player(other);

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
		player_shrine_mana_effect(other);
	}
	else
	{
		// Tell us what sort of shrine we just hit.

		other->shrine_type = SHRINE_MANA;

		// Initialise the shrine animation.

		P_PlayerAnimSetLowerSeq(&other->client->playerinfo, ASEQ_SHRINE);

		// Make us invunerable for a couple of seconds.

		other->client->shrine_framenum = level.time + INVUN_TIME;
	}

	// Decide whether to delete this shrine or disable it for a while.

	deal_with_shrine_node(self);
}

/*QUAKED shrine_mana (.5 .3 .5) ? PERMANENT
*/

void shrine_mana (edict_t *ent)
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
		ent->touch = shrine_random_touch;
	}

	gi.setmodel(ent, ent->model);
	gi.linkentity (ent);
}

// ************************************************************************************************
// Ghost (invisibilty) shrine.
// ************************************************************************************************

// Fire off the ghost shrine effect.

void player_shrine_ghost_effect(edict_t *self)
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
	shrine_restore_player(other);

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
		player_shrine_ghost_effect(other);
	}
	else
	{
		// Tell us what sort of shrine we just hit.

		other->shrine_type = SHRINE_GHOST;

		// Initialise the shrine animation.

		P_PlayerAnimSetLowerSeq(&other->client->playerinfo, ASEQ_SHRINE);

		// Make us invulnerable for a couple of seconds.

		other->client->shrine_framenum = level.time + INVUN_TIME;
	}

	// Decide whether to delete this shrine or disable it for a while.
	
	deal_with_shrine_node(self);
}

/*QUAKED shrine_ghost (.5 .3 .5) ? PERMANENT
*/

void shrine_ghost (edict_t *ent)
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
		ent->touch = shrine_random_touch;
	}

	gi.setmodel(ent, ent->model);
	gi.linkentity (ent);
}

// ************************************************************************************************
// Spell reflecting shrine.
// ************************************************************************************************

// Fire off the reflect shrine effect.

void player_shrine_reflect_effect(edict_t *self)
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
	shrine_restore_player(other);

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
		player_shrine_reflect_effect(other);
	}
	else
	{
		// Tell us what sort of shrine we just hit.

		other->shrine_type = SHRINE_REFLECT;

		// Initialise the shrine animation.

		P_PlayerAnimSetLowerSeq(&other->client->playerinfo, ASEQ_SHRINE);

		// Make us invunerable for a couple of seconds.

		other->client->shrine_framenum = level.time + INVUN_TIME;
	}

	// Decide whether to delete this shrine or disable it for a while.

	deal_with_shrine_node(self);
}

/*QUAKED shrine_reflect (.5 .3 .5) ? PERMANENT
*/

void shrine_reflect (edict_t *ent)
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
		ent->touch = shrine_random_touch;
	}

	gi.setmodel(ent, ent->model);
	gi.linkentity (ent);
}

// ************************************************************************************************
// Spell powerup Shrine
// ************************************************************************************************

// Fire off the powerup shrine effect.

void player_shrine_powerup_effect(edict_t *self)
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
	shrine_restore_player(other);

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
		player_shrine_powerup_effect(other);
	}
	else
	{
		// Tell us what sort of shrine we just hit.

		other->shrine_type = SHRINE_POWERUP;

		// Initialise the shrine animation.

		P_PlayerAnimSetLowerSeq(&other->client->playerinfo, ASEQ_SHRINE);

		// Make us invunerable for a couple of seconds.

		other->client->shrine_framenum = level.time + INVUN_TIME;
	}

	// Decide whether to delete this shrine or disable it for a while.

	deal_with_shrine_node(self);
}

/*QUAKED shrine_powerup (.5 .3 .5) ? PERMANENT
*/

void shrine_powerup (edict_t *ent)
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
		ent->touch = shrine_random_touch;
	}

	gi.setmodel(ent, ent->model);
	gi.linkentity (ent);
}

// ************************************************************************************************
// Speed Shrine
// ************************************************************************************************

// Fire off the powerup shrine effect.

void player_shrine_speed_effect(edict_t *self)
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
	shrine_restore_player(other);

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
		player_shrine_speed_effect(other);
	}
	else
	{
		// Tell us what sort of shrine we just hit.

		other->shrine_type = SHRINE_SPEED;

		// Initialise the shrine animation.

		P_PlayerAnimSetLowerSeq(&other->client->playerinfo, ASEQ_SHRINE);

		// Make us invunerable for a couple of seconds.

		other->client->shrine_framenum = level.time + INVUN_TIME;
	}

	// Decide whether to delete this shrine or disable it for a while.

	deal_with_shrine_node(self);
}

/*QUAKED shrine_speed (.5 .3 .5) ? PERMANENT
*/
void shrine_speed (edict_t *ent)
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
		ent->touch = shrine_random_touch;
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
void shrine_random_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
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

			shrine_heal_core(self,other);
			gi.gamemsg_centerprintf(other, GM_CS_HEALTH);

			break;

		case SHRINE_ARMOR_SILVER:
			
			shrine_armor_silver_core(self,other);
			gi.gamemsg_centerprintf(other, GM_CS_SILVER);

			break;

		case SHRINE_ARMOR_GOLD:

			shrine_armor_gold_core(self,other);			
			gi.gamemsg_centerprintf(other, GM_CS_GOLD);

			break;

		case SHRINE_MANA:

			shrine_mana_core(self,other);
			gi.gamemsg_centerprintf(other, GM_CS_MANA);

			break;

		case SHRINE_STAFF:

			shrine_staff_core(self,other);
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

		other->client->shrine_framenum = level.time + INVUN_TIME;
	}

	// Decide whether to delete this shrine or disable it for a while.

	deal_with_shrine_node(self);
}

/*QUAKED shrine_random (.5 .3 .5) ? PERMANENT
*/

void shrine_random(edict_t *ent)
{	
	ent->movetype = PHYSICSTYPE_NONE;
	ent->svflags |= SVF_NOCLIENT;
	ent->solid = SOLID_TRIGGER;
	ent->shrine_type = SHRINE_RANDOM;
	ent->classname = chaos_text;

	if(!deathmatch->value || (deathmatch->value && !((int)dmflags->value & DF_NO_SHRINE)))
		ent->touch = shrine_random_touch;

	gi.setmodel(ent, ent->model);
	gi.linkentity (ent);
}

