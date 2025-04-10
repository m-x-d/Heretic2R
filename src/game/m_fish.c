//
// m_fish.c
//
// Copyright 1998 Raven Software
//

#include "m_fish.h"
#include "m_fish_shared.h"
#include "m_fish_anim.h"
#include "m_stats.h"
#include "g_debris.h" //mxd
#include "g_DefaultMessageHandler.h"
#include "g_monster.h"
#include "Random.h"
#include "Utilities.h"
#include "Vector.h"
#include "g_local.h"

static void fish_hunt(edict_t *self); //TODO: remove

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

// Choose a run animation to use.
void fish_run(edict_t* self)
{
	const float delta = anglemod(self->s.angles[YAW] - self->movedir[YAW]);

	if (delta > 70.0f && delta <= 180.0f) // Look right.
	{
		// Tell the think function we are doing the turn, so don't play with the yaw.
		self->ai_mood_flags = 1;
		self->best_move_yaw = -FISH_RUN_TURN_ANGLE;
		SetAnim(self, ANIM_RUN3);
	}
	else if (delta > 180.0f && delta < 290.0f) // Look left.
	{
		// Tell the think function we are doing the turn, so don't play with the yaw.
		self->ai_mood_flags = 1;
		self->best_move_yaw = FISH_RUN_TURN_ANGLE;
		SetAnim(self, ANIM_RUN2);
	}
	else
	{
		// Tell the think function we are NOT doing the turn.
		self->ai_mood_flags = 0;
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
		self->ai_mood_flags = 1;
		self->best_move_yaw = -FISH_WALK_TURN_ANGLE;
		SetAnim(self, ANIM_WALK3);
	}
	else if (delta > 180.0f && delta < 320.0f) // Look left. //BUGFIX: mxd. 'delta > 180 && delta < 20' in original logic (e.g. never).
	{
		// Tell the think function we are doing the turn, so don't play with the yaw.
		self->ai_mood_flags = 1;
		self->best_move_yaw = FISH_WALK_TURN_ANGLE;
		SetAnim(self, ANIM_WALK2);
	}
	else
	{
		// Tell the think function we are NOT doing the turn.
		self->ai_mood_flags = 0;
		SetAnim(self, ANIM_WALK1);
	}
}

// Update the yaw on the first frame of a new animation - stop jittering.
void fish_update_yaw(edict_t* self)
{
	self->s.angles[YAW] += self->best_move_yaw;
	self->best_move_yaw = 0.0f;
}

// Generic 'decided on a new direction' reaction - make us select a new direction.
static void FishPickNewDirection(edict_t* self) //mxd. Named 'fish_new_direction' in original logic.
{
	self->movedir[PITCH] = ((irand(0, 1) == 0) ? flrand(-30.0f, 30.0f) : 0.0f);
	self->movedir[YAW] += flrand(-45.0f, 45.0f);

	// Bring all our movedir angles up positive again.
	FishResetMovedir(self);

	// If we change direction, we might hit the same poly we just collided with.
	self->shrine_type = 0;

	// Decide which animation to use.
	if (self->ai_mood == AI_MOOD_WANDER)
	{
		self->speed = self->old_yaw * FISH_SPEED_FAST;
		fish_run(self);
	}
	else
	{
		self->speed = self->old_yaw * FISH_SPEED_SLOW;
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
		self->speed = self->old_yaw * FISH_SPEED_FAST;
		fish_run(self);
	}
	else
	{
		self->speed = self->old_yaw * FISH_SPEED_SLOW;
		fish_walk(self);
	}
}

static float FishChangeYaw(edict_t* self) //mxd. Named 'M_ChangeFishYaw' in original logic. Very similar to MG_ChangeWhichYaw().
{
	const float current = anglemod(self->s.angles[YAW]);
	const float ideal = self->movedir[YAW];
	float move = ideal - current;

	if (FloatIsZeroEpsilon(move)) //mxd. Avoid direct float comparison.
		return 0.0f;

	if (ideal > current)
	{
		if (move >= 180.0f)
			move -= 360.0f;
	}
	else
	{
		if (move <= -180.0f)
			move += 360.0f;
	}

	move = Clamp(move, -self->yaw_speed, self->yaw_speed);
	self->s.angles[YAW] = anglemod(current + move);

	return move;
}

static float FishChangePitch(edict_t* self) //mxd. Named 'M_ChangeFishPitch' in original logic. Very similar to MG_ChangeWhichYaw().
{
	const float current = anglemod(self->s.angles[PITCH]);
	const float ideal = self->movedir[PITCH];
	float move = ideal - current;

	if (FloatIsZeroEpsilon(move)) //mxd. Avoid direct float comparison.
		return 0.0f;

	if (ideal > current)
	{
		if (move >= 180.0f)
			move -= 360.0f;
	}
	else
	{
		if (move <= -180.0f)
			move += 360.0f;
	}

	move = Clamp(move, -self->dmg_radius, self->dmg_radius);
	self->s.angles[PITCH] = anglemod(current + move);

	return move;
}

static void FishThink(edict_t* self) //mxd. Named 'fish_think' in original logic.
{
	// Determine if we are too far from the camera to warrant animating or AI.
	if (!gi.CheckDistances(self->s.origin, FISH_ACTIVATE_DISTANCE)) //mxd. Merged fish_check_distance() logic.
	{
		VectorClear(self->velocity);
		self->nextthink = level.time + 2.0f;

		return;
	}

	self->nextthink = level.time + FRAMETIME;

	if (self->enemy == NULL)
		FindTarget(self);

	if (self->enemy != NULL && self->enemy->waterlevel == 0) // Let's not hunt things out of water!
		self->enemy = NULL;

	// Animate us.
	M_MoveFrame(self);

	// We are already dead or getting hit, we don't need to do anything.
	if ((self->deadflag & DEAD_DEAD) || (self->deadflag & DEAD_DYING))
		return;

	M_CatagorizePosition(self);

	// Did we break the water surface?
	if (self->waterlevel < 3)
	{
		// If we break water - don't let us target anyone anymore.
		self->enemy = NULL;
		self->ai_mood = AI_MOOD_WANDER;
		self->dmg_radius = 10;

		// Make us go down good sir!
		self->movedir[PITCH] = flrand(-35.0f, -15.0f);

		// Only allow one of these every second for this fish.
		if (self->count == 0) //TODO: Add 'fish_ripple_spawned' name.
		{
			// Create a ripple.
			vec3_t top;
			VectorCopy(self->s.origin, top);
			top[2] += self->maxs[2] * 0.75f;

			vec3_t bottom;
			VectorCopy(self->s.origin, bottom);
			bottom[2] += self->mins[2];

			trace_t trace;
			gi.trace(top, vec3_origin, vec3_origin, bottom, self, MASK_WATER, &trace);

			if (trace.fraction <= 1.0f)
			{
				// No ripples while in cinematics.
				if (!SV_CINEMATICFREEZE)
				{
					vec3_t dir;
					AngleVectors(self->s.angles, dir, NULL, NULL);
					VectorScale(dir, 200.0f, dir);

					const byte b_angle = (byte)Q_ftol((self->s.angles[YAW] + DEGREE_180) / 360.0f * 255.0f);

					gi.CreateEffect(NULL, FX_WATER_WAKE, 0, trace.endpos, "sbv", self->s.number, b_angle, dir);
				}

				gi.sound(self, CHAN_WEAPON, sounds[SND_SPLASH], 1.0f, ATTN_NORM, 0.0f);
				self->count = 1;
			}
		}
	}
	else
	{
		self->count = 0;
		self->dmg_radius = 4;
	}

	// Make sure that the movedir angles are between 0-359, or we are in trouble on the pitch and yaw routines.
	FishResetMovedir(self);

	// Change pitch if we should.
	FishChangePitch(self);

	// Move us from one angle to another slowly - unless we are moving through the "turn" anims, in which case the anim takes care of the YAW.
	vec3_t angles;
	VectorDegreesToRadians(self->s.angles, angles);

	if (!self->ai_mood_flags)
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

// The fish hit something.
static void FishIsBlocked(edict_t* self, struct trace_s* trace) //mxd. Named 'fish_blocked' in original logic.
{
	// Dead fish don't rebound off stuff.
	if (self->deadflag == DEAD_DEAD)
		return;

	// Did we hit a monster or player?
	if (trace->ent != NULL && ((trace->ent->svflags & SVF_MONSTER) || trace->ent->client != NULL))
	{
		// Hit another fish - send us on our way.
		if (trace->ent->classID == CID_FISH)
		{
			FishPickBounceDirection(self);
			return;
		}

		// Check if this guy is dead.
		if (trace->ent->deadflag == DEAD_DEAD)
		{
			FishPickBounceDirection(self);
			self->enemy = NULL;

			return;
		}

		// Not dead, so lets BITE THE BASTARD :)
		self->enemy = trace->ent;

		vec3_t diff;
		VectorSubtract(self->s.origin, trace->ent->s.origin, diff);
		const float dist = VectorLength(diff);

		if (dist < self->maxs[0] + self->enemy->maxs[0] + FISH_BITE_DISTANCE + 50.0f && (self->dmg_radius == 4)) // Within 20 of bounding box & not out of water.
		{
			SetAnim(self, ANIM_BITE);
			self->ai_mood = AI_MOOD_ATTACK;
		}
		else
		{
			fish_hunt(self);
		}

		return;
	}

	// Did we hit a model of some type?
	if (trace->ent != NULL)
	{
		FishPickBounceDirection(self);
		return;
	}

	// Did we hit the same wall as last time? Because if we did, we already dealt with it.
	if ((int)trace->surface != self->shrine_type)
	{
		self->shrine_type = (int)trace->surface;
		FishPickBounceDirection(self);
	}
}

// The fish finished a walk swim cycle, shall we just randomly change direction or perhaps target a player or a bad guy? Or maybe just idle a bit.
void finished_swim(edict_t* self) //TODO: rename to fish_walkswim_finished.
{
	if (self->ai_mood == AI_MOOD_PURSUE)
	{
		fish_hunt(self);
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
void finished_runswim(edict_t* self) //TODO: rename to fish_runswim_finished.
{
	if (self->ai_mood == AI_MOOD_PURSUE)
	{
		fish_hunt(self);
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
void finished_fish_pain(edict_t* self) //TODO: rename to fish_pain_finished.
{
	// Run the hell away.
	self->ai_mood = AI_MOOD_WANDER;
	self->deadflag = DEAD_NO;

	if (self->waterlevel == 3)
		fish_hunt(self);
}

// Decide whether to stay idling, or go walking somewhere.
void fish_idle(edict_t* self)
{
	if (self->ai_mood == AI_MOOD_PURSUE)
		fish_hunt(self);
	else if (irand(0, 3) == 0)
		SetAnim(self, ANIM_STAND1);
	else
		FishPickNewDirection(self);
}

static void FishDeadPainMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'fish_dead_pain' in original logic.
{
	if (self->health < -60)
		BecomeDebris(self); //TODO: also play SND_GIB sound?
}

static void FishDeadMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'fish_death' in original logic.
{
	VectorClear(self->velocity);
	self->deadflag = DEAD_DEAD;

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
	ParseMsgParms(msg, "eeiii", &temp, &temp, &force_pain, &damage, &temp);

	if (!force_pain && irand(0, 3) == 0) //mxd. flrand() in original logic.
		return;

	SetAnim(self, ANIM_PAIN1);
	VectorClear(self->velocity);
	self->deadflag = DEAD_DYING;

	// Switch to damaged skin?
	if (irand(0, 2) == 0 && (self->s.skinnum == FISH_SKIN1 || self->s.skinnum == FISH_SKIN2))
		self->s.skinnum += 1;

	gi.sound(self, CHAN_WEAPON, sounds[irand(SND_PAIN1, SND_PAIN2)], 1.0f, ATTN_NORM, 0.0f);
}

static void FishDeadBobThink(edict_t* self) //mxd. Named 'fish_deadbob' in original logic.
{
	if (self->velocity[2] > 0.0f)
	{
		if (self->s.origin[2] > self->monsterinfo.misc_debounce_time + flrand(3.0f, 6.0f)) // So it doesn't always go to the same height.
			self->velocity[2] = flrand(-7.0f, -2.0f);
	}
	else
	{
		if (self->s.origin[2] < self->monsterinfo.misc_debounce_time)
			self->velocity[2] = flrand(2.0f, 7.0f);
	}

	self->nextthink = level.time + 0.2f;
}

// Make the fish float to the surface.
void FishDeadFloatThink(edict_t* self) //mxd. Named 'fish_deadfloat' in original logic.
{
	M_CatagorizePosition(self);

	if (self->waterlevel == 3) // Below water surface.
	{
		if (self->velocity[2] < 10.0f)
			self->velocity[2] += 10.0f;
		else
			self->velocity[2] = 20.0f; // Just in case something blocked it going up.
	}
	else if (self->waterlevel < 2) // Above water surface.
	{
		if (self->velocity[2] > -150.0f)
			self->velocity[2] -= 50.0f; // Fall back in now!
		else
			self->velocity[2] = -200.0f;
	}
	else // // On water surface (waterlevel == 2).
	{
		self->monsterinfo.misc_debounce_time = self->s.origin[2]; //TODO: use edict_s prop, add custom name.
		self->think = FishDeadBobThink;
	}

	self->nextthink = level.time + FRAMETIME; //mxd. Use define.
}

//----------------------------------------------------------------------
//  Fish Dead - he's dead, figure how far it is to the top of the water so he can float
//----------------------------------------------------------------------
void fish_dead(edict_t *self)
{
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	VectorClear(self->velocity);

	self->think = FishDeadFloatThink;
	self->nextthink = level.time + 0.1;
			
	// stop the fish making bubbles
	gi.RemoveEffects(&self->s, FX_WATER_BUBBLE);
	if (self->PersistantCFX)
	{
		gi.RemovePersistantEffect(self->PersistantCFX, REMOVE_FISH);
		self->PersistantCFX = 0;
	}

	gi.linkentity (self);
}

// he bit the player - decide what to do
void fishbite (edict_t *self)
{
	vec3_t	v;
	float	scale;
	float	len;

	if (!self->enemy || sv_cinematicfreeze->value)
		return;

	VectorSubtract (self->s.origin, self->enemy->s.origin, v);
	len = VectorLength (v);

	if (len < (self->maxs[0] + self->enemy->maxs[0] + FISH_BITE_DISTANCE))	// Within 20 of bounding box
	{
		if (irand(0, 1))
		{
			gi.sound (self, CHAN_WEAPON, sounds[SND_BITEHIT1], 1, ATTN_NORM, 0);
		}	
		else
		{
			gi.sound (self, CHAN_WEAPON, sounds[SND_BITEHIT2], 1, ATTN_NORM, 0);
		}

		scale = -3;
		VectorScale(v, scale, v);
		VectorAdd (self->enemy->velocity, v, self->enemy->velocity);

		T_Damage (self->enemy, self, self, vec3_origin, self->enemy->s.origin, vec3_origin, irand(FISH_DMG_BITE_MIN, FISH_DMG_BITE_MAX) , 0, DAMAGE_DISMEMBER,MOD_DIED);

	}
	else			// A misssss
	{
		if (irand(0, 1))
		{
			gi.sound (self, CHAN_WEAPON, sounds[SND_BITEMISS1], 1, ATTN_NORM, 0);
		}
		else
		{
			gi.sound (self, CHAN_WEAPON, sounds[SND_BITEMISS2], 1, ATTN_NORM, 0);
		}
	}
}

void fish_target(edict_t *self)
{
	vec3_t	dir;

	if (self->enemy)
	{
		// figure out the vector from the fish to the target
		VectorSubtract(self->enemy->s.origin, self->s.origin, dir);
		// normalise it
		Vec3Normalize(dir);
		if(Vec3IsZero(dir))
			return;
		// figure out the angles we want
		AnglesFromDir(dir, self->movedir);
		VectorRadiansToDegrees (self->movedir, self->movedir);
	}
}

// figure out where our prey is, and go get him
static void fish_hunt(edict_t *self)
{

	// make sure we still have a target - bouncing off stuff tends to clear it out
	if (!self->enemy)
	{
		FindTarget(self);
		// if we can't find one, let him just swim on alone..
		if (!self->enemy)
		{
			if(self->curAnimID == ANIM_PAIN1)
			{
				self->speed = 20;
	  			self->ai_mood = AI_MOOD_STAND;
				SetAnim(self, ANIM_STAND1);
			}
			return;
		}
	}

	fish_target(self);
	// set movement type
	self->ai_mood = AI_MOOD_PURSUE;
	//	make us run after it
	self->speed = FISH_SPEED_HUNT * self->old_yaw;
	fish_run(self);
}

// we are done attacking.. what do we do now ? attack again ?
void fish_pause (edict_t *self)
{
	vec3_t	v;
	float	len;

	FindTarget(self);

	// is the target either not there or already dead ?
	if (!self->enemy || self->enemy->deadflag == DEAD_DEAD)
	{
		self->enemy = NULL;
		self->ai_mood = AI_MOOD_WANDER;
		FishPickBounceDirection(self);
		return;//right?  crashes if not!
	}

	VectorSubtract (self->s.origin, self->enemy->s.origin, v);
	len = VectorLength (v);

	// we are close	enough to bite
	if (len < (self->maxs[0] + self->enemy->maxs[0] + FISH_BITE_DISTANCE))	// Within BITE_DIST of bounding box
	{
		VectorClear(self->velocity);
		// if he's low on health, eat the bastard..
		if (self->enemy->health < (self->enemy->max_health / 2))
		{
			SetAnim(self, ANIM_MELEE);
			self->ai_mood = AI_MOOD_ATTACK;
		}
		// other wise a quick bite
		else
		{
			// randomly swim off anyway
			if (!irand(0,4))
			{
				self->ai_mood = AI_MOOD_ATTACK;
				SetAnim(self, ANIM_BITE);
			}
			else
			{
				self->enemy = NULL;
				self->ai_mood = AI_MOOD_WANDER;
				FishPickBounceDirection(self);
			}
		}
	}

	else
	{
		if (len < 120)  // close enough to just zoom in on
		{
			fish_hunt(self);
		}
		else	// far enough that I break off..
		{
		  	self->enemy = NULL;
			self->ai_mood = AI_MOOD_WANDER;
			FishPickBounceDirection(self);
		}
	}
}



// shall we chase after someone ?
void fish_chase(edict_t *self)
{

	// shall we hunt someone ?
	if (irand(0,1))
		return;

	// find a target to chase after
	FindTarget(self);

	// if we got one..
	if (self->enemy)
		fish_hunt(self);
}

/*----------------------------------------------------------------------

  SOUND FUNCTIONS FOR THE FISH

-----------------------------------------------------------------------*/

// random growl
void fish_growl (edict_t *self)
{
	int chance;

	return;
	chance = irand(0, 200);

	if (chance > 60)
	{
	}
	else if (chance < 20 )
	{
		gi.sound (self, CHAN_WEAPON, sounds[SND_GROWL1], 1, ATTN_NORM, 0);
	}
	else if (chance < 40)
	{
		gi.sound (self, CHAN_WEAPON, sounds[SND_GROWL2], 1, ATTN_NORM, 0);
	}
	else
	{
		gi.sound (self, CHAN_WEAPON, sounds[SND_GROWL3], 1, ATTN_NORM, 0);
	}
}


void FishStaticsInit(void)
{
	static ClassResourceInfo_t res_info; //mxd. Made local static.

	classStatics[CID_FISH].msgReceivers[MSG_PAIN] = FishPainMsgHandler;
	classStatics[CID_FISH].msgReceivers[MSG_DEATH] = FishDeadMsgHandler;
	classStatics[CID_FISH].msgReceivers[MSG_DEATH_PAIN] = FishDeadPainMsgHandler;

	res_info.numAnims = NUM_ANIMS;
	res_info.animations = animations;
	res_info.modelIndex = gi.modelindex("models/monsters/fish/tris.fm");

	sounds[SND_PAIN1] = gi.soundindex ("monsters/fish/pain1.wav");	
	sounds[SND_PAIN2] = gi.soundindex ("monsters/fish/pain2.wav");	
	sounds[SND_DIE] = gi.soundindex ("monsters/fish/death1.wav");	
	sounds[SND_GIB] = gi.soundindex ("monsters/fish/gib.wav");
	sounds[SND_BITEHIT1] = gi.soundindex ("monsters/fish/meleehit1.wav");	
	sounds[SND_BITEHIT2] = gi.soundindex ("monsters/fish/meleehit2.wav");	
	sounds[SND_BITEMISS1] = gi.soundindex ("monsters/fish/meleemiss1.wav");	
	sounds[SND_BITEMISS2] = gi.soundindex ("monsters/fish/meleemiss2.wav");	
	sounds[SND_GROWL1] = gi.soundindex ("monsters/fish/growl1.wav");	
	sounds[SND_GROWL2] = gi.soundindex ("monsters/fish/growl2.wav");	
	sounds[SND_GROWL3] = gi.soundindex ("monsters/fish/growl3.wav");
	sounds[SND_SPLASH] = gi.soundindex("player/breaststroke.wav");

	sounds[SND_SLOW_SWIM1] = gi.soundindex("monsters/fish/fishmov3.wav");
	sounds[SND_SLOW_SWIM2] = gi.soundindex("monsters/fish/fishmov4.wav");
	sounds[SND_FAST_SWIM1] = gi.soundindex("monsters/fish/fishmov1.wav");
	sounds[SND_FAST_SWIM2] = gi.soundindex("monsters/fish/fishmov2.wav");

	res_info.numSounds = NUM_SOUNDS;
	res_info.sounds = sounds;

	classStatics[CID_FISH].resInfo = &res_info;
}

/*QUAKED monster_fish (1 .5 0) (-25 -25 -14) (25 25 14) 

The fish

"wakeup_target" - monsters will fire this target the first time it wakes up (only once)

"pain_target" - monsters will fire this target the first time it gets hurt (only once)
*/

void SP_monster_fish (edict_t *self)
{
	if ((deathmatch->value == 1) && !((int)sv_cheats->value & self_spawn))
	{
		G_FreeEdict (self);
		return;
	}

	// Generic Monster Initialization

	if (!self->health)
		self->health = FISH_HEALTH;

	//Apply to the end result (whether designer set or not)
	self->max_health = self->health = MonsterHealth(self->health);

	self->nextthink = level.time + FRAMETIME;
	self->svflags |= SVF_MONSTER | SVF_TAKE_NO_IMPACT_DMG | SVF_DO_NO_IMPACT_DMG;
	self->svflags &= ~SVF_DEADMONSTER;
	self->s.renderfx |= RF_FRAMELERP;
	self->takedamage = DAMAGE_AIM;
	self->max_health = self->health;
	self->clipmask = MASK_MONSTERSOLID;
	self->materialtype = MAT_FLESH;
	self->flags |= FL_SWIM|FL_NO_KNOCKBACK;
	
	self->s.effects|=EF_CAMERA_NO_CLIP;

	// random skin of three
	if(irand(0, 1))
		self->s.skinnum = FISH_SKIN1;
	else
		self->s.skinnum = FISH_SKIN2;

	self->deadflag = DEAD_NO;
	self->isBlocked = FishIsBlocked;
	self->ai_mood = AI_MOOD_STAND;
	self->ai_mood_flags = 0;
	self->gravity = self->best_move_yaw = 0;
	self->wakeup_distance = 1024;
	self->monsterinfo.aiflags |= AI_NIGHTVISION;

	self->monsterinfo.aiflags |= AI_NO_ALERT;//pay no attention to alert ents

	VectorCopy (self->s.origin, self->s.old_origin);
	VectorCopy (self->s.angles, self->movedir);

	if (!self->mass)
		self->mass = FISH_MASS;

	self->s.frame = 0;

	self->oldenemy_debounce_time = -1;
	
	self->msgHandler = DefaultMsgHandler;
	self->think = FishThink;
	self->nextthink = level.time + FRAMETIME;

	self->yaw_speed = 11;
	self->dmg_radius = 4;
	// random(ish) speed
	self->old_yaw = flrand(0.65,1.0); //TODO: part of union, add fish-specific name?

	self->movetype=PHYSICSTYPE_STEP;
	VectorClear(self->knockbackvel);

	self->solid=SOLID_BBOX;

	self->s.modelindex = classStatics[CID_FISH].resInfo->modelIndex;

	self->shrine_type = 0;
	
	if (self->s.scale == 1)
		self->s.scale = self->monsterinfo.scale = MODEL_SCALE * flrand(0.5,1.0);

	VectorSet(self->mins, -16, -16, -8);
	VectorSet(self->maxs, 16, 16, 8);

	// scale the max's and mins according to scale of model
	Vec3ScaleAssign(self->s.scale, self->mins);
	Vec3ScaleAssign(self->s.scale, self->maxs);

	// give us the bubble spawner
 	self->PersistantCFX = gi.CreatePersistantEffect(&self->s,
 												FX_WATER_BUBBLE,
 												CEF_OWNERS_ORIGIN | CEF_BROADCAST,
 												NULL,
												"");

	SetAnim(self, ANIM_STAND1);

	gi.linkentity(self); 

	M_CatagorizePosition(self);
}
