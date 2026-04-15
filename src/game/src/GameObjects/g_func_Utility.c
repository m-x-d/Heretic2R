//
// g_func_Utility.c -- Originally part of g_func.c
//
// Copyright 1998 Raven Software
//

#include "g_func_Utility.h"
#include "Vector.h"

#pragma region ====================== Support routines for movement (changes in origin using velocity) ======================

void MoveDone(edict_t* ent) //mxd. Named 'Move_Done' in original logic.
{
	VectorClear(ent->velocity);
	ent->think = NULL;

	if (ent->moveinfo.endfunc != NULL)
		ent->moveinfo.endfunc(ent);
}

void MoveFinal(edict_t* ent) //mxd. Named 'Move_Final' in original logic.
{
	VectorScale(ent->moveinfo.dir, ent->moveinfo.remaining_distance / FRAMETIME, ent->velocity);

	ent->think = MoveDone;
	ent->nextthink = level.time + FRAMETIME;
}

void MoveBegin(edict_t* ent) //mxd. Named 'Move_Begin' in original logic.
{
	if ((ent->moveinfo.speed * FRAMETIME) >= ent->moveinfo.remaining_distance)
	{
		MoveFinal(ent);
		return;
	}

	VectorScale(ent->moveinfo.dir, ent->moveinfo.speed, ent->velocity);

	const float frames = floorf(ent->moveinfo.remaining_distance / ent->moveinfo.speed / FRAMETIME);
	ent->moveinfo.remaining_distance -= frames * ent->moveinfo.speed * FRAMETIME;
	ent->nextthink = level.time + frames * FRAMETIME;
	ent->think = MoveFinal;
}

void MoveCalc(edict_t* ent, const vec3_t dest, void (*func)(edict_t*)) //mxd. Named 'Move_Calc' in original logic.
{
	VectorClear(ent->velocity);

	VectorSubtract(dest, ent->s.origin, ent->moveinfo.dir);
	ent->moveinfo.remaining_distance = VectorNormalize(ent->moveinfo.dir);
	ent->moveinfo.endfunc = func;

	if (ent->moveinfo.speed == ent->moveinfo.accel && ent->moveinfo.speed == ent->moveinfo.decel)
	{
		if (level.current_entity == ((ent->flags & FL_TEAMSLAVE) ? ent->teammaster : ent))
		{
			MoveBegin(ent);
		}
		else
		{
			ent->nextthink = level.time + FRAMETIME;
			ent->think = MoveBegin;
		}
	}
	else
	{
		// Accelerative.
		ent->moveinfo.current_speed = 0.0f;
		ent->nextthink = level.time + FRAMETIME;
		ent->think = AccelMoveThink;
	}
}

#pragma endregion

#pragma region ================== Support routines for angular movement of trains (changes in angle using avelocity) ==================

void FuncTrainAngleMoveCalc(edict_t* self, const edict_t* ent, const vec3_t dest) //mxd. Named 'TrainAngleMove_Calc' in original logic.
{
	VectorClear(self->avelocity);

	if (ent == NULL || Vec3IsZero(ent->s.angles))
		return;

	// Put angles into wacky order (required).
	const vec3_t angles = { ent->s.angles[1], ent->s.angles[2], ent->s.angles[0] };

	// Length to travel.
	const float len = VectorSeparation(self->s.origin, dest);

	// Divide by speed to get time to reach dest.
	const float travel_time = (len / self->moveinfo.speed) + self->moveinfo.wait;

	if (travel_time < FRAMETIME)
	{
		VectorScale(angles, 1.0f / FRAMETIME, self->avelocity);
	}
	else
	{
		// Scale the destdelta vector by the time spent traveling to get velocity.
		// 0.97 is used because we want the train to change angles a little slow, that way it never overshoots the angle it should be at.
		// In train_next the final angle is set using en_angles.
		const float hold_time = 0.97f / travel_time;
		VectorScale(angles, hold_time, self->avelocity);
	}

	VectorAdd(self->s.angles, angles, self->moveinfo.end_angles);
}

void AngleMoveDone(edict_t* ent) //mxd. Named 'AngleMove_Done' in original logic.
{
	VectorClear(ent->avelocity);
	ent->think = NULL;

	if (ent->moveinfo.endfunc != NULL)
		ent->moveinfo.endfunc(ent);
}

void AngleMoveFinal(edict_t* ent) //mxd. Named 'AngleMove_Final' in original logic.
{
	vec3_t move;
	const vec3_t* src_angles = ((ent->moveinfo.state == STATE_UP) ? &ent->moveinfo.end_angles : &ent->moveinfo.start_angles); //mxd
	VectorSubtract(*src_angles, ent->s.angles, move);

	VectorScale(move, 1.0f / FRAMETIME, ent->avelocity);

	ent->nextthink = level.time + FRAMETIME;
	ent->think = AngleMoveDone;
}

void AngleMoveBegin(edict_t* ent) //mxd. Named 'AngleMove_Begin' in original logic.
{
	// Set dest_delta to the vector needed to move.
	vec3_t dest_delta;
	const vec3_t* src_angles = ((ent->moveinfo.state == STATE_UP) ? &ent->moveinfo.end_angles : &ent->moveinfo.start_angles); //mxd
	VectorSubtract(*src_angles, ent->s.angles, dest_delta);

	// Calculate length of vector.
	const float len = VectorLength(dest_delta);

	// Divide by speed to get time to reach dest.
	const float travel_time = len / ent->moveinfo.speed;

	if (travel_time < FRAMETIME)
	{
		AngleMoveFinal(ent);
		return;
	}

	// Scale the dest_delta vector by the time spent traveling to get velocity.
	VectorScale(dest_delta, 1.0f / travel_time, ent->avelocity);

	// Set nextthink to trigger a think when dest is reached.
	const float frames = floorf(travel_time / FRAMETIME);
	ent->nextthink = level.time + frames * FRAMETIME;
	ent->think = AngleMoveFinal;
}

void AngleMoveCalc(edict_t* ent, void (*func)(edict_t*)) //mxd. Named 'AngleMove_Calc' in original logic.
{
	VectorClear(ent->avelocity);
	ent->moveinfo.endfunc = func;

	if (level.current_entity == ((ent->flags & FL_TEAMSLAVE) ? ent->teammaster : ent))
	{
		AngleMoveBegin(ent);
	}
	else
	{
		ent->nextthink = level.time + FRAMETIME;
		ent->think = AngleMoveBegin;
	}
}

#pragma endregion

#pragma region ========================== AccelMoveThink ==========================

static float AccelerationDistance(const float target, const float rate) //mxd. #define in original logic.
{
	return target * ((target / rate) + 1.0f) / 2.0f;
}

static void FuncPlatCalcAcceleratedMove(moveinfo_t* info) //mxd. Named 'plat_CalcAcceleratedMove' in original logic.
{
	info->move_speed = info->speed;

	if (info->remaining_distance < info->accel)
	{
		info->current_speed = info->remaining_distance;
		return;
	}

	const float accel_dist = AccelerationDistance(info->speed, info->accel);
	float decel_dist = AccelerationDistance(info->speed, info->decel);

	if ((info->remaining_distance - accel_dist - decel_dist) < 0.0f)
	{
		const float f = (info->accel + info->decel) / (info->accel * info->decel);
		info->move_speed = (-2.0f + sqrtf(4.0f - (4.0f * f) * (-2.0f * info->remaining_distance))) / (2.0f * f);
		decel_dist = AccelerationDistance(info->move_speed, info->decel);
	}

	info->decel_distance = decel_dist;
}

static void FuncPlatAccelerate(moveinfo_t* info) //mxd. Named 'plat_Accelerate' in original logic.
{
	// Are we decelerating?
	if (info->remaining_distance <= info->decel_distance)
	{
		if (info->remaining_distance < info->decel_distance)
		{
			if (info->next_speed != 0.0f)
			{
				info->current_speed = info->next_speed;
				info->next_speed = 0.0f;
			}
			else if (info->current_speed > info->decel)
			{
				info->current_speed -= info->decel;
			}
		}

		return;
	}

	// Are we at full speed and need to start decelerating during this move?
	if (info->current_speed == info->move_speed)
	{
		if ((info->remaining_distance - info->current_speed) < info->decel_distance)
		{
			const float p1_distance = info->remaining_distance - info->decel_distance;
			const float p2_distance = info->move_speed * (1.0f - (p1_distance / info->move_speed));
			const float distance = p1_distance + p2_distance;
			info->current_speed = info->move_speed;
			info->next_speed = info->move_speed - info->decel * (p2_distance / distance);

			return;
		}
	}

	// Are we accelerating?
	if (info->current_speed < info->speed)
	{
		const float old_speed = info->current_speed;

		// Figure simple acceleration up to move_speed.
		info->current_speed += info->accel;
		info->current_speed = min(info->speed, info->current_speed);

		// Are we accelerating throughout this entire move?
		if (info->remaining_distance - info->current_speed >= info->decel_distance)
			return;

		// During this move we will accelerate from current_speed to move_speed and cross over the decel_distance;
		// Figure the average speed for the entire move.
		const float p1_distance = info->remaining_distance - info->decel_distance;
		const float p1_speed = (old_speed + info->move_speed) / 2.0f;
		const float p2_distance = info->move_speed * (1.0f - (p1_distance / p1_speed));
		const float distance = p1_distance + p2_distance;
		info->current_speed = (p1_speed * (p1_distance / distance)) + (info->move_speed * (p2_distance / distance));
		info->next_speed = info->move_speed - info->decel * (p2_distance / distance);
	}

	// We are at constant velocity (move_speed).
}

// The team has completed a frame of movement, so change the speed for the next frame.
void AccelMoveThink(edict_t* ent) //mxd. Named 'Think_AccelMove' in original logic.
{
	ent->moveinfo.remaining_distance -= ent->moveinfo.current_speed;

	if (ent->moveinfo.current_speed == 0.0f) // Starting or blocked.
		FuncPlatCalcAcceleratedMove(&ent->moveinfo);

	FuncPlatAccelerate(&ent->moveinfo);

	// Will the entire move complete on next frame?
	if (ent->moveinfo.remaining_distance <= ent->moveinfo.current_speed)
	{
		MoveFinal(ent);
	}
	else
	{
		VectorScale(ent->moveinfo.dir, ent->moveinfo.current_speed * 10.0f, ent->velocity);
		ent->nextthink = level.time + FRAMETIME;
		ent->think = AccelMoveThink;
	}
}

#pragma endregion

#pragma region ====================== Movement sounds ======================

void FuncPlayMoveStartSound(edict_t* ent) //mxd. Added to reduce code duplication.
{
	if (!(ent->flags & FL_TEAMSLAVE))
	{
		if (ent->moveinfo.sound_start > 0)
			gi.sound(ent, CHAN_NO_PHS_ADD + CHAN_VOICE, ent->moveinfo.sound_start, 1.0f, ATTN_IDLE, 0.0f);

		ent->s.sound = (byte)ent->moveinfo.sound_middle;
		ent->s.sound_data = (255 & ENT_VOL_MASK) | ATTN_IDLE;
	}
}

void FuncPlayMoveEndSound(edict_t* ent) //mxd. Added to reduce code duplication.
{
	if (!(ent->flags & FL_TEAMSLAVE))
	{
		if (ent->moveinfo.sound_end > 0)
			gi.sound(ent, CHAN_NO_PHS_ADD + CHAN_VOICE, ent->moveinfo.sound_end, 1.0f, ATTN_IDLE, 0.0f);

		ent->s.sound = 0;
	}
}

void FuncHandleCrushingSounds(const edict_t* ent, const edict_t* other) //mxd
{
	edict_t* master = ent->teammaster;

	// When crushed player, stop movement sound (because player's entity won't be despawned in SP and will linger until respawned in COOP/DM).
	if (other->client != NULL && other->health < 1 && master->s.sound == master->moveinfo.sound_middle)
	{
		if (master->moveinfo.sound_end > 0)
			gi.sound(ent, CHAN_NO_PHS_ADD + CHAN_VOICE, master->moveinfo.sound_end, 1.0f, ATTN_IDLE, 0.0f);

		master->s.sound = 0;
	}
}

#pragma endregion