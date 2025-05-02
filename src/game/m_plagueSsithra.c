//
// m_plagueSsitra.c
//
// Copyright 1998 Raven Software
//

#include <float.h> //mxd
#include "m_plaguesSithra.h"
#include "m_plaguesSithra_shared.h"
#include "m_plaguesSithra_anim.h"
#include "g_debris.h" //mxd
#include "g_DefaultMessageHandler.h"
#include "g_obj.h" //mxd
#include "mg_ai.h" //mxd
#include "mg_guide.h" //mxd
#include "m_stats.h"
#include "Random.h"
#include "Utilities.h"
#include "Vector.h"
#include "g_monster.h"
#include "g_local.h"

static void create_ssith_arrow(edict_t* Arrow); //TODO: remove.
qboolean ssithraCheckInWater(edict_t* self); //TODO: remove.
extern void FishDeadFloatThink(edict_t* self); //TODO: move to g_monster.c as M_DeadFloatThink?..

#pragma region ========================== Plague Ssithra Base Info ==========================

static const animmove_t* animations[NUM_ANIMS] =
{
	&ssithra_move_idle1,
	&ssithra_move_walk1,
	&ssithra_move_backpedal1,
	&ssithra_move_bound1,
	&ssithra_move_death_a1,
	&ssithra_move_death_b1,
	&ssithra_move_dive1,
	&ssithra_move_duckshoot1,
	&ssithra_move_duck1,
	&ssithra_move_gallop1,
	&ssithra_move_fjump,
	&ssithra_move_idlebasic1,
	&ssithra_move_idleright1,
	&ssithra_move_melee1,
	&ssithra_move_meleest,
	&ssithra_move_namor1,
	&ssithra_move_pain_a1,
	&ssithra_move_shoot1,
	&ssithra_move_startle1,
	&ssithra_move_swimforward1,
	&ssithra_move_swimwander,
	&ssithra_move_water_death1,
	&ssithra_move_water_idle1,
	&ssithra_move_water_pain_a1,
	&ssithra_move_water_pain_b1,
	&ssithra_move_water_shoot1,
	&ssithra_move_run1,
	&ssithra_move_spinright,
	&ssithra_move_spinright_go,
	&ssithra_move_spinleft,
	&ssithra_move_spinleft_go,
	&ssithra_move_faceandnamor,
	&ssithra_move_dead_a,
	&ssithra_move_lookright,
	&ssithra_move_lookleft,
	&ssithra_move_transup,
	&ssithra_move_transdown,
	&ssithra_move_headless,
	&ssithra_move_headlessloop,
	&ssithra_move_death_c,
	&ssithra_move_dead_b,
	&ssithra_move_dead_water,
	&ssithra_move_sliced,
	&ssithra_move_delay,
	&ssithra_move_duckloop,
	&ssithra_move_unduck,
	&ssithra_move_lunge
};

static int sounds[NUM_SOUNDS];

#pragma endregion

static void SsithraBlocked(edict_t* self, trace_t* trace) //mxd. Named 'ssithra_blocked' in original logic.
{
	if (trace->ent == NULL || trace->ent->movetype == PHYSICSTYPE_NONE || trace->ent->movetype == PHYSICSTYPE_PUSH)
		return;

	const float strength = VectorLength(self->velocity);

	if (strength < 50.0f)
		return;

	vec3_t hit_dir;
	VectorCopy(self->velocity, hit_dir);
	hit_dir[2] = max(0.0f, hit_dir[2]);

	VectorNormalize(hit_dir);
	VectorScale(hit_dir, strength, hit_dir);
	VectorAdd(trace->ent->velocity, hit_dir, trace->ent->knockbackvel);

	if (!(self->spawnflags & MSF_FIXED))
		ssithraJump(self, 150.0f, 200.0f, 0.0f);
}

static void SsithraStandMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'ssithra_stand' in original logic.
{
	if (self->ai_mood == AI_MOOD_DELAY)
	{
		SetAnim(self, ANIM_DELAY);
		return;
	}

	if (ssithraCheckInWater(self))
	{
		SetAnim(self, ANIM_WATER_IDLE);
		return;
	}

	switch (self->curAnimID)
	{
		case ANIM_STAND1:
			SetAnim(self, irand(0, 10) < 8 ? ANIM_STAND1 : ANIM_IDLEBASIC); //mxd. flrand() in original logic.
			break;

		case ANIM_IDLERIGHT:
			SetAnim(self, irand(0, 10) < 6 ? ANIM_STAND1 : ANIM_IDLEBASIC); //mxd. flrand() in original logic.
			break;

		default:
			SetAnim(self, ANIM_STAND1);
			break;
	}
}

static void SsithraWalkMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'ssithra_walk' in original logic.
{
	SetAnim(self, ((self->spawnflags & MSF_FIXED) ? ANIM_DELAY : ANIM_WALK1));
}

static void SsithraRunMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'ssithra_gallop' in original logic.
{
	if (self->curAnimID == ANIM_SPINRIGHT)
	{
		SetAnim(self, ANIM_SPINRIGHT_GO);
		return;
	}

	if (self->curAnimID == ANIM_SPINLEFT)
	{
		SetAnim(self, ANIM_SPINLEFT_GO);
		return;
	}

	if (self->enemy == NULL || self->enemy->health <= 0)
	{
		SetAnim(self, ANIM_STAND1);
		return;
	}

	if (self->spawnflags & MSF_FIXED)
	{
		SetAnim(self, ANIM_DELAY);
		return;
	}

	if (self->spawnflags & MSF_SSITHRA_NAMOR) // Out of water jump.
	{
		self->spawnflags &= ~MSF_SSITHRA_NAMOR;
		SetAnim(self, ANIM_NAMOR);

		return;
	}

	if (self->spawnflags & MSF_SSITHRA_SPIN) // Spin.
	{
		self->spawnflags &= ~MSF_SSITHRA_SPIN;
		SetAnim(self, (irand(0, 1) == 1 ? ANIM_SPINRIGHT : ANIM_SPINLEFT));

		return;
	}

	SetAnim(self, ANIM_RUN1);
}

static void SsithraWatchMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'ssithra_idlebasic' in original logic.
{
	SetAnim(self, ANIM_IDLEBASIC);
}

static void SsithraDecideStand(edict_t* self) //mxd. Named 'ssithra_decide_stand' in original logic.
{
	if (ssithraCheckInWater(self))
	{
		SetAnim(self, ANIM_WATER_IDLE);
		return;
	}

	switch (self->curAnimID)
	{
		case ANIM_STAND1:
		case ANIM_IDLEBASIC:
			if (irand(0, 10) < 7) //mxd. flrand() in original logic.
				SetAnim(self, ANIM_STAND1);
			else if (irand(0, 10) < 7) //mxd. flrand() in original logic.
				SetAnim(self, ANIM_IDLERIGHT);
			else if (irand(0, 10) < 5) //mxd. flrand() in original logic.
				SetAnim(self, ANIM_LOOKRIGHT);
			else
				SetAnim(self, ANIM_LOOKLEFT);
			break;

		case ANIM_IDLERIGHT:
		case ANIM_LOOKLEFT:
		case ANIM_LOOKRIGHT:
			SetAnim(self, ((irand(0, 10) < 6) ? ANIM_STAND1 : ANIM_IDLEBASIC)); //mxd. flrand() in original logic.
			break;

		default:
			SetAnim(self, ANIM_STAND1);
			break;
	}
}

void ssithra_decide_gallop(edict_t* self) //TODO: rename to ssithra_decide_run.
{
	if (self->spawnflags & MSF_FIXED)
	{
		SetAnim(self, ANIM_DELAY);
		return;
	}

	VectorClear(self->velocity);
	self->count = false; //TODO: add ssithra_watersplash_spawned name.

	SetAnim(self, (ssithraCheckInWater(self) ? ANIM_SWIMFORWARD : ANIM_RUN1));
	SsithraCheckMood(self);
}

void ssithra_decide_swimforward(edict_t* self)
{
	//FIXME: climb out of water check!
	self->count = false;
	VectorClear(self->velocity);

	if (!ssithraCheckInWater(self))
		SetAnim(self, ANIM_RUN1); // Not actually in water!
	else if (self->curAnimID == ANIM_WATER_SHOOT)
		SetAnim(self, ANIM_TRANSDOWN);

	SsithraCheckMood(self);
}

void ssithra_decide_backpedal(edict_t* self) //TODO: replace with SsithraCheckMood()?
{
	SsithraCheckMood(self);
}

void ssithraCheckRipple(edict_t* self) //TODO: rename to ssithra_check_ripple.
{
	// No ripples while in cinematics.
	if (SV_CINEMATICFREEZE)
		return;

	vec3_t top;
	VectorCopy(self->s.origin, top);
	top[2] += self->maxs[2] * 0.75f;

	vec3_t bottom;
	VectorCopy(self->s.origin, bottom);
	bottom[2] += self->mins[2];

	trace_t trace;
	gi.trace(top, vec3_origin, vec3_origin, bottom, self, MASK_WATER, &trace);

	if (trace.fraction < 1.0f)
	{
		vec3_t forward;
		AngleVectors(self->s.angles, forward, NULL, NULL);
		Vec3ScaleAssign(200.0f, forward);

		const byte b_angle = (byte)Q_ftol((self->s.angles[YAW] + DEGREE_180) / 360.0f * 255.0f);
		gi.CreateEffect(NULL, FX_WATER_WAKE, 0, trace.endpos, "sbv", self->s.number, b_angle, forward);
	}
}

qboolean ssithraCheckInWater(edict_t* self) //TODO: rename to SsithraCheckInWater, make static.
{
	// In water?
	if ((self->flags & FL_INWATER) && !(self->flags & FL_INLAVA) && !(self->flags & FL_INSLIME) && (self->waterlevel > 2 || self->groundentity == NULL))
	{
		self->monsterinfo.aiflags |= AI_NO_MELEE;
		return true;
	}

	if (!(self->s.fmnodeinfo[MESH__LEFTARM].flags & FMNI_NO_DRAW))
		self->monsterinfo.aiflags &= ~AI_NO_MELEE;

	return false;
}

void ssithraVOfs(edict_t* self, float pitch_offset, float yaw_offset, float roll_offset) //TODO: rename to ssithra_set_view_angle_offsets.
{
	self->v_angle_ofs[PITCH] = pitch_offset;
	self->v_angle_ofs[YAW] = yaw_offset;
	self->v_angle_ofs[ROLL] = roll_offset;
}

static qboolean SsithraHaveWaterLedgeNearEnemy(edict_t* self) //mxd. Named 'ssithraWaterLedgeNearEnemy' in original logic.
{
	vec3_t target_origin;

	if ((self->spawnflags & MSF_FIXED) || !MG_TryGetTargetOrigin(self, target_origin))
		return false;

	vec3_t enemy_dir;
	VectorSubtract(target_origin, self->s.origin, enemy_dir);
	VectorNormalize(enemy_dir);

	vec3_t end_pos;
	VectorMA(self->s.origin, 128.0f, enemy_dir, end_pos);

	trace_t trace;
	gi.trace(self->s.origin, self->mins, self->maxs, end_pos, self, MASK_SOLID, &trace);

	return (trace.fraction < 1.0f); // When trace.fraction == 1, no ledge to jump up on.
}

void ssithra_check_namor(edict_t* self) //TODO: rename to ssithra_try_out_of_water_jump.
{
	//FIXME: climb out of water check!
	vec3_t target_origin;

	if ((self->spawnflags & MSF_FIXED) || !MG_TryGetTargetOrigin(self, target_origin))
		return;

	if (!(gi.pointcontents(target_origin) & CONTENTS_WATER) && MG_IsVisiblePos(self, target_origin) && SsithraHaveWaterLedgeNearEnemy(self))
		SetAnim(self, ANIM_FACEANDNAMOR);
}

static void SsithraTryJump(edict_t* self) //mxd. Named 'ssithraWhichJump' in original logic.
{
	vec3_t target_origin;

	if ((self->spawnflags & MSF_FIXED) || !MG_TryGetTargetOrigin(self, target_origin))
		return;

	if (ssithraCheckInWater(self))
	{
		if (!(gi.pointcontents(target_origin) & CONTENTS_WATER))
			SetAnim(self, ANIM_NAMOR);

		return;
	}

	SetAnim(self, ANIM_BOUND);

	vec3_t forward;
	AngleVectors(self->s.angles, forward, NULL, NULL);

	VectorScale(forward, SSITHRA_HOP_VELOCITY, self->velocity);
	self->velocity[2] = SSITHRA_HOP_VELOCITY + 32.0f;
}

static void SsithraJumpMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'ssithraMsgJump' in original logic.
{
	if (self->spawnflags & MSF_FIXED)
		SetAnim(self, ANIM_DELAY);
	else
		SsithraTryJump(self);
}

void ssithraBoundCheck(edict_t* self) //TODO: rename to ssithra_check_bound.
{
	//FIXME: do checks and traces first.
	if (self->spawnflags & MSF_FIXED)
		return;

	if (ssithraCheckInWater(self))
	{
		if (self->curAnimID != ANIM_SWIMFORWARD)
			SetAnim(self, ANIM_SWIMFORWARD);

		return;
	}

	vec3_t forward;
	vec3_t up;
	AngleVectors(self->s.angles, forward, NULL, up);

	vec3_t start_pos;
	VectorCopy(self->s.origin, start_pos);

	vec3_t end_pos;
	VectorMA(start_pos, 48.0f, forward, end_pos); // Forward.

	trace_t trace;
	gi.trace(start_pos, self->mins, self->maxs, end_pos, self, MASK_SOLID, &trace);

	VectorCopy(trace.endpos, start_pos);
	VectorMA(start_pos, -128.0f, up, end_pos); // Down.

	gi.trace(start_pos, self->mins, self->maxs, end_pos, self, MASK_SOLID | MASK_WATER, &trace);

	// If it's a step down or less, no jump.
	if (Q_fabs(trace.endpos[2] - self->s.origin[2]) <= 18.0f)
		return;

	if (trace.fraction == 1.0f || trace.allsolid || trace.startsolid)
		return; // Too far to jump down, or in solid.

	if (trace.contents & (CONTENTS_WATER | CONTENTS_SLIME))
	{
		VectorCopy(trace.endpos, start_pos);
		VectorMA(start_pos, -64.0f, up, end_pos); // Down from water surface.

		const vec3_t mins = { self->mins[0], self->mins[1], 0.0f };
		const vec3_t maxs = { self->maxs[0], self->maxs[1], 1.0f };

		gi.trace(start_pos, mins, maxs, end_pos, self, MASK_SOLID, &trace);

		if (trace.fraction < 1.0f || trace.allsolid || trace.startsolid)
			SsithraTryJump(self);
		else
			SetAnim(self, ANIM_DIVE);
	}
	else
	{
		SetAnim(self, ANIM_GALLOP);
	}
}

void ssithraDiveCheck(edict_t* self) //TODO: rename to ssithra_check_dive.
{
	//FIXME: do checks and traces first.
	if (self->spawnflags & MSF_FIXED)
		return;

	vec3_t target_origin;
	vec3_t targ_mins;

	if (self->monsterinfo.searchType == SEARCH_BUOY)
	{
		if (self->buoy_index < 0 || self->buoy_index > level.active_buoys)
			return;

		VectorCopy(level.buoy_list[self->buoy_index].origin, target_origin);
		VectorClear(targ_mins);
	}
	else
	{
		if (self->goalentity == NULL)
			return;

		VectorCopy(self->goalentity->s.origin, target_origin);
		VectorCopy(self->goalentity->mins, targ_mins);
	}

	if (ssithraCheckInWater(self))
	{
		SetAnim(self, ANIM_SWIMFORWARD);
		return;
	}

	if (!MG_IsInforntPos(self, target_origin))
		return;

	// Make sure the enemy isn't right here and accessible before diving in.
	if (vhlen(target_origin, self->s.origin) < 96.0f && // Close enough?
		Q_fabs((target_origin[2] + targ_mins[2]) - (self->s.origin[2] + self->mins[2])) < 18.0f && // Relatively same stepheight.
		!(gi.pointcontents(target_origin) & CONTENTS_WATER) && !(self->monsterinfo.aiflags & AI_FLEE))
	{
		return;
	}

	vec3_t forward;
	vec3_t up;
	AngleVectors(self->s.angles, forward, NULL, up);

	vec3_t start_pos;
	VectorCopy(self->s.origin, start_pos);

	vec3_t end_pos;
	VectorMA(start_pos, 48.0f, forward, end_pos); // Forward.

	trace_t trace;
	gi.trace(start_pos, self->mins, self->maxs, end_pos, self, MASK_SOLID, &trace);

	VectorCopy(trace.endpos, start_pos);
	VectorMA(start_pos, -128.0f, up, end_pos); // Down.

	gi.trace(start_pos, self->mins, self->maxs, end_pos, self, MASK_SOLID | MASK_WATER, &trace);

	if (trace.fraction == 1.0f || trace.allsolid || trace.startsolid)
		return; // Too far to jump down, or in solid.

	if (trace.contents & (CONTENTS_WATER | CONTENTS_SLIME))
	{
		VectorCopy(trace.endpos, start_pos);
		VectorMA(start_pos, -64.0f, up, end_pos); // Down from water surface.

		const vec3_t mins = { self->mins[0], self->mins[1], 0.0f };
		const vec3_t maxs = { self->maxs[0], self->maxs[1], 1.0f };

		gi.trace(start_pos, mins, maxs, end_pos, self, MASK_SOLID, &trace);

		if (trace.fraction < 1.0f || trace.allsolid || trace.startsolid)
			SsithraTryJump(self);
		else
			SetAnim(self, ANIM_DIVE);
	}
}

void ssithraApplyJump(edict_t* self) //TODO: rename to ssithra_apply_jump.
{
	if (self->spawnflags & MSF_FIXED)
		return;

	self->jump_time = level.time + 1.0f;
	VectorCopy(self->movedir, self->velocity);
	VectorNormalize(self->movedir);
}

void ssithraJump(edict_t* self, float up_speed, float forward_speed, float right_speed) //TODO: rename to ssithra_jump.
{
	//FIXME: do checks and traces first.
	if (self->spawnflags & MSF_FIXED)
		return;

	if ((self->s.fmnodeinfo[MESH__LEFTLEG].flags & FMNI_NO_DRAW) || (self->s.fmnodeinfo[MESH__RIGHTLEG].flags & FMNI_NO_DRAW))
	{
		up_speed *= 2.0f;
		forward_speed /= 2.0f;
	}

	vec3_t forward;
	vec3_t right;
	vec3_t up;
	AngleVectors(self->s.angles, forward, right, up);

	VectorMA(self->velocity, up_speed, up, self->velocity);
	VectorMA(self->velocity, forward_speed, forward, self->velocity);
	VectorMA(self->velocity, right_speed, right, self->velocity);
}

void ssithraNamorJump(edict_t* self) //TODO: rename to ssithra_out_of_water_jump.
{
	vec3_t target_origin;

	if ((self->spawnflags & MSF_FIXED) || !MG_TryGetTargetOrigin(self, target_origin))
		return;

	//FIXME: jumps too high sometimes?
	self->count = false;

	vec3_t top;
	VectorCopy(self->s.origin, top);
	top[2] += 512.0f;

	trace_t trace;
	gi.trace(self->s.origin, vec3_origin, vec3_origin, top, self, MASK_SOLID, &trace);

	VectorCopy(trace.endpos, top);
	gi.trace(top, vec3_origin, vec3_origin, self->s.origin, self, MASK_SOLID | MASK_WATER, &trace);

	// How far above my feet is waterlevel?
	vec3_t diff;
	VectorSubtract(trace.endpos, self->s.origin, diff);
	const float watersurf_zdist = VectorLength(diff) - self->mins[2]; // Adjust for my feet.

	// How high above water level is player?
	const float enemy_zdiff = target_origin[2] - trace.endpos[2];

	//FIXME: aim a little to side if enemy close so don't land on top of him? Or hit him if land on top?
	ssithraJump(self, (watersurf_zdist + enemy_zdiff) * 2.0f + 200.0f, 100.0f, 0.0f);
}

void ssithraCheckJump(edict_t* self) //TODO: rename to SsithraCheckJump.
{
	if (self->spawnflags & MSF_FIXED)
		return;

	vec3_t target_origin;
	vec3_t target_mins;

	if (self->monsterinfo.searchType == SEARCH_BUOY)
	{
		if (self->buoy_index < 0 || self->buoy_index > level.active_buoys)
			return;

		VectorCopy(level.buoy_list[self->buoy_index].origin, target_origin);
		VectorClear(target_mins);
	}
	else
	{
		if (self->goalentity == NULL)
			return;

		VectorCopy(self->goalentity->s.origin, target_origin);
		VectorCopy(self->goalentity->mins, target_mins);
	}

	if (!MG_IsInforntPos(self, target_origin))
		return;

	// Jumping down?
	if (target_origin[2] < self->s.origin[2] - 28.0f)
	{
		// Setup the trace
		if (ssithraCheckInWater(self))
			return;

		vec3_t s_maxs;
		VectorCopy(self->maxs, s_maxs);
		s_maxs[2] += 32.0f;

		vec3_t source;
		VectorCopy(self->s.origin, source);

		vec3_t forward;
		AngleVectors(self->s.angles, forward, NULL, NULL);

		VectorMA(source, 128.0f, forward, source);

		trace_t trace;
		gi.trace(self->s.origin, self->mins, s_maxs, source, self, MASK_MONSTERSOLID, &trace);

		if (trace.fraction == 1.0f)
		{
			// Clear ahead and above.
			vec3_t source2;
			VectorCopy(source, source2);
			source2[2] -= 1024.0f;

			// Trace down.
			gi.trace(source, self->mins, self->maxs, source2, self, MASK_ALL, &trace);

			if (trace.fraction == 1.0f || trace.startsolid || trace.allsolid) // Check down - too far or allsolid.
				return;

			if (trace.contents != CONTENTS_SOLID)
			{
				// Jumping into water?
				if ((trace.contents & (CONTENTS_WATER | CONTENTS_SLIME)) || trace.ent == self->enemy)
				{
					vec3_t dir;
					VectorSubtract(trace.endpos, self->s.origin, dir);
					VectorNormalize(dir);
					self->ideal_yaw = VectorYaw(dir);

					if (self->monsterinfo.jump_time < level.time)
					{
						// Check depth.
						VectorCopy(trace.endpos, source);
						VectorCopy(source, source2);
						source2[2] -= 64.0f;

						const vec3_t mins = { self->mins[0], self->mins[1], 0.0f };
						const vec3_t maxs = { self->maxs[0], self->maxs[1], 1.0f };

						gi.trace(source, mins, maxs, source2, self, MASK_SOLID, &trace);

						if (trace.fraction < 1.0f || trace.startsolid || trace.allsolid)
							SsithraTryJump(self);
						else
							SetAnim(self, ANIM_DIVE);

						self->monsterinfo.jump_time = level.time + 1.0f;
					}
				}
			}
			else
			{
				vec3_t dir;
				VectorSubtract(trace.endpos, self->s.origin, dir);
				VectorNormalize(dir);
				self->ideal_yaw = VectorYaw(dir);

				if (self->monsterinfo.jump_time < level.time)
				{
					SsithraTryJump(self);
					self->monsterinfo.jump_time = level.time + 1.0f;
				}
			}
		} // Else not clear infront.

		return;
	}

	// Check if we should jump up.
	qboolean jump_up_check = (vhlen(self->s.origin, target_origin) < 200.0f);

	if (!jump_up_check)
	{
		vec3_t source;
		VectorCopy(self->s.origin, source);
		source[2] -= 10.0f;

		if (gi.pointcontents(source) & CONTENTS_WATER)
		{
			//FIXME: swimming can bring origin out of water!
			vec3_t forward;
			AngleVectors(self->s.angles, forward, NULL, NULL);
			VectorMA(self->s.origin, 72.0f, forward, source);

			trace_t trace;
			gi.trace(self->s.origin, self->mins, self->maxs, source, self, MASK_SOLID, &trace);

			if (trace.fraction < 1.0f)
				jump_up_check = true;

			// Shore is within 72 units of me.
		}
		else // Enemy far away, in front, and water in front of me.
		{
			// Check if water in front.
			vec3_t forward;
			AngleVectors(self->s.angles, forward, NULL, NULL);
			VectorMA(self->s.origin, 48.0f, forward, source);

			trace_t trace;
			gi.trace(self->s.origin, self->mins, self->maxs, source, self, MASK_SOLID, &trace);

			VectorCopy(trace.endpos, source);
			source[2] -= 128.0f;

			gi.trace(trace.endpos, self->mins, self->maxs, source, self, MASK_SOLID | MASK_WATER, &trace);

			if (trace.fraction < 1.0f && (trace.contents & CONTENTS_WATER))
			{
				VectorCopy(trace.endpos, source);

				vec3_t source2;
				VectorCopy(source, source2);
				source[2] -= 64.0f;

				const vec3_t mins = { self->mins[0], self->mins[1], 0.0f };
				const vec3_t maxs = { self->maxs[0], self->maxs[1], 1.0f };

				gi.trace(source, mins, maxs, source2, self, MASK_SOLID, &trace);

				if (trace.fraction < 1.0f || trace.allsolid || trace.startsolid)
					SsithraTryJump(self);
				else
					SetAnim(self, ANIM_DIVE);

				return;
			}
		}
	}

	if (!jump_up_check)
		return;

	// Jumping up?
	if (target_origin[2] > self->s.origin[2] + 28.0f || !(self->monsterinfo.aiflags & AI_FLEE))
	{
		vec3_t source;
		VectorCopy(self->s.origin, source);

		//FIXME: what about if running away?
		const float height_diff = (target_origin[2] + target_mins[2]) - (self->s.origin[2] + self->mins[2]) + 32.0f;
		source[2] += height_diff;

		trace_t trace;
		gi.trace(self->s.origin, self->mins, self->maxs, source, self, MASK_ALL, &trace);

		if (trace.fraction == 1.0f)
		{
			// Clear above.
			vec3_t forward;
			AngleVectors(self->s.angles, forward, NULL, NULL);

			vec3_t source2;
			VectorMA(source, 64.0f, forward, source2);
			source2[2] -= 24.0f;

			// Trace forward and down a little.
			gi.trace(source, self->mins, self->maxs, source2, self, MASK_ALL, &trace);

			if (trace.fraction < 0.1f || trace.allsolid || trace.startsolid || trace.ent == (struct edict_s*)-1) // Can't jump up, no ledge.
				return;

			vec3_t dir;
			VectorSubtract(trace.endpos, self->s.origin, dir);
			VectorNormalize(dir);
			self->ideal_yaw = VectorYaw(dir);

			if (self->monsterinfo.jump_time < level.time)
			{
				SsithraTryJump(self);
				self->monsterinfo.jump_time = level.time + 1.0f;
			}
		}

		return;
	}

	// Check to jump over something.
	vec3_t save_org;
	VectorCopy(self->s.origin, save_org);
	const qboolean can_move = M_walkmove(self, self->s.angles[YAW], 64.0f);
	VectorCopy(save_org, self->s.origin);

	if (!can_move)
	{
		vec3_t forward;
		AngleVectors(self->s.angles, forward, NULL, NULL);

		vec3_t end_pos;
		VectorMA(self->s.origin, 128.0f, forward, end_pos);

		vec3_t mins;
		VectorCopy(self->mins, mins);
		mins[2] += 24.0f; // Can clear it.

		trace_t trace;
		gi.trace(self->s.origin, mins, self->maxs, end_pos, self, MASK_SOLID, &trace);

		if (trace.allsolid || trace.startsolid || (trace.fraction < 1.0f && trace.ent != self->enemy))
			return;

		// Go for it!
		ssithraJump(self, 128.0f, trace.fraction * 200.0f, 0.0f);
		SetAnim(self, ANIM_BOUND);
	}
}

// Simple addition of velocity, if on ground or not.
void ssithraForward(edict_t* self, float forward_dist) //TODO: rename to ssithra_set_forward_velocity.
{
	ssithraCheckInWater(self);

	if (self->groundentity != NULL) // On ground.
	{
		VectorClear(self->velocity);
	}
	else // In air (or water?).
	{
		vec3_t forward;
		AngleVectors(self->s.angles, forward, NULL, NULL);

		Vec3ScaleAssign(forward_dist, forward);
		forward[2] = self->velocity[2];

		VectorCopy(forward, self->velocity);
	}
}

void ssithraCheckLeaveWaterSplash(edict_t* self) //TODO: rename to ssithra_spawn_water_exit_splash.
{
	if (self->count || ssithraCheckInWater(self))
		return;

	vec3_t dir;
	VectorCopy(self->velocity, dir);
	VectorNormalize(dir);

	vec3_t end_pos;
	VectorMA(self->s.origin, -256.0f, dir, end_pos);

	trace_t trace;
	gi.trace(self->s.origin, vec3_origin, vec3_origin, end_pos, self, MASK_WATER, &trace);

	if (trace.fraction < 1.0f)
	{
		gi.sound(self, CHAN_BODY, sounds[SND_NAMOR], 1.0f, ATTN_NORM, 0.0f);

		// FIXME: Size proportional to exit velocity.
		const vec3_t fx_dir = { 0.0f, 0.0f, 300.0f }; //TODO: normalized in ssithraCheckHitWaterSplash(). Which is correct?
		gi.CreateEffect(NULL, FX_WATER_ENTRYSPLASH, 0, trace.endpos, "bd", 128 | 96, fx_dir);

		self->count = true;
	}
}

void ssithraCheckHitWaterSplash(edict_t* self) //TODO: rename to ssithra_spawn_water_entry_splash.
{
	if (self->count)
		return;

	if (Q_fabs(self->velocity[0]) + Q_fabs(self->velocity[1]) < 200.0f)
	{
		vec3_t end_pos;
		VectorCopy(self->s.origin, end_pos);
		end_pos[2] -= 128.0f;

		trace_t trace;
		gi.trace(self->s.origin, self->mins, self->maxs, end_pos, self, MASK_ALL, &trace);

		if (trace.fraction < 1.0f && !trace.allsolid && !trace.startsolid && !(trace.contents & CONTENTS_WATER) && !(trace.contents & CONTENTS_SLIME))
		{
			// Not going to hit water!
			SetAnim(self, ANIM_BOUND);
			return;
		}
	}

	if (self->flags & FL_INWATER)
	{
		vec3_t dir;
		VectorNormalize2(self->velocity, dir);

		vec3_t end_pos;
		VectorMA(self->s.origin, -256.0f, dir, end_pos);

		trace_t trace;
		gi.trace(self->s.origin, vec3_origin, vec3_origin, end_pos, self, MASK_WATER, &trace);
		gi.trace(trace.endpos, vec3_origin, vec3_origin, self->s.origin, self, MASK_WATER, &trace);

		if (trace.fraction < 1.0f)
		{
			gi.sound(self, CHAN_BODY, sounds[SND_INWATER], 1.0f, ATTN_NORM, 0.0f);
			gi.sound(self, CHAN_BODY, gi.soundindex("player/Water Enter.wav"), 1.0f, ATTN_NORM, 0.0f); //TODO: why 2 similar sounds? Use SND_INWATER only?

			vec3_t fx_dir = { 0.0f, 0.0f, self->velocity[2] };
			VectorNormalize(fx_dir);

			// FIXME: Size proportional to entry velocity.
			gi.CreateEffect(NULL, FX_WATER_ENTRYSPLASH, CEF_FLAG7, trace.endpos, "bd", 128 | 96, fx_dir);

			self->count = true;
		}
	}
}

void ssithraCheckFacedNamor(edict_t* self) //TODO: rename to ssithra_check_faced_out_of_water_jump.
{
	if (!(self->spawnflags & MSF_FIXED) && Q_fabs(self->ideal_yaw - self->s.angles[YAW]) < self->yaw_speed)
		SetAnim(self, ANIM_NAMOR);
}

static void SsithraSlideFallThink(edict_t* self) //mxd. Named 'ssithraSlideFall' in original logic.
{
	if (self->mins[2] < 0.0f)
	{
		if (self->mins[2] <= -6.0f)
			self->mins[2] += 6.0f;
		else
			self->mins[2] = 0.0f;

		self->nextthink = level.time + FRAMETIME;
	}
	else
	{
		self->friction = 1.0f;
		SetAnim(self->owner, ANIM_SLICED);

		self->owner->msgHandler = DyingMsgHandler;
		self->owner->nextthink = level.time;

		self->think = NULL;
		self->nextthink = -1.0f;
	}
}

static void SsithraSlideOffThink(edict_t* self) //mxd. Named 'ssithraSlideOff' in original logic.
{
	vec3_t right;
	AngleVectors(self->s.angles, NULL, right, NULL);
	VectorScale(right, 100.0f, self->velocity);

	self->think = SsithraSlideFallThink;
	self->nextthink = level.time + FRAMETIME;
}

static void SsithraSplit(edict_t* self, const int body_part) //mxd. Named 'ssithraSplit' in original logic.
{
	// Blood stripe.
	vec3_t right;
	vec3_t up;
	AngleVectors(self->s.angles, NULL, right, up);

	vec3_t p1 = { 0 };
	VectorMA(p1, 6.0f, up, p1);
	VectorMA(p1, 10.0f, right, p1);

	vec3_t p2 = { 0 };
	VectorMA(p2, -6.0f, up, p2);
	VectorMA(p2, -10.0f, right, p2);

	vec3_t dir;
	VectorSubtract(p2, p1, dir);
	VectorNormalize(dir);
	Vec3ScaleAssign(40.0f, dir);

	// Why doesn't this work?
	gi.CreateEffect(&self->s, FX_BLOOD, 0, p1, "ub", dir, 20);

	Vec3AddAssign(self->s.origin, p2);
	SprayDebris(self, p2, 6, 200);

	// Spawn top part.
	edict_t* top_half = G_Spawn();

	top_half->svflags |= (SVF_MONSTER | SVF_DEADMONSTER);
	top_half->s.renderfx |= RF_FRAMELERP;
	top_half->s.effects |= EF_CAMERA_NO_CLIP;
	top_half->takedamage = DAMAGE_AIM;
	top_half->health = 25;
	top_half->max_health = top_half->health;
	top_half->clipmask = MASK_MONSTERSOLID;

	top_half->deadflag = DEAD_DEAD;
	top_half->deadState = DEAD_DEAD;
	top_half->monsterinfo.thinkinc = MONSTER_THINK_INC;
	top_half->monsterinfo.nextframeindex = -1;
	top_half->friction = 0.1f;

	VectorCopy(self->s.origin, top_half->s.origin);
	VectorCopy(top_half->s.origin, top_half->s.old_origin);
	top_half->s.origin[2] += 10.0f;

	VectorCopy(self->s.angles, top_half->s.angles);

	top_half->think = SsithraSlideOffThink;
	top_half->nextthink = level.time + FRAMETIME * 10.0f;

	top_half->materialtype = MAT_FLESH;
	top_half->mass = 300;
	self->mass = top_half->mass;

	top_half->movetype = PHYSICSTYPE_STEP;
	top_half->solid = SOLID_BBOX;
	top_half->owner = self;

	VectorSet(top_half->mins, -16.0f, -16.0f, self->mins[2]);
	VectorSet(top_half->maxs, 16.0f, 16.0f, 16.0f);

	VectorSet(self->maxs, 16.0f, 16.0f, 0.0f);

	//FIXME: sometimes top half appears too low and forward?
	VectorClear(self->knockbackvel);
	VectorClear(self->velocity);

	top_half->s.modelindex = self->s.modelindex;
	top_half->s.frame = (short)((self->s.frame == 0) ? FRAME_startle32 : self->s.frame);
	top_half->s.skinnum = self->s.skinnum;
	top_half->s.scale = self->s.scale; //BUGFIX: mxd. 'top_half->s.scale = top_half->s.scale' in original logic.
	top_half->monsterinfo.otherenemyname = "obj_barrel"; //TODO: why?..

	int node_num = 1;
	for (int which_node = 1; which_node <= 16384; which_node *= 2, node_num++) // Bitwise.
	{
		if (body_part & which_node)
		{
			// Turn on this node on top and keep them.
			top_half->s.fmnodeinfo[node_num] = self->s.fmnodeinfo[node_num]; // Copy skins and flags and colors.
			top_half->s.fmnodeinfo[node_num].flags &= ~FMNI_NO_DRAW;
			self->s.fmnodeinfo[node_num].flags |= FMNI_NO_DRAW;
		}
		else
		{
			// Turn off this node on top.
			top_half->s.fmnodeinfo[node_num].flags |= FMNI_NO_DRAW;
		}
	}

	top_half->s.fmnodeinfo[MESH__CAPBOTTOMUPPERTORSO].flags &= ~FMNI_NO_DRAW;
	self->s.fmnodeinfo[MESH__CAPLOWERTORSO].flags &= ~FMNI_NO_DRAW;
	self->s.fmnodeinfo[MESH__RIGHT2SPIKE].flags |= FMNI_NO_DRAW;

	self->nextthink = FLT_MAX; //mxd. 9999999999999999 in original logic.
}

static qboolean SsithraCanThrowNode(edict_t* self, const int node_id, int* throw_nodes) //mxd. Named 'canthrownode' in original logic.
{
	static const int bit_for_mesh_node[16] = //mxd. Made local static.
	{
		BIT_POLY,
		BIT_LOWERTORSO,
		BIT_CAPLOWERTORSO,
		BIT_LEFTLEG,
		BIT_RIGHTLEG,
		BIT_UPPERTORSO,
		BIT_CAPTOPUPPERTORSO,
		BIT_CAPBOTTOMUPPERTORSO,
		BIT_LEFTARM,
		BIT_RIGHTARM,
		BIT_HEAD,
		BIT_CENTERSPIKE,
		BIT_LEFT1SPIKE,
		BIT_RIGHT1SPIKE,
		BIT_RIGHT2SPIKE,
		BIT_CAPHEAD
	};

	// See if it's on, if so, add it to throw_nodes. Turn it off on thrower.
	if (!(self->s.fmnodeinfo[node_id].flags & FMNI_NO_DRAW))
	{
		*throw_nodes |= bit_for_mesh_node[node_id];
		self->s.fmnodeinfo[node_id].flags |= FMNI_NO_DRAW;

		return true;
	}

	return false;
}

int ssithra_convert_hitloc_dead(edict_t *self, int hl)
{
	qboolean	fellback = false;

	if(self->curAnimID == ANIM_DEATH_A)
		fellback = true;

	switch(hl)
	{
		case hl_Head:
			if(fellback)
				return hl_TorsoFront;
			else
				return hl_TorsoBack;
			break;
		
		case hl_TorsoFront://split in half?
			if(fellback)
			{
				if(!irand(0,1))
					return hl_LegUpperRight;
				else
					return hl_LegUpperLeft;
			}
			else
				return hl_Head;
			break;
		
		case hl_TorsoBack://split in half?
			if(fellback)
				return hl_Head;
			else
			{
				if(!irand(0,1))
					return hl_LegUpperRight;
				else
					return hl_LegUpperLeft;
			}
			break;
		
		case hl_ArmUpperLeft:
				return hl_ArmLowerLeft;
			break;
		
		case hl_ArmLowerLeft://left arm
			return hl_ArmUpperLeft;
			break;
		
		case hl_ArmUpperRight:
			return hl_ArmLowerRight;
			break;
		
		case hl_ArmLowerRight://right arm
			return hl_ArmUpperRight;
			break;

		case hl_LegUpperLeft:
			return hl_LegLowerLeft;
			break;
		
		case hl_LegLowerLeft://left leg
			return hl_LegUpperLeft;
			break;
		
		case hl_LegUpperRight:
			return hl_LegLowerRight;
			break;
		
		case hl_LegLowerRight://right leg
			return hl_LegUpperRight;
			break;

		default:
			return irand(hl_Head, hl_LegLowerRight);
			break;
	}

}

void ssithra_dismember(edict_t *self, int damage, int HitLocation)
{//fixme - make part fly dir the vector from hit loc to sever loc
//remember- turn on caps!
	int				throw_nodes = 0;
	vec3_t			gore_spot, right;
	qboolean dismember_ok = false;

	if(HitLocation & hl_MeleeHit)
	{
		dismember_ok = true;
		HitLocation &= ~hl_MeleeHit;
	}

	if(HitLocation<1)
		return;

	if(HitLocation>hl_Max)
		return;

	if(self->health>0)
	{
		switch (self->curAnimID)
		{//Hit front chest during shoot or melee, may have hit arms
			case ANIM_DUCKSHOOT:
			case ANIM_SHOOT:
			case ANIM_WATER_SHOOT:
			case ANIM_HEADLESS:
			case ANIM_HEADLESSLOOP:
				if(HitLocation == hl_TorsoFront&&irand(0,10)<4)
					HitLocation = hl_ArmLowerRight;
				break;

			case ANIM_MELEE:
			case ANIM_MELEE_STAND:
				if(HitLocation == hl_TorsoFront&&irand(0,10)<4)
					HitLocation = hl_ArmLowerLeft;
				break;

			default:
				break;
		}

		if((HitLocation == hl_ArmUpperLeft&& self->s.fmnodeinfo[MESH__LEFTARM].flags & FMNI_NO_DRAW) ||
			(HitLocation == hl_ArmUpperRight&& self->s.fmnodeinfo[MESH__RIGHTARM].flags & FMNI_NO_DRAW)||
			((HitLocation == hl_TorsoFront|| HitLocation == hl_TorsoBack) &&
			self->s.fmnodeinfo[MESH__RIGHTARM].flags & FMNI_NO_DRAW &&
			self->s.fmnodeinfo[MESH__LEFTARM].flags & FMNI_NO_DRAW)&&
			irand(0,10)<4)
			HitLocation = hl_Head;//Decap
	}
	else
		HitLocation = ssithra_convert_hitloc_dead(self, HitLocation);

	VectorClear(gore_spot);
	switch(HitLocation)
	{
		case hl_Head:
			if(self->s.fmnodeinfo[MESH__HEAD].flags & FMNI_NO_DRAW)
				break;
			// Is the pain skin engaged?
			if(self->s.fmnodeinfo[MESH__HEAD].flags & FMNI_USE_SKIN)
				damage*=1.5;//greater chance to cut off if previously damaged
			if(flrand(0,self->health)<damage*0.3&&dismember_ok)
			{
				SsithraCanThrowNode(self, MESH__HEAD,&throw_nodes);
				SsithraCanThrowNode(self, MESH__CENTERSPIKE,&throw_nodes);
				SsithraCanThrowNode(self, MESH__LEFT1SPIKE,&throw_nodes);
				SsithraCanThrowNode(self, MESH__RIGHT1SPIKE,&throw_nodes);
				SsithraCanThrowNode(self, MESH__RIGHT2SPIKE,&throw_nodes);

				self->s.fmnodeinfo[MESH__CAPTOPUPPERTORSO].flags &= ~FMNI_NO_DRAW;

				gore_spot[2]+=18;
				ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);

				VectorAdd(self->s.origin, gore_spot, gore_spot);
				SprayDebris(self,gore_spot,8,damage);

				if(self->health > 0 && irand(0,10)<3&&!(self->s.fmnodeinfo[MESH__RIGHTARM].flags & FMNI_NO_DRAW))
				{//shooting blind, headless, FIX: make it so can still chop off arms or legs here
					SetAnim(self,ANIM_HEADLESS);
					self->msgHandler=DyingMsgHandler;
				}
				else
				{
					self->health = 1;
					T_Damage (self, self, self, vec3_origin, gore_spot, vec3_origin, 10, 20,0,MOD_DIED);
				}
				return;
			}
			else
			{
				// Set the pain skin
				self->s.fmnodeinfo[MESH__HEAD].flags |= FMNI_USE_SKIN;
				self->s.fmnodeinfo[MESH__HEAD].skin = self->s.skinnum+1;

				if(flrand(0,self->health/4)<damage)
				{//no red spray with these, particles?
					gore_spot[2]+=18;
					if(irand(0,10)<3)
					{
						if(SsithraCanThrowNode(self, MESH__CENTERSPIKE, &throw_nodes))
							ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);
					}
					if(irand(0,10)<3)
					{
						if(SsithraCanThrowNode(self, MESH__RIGHT1SPIKE, &throw_nodes))
							ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);
					}
					if(irand(0,10)<3)
					{
						if(SsithraCanThrowNode(self, MESH__RIGHT2SPIKE, &throw_nodes))
							ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);
					}
					if(irand(0,10)<3)
					{
						if(SsithraCanThrowNode(self, MESH__LEFT1SPIKE, &throw_nodes))
							ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);
					}
				}
			}
			break;
		case hl_TorsoFront://split in half?
		case hl_TorsoBack://split in half?
			if(self->s.fmnodeinfo[MESH__UPPERTORSO].flags & FMNI_NO_DRAW)
				break;
			if(self->s.fmnodeinfo[MESH__UPPERTORSO].flags & FMNI_USE_SKIN)
				damage*=1.5;//greater chance to cut off if previously damaged
			if(flrand(0,self->health)<damage*0.3&&dismember_ok)
			{
				gore_spot[2]+=12;
				//seal up the caps left by this split
				self->s.fmnodeinfo[MESH__CAPBOTTOMUPPERTORSO].flags &= ~FMNI_NO_DRAW;
				self->s.fmnodeinfo[MESH__CAPLOWERTORSO].flags &= ~FMNI_NO_DRAW;

				SsithraCanThrowNode(self, MESH__UPPERTORSO,&throw_nodes);
				SsithraCanThrowNode(self, MESH__CAPBOTTOMUPPERTORSO,&throw_nodes);
				SsithraCanThrowNode(self, MESH__CAPTOPUPPERTORSO,&throw_nodes);
				SsithraCanThrowNode(self, MESH__LEFTARM,&throw_nodes);
				SsithraCanThrowNode(self, MESH__RIGHTARM,&throw_nodes);
				SsithraCanThrowNode(self, MESH__HEAD,&throw_nodes);
				SsithraCanThrowNode(self, MESH__CENTERSPIKE,&throw_nodes);
				SsithraCanThrowNode(self, MESH__LEFT1SPIKE,&throw_nodes);
				SsithraCanThrowNode(self, MESH__RIGHT1SPIKE,&throw_nodes);
				SsithraCanThrowNode(self, MESH__RIGHT2SPIKE,&throw_nodes);

				if(self->health > 0 && irand(0,10)<3)//Slide off
					SsithraSplit(self, throw_nodes);
				else
				{
					ThrowBodyPart(self, &gore_spot, throw_nodes, damage, FRAME_partrest1);

					VectorAdd(self->s.origin, gore_spot, gore_spot);
					SprayDebris(self,gore_spot,12,damage);
					SetAnim(self,ANIM_SLICED);
				}
				self->msgHandler=DyingMsgHandler;
			}
			else
			{
				// Set the pain skin
				self->s.fmnodeinfo[MESH__UPPERTORSO].flags |= FMNI_USE_SKIN;
				self->s.fmnodeinfo[MESH__UPPERTORSO].skin = self->s.skinnum+1;
			}
			break;
		case hl_ArmUpperLeft:
		case hl_ArmLowerLeft://left arm
			if(self->s.fmnodeinfo[MESH__LEFTARM].flags & FMNI_NO_DRAW)
				break;
			if(self->s.fmnodeinfo[MESH__LEFTARM].flags & FMNI_USE_SKIN)
				damage*=1.5;//greater chance to cut off if previously damaged
			if(flrand(0,self->health)<damage*0.75&&dismember_ok)
			{
				if(SsithraCanThrowNode(self, MESH__LEFTARM, &throw_nodes))
				{
					AngleVectors(self->s.angles,NULL,right,NULL);
					gore_spot[2]+=self->maxs[2]*0.3;
					VectorMA(gore_spot,-10,right,gore_spot);
					ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);
				}
			}
			else
			{
				// Set the pain skin
				self->s.fmnodeinfo[MESH__LEFTARM].flags |= FMNI_USE_SKIN;
				self->s.fmnodeinfo[MESH__LEFTARM].skin = self->s.skinnum+1;
			}
			break;
		case hl_ArmUpperRight:
		case hl_ArmLowerRight://right arm
			if(self->s.fmnodeinfo[MESH__RIGHTARM].flags & FMNI_NO_DRAW)
				break;
			if(self->s.fmnodeinfo[MESH__RIGHTARM].flags & FMNI_USE_SKIN)
				damage*=1.5;//greater chance to cut off if previously damaged
			if(flrand(0,self->health)<damage*0.75&&dismember_ok)
			{
				if(SsithraCanThrowNode(self, MESH__RIGHTARM, &throw_nodes))
				{
					AngleVectors(self->s.angles,NULL,right,NULL);
					gore_spot[2]+=self->maxs[2]*0.3;
					VectorMA(gore_spot,10,right,gore_spot);
					ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);
				}
			}
			else
			{
				// Set the pain skin
				self->s.fmnodeinfo[MESH__RIGHTARM].flags |= FMNI_USE_SKIN;
				self->s.fmnodeinfo[MESH__RIGHTARM].skin = self->s.skinnum+1;
			}
			break;
		
		case hl_LegUpperLeft:
		case hl_LegLowerLeft://left leg
			if(self->health>0)
			{//still alive
				if(self->s.fmnodeinfo[MESH__LEFTLEG].flags & FMNI_USE_SKIN)
					break;
				self->s.fmnodeinfo[MESH__LEFTLEG].flags |= FMNI_USE_SKIN;			
				self->s.fmnodeinfo[MESH__LEFTLEG].skin = self->s.skinnum+1;
				break;
			}
			else
			{
				if(self->s.fmnodeinfo[MESH__LEFTLEG].flags & FMNI_NO_DRAW)
					break;
				if(SsithraCanThrowNode(self, MESH__LEFTLEG, &throw_nodes))
				{
					AngleVectors(self->s.angles,NULL,right,NULL);
					gore_spot[2]+=self->maxs[2]*0.3;
					VectorMA(gore_spot,-10,right,gore_spot);
					ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);
				}
				break;
			}
		case hl_LegUpperRight:
		case hl_LegLowerRight://right leg
			if(self->health>0)
			{//still alive
				if(self->s.fmnodeinfo[MESH__RIGHTLEG].flags & FMNI_USE_SKIN)
					break;
				self->s.fmnodeinfo[MESH__RIGHTLEG].flags |= FMNI_USE_SKIN;
				self->s.fmnodeinfo[MESH__RIGHTLEG].skin = self->s.skinnum+1;
				break;
			}
			else
			{
				if(self->s.fmnodeinfo[MESH__RIGHTLEG].flags & FMNI_NO_DRAW)
					break;
				if(SsithraCanThrowNode(self, MESH__RIGHTLEG, &throw_nodes))
				{
					AngleVectors(self->s.angles,NULL,right,NULL);
					gore_spot[2]+=self->maxs[2]*0.3;
					VectorMA(gore_spot,-10,right,gore_spot);
					ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);
				}
				break;
			}
		default:
			break;
	}

	if(throw_nodes)
		self->pain_debounce_time = 0;

	if(self->s.fmnodeinfo[MESH__LEFTARM].flags&FMNI_NO_DRAW&&
		self->s.fmnodeinfo[MESH__RIGHTARM].flags&FMNI_NO_DRAW)			
	{
		self->monsterinfo.aiflags |= AI_COWARD;
		self->ai_mood_flags &= ~AI_MOOD_FLAG_BACKSTAB;
		self->spawnflags &= ~MSF_FIXED;
	}
	else
	{
		if(self->s.fmnodeinfo[MESH__LEFTARM].flags&FMNI_NO_DRAW)
		{
			self->monsterinfo.aiflags |= AI_NO_MELEE;
			self->ai_mood_flags &= ~AI_MOOD_FLAG_BACKSTAB;
		}
		if(self->s.fmnodeinfo[MESH__RIGHTARM].flags&FMNI_NO_DRAW)
		{
			self->monsterinfo.aiflags |= AI_NO_MISSILE;
			self->ai_mood_flags &= ~AI_MOOD_FLAG_BACKSTAB;
			self->spawnflags &= ~MSF_FIXED;
		}
	}
}

void ssithra_dead_pain (edict_t *self, G_Message_t *msg)
{
	if(msg)
		if(!(self->svflags & SVF_PARTS_GIBBED))
			DismemberMsgHandler(self, msg);
}

void ssithra_pain(edict_t *self, G_Message_t *msg)
{//fixme - make part fly dir the vector from hit loc to sever loc
	int inwater;
	int				temp, damage;
	qboolean		force_pain;
	

	if(self->deadflag == DEAD_DEAD) //Dead but still being hit	
		return;

	ParseMsgParms(msg, "eeiii", &temp, &temp, &force_pain, &damage, &temp);

	if(!force_pain)
	{
		if(self->pain_debounce_time)
			if(irand(0,10)<5||!self->groundentity)
				return;

		if(self->pain_debounce_time > level.time)
			return;
	}

	ssithraUnCrouch(self);

	self->pain_debounce_time = level.time + 2;

	if(irand(0,10)<5)
		gi.sound (self, CHAN_VOICE, sounds[SND_PAIN1], 1, ATTN_NORM, 0);
	else
		gi.sound (self, CHAN_VOICE, sounds[SND_PAIN2], 1, ATTN_NORM, 0);


	inwater = ssithraCheckInWater(self);

	if(inwater)
	{//underwater pain sound?
		if(self->curAnimID!=ANIM_SWIMFORWARD)
			SetAnim(self, ANIM_WATER_PAIN_A);
		else//swimming
			SetAnim(self, ANIM_WATER_PAIN_B);
	}
	else 
	{
		SetAnim(self, ANIM_PAIN_A);
	}
}

void ssithra_pain_react (edict_t *self)
{
	if(!self->enemy)
	{
#ifdef _DEVEL
		if(MGAI_DEBUG)
			gi.dprintf("No react to pain\n");
#endif
		SsithraDecideStand(self);
		return;
	}

	if(self->enemy->health<=0||self->enemy == self||!self->enemy->takedamage)
	{
#ifdef _DEVEL
		if(MGAI_DEBUG)
			gi.dprintf("No react to pain\n");
#endif
		self->enemy=NULL;
		SsithraDecideStand(self);
		return;
	}
	//go get him!
#ifdef _DEVEL
	if(MGAI_DEBUG)
		gi.dprintf("pain_react -> run\n");
#endif
	ssithra_decide_gallop(self);
}

//===========================================
//DEATHS
//===========================================

void ssithra_death(edict_t *self, G_Message_t *msg)
{//FIXME: still cut off limbs as dying?
	int inwater;

	if(self->monsterinfo.aiflags&AI_DONT_THINK)
	{
		gi.sound(self,CHAN_BODY,sounds[SND_DIE],1,ATTN_NORM,0);
		if (irand(0,10) < 5)
			SetAnim(self, ANIM_DEATH_B);
		else
			SetAnim(self, ANIM_DEATH_A);
		return;
	}
	self->msgHandler=DyingMsgHandler;

	if(self->deadflag == DEAD_DEAD) //Dead but still being hit	
	{
//		gi.dprintf("already dead!\n");
		return;
	}
	
	self->deadflag = DEAD_DEAD;

	if(self->health <= -80) //gib death
	{
		int	i, num_limbs;

		num_limbs = irand(1, 3);
		for(i = 0; i < num_limbs; i++)
			ssithra_dismember(self, flrand(80, 160), irand(hl_Head, hl_LegLowerRight) | hl_MeleeHit);

		gi.sound(self,CHAN_BODY,sounds[SND_GIB],1,ATTN_NORM,0);
		self->think = BecomeDebris;
		self->nextthink = level.time + 0.1;
		return;
	}

	ssithraUnCrouch(self);
	inwater = ssithraCheckInWater(self);

	if(inwater)
	{
		SetAnim(self, ANIM_WATER_DEATH);
	}
	else
	{
		if (self->health == -69)
		{//maybe allow dead bodies to be chopped?  Make BBOX small?
			self->deadState = DEAD_DEAD;

			gi.linkentity(self);

			self->flags |= FL_DONTANIMATE;

			self->msgHandler = DeadMsgHandler;

			self->svflags |= SVF_DEADMONSTER;	// now treat as a different content type

			SetAnim(self, ANIM_DEAD_B);
		}
		else if (self->health == -33)
			SetAnim(self, ANIM_DEATH_C);
		else if (irand(0,10) < 4 || self->health > -10)//barely dead
			SetAnim(self, ANIM_DEATH_B);
		else
			SetAnim(self, ANIM_DEATH_A);
	}
}

void ssithra_dead(edict_t *self)
{//maybe allow dead bodies to be chopped?  Make BBOX small?
	self->msgHandler = DeadMsgHandler;
	self->svflags |= SVF_DEADMONSTER;	// now treat as a different content type
	self->deadState = DEAD_DEAD;

	self->flags |= FL_DONTANIMATE;

	M_EndDeath(self);
}

void ssithraWaterDead(edict_t *self)
{
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
			
	self->think = FishDeadFloatThink; //TODO: add our own version of fish_deadfloat?
	self->nextthink = level.time + 0.1;

	gi.linkentity (self);
}

void ssithraCollapse (edict_t *self)
{
	vec3_t gore_spot;

	if(irand(0,10)<5)
	{
		self->msgHandler = DefaultMsgHandler;
		SetAnim(self,ANIM_HEADLESSLOOP);
		self->msgHandler = DyingMsgHandler;
		return;
	}
	else
	{
		self->svflags &= ~SVF_DEADMONSTER;	// now treat as a different content type
		self->msgHandler = DefaultMsgHandler;
		VectorCopy(self->s.origin,gore_spot);
		gore_spot[2]+=self->maxs[2]*0.75;
		self->health = 1;
		T_Damage (self, self, self, vec3_origin, gore_spot, vec3_origin, 10, 20,0,MOD_DIED);
		self->health = -33;
	}
}

void ssithraKillSelf (edict_t *self)
{
	vec3_t gore_spot;

	self->svflags &= ~SVF_DEADMONSTER;	// now treat as a different content type
	self->msgHandler = DefaultMsgHandler;
	self->deadflag = false;
	VectorCopy(self->s.origin,gore_spot);
	gore_spot[2]+=12;
	self->health = 1;
	T_Damage (self, self, self, vec3_origin, gore_spot, vec3_origin, 10, 20,0,MOD_DIED);
	self->health = -69;
}

//===========================================
//SOUNDS
//===========================================

void ssithraSound(edict_t *self, float soundnum, float channel, float attenuation)
{
	if(!channel)
		channel = CHAN_AUTO;

	if(!attenuation)
		attenuation = ATTN_NORM;
	else if(attenuation == -1)
		attenuation = ATTN_NONE;

	if(soundnum == SND_SWIM)
		if(irand(0,10)<5)
			soundnum = SND_SWIM2;

	gi.sound(self,channel,sounds[(int)(soundnum)],1,attenuation,0);
}

void ssithraGrowlSound(edict_t *self)
{
	if(!irand(0, 3))
		gi.sound(self,CHAN_VOICE,sounds[irand(SND_GROWL1, SND_GROWL3)],1,ATTN_IDLE,0);
}
//===========================================
//ATTACKS
 //===========================================

void ssithra_melee(edict_t *self, G_Message_t *msg)
{
	if (M_ValidTarget(self, self->enemy))
	{
		if(ssithraCheckInWater(self))
		{
			QPostMessage(self, MSG_MISSILE, PRI_DIRECTIVE, NULL);
			return;
		}

		if(!(self->monsterinfo.aiflags & AI_NO_MISSILE) && !(self->spawnflags&MSF_FIXED))
		{
			if(vhlen(self->enemy->s.origin, self->s.origin) - 16 < flrand(0, self->melee_range))
			{
				SetAnim(self, ANIM_BACKPEDAL);
				return;
			}
		}

		if(M_DistanceToTarget(self, self->enemy) > self->melee_range*2 &&!(self->spawnflags&MSF_FIXED))
			SetAnim(self, ANIM_MELEE);
		else
			SetAnim(self, ANIM_MELEE_STAND);
	}
	else
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
}

void ssithra_missile(edict_t *self, G_Message_t *msg)
{
	int	chance;

	if (M_ValidTarget(self, self->enemy))
	{
		if (ssithraCheckInWater(self))
		{
			if(M_DistanceToTarget(self, self->enemy) < self->melee_range)
			{
				if(self->curAnimID == ANIM_SWIMFORWARD)
					SetAnim(self, ANIM_TRANSUP);
				else
					SetAnim(self, ANIM_WATER_SHOOT);
			}
			else
				ssithraArrow(self);
		}
		else
		{
			if(self->spawnflags & MSF_SSITHRA_CLOTHED)
				chance = 20;
			else
				chance = 80;

			if (irand(0, skill->value * 100) > chance)
				SetAnim(self, ANIM_DUCKSHOOT);
			else
				SetAnim(self, ANIM_SHOOT);
		}
	}
	else
	{
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
	}
}

void ssithra_backup(edict_t *self, G_Message_t *msg)
{
	if (M_ValidTarget(self, self->enemy))
	{
		if(self->spawnflags&MSF_FIXED)
		{
			SetAnim(self, ANIM_DELAY);
			return;
		}
		
		if (ssithraCheckInWater(self))
		{
			SetAnim(self, ANIM_WATER_SHOOT);
		}
		else
		{
			SetAnim(self, ANIM_BACKPEDAL);
		}
	}
	else
	{
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
	}
}

void ssithraSwipe (edict_t *self)
{//use melee swipe functoin in g_monster
	trace_t	trace;
	edict_t *victim;
	vec3_t	soff, eoff, mins, maxs, bloodDir, direction;

	VectorSet(soff, 16, -16, 24);
	VectorSet(eoff, 50, 0, -8);
	
	VectorSet(mins, -2, -2, -2);
	VectorSet(maxs,  2,  2,  2);

	VectorSubtract(soff, eoff, bloodDir);
	VectorNormalize(bloodDir);

	victim = M_CheckMeleeLineHit(self, soff, eoff, mins, maxs, &trace, direction);	

	if (victim)
	{
		if (victim == self)
		{
			//Create a puff effect
			//gi.CreateEffect(NULL, FX_SPARKS, 0, hitPos, "db", vec3_origin, irand(1,3));
		}
		else
		{
			//Hurt whatever we were whacking away at
			gi.sound (self, CHAN_WEAPON, sounds[SND_SWIPEHIT], 1, ATTN_NORM, 0);
			if(self->spawnflags&MSF_SSITHRA_ALPHA)
				T_Damage(victim, self, self, direction, trace.endpos, bloodDir, irand(SSITHRA_DMG_MIN*1.2, SSITHRA_DMG_MAX*1.2), 10, 0,MOD_DIED);
			else
				T_Damage(victim, self, self, direction, trace.endpos, bloodDir, irand(SSITHRA_DMG_MIN, SSITHRA_DMG_MAX), 0, 0,MOD_DIED);
		}
	}
	else
	{
		//Play swoosh sound?
	}
}

// the arrow needs to bounce
void make_arrow_reflect(edict_t *self, edict_t *Arrow)
{
	create_ssith_arrow(Arrow);
	Arrow->s.modelindex = self->s.modelindex;
	VectorCopy(self->s.origin, Arrow->s.origin);
	Arrow->owner = self->owner;
	Arrow->enemy = self->enemy;

	Arrow->touch=self->touch;
	Arrow->nextthink=self->nextthink;
	Arrow->think=G_FreeEdict;
	Arrow->health = self->health;

	Create_rand_relect_vect(self->velocity, Arrow->velocity);

	vectoangles(Arrow->velocity, Arrow->s.angles);
	Arrow->s.angles[YAW]+=90;

	Vec3ScaleAssign(SSITHRA_SPOO_SPEED/2,Arrow->velocity);

	G_LinkMissile(Arrow);
}

void ssithraAlphaArrowTouch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surface)
{
	float damage;
	vec3_t	normal;
	edict_t	*Arrow;
	
	// are we reflecting ?
	if(EntReflecting(other, true, true))
	{
		Arrow = G_Spawn();

		make_arrow_reflect(self,Arrow);

		gi.CreateEffect(&Arrow->s,
			FX_SSITHRA_ARROW,
			CEF_OWNERS_ORIGIN,
			NULL,
			"bv",
			FX_SS_MAKE_ARROW2,
			Arrow->velocity);

		G_SetToFree(self);

		return;
	}


	VectorSet(normal, 0, 0, 1);
	if(plane)
	{
		if(plane->normal)
		{
			VectorCopy(plane->normal, normal);
		}
	}

	if(other->takedamage)
	{
		damage = flrand(SSITHRA_DMG_MIN,SSITHRA_DMG_MAX);
		T_Damage(other,self,self->owner,self->movedir,self->s.origin,normal,damage,0,0,MOD_DIED);
	}
	else
		damage = 0;

	T_DamageRadius(self, self->owner, self->owner, SSITHRA_DMG_ARROW_RADIUS, 
			(20 - damage*2), (30 - damage), DAMAGE_ATTACKER_IMMUNE,MOD_DIED);

	VectorNormalize(self->velocity);

	gi.CreateEffect(NULL,
		FX_SSITHRA_ARROW,
		0,
		self->s.origin, 
		"bv",
		FX_SS_EXPLODE_ARROW2,
		self->velocity);

	VectorClear(self->velocity);

	G_FreeEdict(self);
}

void ssithraArrowTouch (edict_t *self,edict_t *Other,cplane_t *Plane,csurface_t *Surface)
{
	float damage;
	vec3_t	normal;
	edict_t	*Arrow;

	if(Surface&&(Surface->flags&SURF_SKY))
	{
		SkyFly(self);
		return;
	}

	if(EntReflecting(Other, true, true))
	{
		Arrow = G_Spawn();

		make_arrow_reflect(self,Arrow);

		gi.CreateEffect( NULL,
					 FX_M_EFFECTS,
					 CEF_FLAG6,
					 self->s.origin,
					 "bv",
					 FX_MSSITHRA_EXPLODE,
					 self->movedir);
		/*
		gi.CreateEffect(&Arrow->s,
			FX_SSITHRA_ARROW,
			CEF_OWNERS_ORIGIN,
			NULL,
			"bv",
			FX_SS_MAKE_ARROW,
			Arrow->velocity);
		*/

		G_SetToFree(self);

		return;
	}

	if(Other->takedamage)
	{
		VectorSet(normal, 0, 0, 1);
		if(Plane)
		{
			if(Plane->normal)
			{
				VectorCopy(Plane->normal, normal);
			}
		}
		damage = flrand(SSITHRA_DMG_MIN,SSITHRA_DMG_MAX);
		T_Damage(Other,self,self->owner,self->movedir,self->s.origin,normal,damage,0,0,MOD_DIED);
	}

	VectorNormalize(self->velocity);

	gi.CreateEffect( NULL,
				 FX_M_EFFECTS,
				 CEF_FLAG6,
				 self->s.origin,
				 "bv",
				 FX_MSSITHRA_EXPLODE,
				 self->movedir);

	/*
	gi.CreateEffect(NULL,
		FX_SSITHRA_ARROW,
		0,
		self->s.origin, 
		"bv",
		FX_SS_EXPLODE_ARROW,
		self->velocity);
	*/

	VectorClear(self->velocity);

	G_FreeEdict(self);
}

void ssithraArrowExplode(edict_t *self)
{
	int damage = irand(SSITHRA_BIGARROW_DMG_MIN, SSITHRA_BIGARROW_DMG_MAX);

	//TODO: Spawn an explosion effect
	gi.CreateEffect( NULL,
					 FX_M_EFFECTS,
					 0,
					 self->s.origin,
					 "bv",
					 FX_MSSITHRA_EXPLODE,
					 self->movedir);

	T_DamageRadius(self, self->owner, self->owner, 64, damage, damage/2, DAMAGE_ATTACKER_IMMUNE, MOD_DIED);

	G_FreeEdict(self);
}

void ssithraDuckArrowTouch (edict_t *self,edict_t *other,cplane_t *plane,csurface_t *surface)
{
	if(surface&&(surface->flags&SURF_SKY))
	{
		SkyFly(self);
		return;
	}

	//NOTENOTE: NO REFLECTION FOR THIS MISSILE!
	
	if(other->takedamage)
	{
		if (plane->normal)
			VectorCopy(plane->normal, self->movedir);

		self->dmg = irand(SSITHRA_DMG_MIN*2, SSITHRA_DMG_MAX*2);
		ssithraArrowExplode(self);
	}
	else
	{
		VectorClear(self->velocity);
		
		self->s.effects |= EF_MARCUS_FLAG1;

		if (plane->normal)
			VectorCopy(plane->normal, self->movedir);

		self->dmg = irand(SSITHRA_DMG_MIN, SSITHRA_DMG_MAX);

		self->think = ssithraArrowExplode;
		self->nextthink = level.time + flrand(0.5, 1.5);
	}
}

void create_ssith_arrow(edict_t *Arrow)
{
	Arrow->movetype = MOVETYPE_FLYMISSILE;
	Arrow->solid = SOLID_BBOX;
	Arrow->classname = "Ssithra_Arrow";
	Arrow->touch = ssithraArrowTouch;
	Arrow->enemy = NULL;
	Arrow->clipmask = MASK_SHOT;
	Arrow->s.scale = 0.75;
	Arrow->s.effects |= EF_CAMERA_NO_CLIP;
	Arrow->svflags |= SVF_ALWAYS_SEND;
	Arrow->s.modelindex = gi.modelindex("models/objects/exarrow/tris.fm");
}

void ssithraDoArrow(edict_t *self, float z_offs)
{
	vec3_t	Forward,check_lead, right, enemy_dir;
	edict_t	*Arrow;

	if(self->s.fmnodeinfo[MESH__RIGHTARM].flags&FMNI_NO_DRAW)
		return;

	gi.sound(self,CHAN_WEAPON,sounds[SND_ARROW1],1,ATTN_NORM,0);
	self->monsterinfo.attack_finished = level.time + 0.4;
	Arrow = G_Spawn();

	create_ssith_arrow(Arrow);

	if(self->spawnflags & MSF_SSITHRA_ALPHA)
		Arrow->touch=ssithraArrowTouch;
	else
		Arrow->touch=ssithraArrowTouch;

	Arrow->owner=self;
	Arrow->enemy=self->enemy;
	
	Arrow->health = 0; // tell the touch function what kind of arrow we are;

	AngleVectors(self->s.angles, Forward, right, NULL);

	VectorCopy(self->s.origin,Arrow->s.origin);	
	VectorMA(Arrow->s.origin, 16, Forward, Arrow->s.origin);
	VectorMA(Arrow->s.origin, -4, right, Arrow->s.origin);
	Arrow->s.origin[2] += 16;

	VectorCopy(self->movedir,Arrow->movedir);
	vectoangles(Forward,Arrow->s.angles);

	VectorClear(check_lead);
	if(skill->value > 1)
	{
		ExtrapolateFireDirection(self, Arrow->s.origin,
			SSITHRA_SPOO_SPEED, self->enemy, 0.3, check_lead);
	}
	else
	{
		VectorSubtract(self->enemy->s.origin, Arrow->s.origin, enemy_dir);
		VectorNormalize(enemy_dir);
		if(DotProduct(enemy_dir, Forward) >= 0.3)
		{
			Forward[2] = enemy_dir[2];
		}
	}

	if(Vec3IsZero(check_lead))
	{
		VectorScale(Forward,SSITHRA_SPOO_SPEED,Arrow->velocity);
	}
	else
	{
		VectorScale(check_lead,SSITHRA_SPOO_SPEED,Arrow->velocity);
	}

	VectorCopy(Arrow->velocity, Arrow->movedir);
	VectorNormalize(Arrow->movedir);
	vectoangles(Arrow->movedir, Arrow->s.angles);
	Arrow->s.angles[PITCH] = anglemod(Arrow->s.angles[PITCH] * -1);
	Arrow->s.angles[YAW] += 90;

	gi.CreateEffect(&Arrow->s,
		FX_M_EFFECTS,
		CEF_OWNERS_ORIGIN,
		Arrow->s.origin,
		"bv",
		FX_MSSITHRA_ARROW,
		Arrow->velocity);
	
	G_LinkMissile(Arrow); 

	Arrow->nextthink=level.time+3;
	Arrow->think=G_FreeEdict;//ssithraArrowThink;
}

void ssithraDoDuckArrow(edict_t *self, float z_offs)
{
	vec3_t	Forward,check_lead, right, enemy_dir;
	edict_t	*Arrow;

	if(self->s.fmnodeinfo[MESH__RIGHTARM].flags&FMNI_NO_DRAW)
		return;

	gi.sound(self, CHAN_WEAPON, sounds[SND_ARROW_FIRE] , 1, ATTN_NORM, 0);

	self->monsterinfo.attack_finished = level.time + 0.4;
	Arrow = G_Spawn();

	create_ssith_arrow(Arrow);

	Arrow->touch=ssithraDuckArrowTouch;

	Arrow->owner=self;
	Arrow->enemy=self->enemy;
	
	Arrow->health = 0; // tell the touch function what kind of arrow we are;

	AngleVectors(self->s.angles, Forward, right, NULL);

	VectorCopy(self->s.origin,Arrow->s.origin);	
	VectorMA(Arrow->s.origin, 12*self->s.scale, Forward, Arrow->s.origin);
	VectorMA(Arrow->s.origin, 4*self->s.scale, right, Arrow->s.origin);
	Arrow->s.origin[2] += z_offs;

	Arrow->s.scale = 1.5;

	VectorCopy(self->movedir,Arrow->movedir);
	vectoangles(Forward,Arrow->s.angles);

	VectorClear(check_lead);
	if(skill->value > 1)
	{
		ExtrapolateFireDirection(self, Arrow->s.origin,
			SSITHRA_SPOO_SPEED, self->enemy, 0.3, check_lead);
	}
	else
	{
		VectorSubtract(self->enemy->s.origin, Arrow->s.origin, enemy_dir);
		VectorNormalize(enemy_dir);
		if(DotProduct(enemy_dir, Forward) >= 0.3)
		{
			Forward[2] = enemy_dir[2];
		}
	}

	if(Vec3IsZero(check_lead))
	{
		VectorScale(Forward,SSITHRA_SPOO_SPEED*1.5,Arrow->velocity);
	}
	else
	{
		VectorScale(check_lead,SSITHRA_SPOO_SPEED*1.5,Arrow->velocity);
	}

	VectorCopy(Arrow->velocity, Arrow->movedir);
	VectorNormalize(Arrow->movedir);
	vectoangles(Arrow->movedir, Arrow->s.angles);
	Arrow->s.angles[PITCH] = anglemod(Arrow->s.angles[PITCH] * -1);
	Arrow->s.angles[YAW] += 90;

	gi.CreateEffect(&Arrow->s,
					FX_M_EFFECTS,
					CEF_OWNERS_ORIGIN | CEF_FLAG6,
					Arrow->s.origin,
					"bv",
					FX_MSSITHRA_ARROW,
					Arrow->velocity);
	
	G_LinkMissile(Arrow); 

	Arrow->nextthink=level.time+5;
	Arrow->think=ssithraArrowExplode;
}

void ssithraStartDuckArrow(edict_t *self)
{
	vec3_t	startpos, vf, vr;

	AngleVectors(self->s.angles, vf, vr, NULL);
	VectorMA(self->s.origin, 18*self->s.scale, vf, startpos);
	VectorMA(startpos, 4*self->s.scale, vr, startpos);

	gi.sound(self, CHAN_WEAPON, sounds[SND_ARROW_CHARGE] , 1, ATTN_NORM, 0);
	
	gi.CreateEffect(NULL,
					FX_M_EFFECTS,
					0,
					self->s.origin,
					"bv",
					FX_MSSITHRA_ARROW_CHARGE,
					startpos);
}

void ssithraArrow(edict_t *self)
{//fixme; adjust for up/down
	if(!self->enemy)
	{
		SsithraDecideStand(self);
		return;
	}

	if(self->enemy->health<=0)
	{
		self->enemy=NULL;
		SsithraDecideStand(self);
		return;
	}

	if(self->monsterinfo.attack_finished>level.time)
		return;

	if(self->spawnflags & MSF_SSITHRA_ALPHA)
		ssithraDoDuckArrow(self, self->maxs[2] * 0.8);
	else
		ssithraDoArrow(self, 8);
}

void ssithraPanicArrow(edict_t *self)
{//fixme; adjust for up/down
	vec3_t	Forward,firedir;//, up;
	edict_t	*Arrow;

	if(self->s.fmnodeinfo[MESH__RIGHTARM].flags&FMNI_NO_DRAW)
	{
		if(self->curAnimID == ANIM_HEADLESS || self->curAnimID == ANIM_HEADLESSLOOP)
			ssithraKillSelf(self);
		return;
	}

//	gi.dprintf("Ssithra fire panic arrow\n");
	gi.sound(self,CHAN_WEAPON,sounds[SND_ARROW2],1,ATTN_NORM,0);
	self->monsterinfo.attack_finished = level.time + 0.4;
	Arrow = G_Spawn();

//	Arrow->s.modelindex=gi.modelindex("models/objects/projectiles/sitharrow/tris.fm");

	create_ssith_arrow(Arrow);
	
	Arrow->owner=self;

	Arrow->health = 1; // tell the touch function what kind of arrow we are;

	VectorAdd(self->s.angles,self->v_angle_ofs,firedir);
	AngleVectors(firedir,Forward,NULL,NULL);
	VectorCopy(self->s.origin,Arrow->s.origin);	
	VectorMA(Arrow->s.origin,12,Forward,Arrow->s.origin);
	VectorCopy(self->movedir,Arrow->movedir);
	vectoangles(Forward,Arrow->s.angles);
	
	VectorScale(Forward,SSITHRA_SPOO_SPEED,Arrow->velocity);

	vectoangles(Arrow->velocity, Arrow->s.angles);
	Arrow->s.angles[YAW]+=90;
//fixme: redo these- make them look like squid ink?
	gi.CreateEffect(&Arrow->s,
		FX_SSITHRA_ARROW,
		CEF_OWNERS_ORIGIN,
		NULL,
		"bv",
		FX_SS_MAKE_ARROW,
		Arrow->velocity);

	G_LinkMissile(Arrow); 

	Arrow->nextthink=level.time+3;
	Arrow->think=G_FreeEdict;//ssithraArrowThink;
}

void ssithra_water_shoot (edict_t *self)
{
	SetAnim(self,ANIM_WATER_SHOOT);
}

void ssithraCheckLoop (edict_t *self)
{//see if should fire again
	vec3_t	v;
	float	len, melee_range, min_seperation, jump_range;

	if(!self->enemy)
		return;

	if(!AI_IsVisible(self, self->enemy))
		return;

	if(!AI_IsInfrontOf(self, self->enemy))
		return;

	if(irand(0, 100) < self->bypass_missile_chance)
		return;

	VectorSubtract (self->enemy->s.origin, self->s.origin, v);
	len = VectorLength (v);
	melee_range = 64;
	jump_range = 128;
	min_seperation = self->maxs[0] + self->enemy->maxs[0];

	if (AI_IsInfrontOf(self, self->enemy))
	{//don't loop if enemy close enough
		if (len < min_seperation + melee_range)
		{
			QPostMessage(self, MSG_MELEE, PRI_DIRECTIVE, NULL);
			return;
		}
		else if (len < min_seperation + jump_range && irand(0,10)<3)
		{
			VectorScale(v, 3, self->movedir);
			self->movedir[2] += 150;
			SetAnim(self, ANIM_LUNGE);
			return;
		}
	}

	self->monsterinfo.currframeindex = self->monsterinfo.currframeindex - 2;
}

//========================================
//EVASION
//========================================
void ssithraCheckDuckArrow (edict_t *self)
{
	if(M_ValidTarget(self, self->enemy))
	{
		if(MG_IsAheadOf(self, self->enemy))
		{
//			if(M_DistanceToTarget(self, self->enemy)<self->missile_range)
//			{
//				if(clear_visible(self, self->enemy))
//				{
					ssithraDoDuckArrow(self, -18);
//				}
//			}
		}
	}
}

void ssithraCheckUnDuck (edict_t *self)
{
	if(self->evade_debounce_time < level.time)
		SetAnim(self, ANIM_UNDUCK);
	else
		SetAnim(self, ANIM_DUCKLOOP);
}

void ssithraJumpEvade (edict_t *self)
{
	vec3_t	forward;

	AngleVectors(self->s.angles, forward, NULL, NULL);
	VectorSet(self->movedir, 0, 0, 300);
	VectorMA(self->movedir, 200, forward, self->movedir);
	SetAnim(self, ANIM_FJUMP);
}

void ssithraCrouch (edict_t *self)
{
	self->maxs[2] = 0;
	self->viewheight = -6;
	gi.linkentity(self);
	SetAnim(self, ANIM_DUCKSHOOT);
}

void ssithraUnCrouch(edict_t *self)
{
	self->maxs[2] = STDMaxsForClass[self->classID][2] * self->s.scale;
	gi.linkentity(self);
	self->viewheight = self->maxs[2]*0.8;
}

void ssithra_evade (edict_t *self, G_Message_t *msg)
{
	edict_t			*projectile;		
	HitLocation_t	HitLocation;
	int duck_chance, jump_chance;
	int chance;
	float eta;

	ParseMsgParms(msg, "eif", &projectile, &HitLocation, &eta);
	
	switch(HitLocation)
	{
		case hl_Head:
			duck_chance = 90;
			jump_chance = 0;
		break;
		case hl_TorsoFront://split in half?
			duck_chance = 70;
			jump_chance = 0;
		break;
		case hl_TorsoBack://split in half?
			duck_chance = 70;
			jump_chance = 0;
		break;
		case hl_ArmUpperLeft:
			duck_chance = 60;
			jump_chance = 0;
		break;
		case hl_ArmLowerLeft://left arm
			duck_chance = 20;
			jump_chance = 30;
		break;
		case hl_ArmUpperRight:
			duck_chance = 60;
			jump_chance = 0;
		break;
		case hl_ArmLowerRight://right arm
			duck_chance = 20;
			jump_chance = 30;
		break;
		case hl_LegUpperLeft:
			duck_chance = 0;
			jump_chance = 50;
		break;
		case hl_LegLowerLeft://left leg
			duck_chance = 0;
			jump_chance = 90;
		break;
		case hl_LegUpperRight:
			duck_chance = 0;
			jump_chance = 50;
		break;
		case hl_LegLowerRight://right leg
			duck_chance = 0;
			jump_chance = 90;
		break;
		default:
			duck_chance = 20;
			jump_chance = 10;
		break;
	}

	if(!(self->spawnflags&MSF_FIXED))
	{
		chance = irand(0, 100);
		if(chance < jump_chance)
		{
			ssithraJumpEvade(self);
			return;
		}
	}

	chance = irand(0, 100);
	if(chance < duck_chance)
	{
		self->evade_debounce_time = level.time + eta;
		ssithraCrouch(self);
		return;
	}
}


//========================================
//MOODS
//========================================
qboolean SsithraCheckMood (edict_t *self)
{
	if(self->spawnflags & MSF_FIXED && self->curAnimID == ANIM_DELAY && self->enemy)
	{
		self->monsterinfo.searchType = SEARCH_COMMON;
		MG_FaceGoal(self, true);
	}

	self->mood_think(self);

	if(self->ai_mood == AI_MOOD_NORMAL)
		return false;

	switch (self->ai_mood)
	{
		case AI_MOOD_ATTACK:
			if(self->ai_mood_flags & AI_MOOD_FLAG_MISSILE)
				QPostMessage(self, MSG_MISSILE, PRI_DIRECTIVE, NULL);
			else
				QPostMessage(self, MSG_MELEE, PRI_DIRECTIVE, NULL);
			return true;
			break;
		case AI_MOOD_PURSUE:
		case AI_MOOD_NAVIGATE:
			//gi.dprintf("Pursue- Nav\n");
			QPostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
			return true;
			break;
		case AI_MOOD_WALK:
			QPostMessage(self, MSG_WALK, PRI_DIRECTIVE, NULL);
			return true;
			break;
		case AI_MOOD_STAND:
			SsithraDecideStand(self);
			return true;
			break;
			
		case AI_MOOD_DELAY:
#ifdef _DEVEL
			if(MGAI_DEBUG)
				gi.dprintf("Delay on frame %d\n", self->monsterinfo.currframeindex);
#endif
			SetAnim(self, ANIM_DELAY);
			return true;
			break;
			
		case AI_MOOD_WANDER:
			if(ssithraCheckInWater(self))
				SetAnim(self, ANIM_SWIMWANDER);
			else
				SetAnim(self, ANIM_WALK1);
			return true;
			break;

		case AI_MOOD_BACKUP:
			QPostMessage(self, MSG_FALLBACK, PRI_DIRECTIVE, NULL);
			return true;
			break;

		case AI_MOOD_JUMP:
			if(self->spawnflags&MSF_FIXED)
				SetAnim(self, ANIM_DELAY);
			else
				SetAnim(self, ANIM_FJUMP);
			return true;
			break;

		default :
#ifdef _DEVEL
//			if(MGAI_DEBUG)
				gi.dprintf("ssithra: Unusable mood %d!\n", self->ai_mood);
#endif
			break;
	}
	return false;
}

void ssithra_check_mood (edict_t *self, G_Message_t *msg)
{
	ParseMsgParms(msg, "i", &self->ai_mood);

	SsithraCheckMood(self);
}

/*-----------------------------------------------
	ssithra_sight
-----------------------------------------------*/
#define SSITHRA_SUPPORT_RADIUS 200

void ssithra_sight (edict_t *self, G_Message_t *msg)
{
	edict_t	*enemy = NULL;
	byte	sight_type;
	int		sound;

	if(self->targetname || self->monsterinfo.c_mode)
		return;//cinematic waiting to be activated, don't do this

	//Have we already said something?
	if (self->monsterinfo.supporters != -1)
		return;
	
	ParseMsgParms(msg, "be", &sight_type, &enemy);

	//See if we are the first to see the player
	if(M_CheckAlert(self, SSITHRA_SUPPORT_RADIUS))
	{
		sound = irand(SND_SIGHT1, SND_SIGHT6);
		gi.sound(self, CHAN_BODY, sounds[sound], 1, ATTN_NORM, 0);
	}
}

qboolean ssithraAlerted (edict_t *self, alertent_t *alerter, edict_t *enemy)
{
	alertent_t	*checkent = NULL;
	vec3_t	saveangles;

	if(self->alert_time < level.time)
	{//not looking around
		if(!(alerter->alert_svflags&SVF_ALERT_NO_SHADE) && skill->value < 3.0 && !(self->monsterinfo.aiflags & AI_NIGHTVISION))
		{//not a sound-alert, 
			if(enemy->light_level < flrand(6, 77))
			{
				return false;
			}
		}
	}

	//the alert action happened in front of me, but the enemy is behind or the alert is behind me
	if(!MG_IsInforntPos(self, alerter->origin))
	{
		if(irand(0, 1)&&self->curAnimID!=ANIM_IDLEBASIC)
		{//50% chance of startling them up if not already in startle anim
			//startle me, but don't wake up just yet
			if(alerter->lifetime < level.time + 4)
				self->alert_time = level.time + 4;//be ready for 4 seconds to wake up if alerted again
			else
				self->alert_time = alerter->lifetime;//be alert as long as the alert sticks around
			VectorCopy(self->v_angle_ofs, saveangles);
			VectorClear(self->v_angle_ofs);
			self->v_angle_ofs[YAW]=-90;

			if(MG_IsInforntPos(self, alerter->origin))//fancy way of seeing if explosion was to right
			{
				VectorCopy(saveangles,self->v_angle_ofs);
				SetAnim(self, ANIM_IDLERIGHT); //mxd. Inlined ssithraLookRight() //FIXME: if already looking right, see you.
			}
			else
			{
				VectorCopy(saveangles,self->v_angle_ofs);
				SetAnim(self, ANIM_STARTLE); //mxd. Inlined ssithraStartle().
			}
			return false;
		}
		else//spin around and wake up!
			self->spawnflags |= MSF_SSITHRA_SPIN;
	}
	else if(!AI_IsInfrontOf(self,enemy))
	{
		if(irand(0, 1)&&self->curAnimID!=ANIM_IDLEBASIC)
		{//50% chance of startling them up if not already in startle anim
			//startle me, but don't wake up just yet
			self->alert_time = level.time + 4;//be ready to wake up for next 4 seconds
			VectorCopy(self->v_angle_ofs, saveangles);
			VectorClear(self->v_angle_ofs);
			self->v_angle_ofs[YAW]=-90;

			if(AI_IsInfrontOf(self, enemy))//fancy way of seeing if explosion was to right
			{
				VectorCopy(saveangles,self->v_angle_ofs);
				SetAnim(self, ANIM_IDLERIGHT); //mxd. Inlined ssithraLookRight() //FIXME: if already looking right, see you.
			}
			else
			{
				VectorCopy(saveangles,self->v_angle_ofs);
				SetAnim(self, ANIM_STARTLE); //mxd. Inline ssithraStartle().
			}
			return false;
		}
		else//spin around and wake up!
			self->spawnflags |= MSF_SSITHRA_SPIN;
	}
	
	if(checkent)
	{//enemy of alert is behind me
	}

	if(enemy->svflags&SVF_MONSTER)
		self->enemy = alerter->enemy;
	else
		self->enemy = enemy;

	AI_FoundTarget(self, true);

	return true;
}
//================================================================================


void SsithraStaticsInit(void)
{
	static ClassResourceInfo_t resInfo;

	classStatics[CID_SSITHRA].msgReceivers[MSG_STAND] = SsithraStandMsgHandler;
	classStatics[CID_SSITHRA].msgReceivers[MSG_WALK] = SsithraWalkMsgHandler;
	classStatics[CID_SSITHRA].msgReceivers[MSG_RUN] = SsithraRunMsgHandler;
	classStatics[CID_SSITHRA].msgReceivers[MSG_MELEE] = ssithra_melee;
	classStatics[CID_SSITHRA].msgReceivers[MSG_MISSILE] = ssithra_missile;
	classStatics[CID_SSITHRA].msgReceivers[MSG_WATCH] = SsithraWatchMsgHandler;
	classStatics[CID_SSITHRA].msgReceivers[MSG_PAIN] = ssithra_pain;
	classStatics[CID_SSITHRA].msgReceivers[MSG_DEATH] = ssithra_death;
	classStatics[CID_SSITHRA].msgReceivers[MSG_DISMEMBER] = DismemberMsgHandler;
	classStatics[CID_SSITHRA].msgReceivers[MSG_JUMP] = SsithraJumpMsgHandler;
	classStatics[CID_SSITHRA].msgReceivers[MSG_FALLBACK] = ssithra_backup;
	classStatics[CID_SSITHRA].msgReceivers[MSG_DEATH_PAIN] = ssithra_dead_pain;
	classStatics[CID_SSITHRA].msgReceivers[MSG_EVADE] = ssithra_evade;
	classStatics[CID_SSITHRA].msgReceivers[MSG_CHECK_MOOD] = ssithra_check_mood;
	classStatics[CID_SSITHRA].msgReceivers[MSG_VOICE_SIGHT] = ssithra_sight;
	

	resInfo.numAnims = NUM_ANIMS;
	resInfo.animations = animations;
	resInfo.modelIndex = gi.modelindex("models/monsters/ssithra/tris.fm");

	sounds[SND_PAIN1]=gi.soundindex("monsters/pssithra/pain1.wav");
	sounds[SND_PAIN2]=gi.soundindex("monsters/pssithra/pain2.wav");
	sounds[SND_DIE]=gi.soundindex("monsters/pssithra/die.wav");
	sounds[SND_GIB]=gi.soundindex("monsters/pssithra/gib.wav");
	sounds[SND_SWIPEHIT]=gi.soundindex("monsters/pssithra/swipehit.wav");
	sounds[SND_ARROW1]=gi.soundindex("monsters/pssithra/arrow1.wav");
	sounds[SND_ARROW2]=gi.soundindex("monsters/pssithra/arrow2.wav");
	sounds[SND_GROWL1]=gi.soundindex("monsters/pssithra/growl1.wav");
	sounds[SND_GROWL2] = gi.soundindex ("monsters/pssithra/growl2.wav");
	sounds[SND_GROWL3] = gi.soundindex ("monsters/pssithra/growl3.wav");
	sounds[SND_INWATER] = gi.soundindex ("monsters/pssithra/inwater.wav");
	sounds[SND_NAMOR] = gi.soundindex ("monsters/pssithra/namor.wav");
	sounds[SND_LAND] = gi.soundindex ("monsters/pssithra/land.wav");
	sounds[SND_SWIPE] = gi.soundindex ("monsters/pssithra/swipe.wav");
	sounds[SND_SWIM] = gi.soundindex ("monsters/pssithra/swim.wav");
	sounds[SND_SWIM2] = gi.soundindex ("monsters/pssithra/swim2.wav");

	sounds[SND_SIGHT1] = gi.soundindex ("monsters/pssithra/ssithvoice1.wav");
	sounds[SND_SIGHT2] = gi.soundindex ("monsters/pssithra/ssithvoice2.wav");
	sounds[SND_SIGHT3] = gi.soundindex ("monsters/pssithra/ssithvoice3.wav");
	sounds[SND_SIGHT4] = gi.soundindex ("monsters/pssithra/ssithvoice4.wav");
	sounds[SND_SIGHT5] = gi.soundindex ("monsters/pssithra/ssithvoice5.wav");
	sounds[SND_SIGHT6] = gi.soundindex ("monsters/pssithra/ssithvoice6.wav");

	sounds[SND_ARROW_CHARGE] = gi.soundindex ("monsters/pssithra/guncharge.wav");
	sounds[SND_ARROW_FIRE] = gi.soundindex ("monsters/pssithra/gunfire.wav");

	resInfo.numSounds = NUM_SOUNDS;
	resInfo.sounds = sounds;

	classStatics[CID_SSITHRA].resInfo = &resInfo;
}

/*QUAKED monster_ssithra (1 .5 0) (-16 -16 -32) (16 16 26) AMBUSH ASLEEP 4 Namor Spin ToughGuy Clothed FIXED WANDER MELEE_LEAD STALK COWARD EXTRA1 EXTRA2 EXTRA3 EXTRA4

The plague ssithra 

AMBUSH - Will not be woken up by other monsters or shots from player

ASLEEP - will not appear until triggered

WALKING- Use WANDER instead

WANDER - Monster will wander around aimlessly (but follows buoys)

MELEE_LEAD - Monster will tryto cut you off when you're running and fighting him, works well if there are a few monsters in a group, half doing this, half not

STALK - Monster will only approach and attack from behind- if you're facing the monster it will just stand there.  Once the monster takes pain, however, it will stop this behaviour and attack normally

COWARD - Monster starts off in flee mode- runs away from you when woken up

"homebuoy" - monsters will head to this buoy if they don't have an enemy ("homebuoy" should be targetname of the buoy you want them to go to)

"wakeup_target" - monsters will fire this target the first time it wakes up (only once)

"pain_target" - monsters will fire this target the first time it gets hurt (only once)

mintel - monster intelligence- this basically tells a monster how many buoys away an enemy has to be for it to give up.

melee_range - How close the player has to be, maximum, for the monster to go into melee.  If this is zero, the monster will never melee.  If it is negative, the monster will try to keep this distance from the player.  If the monster has a backup, he'll use it if too clode, otherwise, a negative value here means the monster will just stop running at the player at this distance.
	Examples:
		melee_range = 60 - monster will start swinging it player is closer than 60
		melee_range = 0 - monster will never do a mele attack
		melee_range = -100 - monster will never do a melee attack and will back away (if it has that ability) when player gets too close

missile_range - Maximum distance the player can be from the monster to be allowed to use it's ranged attack.

min_missile_range - Minimum distance the player can be from the monster to be allowed to use it's ranged attack.

bypass_missile_chance - Chance that a monster will NOT fire it's ranged attack, even when it has a clear shot.  This, in effect, will make the monster come in more often than hang back and fire.  A percentage (0 = always fire/never close in, 100 = never fire/always close in).- must be whole number

jump_chance - every time the monster has the opportunity to jump, what is the chance (out of 100) that he will... (100 = jump every time)- must be whole number

wakeup_distance - How far (max) the player can be away from the monster before it wakes up.  This just means that if the monster can see the player, at what distance should the monster actually notice him and go for him.

DEFAULTS:
mintel					= 28
melee_range				= 48
missile_range			= 512
min_missile_range		= 48
bypass_missile_chance	= 25
jump_chance				= 100
wakeup_distance			= 1024

NOTE: A value of zero will result in defaults, if you actually want zero as the value, use -1
*/
void SP_monster_plague_ssithra (edict_t *self)
{
	qboolean alpha = true;

	if(!skill->value)
	{
		if(!irand(0, 2))//30% - but won't fire explosives
			self->spawnflags |= MSF_SSITHRA_CLOTHED;
	}
	else if (skill->value == 1)
	{
		if(!irand(0, 3))//25%
			self->spawnflags |= MSF_SSITHRA_CLOTHED;
	}
	else
	{
		if(irand(0, 1))//50%
			self->spawnflags |= MSF_SSITHRA_CLOTHED;
	}

	if(self->spawnflags&MSF_SSITHRA_NAMOR)
		self->spawnflags |= MSF_AMBUSH;

	// Generic Monster Initialization
	if (!M_Start(self))
		return;					// Failed initialization

	self->msgHandler = DefaultMsgHandler;
	self->monsterinfo.alert = ssithraAlerted;
	self->think = M_WalkmonsterStartGo;
	self->monsterinfo.dismember = ssithra_dismember;

	self->materialtype = MAT_FLESH;
//	self->monsterinfo.aiflags |= AI_SWIM_OK;
	self->flags |= FL_IMMUNE_SLIME;
	
	ssithraCheckInWater(self);

	self->isBlocked = SsithraBlocked;

	if(self->health<=0)
		self->health = SSITHRA_HEALTH;

	//Apply to the end result (whether designer set or not)
	self->max_health = self->health = MonsterHealth(self->health);

	self->mass = SSITHRA_MASS;
	self->yaw_speed = 20;

	self->movetype = PHYSICSTYPE_STEP;
	VectorClear(self->knockbackvel);

	self->solid=SOLID_BBOX;

	self->monsterinfo.supporters = -1;
	VectorCopy(STDMinsForClass[self->classID], self->mins);
	VectorCopy(STDMaxsForClass[self->classID], self->maxs);	
	self->viewheight = self->maxs[2]*0.8;

	self->s.modelindex = classStatics[CID_SSITHRA].resInfo->modelIndex;

	self->touch = M_Touch;
	
	if(self->spawnflags & MSF_SSITHRA_CLOTHED)
		self->s.skinnum = 2;
	else
		self->s.skinnum = 0;

	//scaling them up in code like this is bad for designers
	self->s.scale = self->monsterinfo.scale = (MODEL_SCALE)+0.1;// + flrand(0.1, 0.3));
	// Note that at less than 110% of the size 

	//Turn off dismemberment caps, can't see them, so save some polys
	self->s.fmnodeinfo[MESH__CAPLOWERTORSO].flags |= FMNI_NO_DRAW;
	self->s.fmnodeinfo[MESH__CAPTOPUPPERTORSO].flags |= FMNI_NO_DRAW;
	self->s.fmnodeinfo[MESH__CAPBOTTOMUPPERTORSO].flags |= FMNI_NO_DRAW;
	self->s.fmnodeinfo[MESH__CAPHEAD].flags |= FMNI_NO_DRAW;
	if(!(self->spawnflags&MSF_SSITHRA_ALPHA))
	{
		if(irand(0,10)<6)
		{
			if(irand(0,10)<5)
			{
				self->s.fmnodeinfo[MESH__CENTERSPIKE].flags |= FMNI_USE_SKIN;
				self->s.fmnodeinfo[MESH__CENTERSPIKE].skin = self->s.skinnum+1;
			}
			else
				self->s.fmnodeinfo[MESH__CENTERSPIKE].flags |= FMNI_NO_DRAW;
			alpha = false;
		}
		if(irand(0,10)<6)
		{
			if(irand(0,10)<5)
			{
				self->s.fmnodeinfo[MESH__LEFT1SPIKE].flags |= FMNI_USE_SKIN;
				self->s.fmnodeinfo[MESH__LEFT1SPIKE].skin = self->s.skinnum+1;
			}
			else
				self->s.fmnodeinfo[MESH__LEFT1SPIKE].flags |= FMNI_NO_DRAW;
			alpha = false;
		}
		if(irand(0,10)<6)
		{
			if(irand(0,10)<5)
			{
				self->s.fmnodeinfo[MESH__RIGHT1SPIKE].flags |= FMNI_USE_SKIN;
				self->s.fmnodeinfo[MESH__RIGHT1SPIKE].skin = self->s.skinnum+1;
			}
			else
				self->s.fmnodeinfo[MESH__RIGHT1SPIKE].flags |= FMNI_NO_DRAW;
			alpha = false;
		}
		if(irand(0,10)<6)
		{
			if(irand(0,10)<5)
			{
				self->s.fmnodeinfo[MESH__RIGHT2SPIKE].flags |= FMNI_USE_SKIN;
				self->s.fmnodeinfo[MESH__RIGHT2SPIKE].skin = self->s.skinnum+1;
			}
			else
				self->s.fmnodeinfo[MESH__RIGHT2SPIKE].flags |= FMNI_NO_DRAW;
			alpha = false;
		}
	}

	self->s.color.a = 255;
	if(alpha)//tough guy!
	{//TODO: other ssithras won't attack this guy and will follow him
		self->health += 75;
		self->s.scale = self->monsterinfo.scale = MODEL_SCALE + 0.5;
		self->spawnflags|=MSF_SSITHRA_ALPHA;
		self->s.color.r = 255;
		self->s.color.g = 255;
		self->s.color.b = 128;
	}
	else
	{
		self->s.color.r = 200 + irand(-50, 50);
		self->s.color.g = 200 + irand(-50, 50);
		self->s.color.b = 200 + irand(-50, 50);
	}

	self->monsterinfo.otherenemyname = "obj_barrel";

	//set up my mood function
	MG_InitMoods(self);
	if(!irand(0,2))
		self->ai_mood_flags |= AI_MOOD_FLAG_PREDICT;

	QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
	self->svflags |= SVF_WAIT_NOTSOLID;
	self->flags |= FL_AMPHIBIAN;

//	if(self->spawnflags & MSF_SSITHRA_NAMOR)
//		self->use = ssithraNamorTriggered;
}


/*QUAKED obj_corpse_ssithra (1 .5 0) (-30 -12 -2) (30 12 2) INVULNERABLE ANIMATE EXPLODING NOPUSH
A dead plague ssithra
---------- KEYS -----------------  
style - skin of ssithra (default 0)
0 - damage skin
1 - some bad bad skin. (not used)
2 - normal skin
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (corpse can't be pushed)
-----------------------------------
*/
void SP_obj_corpse_ssithra(edict_t *self)
{
	self->s.origin[2] += 26.0;

	VectorSet(self->mins,-30,-12,-2);
	VectorSet(self->maxs,30,12,2);

	self->s.modelindex = gi.modelindex("models/monsters/ssithra/tris.fm");

	self->s.frame = FRAME_death_a12;	//Ths is the reason the function can't be put in g_obj.c

	// Setting the skinnum correctly
	if (!self->style)
		self->s.skinnum = 1;
	else
		self->s.skinnum = 0;

	self->spawnflags |= SF_OBJ_NOPUSH;	// Can't be pushed
	self->svflags |= SVF_DEADMONSTER;//doesn't block walking

	ObjectInit(self,120,80,MAT_FLESH,SOLID_BBOX);
}

