//
// g_ai.c
//
// Copyright 1998 Raven Software
//

#include "g_ai.h" //mxd
#include "g_ai_local.h" //mxd
#include "m_move.h" //mxd
#include "mg_ai.h" //mxd
#include "mg_guide.h"
#include "m_ogle.h" //mxd
#include "m_stats.h"
#include "p_anims.h"
#include "Random.h"
#include "Vector.h"

// AI targeting globals.
static int enemy_range; // Range from enemy RANGE_MELEE, RANGE_NEAR, RANGE_MID, RANGE_FAR.
static float enemy_yaw; // Ideal yaw to face enemy.

// Called once each frame to set level.sight_client to the player to be checked for in findtarget.
// If all clients are either dead or in notarget, sight_client will be null.
// In coop games, sight_client will cycle between the clients.
void AI_SetSightClient(void)
{
	const int start = ((level.sight_client == NULL) ? 1 : level.sight_client - g_edicts);
	int check = start;

	while (true)
	{
		if (++check > game.maxclients)
			check = 1;

		edict_t* ent = &g_edicts[check];

		if (ent->inuse && ent->health > 0 && !(ent->flags & FL_NOTARGET))
		{
			level.sight_client = ent;
			return; // Got one.
		}

		if (check == start)
		{
			level.sight_client = NULL;
			return; // Nobody to see.
		}
	}
}

// Monster will eat until harm is done to him or the player moves within range.
void ai_eat(edict_t* self, float dist) //TODO: used only in m_gkrokon_anim.c. Move to m_gkrokon.c as gkrokon_eat()?
{
	self->enemy = NULL;
	FindTarget(self);
}

// Move the specified distance at current facing.
void ai_move(edict_t* self, const float dist)
{
	MG_WalkMove(self, self->s.angles[YAW], dist);
}

// Used for standing around and looking for players. Distance is for slight position adjustments needed by the animations.
void ai_stand(edict_t* self, const float dist) //mxd. 'dist' is always 0.
{
	if (dist != 0.0f)
		M_walkmove(self, self->s.angles[YAW], dist);

	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
	{
		if (self->enemy != NULL) //mxd. Never executed in original logic. 
		{
			vec3_t diff;
			VectorSubtract(self->enemy->s.origin, self->s.origin, diff);
			self->ideal_yaw = VectorYaw(diff);

			if (self->s.angles[YAW] != self->ideal_yaw && (self->monsterinfo.aiflags & AI_TEMP_STAND_GROUND))
			{
				self->monsterinfo.aiflags &= ~(AI_STAND_GROUND | AI_TEMP_STAND_GROUND);
				G_PostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
			}

			M_ChangeYaw(self);
			AI_CheckAttack(self);
		}
		else
		{
			FindTarget(self);
		}

		return;
	}

	if (self->enemy != NULL || FindTarget(self)) //mxd. Moved self->enemy NULL check below AI_STAND_GROUND check.
		return;

	//FIXME: 'walking a beat' monsters, but that may not be working, need to test! Check for target also?
	if (self->monsterinfo.pausetime == -1.0f)
	{
		self->spawnflags |= MSF_WANDER;
		self->ai_mood = AI_MOOD_WANDER;
		G_PostMessage(self, MSG_CHECK_MOOD, PRI_DIRECTIVE, "i", AI_MOOD_WANDER);

		return;
	}

	if (level.time > self->monsterinfo.pausetime)
	{
		self->ai_mood = AI_MOOD_WALK;
		G_PostMessage(self, MSG_WALK, PRI_DIRECTIVE, NULL);

		return;
	}

	if (!(self->spawnflags & MSF_AMBUSH) && self->monsterinfo.idle != NULL && level.time > self->monsterinfo.idle_time)
	{
		if (self->monsterinfo.idle_time > 0.0f)
		{
			self->monsterinfo.idle(self);
			self->monsterinfo.idle_time = level.time + flrand(15.0f, 30.0f);
		}
		else
		{
			self->monsterinfo.idle_time = level.time + flrand(0.0f, 15.0f);
		}
	}
}

// The monster is walking it's beat.
void ai_walk(edict_t* self, const float dist)
{
	if (FindTarget(self))
	{
		G_PostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
		return;
	}

	if (self->monsterinfo.searchType == SEARCH_BUOY)
	{
		if (!(self->monsterinfo.aiflags & AI_STRAIGHT_TO_ENEMY))
		{
			MG_BuoyNavigate(self); // Only called if already wandering.
			MG_Pathfind(self, false);
		}
	}
	else if (self->enemy == NULL && self->pathtarget == NULL)
	{
		if (self->movetarget != NULL)
		{
			if (self->movetarget->classname != NULL && strcmp(self->movetarget->classname, "path_corner") != 0 && MG_ReachedBuoy(self, self->movetarget->s.origin))
			{
				self->movetarget = NULL;
				G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
			}
		}
		else if (irand(0, 30) == 0 || (Vec3NotZero(self->monsterinfo.nav_goal) && MG_ReachedBuoy(self, self->monsterinfo.nav_goal)))
		{
			self->monsterinfo.pausetime = 0.0f;
			G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
		}
	}

	MG_MoveToGoal(self, dist);
}

// Turns towards enemy and advances. Use this call with a distance of 0 to replace ai_face.
void ai_charge(edict_t* self, const float dist)
{
	if (self->enemy == NULL)
	{
		MG_FaceGoal(self, true); // Get ideal yaw and turn.
		return; //FIXME: send stand MSG?
	}

	vec3_t diff;
	VectorSubtract(self->enemy->s.origin, self->s.origin, diff);
	self->ideal_yaw = VectorYaw(diff);

	MG_ChangeYaw(self);

	if (!(self->spawnflags & MSF_FIXED) && dist != 0.0f)
		MG_WalkMove(self, self->s.angles[YAW], dist);
}

// Turns towards target and advances. Use this call with a distance of 0 to replace ai_face.
void ai_charge2(edict_t* self, const float dist) //mxd. Same ai_charge(), except extra MG_FaceGoal() call when no enemy...
{
	if (self->enemy == NULL)
		return;

	vec3_t diff;
	VectorSubtract(self->enemy->s.origin, self->s.origin, diff);
	self->ideal_yaw = VectorYaw(diff);

	MG_ChangeYaw(self);

	if (!(self->spawnflags & MSF_FIXED) && dist != 0.0f)
		MG_WalkMove(self, self->s.angles[YAW], dist);
}

// Simple sliding right (or left if dist < 0).
void ai_moveright(edict_t* self, const float dist)
{
	if (!(self->spawnflags & MSF_FIXED) && dist != 0.0f)
	{
		vec3_t right;
		AngleVectors(self->s.angles, NULL, right, NULL);
		MG_WalkMove(self, VectorYaw(right), dist);
	}
}

// Turns towards target and advances. Use this call with a distance of 0 to replace ai_face.
void ai_goal_charge(edict_t* self, const float dist)
{
	MG_FaceGoal(self, true);

	if (!(self->spawnflags & MSF_FIXED) && dist != 0.0f)
		MG_WalkMove(self, self->s.angles[YAW], dist);
}

static int CategorizeRange(const edict_t* self, const edict_t* other, const float len) //mxd. Named 'categorize_range' in original logic.
{
	// Eating.
	if (self->monsterinfo.aiflags & AI_EATING)
	{
		if (len < MELEE_DISTANCE && len < self->maxs[0] + other->maxs[0] + 15.0f) // Melee distance.
			return RANGE_MELEE;

		if (len < 175.0f) // Attacking distance.
			return RANGE_NEAR;

		if (len < 350.0f) // Hissing distance.
			return RANGE_MID;

		return RANGE_FAR;
	}

	// Not eating.
	if (len < MELEE_DISTANCE && len < self->maxs[0] + other->maxs[0] + 25.0f) // Melee distance.
		return RANGE_MELEE;

	if (len < 500.0f) // Attacking distance.
		return RANGE_NEAR;

	const float dist = ((self->wakeup_distance > 0.0f ? self->wakeup_distance : MAX_SIGHT_PLAYER_DIST)); //mxd
	if (len < dist) // Hissing distance.
		return RANGE_MID;

	return RANGE_FAR;
}

// Returns the range categorization of an entity relative to self:
// RANGE_MELEE (0)	- Melee range, will become hostile even if back is turned.
// RANGE_NEAR (1)	- Visible and infront, or visible and show hostile.
// RANGE_MID (2)	- Infront and show hostile.
// RANGE_FAR (3)	- Only triggered by damage.
static int GetRange(const edict_t* self, const edict_t* other) //mxd. Named 'range' in original logic.
{
	vec3_t diff;
	VectorSubtract(self->s.origin, other->s.origin, diff);
	return CategorizeRange(self, other, VectorLength(diff));
}

// Returns true if the entity is visible to self, even if not infront().
qboolean AI_IsVisible(const edict_t* self, const edict_t* other) //mxd. Named 'visible' in original logic.
{
	if (self == NULL || other == NULL)
		return false;

	vec3_t self_pos;
	VectorCopy(self->s.origin, self_pos);
	self_pos[2] += (float)self->viewheight;

	if (self->classID == CID_TBEAST)
	{
		vec3_t forward;
		AngleVectors(self->s.angles, forward, NULL, NULL);

		VectorMA(self_pos, self->maxs[0], forward, self_pos);
	}

	vec3_t other_pos;
	VectorCopy(other->s.origin, other_pos);
	other_pos[2] += (float)other->viewheight;

	trace_t trace;
	gi.trace(self_pos, vec3_origin, vec3_origin, other_pos, self, MASK_OPAQUE, &trace);

	return (trace.fraction == 1.0f);
}

// Returns true if the entity is visible, but not through transparencies.
qboolean AI_IsClearlyVisible(const edict_t* self, const edict_t* other) //mxd. Named 'clear_visible' in original logic.
{
	if (self == NULL || other == NULL)
		return false;

	vec3_t self_pos;
	VectorCopy(self->s.origin, self_pos);
	self_pos[2] += (float)self->viewheight;

	vec3_t other_pos;
	VectorCopy(other->s.origin, other_pos);
	other_pos[2] += (float)other->viewheight;

	trace_t trace;
	gi.trace(self_pos, vec3_origin, vec3_origin, other_pos, self, MASK_SOLID, &trace);

	return (trace.fraction == 1.0f);
}

// Returns true if the entity is in front (in sight) of self.
qboolean AI_IsInfrontOf(const edict_t* self, const edict_t* other) //mxd. Named 'infront' in original logic.
{
	vec3_t check_angles;

	if (Vec3NotZero(self->v_angle_ofs))
		VectorAdd(self->v_angle_ofs, self->s.angles, check_angles);
	else
		VectorCopy(self->s.angles, check_angles);

	vec3_t forward;
	AngleVectors(check_angles, forward, NULL, NULL);

	vec3_t diff;
	VectorSubtract(other->s.origin, self->s.origin, diff);
	VectorNormalize(diff);

	return (DotProduct(diff, forward) > 0.3f);
}

// Checks and see if an alert entity is capable of waking up a monster.
static qboolean AI_IsAlerted(edict_t* self) //mxd. Named 'Alerted' in original logic.
{
	// This alert entity wakes up monsters, let's see what's up...
	if ((self->monsterinfo.aiflags & AI_NO_ALERT) || self->enemy != NULL || self->monsterinfo.alert == NULL) //mxd. Moved alert() check outside the loop.
		return false;

	// Get my view spot. //mxd. Moved outside the loop.
	vec3_t view_pos;
	VectorCopy(self->s.origin, view_pos);
	view_pos[2] += (float)self->viewheight;

	// Start the search from the most recent alert to the oldest.
	alertent_t* alerter = level.last_alert; // OOPS, SKIPS LAST.
	qboolean first_step = true; //mxd

	// The loop.
	while (true)
	{
		if (first_step)
			first_step = false;
		else if (alerter != NULL)
			alerter = alerter->prev_alert;

		if (alerter == NULL)
			return false;

		// Loading a saved game invalidates all alerts.
		if (!alerter->inuse)
			continue;

		// Alert timed out, remove from list.
		if (alerter->lifetime < level.time)
		{
			AlertTimedOut(alerter);
			continue;
		}

		if (self->last_alert != NULL)
		{
			// Don't be woken up by the same alert twice.
			if (self->last_alert == alerter)
				continue;

			self->last_alert = NULL;
		}

		// Alerter's enemy is gone.
		if (alerter->enemy == NULL)
			continue;

		// No alerts for notarget players.
		if (alerter->enemy->client != NULL && (alerter->enemy->flags & FL_NOTARGET))
			continue;

		if (!(self->svflags & SVF_MONSTER) || self->health <= 0)
			continue;

		// Eating or in a cinematic or not awake, leave them alone.
		if (!AI_OkToWake(self, false, true))
			continue;

		vec3_t dir;
		VectorSubtract(self->s.origin, alerter->origin, dir);
		const float dist = VectorLength(dir);

		// If monster's wakeup_distance is shorter than distance to alerter, leave it alone.
		if (dist > self->wakeup_distance)
			continue;

		// Closer means better chance to alert. Problem: different alerts might be more likely to be heard/seen...
		if (dist > flrand(100.0f, self->wakeup_distance)) // If within 100 always wake up?
			continue;

		// If not a player, a player's missile or a monster, continue loop?

		// Get center of alert enemy.
		edict_t* enemy = alerter->enemy;

		vec3_t enemy_pos;
		VectorAdd(enemy->s.origin, enemy->mins, enemy_pos);
		VectorMA(enemy_pos, 0.5f, enemy->size, enemy_pos);

		// If being alerted by a monster and not waiting to ambush.
		if ((alerter->alert_svflags & SVF_MONSTER) && !(self->spawnflags & MSF_AMBUSH))
		{
			// Can "see" the owner of the alert even around a corner.
			if (!gi.inPVS(enemy_pos, view_pos))
				continue;
		}
		else
		{
			// No line of sight and not an ambush monster.
			if (!MG_IsVisiblePos(self, enemy_pos) && !(self->spawnflags & MSF_AMBUSH) && gi.inPVS(view_pos, alerter->origin))
				if (irand(0, 3) != 0) // 25% chance will see impact (alerter) and detect alert owner anyway.
					continue;
		}

		self->last_alert = alerter;

		// Break the loop.
		return self->monsterinfo.alert(self, alerter, enemy);
	}
}

// A target has been found, it is visible, so do we attack it, watch it, or stand there?
static void HuntTarget(edict_t* self)
{
	self->goalentity = self->enemy;

	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
	{
		G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
	}
	else
	{
		const int r = GetRange(self, self->enemy);
		const G_MsgID_t id = (((self->monsterinfo.aiflags & AI_EATING) && r == RANGE_MID) ? MSG_WATCH : MSG_RUN); //mxd. //TODO: range check is strange. Should be 'r >= RANGE_MID'?
		G_PostMessage(self, id, PRI_DIRECTIVE, NULL);
	}

	vec3_t diff;
	VectorSubtract(self->enemy->s.origin, self->s.origin, diff);
	self->ideal_yaw = VectorYaw(diff);

	// Wait a while before first attack.
	if (!(self->monsterinfo.aiflags & AI_STAND_GROUND))
		self->monsterinfo.attack_finished = level.time + 1.0f; //mxd. Inline AttackFinished().
}

static void PlaySightSound(edict_t* self) //mxd. Added to reduce code duplication.
{
	if (self->oldenemy == NULL)
	{
		const byte snd_type = ((self->monsterinfo.aiflags & AI_SOUND_TARGET) ? SIGHT_VISIBLE_TARGET : SIGHT_SOUND_TARGET); //mxd. //TODO: logic should be inverted?
		G_PostMessage(self, MSG_VOICE_SIGHT, PRI_DIRECTIVE, "be", snd_type, self->enemy);
	}

	self->spawnflags &= ~MSF_AMBUSH;
}

// A target has been found, let other monsters know this. Then hunt it.
void AI_FoundTarget(edict_t* self, const qboolean set_sight_ent) //mxd. Named 'FoundTarget' in original logic.
{
	// Let other monsters see this monster for a while.
	if (self->enemy == NULL)
		return;

	if (self->classID == CID_OGLE)
		self->spawnflags = 0; //TODO: what does this do?

	self->monsterinfo.awake = true;
	self->spawnflags &= ~MSF_AMBUSH;
	self->targetname = NULL;
	self->monsterinfo.pausetime = -1.0f;

	if (self->wakeup_target != NULL)
	{
		char* save_target = self->target;
		self->target = self->wakeup_target;
		G_UseTargets(self, self->enemy);
		self->target = save_target;
		self->wakeup_target = NULL;
	}

	if (set_sight_ent && self->enemy->client != NULL)
	{
		level.sight_entity = self;
		level.sight_entity_framenum = level.framenum;
		level.sight_entity->light_level = 128;
	}

	self->show_hostile = level.time + 1.0f; // Wake up other monsters.
	VectorCopy(self->enemy->s.origin, self->monsterinfo.last_sighting);

	if (self->combattarget == NULL) // Not going for a combat point?
	{
		// Don't want to do this if we are a fish.
		if (self->classID != CID_FISH)
			HuntTarget(self);

		PlaySightSound(self); //mxd

		return;
	}

	self->goalentity = G_PickTarget(self->combattarget);
	self->movetarget = self->goalentity;

	if (self->movetarget == NULL)
	{
		self->goalentity = self->enemy;
		self->movetarget = self->enemy;

		// Don't want to do this if we are a fish.
		if (self->classID != CID_FISH)
			HuntTarget(self);

		PlaySightSound(self); //mxd

		return;
	}

	// Clear out our combattarget, these are a one shot deal.
	self->combattarget = NULL;
	self->monsterinfo.aiflags |= AI_COMBAT_POINT;

	// Clear the targetname, that point is ours!
	self->movetarget->targetname = NULL;

	// Run for it, assuming we aren't a fish.
	if (self->classID != CID_FISH)
		G_PostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);

	PlaySightSound(self); //mxd
}

// Can this monster be woken up by something other than direct line of sight to player?
qboolean AI_OkToWake(const edict_t* monster, const qboolean gorgon_roar, const qboolean ignore_ambush) //mxd. Named 'ok_to_wake' in original logic.
{
	if (gorgon_roar)
		return !monster->monsterinfo.c_mode;

	// When targetname is set: a monster that's supposed to be triggered - problem, one a monster is used and woken up, won't respond to alerts like others...?
	if ((monster->monsterinfo.aiflags & AI_EATING) || monster->targetname != NULL ||
		monster->monsterinfo.c_mode || (monster->spawnflags & MSF_ASLEEP) || (monster->spawnflags & MSF_AMBUSH && !ignore_ambush))
		return false;

	return true;
}

static qboolean PlayerIsCreeping(const playerinfo_t* info) //mxd. Named 'PlayerCreeping' in original logic.
{
	static const int creep_seq[] = //mxd
	{
		ASEQ_STAND,
		ASEQ_CREEPF, ASEQ_CREEPB, ASEQ_CREEPB_END,
		ASEQ_CROUCH_GO, ASEQ_CROUCH, ASEQ_CROUCH_END,
		ASEQ_CROUCH_WALK_F, ASEQ_CROUCH_WALK_B,
		ASEQ_CROUCH_WALK_L, ASEQ_CROUCH_WALK_R
	};

	for (uint i = 0; i < ARRAY_SIZE(creep_seq); i++)
		if (info->upperseq == creep_seq[i] || info->lowerseq == creep_seq[i])
			return true;

	return false;
}

// Self is currently not attacking anything, so try to find a target.
// Returns TRUE if an enemy was sighted.
// When a player fires a missile or does other things to make noise, the point of impact becomes an alertent
// so that monsters that see the impact will respond as if they had seen the player.
// Since FindTarget is not called every frame for monsters (average about once every 3 frames per monster), this does two potential checks.
// First it checks against the current sight_client which cycles through the players.
// If that check fails, it will check for all the secondary alerts and enemies.
// If it can't find any, it will check for another player.
qboolean FindTarget(edict_t* self)
{
	//FIXME: wakeup_distance -1 never look?
	if (self->classID == CID_OGLE)
		return OgleFindTarget(self);

	if (self->monsterinfo.aiflags & AI_GOOD_GUY)
		return false; //FIXME: look for monsters? //mxd. Skip unnecessary self->goalentity->classname == "target_actor" check.

	// If we're going to a combat point, just proceed.
	if (self->monsterinfo.aiflags & AI_COMBAT_POINT)
		return false;

	// Being forced to use buoys, and ignore enemy until get to forced_buoy.
	if (self->ai_mood_flags & AI_MOOD_FLAG_IGNORE_ENEMY)
		return false;

	// If the first spawnflag bit is set, the monster will only wake up on really seeing the player,
	// not another monster getting angry or hearing something.
	// Revised behavior so they will wake up if they "see" a player make a noise, but not weapon impact/explosion noises.

	const edict_t* first_client = NULL;

	// The loop.
	for (int i = 0; i < 2; i++)
	{
		edict_t* client;
		qboolean is_primary_enemy = true;

		if (i == 0)
		{
			// Look only at the level.sight_client.
			first_client = level.sight_client;
			client = level.sight_client;
		}
		else
		{
			if (ANARCHY)
			{
				// Crazy monsters mode.
				int	check_count = 0;
				client = self;

				while ((client == NULL || !client->inuse || !(client->svflags & SVF_MONSTER) || client->health <= 0 || client == self) && check_count < globals.num_edicts)
				{
					client = &g_edicts[irand(0, globals.num_edicts)];
					check_count++;
				}
			}
			else
			{
				if (level.sight_entity == self)
				{
					level.sight_entity = NULL;
					return false;
				}

				if (level.sight_entity != NULL && level.sight_entity_framenum >= level.framenum - 1 && !(self->spawnflags & MSF_AMBUSH))
				{
					// Go after the enemy another monster saw saw, but only if not in ambush.
					client = level.sight_entity;

					if (client->enemy == self->enemy)
						return false;
				}
				else if (AI_IsAlerted(self))
				{
					return true; // Picked up an enemy from an alert.
				}
				else
				{
					client = NULL;

					// Looking for secondary enemies.
					if (self->monsterinfo.otherenemyname != NULL && self->monsterinfo.chase_finished < level.time)
					{
						edict_t* ent = NULL;
						while ((ent = FindInRadius(ent, self->s.origin, 175.0f)) != NULL)
						{
							if (strcmp(ent->classname, self->monsterinfo.otherenemyname) == 0 && ent != self)
							{
								is_primary_enemy = false;
								client = ent;

								break;
							}
						}
					}

					//  Look at the sight client.
					if (client == NULL)
					{
						// Found no non-clients, cycle to next client and check it for second check.
						AI_SetSightClient();

						if (first_client == level.sight_client)
							return false; // Same as first check, and that failed if we're here, so return.

						client = level.sight_client;
					}
				}
			}
		}

		if (client == NULL || !client->inuse || client == self)
			continue; // No clients to get mad at.

		if (client == self->enemy)
			return true;

		if (self->monsterinfo.otherenemyname != NULL && strcmp(client->classname, self->monsterinfo.otherenemyname) == 0)
			client->light_level = 128; // Let it be seen.

		// If we are a fish - is the target in the water - have to be at least waist deep?
		if (self->classID == CID_FISH && client->waterlevel < 2)
			continue; //???

		if (client->client != NULL)
		{
			if (client->flags & FL_NOTARGET)
				continue;
		}
		else if (client->svflags & SVF_MONSTER)
		{
			// Not a secondary enemy.
			if (is_primary_enemy)
			{
				if (!ANARCHY)
				{
					// Eating or in a cinematic or not awake or targeted, leave them alone.
					if (AI_OkToWake(self, false, true))
						continue;

					if (client->enemy == NULL || client->enemy->health < 0)
						continue;
				}

				if (client->enemy != NULL && (client->enemy->flags & FL_NOTARGET))
					continue;

				if (!AI_IsVisible(self, client))
					continue;

				self->enemy = (ANARCHY ? client : client->enemy);
				AI_FoundTarget(self, client->ai_mood != AI_FLEE); // When AI_FLEE, let them stay the sight entity; otherwise make me the sight entity. 

				return true;
			}
		}
		else
		{
			continue;
		}

		const qboolean enemy_infront = ((self->classID == CID_ASSASSIN) ? true : AI_IsInfrontOf(self, client)); //mxd

		if (!enemy_infront && client->client != NULL && PlayerIsCreeping(&client->client->playerinfo))
			continue;

		vec3_t diff;
		VectorSubtract(client->s.origin, self->s.origin, diff);
		const float dist = VectorLength(diff);

		if (dist > self->wakeup_distance)
			continue;

		const int r = CategorizeRange(self, client, dist);

		if (r == RANGE_FAR)
		{
			if (self->monsterinfo.aiflags & AI_EATING) //mxd. Never executed in original logic.
				self->enemy = client;

			continue;
		}

		// This is where we would check invisibility.

		// Is client in an spot too dark to be seen?
		if (client->light_level <= 5)
			continue;

		if ((self->svflags & SVF_MONSTER) && client->client != NULL)
			if (SKILL < SKILL_HARD && !(self->monsterinfo.aiflags & AI_NIGHTVISION) && client->light_level < irand(6, 77))
				continue;

		if (r == RANGE_NEAR)
		{
			if (!enemy_infront && client->show_hostile < level.time)
				continue;
		}
		else if (r == RANGE_MID)
		{
			if (!enemy_infront)
				continue;
		}

		//sfs -- This check is much less trivial than infront: probably wasn't a noticeable deal, since RANGE_FAR was first rejection check,
		// but it's still better to try the dotproduct before the traceline.
		if (!AI_IsVisible(self, client))
			continue;

		self->enemy = client;
		is_primary_enemy = true;


		if (self->monsterinfo.otherenemyname != NULL && strcmp(self->enemy->classname, self->monsterinfo.otherenemyname) == 0)
		{
			// This is a secondary enemy.
			self->monsterinfo.chase_finished = level.time + 15.0f;
			is_primary_enemy = false;
		}

		if (is_primary_enemy) // This is not a secondary enemy.
		{
			self->monsterinfo.aiflags &= ~AI_SOUND_TARGET;

			if (self->enemy->client == NULL)
			{
				self->enemy = self->enemy->enemy;

				if (self->enemy->client == NULL)
				{
					self->enemy = NULL;
					continue;
				}
			}
		}

		// Got one.
		AI_FoundTarget(self, true);

		// Break the loop.
		return true;
	}

	return false;
}

static qboolean FacingIdeal(const edict_t* self)
{
	const float delta = anglemod(self->s.angles[YAW] - self->ideal_yaw);
	return (delta <= 45.0f || delta >= 315.0f);
}

qboolean M_CheckAttack(edict_t* self) //TODO: move to g_monster.c, make static.
{
	if (self->enemy->health > 0)
	{
		// See if any entities are in the way of the shot.
		vec3_t self_pos;
		VectorCopy(self->s.origin, self_pos);
		self_pos[2] += (float)self->viewheight;

		vec3_t enemy_pos;
		VectorCopy(self->enemy->s.origin, enemy_pos);
		enemy_pos[2] += (float)self->enemy->viewheight;

		trace_t trace;
		gi.trace(self_pos, vec3_origin, vec3_origin, enemy_pos, self, CONTENTS_SOLID | CONTENTS_MONSTER | CONTENTS_SLIME | CONTENTS_LAVA, &trace);

		// Do we have a clear shot?
		if (trace.ent != self->enemy)
			return false;
	}

	// Melee attack.
	if (enemy_range == RANGE_MELEE)
	{
		// Don't always melee in easy mode.
		if (SKILL == SKILL_EASY && irand(0, 3) != 0)
			return false;

		self->monsterinfo.attack_state = ((classStatics[self->classID].msgReceivers[MSG_MELEE] != NULL) ? AS_MELEE : AS_MISSILE);
		return true;
	}

	// Missile attack.
	if (classStatics[self->classID].msgReceivers[MSG_MISSILE] == NULL || level.time < self->monsterinfo.attack_finished || enemy_range == RANGE_FAR)
		return false;

	float chance;

	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		chance = 0.4f;
	else if (enemy_range == RANGE_NEAR)
		chance = 0.1f;
	else if (enemy_range == RANGE_MID)
		chance = 0.02f;
	else
		return false;

	if (SKILL == SKILL_EASY)
		chance *= 0.5f;
	else if (SKILL >= SKILL_HARD)
		chance *= 2.0f;

	if (flrand(0.0f, 1.0f) < chance)
	{
		self->monsterinfo.attack_state = AS_MISSILE;
		self->monsterinfo.attack_finished = level.time + flrand(0.0f, 2.0f);

		return true;
	}

	if (self->flags & FL_FLY)
		self->monsterinfo.attack_state = (irand(0, 2) ? AS_STRAIGHT : AS_SLIDING); // 66% chance of AS_STRAIGHT. // TODO: scale by skill?

	return false;
}

// Turn and close until within an angle to launch a melee attack.
static void ai_run_melee(edict_t* self) //mxd. Never used as action function.
{
	self->ideal_yaw = enemy_yaw;
	M_ChangeYaw(self);

	if (FacingIdeal(self))
	{
		G_PostMessage(self, MSG_MELEE, PRI_DIRECTIVE, NULL);
		self->monsterinfo.attack_state = AS_STRAIGHT;
	}
}

// Turn in place until within an angle to launch a missile attack.
static void ai_run_missile(edict_t* self) //mxd. Never used as action function.
{
	self->ideal_yaw = enemy_yaw;
	M_ChangeYaw(self);

	if (FacingIdeal(self))
	{
		G_PostMessage(self, MSG_MISSILE, PRI_DIRECTIVE, NULL);
		self->monsterinfo.attack_state = AS_STRAIGHT;
	}
}

// Decides if we're going to attack or do something else. Used by ai_run, and ai_stand.
static qboolean AI_CheckAttack(edict_t* self) //mxd. Removed unused 'dist' arg. Named 'ai_checkattack' in original logic.
{
	if ((self->monsterinfo.aiflags & AI_FLEE) || (self->monsterinfo.aiflags & AI_COWARD)) // He's running away, not attacking.
		return false;

	// See if the enemy is dead.
	qboolean enemy_dead = false;

	if (self->enemy == NULL || !self->enemy->inuse || self->enemy->health <= ((self->monsterinfo.aiflags & AI_BRUTAL) ? -80 : 0))
		enemy_dead = true;

	if (enemy_dead)
	{
		self->enemy = NULL;

		// FIXME: look all around for other targets.
		if (self->oldenemy != NULL && self->oldenemy->health > 0)
		{
			self->enemy = self->oldenemy;
			self->oldenemy = NULL;
			HuntTarget(self);
		}
		else
		{
			if (self->movetarget != NULL)
			{
				self->goalentity = self->movetarget;
				G_PostMessage(self, MSG_WALK, PRI_DIRECTIVE, NULL);
			}
			else
			{
				// We need the pausetime, otherwise the stand code will just revert to walking with no target
				// and the monsters will wonder around aimlessly trying to hunt the world entity.
				self->monsterinfo.pausetime = level.time + 100000000.0f;
				G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
			}

			return true;
		}
	}

	self->show_hostile = level.time + 1.0f; // Wake up other monsters.

	// Check knowledge of enemy.
	const qboolean enemy_vis = AI_IsClearlyVisible(self, self->enemy);

	if (enemy_vis)
	{
		self->monsterinfo.search_time = level.time + 5.0f;
		VectorCopy(self->enemy->s.origin, self->monsterinfo.last_sighting);
	}

	enemy_range = GetRange(self, self->enemy);

	vec3_t diff;
	VectorSubtract(self->enemy->s.origin, self->s.origin, diff);
	enemy_yaw = VectorYaw(diff);

	if (self->monsterinfo.attack_state == AS_MISSILE)
	{
		ai_run_missile(self);
		return true;
	}

	if (self->monsterinfo.attack_state == AS_MELEE)
	{
		ai_run_melee(self);
		return true;
	}

	// If enemy is not currently visible, we will never attack.
	if (enemy_vis)
		return self->monsterinfo.checkattack(self);

	return false;
}

float AI_FaceGoal(edict_t* self) //mxd. Named 'ai_face_goal' in original logic.
{
	if (self->monsterinfo.idle_time >= level.time)
		return false;

	vec3_t diff;

	if (self->monsterinfo.searchType == SEARCH_BUOY)
		VectorSubtract(self->monsterinfo.nav_goal, self->s.origin, diff);
	else if (self->goalentity != NULL)
		VectorSubtract(self->goalentity->s.origin, self->s.origin, diff);
	else if (self->enemy != NULL)
		VectorSubtract(self->enemy->s.origin, self->s.origin, diff);
	else
		return false;

	self->ideal_yaw = VectorYaw(diff);
	return M_ChangeYaw(self);
}

// The monster has an enemy it is trying to get away from.
void AI_Flee(edict_t* self, const float dist) //mxd. Named 'ai_flee' in original logic.
{
	if (self->enemy != NULL)
	{
		vec3_t diff;
		VectorSubtract(self->enemy->s.origin, self->s.origin, diff);
		self->ideal_yaw = VectorYaw(diff);
		self->ideal_yaw = anglemod(self->ideal_yaw + self->best_move_yaw);
		M_ChangeYaw(self);

		if (!M_walkmove(self, self->s.angles[YAW], dist) && AnglesEqual(self->s.angles[YAW], self->ideal_yaw, 5.0f))
			self->best_move_yaw = flrand(60.0f, 300.0f);
		else
			self->best_move_yaw = 180.0f;
	}
}

// Estimates where the "target" will be by the time a projectile traveling at "proj_speed" leaving "origin" arrives at "target"'s origin.
// It then calculates a new spot to shoot at so that the projectile will arrive at such spot at the same time as "target".
// Will set "out_pos" to '0 0 0' if there is not a clear line of fire to the spot or if the new vector is out of the acceptable range
// (based on dot product of original vec and the new vec).
// PROPOSAL: Factor in skill->value? 0 = no leading, 4 = perfect leading, 1-3 inaccuracy levels for leading.
void ExtrapolateFireDirection(const edict_t* self, const vec3_t origin, const float proj_speed, const edict_t* target, const float accepted_dot, vec3_t out_pos) //mxd. Named 'extrapolateFiredir' in original logic.
{
	if (target == NULL)
	{
		VectorClear(out_pos);
		return;
	}

	const float targ_dist = vhlen(target->s.origin, self->s.origin);

	if (SKILL == SKILL_EASY)
	{
		// Poor shot, take forward and screw it up.
		vec3_t forward;
		AngleVectors(self->s.angles, forward, NULL, NULL);

		vec3_t expected_pos;
		VectorMA(origin, proj_speed, forward, expected_pos);

		float offset = 48.0f;
		if (targ_dist < 128.0f)
			offset *= targ_dist / 128.0f;

		expected_pos[0] += flrand(-offset, offset);
		expected_pos[1] += flrand(-offset, offset);
		expected_pos[2] += flrand(-offset / 2.0f, offset * 0.666f);

		VectorSubtract(expected_pos, origin, out_pos);
		VectorNormalize(out_pos);

		return;
	}

	float offset = 2.0f - skill->value; // skill >= 2 - perfect aim, skill 1 is very poor.

	if (targ_dist < 128.0f)
		offset *= targ_dist / 128.0f;

	offset = max(0.0f, offset);

	// Modify by player's light level?
	if (SKILL < SKILL_HARD && !(self->monsterinfo.aiflags & AI_NIGHTVISION) && target->client != NULL)
		offset += (float)(target->light_level / 32);

	vec3_t target_pos;
	VectorCopy(target->s.origin, target_pos);

	if (offset > 0.0f)
	{
		target_pos[0] += flrand(-offset * 12.0f, offset * 12.0f);
		target_pos[1] += flrand(-offset * 12.0f, offset * 12.0f);
		target_pos[2] += flrand(-offset * 8.0f,  offset * 8.0f);
	}

	vec3_t diff;
	VectorSubtract(target_pos, origin, diff);

	const float targ_dist1 = VectorNormalize(diff);

	vec3_t targ_vel;
	VectorCopy(target->velocity, targ_vel);

	const float targ_speed = VectorNormalize(targ_vel);
	const float eta1 = targ_dist1 / proj_speed; // Estimated time of arrival of projectile to target_pos.

	vec3_t expected_pos;
	VectorMA(target_pos, targ_speed * eta1, targ_vel, expected_pos);

	vec3_t targ_dir;
	VectorSubtract(expected_pos, origin, targ_dir);

	const float dist2 = VectorNormalize(targ_dir);
	const float eta2 = dist2 / proj_speed; // Estimated time of arrival of projectile to expected_pos.
	const float eta_delta = eta2 - eta1; // Change in ETA's.

	VectorMA(expected_pos, targ_speed * eta_delta * flrand(0.0f, 1.0f), targ_vel, expected_pos);
	// Careful, above version does not modify targ_vel.

	trace_t trace;
	gi.trace(origin, vec3_origin, vec3_origin, expected_pos, self, MASK_SOLID, &trace);

	if (trace.fraction < 1.0f || DotProduct(diff, out_pos) < accepted_dot) // Change if dir too great.
	{
		VectorClear(out_pos);
	}
	else
	{
		VectorSubtract(expected_pos, origin, out_pos);
		VectorNormalize(out_pos);
	}
}

// Takes last alerts away from any monster who's last alert was the "self" alert.
static void ClearLastAlerts(const alertent_t* self)
{
	edict_t* ent = &g_edicts[0];
	for (int i = 0; i < game.maxentities; i++, ent++)
		if ((ent->svflags & SVF_MONSTER) && ent->last_alert == self)
			ent->last_alert = NULL;
}

// Clears out all the fields for an alert, removes it from the linked list and sets it's slot to being empty to make room for insertion of others later.
static void AlertTimedOut(alertent_t* self) //mxd. Named 'alert_timed_out' in original logic.
{
	if (self == NULL)
		return;

	// Take self out of alert chain.
	if (self->prev_alert != NULL)
	{
		if (self->next_alert != NULL)
		{
			self->prev_alert->next_alert = self->next_alert;
		}
		else
		{
			// I'm the last one!
			level.last_alert = self->prev_alert;
			self->prev_alert->next_alert = NULL;
		}
	}
	else
	{
		// I'm the first one!
		if (self->next_alert != NULL)
		{
			level.alert_entity = self->next_alert;
		}
		else
		{
			level.last_alert = NULL;
			level.alert_entity = NULL;
		}
	}

	// Clear out all fields.
	self->next_alert = NULL;
	self->prev_alert = NULL;
	self->enemy = NULL;
	VectorClear(self->origin);
	self->alert_svflags = 0;
	self->lifetime = 0.0f;
	self->inuse = false;
	ClearLastAlerts(self);

	level.num_alert_ents--;
}

// Returns the first alert in the level.alertents list that isn't in use.
static alertent_t* GetFirstEmptyAlertInList(void)
{
	// Have max number of alerts, remove the first one.
	if (level.num_alert_ents >= MAX_ALERT_ENTS)
		AlertTimedOut(level.alert_entity);

	for (int i = 0; i < MAX_ALERT_ENTS; i++)
		if (!level.alertents[i].inuse)
			return &level.alertents[i];

	return NULL;
}

static alertent_t* GetEmptyAlert(void) //mxd. Added to reduce code duplication.
{
	alertent_t* alerter = level.alert_entity;
	alertent_t* last_alert = NULL;

	// Stick into the level's alerter chain.
	if (alerter != NULL)
	{
		// Go down the list and find an empty slot.	//FIXME: just store the entnum?
		while (alerter->next_alert != NULL)
		{
			last_alert = alerter;
			alerter = alerter->next_alert;
		}

		alerter = GetFirstEmptyAlertInList();
		level.last_alert = alerter;
	}
	else // We're the first one!
	{
		alerter = GetFirstEmptyAlertInList();
		level.last_alert = alerter;
		level.alert_entity = alerter;
	}

	if (alerter == NULL)
		return NULL; // Out of alerts and can't find any empty slots.

	alerter->inuse = true; // I'm active, don't let my slot be used until I'm freed.
	alerter->prev_alert = last_alert; // Point to the previous alerter, if any.

	// Make the previous alerter point to me as the next one.
	if (alerter->prev_alert != NULL)
		alerter->prev_alert->next_alert = alerter;

	level.num_alert_ents++;

	return alerter;
}

// Allots an alertent monsters will check during FindTarget to see if they should be alerted by it.
// self				- Used for sv_flags info and positioning of the alert entity.
// enemy			- Entity to make the monsters mad at if they're alerted.
// lifetime			- How many seconds the alert exists for.
// ignore_shadows	- This alert gives away enemy's position, even if he is in shadows
//					  (I use this for staff hits on the floor and any other effect the player makes at his own location
//					  (like shooting a weapon), not for projectiles impacting).
void AlertMonsters(const edict_t* self, edict_t* enemy, float lifetime, const qboolean ignore_shadows)
{
	if (DEATHMATCH) // Don't need this if no monsters...
		return;

	alertent_t* alerter = GetEmptyAlert(); //mxd

	if (alerter != NULL)
	{
		VectorCopy(self->s.origin, alerter->origin); // Put me in the "self"'s spot.
		alerter->enemy = enemy;
		alerter->alert_svflags = self->svflags;

		// Whatever happened would give away enemy's position, even in shadows.
		if (ignore_shadows)
			alerter->alert_svflags |= SVF_ALERT_NO_SHADE;

		// Stick around until after this point in time.
		if (lifetime == 0.0f)
			lifetime = 1.0f; // Stick around for at least 1 second.

		alerter->lifetime = level.time + lifetime;
	}
}

//mxd. AlertMonsters() variant for hitscan weapons...
// Only relevant SVFlags are:
//	- SVF_MONSTER (alert is created by monster);
//	- SVF_ALERT_NO_SHADE (give away enemy's position, even in shadows).
void AlertMonstersAt(const vec3_t alert_origin, edict_t* enemy, float lifetime, const int alert_svflags)
{
	if (DEATHMATCH) // Don't need this if no monsters...
		return;

	alertent_t* alerter = GetEmptyAlert(); //mxd

	if (alerter != NULL)
	{
		VectorCopy(alert_origin, alerter->origin);
		alerter->enemy = enemy;
		alerter->alert_svflags = alert_svflags;

		// Stick around until after this point in time.
		if (lifetime == 0.0f)
			lifetime = 1.0f; // Stick around for at least 1 second.

		alerter->lifetime = level.time + lifetime;
	}
}

void ai_spin(edict_t* self, const float amount)
{
	self->s.angles[YAW] += amount;
}

qboolean AI_HaveEnemy(edict_t* self) //mxd. Named 'ai_have_enemy' in original logic.
{
	if (self->enemy != NULL && self->enemy->health > 0)
		return true;

	if (self->oldenemy != NULL && self->oldenemy->health > 0)
	{
		self->enemy = self->oldenemy;
		self->oldenemy = NULL;

		return true;
	}

	G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
	return false;
}

qboolean AI_IsMovable(const edict_t* ent) //mxd. Named 'movable' in original logic.
{
	return (ent->movetype != PHYSICSTYPE_NONE && ent->movetype != PHYSICSTYPE_PUSH);
}