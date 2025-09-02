//
// spl_maceballs.c
//
// Copyright 1998 Raven Software
//

#include "spl_maceballs.h" //mxd
#include "g_Physics.h"
#include "g_playstats.h"
#include "g_combat.h" //mxd
#include "p_dll.h" //mxd
#include "spl_teleport.h"
#include "FX.h"
#include "Vector.h"
#include "Utilities.h"
#include "g_local.h"

#define MACEBALL_UPSPEED			200.0f
#define MACEBALL_SPEED				250.0f
#define MACEBALL_DOWNSPEED			150.0f
#define MACEBALL_RADIUS				16.0f
#define MACEBALL_SCALE				0.17f
#define MACEBALL_SCALE_INCREMENT	0.03f
#define MACEBALL_SEARCH_RADIUS		500.0f

static void MaceballThink(edict_t* self)
{
	if (self->s.scale < MACEBALL_SCALE)
		self->s.scale = min(MACEBALL_SCALE, self->s.scale + MACEBALL_SCALE_INCREMENT);

	// Check the NOTARGET flag to see if the mace should readjust to a new target.
	if (self->flags & FL_NOTARGET)
	{
		// Head towards its enemy
		self->flags &= ~FL_NOTARGET;

		if (self->enemy != NULL)
		{
			VectorScale(self->movedir, MACEBALL_UPSPEED, self->velocity);

			vec3_t dir;
			VectorSubtract(self->enemy->s.origin, self->s.origin, dir);
			VectorNormalize(dir);
			VectorMA(self->velocity, MACEBALL_SPEED, dir, self->velocity);
		}

		if (self->velocity[2] > 0.0f)
			self->velocity[2] = MACEBALL_UPSPEED; // Adjust anyway so that the ball has a consistent bounce height.
	}

	VectorCopy(self->s.origin, self->last_org);

	self->nextthink = level.time + FRAMETIME; //mxd. Use define.

	// Now check if we should die soon.
	if (self->dead_state == DEAD_DYING || self->touch_debounce_time + MACEBALL_EXTRALIFE <= level.time)
	{
		gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/MaceBallDeath.wav"), 2.0f, ATTN_NORM, 0.0f); //TODO: why 2.0 volume?
		gi.CreateEffect(NULL, FX_WEAPON_MACEBALLEXPLODE, 0, self->s.origin, "d", self->velocity);
		G_SetToFree(self);
	}
}

static void GetCollisionPoint(const vec3_t velocity, const vec3_t origin, const float size, vec3_t point)
{
	static vec3_t box_normals[] = //mxd. Made local static.
	{
		{ 0.0f, 0.0f, 1.0f }, {  0.0f,  0.0f, -1.0f }, // Up / down.
		{ 0.0f, 1.0f, 0.0f }, {  0.0f, -1.0f,  0.0f }, // Left / right.
		{ 1.0f, 0.0f, 0.0f }, { -1.0f,  0.0f,  0.0f }, // Front / back.
	};

	int max_axis = 1;
	float max = -1.0f;

	for (int i = 0; i < 6; i++)
	{
		const float theta = DotProduct(velocity, box_normals[i]);

		if (theta > max)
		{
			max = theta;
			max_axis = i;
		}
	}

	VectorMA(origin, size, box_normals[max_axis], point);
}

static void MaceballBounce(edict_t* self, trace_t* trace)
{
	// Did we hit something we can destroy?
	if (trace->ent != NULL && trace->ent->takedamage != DAMAGE_NO && trace->ent->health > 0)
	{
		vec3_t move_vec;
		VectorNormalize2(self->velocity, move_vec);

		if (EntReflecting(trace->ent, true, true))
		{
			// Do nothing except bounce if we hit someone who's reflecting and make whoever it bounced off the owner of the ball now.
			self->enemy = self->owner;
			self->owner = trace->ent;
		}
		else if (trace->ent->svflags & SVF_BOSS)
		{
			T_Damage(trace->ent, self, self->owner, move_vec, trace->endpos, move_vec,
				MACEBALL_BOSS_DAMAGE, MACEBALL_BOSS_DAMAGE, 0, MOD_P_IRONDOOM);

			self->dead_state = DEAD_DYING;
		}
		else
		{
			qboolean teleported = false; //mxd

			// Can we teleport the player out of danger?
			if (trace->ent->client != NULL) //TODO: this logic is very strange. Shouldn't we just teleport player if he has teleport spell and enough mana to use it?
			{
				const gitem_t* defence = trace->ent->client->playerinfo.pers.defence;
				const int quantity = playerExport.p_itemlist[12].quantity; // Quantity of item_defense_shield (MANA_USE_SHIELD).

				if (defence->ammo != NULL && quantity > 0)
				{
					// Do we have enough mana to teleport?
					const gitem_t* mana_item = P_FindItem(defence->ammo);
					const int mana_index = ITEM_INDEX(mana_item);

					if (trace->ent->client->playerinfo.pers.inventory.Items[mana_index] / quantity > 0)
					{
						// Yes, do we actually have a teleport spell?
						if (trace->ent->client->playerinfo.pers.inventory.Items[13] > 0)
						{
							SpellCastTeleport(trace->ent);

							if (!DEATHMATCH || !(DMFLAGS & DF_INFINITE_MANA))
								trace->ent->client->playerinfo.pers.inventory.Items[mana_index] -= quantity;

							teleported = true;
						}
					}
				}

			}

			if (!teleported)
			{
				T_Damage(trace->ent, self, self->owner, move_vec, trace->endpos, move_vec,
					self->dmg, self->dmg, 0, MOD_P_IRONDOOM);

				// If we hit a player or a monster, kill this maceball.
				if (trace->ent->client != NULL || (trace->ent->svflags & SVF_MONSTER))
					self->dead_state = DEAD_DYING;
			}
		}
	}

	// If it's time is up, then kill it.
	if (self->touch_debounce_time <= level.time)
	{
		self->dead_state = DEAD_DYING;
		return;
	}

	gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/MaceBallBounce.wav"), 1.0f, ATTN_NORM, 0.0f);

	// Do a spiffy hit effect.
	vec3_t point;
	GetCollisionPoint(self->velocity, self->s.origin, self->maxs[0], point);
	gi.CreateEffect(NULL, FX_WEAPON_MACEBALLBOUNCE, 0, point, "d", trace->plane.normal);

	// Hit a vertical surface? Don't track a target.
	if (trace->plane.normal[2] < 0.5f)
		return;

	self->flags |= FL_NOTARGET; // This indicates to the thinker to revise the trajectory.

	// It should track its target.
	if (self->enemy == NULL || self->enemy->health <= 0)
	{
		// Find new enemy
		self->enemy = FindSpellTargetInRadius(self, MACEBALL_SEARCH_RADIUS, self->s.origin, self->mins, self->maxs);

		if (self->enemy == NULL) // No target, don't head for a target.
		{
			self->health = 1;
			return;
		}
	}

	// Since we have an enemy, set the flag to readjust next think.
	VectorCopy(trace->plane.normal, self->movedir);
}

void SpellCastMaceball(edict_t* caster, const vec3_t start_pos, const vec3_t aim_angles)
{
	edict_t* ball = G_Spawn();

	VectorCopy(start_pos, ball->s.origin);

	VectorSet(ball->mins, -MACEBALL_RADIUS, -MACEBALL_RADIUS, -MACEBALL_RADIUS);
	VectorSet(ball->maxs,  MACEBALL_RADIUS,  MACEBALL_RADIUS,  MACEBALL_RADIUS);

	GetAimVelocity(caster->enemy, start_pos, MACEBALL_SPEED, aim_angles, ball->velocity);

	// Throw the ball down.
	ball->velocity[2] = -MACEBALL_DOWNSPEED;
	VectorAdd(ball->velocity, caster->velocity, ball->velocity);

	// If the caster has an enemy, then aim it at the enemy.
	ball->enemy = caster->enemy;

	// Spin it forward.
	ball->mass = 2500;
	ball->elasticity = ELASTICITY_MACEBALL;
	ball->friction = 0.0f;
	ball->gravity = MACEBALL_GRAVITY;
	ball->svflags = SVF_DO_NO_IMPACT_DMG;
	ball->movetype = PHYSICSTYPE_STEP;
	ball->solid = SOLID_BBOX;
	ball->clipmask = MASK_MONSTERSOLID;
	ball->owner = caster;

	ball->classname = "Spell_Maceball";
	ball->touch_debounce_time = level.time + MACEBALL_LIFE; // The ball will expire the next bounce after this one.
	ball->s.modelindex = (byte)gi.modelindex("models/spells/maceball/tris.fm");
	ball->s.scale = MACEBALL_SCALE_INCREMENT;
	ball->dmg = MACEBALL_DAMAGE;
	ball->health = 2;
	ball->s.effects |= EF_MACE_ROTATE;
	VectorCopy(ball->s.origin, ball->last_org);

	ball->bounced = MaceballBounce;
	ball->think = MaceballThink;
	ball->nextthink = level.time + FRAMETIME; //mxd. Use define.

	gi.linkentity(ball);

	trace_t trace;
	gi.trace(caster->s.origin, ball->mins, ball->maxs, start_pos, caster, MASK_PLAYERSOLID, &trace);

	if (trace.startsolid)
	{
		// Spawning in something, give up now, and kill the thing.
		VectorClear(ball->velocity);

		gi.sound(ball, CHAN_WEAPON, gi.soundindex("weapons/MaceBallDeath.wav"), 2.0f, ATTN_NORM, 0.0f); //TODO: why 2.0 volume?
		gi.CreateEffect(NULL, FX_WEAPON_MACEBALLEXPLODE, 0, ball->s.origin, "d", ball->velocity);
		G_SetToFree(ball);
	}
	else
	{
		// Hit something along the way from the center of the player to here.
		if (trace.fraction < 0.99f)
		{
			VectorCopy(trace.endpos, ball->s.origin);
			VectorCopy(trace.endpos, ball->last_org);
		}

		gi.sound(caster, CHAN_WEAPON, gi.soundindex("weapons/MaceBallCast.wav"), 1.0f, ATTN_NORM, 0.0f);
	}
}