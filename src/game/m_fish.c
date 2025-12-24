//
// m_fish.c
//
// Copyright 1998 Raven Software
//

#include "m_fish.h"
#include "m_fish_anim.h"
#include "m_fish_moves.h"
#include "m_fish_shared.h"
#include "m_stats.h"
#include "g_debris.h" //mxd
#include "g_DefaultMessageHandler.h"
#include "g_monster.h"
#include "Random.h"
#include "Utilities.h"
#include "Vector.h"
#include "g_local.h"

//TODO: move to m_stats.h?
#define FISH_WALK_TURN_ANGLE	40.0f //mxd. Named WALK_TURN_ANGLE in original logic.
#define FISH_RUN_TURN_ANGLE		70.0f //mxd. Named RUN_TURN_ANGLE in original logic.

#define FISH_BITE_DISTANCE		32.0f //mxd. Named BITE_DIST in original logic.
#define FISH_ACTIVATE_DISTANCE	3000.0f //mxd. Named FISH_ACTIVATE_DIST in original logic.

#define FISH_SPEED_FAST			160.0f //mxd. Named FISH_FAST in original logic.
#define FISH_SPEED_HUNT			(40.0f + FISH_SPEED_FAST) //mxd. Named FISH_HUNT in original logic.
#define FISH_SPEED_SLOW			100.0f //mxd. Named FISH_SLOW in original logic.
#define FISH_SPEED_DEFAULT		20.0f //mxd

#define FISH_SKIN1				0
#define FISH_SKIN2				2

#pragma region ========================== Fish base info ==========================

static const animmove_t* animations[NUM_ANIMS] =
{
	&fish_move_bite,
	&fish_move_melee,
	&fish_move_run1,
	&fish_move_run2,
	&fish_move_run3,
	&fish_move_walk1,
	&fish_move_walk2,
	&fish_move_walk3,
	&fish_move_stand1,
	&fish_move_pain1,
	&fish_move_death,
};

static int sounds[NUM_SOUNDS];

#pragma endregion

#pragma region ========================== Utility functions =========================

// Bring all our movedir angles up positive again.
static void FishResetMovedir(edict_t* self) //mxd. Named 'reset_fish_movedir' in original logic.
{
	for (int i = 0; i < 3; i++)
	{
		// Returns the remainder.
		self->movedir[i] = fmodf(self->movedir[i], 360.0f);

		// Make the angle unsigned.
		if (self->movedir[i] < 0.0f)
			self->movedir[i] += 360.0f;
	}
}

// Generic 'decided on a new direction' reaction - make us select a new direction.
static void FishPickNewDirection(edict_t* self) //mxd. Named 'fish_new_direction' in original logic.
{
	self->movedir[PITCH] = ((irand(0, 1) == 0) ? flrand(-30.0f, 30.0f) : 0.0f);
	self->movedir[YAW] += flrand(-45.0f, 45.0f);

	// Bring all our movedir angles up positive again.
	FishResetMovedir(self);

	// If we change direction, we might hit the same poly we just collided with.
	self->fish_last_collision_surface = NULL;

	// Decide which animation to use.
	if (self->ai_mood == AI_MOOD_WANDER)
	{
		self->speed = self->fish_speed_scaler * FISH_SPEED_FAST;
		fish_run(self);
	}
	else
	{
		self->speed = self->fish_speed_scaler * FISH_SPEED_SLOW;
		fish_walk(self);
	}
}

// Generic 'hit something' reaction - make us bounce off in a new direction.
static void FishPickBounceDirection(edict_t* self) //mxd. Named 'fish_bounce_direction' in original logic.
{
	// Reverse our direction with some randomness in the angles too.
	VectorCopy(self->s.angles, self->movedir);

	// Reverse our direction.
	self->movedir[YAW] += 180.0f;
	self->movedir[PITCH] *= -1.0f;

	// Add some randomness.
	self->movedir[YAW] += flrand(-15.0f, 15.0f); //mxd. irand() in original logic.
	self->movedir[PITCH] += flrand(-5.0f, 5.0f); //mxd. irand() in original logic.

	// Bring all our movedir angles up positive again.
	FishResetMovedir(self);

	// Decide which animation to use.
	if (self->ai_mood == AI_MOOD_WANDER)
	{
		self->speed = self->fish_speed_scaler * FISH_SPEED_FAST;
		fish_run(self);
	}
	else
	{
		self->speed = self->fish_speed_scaler * FISH_SPEED_SLOW;
		fish_walk(self);
	}
}

static float FishChangeYaw(edict_t* self) //mxd. Named 'M_ChangeFishYaw' in original logic. Very similar to MG_ChangeWhichYaw().
{
	const float current = anglemod(self->s.angles[YAW]);
	const float ideal = self->movedir[YAW];
	float move = NormalizeAngleDeg(ideal - current); //mxd. Use function instead of doing it manually.

	if (FloatIsZeroEpsilon(move)) //mxd. Avoid direct float comparison.
		return 0.0f;

	move = Clamp(move, -self->yaw_speed, self->yaw_speed);
	self->s.angles[YAW] = anglemod(current + move);

	return move;
}

static float FishChangePitch(edict_t* self) //mxd. Named 'M_ChangeFishPitch' in original logic. Very similar to MG_ChangeWhichYaw().
{
	const float current = anglemod(self->s.angles[PITCH]);
	const float ideal = self->movedir[PITCH];
	float move = NormalizeAngleDeg(ideal - current); //mxd. Use function instead of doing it manually.

	if (FloatIsZeroEpsilon(move)) //mxd. Avoid direct float comparison.
		return 0.0f;

	move = Clamp(move, -self->fish_max_pitch_speed, self->fish_max_pitch_speed);
	self->s.angles[PITCH] = anglemod(current + move);

	return move;
}

// Figure out where our prey is, and go get him.
static void FishMoveToTarget(edict_t* self) //mxd. Named 'fish_hunt' in original logic.
{
	// Make sure we still have a target - bouncing off stuff tends to clear it out.
	if (self->enemy == NULL && !FindTarget(self))
	{
		// If we can't find one, let him just swim on alone...
		if (self->curAnimID == ANIM_PAIN1)
		{
			self->speed = FISH_SPEED_DEFAULT;
			self->ai_mood = AI_MOOD_STAND;
			SetAnim(self, ANIM_STAND1);
		}

		return;
	}

	fish_update_target_movedir(self);

	// Set movement type.
	self->ai_mood = AI_MOOD_PURSUE;

	// Make us run after it.
	self->speed = self->fish_speed_scaler * FISH_SPEED_HUNT;
	fish_run(self);
}

#pragma endregion

#pragma region ========================== Message handlers ==========================

static void FishDeadPainMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'fish_dead_pain' in original logic.
{
	if (self->health < -60)
		BecomeDebris(self); //TODO: also play SND_GIB sound?
}

static void FishDeadMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'fish_death' in original logic.
{
	VectorClear(self->velocity);
	self->dead_state = DEAD_DEAD;

	if (self->health < -60)
	{
		gi.sound(self, CHAN_BODY, sounds[SND_GIB], 1.0f, ATTN_NORM, 0.0f);
		BecomeDebris(self);
	}
	else
	{
		gi.sound(self, CHAN_WEAPON, sounds[SND_DIE], 1.0f, ATTN_NORM, 0.0f);

		// Switch to damaged skin?
		if (self->s.skinnum == FISH_SKIN1 || self->s.skinnum == FISH_SKIN2)
			self->s.skinnum += 1;

		SetAnim(self, ANIM_DEATH1);
	}
}

static void FishPainMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'fish_pain' in original logic.
{
	int temp;
	int damage;
	qboolean force_pain;
	G_ParseMsgParms(msg, "eeiii", &temp, &temp, &force_pain, &damage, &temp);

	if (!force_pain && irand(0, 3) == 0) //mxd. flrand() in original logic.
		return;

	SetAnim(self, ANIM_PAIN1);
	VectorClear(self->velocity);
	self->dead_state = DEAD_DYING;

	// Switch to damaged skin?
	if (irand(0, 2) == 0 && (self->s.skinnum == FISH_SKIN1 || self->s.skinnum == FISH_SKIN2))
		self->s.skinnum += 1;

	gi.sound(self, CHAN_WEAPON, sounds[irand(SND_PAIN1, SND_PAIN2)], 1.0f, ATTN_NORM, 0.0f);
}

#pragma endregion

#pragma region ========================== Edict callbacks ===========================

void FishThink(edict_t* self) //mxd. Named 'fish_think' in original logic.
{
	// Determine if we are too far from the camera to warrant animating or AI.
	if (!gi.CheckDistances(self->s.origin, FISH_ACTIVATE_DISTANCE)) //mxd. Merged fish_check_distance() logic.
	{
		VectorClear(self->velocity);
		self->nextthink = level.time + 2.0f;

		return;
	}

	if (self->enemy == NULL)
		FindTarget(self);

	if (self->enemy != NULL && self->enemy->waterlevel == 0) // Let's not hunt things out of water!
		self->enemy = NULL;

	// Animate us.
	M_MoveFrame(self); //mxd. Also sets self->nextthink.

	// We are already dead or getting hit, we don't need to do anything.
	if ((self->dead_state & DEAD_DEAD) || (self->dead_state & DEAD_DYING))
		return;

	M_CatagorizePosition(self);

	// Did we break the water surface?
	if (self->waterlevel < 3)
	{
		// If we break water - don't let us target anyone anymore.
		self->enemy = NULL;
		self->ai_mood = AI_MOOD_WANDER;
		self->fish_max_pitch_speed = 10.0f;

		// Make us go down good sir!
		self->movedir[PITCH] = flrand(-35.0f, -15.0f);

		// Only allow one of these every second for this fish.
		if (!self->fish_ripple_spawned)
		{
			// Create a ripple.
			const vec3_t top =    VEC3_INITA(self->s.origin, 0.0f, 0.0f, self->maxs[2] * 0.75f);
			const vec3_t bottom = VEC3_INITA(self->s.origin, 0.0f, 0.0f, self->mins[2]);

			trace_t trace;
			gi.trace(top, vec3_origin, vec3_origin, bottom, self, MASK_WATER, &trace);

			if (trace.fraction <= 1.0f) //TODO: this is ALWAYS true. Remove or change to < 1.0?
			{
				// No ripples while in cinematics.
				if (!SV_CINEMATICFREEZE)
				{
					vec3_t forward;
					AngleVectors(self->s.angles, forward, NULL, NULL);
					Vec3ScaleAssign(200.0f, forward);

					const byte b_angle = (byte)((self->s.angles[YAW] + DEGREE_180) / 360.0f * 255.0f);
					gi.CreateEffect(NULL, FX_WATER_WAKE, 0, trace.endpos, "sbv", self->s.number, b_angle, forward);
				}

				gi.sound(self, CHAN_WEAPON, sounds[SND_SPLASH], 1.0f, ATTN_NORM, 0.0f);
				self->fish_ripple_spawned = true;
			}
		}
	}
	else
	{
		self->fish_ripple_spawned = false;
		self->fish_max_pitch_speed = 4.0f;
	}

	// Make sure that the movedir angles are between 0-359, or we are in trouble on the pitch and yaw routines.
	FishResetMovedir(self);

	// Change pitch if we should.
	FishChangePitch(self);

	// Move us from one angle to another slowly - unless we are moving through the "turn" anims, in which case the anim takes care of the YAW.
	vec3_t angles;
	VectorDegreesToRadians(self->s.angles, angles);

	if (!self->fish_is_turning)
	{
		// Update yaw.
		FishChangeYaw(self);

		// Update velocity.
		DirFromAngles(angles, self->velocity);
		Vec3ScaleAssign(self->speed, self->velocity);
	}
	else
	{
		// Update velocity
		DirFromAngles(angles, self->velocity);

		// We aren't updating yaw.
		self->velocity[PITCH] = 0.0f;
		self->velocity[YAW] = 0.0f;
		Vec3ScaleAssign(self->speed, self->velocity);
	}

	M_WorldEffects(self);
}

// The fish hit something.
void FishIsBlocked(edict_t* self, trace_t* trace) //mxd. Named 'fish_blocked' in original logic.
{
	// Dead fish don't rebound off stuff.
	if (self->dead_state == DEAD_DEAD)
		return;

	// We hit something, which is not world geometry.
	if (trace->ent != NULL)
	{
		// Did we hit a monster or player?
		if ((trace->ent->svflags & SVF_MONSTER) || trace->ent->client != NULL)
		{
			// Hit another fish - send us on our way. //mxd. Also bounce off plague ssithras (biting them seem to greatly confuse them), ambushing monsters (to avoid breaking ambush setups) and non-targetable players.
			if (trace->ent->classID == CID_FISH || trace->ent->classID == CID_SSITHRA || (trace->ent->spawnflags & MSF_AMBUSH) || (trace->ent->flags & FL_NOTARGET))
			{
				FishPickBounceDirection(self);
				return;
			}

			// Check if this guy is dead.
			if (trace->ent->dead_state == DEAD_DEAD)
			{
				FishPickBounceDirection(self);

				if (self->enemy == trace->ent) //mxd. Original logic unconditionally clears enemy (which is kinda strange).
					self->enemy = NULL;

				return;
			}

			// Not dead, so lets BITE THE BASTARD :)
			self->enemy = trace->ent;

			vec3_t diff;
			VectorSubtract(self->s.origin, trace->ent->s.origin, diff);
			const float dist = VectorLength(diff);

			if (dist < self->maxs[0] + self->enemy->maxs[0] + FISH_BITE_DISTANCE + 50.0f && self->fish_max_pitch_speed == 4.0f) // Within 20 of bounding box & not out of water.
			{
				SetAnim(self, ANIM_BITE);
				self->ai_mood = AI_MOOD_ATTACK;
			}
			else
			{
				FishMoveToTarget(self);
			}
		}
		else // Did we hit a model of some type?
		{
			FishPickBounceDirection(self);
		}

		return;
	}

	// Did we hit the same wall as last time? Because if we did, we already dealt with it.
	if (trace->surface != self->fish_last_collision_surface)
	{
		self->fish_last_collision_surface = trace->surface;
		FishPickBounceDirection(self);
	}
}

#pragma endregion

#pragma region ========================== Action functions ==========================

// Choose a run animation to use.
void fish_run(edict_t* self)
{
	const float delta = anglemod(self->s.angles[YAW] - self->movedir[YAW]);

	if (delta > 70.0f && delta <= 180.0f) // Look right.
	{
		// Tell the think function we are doing the turn, so don't play with the yaw.
		self->fish_is_turning = true;
		self->best_move_yaw = -FISH_RUN_TURN_ANGLE;
		SetAnim(self, ANIM_RUN3);
	}
	else if (delta > 180.0f && delta < 290.0f) // Look left.
	{
		// Tell the think function we are doing the turn, so don't play with the yaw.
		self->fish_is_turning = true;
		self->best_move_yaw = FISH_RUN_TURN_ANGLE;
		SetAnim(self, ANIM_RUN2);
	}
	else
	{
		// Tell the think function we are NOT doing the turn.
		self->fish_is_turning = false;
		SetAnim(self, ANIM_RUN1);
	}
}

// Choose a walk animation to use.
void fish_walk(edict_t* self)
{
	const float delta = anglemod(self->s.angles[YAW] - self->movedir[YAW]);

	if (delta > 40.0f && delta <= 180.0f) // Look right.
	{
		// tell the think function we are doing the turn, so don't play with the yaw.
		self->fish_is_turning = true;
		self->best_move_yaw = -FISH_WALK_TURN_ANGLE;
		SetAnim(self, ANIM_WALK3);
	}
	else if (delta > 180.0f && delta < 320.0f) // Look left. //BUGFIX: mxd. 'delta > 180 && delta < 20' in original logic (e.g. never).
	{
		// Tell the think function we are doing the turn, so don't play with the yaw.
		self->fish_is_turning = true;
		self->best_move_yaw = FISH_WALK_TURN_ANGLE;
		SetAnim(self, ANIM_WALK2);
	}
	else
	{
		// Tell the think function we are NOT doing the turn.
		self->fish_is_turning = false;
		SetAnim(self, ANIM_WALK1);
	}
}

// Update the yaw on the first frame of a new animation - stop jittering.
void fish_update_yaw(edict_t* self)
{
	self->s.angles[YAW] += self->best_move_yaw;
	self->best_move_yaw = 0.0f;
}

void fish_under_water_wake(edict_t* self)
{
	gi.CreateEffect(&self->s, FX_M_EFFECTS, CEF_OWNERS_ORIGIN, self->s.origin, "bv", FX_UNDER_WATER_WAKE, vec3_origin);
}

void fish_swim_sound(edict_t* self, float fast)
{
	if (fast != 0.0f)
	{
		if (irand(0, 1) == 0)
			gi.sound(self, CHAN_BODY, sounds[irand(SND_FAST_SWIM1, SND_FAST_SWIM2)], 0.75f, ATTN_IDLE, 0.0f);
	}
	else
	{
		if (irand(0, 4) == 0)
			gi.sound(self, CHAN_BODY, sounds[irand(SND_SLOW_SWIM1, SND_SLOW_SWIM2)], 0.5f, ATTN_IDLE, 0.0f);
	}
}

// The fish finished a walk swim cycle, shall we just randomly change direction or perhaps target a player or a bad guy? Or maybe just idle a bit.
void fish_walkswim_finished(edict_t* self) //mxd. Named 'finished_swim' in original logic.
{
	if (self->ai_mood == AI_MOOD_PURSUE)
	{
		FishMoveToTarget(self);
		return;
	}

	const int chance = irand(0, 10);

	if (chance < 4)
	{
		self->ai_mood = ((irand(0, 3) == 0) ? AI_MOOD_WANDER : AI_MOOD_STAND);
		FishPickNewDirection(self);
	}
	else if (chance < 6)
	{
		self->speed = FISH_SPEED_DEFAULT;
		self->ai_mood = AI_MOOD_STAND;
		SetAnim(self, ANIM_STAND1);
	}
}

// The fish finished a run swim cycle, shall we just randomly change direction or perhaps target a player or a bad guy? Or maybe just idle a bit.
void fish_runswim_finished(edict_t* self) //mxd. Named 'finished_swim' in original logic.
{
	if (self->ai_mood == AI_MOOD_PURSUE)
	{
		FishMoveToTarget(self);
		return;
	}

	const int chance = irand(0, 10);

	if (chance < 4)
	{
		self->ai_mood = ((irand(0, 3) == 0) ? AI_MOOD_STAND : AI_MOOD_WANDER); //TODO: the only difference between this and finished_swim().
		FishPickNewDirection(self);
	}
	else if (chance < 6)
	{
		self->speed = FISH_SPEED_DEFAULT;
		self->ai_mood = AI_MOOD_STAND;
		SetAnim(self, ANIM_STAND1);
	}
}

// The fish finished a pain cycle, shall we just randomly change direction or perhaps target a player or a bad guy? Or maybe just idle a bit.
void fish_pain_finished(edict_t* self) //mxd. Named 'finished_fish_pain' in original logic.
{
	// Run the hell away.
	self->ai_mood = AI_MOOD_WANDER;
	self->dead_state = DEAD_NO;

	if (self->waterlevel == 3)
		FishMoveToTarget(self);
}

// Decide whether to stay idling, or go walking somewhere.
void fish_idle(edict_t* self)
{
	if (self->ai_mood == AI_MOOD_PURSUE)
		FishMoveToTarget(self);
	else if (irand(0, 3) == 0)
		SetAnim(self, ANIM_STAND1);
	else
		FishPickNewDirection(self);
}

// Fish's dead, figure how far it is to the top of the water so he can float.
void fish_dead(edict_t* self)
{
	self->dead_state = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	VectorClear(self->velocity);

	self->think = M_DeadFloatThink; //mxd. fish_deadfloat() in original logic.
	self->nextthink = level.time + FRAMETIME; //mxd. Use define.

	// Stop the fish making bubbles.
	gi.RemoveEffects(&self->s, FX_WATER_BUBBLE);

	if (self->PersistantCFX > 0)
	{
		gi.RemovePersistantEffect(self->PersistantCFX, REMOVE_FISH);
		self->PersistantCFX = 0;
	}

	gi.linkentity(self);
}

// Fish bit the player - decide what to do.
void fish_bite(edict_t* self) //mxd. Named 'fishbite' in original logic.
{
	if (self->enemy == NULL || SV_CINEMATICFREEZE)
		return;

	vec3_t diff;
	VectorSubtract(self->s.origin, self->enemy->s.origin, diff);
	const float dist = VectorLength(diff);

	if (dist < self->maxs[0] + self->enemy->maxs[0] + FISH_BITE_DISTANCE) // Within 20 of bounding box.
	{
		gi.sound(self, CHAN_WEAPON, sounds[irand(SND_BITEHIT1, SND_BITEHIT2)], 1.0f, ATTN_NORM, 0.0f);

		VectorScale(diff, -3.0f, diff);
		VectorAdd(self->enemy->velocity, diff, self->enemy->velocity);

		T_Damage(self->enemy, self, self, vec3_origin, self->enemy->s.origin, vec3_origin, irand(FISH_DMG_BITE_MIN, FISH_DMG_BITE_MAX), 0, DAMAGE_DISMEMBER, MOD_DIED);

	}
	else // A miss.
	{
		gi.sound(self, CHAN_WEAPON, sounds[irand(SND_BITEMISS1, SND_BITEMISS2)], 1.0f, ATTN_NORM, 0.0f);
	}
}

void fish_update_target_movedir(edict_t* self) //mxd. Named 'fish_target' in original logic.
{
	if (self->enemy == NULL)
		return;

	// Figure out the vector from the fish to the target.
	vec3_t dir;
	VectorSubtract(self->enemy->s.origin, self->s.origin, dir);
	VectorNormalize(dir); // Normalize it.

	if (Vec3IsZero(dir))
		return;

	// Figure out the angles we want.
	AnglesFromDir(dir, self->movedir);
	VectorRadiansToDegrees(self->movedir, self->movedir);
}

// We are done attacking. What do we do now? Attack again?
void fish_pause(edict_t* self)
{
	// Is the target either not there or already dead?
	if (!FindTarget(self) || self->enemy->dead_state == DEAD_DEAD)
	{
		self->enemy = NULL;
		self->ai_mood = AI_MOOD_WANDER;
		FishPickBounceDirection(self);

		return;
	}

	vec3_t diff;
	VectorSubtract(self->s.origin, self->enemy->s.origin, diff);
	const float dist = VectorLength(diff);

	// We are close	enough to bite.
	if (dist < (self->maxs[0] + self->enemy->maxs[0] + FISH_BITE_DISTANCE))	// Within BITE_DIST of bounding box.
	{
		VectorClear(self->velocity);

		// If he's low on health, eat the bastard.
		if (self->enemy->health < self->enemy->max_health / 2)
		{
			self->ai_mood = AI_MOOD_ATTACK;
			SetAnim(self, ANIM_MELEE);

			return;
		}

		// Otherwise do a quick bite.
		if (irand(0, 4) == 0)
		{
			self->ai_mood = AI_MOOD_ATTACK;
			SetAnim(self, ANIM_BITE);
		}
		else // Randomly swim off anyway.
		{
			self->enemy = NULL;
			self->ai_mood = AI_MOOD_WANDER;
			FishPickBounceDirection(self);
		}

		return;
	}

	// Close enough to just zoom in on.
	if (dist < 120.0f)
	{
		FishMoveToTarget(self);
		return;
	}

	// Far enough that I break off.
	self->enemy = NULL;
	self->ai_mood = AI_MOOD_WANDER;
	FishPickBounceDirection(self);
}

// Shall we chase after someone?
void fish_chase(edict_t* self)
{
	// Shall we hunt someone?
	if (irand(0, 1) == 0 && FindTarget(self))
		FishMoveToTarget(self);
}

#pragma endregion

void FishStaticsInit(void)
{
	static ClassResourceInfo_t res_info; //mxd. Made local static.

	classStatics[CID_FISH].msgReceivers[MSG_PAIN] = FishPainMsgHandler;
	classStatics[CID_FISH].msgReceivers[MSG_DEATH] = FishDeadMsgHandler;
	classStatics[CID_FISH].msgReceivers[MSG_DEATH_PAIN] = FishDeadPainMsgHandler;

	res_info.numAnims = NUM_ANIMS;
	res_info.animations = animations;
	res_info.modelIndex = gi.modelindex("models/monsters/fish/tris.fm");

	sounds[SND_PAIN1] = gi.soundindex("monsters/fish/pain1.wav");
	sounds[SND_PAIN2] = gi.soundindex("monsters/fish/pain2.wav");
	sounds[SND_DIE] = gi.soundindex("monsters/fish/death1.wav");
	sounds[SND_GIB] = gi.soundindex("monsters/fish/gib.wav");
	sounds[SND_BITEHIT1] = gi.soundindex("monsters/fish/meleehit1.wav");
	sounds[SND_BITEHIT2] = gi.soundindex("monsters/fish/meleehit2.wav");
	sounds[SND_BITEMISS1] = gi.soundindex("monsters/fish/meleemiss1.wav");
	sounds[SND_BITEMISS2] = gi.soundindex("monsters/fish/meleemiss2.wav");
	//sounds[SND_GROWL1] = gi.soundindex("monsters/fish/growl1.wav"); //mxd. Unused.
	//sounds[SND_GROWL2] = gi.soundindex("monsters/fish/growl2.wav"); //mxd. Unused.
	//sounds[SND_GROWL3] = gi.soundindex("monsters/fish/growl3.wav"); //mxd. Unused.
	sounds[SND_SPLASH] = gi.soundindex("player/breaststroke.wav");

	sounds[SND_SLOW_SWIM1] = gi.soundindex("monsters/fish/fishmov3.wav");
	sounds[SND_SLOW_SWIM2] = gi.soundindex("monsters/fish/fishmov4.wav");
	sounds[SND_FAST_SWIM1] = gi.soundindex("monsters/fish/fishmov1.wav");
	sounds[SND_FAST_SWIM2] = gi.soundindex("monsters/fish/fishmov2.wav");

	res_info.numSounds = NUM_SOUNDS;
	res_info.sounds = sounds;

	classStatics[CID_FISH].resInfo = &res_info;
}

// QUAKED monster_fish (1 .5 0) (-25 -25 -14) (25 25 14)
// The fish.
// wakeup_target	- Monsters will fire this target the first time it wakes up (only once).
// pain_target		- Monsters will fire this target the first time it gets hurt (only once).
void SP_monster_fish(edict_t* self) //TODO: perform default initialization via M_Start(), remove SV_CHEATS check.
{
	if (DEATHMATCH && !(SV_CHEATS & self_spawn))
	{
		G_FreeEdict(self);
		return;
	}

	// Generic monster initialization.
	if (self->health == 0)
		self->health = FISH_HEALTH;

	// Apply to the end result (whether designer set or not).
	self->health = MonsterHealth(self->health);
	self->max_health = self->health;

	self->svflags |= (SVF_MONSTER | SVF_TAKE_NO_IMPACT_DMG | SVF_DO_NO_IMPACT_DMG);
	self->svflags &= ~SVF_DEADMONSTER;
	self->s.renderfx |= RF_FRAMELERP;
	self->s.effects |= EF_CAMERA_NO_CLIP;
	self->takedamage = DAMAGE_AIM;
	self->clipmask = MASK_MONSTERSOLID;
	self->materialtype = MAT_FLESH;
	self->flags |= (FL_SWIM | FL_NO_KNOCKBACK);
	self->s.skinnum = (irand(0, 1) == 1 ? FISH_SKIN1 : FISH_SKIN2);
	self->dead_state = DEAD_NO;

	//mxd. Missing in original logic.
	self->monsterinfo.thinkinc = MONSTER_THINK_INC;
	self->monsterinfo.nextframeindex = -1;

	self->ai_mood = AI_MOOD_STAND;
	self->fish_is_turning = false;
	self->gravity = 0.0f;
	self->best_move_yaw = 0.0f;
	self->wakeup_distance = 1024.0f;
	self->monsterinfo.aiflags |= (AI_NIGHTVISION | AI_NO_ALERT); // Pay no attention to alert ents.

	VectorCopy(self->s.origin, self->s.old_origin); //TODO: not needed?
	VectorCopy(self->s.angles, self->movedir);

	if (self->mass == 0)
		self->mass = FISH_MASS;

	self->s.frame = 0;
	self->oldenemy_debounce_time = -1.0f;

	self->msgHandler = DefaultMsgHandler;
	self->isBlocked = FishIsBlocked;
	self->think = FishThink;
	self->nextthink = level.time + FRAMETIME;

	self->yaw_speed = 11.0f;
	self->fish_max_pitch_speed = 4.0f;
	self->fish_speed_scaler = flrand(0.65f, 1.0f); // Random(ish) speed.
	self->fish_last_collision_surface = NULL;

	self->movetype = PHYSICSTYPE_STEP;
	VectorClear(self->knockbackvel);

	self->solid = SOLID_BBOX;
	self->s.modelindex = (byte)classStatics[CID_FISH].resInfo->modelIndex;

	if (self->s.scale == 1.0f)
	{
		self->s.scale = MODEL_SCALE * flrand(0.5f, 1.0f);
		self->monsterinfo.scale = self->s.scale;
	}

	VectorSet(self->mins, -16.0f, -16.0f, -8.0f);
	VectorSet(self->maxs,  16.0f,  16.0f,  8.0f);

	// Scale the maxs and mins according to scale of model.
	Vec3ScaleAssign(self->s.scale, self->mins);
	Vec3ScaleAssign(self->s.scale, self->maxs);

	// Give us the bubble spawner.
	self->PersistantCFX = gi.CreatePersistantEffect(&self->s, FX_WATER_BUBBLE, CEF_OWNERS_ORIGIN | CEF_BROADCAST, NULL, "");

	SetAnim(self, ANIM_STAND1);
	gi.linkentity(self);
	M_CatagorizePosition(self);

	//mxd. Avoid playing splash sound in M_WorldEffects()...
	if (self->waterlevel > 0)
		self->flags |= FL_INWATER;
	else
		gi.dprintf("%s spawned outside of water at %s!\n", self->classname, pv(self->s.origin));
}