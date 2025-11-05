//
// m_beast.c
//
// Copyright 1998 Raven Software
//

#include <float.h>
#include "m_beast.h"
#include "m_beast_shared.h"
#include "m_beast_anim.h"
#include "mg_ai.h" //mxd
#include "mg_guide.h" //mxd
#include "g_debris.h" //mxd
#include "g_DefaultMessageHandler.h"
#include "g_monster.h"
#include "g_Physics.h"
#include "m_stats.h"
#include "p_anims.h"
#include "Random.h"
#include "Utilities.h"
#include "Vector.h"

#pragma region ========================== Trial Beast base info ==========================

#define MAX_CHOMP_DIST	64.0f //mxd

static vec3_t left_foot_offset_for_frame_index[18] = //mxd. Named 'GetLeftFootOffsetForFrameIndex' in original logic.
{
	{  160.0f, -64.0f,  0.0f },
	{  144.0f, -60.0f,  0.0f },
	{  96.0f,  -56.0f,  0.0f },
	{  64.0f,  -52.0f,  0.0f },
	{  16.0f,  -48.0f, -2.0f },
	{ -48.0f,  -44.0f, -4.0f },
	{ -80.0f,  -40.0f, -2.0f },
	{ -112.0f, -40.0f,  0.0f },
	{ -160.0f, -40.0f, -4.0f },
	{ -192.0f, -40.0f,  0.0f },
	{ -120.0f, -42.0f,  2.0f },
	{ -48.0f,  -44.0f,  6.0f },
	{  0.0f,   -44.0f,  14.0f },
	{  24.0f,  -42.0f,  10.0f },
	{  28.0f,  -48.0f,  16.0f },
	{  34.0f,  -48.0f,  14.0f },
	{  110.0f, -58.0f,  0.0f },
	{  144.0f, -72.0f,  8.0f },
};

static vec3_t right_foot_offset_for_frame_index[18] = //mxd. Named 'GetRightFootOffsetForFrameIndex' in original logic.
{
	{ -160.0f, 32.0f,  0.0f },
	{ -144.0f, 32.0f,  12.0f },
	{ -96.0f,  28.0f,  14.0f },
	{ -64.0f,  28.0f,  18.0f },
	{ -16.0f,  28.0f,  22.0f },
	{  32.0f,  32.0f,  28.0f },
	{  64.0f,  38.0f,  28.0f },
	{  104.0f, 40.0f,  24.0f },
	{  160.0f, 48.0f, -4.0f },
	{  128.0f, 52.0f, -8.0f },
	{  112.0f, 54.0f,  0.0f },
	{  108.0f, 52.0f,  0.0f },
	{  72.0f,  48.0f, -2.0f },
	{  32.0f,  40.0f,  0.0f },
	{ -4.0f,   36.0f,  0.0f },
	{ -24.0f,  34.0f,  0.0f },
	{ -80.0f,  32.0f,  2.0f },
	{ -128.0f, 32.0f, -2.0f },
};

static const animmove_t* animations[NUM_ANIMS] =
{
	&tbeast_move_biteup,
	&tbeast_move_bitelow,
	&tbeast_move_biteup2,
	&tbeast_move_eating_twitch,
	&tbeast_move_eating,
	&tbeast_move_eatdown,
	&tbeast_move_walk,
	&tbeast_move_walkleft,
	&tbeast_move_walkrt,
	&tbeast_move_jump,
	&tbeast_move_forced_jump,
	&tbeast_move_inair,
	&tbeast_move_land,
	&tbeast_move_ginair,
	&tbeast_move_gland,
	&tbeast_move_stand,
	&tbeast_move_delay,
	&tbeast_move_die,
	&tbeast_move_die_norm,
	&tbeast_move_charge,
	&tbeast_move_roar,
	&tbeast_move_walkatk,
	&tbeast_move_stun,
	&tbeast_move_snatch,
	&tbeast_move_ready_catch,
	&tbeast_move_catch,
	&tbeast_move_biteup_sfin,
	&tbeast_move_bitelow_sfin,
	&tbeast_move_biteup2_sfin,
	&tbeast_move_quick_charge
};

static int sounds[NUM_SOUNDS];

#pragma endregion

#pragma region ========================== Public utility functions ==========================

qboolean TBeastCheckBottom(edict_t* self)
{
	vec3_t end;
	VectorCopy(self->s.origin, end);
	end[2] -= 1.0f;

	trace_t trace;
	gi.trace(self->s.origin, self->mins, self->maxs, end, self, MASK_ALL, &trace);

	//mxd. Skip non-functional stomping damage logic.

	if (trace.fraction < 1.0f || trace.startsolid || trace.allsolid)
	{
		self->groundentity = trace.ent;
		return true;
	}

	return false;
}

qboolean TBeastCheckJump(edict_t* self)
{
	qboolean skip_low = false;

	if (self->enemy != NULL)
	{
		if (!MG_IsAheadOf(self, self->enemy))
			return false;

		if (vhlen(self->enemy->s.origin, self->s.origin) < 200.0f)
		{
			const float z_diff = self->s.origin[2] + TB_HIBITE_U + TB_UP_OFFSET - self->enemy->s.origin[2];

			if (z_diff < -128.0f)
			{
				SetAnim(self, ANIM_BITEUP2);
				return true;
			}

			if (fabsf(z_diff) <= 32.0f)
			{
				SetAnim(self, ANIM_BITEUP);
				return true;
			}

			if (z_diff < -32.0f && z_diff > -200.0f)
			{
				skip_low = true;
			}
			else if (z_diff > 40.0f && z_diff < -24.0f)
			{
				SetAnim(self, ANIM_BITELOW);
				return true;
			}
		}

		if (self->enemy->s.origin[2] < self->s.origin[2])
			return false;
	}

	if (self->monsterinfo.jump_time > level.time)
		return false;

	vec3_t start;
	VectorCopy(self->s.origin, start);

	vec3_t end;
	VectorCopy(start, end);
	end[2] += self->size[2]; // Try a jump of 186.

	if (!skip_low)
	{
		trace_t tr_low;
		gi.trace(start, self->mins, self->maxs, end, self, MASK_SOLID, &tr_low);

		if (tr_low.fraction == 1.0f && !tr_low.startsolid && !tr_low.allsolid)
		{
			vec3_t forward;
			AngleVectors(self->s.angles, forward, NULL, NULL);

			vec3_t start2;
			VectorCopy(end, start2);

			vec3_t end2;
			VectorMA(end, 64.0f, forward, end2);

			gi.trace(start2, self->mins, self->maxs, end2, self, MASK_SOLID, &tr_low);

			if (tr_low.fraction == 1.0f && !tr_low.startsolid && !tr_low.allsolid)
			{
				// Beast blocked low jump!
				VectorScale(forward, 250.0f, self->movedir);
				self->movedir[2] = 400.0f;
				self->monsterinfo.jump_time = level.time + 7.0f;
				SetAnim(self, ANIM_FJUMP);

				return true;
			}
		}
	}

	// Try a jump of 372.
	end[2] += self->size[2];

	trace_t trace;
	gi.trace(start, self->mins, self->maxs, end, self, MASK_SOLID, &trace);

	if (trace.fraction == 1.0f && !trace.startsolid && !trace.allsolid)
	{
		vec3_t forward;
		AngleVectors(self->s.angles, forward, NULL, NULL);

		vec3_t start2;
		VectorCopy(end, start2);

		vec3_t end2;
		VectorMA(end, 64.0f, forward, end2);

		gi.trace(start2, self->mins, self->maxs, end2, self, MASK_SOLID, &trace);

		if (trace.fraction == 1.0f && !trace.startsolid && !trace.allsolid)
		{
			// Beast blocked high jump!
			VectorScale(forward, 300.0f, self->movedir);
			self->movedir[2] = 600.0f;
			self->monsterinfo.jump_time = level.time + 7.0f;
			SetAnim(self, ANIM_FJUMP);

			return true;
		}
	}

	return false;
}

edict_t* TBeastCheckHit(const vec3_t start, vec3_t end) //mxd. Named 'check_hit_beast' in original logic.
{
	vec3_t shot_dir;
	VectorSubtract(end, start, shot_dir);
	const float shot_dist = VectorNormalize(shot_dir);

	edict_t* found = NULL;
	while ((found = G_Find(found, FOFS(classname), "monster_trial_beast")) != NULL)
	{
		vec3_t beast_dir;
		VectorSubtract(found->s.origin, start, beast_dir);
		const float beast_dist = VectorLength(beast_dir) - 128.0f;

		if (beast_dist > shot_dist)
			continue;

		// Beast closer than trace endpos, let's do an incremental check.
		vec3_t check_pos;
		VectorCopy(start, check_pos);

		for (int i = 16; i < (int)shot_dist; i += 16)
		{
			vec3_t diff_vec;
			VectorMA(check_pos, 16.0f, shot_dir, check_pos);
			VectorSubtract(check_pos, found->s.origin, diff_vec);

			if (VectorLengthSquared(diff_vec) < 16384) // 128 squared.
			{
				// This spot is within 128 of beast origin, so you hit him, ok?
				VectorCopy(check_pos, end);
				return found;
			}
		}
	}

	return NULL;
}

#pragma endregion

#pragma region ========================== Utility functions ==========================

static qboolean IsVisibleToClient(const edict_t* self) //mxd. Named 'visible_to_client' in original logic.
{
	for (int i = 1; i <= game.maxclients; i++) //mxd. 'i = 0' in original logic.
	{
		const edict_t* cl = &g_edicts[i];

		if (cl->client == NULL)
			continue;

		edict_t* temp = G_Spawn();

		VectorSet(temp->s.origin,
			(float)cl->client->playerinfo.pcmd.camera_vieworigin[0] * 0.125f,
			(float)cl->client->playerinfo.pcmd.camera_vieworigin[1] * 0.125f,
			(float)cl->client->playerinfo.pcmd.camera_vieworigin[2] * 0.125f);

		VectorSet(temp->s.angles,
			SHORT2ANGLE(cl->client->playerinfo.pcmd.camera_viewangles[0]),
			SHORT2ANGLE(cl->client->playerinfo.pcmd.camera_viewangles[1]),
			SHORT2ANGLE(cl->client->playerinfo.pcmd.camera_viewangles[2]));

		const qboolean is_visible = (MG_IsInforntPos(temp, self->s.origin) && gi.inPVS(temp->s.origin, self->s.origin));
		G_FreeEdict(temp);

		if (is_visible)
			return true;
	}

	return false;
}

static qboolean HaveShoulderRoomAhead(const edict_t* self) //mxd. Named 'shoulder_room_ahead' in original logic.
{
	vec3_t forward;
	const vec3_t angles = { 0.0f, self->s.angles[YAW], 0.0f };
	AngleVectors(angles, forward, NULL, NULL);

	vec3_t end_pos;
	VectorMA(self->s.origin, 128.0f, forward, end_pos);

	const vec3_t mins = { self->mins[0], self->mins[1], 0.0f };
	const vec3_t maxs = { self->maxs[0], self->maxs[1], 1.0f };

	trace_t trace;
	gi.trace(self->s.origin, mins, maxs, end_pos, self, MASK_SOLID, &trace);

	return (!trace.allsolid && !trace.startsolid && trace.fraction == 1.0f);
}

static qboolean TBeastCheckMood(edict_t* self) //mxd. Named 'tbeastCheckMood' in original logic.
{
	self->mood_think(self);

	if (self->ai_mood == AI_MOOD_NORMAL)
		return false;

	switch (self->ai_mood)
	{
		case AI_MOOD_ATTACK:
		{
			const int msg_id = ((self->ai_mood_flags & AI_MOOD_FLAG_MISSILE) ? MSG_MISSILE : MSG_MELEE); //mxd
			G_PostMessage(self, msg_id, PRI_DIRECTIVE, NULL);
		} break;

		case AI_MOOD_PURSUE:
			self->tbeast_toy_materialtype = 0;
			G_PostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_NAVIGATE:
			SetAnim(self, ANIM_WALK);
			break;

		case AI_MOOD_STAND:
		{
			const int msg_id = ((self->monsterinfo.aiflags & AI_EATING) ? MSG_EAT : MSG_STAND); //mxd
			G_PostMessage(self, msg_id, PRI_DIRECTIVE, NULL);
		} break;

		case AI_MOOD_DELAY:
			G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_EAT:
			G_PostMessage(self, MSG_EAT, PRI_DIRECTIVE, NULL);
			break;
	}

	return true;
}

static qboolean TBeastCanCharge(const edict_t* self) //mxd.
{
	return (self->enemy->classID != CID_TCHECKRIK && ((irand(0, 1) == 1 && AI_IsInfrontOf(self, self->enemy)) || MG_IsAheadOf(self, self->enemy)) && HaveShoulderRoomAhead(self));
}

static void TBeastInitCharge(edict_t* self) //mxd. Named 'tbeast_init_charge' in original logic.
{
	if (!MG_IsAheadOf(self, self->enemy) || irand(0, 3) == 0)
		SetAnim(self, ANIM_QUICK_CHARGE);
	else
		SetAnim(self, ANIM_CHARGE);
}

static void TBeastChomp(edict_t* self, float forward_offset, float right_offset, float up_offset) //mxd. Named 'tbeast_chomp' in original logic.
{
	if (self->enemy == NULL)
		return;

	vec3_t forward;
	vec3_t right;
	vec3_t up;
	AngleVectors(self->s.angles, forward, right, up);

	vec3_t start_pos;
	VectorMA(self->s.origin, forward_offset + TB_FWD_OFFSET, forward, start_pos);
	VectorMA(start_pos, right_offset, right, start_pos);
	VectorMA(start_pos, up_offset + TB_UP_OFFSET, up, start_pos);

	vec3_t end_pos;
	VectorSubtract(self->enemy->s.origin, start_pos, end_pos);

	const float enemy_dist = VectorLength(end_pos);

	if (enemy_dist <= MAX_CHOMP_DIST)
	{
		gi.sound(self, CHAN_WEAPON, sounds[SND_CHOMP], 1.0f, ATTN_NORM, 0.0f);

		vec3_t normal;
		VectorSubtract(self->enemy->s.origin, self->s.origin, normal);
		VectorNormalize(normal);

		const int damage = irand(TB_DMG_BITE_MIN, TB_DMG_BITE_MAX);
		T_Damage(self->enemy, self, self, forward, end_pos, normal, damage, damage / 2, DAMAGE_DISMEMBER, MOD_DIED);
	}
	else if (enemy_dist + 64.0f < MAX_CHOMP_DIST)
	{
		// Let them know it was close and we tried - spittle effect?
		gi.sound(self, CHAN_WEAPON, sounds[SND_SNATCH], 1.0f, ATTN_NORM, 0.0f);
	}
}

// Hacky Trial Beast fake collision and slope-standing code.
static float LerpAngleChange(float cur_angle, float end_angle, const float step)
{
	cur_angle = NormalizeAngleDeg(anglemod(cur_angle));
	end_angle = NormalizeAngleDeg(anglemod(end_angle));

	float diff = end_angle - cur_angle;

	if (FloatIsZeroEpsilon(diff)) //mxd. Avoid direct floats comparison. 
		return 0.0f;

	diff = NormalizeAngleDeg(diff);
	const float final = NormalizeAngleDeg(anglemod(cur_angle + diff / step));

	return final;
}

static int TBeastGetWalkFrame(const edict_t* self) //mxd. Named 'tbeast_inwalkframes' in original logic.
{
	if (self->curAnimID == ANIM_CHARGE || self->curAnimID == ANIM_QUICK_CHARGE)
	{
		switch (self->s.frame)
		{
			case FRAME_charge1: return 2;
			case FRAME_charge2: return 4;
			case FRAME_charge3: return 6;
			case FRAME_charge4: return 8;
			case FRAME_charge5: return 9;
			case FRAME_charge6: return 7;
			case FRAME_charge7: return 13;
			case FRAME_charge8: return 15;
			case FRAME_charge9: return 1;
			case FRAME_charge10: return 0;
			default: return -1;
		}
	}

	if (self->s.frame >= FRAME_walk1 && self->s.frame <= FRAME_walk18)
		return self->monsterinfo.currframeindex;

	if (self->s.frame >= FRAME_wlklft1 && self->s.frame <= FRAME_wlklft18)
		return self->monsterinfo.currframeindex;

	if (self->s.frame >= FRAME_wlkrt1 && self->s.frame <= FRAME_wlkrt18)
		return self->monsterinfo.currframeindex;

	if (self->s.frame >= FRAME_wlkatk1 && self->s.frame <= FRAME_wlkatk18)
		return self->monsterinfo.currframeindex;

	if (self->s.frame >= FRAME_wait1 && self->s.frame <= FRAME_wait14)
		return 16;

	return -1;
}

// I'm a big guy, level me out. What slope am I on?
static void LevelToGround(edict_t* self) //mxd. Removed unused args.
{
	const int leg_check_index = TBeastGetWalkFrame(self);

	if (leg_check_index == -1)
		return;

	vec3_t forward;
	vec3_t right;
	vec3_t up;
	AngleVectors(self->s.angles, forward, right, up);

	// Setup leg checks - only if in these frames.
	const qboolean right_front = (leg_check_index > 5 && leg_check_index < 15);
	vec3_t foot_offset;

	// Left leg.
	vec3_t left_pos;
	VectorCopy(left_foot_offset_for_frame_index[leg_check_index], foot_offset);
	VectorMA(self->s.origin, foot_offset[0] + TB_FWD_OFFSET, forward, left_pos);
	VectorMA(left_pos, foot_offset[1] + TB_RT_OFFSET, right, left_pos);
	VectorMA(left_pos, foot_offset[2] + TB_UP_OFFSET, up, left_pos);

	// Right leg.
	vec3_t right_pos;
	VectorCopy(right_foot_offset_for_frame_index[leg_check_index], foot_offset);
	VectorMA(self->s.origin, foot_offset[0] + TB_FWD_OFFSET, forward, right_pos);
	VectorMA(right_pos, foot_offset[1] + TB_RT_OFFSET, right, right_pos);
	VectorMA(right_pos, foot_offset[2] + TB_UP_OFFSET, up, right_pos);

	vec3_t front_pos;
	vec3_t back_pos;

	if (right_front)
	{
		// This is also the front check.
		VectorCopy(right_pos, front_pos);
		VectorCopy(left_pos, back_pos);
	}
	else
	{
		VectorCopy(left_pos, front_pos);
		VectorCopy(right_pos, back_pos);
	}

	// Set pitch.
	vec3_t bottom1;
	VectorCopy(front_pos, bottom1);
	bottom1[2] -= self->size[2] * 2.0f;

	trace_t trace;
	gi.trace(front_pos, vec3_origin, vec3_origin, bottom1, self, MASK_SOLID, &trace);

	if (trace.fraction == 1.0f)
	{
		self->s.angles[PITCH] = LerpAngleChange(self->s.angles[PITCH], 0.0f, 8.0f);
	}
	else
	{
		VectorCopy(trace.endpos, bottom1);

		vec3_t bottom2;
		VectorCopy(back_pos, bottom2);
		bottom2[2] -= self->size[2] * 2.0f;

		gi.trace(back_pos, vec3_origin, vec3_origin, bottom2, self, MASK_SOLID, &trace);

		if (trace.fraction == 1.0f)
		{
			self->s.angles[PITCH] = LerpAngleChange(self->s.angles[PITCH], 0.0f, 8.0f);
		}
		else
		{
			VectorCopy(trace.endpos, bottom2);

			vec3_t dir;
			VectorSubtract(bottom1, bottom2, dir);

			vec3_t angles;
			vectoangles(dir, angles);

			self->s.angles[PITCH] = LerpAngleChange(self->s.angles[PITCH], angles[PITCH], 8.0f);
		}
	}

	// Set roll.
	VectorCopy(right_pos, bottom1);
	bottom1[2] -= self->size[2] * 2.0f;

	gi.trace(right_pos, vec3_origin, vec3_origin, bottom1, self, MASK_SOLID, &trace);

	if (trace.fraction == 1.0f)
	{
		self->s.angles[ROLL] = LerpAngleChange(self->s.angles[ROLL], 0.0f, 8.0f);
	}
	else
	{
		VectorCopy(trace.endpos, bottom1);

		vec3_t bottom2;
		VectorCopy(left_pos, bottom2);
		bottom2[2] -= self->size[2] * 2.0f;

		gi.trace(left_pos, vec3_origin, vec3_origin, bottom2, self, MASK_SOLID, &trace);

		if (trace.fraction == 1.0f)
		{
			self->s.angles[ROLL] = LerpAngleChange(self->s.angles[ROLL], 0.0f, 8.0f);
		}
		else
		{
			VectorCopy(trace.endpos, bottom2);

			vec3_t dir;
			VectorSubtract(bottom1, bottom2, dir);

			vec3_t angles;
			vectoangles(dir, angles);

			self->s.angles[ROLL] = LerpAngleChange(self->s.angles[ROLL], angles[PITCH], 8.0f);
		}
	}
}

static qboolean BBoxesOverlap(const vec3_t mins1, const vec3_t maxs1, const vec3_t mins2, const vec3_t maxs2) //mxd. Named 'boxes_overlap' in original logic.
{
	for (int i = 0; i < 3; i++)
		if (mins1[i] > maxs2[i] || maxs1[i] < mins2[i])
			return false;

	return true;
}

static void TBeastFakeImpact(edict_t* self, trace_t* trace, const qboolean crush) //mxd. Named 'tbeast_fake_impact' in original logic.
{
	if (trace->ent == NULL || (trace->ent->svflags & SVF_TOUCHED_BEAST) || trace->ent == self->targetEnt) //mxd. Added trace->ent NULL check (was done below in original logic).
		return;

	if (trace->ent->classID == CID_FUNC_DOOR || trace->ent->classID == CID_TCHECKRIK) // We want to pick up and eat insects.
		return;

	if (trace->ent == world || trace->ent->movetype == PHYSICSTYPE_NONE) //mxd. Skip redundant ent->classname check.
		return;

	qboolean throw_them = true;

	if ((trace->ent->client != NULL || (trace->ent->svflags & SVF_MONSTER)) && trace->ent->s.origin[2] > self->absmax[2] - 10.0f)
	{
		//FIXME: chance of throwing them off.
		trace->ent->s.origin[2] = self->absmax[2];
		trace->ent->velocity[2] = 0.0f;
		trace->ent->groundentity = self;

		throw_them = false;
	}

	vec3_t dir;

	if (throw_them)
	{
		vec3_t bottom;
		VectorCopy(self->s.origin, bottom);
		bottom[2] += self->mins[2];

		VectorSubtract(trace->ent->s.origin, bottom, dir);
		VectorNormalize(dir);
	}

	if (trace->ent->takedamage != DAMAGE_NO || AI_IsMovable(trace->ent))
	{
		if (throw_them)
			VectorScale(dir, 200.0f, trace->ent->velocity);
	}
	else if (Vec3NotZero(self->velocity) && trace->fraction < GROUND_NORMAL && AI_IsInfrontOf(self, trace->ent)) //mxd. Use define.
	{
		// Hit pillar?
		if (trace->ent->targetname != NULL && Q_stricmp(trace->ent->targetname, "pillar") == 0) //mxd. stricmp -> Q_stricmp
		{
			//FIXME: In higher skills, less chance of breaking it? Or debounce time?
			if (IsVisibleToClient(self))
			{
				G_UseTargets(trace->ent, self);

				trace->ent->targetname = ""; // So we don't hit them again.
				trace->ent->target = "stop"; // So it doesn't fire the target when it's broken later.
				self->monsterinfo.attack_finished = level.time + 3.0f;

				self->velocity[0] = 0.0f;
				self->velocity[1] = 0.0f;
				self->tbeast_pillars_destroyed++;

				if (self->tbeast_pillars_destroyed >= 2) // Got both pillars, now die.
				{
					self->solid = SOLID_NOT;
					self->takedamage = DAMAGE_NO;
				}
			}

			gi.CreateEffect(&self->s, FX_QUAKE, 0, vec3_origin, "bbb", 4, 3, 7);

			if (self->tbeast_pillars_destroyed < 2 && irand(0, 1) == 1) //mxd. Use 'tbeast_pillars_destroyed' instead of separate stun counter var.
				SetAnim(self, ANIM_STUN);
		}
	}

	if (trace->ent->touch != NULL && trace->ent->solid != SOLID_NOT)
		trace->ent->touch(trace->ent, self, &trace->plane, trace->surface);

	if (trace->ent->isBlocked != NULL && trace->ent->solid != SOLID_NOT)
	{
		trace_t tr = *trace;
		tr.ent = self;
		trace->ent->isBlocked(trace->ent, &tr);
	}

	if (throw_them && trace->ent->takedamage != DAMAGE_NO)
	{
		if (Vec3NotZero(self->velocity))
		{
			// Knock down player?
			if (trace->ent->client != NULL)
			{
				if (trace->ent->health > 30)
					DoImpactDamage(self, trace);

				if (trace->ent->groundentity != NULL && trace->ent->health > 0 && trace->ent->client->playerinfo.lowerseq != ASEQ_KNOCKDOWN)
					P_KnockDownPlayer(&trace->ent->client->playerinfo);
			}
			else
			{
				DoImpactDamage(self, trace);
			}
		}
		else
		{
			if (trace->ent->client != NULL)
			{
				if (irand(0, 5) == 0 || (crush && irand(0, 1) == 0))
					if (trace->ent->client->playerinfo.lowerseq != ASEQ_KNOCKDOWN)
						P_KnockDownPlayer(&trace->ent->client->playerinfo);

				if (crush || trace->ent->health > 30)
					T_Damage(trace->ent, self, self, dir, trace->endpos, dir, irand(TB_DMG_IMPACT_MIN, TB_DMG_IMPACT_MAX), TB_DMG_IMPACT_KB, 0, MOD_DIED);
			}
			else
			{
				T_Damage(trace->ent, self, self, dir, trace->endpos, dir, 1000, 250, 0, MOD_DIED);
			}
		}
	}
	else
	{
		// Knock down player?
		if (trace->ent->client != NULL && trace->ent->groundentity != NULL && trace->ent->health > 0 && trace->ent->client->playerinfo.lowerseq != ASEQ_KNOCKDOWN)
			P_KnockDownPlayer(&trace->ent->client->playerinfo);
	}
}

static void TBeastCheckImpacts(edict_t* self) //mxd. Named 'tbeast_check_impacts' in original logic.
{
	// Used by Trial Beast for Simulated Complex Bounding Box Collision.
	vec3_t forward;
	vec3_t right;
	vec3_t up;
	AngleVectors(self->s.angles, forward, right, up);

	// Setup body check.
	vec3_t start;
	VectorCopy(self->s.origin, start);
	start[2] += 64.0f + TB_UP_OFFSET; // Bottom of torso.
	start[2] += 61.0f; // Halfway up to the top of torso.

	vec3_t end;
	VectorMA(start, 150.0f + TB_FWD_OFFSET, forward, end);

	VectorMA(start, -120.0f, forward, start); // + TB_FWD_OFFSET???

	const int leg_check_index = TBeastGetWalkFrame(self);

	vec3_t left_start;
	vec3_t left_end;
	vec3_t right_start;
	vec3_t right_end;
	vec3_t foot_mins;
	vec3_t foot_maxs;

	if (leg_check_index > -1)
	{
		vec3_t foot_offset;

		// Setup leg checks - FIXME: trace from last footpos to current one.
		VectorSet(foot_mins, -8.0f, -8.0f, 0.0f);
		VectorSet(foot_maxs, 8.0f, 8.0f, 1.0f);

		// Left leg.
		VectorCopy(left_foot_offset_for_frame_index[leg_check_index], foot_offset);
		VectorMA(self->s.origin, foot_offset[0] + TB_FWD_OFFSET, forward, left_end);
		VectorMA(left_end, foot_offset[1] + TB_RT_OFFSET, right, left_end);
		VectorMA(left_end, foot_offset[2] + TB_UP_OFFSET, up, left_end);
		VectorCopy(left_end, left_start);
		left_start[2] += 63.0f;

		// Right leg.
		VectorCopy(right_foot_offset_for_frame_index[leg_check_index], foot_offset);
		VectorMA(self->s.origin, foot_offset[0] + TB_FWD_OFFSET, forward, right_end);
		VectorMA(right_end, foot_offset[1] + TB_RT_OFFSET, right, right_end);
		VectorMA(right_end, foot_offset[2] + TB_UP_OFFSET, up, right_end);
		VectorCopy(right_end, right_start);
		right_start[2] += 63.0f;
	}

	// Do body checks.

	vec3_t mins = { -50.0f, -50.0f, -61.0f };
	vec3_t maxs = {  50.0f,  50.0f,  70.0f };

	trace_t trace;
	gi.trace(start, mins, maxs, end, self, MASK_MONSTERSOLID, &trace); //FIXME: continue the trace if less than 1.0 or save for next touch?

	// Check and see if they're close to my mouth and chomp 'em!
	TBeastFakeImpact(self, &trace, false);

	if (leg_check_index > -1)
	{
		// Do leg checks.

		// Left leg.
		gi.trace(left_start, foot_mins, foot_maxs, left_end, self, MASK_MONSTERSOLID, &trace); //FIXME: continue the trace if less than 1.0 or save for next touch?
		TBeastFakeImpact(self, &trace, true);

		// Right leg.
		//FIXME: continue the trace if less than 1.0 or save for next touch?
		gi.trace(right_start, foot_mins, foot_maxs, right_end, self, MASK_MONSTERSOLID, &trace); //FIXME: continue the trace if less than 1.0 or save for next touch?
		TBeastFakeImpact(self, &trace, true);
	}
	else
	{
		VectorCopy(self->s.origin, end);
		end[2] += self->mins[2];

		VectorCopy(end, start);
		start[2] += 63.0f;

		VectorSet(mins, -32.0f, -32.0f, 0.0f);
		VectorSet(maxs,  32.0f,  32.0f, 1.0f);

		gi.trace(start, mins, maxs, end, self, MASK_MONSTERSOLID, &trace);
		TBeastFakeImpact(self, &trace, false);
	}
}

static void TBeastFakeTouch(edict_t* self) //mxd. Named 'tbeast_fake_touch' in original logic.
{
	// Used by Trial Beast for Simulated Complex Bounding Box Collision.
	edict_t* touch[MAX_EDICTS];
	const int num_touching = gi.BoxEdicts(self->absmin, self->absmax, touch, MAX_EDICTS, AREA_SOLID);
	// Be careful, it is possible to have an entity in this list removed before we get to it (killtriggered).

	if (num_touching == 0) //mxd. 'if (touch[0] == NULL)' in original logic.
	{
		TBeastCheckImpacts(self);
		return;
	}

	vec3_t forward;
	vec3_t right;
	vec3_t up;
	AngleVectors(self->s.angles, forward, right, up);

	const int leg_check_index = TBeastGetWalkFrame(self);

	vec3_t melee_point;
	vec3_t left_foot_mins;
	vec3_t left_foot_maxs;
	vec3_t right_foot_mins;
	vec3_t right_foot_maxs;

	if (leg_check_index > -1)
	{
		// Setup leg checks - FIXME: trace from last footpos to current one.

		// Walking, check melee point in front.
		VectorMA(self->s.origin, self->maxs[2] * 0.5f + TB_UP_OFFSET, up, melee_point);
		VectorMA(melee_point, 150.0f + TB_FWD_OFFSET, forward, melee_point);

		vec3_t foot_offset;
		const vec3_t foot_mins = { -8.0f, -8.0f, 0.0f };
		const vec3_t foot_maxs = {  8.0f,  8.0f, 1.0f }; //TODO: unused! Should add to left_foot_maxs / right_foot_maxs?..

		// Left leg.
		VectorCopy(left_foot_offset_for_frame_index[leg_check_index], foot_offset);

		vec3_t left_end;
		VectorMA(self->s.origin, foot_offset[0] + TB_FWD_OFFSET, forward, left_end);
		VectorMA(left_end, foot_offset[1] + TB_RT_OFFSET, right, left_end);
		VectorMA(left_end, foot_offset[2] + TB_UP_OFFSET, up, left_end);

		VectorAdd(left_end, foot_mins, left_foot_mins);
		VectorCopy(left_foot_mins, left_foot_maxs);
		left_foot_maxs[2] += 64.0f;

		// Right leg.
		VectorCopy(right_foot_offset_for_frame_index[leg_check_index], foot_offset);

		vec3_t right_end;
		VectorMA(self->s.origin, foot_offset[0] + TB_FWD_OFFSET, forward, right_end);
		VectorMA(right_end, foot_offset[1] + TB_RT_OFFSET, right, right_end);
		VectorMA(right_end, foot_offset[2] + TB_UP_OFFSET, up, right_end);

		VectorAdd(right_end, foot_mins, right_foot_mins);
		VectorCopy(right_foot_mins, right_foot_maxs);
		right_foot_maxs[2] += 64.0f;
	}

	// Setup body check.
	vec3_t start;
	VectorCopy(self->s.origin, start);
	start[2] += 64.0f + TB_UP_OFFSET; // Bottom of torso.
	start[2] += 35.0f; // Halfway up to the top of torso.

	vec3_t end;
	VectorMA(start, 150.0f + TB_FWD_OFFSET, forward, end);
	VectorMA(start, -120.0f, forward, start); // + TB_FWD_OFFSET???

	const vec3_t mins = { -50.0f, -50.0f, -61.0f };
	const vec3_t maxs = {  50.0f,  50.0f,  70.0f };

	for (int i = 0; i < num_touching; i++)
	{
		edict_t* other = touch[i];

		if (other == NULL || !other->inuse || other == self || other == self->targetEnt || Q_stricmp(other->classname, "worldspawn") == 0) //mxd. Added 'other' NULL check. stricmp -> Q_stricmp //TODO: replace Q_stricmp with world check?
			continue;

		if (self->curAnimID != ANIM_CHARGE && self->curAnimID != ANIM_QUICK_CHARGE)
		{
			if (leg_check_index > -1 && other->takedamage != DAMAGE_NO && AI_IsMovable(other))
			{
				// Check and see if they're close to my mouth and chomp 'em!
				if (vhlen(other->s.origin, melee_point) < 100.0f)
				{
					self->oldenemy = self->enemy;
					self->enemy = other;
					G_PostMessage(self, MSG_MELEE, PRI_DIRECTIVE, NULL);

					break;
				}
			}
		}

		if (other->classID == CID_TCHECKRIK)
			continue; // We want to pick up and eat insects.

		// Make other solid and size temp for trace.
		const int other_clipmask = other->clipmask;
		const int other_solid = other->solid;

		vec3_t other_mins;
		if (Vec3NotZero(other->mins))
			VectorCopy(other->mins, other_mins);
		else
			VectorSet(other->mins, -1.0f, -1.0f, -1.0f);

		vec3_t other_maxs;
		if (Vec3NotZero(other->maxs))
			VectorCopy(other->maxs, other_maxs);
		else
			VectorSet(other->maxs, 1.0f, 1.0f, 1.0f);

		other->solid = SOLID_BBOX;
		other->clipmask = MASK_ALL;

		// BODY

		trace_t trace;
		gi.trace(start, mins, maxs, end, self, MASK_MONSTERSOLID, &trace); //FIXME: continue the trace if less than 1.0 or save for next touch?

		// Put other back to normal.
		other->solid = other_solid;
		other->clipmask = other_clipmask;
		VectorCopy(other_mins, other->mins);
		VectorCopy(other_maxs, other->maxs);

		const qboolean hit_other = (trace.ent == other); // If hit something with BODY, touch it. //BUGFIX: mxd. When this check succeeds, original logic enables hit_other for all subsequent checks.

		if (!hit_other && other->absmin[2] > self->absmax[2] - 10.0f)
		{
			if (irand(0, 10) == 0)
			{
				// Jump to throw off something.
				SetAnim(self, ANIM_JUMP);

				vec3_t dir;
				VectorSubtract(other->s.origin, self->s.origin, dir);
				VectorNormalize(dir);

				VectorScale(dir, 500.0f, other->velocity);
			}
		}
		else
		{
			qboolean hit_me = hit_other;

			if (!hit_me && leg_check_index > -1)
			{
				VectorAdd(other->s.origin, other->mins, other_mins);
				VectorAdd(other->s.origin, other->maxs, other_maxs);

				hit_me = (BBoxesOverlap(other_mins, other_maxs, left_foot_mins, left_foot_maxs) || BBoxesOverlap(other_mins, other_maxs, right_foot_mins, right_foot_maxs));
			}

			if (hit_me)
			{
				if (other->isBlocked != NULL && other->solid != SOLID_NOT)
				{
					gi.trace(other->s.origin, vec3_origin, vec3_origin, self->s.origin, other, MASK_ALL, &trace);
					trace.ent = self;
					VectorCopy(other->s.origin, trace.endpos);
					other->isBlocked(other, &trace);
				}

				if (other->touch != NULL && other->solid != SOLID_NOT)
				{
					gi.trace(other->s.origin, vec3_origin, vec3_origin, self->s.origin, other, MASK_ALL, &trace);
					trace.ent = self;
					VectorCopy(other->s.origin, trace.endpos);
					other->touch(other, self, &trace.plane, trace.surface);
				}

				if (other == trace.ent)
				{
					// If other still valid, do my impact with it.
					TBeastFakeImpact(self, &trace, false);
					other->svflags |= SVF_TOUCHED_BEAST; // So check_impacts doesn't do anything with it.
				}
			}
		}
	}

	TBeastCheckImpacts(self);

	for (int i = 0; i < num_touching; i++)
		if (touch[i] != NULL) //mxd. Added NULL check.
			touch[i]->svflags &= ~SVF_TOUCHED_BEAST;
}

#pragma endregion

#pragma region ========================== Callback functions ==========================

// Assigned to 'isBlocked' and 'bounce' callbacks.
static void TBeastBlocked(edict_t* self, trace_t* trace) //mxd. Named 'tbeast_blocked' in original logic.
{
	// fake_touch does all the actual damage, this is just a check for the charge stuff.
	if (self->curAnimID == ANIM_CHARGE || (self->curAnimID == ANIM_QUICK_CHARGE && self->s.frame >= FRAME_charge1 && self->s.frame <= FRAME_charge10))
	{
		qboolean stop = false;
		const qboolean hit_slope = (trace->ent == world && Vec3NotZero(trace->plane.normal) && trace->plane.normal[2] > GROUND_NORMAL); //mxd. Use define.

		if (!hit_slope)
		{
			gi.sound(self, CHAN_ITEM, sounds[SND_SLAM], 1.0f, ATTN_NORM, 0.0f);

			if (trace->ent != NULL && !AI_IsMovable(trace->ent) && trace->ent->takedamage == DAMAGE_NO && trace->plane.normal[2] < 0.5f)
				stop = true;
		}

		edict_t* pillar;

		if (trace->ent != NULL && trace->ent->targetname != NULL && Q_stricmp(trace->ent->targetname, "pillar") == 0) //mxd. stricmp -> Q_stricmp
		{
			pillar = trace->ent;
		}
		else
		{
			vec3_t start;
			VectorCopy(self->s.origin, start);
			start[2] = self->absmin[2] + self->size[2] * 0.8f + TB_UP_OFFSET;

			vec3_t forward;
			AngleVectors(self->s.angles, forward, NULL, NULL);

			VectorMA(start, self->maxs[0] * 0.8f + TB_FWD_OFFSET, forward, start);

			vec3_t end;
			VectorMA(start, 150.0f, forward, end);

			const vec3_t mins = { -24.0f, -24.0f, -1.0f };
			const vec3_t maxs = { 24.0f,  24.0f,  1.0f };

			trace_t tr;
			gi.trace(start, mins, maxs, end, self, MASK_SOLID, &tr);

			if (tr.fraction < 1.0f && tr.ent != NULL && tr.ent->targetname != NULL && Q_stricmp(tr.ent->targetname, "pillar") == 0) //mxd. stricmp -> Q_stricmp
				pillar = tr.ent;
			else
				pillar = NULL;
		}

		if (pillar != NULL)
		{
			//FIXME: In higher skills, less chance of breaking it? Or debounce time?
			if (IsVisibleToClient(self))
			{
				self->tbeast_pillars_destroyed++;

				if (self->tbeast_pillars_destroyed >= 2) // Got both pillars, now die.
				{
					self->solid = SOLID_NOT;
					self->takedamage = DAMAGE_NO;
				}

				G_UseTargets(pillar, self);
				pillar->targetname = ""; // So we don't hit them again.
				pillar->target = "stop"; // So it doesn't fire the target when it's broken later.

				self->monsterinfo.attack_finished = level.time + 3.0f;
			}

			stop = true;
		}

		if (stop)
		{
			gi.CreateEffect(&self->s, FX_QUAKE, 0, vec3_origin, "bbb", 4, 3, 7);

			self->velocity[0] = 0.0f;
			self->velocity[1] = 0.0f;

			if (self->tbeast_pillars_destroyed < 2 && irand(0, 1) == 1) //mxd. Use 'tbeast_pillars_destroyed' instead of separate stun counter var.
				SetAnim(self, ANIM_STUN);
		}
	}
}

static void TBeastDieUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'tbeast_go_die' in original logic.
{
	M_ShowLifeMeter(0, 0);

	gi.sound(self, CHAN_VOICE, sounds[SND_DIE], 1.0f, ATTN_NONE, 0.0f);

	self->msgHandler = DeadMsgHandler;
	self->dead_state = DEAD_DEAD;
	self->takedamage = DAMAGE_NO;
	self->solid = SOLID_NOT;
	self->movetype = PHYSICSTYPE_NONE;
	self->clipmask = 0;
	self->health = 0;
	self->post_think = NULL;
	self->next_post_think = -1.0f;
	self->touch = NULL;

	VectorClear(self->mins);
	VectorClear(self->maxs);

	SetAnim(self, ANIM_DIE);
	M_GetSlopePitchRoll(self, NULL);
	G_UseTargets(self, activator);
}

static void TBeastUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'tbeast_go_charge' in original logic.
{
	self->enemy = activator; // Are we certain activator is client?
	//FIXME: do a FoundTarget(self, false);?
	self->dmg = true; // Activate charge mode.
	SetAnim(self, ANIM_CHARGE);
	self->use = TBeastDieUse;
}

static void TBeastTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf) //mxd. Named 'tbeast_touch' in original logic.
{
	TBeastFakeTouch(self);
}

static void TBeastPostThink(edict_t* self) //mxd. Named 'tbeast_post_think' in original logic.
{
#define TBEAST_SBAR_SIZE	3500

	if (self->monsterinfo.awake)
	{
		if (self->tbeast_healthbar_buildup < self->max_health)
		{
			// Initial healthbar buildup.
			const int val = (int)(ceilf((float)self->tbeast_healthbar_buildup / (float)self->max_health * TBEAST_SBAR_SIZE));
			M_ShowLifeMeter(val, val);
			self->tbeast_healthbar_buildup += self->max_health / 10;
		}
		else if (self->health > 0)
		{
			M_ShowLifeMeter((int)(ceilf((float)self->health / (float)self->max_health * TBEAST_SBAR_SIZE)), TBEAST_SBAR_SIZE);
		}
	}

	const qboolean moved = (FloatIsZeroEpsilon(self->s.origin[0] - self->s.old_origin[0]) || FloatIsZeroEpsilon(self->s.origin[1] - self->s.old_origin[1])); //mxd. Avoid direct floats comparison.

	if (moved)
		LevelToGround(self);

	qboolean go_jump = false;

	if (fabsf(self->s.angles[PITCH]) > 45.0f || fabsf(self->s.angles[ROLL]) > 45.0f)
	{
		go_jump = true;
	}
	else if (moved)
	{
		// Raise him up if on flat ground, lower is on slope - to keep feet on ground!
		//FIXME - use checkbottom plane instead?
		float mins_z = self->mins[2];
		self->mins[2] = ((fabsf(self->s.angles[PITCH]) + fabsf(self->s.angles[ROLL])) * 0.5f) / 45.0f * 144.0f - 6.0f + TB_UP_OFFSET;
		mins_z -= self->mins[2];

		if (mins_z != 0.0f)
		{
			vec3_t end;
			VectorCopy(self->s.origin, end);
			end[2] += mins_z;

			trace_t trace;
			gi.trace(self->s.origin, self->mins, self->maxs, end, self, MASK_SOLID, &trace);
			VectorCopy(trace.endpos, self->s.origin);
		}

		gi.linkentity(self);
	}

	VectorCopy(self->s.origin, self->s.old_origin);

	if (irand(0, 10) == 0)
	{
		if (self->curAnimID == ANIM_WALK ||
			self->curAnimID == ANIM_WALKLEFT ||
			self->curAnimID == ANIM_WALKRT ||
			self->curAnimID == ANIM_WALKATK)
		{
			vec3_t end;
			VectorCopy(self->s.origin, end);
			end[2] -= 64.0f;

			const vec3_t mins = { -8.0f, -8.0f, 0.0f };
			const vec3_t maxs = {  8.0f,  8.0f, 2.0f };

			trace_t trace;
			gi.trace(self->s.origin, mins, maxs, end, self, MASK_SOLID, &trace);

			if (trace.fraction == 1.0f && !trace.startsolid && !trace.allsolid)
				go_jump = true;
		}
	}

	if (go_jump)
		TBeastCheckJump(self);

	TBeastFakeTouch(self);
	self->next_post_think = level.time + 0.1f;
}

#pragma endregion

#pragma region ========================== Message handlers ==========================

// Decide which standing animations to use.
static void TBeastStandMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'tbeast_stand' in original logic.
{
	if (self->ai_mood == AI_MOOD_DELAY)
	{
		SetAnim(self, ANIM_DELAY);
		return;
	}

	if (self->enemy == NULL)
	{
		SetAnim(self, ANIM_STAND);
		return;
	}

	if (AI_IsClearlyVisible(self, self->enemy))
		return;

	vec3_t diff;
	VectorSubtract(self->enemy->s.origin, self->s.origin, diff);
	const float dist = VectorNormalize(diff);

	if (dist < 200.0f)
	{
		self->show_hostile = level.time + 1.0f; // Wake up other monsters.
		G_PostMessage(self, MSG_MELEE, PRI_DIRECTIVE, NULL);

		return;
	}

	if (irand(0, 3) == 0)
	{
		tbeast_growl(self);
		SetAnim(self, ANIM_STAND);

		return;
	}

	//mxd. Moved below previous 'if' check to avoid a slight chance to play growl sound twice in the same call.
	if (dist < 400.0f && irand(0, 3) == 0)
		tbeast_growl(self);

	if (self->monsterinfo.aiflags & AI_EATING)
		SetAnim(self, ANIM_EATING);
}

// Decide which walk animations to use.
static void TBeastWalkMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'tbeast_walk' in original logic.
{
	vec3_t target_origin;
	if (!MG_TryGetTargetOrigin(self, target_origin))
		return;

	if (self->enemy == NULL) // ?goal?
	{
		SetAnim(self, ANIM_WALK);
		return;
	}

	const float delta = anglemod(self->s.angles[YAW] - self->ideal_yaw);

	if (delta > 25.0f && delta <= 180.0f)
		SetAnim(self, ANIM_WALKRT);
	else if (delta > 180.0f && delta < 335.0f)
		SetAnim(self, ANIM_WALKLEFT);
	else
		SetAnim(self, ANIM_WALK);
}

// Decide which run animation to use.
static void TBeastRunMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'tbeast_run' in original logic.
{
	if (!M_ValidTarget(self, self->enemy))
		return;

	vec3_t target_origin;
	if (!MG_TryGetTargetOrigin(self, target_origin))
		return;

	if (!self->dmg)
	{
		self->dmg = true;
		SetAnim(self, ANIM_CHARGE);

		return;
	}

	if (AI_IsClearlyVisible(self, self->enemy) && TBeastCanCharge(self))
	{
		TBeastInitCharge(self);
		return;
	}

	const float delta = anglemod(self->s.angles[YAW] - self->ideal_yaw);

	if (delta > 45.0f && delta <= 180.0f)
		SetAnim(self, ANIM_WALKRT); // Turn right.
	else if (delta > 180.0f && delta < 315.0f)
		SetAnim(self, ANIM_WALKLEFT); // Turn left.
	else
		SetAnim(self, ANIM_WALK); // Run on.
}

// Decide which eating animations to use.
static void TBeastEatMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'tbeast_eat' in original logic.
{
	if (!FindTarget(self))
		SetAnim(self, irand(ANIM_EATING_TWITCH, ANIM_EATING));
}

// Decide which melee animations to use.
static void TBeastMeleeMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'tbeast_melee' in original logic.
{
	if (!M_ValidTarget(self, self->enemy))
	{
		G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
		return;
	}

	vec3_t forward;
	vec3_t up;
	AngleVectors(self->s.angles, forward, NULL, up);

	vec3_t melee_point;
	VectorMA(self->s.origin, self->maxs[2] * 0.5f + TB_UP_OFFSET, up, melee_point);
	VectorMA(melee_point, 150.0f + TB_FWD_OFFSET, forward, melee_point);

	vec3_t diff;
	VectorSubtract(self->enemy->s.origin, melee_point, diff);
	const float dist = VectorLength(diff);

	if ((dist > 250.0f || SKILL == SKILL_EASY) && self->monsterinfo.attack_finished > level.time)
	{
		// Too soon to attack again.
		if (flrand(0.0f, 1.0f) < 0.9f && SKILL == SKILL_EASY)
			SetAnim(self, ANIM_STAND);
		else
			SetAnim(self, ANIM_ROAR);

		return;
	}

	const float attack_range = dist - self->enemy->maxs[0]; //mxd

	// OK to attack.
	if (attack_range < 100.0f)
	{
		// Do bite attack.
		if (self->enemy->absmin[2] > melee_point[2] + 128.0f)
			SetAnim(self, ANIM_BITEUP2);
		else if (self->enemy->absmin[2] > melee_point[2])
			SetAnim(self, ANIM_BITEUP);
		else
			SetAnim(self, ANIM_BITELOW);

		self->monsterinfo.attack_finished = level.time + flrand(2.0f, 3.0f);
		return;
	}

	if (attack_range < self->melee_range && irand(0, 2) == 0)
		SetAnim(self, ANIM_WALKATK); //TODO: always ignored. Should return here?

	if (TBeastCanCharge(self)) //mxd. Use function.
		TBeastInitCharge(self);
	else
		SetAnim(self, ANIM_WALK);
}

// Try to start charge attack.
static void TBeastMissileMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'tbeast_start_charge' in original logic.
{
	MG_ChangeYaw(self);

	if (TBeastCanCharge(self)) //mxd. Use function.
		TBeastInitCharge(self);
	else
		SetAnim(self, ANIM_WALK);
}

static void TBeastPainMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'tbeast_pain' in original logic.
{
	if (self->health < 1000 || self->groundentity == NULL || self->pain_debounce_time > level.time)
		return;

	edict_t* temp_ent;
	qboolean force_pain;
	int damage;
	int	temp;
	G_ParseMsgParms(msg, "eeiii", &temp_ent, &temp_ent, &force_pain, &damage, &temp);

	if (damage >= irand(100, 200))
	{
		self->pain_debounce_time = level.time + 10.0f;
		gi.sound(self, CHAN_VOICE, sounds[irand(SND_PAIN1, SND_PAIN2)], 1.0f, ATTN_NORM, 0.0f);

		SetAnim(self, ANIM_STUN);
	}
}

static void TBeastDeathMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'tbeast_death' in original logic.
{
	self->msgHandler = DeadMsgHandler;

	if (self->dead_state == DEAD_DEAD) // Dead but still being hit.
		return;

	M_ShowLifeMeter(0, 0);

	// Regular death.
	gi.sound(self, CHAN_VOICE, sounds[SND_DIE], 1.0f, ATTN_NONE, 0.0f);
	self->dead_state = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	SetAnim(self, ANIM_DIE_NORM);
}

#pragma endregion

#pragma region ========================== Action functions ==========================

void tbeast_charge(edict_t* self, float force)
{
	if (!M_ValidTarget(self, self->enemy))
	{
		SetAnim(self, ANIM_WALK);
		return;
	}

	vec3_t forward;
	AngleVectors(self->s.angles, forward, NULL, NULL);

	vec3_t enemy_dir;
	VectorSubtract(self->enemy->s.origin, self->s.origin, enemy_dir);
	VectorNormalize(enemy_dir);

	if (DotProduct(forward, enemy_dir) < 0.75f) // Enemy not generally ahead.
		ai_charge(self, 0);

	MG_WalkMove(self, self->s.angles[YAW], force);

	if (self->groundentity != NULL)
	{
		const float vel_z = self->velocity[2];
		VectorScale(forward, force * 20.0f, self->velocity);
		self->velocity[2] = vel_z;
	}
}

void tbeast_stand_order(edict_t* self) //mxd. Named 'tbeast_standorder' in original logic.
{
	if (!TBeastCheckMood(self))
		G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
}

void tbeast_walk_order(edict_t* self) //mxd. Named 'tbeast_walkorder' in original logic.
{
	if (!TBeastCheckMood(self))
		G_PostMessage(self, MSG_WALK, PRI_DIRECTIVE, NULL);
}

void tbeast_footstep(edict_t* self)
{
	vec3_t forward;
	vec3_t right;
	vec3_t up;
	AngleVectors(self->s.angles, forward, right, up);

	vec3_t pos;
	const int leg_check_index = TBeastGetWalkFrame(self);

	if (leg_check_index > -1)
	{
		vec3_t foot_offset;

		// Setup leg checks - only if in these frames.
		if (leg_check_index < 6 || leg_check_index > 14)
			VectorCopy(left_foot_offset_for_frame_index[leg_check_index], foot_offset); // Left leg.
		else
			VectorCopy(right_foot_offset_for_frame_index[leg_check_index], foot_offset); // Right leg.

		VectorMA(self->s.origin, foot_offset[0] + TB_FWD_OFFSET, forward, pos);
		VectorMA(pos, foot_offset[1] + TB_RT_OFFSET, right, pos);
		VectorMA(pos, foot_offset[2] + TB_UP_OFFSET, up, pos);
	}
	else
	{
		VectorCopy(self->s.origin, pos);
		VectorMA(pos, self->maxs[0], forward, pos);
		pos[2] += self->mins[2];

		if (self->monsterinfo.currframeindex == FRAME_walk11 || self->monsterinfo.currframeindex == FRAME_wlkrt11 || self->monsterinfo.currframeindex == FRAME_wlklft11)
			VectorMA(pos, -32.0f, right, pos);
		else
			VectorMA(pos, 32.0f, right, pos);
	}

	vec3_t bottom;
	VectorCopy(pos, bottom);
	bottom[2] -= 128.0f;

	trace_t trace;
	gi.trace(pos, vec3_origin, vec3_origin, bottom, self, MASK_SOLID, &trace);

	if (trace.fraction < 1.0f)
		VectorCopy(trace.endpos, pos);

	pos[2] += 10.0f;

	gi.CreateEffect(NULL, FX_TB_EFFECTS, 0, pos, "bv", FX_TB_PUFF, vec3_origin);

	vec3_t fx_dir = { flrand(-20.0f, 20.0f), flrand(-20.0f, 20.0f), flrand(20.0f, 100.0f) }; //mxd
	gi.CreateEffect(NULL, FX_OGLE_HITPUFF, 0, pos, "v", fx_dir);
	gi.CreateEffect(&self->s, FX_QUAKE, 0, vec3_origin, "bbb", 3, 1, 2);

	gi.sound(self, CHAN_BODY, sounds[irand(SND_STEP1, SND_STEP2)], 1.0f, ATTN_NORM, 0.0f);
}

void tbeast_growl(edict_t* self)
{
	const int chance = irand(0, 200);

	if (chance < 10)
		gi.sound(self, CHAN_VOICE, sounds[SND_GROWL1], 1.0f, ATTN_NORM, 0.0f);
	else if (chance < 20)
		gi.sound(self, CHAN_VOICE, sounds[SND_GROWL2], 1.0f, ATTN_NORM, 0.0f);
	else if (chance < 30)
		gi.sound(self, CHAN_VOICE, sounds[SND_GROWL3], 1.0f, ATTN_NORM, 0.0f);
}

void tbeast_snort(edict_t* self)
{
	const int chance = irand(0, 20);

	if (chance > 1)
		return;

	gi.sound(self, CHAN_WEAPON, sounds[irand(SND_SNORT1, SND_SNORT2)], 1.0f, ATTN_NORM, 0.0f);

	// Make snort effect from nose.
	vec3_t forward;
	vec3_t right;
	AngleVectors(self->s.angles, forward, right, NULL);

	vec3_t spot;
	VectorCopy(self->s.origin, spot);
	spot[2] += 36.0f;

	vec3_t spot2;
	VectorMA(spot, 100.0f, forward, spot2);

	VectorMA(spot2, 64.0f, right, spot2); // More than we want to get a nice vec.

	vec3_t fx_dir;
	VectorSubtract(spot2, spot, fx_dir);
	VectorNormalize(fx_dir);

	VectorMA(spot2, -56.0f, right, spot2); // Back to +8.

	gi.CreateEffect(NULL, FX_FLAMETHROWER, CEF_FLAG6 | CEF_FLAG7, spot2, "df", fx_dir, 100.0f);

	Vec3ScaleAssign(-1.0f, right);

	VectorMA(spot2, 72.0f, right, spot2); // More than we want to get a nice vec.
	VectorSubtract(spot2, spot, fx_dir);
	VectorNormalize(fx_dir);

	VectorMA(spot2, -56.0f, right, spot2); // Back to +16.

	gi.CreateEffect(NULL, FX_FLAMETHROWER, CEF_FLAG6 | CEF_FLAG7, spot2, "df", fx_dir, 100.0f);
}

void tbeast_check_mood(edict_t* self) //mxd. Added action function version.
{
	TBeastCheckMood(self);
}

// Decide what to do after attacking.
void tbeast_pause(edict_t* self)
{
	if (self->enemy != NULL && self->enemy->classID != CID_TCHECKRIK && self->curAnimID == ANIM_STUN && self->pain_debounce_time > level.time + 7.0f && MG_IsAheadOf(self, self->enemy))
	{
		TBeastInitCharge(self);
		return;
	}

	if (TBeastCheckMood(self) || !M_ValidTarget(self, self->enemy))
		return;

	float dist;

	if (AI_IsClearlyVisible(self, self->enemy))
	{
		vec3_t diff;
		VectorSubtract(self->s.origin, self->enemy->s.origin, diff);
		dist = VectorLength(diff);
	}
	else
	{
		dist = FLT_MAX; //mxd. 999999 in original logic.
	}

	if (dist > 120.0f) // Far enough to run after.
		G_PostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
	else // Close enough to Attack or Hop.
		G_PostMessage(self, MSG_MELEE, PRI_DIRECTIVE, NULL);
}

void tbeast_bite(edict_t* self, float forward_offset, float right_offset, float up_offset) //mxd. Named 'tbeastbite' in original logic.
{
	//FIXME: do a checkenemy that checks oldenemy & posts messages.
	if (self->ai_mood == AI_MOOD_NAVIGATE || !M_ValidTarget(self, self->enemy))
		return;

	vec3_t forward;
	vec3_t right;
	vec3_t up;
	AngleVectors(self->s.angles, forward, right, up);

	vec3_t melee_point;
	VectorMA(self->s.origin, up_offset + TB_UP_OFFSET, up, melee_point);
	VectorMA(melee_point, forward_offset + TB_FWD_OFFSET, forward, melee_point);
	VectorMA(melee_point, right_offset, right, melee_point);

	// Give extra range.
	vec3_t bite_end_pos;
	VectorMA(melee_point, TBEAST_STD_MELEE_RNG, forward, bite_end_pos);

	// Let's do this the right way.
	trace_t trace;
	gi.trace(melee_point, vec3_origin, vec3_origin, bite_end_pos, self, MASK_SHOT, &trace);

	if (trace.fraction < 1.0f && !trace.startsolid && !trace.allsolid && trace.ent->takedamage != DAMAGE_NO) // A hit.
	{
		gi.sound(self, CHAN_WEAPON, sounds[SND_CHOMP], 1.0f, ATTN_NORM, 0.0f);

		vec3_t normal;
		VectorSubtract(self->enemy->s.origin, self->s.origin, normal);
		VectorNormalize(normal);

		const int damage = irand(TB_DMG_BITE_MIN, TB_DMG_BITE_MAX);
		T_Damage(self->enemy, self, self, forward, trace.endpos, normal, damage, damage / 2, DAMAGE_DISMEMBER, MOD_DIED);
	}
	else // A miss.
	{
		gi.sound(self, CHAN_WEAPON, sounds[SND_SNATCH], 1.0f, ATTN_NORM, 0.0f);
	}
}

void tbeast_dead(edict_t* self)
{
	self->movetype = PHYSICSTYPE_NONE;
	self->dead_state = DEAD_DEAD;
	self->think = NULL;
	self->nextthink = THINK_NEVER; //mxd. '0' in original logic. Changed for consistency sake.

	level.fighting_beast = false;
	gi.linkentity(self);
}

void tbeast_land(edict_t* self)
{
	gi.CreateEffect(&self->s, FX_QUAKE, 0, vec3_origin, "bbb", 7, 7, 7);

	vec3_t up = { flrand(-50.0f, 50.0f), flrand(-50.0f, 50.0f), flrand(50.0f, 300.0f) };

	for (int i = 0; i < 4; i++)
	{
		vec3_t pos;
		VectorCopy(self->s.origin, pos);
		pos[0] += flrand(-50.0f, 50.0f);
		pos[1] += flrand(-50.0f, 50.0f);
		pos[2] += self->mins[2];

		gi.CreateEffect(NULL, FX_OGLE_HITPUFF, 0, pos, "v", up); //TODO: randomize 'up' vector on each iteration?
	}

	gi.sound(self, CHAN_ITEM, sounds[SND_LAND], 1.0f, ATTN_NORM, 0.0f);

	edict_t* e = NULL;
	while ((e = FindInRadius(e, self->s.origin, 512.0f)) != NULL)
		if (e->client != NULL && e->client->playerinfo.lowerseq != ASEQ_KNOCKDOWN && e->health > 0 && e->groundentity != NULL)
			P_KnockDownPlayer(&e->client->playerinfo);
}

void tbeast_roar_knockdown(edict_t* self)
{
	if (irand(0, 2) > 0)
		return;

	edict_t* e = NULL;
	while ((e = FindInRadius(e, self->s.origin, 512.0f)) != NULL)
		if (e->client != NULL && e->client->playerinfo.lowerseq != ASEQ_KNOCKDOWN && MG_IsAheadOf(self, e) && e->health > 0 && e->groundentity != NULL)
			P_KnockDownPlayer(&e->client->playerinfo);
}

void tbeast_roar(edict_t* self)
{
	gi.sound(self, CHAN_VOICE, sounds[SND_ROAR2], 1.0f, ATTN_NONE, 0.0f);
}

void tbeast_roar_short(edict_t* self)
{
	if (self->delay == 0.0f) //TODO: never used? Initialized to 1.0 in SP_monster_trial_beast().
	{
		self->monsterinfo.currframeindex = FRAME_atkc6; //mxd. Use define.
		self->monsterinfo.nextframeindex = FRAME_atkc7; //mxd. Use define.
		self->s.frame = FRAME_charge1;
		self->delay = 1.0f;
	}
	else
	{
		gi.sound(self, CHAN_VOICE, sounds[SND_ROAR], 1.0f, ATTN_NONE, 0.0f);
	}
}

void tbeast_eat_order(edict_t* self) //mxd. Named 'tbeast_eatorder' in original logic.
{
	if (!TBeastCheckMood(self))
		G_PostMessage(self, MSG_EAT, PRI_DIRECTIVE, NULL);
}

void tbeast_apply_jump(edict_t* self)
{
	VectorCopy(self->movedir, self->velocity);
	VectorNormalize(self->movedir);
}

void tbeast_run(edict_t* self, float dist) //mxd. Named 'tbeast_run_think' in original logic.
{
	if (!M_ValidTarget(self, self->enemy))
	{
		G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
		return;
	}

	// See if I'm on ground.
	TBeastCheckBottom(self);

	if (self->monsterinfo.aiflags & AI_USING_BUOYS)
		MG_Pathfind(self, false);

	if (MG_MoveToGoal(self, dist))
		return;

	vec3_t forward;
	const vec3_t angles = { 0.0f, self->s.angles[YAW], 0.0f };
	AngleVectors(angles, forward, NULL, NULL);

	vec3_t end;
	VectorMA(self->s.origin, 128.0f, forward, end);

	vec3_t mins;
	VectorCopy(self->mins, mins);
	mins[2] += 54.0f; // His step height.

	trace_t trace;
	gi.trace(self->s.origin, mins, self->maxs, end, self, MASK_SOLID, &trace);

	if (trace.ent != NULL && (AI_IsMovable(trace.ent) || trace.ent->solid != SOLID_BSP))
		return;

	if (trace.fraction == 1.0f || (Vec3NotZero(trace.plane.normal) && trace.plane.normal[2] < GROUND_NORMAL)) // Nothing there || Not a slope can go up.
		TBeastCheckJump(self); // Enemy was ahead!
}

void tbeast_ready_catch(edict_t* self)
{
	if (self->targetEnt == NULL)
		return;

	const float enemy_zdist = (self->targetEnt->s.origin[2] + self->targetEnt->mins[2]) - self->s.origin[2];

	if (enemy_zdist <= self->maxs[2] + 128.0f && self->targetEnt->velocity[2] <= -60.0f)
		SetAnim(self, ANIM_CATCH);
	else
		SetAnim(self, ANIM_READY_CATCH);
}

//mxd. Similar to gorgon_throw_toy().
void tbeast_throw_toy(edict_t* self)
{
	if (self->targetEnt == NULL)
		return;

	self->targetEnt->flags &= ~FL_FLY;
	VectorSet(self->targetEnt->velocity, 0.0f, 0.0f, 500.0f);

	if (self->targetEnt->movetype > NUM_PHYSICSTYPES) //TODO: Eh? Should check for PHYSICSTYPE_FLY instead?..
		self->targetEnt->movetype = PHYSICSTYPE_STEP;

	VectorRandomCopy(vec3_origin, self->targetEnt->avelocity, 300.0f);

	if (Q_stricmp(self->targetEnt->classname, "player") != 0) //TODO: strange way to check for non-players. Should check self->targetEnt->client instead?..
		G_PostMessage(self->targetEnt, MSG_DEATH, PRI_DIRECTIVE, NULL);

	if (self->targetEnt->client != NULL)
		gi.sound(self->targetEnt, CHAN_VOICE, sounds[SND_CORVUS_DIE], 1.0f, ATTN_NORM, 0.0f);

	//TODO: play SND_THROW?
}

//mxd. Similar to gorgon_shake_toy().
void tbeast_shake_toy(edict_t* self, float forward_offset, float right_offset, float up_offset) //mxd. Named 'tbeast_toy_ofs' in original logic.
{
	if (self->enemy == NULL)
		return;

	vec3_t forward;
	vec3_t right;
	vec3_t up;
	AngleVectors(self->s.angles, forward, right, up);

	vec3_t enemy_offset;
	VectorMA(self->s.origin, forward_offset + TB_FWD_OFFSET - 32.0f, forward, enemy_offset);
	VectorMA(enemy_offset, right_offset, right, enemy_offset);
	VectorMA(enemy_offset, up_offset + TB_UP_OFFSET, up, self->targetEnt->s.origin);

	vec3_t blood_dir;
	VectorSubtract(self->targetEnt->s.origin, self->s.origin, blood_dir);

	vec3_t enemy_dir;
	VectorScale(blood_dir, -1.0f, enemy_dir);
	enemy_dir[2] /= 10.0f;
	vectoangles(enemy_dir, self->targetEnt->s.angles);

	switch (self->targetEnt->count) //TODO: never assigned. Assign in tbeast_check_snatch()?
	{
		case 1:
			self->targetEnt->s.angles[PITCH] = anglemod(self->targetEnt->s.angles[PITCH] + 90.0f); // Can't do roll?
			break;

		case 2:
			self->targetEnt->s.angles[PITCH] = anglemod(self->targetEnt->s.angles[PITCH] - 90.0f); // Can't do roll?
			break;

		case 3:
			self->targetEnt->s.angles[ROLL] = anglemod(self->targetEnt->s.angles[ROLL] + 90.0f); // Can't do roll?
			break;

		case 4:
			self->targetEnt->s.angles[ROLL] = anglemod(self->targetEnt->s.angles[ROLL] - 90.0f); // Can't do roll?
			break;

		default:
			break;
	}

	VectorClear(self->targetEnt->velocity);
	VectorClear(self->targetEnt->avelocity);

	if (flrand(0.0f, 1.0f) < 0.5f)
	{
		const int fx_flags = ((self->targetEnt->materialtype == MAT_INSECT) ? CEF_FLAG8 : 0); //mxd
		gi.CreateEffect(&self->targetEnt->s, FX_BLOOD, fx_flags, self->targetEnt->s.origin, "ub", blood_dir, 200);
	}
}

//mxd. Similar to gorgon_check_snatch().
void tbeast_check_snatch(edict_t* self, float forward_offset, float right_offset, float up_offset)
{
	if (self->enemy == NULL)
		return;

	vec3_t forward;
	vec3_t right;
	vec3_t up;
	AngleVectors(self->s.angles, forward, right, up);

	vec3_t start_pos;
	VectorMA(self->s.origin, forward_offset + TB_FWD_OFFSET, forward, start_pos);
	VectorMA(start_pos, right_offset, right, start_pos);
	VectorMA(start_pos, up_offset + TB_UP_OFFSET, up, start_pos);

	vec3_t end_pos;
	VectorSubtract(self->enemy->s.origin, start_pos, end_pos);

	edict_t* found = NULL;
	const float enemy_dist = VectorLength(end_pos);

	if (enemy_dist > MAX_CHOMP_DIST || irand(0, 50) > self->enemy->health) //mxd. Original logic uses flrand() here.
	{
		// If missed or health is low, just chomp it now.
		while ((found = FindInRadius(found, start_pos, MAX_CHOMP_DIST)) != NULL)
		{
			if (found->takedamage != DAMAGE_NO && AI_IsMovable(found))
			{
				if (found->health <= 0)
					T_Damage(found, self, self, end_pos, found->s.origin, end_pos, 2000, 300, DAMAGE_DISMEMBER, MOD_DIED);
				else
					break;
			}
		}

		if (found == NULL)
		{
			self->msgHandler = DefaultMsgHandler;
			return;
		}
	}
	else
	{
		found = self->enemy;
	}

	self->msgHandler = DeadMsgHandler;

	if (up_offset == TB_HIBITE_U + 128.0f)
		SetAnim(self, ANIM_BITEUP2_SFIN);
	else if (up_offset == TB_HIBITE_U)
		SetAnim(self, ANIM_BITEUP_SFIN);
	else
		SetAnim(self, ANIM_BITELOW_SFIN);

	self->targetEnt = found;
	self->targetEnt->flags |= FL_FLY;
	self->targetEnt->movetype = PHYSICSTYPE_FLY;

	if (found->client != NULL)
	{
		found->nextthink = level.time + 10.0f; // Stuck for 10 seconds.

		if (found->health > 0 && found->client->playerinfo.lowerseq != ASEQ_KNOCKDOWN)
			P_KnockDownPlayer(&found->client->playerinfo);

		gi.sound(found, CHAN_VOICE, sounds[irand(SND_CORVUS_SCREAM1, SND_CORVUS_SCREAM3)], 1.0f, ATTN_NORM, 0.0f);
	}
	else
	{
		found->monsterinfo.aiflags |= AI_DONT_THINK;
		//TODO: also set found->count (done in gorgon_check_snatch())?
	}

	VectorClear(found->velocity);
	VectorClear(found->avelocity);
}

void tbeast_snatch_go(edict_t* self) //mxd. Named 'tbeast_go_snatch' in original logic.
{
	SetAnim(self, ANIM_SNATCH);
}

//mxd. Similar to gorgon_gore_toy().
void tbeast_gore_toy(edict_t* self, float jump_height)
{
	const qboolean last_frame = (jump_height == -1.0f); //mxd

	if (!last_frame)
	{
		self->velocity[2] += jump_height;

		if (self->groundentity != NULL)
		{
			vec3_t forward;
			AngleVectors(self->s.angles, forward, NULL, NULL);
			VectorMA(self->velocity, -100.0f, forward, self->velocity);
		}
	}
	else
	{
		self->tbeast_grabbed_toy = false;
	}

	if (self->targetEnt == NULL || self->targetEnt->health < 0 || self->tbeast_grabbed_toy)
		return;

	const float enemy_zdist = self->targetEnt->s.origin[2] - self->s.origin[2];

	if (last_frame || enemy_zdist <= self->maxs[2] + 128.0f)
	{
		//FIXME: waits grabs it too low, waits too long.
		self->tbeast_toy_materialtype = self->targetEnt->materialtype;

		gi.sound(self, CHAN_WEAPON, sounds[SND_SNATCH], 1.0f, ATTN_NORM, 0.0f);

		if (!last_frame)
			self->tbeast_grabbed_toy = true;

		vec3_t dir;
		VectorCopy(self->velocity, dir);
		VectorNormalize(dir);

		const int num_chunks = min(15, self->targetEnt->health / 4);
		SprayDebris(self->targetEnt, self->targetEnt->s.origin, num_chunks, (float)self->targetEnt->health * 4.0f); //self->enemy is thingtype wood?!

		if (Q_stricmp(self->targetEnt->classname, "player") != 0) //mxd. stricmp -> Q_stricmp //TODO: check for targetEnt->client instead?
		{
			gi.sound(self->targetEnt, CHAN_WEAPON, sounds[SND_CATCH], 1.0f, ATTN_NORM, 0.0f);
			BecomeDebris(self->targetEnt);
		}
		else
		{
			self->targetEnt->nextthink = level.time;
			T_Damage(self->targetEnt, self, self, self->velocity, self->targetEnt->s.origin, dir, 2000, 300, DAMAGE_DISMEMBER | DAMAGE_NO_PROTECTION, MOD_DIED);
		}

		if (self->enemy == self->targetEnt)
			self->enemy = NULL;

		self->targetEnt = NULL;
	}
}

void tbeast_anger_sound(edict_t* self)
{
	const byte chance = (byte)irand(0, 100);

	if (chance < 10)
		gi.sound(self, CHAN_WEAPON, sounds[SND_SNORT1], 1.0f, ATTN_NORM, 0.0f);
	else if (chance < 20)
		gi.sound(self, CHAN_WEAPON, sounds[SND_SNORT2], 1.0f, ATTN_NORM, 0.0f);
	else if (chance < 30)
		gi.sound(self, CHAN_ITEM, sounds[SND_TEAR1], 1.0f, ATTN_NORM, 0.0f);
	else if (chance < 40)
		gi.sound(self, CHAN_ITEM, sounds[SND_TEAR2], 1.0f, ATTN_NORM, 0.0f);
	else if (chance < 50)
		gi.sound(self, CHAN_WEAPON, sounds[SND_CHOMP], 1.0f, ATTN_NORM, 0.0f);
	else if (chance < 60)
		tbeast_growl(self);

	if (self->targetEnt != NULL)
	{
		SprayDebris(self->targetEnt, self->targetEnt->s.origin, irand(1, 3), 100.0f);

		if (self->targetEnt->client == NULL)
		{
			G_PostMessage(self->targetEnt, MSG_DISMEMBER, PRI_DIRECTIVE, "ii", self->targetEnt->health / 2, irand(1, 13)); // Do I need last three if not sending them?
			G_PostMessage(self->targetEnt, MSG_PAIN, PRI_DIRECTIVE, "eeiii", self, self, true, 200, 0);
		}
	}
}

void tbeast_gibs(edict_t* self)
{
	//FIXME: keep making gibs.
	if (self->tbeast_toy_materialtype == 0)
		return;

	vec3_t forward;
	AngleVectors(self->s.angles, forward, NULL, NULL);

	vec3_t spot;
	VectorMA(self->s.origin, 56.0f, forward, spot);
	spot[2] -= 8.0f;

	const int fx_flags = (self->tbeast_toy_materialtype == MAT_INSECT ? CEF_FLAG7 | CEF_FLAG8 : 0); // Use male insect skin on chunks.
	gi.CreateEffect(NULL, FX_FLESH_DEBRIS, fx_flags, spot, "bdb", (byte)irand(3, 7), self->mins, 16);

	tbeast_anger_sound(self);
}

void tbeast_done_gore(edict_t* self)
{
	self->msgHandler = DefaultMsgHandler;
	self->tbeast_grabbed_toy = false;
	M_ValidTarget(self, self->enemy);

	self->monsterinfo.aiflags |= AI_EATING;
	SetAnim(self, ANIM_EATDOWN);
}

void tbeast_inair(edict_t* self)
{
	SetAnim(self, ANIM_INAIR);
}

void tbeast_check_landed(edict_t* self)
{
	if (TBeastCheckBottom(self))
		SetAnim(self, ANIM_LAND);
}

void tbeast_ginair(edict_t* self)
{
	SetAnim(self, ANIM_GINAIR);
}

void tbeast_gcheck_landed(edict_t* self)
{
	if (TBeastCheckBottom(self))
		SetAnim(self, ANIM_GLAND);
}

void tbeast_leap(edict_t* self, float forward_offset, float right_offset, float up_offset)
{
	if (self->groundentity == NULL && !TBeastCheckBottom(self))
		return;

	if (self->s.frame == FRAME_jumpb7)
		TBeastChomp(self, 36.0f, 0.0f, 232.0f);

	vec3_t forward;
	vec3_t right;
	vec3_t up;
	const vec3_t angles = { 0.0f, self->s.angles[YAW], 0.0f };
	AngleVectors(angles, forward, right, up);

	VectorScale(forward, forward_offset, self->velocity);
	VectorMA(self->velocity, right_offset, right, self->velocity);
	VectorMA(self->velocity, up_offset, up, self->velocity);
}

#pragma endregion

void TBeastStaticsInit(void)
{
	static ClassResourceInfo_t res_info; //mxd. Made local static.

	classStatics[CID_TBEAST].msgReceivers[MSG_STAND] = TBeastStandMsgHandler;
	classStatics[CID_TBEAST].msgReceivers[MSG_WALK] = TBeastWalkMsgHandler;
	classStatics[CID_TBEAST].msgReceivers[MSG_RUN] = TBeastRunMsgHandler;
	classStatics[CID_TBEAST].msgReceivers[MSG_EAT] = TBeastEatMsgHandler;
	classStatics[CID_TBEAST].msgReceivers[MSG_MELEE] = TBeastMeleeMsgHandler;
	classStatics[CID_TBEAST].msgReceivers[MSG_MISSILE] = TBeastMissileMsgHandler;
	classStatics[CID_TBEAST].msgReceivers[MSG_WATCH] = TBeastWalkMsgHandler;
	classStatics[CID_TBEAST].msgReceivers[MSG_PAIN] = TBeastPainMsgHandler;
	classStatics[CID_TBEAST].msgReceivers[MSG_DEATH] = TBeastDeathMsgHandler;

	res_info.numAnims = NUM_ANIMS;
	res_info.animations = animations;
	res_info.modelIndex = gi.modelindex("models/monsters/beast/tris.fm");

	sounds[SND_ROAR] = gi.soundindex("monsters/tbeast/roar.wav");
	sounds[SND_ROAR2] = gi.soundindex("monsters/tbeast/roar2.wav");
	sounds[SND_SNORT1] = gi.soundindex("monsters/tbeast/snort1.wav");
	sounds[SND_SNORT2] = gi.soundindex("monsters/tbeast/snort2.wav");

	sounds[SND_STEP1] = gi.soundindex("monsters/tbeast/step1.wav");
	sounds[SND_STEP2] = gi.soundindex("monsters/tbeast/step2.wav");
	sounds[SND_LAND] = gi.soundindex("monsters/tbeast/land.wav");

	sounds[SND_GROWL1] = gi.soundindex("monsters/tbeast/growl1.wav");
	sounds[SND_GROWL2] = gi.soundindex("monsters/tbeast/growl2.wav");
	sounds[SND_GROWL3] = gi.soundindex("monsters/tbeast/growl3.wav");

	sounds[SND_SLAM] = gi.soundindex("monsters/tbeast/slam.wav");
	sounds[SND_SNATCH] = gi.soundindex("monsters/tbeast/snatch.wav");
	sounds[SND_CHOMP] = gi.soundindex("monsters/tbeast/chomp.wav");
	sounds[SND_TEAR1] = gi.soundindex("monsters/tbeast/tear1.wav");
	sounds[SND_TEAR2] = gi.soundindex("monsters/tbeast/tear2.wav");
	//sounds[SND_THROW] = gi.soundindex("monsters/tbeast/throw.wav"); //mxd. Unused.
	sounds[SND_CATCH] = gi.soundindex("monsters/tbeast/catch.wav");

	sounds[SND_PAIN1] = gi.soundindex("monsters/tbeast/pain1.wav");
	sounds[SND_PAIN2] = gi.soundindex("monsters/tbeast/pain2.wav");
	sounds[SND_DIE] = gi.soundindex("monsters/tbeast/die.wav");

	sounds[SND_CORVUS_SCREAM1] = gi.soundindex("corvus/bdeath1.wav");
	sounds[SND_CORVUS_SCREAM2] = gi.soundindex("corvus/bdeath2.wav");
	sounds[SND_CORVUS_SCREAM3] = gi.soundindex("corvus/bdeath3.wav");
	sounds[SND_CORVUS_DIE] = gi.soundindex("player/falldeath1.wav");

	res_info.numSounds = NUM_SOUNDS;
	res_info.sounds = sounds;

	classStatics[CID_TBEAST].resInfo = &res_info;
}

// QUAKED monster_trial_beast (1 .5 0) (-100 -100 -36) (100 100 150) ?
// The Trial Beastie
// wakeup_target			- Monsters will fire this target the first time it wakes up (only once).
// pain_target				- Monsters will fire this target the first time it gets hurt (only once).
// mintel					- Monster intelligence - this basically tells a monster how many buoys away an enemy has to be for it to give up (default 100).
// melee_range				- How close the player has to be for the monster to go into melee. If this is zero, the monster will never melee.
//							  If it is negative, the monster will try to keep this distance from the player.
//							  If the monster has a backup, he'll use it if too close, otherwise, a negative value here means the monster will just stop
//							  running at the player at this distance (default 400 (bite)).
//							 Examples:
//								melee_range = 60 - monster will start swinging it player is closer than 60.
//								melee_range = 0 - monster will never do a melee attack.
//								melee_range = -100 - monster will never do a melee attack and will back away (if it has that ability) when player gets too close.
// missile_range			- Maximum distance the player can be from the monster to be allowed to use it's ranged attack (default 1500 (charge)).
// min_missile_range		- Minimum distance the player can be from the monster to be allowed to use it's ranged attack (default 100).
// bypass_missile_chance	- Chance that a monster will NOT fire it's ranged attack, even when it has a clear shot. This, in effect, will make the monster
//							  come in more often than hang back and fire. A percentage (0 = always fire/never close in, 100 = never fire/always close in) - must be whole number (default 77).
// jump_chance				- Every time the monster has the opportunity to jump, what is the chance (out of 100) that he will... (100 = jump every time) - must be whole number (default 100).
// wakeup_distance			- How far (max) the player can be away from the monster before it wakes up. This means that if the monster can see the player,
//							  at what distance should the monster actually notice him and go for him (default 3000).
// NOTE: A value of zero will result in defaults, if you actually want zero as the value, use -1.
void SP_monster_trial_beast(edict_t* self)
{
	// Generic Monster Initialization.
	if (!M_WalkmonsterStart(self)) // Failed initialization.
		return;

	self->msgHandler = DefaultMsgHandler;
	self->monsterinfo.aiflags |= (AI_BRUTAL | AI_AGRESSIVE | AI_SHOVE);
	self->monsterinfo.otherenemyname = "monster_tcheckrik_male";

	self->health = TB_HEALTH * (SKILL + 1) / 3;

	self->mass = TB_MASS;
	self->yaw_speed = 10.0f;
	self->isBlocked = TBeastBlocked;
	self->bounced = TBeastBlocked;

	self->movetype = PHYSICSTYPE_STEP;
	VectorClear(self->knockbackvel);

	// Problem: staff won't work on him!
	self->solid = SOLID_TRIGGER;
	self->materialtype = MAT_FLESH;

	VectorSet(self->mins, -100.0f, -100.0f, -36.0f);
	VectorSet(self->maxs, 100.0f, 100.0f, 150.0f);

	self->viewheight = 104 + TB_UP_OFFSET;
	self->s.modelindex = (byte)classStatics[CID_TBEAST].resInfo->modelIndex;

	// Big guy can be stood on top of perhaps?
	if (self->wakeup_distance == 0.0f)
		self->wakeup_distance = 3000.0f;

	MG_InitMoods(self);
	G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);

	self->dmg = false; // Not in charge mode initially.
	self->svflags |= (SVF_BOSS | SVF_NO_AUTOTARGET);

	if (irand(0, 1) == 0)
		self->ai_mood_flags |= AI_MOOD_FLAG_PREDICT;

	self->monsterinfo.aiflags |= AI_NIGHTVISION;

	self->touch = TBeastTouch;
	self->post_think = TBeastPostThink;
	self->next_post_think = level.time + 0.1f;
	self->elasticity = ELASTICITY_SLIDE;
	self->tbeast_grabbed_toy = false;
	self->clipmask = CONTENTS_SOLID;
	self->solid = SOLID_TRIGGER; // WHY IS HE BEING PUSHED BY BSP ENTITIES NOW?!
	self->tbeast_pillars_destroyed = 0; // Pillar init. //BUGFIX: mxd. Original logic stores this in 'red_rain_count', which is NOT a saveable field.
	self->use = TBeastUse;
	self->delay = 1.0f;

	self->max_health = self->health;
	self->tbeast_healthbar_buildup = 0; // Initial healthbar buildup progress.
	self->tbeast_toy_materialtype = 0; // Initialize material id.

	level.fighting_beast = true; // Sorry, only one beast per level
}