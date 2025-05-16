//
// m_imp.c
//
// Copyright 1998 Raven Software
//

#include "m_imp.h"
#include "m_imp_shared.h"
#include "g_debris.h" //mxd
#include "g_DefaultMessageHandler.h"
#include "g_monster.h"
#include "g_playstats.h"
#include "m_stats.h"
#include "Random.h"
#include "Utilities.h"
#include "Vector.h"
#include "Player.h" //mxd
#include "g_local.h"

#define IMP_CHECK_DIST		128.0f
#define IMP_MIN_SWOOP_DIST	128.0f //mxd. Named 'IMP_MIN_SSWOOP_DIST' in original logic.
#define IMP_MIN_HOVER_DIST	128.0f
#define IMP_MAX_HOVER_DIST	512.0f

#define IMP_DRIFT_AMOUNT_X	128.0f
#define IMP_DRIFT_AMOUNT_Y	128.0f
#define IMP_DRIFT_AMOUNT_Z	64.0f

#define IMP_PROJECTILE_SEARCH_RADIUS	1024.0f //mxd. Named 'IMP_PROJECTILE_RADIUS' in original logic.

#pragma region ========================== Imp base info ==========================

static const animmove_t* animations[NUM_ANIMS] =
{
	&imp_move_die1,
	&imp_move_fly1,
	&imp_move_flyback,
	&imp_move_hover1,
	&imp_move_fireball,
	&imp_move_dive_go,
	&imp_move_dive_loop,
	&imp_move_dive_end,
	&imp_move_dive_out,
	&imp_move_pain1,
	&imp_move_tumble,
	&imp_move_perch,
	&imp_move_takeoff,
	&imp_move_dup,
	&imp_move_ddown,
};

static int sounds[NUM_SOUNDS];

static const vec3_t dead_imp_mins = { -16.0f, -16.0f, 0.0f }; //mxd
static const vec3_t dead_imp_maxs = {  16.0f,  16.0f, 16.0f }; //mxd

#pragma endregion

#pragma region ========================== Utility functions =========================

static qboolean ImpCanMove(const edict_t* self, const float dist) //mxd. Named 'imp_check_move' in original logic. //TODO: very similar to HarpyCanMove(). Move to m_move.c as M_FlyMonsterCanMove().
{
	vec3_t end_pos;
	VectorCopy(self->s.origin, end_pos);

	vec3_t forward;
	AngleVectors(self->s.angles, forward, NULL, NULL);
	VectorMA(end_pos, dist, forward, end_pos);

	trace_t trace;
	gi.trace(self->s.origin, self->mins, self->maxs, end_pos, self, MASK_SHOT | MASK_WATER, &trace);

	if (trace.fraction < 1.0f && trace.ent != self->enemy) //TODO: HarpyCanMove() also checks trace.allsolid and trace.startsolid here.
		return false;

	return true;
}

// Replaces ai_walk and ai_run for imp. //TODO: logic is identical to harpy_ai_glide(). Move to g_ai.c as ai_glide()?
static void ImpAIGlide(edict_t* self) //mxd. Named 'imp_ai_glide' in original logic.
{
	if (self->enemy == NULL)
		return;

	// Find our ideal yaw to the player and correct to it.
	vec3_t diff;
	VectorSubtract(self->enemy->s.origin, self->s.origin, diff);

	vec3_t dir;
	VectorCopy(diff, dir);
	VectorNormalize(dir);

	vec3_t forward;
	AngleVectors(self->s.angles, forward, NULL, NULL);

	const float dot = DotProduct(forward, dir);

	self->ideal_yaw = VectorYaw(diff);
	M_ChangeYaw(self);

	const float yaw_delta = self->ideal_yaw - self->s.angles[YAW];

	// If enough, roll the creature to simulate gliding.
	if (Q_fabs(yaw_delta) > self->yaw_speed)
	{
		const float roll = yaw_delta / 4.0f * Q_signf(dot);
		self->s.angles[ROLL] += roll;

		// Going right?
		if (roll > 0.0f)
			self->s.angles[ROLL] = min(65.0f, self->s.angles[ROLL]);
		else
			self->s.angles[ROLL] = max(-65.0f, self->s.angles[ROLL]);
	}
	else
	{
		self->s.angles[ROLL] *= 0.75f;
	}
}

//TODO: identical to HarpyCheckDirections().
static qboolean ImpCheckDirections(const edict_t* self, const vec3_t goal, const vec3_t forward, const vec3_t right, const vec3_t up, const float check_dist, vec3_t goal_direction) //mxd. Named 'imp_check_directions' in original logic.
{
	//mxd. Avoid modifying input vectors.
	vec3_t directions[3];
	VectorCopy(forward, directions[0]);
	VectorCopy(right, directions[1]);
	VectorCopy(up, directions[2]);

	//mxd. Somewhat randomize axis check order.
	int axis = irand(10, 12); // Offset from 0, so going in negative direction works correctly.
	const int increment = Q_sign(irand(-1, 0));

	// Check cardinal directions.
	for (int i = 0; i < 3; i++, axis += increment)
	{
		vec3_t direction;
		VectorCopy(directions[axis % 3], direction);

		// Don't always check same direction first (looks mechanical).
		if (irand(0, 1) == 1)
			Vec3ScaleAssign(-1.0f, direction);

		// Check opposite directions.
		for (int c = 0; c < 2; c++)
		{
			vec3_t start_pos;
			VectorMA(self->s.origin, check_dist, direction, start_pos);

			trace_t trace;
			gi.trace(start_pos, self->mins, self->maxs, goal, self, MASK_SHOT | MASK_WATER, &trace);

			// We've found somewhere to go.
			if (trace.ent == self->enemy)
			{
				VectorCopy(direction, goal_direction);
				return true;
			}

			Vec3ScaleAssign(-1.0f, direction); //BUGFIX: mxd. Original logic checks self->s.origin position instead of check_dist offset from it when checking second direction.
		}
	}

	return false;
}

//mxd. Very similar to HarpyCheckSwoop().
static qboolean ImpCheckSwoop(const edict_t* self, const vec3_t goal_pos) //mxd. Named 'imp_check_swoop' in original logic.
{
	// Find the difference in the target's height and the creature's height.
	float z_diff = Q_fabs(self->enemy->s.origin[2] - self->s.origin[2]);

	if (z_diff < IMP_MIN_SWOOP_DIST)
		return false;

	z_diff -= z_diff / 4.0f;

	vec3_t check_pos;
	VectorCopy(self->s.origin, check_pos);
	check_pos[2] -= z_diff;

	// Trace down about that far and about one forth the distance to the target.
	trace_t trace;
	gi.trace(self->s.origin, self->mins, self->maxs, check_pos, self, MASK_SHOT | MASK_WATER, &trace);

	if (trace.fraction < 1.0f) //mxd. HarpyCheckSwoop() also checks trace.startsolid and trace.allsolid.
		return false;

	// Trace straight to the target.
	gi.trace(check_pos, self->mins, self->maxs, goal_pos, self, MASK_SHOT | MASK_WATER, &trace);

	// If we hit our enemy, there's a clear path.
	return (trace.ent == self->enemy);
}

#pragma endregion

#pragma region ========================== Message handlers ==========================

static void ImpDeathPainMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'imp_death_pain' in original logic.
{
	if (self->health <= -40) // Gib death.
		BecomeDebris(self);
}

static void ImpDeathMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'imp_die' in original logic.
{
	if (self->monsterinfo.aiflags & AI_DONT_THINK)
	{
		SetAnim(self, ANIM_DIE);
		return;
	}

	self->movetype = PHYSICSTYPE_STEP;
	self->gravity = 1.0f;
	self->elasticity = 1.1f;

	VectorCopy(dead_imp_mins, self->mins); //mxd
	VectorCopy(dead_imp_maxs, self->maxs); //mxd

	if (self->health <= -40) // Gib death.
	{
		BecomeDebris(self);
		self->think = NULL;
		self->nextthink = 0.0f;

		gi.linkentity(self);
	}
	else
	{
		self->msgHandler = DeadMsgHandler;
		SetAnim(self, ANIM_DIE);
	}
}

static void ImpPainMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'imp_pain' in original logic.
{
	int temp;
	int damage;
	qboolean force_pain;
	ParseMsgParms(msg, "eeiii", &temp, &temp, &force_pain, &damage, &temp);

	if (self->curAnimID == ANIM_PERCH)
	{
		SetAnim(self, ANIM_TAKEOFF);
	}
	else if (force_pain || (irand(0, 10) < 2 && self->pain_debounce_time < level.time))
	{
		self->pain_debounce_time = level.time + 2.0f;

		if (self->curAnimID == ANIM_DIVE_GO || self->curAnimID == ANIM_DIVE_LOOP)
			SetAnim(self, ANIM_DIVE_END);
		else
			SetAnim(self, ANIM_PAIN1);
	}
}

// Receiver for MSG_RUN and MSG_FLY.
static void ImpFlyMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'imp_hover' in original logic.
{
	if (self->curAnimID != ANIM_PERCH)
		SetAnim(self, ANIM_HOVER1);
}

static void ImpStandMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'imp_stand' in original logic.
{
	if (!(self->spawnflags & MSF_PERCHING))
		SetAnim(self, ANIM_HOVER1);
}

static void ImpWatchMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'imp_perch' in original logic.
{
	SetAnim(self, ANIM_PERCH);
}

#pragma endregion

#pragma region ========================== Edict callbacks ===========================

static void ImpIsBlocked(edict_t* self, trace_t* trace) //mxd. Named 'imp_blocked' in original logic.
{
	if (self->health <= 0 || trace->ent == NULL)
		return;

	if (self->curAnimID == ANIM_DIVE_GO || self->curAnimID == ANIM_DIVE_LOOP || self->curAnimID == ANIM_DIVE_END)
	{
		if (Q_stricmp(trace->ent->classname, "player") == 0 && irand(0, 4) == 0) //mxd. stricmp -> Q_stricmp. //TODO: check ent->client instead?
			P_KnockDownPlayer(&trace->ent->client->playerinfo);

		vec3_t dir; //BUGFIX: mxd. Not initialized in original logic.
		VectorCopy(self->velocity, dir);
		VectorNormalize(dir);

		const int damage = irand(IMP_DMG_MIN, IMP_DMG_MAX);
		T_Damage(trace->ent, self, self, dir, trace->ent->s.origin, trace->plane.normal, damage, damage * 2, 0, MOD_DIED);

		gi.sound(self, CHAN_BODY, sounds[SND_HIT], 1.0f, ATTN_NORM, 0.0f);

		if (self->curAnimID != ANIM_DIVE_END)
			SetAnim(self, ANIM_DIVE_END);
	}
}

#pragma endregion

#pragma region ========================== Action functions ==========================

// Various sound functions.
void imp_flap_noise(edict_t* self)
{
	gi.sound(self, CHAN_ITEM, sounds[SND_FLAP], 1.0f, ATTN_NORM, 0.0f);
}

void imp_dive_noise(edict_t* self)
{
	gi.sound(self, CHAN_VOICE, sounds[SND_DIVE], 1.0f, ATTN_NORM, 0.0f);
}

void imp_ai_fly(edict_t* self, float forward_offset, float right_offset, float up_offset)
{
	if (self->enemy == NULL)
		return;

	// Add "friction" to the movement to allow graceful flowing motion, not jittering.
	Vec3ScaleAssign(0.8f, self->velocity);

	// Find our ideal yaw to the player and correct to it.
	vec3_t diff;
	VectorSubtract(self->enemy->s.origin, self->s.origin, diff);

	self->ideal_yaw = VectorYaw(diff);
	M_ChangeYaw(self);

	if (!ImpCanMove(self, forward_offset / 10.0f))
	{
		SetAnim(self, ANIM_HOVER1);
		return;
	}

	if (self->spawnflags & MSF_FIXED)
		return;

	// Add in the movements relative to the creature's facing.
	vec3_t forward;
	vec3_t right;
	vec3_t up;
	AngleVectors(self->s.angles, forward, right, up);

	VectorMA(self->velocity, forward_offset, forward, self->velocity);
	VectorMA(self->velocity, right_offset, right, self->velocity);
	VectorMA(self->velocity, up_offset, up, self->velocity);

	if (self->groundentity != NULL)
		self->velocity[2] += 32.0f;
}

// Replaces ai_stand for imp.
void imp_ai_hover(edict_t* self, float distance) //TODO: very similar to harpy_ai_hover. Move to mg_ai.c as ai_hover()?
{
	if (self->enemy == NULL && !FindTarget(self))
		return;

	// Add "friction" to the movement to allow graceful flowing motion, not jittering.
	Vec3ScaleAssign(0.8f, self->velocity);

	// Make sure we're not tilted after a turn.
	self->s.angles[ROLL] *= 0.25f;

	// Find our ideal yaw to the player and correct to it.
	vec3_t diff;
	VectorSubtract(self->enemy->s.origin, self->s.origin, diff);

	self->ideal_yaw = VectorYaw(diff);
	M_ChangeYaw(self);

	ImpAIGlide(self);
}

void imp_flyback(edict_t* self)
{
	SetAnim(self, ANIM_FLYBACK1);
}

void imp_ai_perch(edict_t* self) //mxd. Named 'imp_ai_pirch' in original logic.
{
	if (!M_ValidTarget(self, self->enemy) || !AI_IsVisible(self, self->enemy))
		return;

	vec3_t diff;
	VectorSubtract(self->enemy->s.origin, self->s.origin, diff);
	const float dist = VectorNormalize(diff);

	if (dist < 150.0f)
	{
		SetAnim(self, ANIM_TAKEOFF);
	}
	else
	{
		vec3_t forward;
		AngleVectors(self->s.angles, forward, NULL, NULL);

		if (DotProduct(diff, forward) < 0.0f)
			SetAnim(self, ANIM_TAKEOFF);
	}
}

void imp_tumble_move(edict_t* self) //mxd. Named 'move_imp_tumble' in original logic.
{
	self->movetype = PHYSICSTYPE_STEP;
	self->gravity = 1.0f;

	VectorCopy(dead_imp_mins, self->mins); //mxd
	VectorCopy(dead_imp_maxs, self->maxs); //mxd

	if (Vec3IsZeroEpsilon(self->avelocity)) //mxd. Avoid direct floating point number comparisons.
	{
		self->avelocity[PITCH] = flrand(128.0f, 256.0f);
		self->avelocity[YAW] = flrand(64.0f, 512.0f);
		self->avelocity[ROLL] = flrand(64.0f, 512.0f);
	}

	if (self->groundentity != NULL || self->monsterinfo.jump_time < level.time)
	{
		gi.CreateEffect(&self->s, FX_DUST_PUFF, CEF_OWNERS_ORIGIN, self->s.origin, NULL);
		gi.sound(self, CHAN_VOICE, sounds[SND_DEATH], 1.0f, ATTN_NORM, 0.0f); //mxd. Inline imp_death_noise().

		VectorCopy(self->s.angles, self->movedir);
		SetAnim(self, ANIM_DIE);
	}
}

void imp_fix_angles(edict_t* self) //TODO: harpy_fix_angles() duplicate.
{
	// Apply pitch delta.
	if (self->movedir[PITCH] > 0.0f)
	{
		self->s.angles[PITCH] -= self->movedir[PITCH] / 2.0f;

		if (self->s.angles[PITCH] < 2.0f)
			self->s.angles[PITCH] = 0.0f;
	}
	else
	{
		self->s.angles[PITCH] += self->movedir[PITCH] / 2.0f;

		if (self->s.angles[PITCH] > 2.0f)
			self->s.angles[PITCH] = 0.0f;
	}

	// Apply roll delta.
	if (self->movedir[ROLL] > 0.0f)
	{
		self->s.angles[ROLL] -= self->movedir[ROLL] / 2.0f;

		if (self->s.angles[ROLL] < 2.0f)
			self->s.angles[ROLL] = 0.0f;
	}
	else
	{
		self->s.angles[ROLL] += self->movedir[ROLL] / 15.0f;

		if (self->s.angles[ROLL] > 2.0f)
			self->s.angles[ROLL] = 0.0f;
	}
}

void imp_hit(edict_t* self, float stop_swoop)
{
	trace_t trace;
	edict_t* victim = M_CheckMeleeHit(self, VectorLength(self->velocity), &trace);

	if (victim == NULL)
		return;

	if (victim == self && (qboolean)stop_swoop)
	{
		SetAnim(self, ANIM_DIVE_OUT);
	}
	else
	{
		gi.sound(self, CHAN_BODY, sounds[SND_HIT], 1.0f, ATTN_NORM, 0.0f);

		vec3_t forward;
		AngleVectors(self->s.angles, forward, NULL, NULL);

		const int damage = irand(IMP_DMG_MIN, IMP_DMG_MAX);
		T_Damage(victim, self, self, forward, self->enemy->s.origin, trace.plane.normal, damage, damage * 2, 0, MOD_DIED);

		SetAnim(self, ANIM_DIVE_END);
	}
}

void imp_pause(edict_t* self)
{
	if (M_ValidTarget(self, self->enemy))
		QPostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
}

// End of animation func for death animation.
void imp_dead(edict_t* self)
{
	VectorCopy(dead_imp_mins, self->mins); //mxd
	VectorCopy(dead_imp_maxs, self->maxs); //mxd

	M_EndDeath(self);
}

void imp_dive_move(edict_t* self) //mxd. Named 'move_imp_dive' in original logic.
{
	if (self->groundentity != NULL || !ImpCanMove(self, 64.0f))
	{
		if (self->groundentity == self->enemy)
			imp_hit(self, true);

		return;
	}

	const float z_dist = Q_fabs(self->s.origin[2] - self->enemy->s.origin[2]);

	vec3_t forward;
	AngleVectors(self->s.angles, forward, NULL, NULL);

	float forward_dist = 256.0f - z_dist * 0.85f;
	forward_dist = Clamp(forward_dist, 0.0f, 256.0f);

	VectorMA(forward, forward_dist, forward, self->velocity);

	self->velocity[2] = -z_dist * 2.25f;
	self->velocity[2] = max(-300.0f, self->velocity[2]);

	ImpAIGlide(self);
}

void imp_dive_loop(edict_t* self)
{
	SetAnim(self, ANIM_DIVE_LOOP);
}

void imp_check_dodge(edict_t* self)
{
	if (self->enemy == NULL || (self->spawnflags & MSF_FIXED))
		return;

	vec3_t enemy_dir;
	VectorSubtract(self->enemy->s.origin, self->s.origin, enemy_dir);
	VectorNormalize(enemy_dir);

	vec3_t dodge_dir;
	qboolean dodge = false;
	qboolean vertical_dodge = false;

	edict_t* ent = NULL;
	while ((ent = FindInRadius(ent, self->s.origin, IMP_PROJECTILE_SEARCH_RADIUS)) != NULL)
	{
		// We're only interested in his projectiles.
		if (ent->owner != self->enemy)
			continue;

		vec3_t proj_dir;
		VectorNormalize2(ent->velocity, proj_dir);

		if (DotProduct(proj_dir, enemy_dir) < -0.85f && irand(0, 1) == 1)
		{
			dodge = true;

			vec3_t up;
			vec3_t right;
			AngleVectors(self->s.angles, NULL, right, up);

			vec3_t goal_pos;
			VectorCopy(self->s.origin, goal_pos);

			// Pick horizontal or vertical dodge direction.
			if (irand(0, 1) == 1)
			{
				VectorScale(right, Q_signf(flrand(-1.0f, 0.0f)), dodge_dir);
			}
			else
			{
				VectorScale(up, Q_signf(flrand(-1.0f, 0.0f)), dodge_dir);
				vertical_dodge = true;
			}

			VectorMA(goal_pos, 100.0f, dodge_dir, goal_pos);

			trace_t trace;
			gi.trace(self->s.origin, self->mins, self->maxs, goal_pos, self, MASK_SHOT | MASK_WATER, &trace);

			if (trace.fraction < 1.0f) // Bad direction, try another.
			{
				Vec3ScaleAssign(-1.0f, dodge_dir);

				if (vertical_dodge)
				{
					// Ok, better check this new opposite dir.
					gi.trace(self->s.origin, self->mins, self->maxs, goal_pos, self, MASK_SHOT | MASK_WATER, &trace);

					if (trace.fraction < 1.0f)
					{
						// Uh-oh, let's go for a side dir.
						VectorScale(right, Q_signf(flrand(-1.0f, 0.0f)), dodge_dir);

						gi.trace(self->s.origin, self->mins, self->maxs, goal_pos, self, MASK_SHOT | MASK_WATER, &trace);

						if (trace.fraction < 1.0f)// What the hell? Just go the other way...
							VectorScale(dodge_dir, -1, dodge_dir);
					}
				}
			}

			//TODO: we found projectile to dodge. Shouldn't we either break here, or compare distances with previous match to find the closest one?
		}
	}

	if (dodge && self->monsterinfo.misc_debounce_time < level.time) // If he is, dodge!
	{
		if (self->curAnimID != ANIM_FIREBALL)
		{
			if (dodge_dir[2] > 0.1f)
				SetAnim(self, ANIM_DUP);
			else if (dodge_dir[2] < -0.1f)
				SetAnim(self, ANIM_DDOWN);
		}

		VectorMA(self->velocity, flrand(300.0f, 500.0f), dodge_dir, self->velocity); //mxd. irand() in original logic.
		self->monsterinfo.misc_debounce_time = level.time + flrand(2.0f, 4.0f); //mxd. irand() in original logic.
	}

	ImpAIGlide(self);
}

void imp_hover_move(edict_t* self) //mxd. Named 'move_imp_hover' in original logic.
{
	if (self->enemy == NULL || !FindTarget(self))
		return;

	// First check to see that the player is at least 128 units away in (discounting z height).
	vec3_t goal_dir;
	VectorSubtract(self->enemy->s.origin, self->s.origin, goal_dir);
	goal_dir[2] = 0.0f;

	// Face target.
	self->ideal_yaw = VectorYaw(goal_dir);
	M_ChangeYaw(self);

	const float goal_dist = VectorLength(goal_dir);

	// If he is...
	if (goal_dist > IMP_MIN_HOVER_DIST && goal_dist < IMP_MAX_HOVER_DIST)
	{
		// Make sure we've got line of sight.
		const vec3_t mins = { -1.0f, -1.0f, -1.0f };
		const vec3_t maxs = {  1.0f,  1.0f,  1.0f };

		trace_t trace;
		gi.trace(self->s.origin, mins, maxs, self->enemy->s.origin, self, MASK_SHOT | MASK_WATER, &trace);

		// If not, try looking from a bit to the side in all six directions.
		if (trace.ent != self->enemy)
		{
			// Setup the directions.
			vec3_t forward;
			vec3_t right;
			vec3_t up;
			AngleVectors(self->s.angles, forward, right, up);

			// If we can see him from one of these, go there.
			if (ImpCheckDirections(self, self->enemy->s.origin, forward, right, up, IMP_CHECK_DIST, goal_dir))
			{
				VectorMA(self->velocity, flrand(300.0f, 400.0f), goal_dir, self->velocity);
				return;
			}

			// Otherwise just flap around and wait, perhaps lower yourself a bit if high up.
			self->velocity[0] = flrand(-IMP_DRIFT_AMOUNT_X, IMP_DRIFT_AMOUNT_X);
			self->velocity[1] = flrand(-IMP_DRIFT_AMOUNT_Y, IMP_DRIFT_AMOUNT_Y);
			self->velocity[2] = flrand(-IMP_DRIFT_AMOUNT_Z, IMP_DRIFT_AMOUNT_Z);

			return;
		}

		vec3_t dodge_dir;
		// Check to make sure the player isn't shooting anything.

		// This won't change over the calculations
		vec3_t enemy_dir;
		VectorSubtract(self->enemy->s.origin, self->s.origin, enemy_dir);
		const float enemy_dist = VectorNormalize(enemy_dir);

		qboolean dodge = false;
		edict_t* ent = NULL;

		while ((ent = FindInRadius(ent, self->s.origin, IMP_PROJECTILE_SEARCH_RADIUS)) != NULL)
		{
			// We're only interested in his projectiles.
			if (ent->owner != self->enemy)
				continue;

			vec3_t proj_dir;
			VectorNormalize2(ent->velocity, proj_dir);

			if (DotProduct(proj_dir, enemy_dir) < -0.6f)
			{
				dodge = true;

				vec3_t right;
				AngleVectors(self->s.angles, NULL, right, NULL);

				if (irand(0, 1) == 1)
					Vec3ScaleAssign(-1.0f, right);

				vec3_t goal_pos;
				VectorMA(self->s.origin, 100.0f, right, goal_pos);

				gi.trace(self->s.origin, self->mins, self->maxs, goal_pos, self, MASK_SHOT | MASK_WATER, &trace);

				if (trace.fraction < 1.0f) //mxd. harpy_hover_move() also checks trace.startsolid and trace.allsolid here.
					Vec3ScaleAssign(-1.0f, right);

				VectorCopy(right, dodge_dir);

				//TODO: we found projectile to dodge. Shouldn't we either break here, or compare distances with previous match to find the closest one?
			}
		}

		if (dodge)
		{
			// If he is, dodge!
			VectorMA(self->velocity, flrand(300.0f, 500.0f), dodge_dir, self->velocity); //mxd. irand() in original logic.
			return;
		}

		// See if he's too close.
		if (enemy_dist < Q_fabs(self->melee_range))
		{
			SetAnim(self, ANIM_FLYBACK1);
		}
		else if (enemy_dist < self->missile_range && enemy_dist > self->min_missile_range && irand(0, 100) > self->bypass_missile_chance) // See if we can and want to attack him.
		{
			SetAnim(self, ANIM_FIREBALL);
			return;
		}

		// If nothing is happening, check to swoop. If you can - nail'em.
		if (ImpCheckSwoop(self, self->enemy->s.origin))
		{
			self->monsterinfo.jump_time = 2;
			SetAnim(self, ANIM_DIVE_GO);

			return;
		}

		// Find the difference in the target's height and the creature's height.
		const float z_dist = Q_fabs(self->enemy->s.origin[2] - self->s.origin[2]);

		// We can't swoop because we're too low, so fly upwards if possible.
		if (z_dist < IMP_MIN_SWOOP_DIST)
		{
			if (!ImpCanMove(self, -64.0f))
				SetAnim(self, ANIM_FLY1);
			else
				SetAnim(self, ANIM_FLYBACK1);
		}
		else
		{
			// Otherwise just flap around and wait, perhaps lower yourself a bit if high up.			
			self->velocity[0] = flrand(-IMP_DRIFT_AMOUNT_X, IMP_DRIFT_AMOUNT_X);
			self->velocity[1] = flrand(-IMP_DRIFT_AMOUNT_Y, IMP_DRIFT_AMOUNT_Y);
			self->velocity[2] = flrand(-IMP_DRIFT_AMOUNT_Z, IMP_DRIFT_AMOUNT_Z);

			vec3_t forward;
			AngleVectors(self->s.angles, forward, NULL, NULL);
			VectorMA(self->velocity, flrand(200.0f, 300.0f), forward, self->velocity); //mxd. irand() in original logic.
		}

		//FIXME: If he's too far away trace a line (expanded) to see if you can move at him.
		return;
	}

	if (goal_dist < IMP_MIN_HOVER_DIST)
	{
		if (!ImpCanMove(self, -64.0f))
			SetAnim(self, ANIM_FLY1);
		else
			SetAnim(self, ANIM_FLYBACK1);
	}
	else
	{
		if (!ImpCanMove(self, 64.0f))
			SetAnim(self, ANIM_FLYBACK1);
		else
			SetAnim(self, ANIM_FLY1);
	}
}

void imp_fly_move(edict_t* self) //mxd. Named 'move_imp_fly' in original logic.
{
	if (irand(0, 3) == 0)
		imp_check_dodge(self);
}

#pragma endregion

#pragma region ========================== Imp Fireball ==========================

static void ImpFireballFizzle(edict_t* self) //mxd. Named 'FireFizzle' in original logic.
{
	gi.sound(self, CHAN_BODY, sounds[SND_FIZZLE], 1.0f, ATTN_NORM, 0.0f);

	vec3_t dir = { flrand(0.0f, 1.0f), flrand(0.0f, 1.0f), flrand(0.5f, 1.0f) };
	VectorNormalize(dir);

	gi.CreateEffect(&self->s, FX_ENVSMOKE, CEF_BROADCAST, self->s.origin, "bdbbb", irand(1, 3), dir, irand(1, 2), irand(3, 4), irand(1, 2));

	G_SetToFree(self);
}

static void ImpFireballBlocked(edict_t* self, trace_t* trace) //mxd. Named 'fireball_blocked' in original logic.
{
	if (trace->surface != NULL && (trace->surface->flags & SURF_SKY))
	{
		SkyFly(self);
		return;
	}

	if ((trace->contents & CONTENTS_WATER) || (trace->contents & CONTENTS_SLIME))
	{
		ImpFireballFizzle(self);
		return;
	}

	if (trace->ent != NULL && EntReflecting(trace->ent, true, true) && self->reflect_debounce_time > 0)
	{
		Create_rand_relect_vect(self->velocity, self->velocity);
		Vec3ScaleAssign(self->ideal_yaw, self->velocity);
		ImpFireballReflect(self, trace->ent, self->velocity);

		return;
	}

	if (trace->ent != NULL && trace->ent->takedamage != DAMAGE_NO) //mxd. Add trace->ent NULL check.
	{
		vec3_t hit_dir;
		VectorNormalize2(self->velocity, hit_dir);

		const int damage = max(0, self->dmg) + irand(2, 5); //mxd. float in original logic.
		T_Damage(trace->ent, self, self->owner, hit_dir, self->s.origin, trace->plane.normal, damage, 0, DAMAGE_SPELL | DAMAGE_NO_KNOCKBACK, MOD_DIED);
	}

	gi.sound(self, CHAN_BODY, sounds[SND_FBHIT], 1.0f, ATTN_NORM, 0.0f);
	gi.CreateEffect(&self->s, FX_M_EFFECTS, CEF_OWNERS_ORIGIN, self->s.origin, "bv", FX_IMP_FBEXPL, vec3_origin);

	G_SetToFree(self);
}

static void ImpProjectileInit(const edict_t* self, edict_t* proj) //mxd. Named 'create_imp_proj' in original logic.
{
	proj->svflags |= SVF_ALWAYS_SEND;
	proj->movetype = PHYSICSTYPE_FLY;
	proj->gravity = 0.0f;
	proj->solid = SOLID_BBOX;
	proj->classname = "imp fireball";
	proj->s.scale = 1.0f;
	proj->clipmask = (MASK_SHOT | CONTENTS_WATER);
	proj->s.effects = EF_MARCUS_FLAG1;
	proj->enemy = self->enemy;
	proj->reflect_debounce_time = MAX_REFLECT;

	proj->isBlocked = ImpFireballBlocked;
	proj->isBlocking = ImpFireballBlocked;
	proj->bounced = ImpFireballBlocked;

	VectorSet(proj->mins, -1.0f, -1.0f, -1.0f);
	VectorSet(proj->maxs,  1.0f,  1.0f,  1.0f);
	VectorCopy(self->s.origin, proj->s.origin);
}

edict_t* ImpFireballReflect(edict_t* self, edict_t* other, const vec3_t vel)
{
	edict_t* fireball = G_Spawn();

	ImpProjectileInit(self, fireball);

	fireball->s.modelindex = self->s.modelindex;
	VectorCopy(self->s.origin, fireball->s.origin);
	fireball->owner = other;
	fireball->enemy = self->owner;
	fireball->nextthink = self->nextthink;
	VectorScale(self->avelocity, -0.5f, fireball->avelocity);
	VectorCopy(vel, fireball->velocity);
	VectorNormalize2(vel, fireball->movedir);
	AnglesFromDir(fireball->movedir, fireball->s.angles);
	fireball->classID = self->classID;
	fireball->reflect_debounce_time = self->reflect_debounce_time - 1;
	fireball->reflected_time = self->reflected_time;
	fireball->ideal_yaw = self->ideal_yaw;

	gi.CreateEffect(&fireball->s, FX_M_EFFECTS, CEF_OWNERS_ORIGIN, NULL, "bv", FX_IMP_FIRE, fireball->velocity);

	G_LinkMissile(fireball);
	G_SetToFree(self);

	gi.CreateEffect(&fireball->s, FX_LIGHTNING_HIT, CEF_OWNERS_ORIGIN, NULL, "t", vel);

	return fireball;
}

void imp_fireball(edict_t* self)
{
	// Spawn the projectile.
	edict_t* proj = G_Spawn();

	ImpProjectileInit(self, proj);

	proj->reflect_debounce_time = MAX_REFLECT;
	proj->owner = self;
	proj->dmg = irand(10, 20);

	vec3_t forward;
	vec3_t right;
	AngleVectors(self->s.angles, forward, right, NULL);

	if (self->classID == CID_IMP) //TODO: imp_fireball() is used by Imp only.
	{
		VectorMA(self->s.origin, -4.0f * self->monsterinfo.scale, forward, proj->s.origin);
		VectorMA(proj->s.origin, 16.0f * self->monsterinfo.scale, right, proj->s.origin);
		proj->s.origin[2] += 32.0f * self->monsterinfo.scale;

		gi.sound(proj, CHAN_BODY, sounds[SND_ATTACK], 1.0f, ATTN_NORM, 0.0f);
	}
	else
	{
		VectorCopy(self->s.origin, proj->s.origin);
		VectorMA(proj->s.origin, 16.0f, forward, proj->s.origin);
		proj->s.origin[2] += 12.0f;

		gi.sound(proj, CHAN_BODY, gi.soundindex("monsters/imp/fireball.wav"), 1.0f, ATTN_NORM, 0.0f);
	}

	vec3_t check_lead;
	ExtrapolateFireDirection(self, proj->s.origin, 666.0f, self->enemy, 0.3f, check_lead);

	if (Vec3IsZero(check_lead))
		VectorScale(forward, 666.0f, proj->velocity);
	else
		VectorScale(check_lead, 666.0f, proj->velocity);

	VectorCopy(proj->velocity, proj->movedir);
	VectorNormalize(proj->movedir);
	vectoangles(proj->movedir, proj->s.angles);

	gi.CreateEffect(&proj->s, FX_M_EFFECTS, CEF_OWNERS_ORIGIN, NULL, "bv", FX_IMP_FIRE, proj->velocity);
	gi.linkentity(proj);
}

#pragma endregion

void ImpStaticsInit(void)
{
	static ClassResourceInfo_t res_info; //mxd. Made local static.

	classStatics[CID_IMP].msgReceivers[MSG_DEATH] = ImpDeathMsgHandler;
	classStatics[CID_IMP].msgReceivers[MSG_FLY] = ImpFlyMsgHandler;
	classStatics[CID_IMP].msgReceivers[MSG_STAND] = ImpStandMsgHandler;
	classStatics[CID_IMP].msgReceivers[MSG_RUN] = ImpFlyMsgHandler;
	classStatics[CID_IMP].msgReceivers[MSG_PAIN] = ImpPainMsgHandler;
	classStatics[CID_IMP].msgReceivers[MSG_WATCH] = ImpWatchMsgHandler;
	classStatics[CID_IMP].msgReceivers[MSG_DEATH_PAIN] = ImpDeathPainMsgHandler;

	res_info.numAnims = NUM_ANIMS;
	res_info.animations = animations;
	res_info.modelIndex = gi.modelindex("models/monsters/imp/tris.fm"); //TODO: no such model!

	sounds[SND_GIB] = gi.soundindex("misc/fleshbreak.wav");
	sounds[SND_FLAP] = gi.soundindex("monsters/imp/fly.wav");
	sounds[SND_SCREAM] = gi.soundindex("monsters/imp/up.wav");
	sounds[SND_DIVE] = gi.soundindex("monsters/imp/swoop.wav");
	sounds[SND_DEATH] = gi.soundindex("monsters/imp/die.wav");
	sounds[SND_HIT] = gi.soundindex("monsters/imp/swoophit.wav");
	sounds[SND_ATTACK] = gi.soundindex("monsters/imp/fireball.wav");
	sounds[SND_FIZZLE] = gi.soundindex("monsters/imp/fout.wav");
	sounds[SND_FBHIT] = gi.soundindex("monsters/imp/fbfire.wav");

	res_info.numSounds = NUM_SOUNDS;
	res_info.sounds = sounds;

	classStatics[CID_IMP].resInfo = &res_info;
}

// QUAKED monster_imp(1 .5 0) (-16 -16 0) (16 16 32) AMBUSH ASLEEP Perching 8 16 32 64 FIXED
// Our old pal, the fire imp!

// Spawnflags:
// AMBUSH	- Will not be woken up by other monsters or shots from player.
// ASLEEP	- Will not appear until triggered.
// PERCHING	- Will watch player until get too close or get behind the imp.

// Variables:
// wakeup_target			- Monsters will fire this target the first time it wakes up (only once).
// pain_target				- Monsters will fire this target the first time it gets hurt (only once).
// mintel					- Monster intelligence - this basically tells a monster how many buoys away an enemy has to be for it to give up (default 14).
// melee_range				- How close the player has to be for the monster to go into melee. If this is zero, the monster will never melee.
//							  If it is negative, the monster will try to keep this distance from the player.
//							  If the monster has a backup, he'll use it if too close, otherwise, a negative value here means the monster will just stop
//							  running at the player at this distance (default -64).
//							 Examples:
//								melee_range = 60 - monster will start swinging it player is closer than 60.
//								melee_range = 0 - monster will never do a melee attack.
//								melee_range = -100 - monster will never do a melee attack and will back away (if it has that ability) when player gets too close.
// missile_range			- Maximum distance the player can be from the monster to be allowed to use it's ranged attack (default 1024).
// min_missile_range		- Minimum distance the player can be from the monster to be allowed to use it's ranged attack (default 32).
// bypass_missile_chance	- Chance that a monster will NOT fire it's ranged attack, even when it has a clear shot. This, in effect, will make the monster
//							  come in more often than hang back and fire. A percentage (0 = always fire/never close in, 100 = never fire/always close in) - must be whole number (default 20).
// jump_chance				- Every time the monster has the opportunity to jump, what is the chance (out of 100) that he will... (100 = jump every time) - must be whole number (default 0).
// wakeup_distance			- How far (max) the player can be away from the monster before it wakes up. This means that if the monster can see the player,
//							  at what distance should the monster actually notice him and go for him (default 1024).
// NOTE: A value of zero will result in defaults, if you actually want zero as the value, use -1.
void SP_monster_imp(edict_t* self)
{
	if (!M_FlymonsterStart(self))
		return; // Failed initialization

	self->msgHandler = DefaultMsgHandler;

	if (self->health == 0)
		self->health = IMP_HEALTH;

	self->health = MonsterHealth(self->health);
	self->max_health = self->health;

	self->mass = IMP_MASS;
	self->yaw_speed = 14.0f;

	self->movetype = PHYSICSTYPE_FLY;
	self->gravity = 0.0f;
	self->flags |= FL_FLY;
	self->solid = SOLID_BBOX;
	self->clipmask = MASK_MONSTERSOLID;

	VectorCopy(STDMinsForClass[self->classID], self->mins);
	VectorCopy(STDMaxsForClass[self->classID], self->maxs);

	self->svflags |= SVF_TAKE_NO_IMPACT_DMG;
	self->materialtype = MAT_FLESH;
	self->s.modelindex = (byte)classStatics[CID_IMP].resInfo->modelIndex;
	self->s.skinnum = 0;

	self->isBlocked = ImpIsBlocked;

	if (self->s.scale == 0.0f)
	{
		self->s.scale = flrand(0.7f, 1.2f);
		self->monsterinfo.scale = self->s.scale;
	}

	self->monsterinfo.otherenemyname = "monster_rat";

	if (self->spawnflags & MSF_PERCHING)
		SetAnim(self, ANIM_PERCH);
	else
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);

	if (self->melee_range == 0.0f)
		self->melee_range = (float)AttackRangesForClass[self->classID * 4 + 0];

	if (self->missile_range == 0.0f)
		self->missile_range = (float)AttackRangesForClass[self->classID * 4 + 1];

	if (self->min_missile_range == 0.0f)
		self->min_missile_range = (float)AttackRangesForClass[self->classID * 4 + 2];

	if (self->bypass_missile_chance == 0)
		self->bypass_missile_chance = AttackRangesForClass[self->classID * 4 + 3];

	if (self->jump_chance == 0)
		self->jump_chance = JumpChanceForClass[self->classID];

	if (self->wakeup_distance == 0.0f)
		self->wakeup_distance = MAX_SIGHT_PLAYER_DIST;
}