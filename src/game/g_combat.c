//
// g_combat.c
//
// Copyright 1998 Raven Software
//

#include "g_combat.h" //mxd
#include "g_breakable.h" //mxd
#include "g_cmds.h" //mxd
#include "g_debris.h" //mxd
#include "g_HitLocation.h"
#include "g_items.h" //mxd
#include "g_itemstats.h"
#include "g_monster.h" //mxd
#include "g_playstats.h"
#include "g_rope.h" //mxd
#include "m_stats.h"
#include "mg_guide.h" //mxd
#include "p_main.h"
#include "p_client.h" //mxd
#include "p_view.h" //mxd
#include "FX.h"
#include "mg_ai.h" //mxd
#include "Random.h"
#include "Utilities.h" //mxd
#include "Vector.h"

gitem_armor_t silver_armor_info	= { MAX_SILVER_ARMOR, SILVER_HIT_MULT, SILVER_SPELL_MULT };
gitem_armor_t gold_armor_info	= { MAX_GOLD_ARMOR, GOLD_HIT_MULT, GOLD_SPELL_MULT };

// Returns true if the inflictor can directly damage the target. The origin point of the damage doesn't have to be the same as the inflictor's.
static qboolean CanDamageFromLoc(const edict_t* target, const edict_t* inflictor, const vec3_t origin)
{
	// bmodels need special checking because their origin is 0 0 0.
	if (target->movetype == PHYSICSTYPE_PUSH || target->classID == CID_BBRUSH)
	{
		vec3_t dest;
		VectorAdd(target->absmin, target->absmax, dest);
		VectorScale(dest, 0.5f, dest);

		trace_t trace;
		gi.trace(origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID, &trace);

		return (trace.fraction == 1.0f || trace.ent == target);
	}

	// Try a basic trace straight to the origin. This takes care of 99% of the tests.
	trace_t trace;
	gi.trace(origin, vec3_origin, vec3_origin, target->s.origin, inflictor, MASK_SOLID, &trace);
	if (trace.fraction == 1.0f)
		return true;

	// Well, a trace from origin to origin didn't work, so try tracing to the edges of the victim.

	// If there are no edges, let's skip the rest of these checks..
	if (Vec3IsZero(target->mins) || Vec3IsZero(target->maxs))
		return false;

	// First figure out which two sides of the victim to check.
	vec3_t diff;
	VectorSubtract(origin, target->s.origin, diff);

	// Check XY axis with lesser difference.
	const int axis = ((fabsf(diff[0]) < fabsf(diff[1])) ? 0 : 1); //mxd

	// Check opposite edges.
	for (int i = 0; i < 2; i++)
	{
		vec3_t dest;
		VectorCopy(target->s.origin, dest);
		dest[axis] += (i == 0 ? target->mins[axis] : target->maxs[axis]);

		gi.trace(origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID, &trace);
		if (trace.fraction > 0.99f)
			return true;
	}

	// Since the side checks didn't work, check the top and bottom.
	for (int i = 0; i < 2; i++)
	{
		vec3_t dest;
		VectorCopy(target->s.origin, dest);
		dest[2] += (i == 0 ? target->mins[2] : target->maxs[2]);

		gi.trace(origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID, &trace);
		if (trace.fraction > 0.99f)
			return true;
	}

	// None of the traces were successful, so no good.
	return false;
}

// Returns true if the inflictor can directly damage the target. Used for explosions and melee attacks.
qboolean CanDamage(const edict_t* target, const edict_t* inflictor)
{
	return CanDamageFromLoc(target, inflictor, inflictor->s.origin); //mxd. Avoid code duplication.
}

static void SpawnReward(const edict_t* self, const edict_t* attacker)
{
	// Assassins always give you something.
	if (self->classID != CID_ASSASSIN)
	{
		// Randomly refuse to give them anything.
		if (COOP)
		{
			if (irand(0, (MAXCLIENTS + 1) / 2) == 0)
				return;
		}
		else if (SKILL < 3)
		{
			// Easy: 1 in 2 chance; Normal: 1 in 4 chance; Hard: 1 in 6 chance.
			if (irand(0, ITEM_REWARD_CHANCE + SKILL * 2) > 0)
				return;
		}
		else
		{
			// No rewards in skills at or above 3.0.
			return;
		}
	}

	// Only intelligent monsters produce items, not creatures (and not Ogles).
	if (self->classID == CID_RAT || self->classID == CID_HARPY || self->classID == CID_GKROKON ||
		self->classID == CID_GORGON || self->classID == CID_FISH || self->classID == CID_OGLE)
		return;

	// Bosses don't spawn a reward either.
	if (self->svflags & SVF_BOSS)
		return;

	// Check the health amount on the attacker.
	const float health_chance = (attacker->health < attacker->max_health) ? ((float)attacker->health / (float)attacker->max_health) : 9999.0f;

	// Check the offensive mana amount on the attacker.
	const gitem_t* lookup = P_FindItemByClassname("item_mana_offensive_half");
	int index = ITEM_INDEX(lookup);
	const int off_max = attacker->client->playerinfo.pers.max_offmana;
	const int off_amount = attacker->client->playerinfo.pers.inventory.Items[index];

	const float off_chance = (off_amount < off_max) ? ((float)off_amount / (float)off_max) : 9999.0f;

	// Check the defensive mana amount on the attacker.
	lookup = P_FindItemByClassname("item_mana_defensive_half");
	index = ITEM_INDEX(lookup);
	const int def_max = attacker->client->playerinfo.pers.max_defmana;
	const int def_amount = attacker->client->playerinfo.pers.inventory.Items[index];

	const float def_chance = (def_amount < def_max) ? ((float)def_amount / (float)def_max) : 9999.0f;

	// We don't need anything.
	if (health_chance == 9999.0f && off_chance == 9999.0f && def_chance == 9999.0f)
		return;

	// Determine what they get.
	char* item_name;
	if (health_chance < off_chance && health_chance < def_chance)
	{
		item_name = "item_health_half";
	}
	else if (off_chance < health_chance && off_chance < def_chance)
	{
		item_name = "item_mana_offensive_half";
	}
	else if (def_chance < health_chance && def_chance < off_chance)
	{
		item_name = "item_mana_defensive_half";
	}
	else
	{
		const int chance = irand(0, 2);

		if (chance == 0)
			item_name = "item_health_half";
		else if (chance == 1)
			item_name = "item_mana_offensive_half";
		else
			item_name = "item_mana_defensive_half";
	}

	// We know what we want to give them, so create it!
	gitem_t* item = P_FindItemByClassname(item_name);
	edict_t* ed = G_Spawn();

	ed->movetype = PHYSICSTYPE_STEP;
	VectorCopy(self->s.origin, ed->s.origin);
	SpawnItem(ed, item);

	// Make the effect.
	gi.CreateEffect(NULL, FX_PICKUP, 0, ed->s.origin, NULL);
}

void Killed(edict_t* target, edict_t* inflictor, edict_t* attacker, const int damage, vec3_t point, const int mod)
{
	if (target->classID == CID_MORK)
	{
		target->die(target, inflictor, attacker, damage, vec3_origin);
		return;
	}

	// Clear special enemy attack stuff.
	//FIXME? Make attacking monster look for a new target? Or search for all monsters with 'target' as enemy and find new target for them?
	if (attacker->svflags & SVF_MONSTER)
		attacker->monsterinfo.aiflags &= ~AI_STRAIGHT_TO_ENEMY;

	if (target->classID != CID_BBRUSH)
		target->enemy = attacker;

	if ((target->svflags & SVF_MONSTER) && target->dead_state != DEAD_DEAD)
	{
		// What about if off ledge or on steep slope - slide off?
		M_GetSlopePitchRoll(target, NULL);

		target->dead_size = fabsf(target->maxs[2] - target->mins[2]) * 0.5f;
		MG_PostDeathThink(target);

		if (!(target->svflags & SVF_WAIT_NOTSOLID))
			target->svflags |= SVF_DEADMONSTER; // Now treat as a different content type.

		target->enemy = attacker;

		// Spawn an ent that will alert other monsters to my enemy's presence for 7 seconds.
		AlertMonsters(target, attacker, 7.0f, true);

		// Spawn a reward for the kill.
		if (attacker->client != NULL)
			SpawnReward(target, attacker);
	}

	if (target->movetype == PHYSICSTYPE_PUSH || target->movetype == PHYSICSTYPE_STOP || target->movetype == PHYSICSTYPE_NONE)
	{
		// Doors, triggers, breakable brushes, etc. die with their own KillBrush() routine.
		if (target->classID == CID_BBRUSH)
			KillBrush(target, inflictor, attacker, damage);
		else if ((target->classID == CID_NONE || classStatics[target->classID].msgReceivers[MSG_PAIN] == NULL) && target->die != NULL)
			target->die(target, inflictor, attacker, damage, vec3_origin);
		else
			G_PostMessage(target, MSG_DEATH, PRI_DIRECTIVE, "eeei", target, inflictor, attacker, damage);

		return;
	}

	// Also, put the flag of fire on the entity - makes fire lower when die.
	if (target->size[2] > 24.0f)
		target->s.effects |= EF_DISABLE_EXTRA_FX;

	if (target->dead_state != DEAD_DEAD)
	{
		target->touch = NULL;
		M_DeathUse(target);
	}

	if (target->client != NULL)
	{
		//FIXME: Make sure you can still dismember and gib player while dying.
		target->client->meansofdeath = mod;
		PlayerDie(target, inflictor, attacker, damage, point);
	}
	else
	{
		if ((target->classID == CID_NONE || classStatics[target->classID].msgReceivers[MSG_PAIN] == NULL) && target->die != NULL)
			target->die(target, inflictor, attacker, damage, vec3_origin);
		else
			G_PostMessage(target, MSG_DEATH, PRI_DIRECTIVE, "eeei", target, inflictor, attacker, damage);
	}

	if (Vec3IsZero(target->velocity)) //mxd. BUGFIX, sorta: '&& damage != 12345' in original logic. '12345' value is never used anywhere else.
	{
		// Fall back some!
		vec3_t hit_dir;
		VectorSubtract(target->s.origin, inflictor->s.origin, hit_dir);
		hit_dir[2] = 50.0f;

		VectorNormalize(hit_dir);
		VectorMA(target->velocity, (float)damage * 3.0f, hit_dir, target->velocity);
	}
}

static void M_ReactToDamage(edict_t* target, edict_t* attacker)
{
	if ((attacker->client == NULL && !(attacker->svflags & SVF_MONSTER)) || attacker == target)
		return;

	//FIXME: in SP, after player dead, allow this? Or make attacker lose it's enemy?
	if (!ANARCHY && attacker->classID == target->classID)
		return; // Monsters of same type won't fight each other.

	if (target->classID == CID_OGLE && (!target->monsterinfo.awake || attacker->client != NULL)) // Ogles do their own checks to get angry at their first enemy.
		return;

	if (attacker == target->enemy)
	{
		// Ok, no more stalking - now we get serious.
		target->ai_mood_flags &= ~AI_MOOD_FLAG_BACKSTAB;

		if (!target->monsterinfo.awake)
			AI_FoundTarget(target, true);

		return;
	}

	if (attacker->takedamage == DAMAGE_NO) // World, etc.
		return;

	if (target->monsterinfo.c_mode) // Don't anger cinematic monsters.
		return;

	if (attacker->client != NULL)
	{
		target->monsterinfo.chase_finished = level.time + 4.0f; // When the monster can notice secondary enemies.

		if (target->enemy != NULL && target->enemy->client != NULL)
			target->oldenemy = target->enemy;

		target->enemy = attacker;
		AI_FoundTarget(target, true);

		return;
	}

	if ((target->monsterinfo.aiflags & AI_GOOD_GUY) && !(attacker->monsterinfo.aiflags & AI_GOOD_GUY))
	{
		target->enemy = attacker;
		AI_FoundTarget(target, true);

		return;
	}

	// If attacker is a client or it's the same base (walk/swim/fly) type and a different classname
	// and it's a monster that sprays too much, get mad at them.
	if ((target->flags & (FL_FLY | FL_SWIM)) == (attacker->flags & (FL_FLY | FL_SWIM)) && target->classID != attacker->classID &&
		target->enemy != NULL) // Target has an enemy, otherwise always get mad.
	{
		if (target->enemy->client != NULL)
			target->oldenemy = target->enemy;

		target->enemy = attacker;
		AI_FoundTarget(target, true);

		return;
	}

	// Otherwise get mad at whoever they are mad at (help our buddy).
	if (attacker->enemy != NULL) // This really should be an assert, but there are problems with this.
	{
		if (attacker->enemy == target && attacker->classID == target->classID && !(target->monsterinfo.aiflags & AI_AGRESSIVE))
		{
			// Attacker was shooting at me (target) and is my class, but I'm not aggressive so I didn't hit him first.
			if (irand(0, 10) < 7 && target->monsterinfo.flee_finished < level.time + 7.0f)
			{
				// Run away!
				target->monsterinfo.aiflags |= AI_FLEE;
				target->monsterinfo.flee_finished = level.time + flrand(3.0f, 7.0f);
			}

			target->enemy = attacker;
			AI_FoundTarget(target, true);
		}
		else if (attacker->enemy != target && (target->enemy == NULL || target->enemy->health <= 0))
		{
			// Attacker wasn't after me and my enemy is invalid or don't have one... go after attacker's enemy.
			if (target->enemy != NULL && (target->enemy->client != NULL || ANARCHY))
				target->oldenemy = target->enemy;

			target->enemy = attacker->enemy;
			AI_FoundTarget(target, true);
		}
		else if ((attacker->classID != target->classID && !irand(0, 2)) || ANARCHY)
		{
			// 30% chance to get mad (only if they're not my class), or always get mad if ANARCHY.
			if (target->enemy != NULL && (target->enemy->client != NULL || ANARCHY))
				target->oldenemy = target->enemy;

			target->enemy = attacker;
			AI_FoundTarget(target, true);
		}

		return;
	}

	// Attacker's on crack, kill him.
	target->enemy = attacker;
	AI_FoundTarget(target, true);
}

static qboolean IsFlammable(const edict_t* target)
{
	return (target->materialtype == MAT_CLOTH || target->materialtype == MAT_FLESH || target->materialtype == MAT_POTTERY ||
			target->materialtype == MAT_LEAF || target->materialtype == MAT_WOOD || target->materialtype == MAT_INSECT);
}

/**
 * @param target	Entity that is being damaged.
 * @param inflictor	Entity that is causing the damage.
 * @param attacker	Entity that caused the inflictor to damage target.
 *
 * @param p_dir		Direction of the attack.
 *					Directional vector (velocity is acceptable), in the direction the force is GOING.
 *					Normalized in the function, if (0,0,0) then vector from inflictor to target is used.
 *					Used for knockback (scale force by this vector).
 *					Also used for blood and puffs when objects are struck.
 *
 * @param p_point	Point at which the damage is being inflicted.
 *					Absolute point at which the damage is generated.
 *					Used for hit locations, and blood (generation point).
 *
 * @param p_normal	Normal vector from that point.
 *					Directional vector, assumed to be normalized.
 *					Used for blood from monsters and players (squirts in this direction).
 *
 * @param damage	Amount of damage being inflicted.
 * @param knockback	Force to be applied against target as a result of the damage.
 *
 * @param dflags	Flags used to control how T_Damage works:\n
 *					DAMAGE_RADIUS: damage was indirect (from a nearby explosion).\n
 *					DAMAGE_NO_KNOCKBACK: do not affect velocity, just view angles.\n
 *					DAMAGE_NO_PROTECTION: kills godmode, armor, everything.\n
 *					DAMAGE_DISMEMBER: force MSG_DISMEMBER to be used.
 *
 * @param mod		Means of death flag (MOD_XXX).
 */
void T_Damage(edict_t* target, edict_t* inflictor, edict_t* attacker, const vec3_t p_dir, const vec3_t p_point, const vec3_t p_normal, int damage, int knockback, const int dflags, int mod) //TODO: split into smaller functions?
{
	// Copious amounts of damage avoidance cases...

	if (target->takedamage == DAMAGE_NO)
		return;

	// Player is in a remote camera or cinematic mode, so he can't be hurt.
	if (target->client != NULL && (target->client->RemoteCameraLockCount > 0 || target->client->playerinfo.c_mode))
		return;

	if ((dflags & DAMAGE_ALIVE_ONLY) && (target->materialtype != MAT_FLESH || target->health <= 0))
		return;

	if ((target->svflags & SVF_NO_PLAYER_DAMAGE) && attacker->client != NULL)
		return;

	qboolean was_dead = false;

	if (target->health <= 0)
	{
		if (target->client != NULL) // Can't keep killing a dead player.
			return;

		if (target->classID != CID_NONE && classStatics[target->classID].msgReceivers[MSG_DEATH_PAIN] != NULL)
		{
			if ((dflags & DAMAGE_SUFFOCATION) || (dflags & DAMAGE_BLEEDING) || (dflags == DAMAGE_BURNING))
				return;

			was_dead = true;
		}
	}

	// If we are on a shrine or teleporting we are not to be harmed	- assuming we haven't been telefragged, in which case kill us dead.
	if (target->client != NULL && target->client->shrine_framenum > level.time && !(dflags & DAMAGE_NO_PROTECTION))
		return;

	// Friendly-fire avoidance. If enabled, you can't hurt teammates (but you can hurt yourself). Knockback still occurs though.
	if (target->client != NULL && attacker->client != NULL && target != attacker)
	{
		// Both different players, let's check if this will do damage!
		if (COOP)
		{
			if (!(DMFLAGS & DF_HURT_FRIENDS) && !(dflags & DAMAGE_HURT_FRIENDLY))
				damage = 0;
			else
				mod |= MOD_FRIENDLY_FIRE;
		}
		else if (DEATHMATCH)
		{
			if ((DMFLAGS & (DF_MODELTEAMS | DF_SKINTEAMS)) && OnSameTeam(target, attacker))
			{
				if (!(DMFLAGS & DF_HURT_FRIENDS) && (!(dflags & DAMAGE_HURT_FRIENDLY)))
					damage = 0;
				else
					mod |= MOD_FRIENDLY_FIRE;
			}
		}
	}

	if (target->client != NULL)
	{
		if (DEATHMATCH)
		{
			// Factor in deathmatch to increase hit points.
			const float dmg_scaler = 4.0f - skill->value;
			if (dmg_scaler < 1.0f)
				damage *= 4;
			else
				damage = (int)(ceilf((float)damage * 2.0f / dmg_scaler)); // Skill 0: 1/2 damage; skill 1: 2/3 damage, skill 2: full
		}
		else
		{
			// In easy skill-level, the player only takes half damage.
			if (SKILL == 0)
				damage /= 2;
		}
	}

	if (damage == 0 && (DMFLAGS & DF_HURT_FRIENDS) && !(dflags & DAMAGE_HURT_FRIENDLY) && !(DEATHMATCH && (DMFLAGS & (DF_MODELTEAMS | DF_SKINTEAMS))))
		damage = 1;

	const int orig_dmg = damage;

	// Copy passed-in vectors, so we don't overwrite them.
	vec3_t dir;
	if (p_dir != NULL && Vec3NotZero(p_dir))
		VectorCopy(p_dir, dir);
	else
		VectorSet(dir, 0.0f, 0.0f, -1.0f);

	VectorNormalize(dir);

	vec3_t normal;
	if (p_normal != NULL && Vec3NotZero(p_normal))
		VectorCopy(p_normal, normal);
	else
		VectorSet(normal, 0.0f, 0.0f, 1.0f);

	vec3_t point;
	if (p_point != NULL)
		VectorCopy(p_point, point);
	else
		VectorCopy(inflictor->s.origin, point);

	// Deal with player armor - if we have any.
	if (target->client != NULL && target->client->playerinfo.pers.armor_count > 0.0f && !(dflags & DAMAGE_BLEEDING))
	{
		// Figure out where the armor details are located at.
		// Don't let armor effect us if we are drowning, in slime, or in lava.
		if (!(dflags & DAMAGE_AVOID_ARMOR) && dflags != DAMAGE_SUFFOCATION && dflags != DAMAGE_SLIME && dflags != DAMAGE_LAVA)
		{
			gitem_armor_t* info;

			if (target->client->playerinfo.pers.armortype == ARMOR_TYPE_SILVER)
				info = &silver_armor_info;
			else
				info = &gold_armor_info;

			// Figure out how much the armor takes.
			int armor_absorb = damage;

			// Effect damage dependant on what type of effect hit us.
			if (dflags & DAMAGE_SPELL)
				damage = (int)((float)damage * info->spell_protection);
			else
				damage = (int)((float)damage * info->normal_protection);

			// Make the little sparkle effect at the hit point.
			gi.CreateEffect(NULL, FX_ARMOR_HIT, 0, point, "d", normal);

			if (dflags & DAMAGE_SPELL)
			{
				gi.sound(target, CHAN_WEAPON, gi.soundindex("weapons/spellric.wav"), 2.0f, ATTN_NORM, 0.0f); //TODO: does setting vol > 1.0 make any sense? 
			}
			else if (irand(0, 2) == 0) // Don't always make sound - being attacked by rats makes this go off incessantly.
			{
				char armor_sound[50];
				sprintf_s(armor_sound, sizeof(armor_sound), "weapons/armorric%i.wav", irand(1, 3)); //mxd. sprintf -> sprintf_s
				gi.sound(target, CHAN_WEAPON, gi.soundindex(armor_sound), 2.0f, ATTN_NORM, 0.0f); //TODO: does setting vol > 1.0 make any sense? 
			}

			// Be sure we still have some damage.
			if (damage == 0)
			{
				damage = 1;
				armor_absorb = 1;
			}
			else
			{
				// Everything not taken by the player is taken by the armor.
				armor_absorb -= damage;
				if (armor_absorb > (int)target->client->playerinfo.pers.armor_count)
				{
					damage += armor_absorb - (int)target->client->playerinfo.pers.armor_count;
					armor_absorb = (int)target->client->playerinfo.pers.armor_count;
				}
			}

			// Decrease armor count.
			target->client->playerinfo.pers.armor_count -= (float)armor_absorb;

			// Are we down to zero armor?
			if (target->client->playerinfo.pers.armor_count <= 0.0f)
			{
				// Stop drawing the armor.
				target->client->playerinfo.pers.armortype = ARMOR_TYPE_NONE; //mxd. ARMOR_NONE in original version.
				target->client->playerinfo.pers.armor_count = 0.0f;

				Player_UpdateModelAttributes(target); //mxd

				// Play the out-of-armor sound.
				gi.sound(target, CHAN_WEAPON, gi.soundindex("weapons/armorgone.wav"), 1.0f, ATTN_NORM, 0.0f);
			}
		}
	}

	// SV_FREEZEMONSTERS = 2 means no knockback.
	if ((target->flags & FL_NO_KNOCKBACK) || (target->svflags & SVF_BOSS) || ((target->svflags & SVF_MONSTER) && SV_FREEZEMONSTERS == 2))
		knockback = 0;

	// Figure out the knockback momentum to impart to the target.
	if (!(dflags & DAMAGE_NO_KNOCKBACK) && !(target->svflags & SVF_BOSS))
	{
		// Knockback of less than about 25 isn't going to do squat...
		if (knockback > 0 && target->movetype != PHYSICSTYPE_NONE && target->movetype != PHYSICSTYPE_PUSH && target->movetype != PHYSICSTYPE_STOP)
		{
			if (Vec3IsZero(dir)) //TODO: never happens? already checked when copying p_dir.
			{
				VectorSubtract(target->s.origin, inflictor->s.origin, dir);
				dir[2] = max(0.0f, dir[2]);
				VectorNormalize(dir);
			}

			const float mass = VectorLength(target->size) * 3.0f;
			float force = 600.0f * (float)knockback / mass;

			// Players are not as affected by velocities when they are on the ground, so increase what players experience.
			if (target->client != NULL)
				force *= (target->groundentity != NULL ? 4.0f : 0.25f);

			if (dflags & DAMAGE_EXTRA_KNOCKBACK)
				force *= 3.0f;

			force = min(512.0f, force); // Cap this speed, so it doesn't get insane.

			vec3_t knockback_vel;
			VectorScale(dir, force, knockback_vel);

			// Don't force players up quite so much as monsters.
			const float up_vel = (target->client != NULL ? 30.0f : 60.0f);

			// Now if the player isn't being forced DOWN very far, let's force them UP a bit.
			if ((dir[2] > -0.5f || target->groundentity != NULL) && knockback_vel[2] < up_vel && force > 30.0f)
				knockback_vel[2] = min(force, up_vel); // Don't knock UP the player more than we do across...

			VectorAdd(target->velocity, knockback_vel, target->velocity);

			if (target->client != NULL) // If player, then set the player flag that will affect this.
			{
				target->client->playerinfo.flags |= PLAYER_FLAG_USE_ENT_POS;

				// The knockback_time indicates how long this knockback affects the player.
				float knockback_time;
				if (force > 500.0f)
					knockback_time = level.time + 1.25f;
				else
					knockback_time = level.time + (force / 400.0f);

				if (knockback_time > target->client->playerinfo.knockbacktime)
				{
					target->client->playerinfo.knockbacktime = knockback_time;

					// Since we are bing knocked by tornado, let our top speed be higher.
					if (mod == MOD_TORN) //TODO: but what about MOD_FRIENDY_FIRE flag?..
					{
						target->client->playerinfo.effects |= EF_HIGH_MAX;
						target->s.effects |= EF_HIGH_MAX;
					}
					else
					{
						target->client->playerinfo.effects &= ~EF_HIGH_MAX;
						target->s.effects &= ~EF_HIGH_MAX;
					}
				}
			}

			// So knockback doesn't gib them unless it really really should.
			if (force < 300.0f)
				target->jump_time = level.time + 0.5f;
		}
	}

	int dmg_take = damage;

	// If the target has godmode in effect, they take no damage.
	if ((target->flags & FL_GODMODE) && !(dflags & DAMAGE_NO_PROTECTION))
		dmg_take = 0;

	gclient_t* client = target->client;

	// If the player is invincible, or on a shrine, they take no damage.
	if (client != NULL && (int)client->invincible_framenum > level.framenum && !(dflags & DAMAGE_NO_PROTECTION))
	{
		if (target->pain_debounce_time < level.time)
			target->pain_debounce_time = level.time + 2.0f;

		dmg_take = 0;
	}

	//mxd. Skip non-implemented CheckTeamDamage() logic.

	HitLocation_t hl = hl_NoneSpecific; //mxd. Initialize.

	// Okay, we got all the way here, so do the damage.
	if (dmg_take > 0 && !((target->svflags & SVF_MONSTER) && SV_FREEZEMONSTERS) && !(dflags & DAMAGE_ALL_KNOCKBACK))
	{
		// Process fire damage. Not DAMAGE_BURNING, because that is coming from you being on fire already...
		if ((dflags & DAMAGE_FIRE) && IsFlammable(target) && dflags != DAMAGE_BURNING && !(target->svflags & SVF_BOSS))
		{
			// Don't burn underwater.
			if (!(gi.pointcontents(target->s.origin) & (CONTENTS_WATER | CONTENTS_SLIME)))
			{
				//FIXME: not on BBRUSHES - have no origin!
				float duration = ((float)orig_dmg * ((dflags & DAMAGE_FIRE_LINGER) ? 0.4f : 0.2f)); //mxd. int in original logic.

				// The phoenix is just too damn powerful if it can do serious fire damage too...
				if (duration == 0.0f || (dflags & DAMAGE_PHOENIX))
					duration = 1.0f; // This makes a burning death if the player should die, but it goes out right away.
				else if (duration > 8.0f)
					duration = 8.0f;

				// Time to update fire damage effect?
				if (target->fire_damage_time < level.time && !(target->svflags & SVF_BOSS))
				{
					// Not already on fire.
					const float duration_scaler = ((target->client != NULL && DEATHMATCH) ? 0.25f : 0.5f); //mxd
					target->fire_damage_time = level.time + duration * duration_scaler;

					if (!was_dead)
						target->s.effects &= ~EF_DISABLE_EXTRA_FX; // The flag causes the fire to stop generating.

					target->s.effects |= EF_ON_FIRE;

					int scale = (int)(VectorLength(target->size) * 0.125f); // Is scaled up drediculously on other side, quarter it.
					scale = min(255, scale);

					gi.CreateEffect(&target->s, FX_FIRE_ON_ENTITY, CEF_OWNERS_ORIGIN, NULL, "bbb", scale, 255, 1); // We'll turn this off manually.
				}
				else
				{
					target->fire_damage_time += duration;
				}

				// Always set the damage enemy to the most recent entity doing damage.
				target->fire_damage_enemy = attacker;
			}
		}
		else if (!(dflags & DAMAGE_NO_BLOOD))
		{
			if ((target->svflags & SVF_MONSTER) || client != NULL)
			{
				vec3_t vel;
				vec3_t loc;

				// Normal will be NULL from time to time. FIXME:  Hellbolts will damage with a plane.normal that is null.
				if (Vec3IsZero(normal)) //TODO: never happens? already checked when copying p_normal.
				{
					VectorClear(vel);
					VectorCopy(point, loc);
				}
				else
				{
					VectorScale(normal, -64.0f, vel);

					// Now let's try moving the hit point towards the hit object.
					vec3_t diff;
					VectorSubtract(target->s.origin, point, diff);

					// We can't be assured that the vertical origin is the center...
					diff[2] += (target->maxs[2] + target->mins[2]) * 0.5f;

					// Take half that distance, since the hit always tends to be on the outside of the bbox.
					VectorMA(point, 0.5f, diff, loc);
				}

				if (BLOOD_LEVEL == VIOLENCE_NONE)
				{
					gi.CreateEffect(NULL, FX_HITPUFF, CEF_FLAG6, point, "db", dir, 5);
				}
				else
				{
					const int flags = (target->materialtype == MAT_INSECT ? CEF_FLAG8 : 0); //mxd
					const int blood_amt = ((dmg_take > 80) ? 20 : dmg_take / 4); //mxd
					gi.CreateEffect(NULL, FX_BLOOD, flags, loc, "ub", vel, blood_amt);
				}
			}
			else
			{
				if (((target->svflags & SVF_DEADMONSTER) || target->materialtype == MAT_INSECT || target->materialtype == MAT_FLESH) && BLOOD_LEVEL > VIOLENCE_NONE)
				{
					const int flags = (target->materialtype == MAT_INSECT ? CEF_FLAG8 : 0); //mxd
					gi.CreateEffect(NULL, FX_BLOOD, flags, point, "ub", dir, 8);
				}
				else
				{
					gi.CreateEffect(NULL, FX_HITPUFF, 0, point, "db", dir, 5);
				}
			}
		}
		else if (dflags & DAMAGE_BUBBLE)
		{
			vec3_t bubble_pos = { flrand(-10.0f, 10.0f), flrand(-10.0f, 10.0f), (float)target->viewheight };
			VectorAdd(bubble_pos, target->s.origin, bubble_pos);

			gi.CreateEffect(NULL, FX_BUBBLE, 0, bubble_pos, NULL);
		}

		// Apply damage.
		target->health -= dmg_take;

		if (target != attacker && BLOOD_LEVEL > VIOLENCE_BLOOD) // Can't dismember yourself.
		{
			vec3_t hit_spot;
			if (attacker == inflictor)
				VectorCopy(point, hit_spot);
			else
				VectorCopy(inflictor->s.origin, hit_spot);

			//TODO: 2-nd case never used (harpy is monster)? Why CID_HARPY needs separate GetHitLocation() logic?
			if (target->classID != CID_HARPY) // Use new hitlocation function.
				hl = MG_GetHitLocation(target, inflictor, point, dir);
			else if (!(target->svflags & SVF_MONSTER) && client == NULL) // Target not a monster or client.
				hl = MG_GetHitLocation(target, inflictor, hit_spot, vec3_origin);
			else
				hl = MG_GetHitLocation(target, attacker, hit_spot, vec3_origin);

			if (dflags & DAMAGE_DISMEMBER)
				hl |= hl_MeleeHit; // Only melee can dismember. Add the 16th bit to it for melee hit.

			// Don't dismember someone who's already gibbed or is gibbing, no dismember damage from suffocation or bleeding.
			if (!(target->svflags & SVF_PARTS_GIBBED) && !(dflags & DAMAGE_SUFFOCATION) && !(dflags & DAMAGE_BLEEDING))
			{
				int dismemeber_dmg = dmg_take;

				if (dflags & DAMAGE_DOUBLE_DISMEMBER)
					dismemeber_dmg *= 2;

				if (target->client != NULL)
					PlayerDismember(target, attacker, dismemeber_dmg, hl);
				else
					G_PostMessage(target, MSG_DISMEMBER, PRI_DIRECTIVE, "ii", dismemeber_dmg, hl);
			}
		}

		if (target->health <= 0)
		{
			// Target has died, so kill it good and dead.
			if (was_dead)
			{
				//FIXME: if on fire, become a charred husk, no gib.
				if (dflags != DAMAGE_SUFFOCATION && !(dflags & DAMAGE_BLEEDING) && dflags != DAMAGE_BURNING)
				{
					// Drowning, bleeding and burning do not gib.
					if (target->health <= -100)
					{
						if (target->think != BecomeDebris && target->think != G_SetToFree)
						{
							target->post_think = BecomeDebris;
							target->next_post_think = level.time;
						}
					}
					else if (BLOOD_LEVEL > VIOLENCE_BLOOD)
					{
						hl |= hl_MeleeHit; // Force dismember.
						G_PostMessage(target, MSG_DEATH_PAIN, PRI_DIRECTIVE, "ii", dmg_take, hl);
					}
				}

				return;
			}

			// Player died from fire damage. //TODO: can player have SVF_BOSS flag?..
			if (target->client != NULL && (dflags & DAMAGE_FIRE) && !(target->svflags & SVF_BOSS))
			{
				target->fire_damage_time = -1; // So we know we died from fire.

				// Spawn a fire to keep burning for ~ 6 secs.
				target->s.effects &= ~EF_DISABLE_EXTRA_FX; // The flag causes the fire to stop generating.
				target->s.effects |= EF_ON_FIRE; // The flag causes the fire to stop generating.

				int scale = (int)(VectorLength(target->size) * 4.0f); // Eight times the value is sent over, no more than 32 wide.
				scale = min(255, scale);

				gi.CreateEffect(&target->s, FX_FIRE_ON_ENTITY, CEF_OWNERS_ORIGIN, NULL, "bbb", scale, 40, 0);
			}

			// Already killed by decapitation or some other killing dismemberment?
			if (target->takedamage != DAMAGE_NO)
				Killed(target, inflictor, attacker, dmg_take, point, mod);

			return;
		}
	}

	if ((target->svflags & SVF_MONSTER) && SV_FREEZEMONSTERS)
	{
		// Do do anything. Frozen monsters take no damage, don't die.
	}
	else if (target->svflags & SVF_MONSTER)
	{
		target->spawnflags &= ~MSF_AMBUSH;
		target->targetname = NULL;

		M_ReactToDamage(target, attacker);

		if (!(target->monsterinfo.aiflags & AI_DUCKED) && dmg_take > 0 && target->pain_debounce_time < level.time)
		{
			const qboolean force_pain = (target->enemy == NULL); //mxd

			if (target->classID == CID_ASSASSIN)
				G_PostMessage(target, MSG_PAIN, PRI_DIRECTIVE, "eeiii", inflictor, attacker, force_pain, dmg_take, hl);
			else
				G_PostMessage(target, MSG_PAIN, PRI_DIRECTIVE, "eeiii", target, attacker, force_pain, dmg_take, hl);

			// In Nightmare skill-level, monsters don't go into pain frames often.
			if (SKILL >= 3)
				target->pain_debounce_time = level.time + 5;
		}
	}
	else if (client != NULL)
	{
		if (!(target->flags & FL_GODMODE) && dmg_take > 0)
			G_PostMessage(target, MSG_PAIN, PRI_DIRECTIVE, "eeiii", target, attacker, knockback, dmg_take, hl);
	}
	else if (dmg_take > 0 && target->pain != NULL)
	{
		if (target->classID == CID_NONE || classStatics[target->classID].msgReceivers[MSG_PAIN] == NULL)
		{
			if (Q_stricmp(target->classname, TUTORIAL_CHICKEN_CLASSNAME) == 0)
				target->activator = inflictor;

			target->pain(target, attacker, (float)knockback, dmg_take); // Pass spot too.
		}
		else
		{
			G_PostMessage(target, MSG_PAIN, PRI_DIRECTIVE, "eeiii", target, attacker, knockback, dmg_take, hl);
		}
	}

	// Add to the damage inflicted on a player this frame.
	// The total will be turned into screen blends and view angle kicks at the end of the frame.
	if (client != NULL)
	{
		client->damage_gas = (Q_stricmp(inflictor->classname, "plague_mist") == 0 || Q_stricmp(inflictor->classname, "spreader_grenade") == 0);
		client->damage_blood += dmg_take;
		client->damage_knockback += knockback;

		VectorCopy(point, client->damage_from);
	}
}

void T_DamageRadius(edict_t* inflictor, edict_t* attacker, const edict_t* ignore, const float radius, const float maxdamage, const float mindamage, const int dflags, const int mod)
{
	assert(radius > 0.0f);

	edict_t* damage_enemy = NULL;

	if (dflags & DAMAGE_ENEMY_MAX)
	{
		damage_enemy = inflictor->enemy;

		if (damage_enemy != NULL && damage_enemy->takedamage != DAMAGE_NO)
		{
			vec3_t center;
			VectorAdd(damage_enemy->mins, damage_enemy->maxs, center);
			VectorMA(damage_enemy->s.origin, 0.5f, center, center);

			vec3_t v;
			VectorSubtract(inflictor->s.origin, center, v);
			VectorNormalize(v);

			vec3_t dir;
			VectorScale(v, -1.0f, dir);

			vec3_t hit_spot;
			VectorMA(center, damage_enemy->maxs[0], v, hit_spot); // Estimate a good hit spot.

			T_Damage(damage_enemy, inflictor, attacker, dir, hit_spot, vec3_origin, (int)maxdamage, (int)maxdamage, dflags | DAMAGE_RADIUS, mod);
		}
	}

	edict_t* ent = NULL;
	while ((ent = FindInRadius(ent, inflictor->s.origin, radius)) != NULL)
	{
		if (ent == ignore || ent == damage_enemy || ent->takedamage == DAMAGE_NO || ent->takedamage == DAMAGE_NO_RADIUS) // We already dealt with the damage_enemy above...
			continue;

		if (ent == attacker && (dflags & DAMAGE_ATTACKER_IMMUNE))
			continue;

		if ((dflags & DAMAGE_ALIVE_ONLY) && (ent->materialtype != MAT_FLESH || ent->health <= 0))
			continue;

		vec3_t center;
		VectorAdd(ent->mins, ent->maxs, center);
		VectorMA(ent->s.origin, 0.5f, center, center);

		vec3_t v;
		VectorSubtract(inflictor->s.origin, center, v);
		const float dist = VectorNormalize(v);

		vec3_t dir;
		VectorScale(v, -1.0f, dir);

		// Scale from maxdamage at center to mindamage at outer edge.
		const float points = maxdamage - ((maxdamage - mindamage) * (dist / radius));

		if (points > 0.0f && CanDamage(ent, inflictor))
		{
			vec3_t hit_spot;
			VectorMA(center, ent->maxs[0], v, hit_spot); // Estimate a good hit spot.

			// Process self-damage?
			if (ent == attacker)
			{
				if (dflags & DAMAGE_ATTACKER_KNOCKBACK)
					T_Damage(ent, inflictor, attacker, dir, hit_spot, vec3_origin, 0, (int)points, dflags | DAMAGE_RADIUS, MOD_KILLED_SLF);
				else if (dflags & DAMAGE_POWERPHOENIX)
					T_Damage(ent, inflictor, attacker, dir, hit_spot, vec3_origin, (int)(points * 0.25f), (int)(points * 0.5f), dflags | DAMAGE_RADIUS, MOD_KILLED_SLF); // Extra knockback, 0.25 damage.
				else
					T_Damage(ent, inflictor, attacker, dir, hit_spot, vec3_origin, (int)points, (int)points, dflags | DAMAGE_RADIUS, MOD_KILLED_SLF);
			}
			else
			{
				T_Damage(ent, inflictor, attacker, dir, hit_spot, vec3_origin, (int)points, (int)points, dflags | DAMAGE_RADIUS, mod);
			}
		}
	}
}

// Same function, except the origin point of the damage doesn't have to be the same as the inflictor's.
void T_DamageRadiusFromLoc(vec3_t origin, edict_t* inflictor, edict_t* attacker, const edict_t* ignore, const float radius, const float maxdamage, const float mindamage, const int dflags, const int mod)
{
	assert(radius > 0.0f);

	edict_t* ent = NULL;
	while ((ent = FindInRadius(ent, origin, radius)) != NULL)
	{
		if (ent == ignore || ent->takedamage == DAMAGE_NO || ent->takedamage == DAMAGE_NO_RADIUS)
			continue;

		if (ent == attacker && (dflags & DAMAGE_ATTACKER_IMMUNE))
			continue;

		if ((dflags & DAMAGE_ALIVE_ONLY) && (ent->materialtype != MAT_FLESH || ent->health <= 0))
			continue;

		// If we are reflecting, stop us from taking damage.
		if (EntReflecting(ent, true, true)) //TODO: no reflecting check in T_DamageRadius(). Unintentional?
			continue;

		vec3_t center;
		VectorAdd(ent->mins, ent->maxs, center);
		VectorMA(ent->s.origin, 0.5f, center, center);

		vec3_t v;
		VectorSubtract(origin, center, v);
		const float dist = VectorNormalize(v);

		vec3_t dir;
		VectorScale(v, -1.0f, dir);

		// Scale from maxdamage at center to mindamage at outer edge.
		const float points = maxdamage - ((maxdamage - mindamage) * (dist / radius));

		if (points > 0 && CanDamageFromLoc(ent, inflictor, origin))
		{
			vec3_t hit_spot;
			VectorMA(center, ent->maxs[0], v, hit_spot); // Estimate a good hit spot.

			// Process self-damage?
			if (ent == attacker)
			{
				if (dflags & DAMAGE_ATTACKER_KNOCKBACK)
					T_Damage(ent, inflictor, attacker, dir, hit_spot, vec3_origin, 0, (int)points, dflags | DAMAGE_RADIUS, MOD_KILLED_SLF);
				else if (dflags & DAMAGE_POWERPHOENIX)
					T_Damage(ent, inflictor, attacker, dir, hit_spot, vec3_origin, (int)(points * 0.25f), (int)(points * 2.0f), dflags | DAMAGE_RADIUS, MOD_KILLED_SLF); // Extra knockback, 0.25 damage.
				else
					T_Damage(ent, inflictor, attacker, dir, hit_spot, vec3_origin, (int)points, (int)points, dflags | DAMAGE_RADIUS, MOD_KILLED_SLF);
			}
			else
			{
				T_Damage(ent, inflictor, attacker, dir, hit_spot, vec3_origin, (int)points, (int)points, dflags | DAMAGE_RADIUS, mod);
			}
		}
	}
}