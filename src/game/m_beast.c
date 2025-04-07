//
// m_beast.c
//
// Copyright 1998 Raven Software
//

#include <float.h>
#include "m_beast.h"
#include "m_beast_local.h" //mxd
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
static ClassResourceInfo_t res_info; //mxd. Named 'resInfo' in original logic.

#pragma endregion

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

// Assigned to 'isBlocked' and 'bounce' callbacks.
static void TBeastBlocked(edict_t* self, trace_t* trace) //mxd. Named 'tbeast_blocked' in original logic.
{
	// fake_touch does all the actual damage, this is just a check for the charge stuff.
	if (self->curAnimID == ANIM_CHARGE || (self->curAnimID == ANIM_QUICK_CHARGE && self->s.frame >= FRAME_charge1 && self->s.frame <= FRAME_charge10))
	{
		qboolean stop = false;
		const qboolean hit_slope = (trace->ent == world && !Vec3IsZero(trace->plane.normal) && trace->plane.normal[2] > GROUND_NORMAL); //mxd. Use define.

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
				self->red_rain_count++;

				if (self->red_rain_count >= 2) // Got both pillars, now die.
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
			self->sounds++;

			if (self->sounds != 2 && irand(0, 1) == 1)
				SetAnim(self, ANIM_STUN);
		}
	}
}

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

// Decide which eating animations to use.
static void TBeastEatMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'tbeast_eat' in original logic.
{
	if (!FindTarget(self))
		SetAnim(self, irand(ANIM_EATING_TWITCH, ANIM_EATING));
}

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
		QPostMessage(self, MSG_MELEE, PRI_DIRECTIVE, NULL);

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

// Decide which melee animations to use.
static void TBeastMeleeMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'tbeast_melee' in original logic.
{
	if (!M_ValidTarget(self, self->enemy))
	{
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
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

static void TBeastPainMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'tbeast_pain' in original logic.
{
	if (self->health < 1000 || self->groundentity == NULL || self->pain_debounce_time > level.time)
		return;

	edict_t* temp_ent;
	qboolean force_pain;
	int damage;
	int	temp;
	ParseMsgParms(msg, "eeiii", &temp_ent, &temp_ent, &force_pain, &damage, &temp);

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

	if (self->deadflag == DEAD_DEAD) // Dead but still being hit.
		return;

	M_ShowLifeMeter(self, 0, 0);

	// Regular death.
	gi.sound(self, CHAN_VOICE, sounds[SND_DIE], 1.0f, ATTN_NONE, 0.0f);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	SetAnim(self, ANIM_DIE_NORM);
}

static void TBeastDie_UseCallback(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'tbeast_go_die' in original logic. //TODO: group with tbeast_go_charge?
{
	M_ShowLifeMeter(self, 0, 0);

	gi.sound(self, CHAN_VOICE, sounds[SND_DIE], 1.0f, ATTN_NONE, 0.0f);

	self->msgHandler = DeadMsgHandler;
	self->deadflag = DEAD_DEAD;
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

void tbeast_stand_order(edict_t* self) //mxd. Named 'tbeast_standorder' in original logic.
{
	if (!TBeastCheckMood(self))
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
}

void tbeast_walk_order(edict_t* self) //mxd. Named 'tbeast_walkorder' in original logic.
{
	if (!TBeastCheckMood(self))
		QPostMessage(self, MSG_WALK, PRI_DIRECTIVE, NULL);
}

void tbeast_footstep(edict_t* self)
{
	vec3_t forward;
	vec3_t right;
	vec3_t up;
	AngleVectors(self->s.angles, forward, right, up);

	vec3_t pos;
	const int leg_check_index = tbeast_inwalkframes(self);

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
			QPostMessage(self, msg_id, PRI_DIRECTIVE, NULL);
		} break;

		case AI_MOOD_PURSUE:
			self->wait = 0.0f;
			QPostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_NAVIGATE:
			SetAnim(self, ANIM_WALK);
			break;

		case AI_MOOD_STAND:
		{
			const int msg_id = ((self->monsterinfo.aiflags & AI_EATING) ? MSG_EAT : MSG_STAND); //mxd
			QPostMessage(self, msg_id, PRI_DIRECTIVE, NULL);
		} break;

		case AI_MOOD_DELAY:
			QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_EAT:
			QPostMessage(self, MSG_EAT, PRI_DIRECTIVE, NULL);
			break;
	}

	return true;
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
		QPostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
	else // Close enough to Attack or Hop.
		QPostMessage(self, MSG_MELEE, PRI_DIRECTIVE, NULL);
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
	self->deadState = DEAD_DEAD;
	self->think = NULL;
	self->nextthink = 0.0f;

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
	if (self->delay == 0.0f)
	{
		self->monsterinfo.currframeindex = 25;
		self->monsterinfo.nextframeindex = 26;
		self->s.frame = FRAME_charge1;
		self->delay = 1.0f;
	}
	else
	{
		gi.sound(self, CHAN_VOICE, sounds[SND_ROAR], 1.0f, ATTN_NONE, 0.0f);
	}
}

void tbeast_eatorder (edict_t *self)
{
	if(TBeastCheckMood(self))
		return;

	QPostMessage(self, MSG_EAT, PRI_DIRECTIVE, NULL);
}

void tbeast_apply_jump (edict_t *self) //TODO: unused
{
//	gi.dprintf("Jump from TB_CheckJump\n");
//	self->gravity = TB_JUMP_GRAV;
	VectorCopy(self->movedir, self->velocity);
	VectorNormalize(self->movedir);
}


qboolean CheckMoveFoot (edict_t *self, edict_t *foot, vec3_t dest) //TODO: unused
{
	vec3_t	dir;
	trace_t	trace;
	
	VectorSubtract(dest, foot->s.origin, dir);
	VectorNormalize(dir);

	gi.trace(foot->s.origin, foot->mins, foot->maxs, dest, foot, MASK_MONSTERSOLID,&trace);

	if(trace.ent)
	{
		if(trace.ent->takedamage)
		{
			T_Damage(trace.ent, self, self, dir, trace.endpos, dir, 1000, 250, DAMAGE_DISMEMBER,MOD_DIED);
			VectorCopy(foot->s.origin, foot->last_org);
			VectorCopy(dest, foot->s.origin);
//			VectorCopy(trace.endpos, foot->s.origin);
			return true;
		}
//		else
//			return false;
	}

//	if(trace.allsolid||trace.startsolid)
//		return false;

//	if(trace.fraction<0.9)
//		return false;

	VectorCopy(foot->s.origin, foot->last_org);
	VectorCopy(dest, foot->s.origin);
//	VectorCopy(trace.endpos, foot->s.origin);

	return true;
}

qboolean TB_CheckBottom (edict_t *self)
{
	vec3_t	end;
	trace_t	trace;
	vec3_t other_top, down, up;

	VectorCopy(self->s.origin, end);
	end[2] -= 1;
	gi.trace(self->s.origin, self->mins, self->maxs, end, self, MASK_ALL,&trace);

	if(trace.ent && stricmp(trace.ent->classname, "worldspawn"))
	{
		if(trace.ent->takedamage)
		{
			VectorCopy(trace.ent->s.origin, other_top);
			other_top[2] += trace.ent->maxs[2];
			VectorSubtract(trace.ent->s.origin, self->s.origin, down);
			VectorScale(down, -1, up);
//			gi.dprintf("CheckBottom damaging %s\n", trace.ent->classname);
//			T_Damage(trace.ent, self, self, down, other_top, up, 1000, 0, DAMAGE_DISMEMBER);
		}
	}

	if(trace.fraction < 1.0 || trace.startsolid || trace.allsolid)
	{
		/*if(&trace.plane)
		{
			if(!Vec3IsZero(trace.plane.normal))
			{
				if(trace.plane.normal[2]>=0.5&&trace.plane.normal[2]<=1)//not a slope can go up
				{//raise him up if on flat ground, lower is on slope - to keep feet on ground;
					self->mins[2] = (1 - trace.plane.normal[2]) * 72 - 8 + TB_UP_OFFSET;
				}
			}
		}*/
		self->groundentity = trace.ent;
		return true;
	}
	return false;
}

qboolean TB_CheckJump (edict_t *self)//, edict_t *other)
{
	vec3_t	forward, start, end, start2, end2, mins, maxs;
	trace_t	trace;
	float	z_diff;
	qboolean	skiplow = false;

	if(self->enemy)
	{
		if(!MG_IsAheadOf(self, self->enemy))
			return false;

		if(vhlen(self->enemy->s.origin, self->s.origin)<200)
		{
			z_diff = self->s.origin[2] + TB_HIBITE_U + TB_UP_OFFSET - self->enemy->s.origin[2];
			if(z_diff < -128)
			{
				SetAnim(self, ANIM_BITEUP2);
				return true;
			}
			else if(Q_fabs(z_diff)<=32)
			{
				SetAnim(self, ANIM_BITEUP);
				return true;
			}
			else if(z_diff < -32 && z_diff > -200)
				skiplow = true;
			else if(z_diff > 40 && z_diff < -24)
			{
				SetAnim(self, ANIM_BITELOW);
				return true;
			}
		}

		if(self->enemy->s.origin[2] < self->s.origin[2])
			return false;
	}

	if(self->monsterinfo.jump_time > level.time)
		return false;

	VectorCopy(self->s.origin, start);
	VectorCopy(start, end);

//try a jump of 186
	end[2] += self->size[2];
	
	if(!skiplow)
	{
		gi.trace(start, self->mins, self->maxs, end, self, MASK_SOLID,&trace);
		
		if(trace.fraction == 1.0 && !trace.startsolid && !trace.allsolid)
		{
			AngleVectors(self->s.angles, forward, NULL, NULL);
			
			VectorCopy(end, start2);
			VectorMA(end, 64, forward, end2);
			VectorScale(self->maxs, 0.5, maxs);
			VectorCopy(self->mins, mins);
			mins[0]*=0.5;
			mins[1]*=0.5;
			gi.trace(start2, self->mins, self->maxs, end2, self, MASK_SOLID,&trace);
			if(trace.fraction == 1.0 && !trace.startsolid && !trace.allsolid)
			{
//				gi.dprintf("Beast blocked low jump!\n");
				VectorScale(forward, 250, self->movedir);
				self->movedir[2] = 400;
				self->monsterinfo.jump_time = level.time + 7;
				SetAnim(self, ANIM_FJUMP);
				return true;
			}
		}
	}

//try a jump of 372
	end[2] += self->size[2];
	
	gi.trace(start, self->mins, self->maxs, end, self, MASK_SOLID,&trace);
	
	if(trace.fraction == 1.0 && !trace.startsolid && !trace.allsolid)
	{
		AngleVectors(self->s.angles, forward, NULL, NULL);
		
		VectorCopy(end, start2);
		VectorMA(end, 64, forward, end2);
		VectorScale(self->maxs, 0.5, maxs);
		VectorCopy(self->mins, mins);
		mins[0]*=0.5;
		mins[1]*=0.5;
		gi.trace(start2, self->mins, self->maxs, end2, self, MASK_SOLID,&trace);
		if(trace.fraction == 1.0 && !trace.startsolid && !trace.allsolid)
		{
//			gi.dprintf("Beast blocked high jump!\n");
			VectorScale(forward, 300, self->movedir);
			self->movedir[2] = 600;
			self->monsterinfo.jump_time = level.time + 7;
			SetAnim(self, ANIM_FJUMP);
			return true;
		}
	}
	return false;
}

void tbeast_run_think (edict_t *self, float dist)
{
	vec3_t	angles, forward, start, end, mins;
	trace_t	trace;
/*
	matrix3_t	RotationMatrix;
	vec3_t		lfootoffset, rfootoffset, newlfootpos, newrfootpos;
	vec3_t		save_lf_org, save_rf_org, save_my_org;
	float		save_yaw;*/

	if(!M_ValidTarget(self, self->enemy))
	{
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
		return;
	}

//see if I'm on ground
	TB_CheckBottom(self);

	if(self->monsterinfo.aiflags &AI_USING_BUOYS)
		MG_Pathfind(self, false);

	if(!MG_MoveToGoal (self, dist))
	{
		VectorSet(angles, 0, self->s.angles[YAW], 0);
		AngleVectors(angles, forward, NULL, NULL);

		VectorCopy(self->s.origin, start);
		VectorMA(start, 128, forward, end);
		VectorCopy(self->mins, mins);
		mins[2]+=54;//his step height
		gi.trace(start, mins, self->maxs, end, self, MASK_SOLID,&trace);
		if(trace.ent)
		{
			if(AI_IsMovable(trace.ent) || trace.ent->solid!=SOLID_BSP)
			{
				return;
			}
		}
		if(trace.fraction == 1.0 || //nothing there - ledge
			(&trace.plane && !Vec3IsZero(trace.plane.normal) && trace.plane.normal[2]<0.7))//not a slope can go up
		{
#ifdef _DEVEL
			if(TB_CheckJump(self))
				gi.dprintf("Enemy was ahead!\n");
#else
			TB_CheckJump(self);
#endif
		}
	}
}
/*
========================================
TBEAST PICK UP AND GORE SOMETHING
========================================
*/
void tbeast_ready_catch (edict_t *self)
{
	float enemy_zdist, ok_zdist;

	if(!self->targetEnt)
		return;
	
	ok_zdist = 128;
	if(ok_zdist<48)
		ok_zdist = 48;

	enemy_zdist = (self->targetEnt->s.origin[2] + self->targetEnt->mins[2]) - self->s.origin[2];
	if(enemy_zdist<=self->maxs[2] + ok_zdist && self->targetEnt->velocity[2]<=-60)
		SetAnim(self,ANIM_CATCH);
	else
		SetAnim(self,ANIM_READY_CATCH);
}

void tbeast_throw_toy(edict_t *self)
{
	if(!self->targetEnt)
		return;

	self->targetEnt->flags &= ~FL_FLY;
	self->targetEnt->velocity[0] = self->targetEnt->velocity[1] = 0;
	self->targetEnt->velocity[2] = 500;
	if(self->targetEnt->movetype>NUM_PHYSICSTYPES)
		self->targetEnt->movetype = PHYSICSTYPE_STEP;
	VectorRandomCopy(vec3_origin,self->targetEnt->avelocity,300);
	
	if(stricmp(self->targetEnt->classname,"player"))
		QPostMessage(self->targetEnt, MSG_DEATH, PRI_DIRECTIVE, NULL);

	if(self->targetEnt->client)
		gi.sound(self->targetEnt, CHAN_VOICE, sounds[SND_CORVUS_DIE], 1, ATTN_NORM, 0);
}

void tbeast_toy_ofs(edict_t *self, float ofsf, float ofsr, float ofsu)
{
	vec3_t enemy_ofs, forward, right, up, blooddir, enemy_face;

	if(!self->enemy)
		return;

	AngleVectors(self->s.angles, forward, right, up);
	VectorMA(self->s.origin, ofsf + TB_FWD_OFFSET - 32, forward, enemy_ofs);
	VectorMA(enemy_ofs, ofsr, right, enemy_ofs);
	VectorMA(enemy_ofs, ofsu + TB_UP_OFFSET, up, self->targetEnt->s.origin);
	VectorSubtract(self->targetEnt->s.origin, self->s.origin, blooddir);
	
	VectorScale(blooddir, -1, enemy_face);
	enemy_face[2]/=10;
	vectoangles(enemy_face, self->targetEnt->s.angles);

	switch(self->targetEnt->count)
	{
		case 1:
			self->targetEnt->s.angles[PITCH]=anglemod(self->targetEnt->s.angles[PITCH]+90);//can't do roll?
			break;
		case 2:
			self->targetEnt->s.angles[PITCH]=anglemod(self->targetEnt->s.angles[PITCH]-90);//can't do roll?
			break;
		case 3:
			self->targetEnt->s.angles[ROLL]=anglemod(self->targetEnt->s.angles[ROLL]+90);//can't do roll?
			break;
		case 4:
			self->targetEnt->s.angles[ROLL]=anglemod(self->targetEnt->s.angles[ROLL]-90);//can't do roll?
			break;
		default:
			break;
	}


	VectorClear(self->targetEnt->velocity);
	VectorClear(self->targetEnt->avelocity);

	if(flrand(0,1)<0.5)
	{
		if(self->targetEnt->materialtype == MAT_INSECT)
			gi.CreateEffect(&self->targetEnt->s, FX_BLOOD, CEF_FLAG8, self->targetEnt->s.origin, "ub", blooddir, 200);
		else
			gi.CreateEffect(&self->targetEnt->s, FX_BLOOD, 0, self->targetEnt->s.origin, "ub", blooddir, 200);
	}
}

void tbeast_check_snatch(edict_t *self, float ofsf, float ofsr, float ofsu)
{
	float enemy_dist, ok_dist;
	vec3_t forward, right, up, startpos, endpos;
	edict_t	*found = NULL;
//	trace_t trace;

	if(!self->enemy)
		return;

	ok_dist = 64;

	AngleVectors(self->s.angles,forward,right,up);
	VectorMA(self->s.origin, ofsf  + TB_FWD_OFFSET, forward, startpos);
	VectorMA(startpos, ofsr, right, startpos);
	VectorMA(startpos, ofsu + TB_UP_OFFSET, up, startpos);

	VectorSubtract(self->enemy->s.origin, startpos, endpos);

	enemy_dist = VectorLength(endpos);
	if(enemy_dist>ok_dist || flrand(0, 50)>self->enemy->health)
	{//if missed or health is low, just chomp it now
		while(found = FindInRadius(found, startpos, ok_dist))
		{
			if(found->takedamage&&AI_IsMovable(found))
			{
				if(found->health<=0)
					T_Damage (found, self, self, endpos, found->s.origin, endpos, 2000, 300, DAMAGE_DISMEMBER,MOD_DIED);
				else
					break;	
			}
		}

		if(!found)
		{
//			gi.dprintf("Snatch missed by %4.2f!\n", enemy_dist - ok_dist);
			self->msgHandler = DefaultMsgHandler;
			/*
			if(!stricmp(self->enemy->classname,"player"))
			{
				if(self->oldenemy)
				{
					if(self->oldenemy->health>0)
					{
						self->oldenemy = NULL;
						self->enemy = self->oldenemy;
					}
				}
			}*/
			return;
		}
	}
	else
		found = self->enemy;

	self->msgHandler = DeadMsgHandler;
//	gi.dprintf("SNAGGED!\n");

	if(ofsu == TB_HIBITE_U + 128)
		SetAnim(self, ANIM_BITEUP2_SFIN);
	else if(ofsu == TB_HIBITE_U)
		SetAnim(self, ANIM_BITEUP_SFIN);
	else
		SetAnim(self, ANIM_BITELOW_SFIN);

	self->targetEnt = found;
	self->targetEnt->flags |= FL_FLY;
	self->targetEnt->movetype = PHYSICSTYPE_FLY;

	if(!found->client)
	{
		found->monsterinfo.aiflags |= AI_DONT_THINK;
	}
	else
	{
		found->nextthink = level.time + 10;//stuck for 10 seconds.
		if(found->health > 0)
		{
			if(found->client->playerinfo.lowerseq != ASEQ_KNOCKDOWN)
			{
				P_KnockDownPlayer(&found->client->playerinfo);
			}
		}
		gi.sound(found, CHAN_VOICE, sounds[irand(SND_CORVUS_SCREAM1, SND_CORVUS_SCREAM3)], 1, ATTN_NORM, 0);
	}

	VectorClear(found->velocity);
	VectorClear(found->avelocity);
}

void tbeast_go_snatch (edict_t *self)
{
	SetAnim(self, ANIM_SNATCH);
}

void tbeast_gore_toy(edict_t *self, float jumpht)
{
	float enemy_zdist, ok_zdist;
	byte num_chunks;
	vec3_t dir, forward;

	if(jumpht!=-1)
	{
		self->velocity[2] += jumpht;
		if(self->groundentity)
		{
			AngleVectors(self->s.angles, forward, NULL, NULL);
			VectorMA(self->velocity, -100, forward, self->velocity);
		}
	}
	else
		self->count = 0;

	if(!self->targetEnt)
		return;

	if(self->targetEnt->health<0)
		return;

	if(self->count)
		return;

	ok_zdist = 128;
	enemy_zdist = self->targetEnt->s.origin[2] - self->s.origin[2];
	if(enemy_zdist <= self->maxs[2] + ok_zdist || jumpht == -1)
	{//FIXME: waits grabs it too low, waits too long
		self->wait = self->targetEnt->materialtype;

		gi.sound(self, CHAN_WEAPON, sounds[SND_SNATCH], 1, ATTN_NORM, 0);
		if(jumpht!=-1)
			self->count = 1;
		VectorCopy(self->velocity,dir);
		VectorNormalize(dir);
		num_chunks = (byte)(self->targetEnt->health/4);
		if(num_chunks>15)
			num_chunks = 15;
		SprayDebris(self->targetEnt, self->targetEnt->s.origin, num_chunks, self->targetEnt->health*4);//self->enemy is thingtype wood?!

		if(stricmp(self->targetEnt->classname,"player"))
		{
			gi.sound(self->targetEnt, CHAN_WEAPON, sounds[SND_CATCH], 1, ATTN_NORM, 0);
			BecomeDebris(self->targetEnt);
		}
		else
		{
			self->targetEnt->nextthink = level.time;
			T_Damage (self->targetEnt, self, self, self->velocity, self->targetEnt->s.origin, dir, 2000, 300, DAMAGE_DISMEMBER|DAMAGE_NO_PROTECTION,MOD_DIED);
		}
		if(self->enemy == self->targetEnt)
			self->enemy = NULL;
		self->targetEnt = NULL;
	}
}

void tbeast_miss_sound (edict_t *self)
{
	gi.sound(self, CHAN_WEAPON, sounds[SND_SNATCH], 1, ATTN_NORM, 0);
}

void tbeast_anger_sound (edict_t *self)
{
	byte chance;

	chance = irand(0,100);
	if (chance < 10)
		gi.sound(self, CHAN_WEAPON, sounds[SND_SNORT1], 1, ATTN_NORM, 0);
	else if (chance < 20)
		gi.sound(self, CHAN_WEAPON, sounds[SND_SNORT2], 1, ATTN_NORM, 0);
	else if (chance < 30)
		gi.sound(self, CHAN_ITEM, sounds[SND_TEAR1], 1, ATTN_NORM, 0);
	else if (chance < 40)
		gi.sound(self, CHAN_ITEM, sounds[SND_TEAR2], 1, ATTN_NORM, 0);
	else if (chance < 50)
		gi.sound(self, CHAN_WEAPON, sounds[SND_CHOMP], 1, ATTN_NORM, 0);
	else if (chance < 60)
		tbeast_growl(self);

	if(self->targetEnt)
	{
		chance = (byte)irand(1,3);

		SprayDebris(self->targetEnt, self->targetEnt->s.origin, chance, 100);
		if(!self->targetEnt->client)
		{
			QPostMessage(self->targetEnt, MSG_DISMEMBER, PRI_DIRECTIVE, "ii", self->targetEnt->health*0.5, irand(1,13));//do I need last three if not sending them?
			QPostMessage(self->targetEnt, MSG_PAIN, PRI_DIRECTIVE, "eeiii", self, self, true, 200, 0);
		}
	}
}

void tbeast_gibs(edict_t *self)
{//FIXME: keep making gubs
	vec3_t		spot, mins, forward;
	byte		numchunks;
	int			flags = 0;

	if(!self->wait)
		return;

	AngleVectors(self->s.angles, forward, NULL, NULL);
	VectorMA(self->s.origin, 56, forward, spot);
	spot[2] -= 8;

	if(self->wait == MAT_INSECT)
	{
		flags |= CEF_FLAG8;
		flags |= CEF_FLAG7;//use male insect skin on chunks
	}

	numchunks = (byte)(irand(3, 7));
	VectorSet(mins, -1, -1, -1);
	gi.CreateEffect(NULL,
					FX_FLESH_DEBRIS,
   					flags,
   					spot,
   					"bdb",
   					numchunks, self->mins, 16);

	tbeast_anger_sound (self);
}

void tbeast_done_gore (edict_t *self)
{
	self->msgHandler = DefaultMsgHandler;
	self->count = 0;
	M_ValidTarget(self, self->enemy);

	self->monsterinfo.aiflags |= AI_EATING;
	SetAnim(self, ANIM_EATDOWN);
}

void tbeast_inair (edict_t *self)
{
	SetAnim(self, ANIM_INAIR);
}

void tbeast_check_landed (edict_t *self)
{
	if(TB_CheckBottom(self))
		SetAnim(self, ANIM_LAND);
}

void tbeast_ginair (edict_t *self)
{
	SetAnim(self, ANIM_GINAIR);
}

void tbeast_gcheck_landed (edict_t *self)
{
	if(TB_CheckBottom(self))
		SetAnim(self, ANIM_GLAND);
}

void tbeast_chomp(edict_t *self, float ofsf, float ofsr, float ofsu)
{
	float enemy_dist, ok_dist, damage;
	vec3_t forward, right, up, startpos, endpos, v;

	if(!self->enemy)
		return;

	ok_dist = 64;

	AngleVectors(self->s.angles,forward,right,up);
	VectorMA(self->s.origin, ofsf + TB_FWD_OFFSET, forward, startpos);
	VectorMA(startpos, ofsr, right, startpos);
	VectorMA(startpos, ofsu + TB_UP_OFFSET, up, startpos);

	VectorSubtract(self->enemy->s.origin, startpos, endpos);

	enemy_dist = VectorLength(endpos);
	if(enemy_dist>ok_dist)
	{//if missed or health is low, just chomp it now
//		gi.dprintf("Chomp missed by %4.2f!\n", enemy_dist - ok_dist);
		if(enemy_dist - ok_dist < 64)
		{//let them know it was close and we tried - spittle effect?
			gi.sound(self, CHAN_WEAPON, sounds[SND_SNATCH], 1, ATTN_NORM, 0);
		}
		return;
	}
	gi.sound(self, CHAN_WEAPON, sounds[SND_CHOMP], 1, ATTN_NORM, 0);
	VectorSubtract(self->enemy->s.origin, self->s.origin, v);
	VectorNormalize(v);
	damage = irand(TB_DMG_BITE_MIN, TB_DMG_BITE_MAX);
	T_Damage (self->enemy, self, self, forward, endpos, v, damage, damage/2, DAMAGE_DISMEMBER,MOD_DIED);
}

void tbeast_leap (edict_t *self, float fwdf, float rghtf, float upf)
{
	vec3_t	forward, right, up, angles;

	if(!self->groundentity)
	{
		if(!TB_CheckBottom(self))
			return;
	}

	if(self->s.frame == FRAME_jumpb7)
		tbeast_chomp(self, 36, 0, 232);

//	self->gravity = TB_JUMP_GRAV;
	VectorSet(angles, 0, self->s.angles[YAW], 0);
	AngleVectors(angles, forward, right, up);
	VectorScale(forward, fwdf, self->velocity);
	VectorMA(self->velocity, rghtf, right, self->velocity);
	VectorMA(self->velocity, upf, up, self->velocity);
}

/*========================

  Hacky Trial Beast fake collision and slope-standing code

  ========================*/

/*

  LerpAngleChange

  */

float LerpAngleChange (float curangle, float endangle, float step)
{
	float final, diff;

	curangle = anglemod(curangle);
	endangle = anglemod(endangle);

	if(curangle>180)
		curangle-=360;
	else if(curangle<-180)
		curangle+=360;

	if(endangle>180)
		endangle-=360;
	else if(endangle<-180)
		endangle+=360;

	if(curangle == endangle)
		return 0;

	diff = endangle - curangle;

	if(diff > 180)
		diff -= 360;
	else if(diff < -180)
		diff += 360;

	final = anglemod(curangle + diff/step);

	if(final>180)
		final-=360;
	else if(final<-180)
		final+=360;

	return final;
}

int tbeast_inwalkframes(edict_t *self)
{
	if(self->curAnimID == ANIM_CHARGE||self->curAnimID==ANIM_QUICK_CHARGE)
	{
		switch(self->s.frame)
		{
		case FRAME_charge1:
			return 2;
			break;
		case FRAME_charge2:
			return 4;
			break;
		case FRAME_charge3:
			return 6;
			break;
		case FRAME_charge4:
			return 8;
			break;
		case FRAME_charge5:
			return 9;
			break;
		case FRAME_charge6:
			return 7;
			break;
		case FRAME_charge7:
			return 13;
			break;
		case FRAME_charge8:
			return 15;
			break;
		case FRAME_charge9:
			return 1;
			break;
		case FRAME_charge10:
			return 0;
			break;
		default:
			return -1;
			break;
		}
	}

	if(self->s.frame>=FRAME_walk1 && self->s.frame<=FRAME_walk18)
		return self->monsterinfo.currframeindex;

	if(self->s.frame>=FRAME_wlklft1 && self->s.frame<=FRAME_wlklft18)
		return self->monsterinfo.currframeindex;
	
	if(self->s.frame>=FRAME_wlkrt1 && self->s.frame<=FRAME_wlkrt18)
		return self->monsterinfo.currframeindex;

	if(self->s.frame>=FRAME_wlkatk1 && self->s.frame<=FRAME_wlkatk18)
		return self->monsterinfo.currframeindex;

	if(self->s.frame>=FRAME_wait1 && self->s.frame<=FRAME_wait14)
		return 16;

	return -1;
}

/*

  LevelToGround

  I'm a big guy, level me out
	What slope am I on
	?
*/
void LevelToGround (edict_t *self, float fscale, float rscale, qboolean z_adjust)
{
	vec3_t		forward, right, backpos, frontpos, leftpos, rightpos, dir, angles, bottom1, bottom2;
	vec3_t		lfootoffset, rfootoffset, up;
	trace_t		trace;
	qboolean	right_front;
	int			leg_check_index;
	int			count = 0;

	AngleVectors(self->s.angles, forward, right, up);
	
	leg_check_index = tbeast_inwalkframes(self);

	if(leg_check_index > -1)
	{//set up leg checks - only if in these frames
		if(leg_check_index > 5 && leg_check_index < 15)
			right_front = true;
		else
			right_front = false;

	//left leg
		VectorCopy(left_foot_offset_for_frame_index[leg_check_index], lfootoffset);
		VectorMA(self->s.origin, lfootoffset[0] + TB_FWD_OFFSET, forward, leftpos);
		VectorMA(leftpos, lfootoffset[1] + TB_RT_OFFSET, right, leftpos);
		VectorMA(leftpos, lfootoffset[2] + TB_UP_OFFSET, up, leftpos);
		
	//right leg
		VectorCopy(right_foot_offset_for_frame_index[leg_check_index], rfootoffset);
		VectorMA(self->s.origin, rfootoffset[0] + TB_FWD_OFFSET, forward, rightpos);
		VectorMA(rightpos, rfootoffset[1] + TB_RT_OFFSET, right, rightpos);
		VectorMA(rightpos, rfootoffset[2] + TB_UP_OFFSET, up, rightpos);

		if(right_front)
		{//this is also the front check
			VectorCopy(rightpos, frontpos);	
			VectorCopy(leftpos, backpos);	
		}
		else
		{
			VectorCopy(leftpos, frontpos);	
			VectorCopy(rightpos, backpos);	
		}
	}
	else return;
	/*{
		VectorCopy(self->s.origin, backpos);
		backpos[2] += self->mins[2] + 10;
	
		VectorMA(backpos, self->maxs[0] * fscale, forward, frontpos);
		VectorMA(backpos, self->mins[0] * fscale, forward, backpos);

		VectorCopy(self->s.origin, leftpos);
		leftpos[2] += self->mins[2] + 10;
		
		VectorMA(leftpos, self->maxs[0] * rscale, right, rightpos);
		VectorMA(leftpos, self->mins[0] * rscale, right, leftpos);
	}*/
	
	VectorCopy(frontpos, bottom1);
	bottom1[2] -= self->size[2] * 2;
	gi.trace(frontpos, vec3_origin, vec3_origin, bottom1, self, MASK_SOLID,&trace);
	if(trace.fraction == 1.0)
	{
		self->s.angles[PITCH] = LerpAngleChange (self->s.angles[PITCH], 0, 8);
	}
	else
	{
		VectorCopy(trace.endpos, bottom1);

		VectorCopy(backpos, bottom2);
		bottom2[2] -= self->size[2] * 2;
		gi.trace(backpos, vec3_origin, vec3_origin, bottom2, self, MASK_SOLID,&trace);
		if(trace.fraction == 1.0)
		{
			self->s.angles[PITCH] = LerpAngleChange (self->s.angles[PITCH], 0, 8);
		}
		else
		{
			VectorCopy(trace.endpos, bottom2);

			VectorSubtract(bottom1, bottom2, dir);
			vectoangles(dir, angles);

			self->s.angles[PITCH] = LerpAngleChange (self->s.angles[PITCH], angles[PITCH], 8);
		}
	}

	VectorCopy(rightpos, bottom1);
	bottom1[2] -= self->size[2] * 2;
	gi.trace(rightpos, vec3_origin, vec3_origin, bottom1, self, MASK_SOLID,&trace);
	if(trace.fraction == 1.0)
	{
		self->s.angles[ROLL] = LerpAngleChange (self->s.angles[ROLL], 0, 8);
	}
	else
	{
		VectorCopy(trace.endpos, bottom1);

		VectorCopy(leftpos, bottom2);
		bottom2[2] -= self->size[2] * 2;
		gi.trace(leftpos, vec3_origin, vec3_origin, bottom2, self, MASK_SOLID,&trace);
		if(trace.fraction == 1.0)
		{
			self->s.angles[ROLL] = LerpAngleChange (self->s.angles[ROLL], 0, 8);
		}
		else
		{
			VectorCopy(trace.endpos, bottom2);

			VectorSubtract(bottom1, bottom2, dir);
			vectoangles(dir, angles);

			self->s.angles[ROLL] = LerpAngleChange (self->s.angles[ROLL], angles[PITCH], 8);
		}
	}

	/*
	if(z_adjust)
	{
		if((gi.pointcontents(rightpos) & MASK_SOLID) ||	(gi.pointcontents(leftpos) & MASK_SOLID))
		{
			gi.dprintf("Beast feet in ground, raising up\n");
			while(((gi.pointcontents(rightpos) & MASK_SOLID) ||
				(gi.pointcontents(leftpos) & MASK_SOLID)) && count < 10)
			{
				self->mins[2]++;
				self->s.origin[2]++;
				rightpos[2]++;
				leftpos[2]++;
				count++;
			}
			gi.linkentity(self);
		}
		else
		{
			leftpos[2] -= 4;
			rightpos[2] -= 4;
			if(!(gi.pointcontents(rightpos) & MASK_SOLID) && !(gi.pointcontents(leftpos) & MASK_SOLID))
			{
				gi.dprintf("Beast feet not on ground, lowering\n");
				while(!(gi.pointcontents(rightpos) & MASK_SOLID) &&
					!(gi.pointcontents(leftpos) & MASK_SOLID) && count < 10)
				{
					self->mins[2]--;
					self->s.origin[2]--;
					rightpos[2]--;
					leftpos[2]--;
					count++;
				}
				gi.linkentity(self);
			}
		}
	}*/
}

void tbeast_fake_impact(edict_t *self, trace_t *trace, qboolean crush)
{
	trace_t	tr;
	vec3_t	dir, bottom;
	qboolean	throwthem = true;

	if(trace->ent->svflags & SVF_TOUCHED_BEAST)
		return;

	if(trace->ent == self->targetEnt)
		return;

	if(trace->ent->classID == CID_FUNC_DOOR)
		return;

	if(trace->ent->classID == CID_TCHECKRIK)
		return;//we want to pick up and eat insects

	if(trace->ent  && trace->ent->movetype && trace->ent !=world && stricmp(trace->ent->classname, "worldspawn"))
	{
		if(trace->ent->client||trace->ent->svflags&SVF_MONSTER)
		{
			if(trace->ent->s.origin[2] > self->absmax[2] - 10)
			{//FIXME: chance of throwing them off
				trace->ent->s.origin[2] = self->absmax[2];
				trace->ent->velocity[2] = 0;
				trace->ent->groundentity = self;
				throwthem = false;
			}
		}
		if(throwthem)
		{
			VectorCopy(self->s.origin, bottom);
			bottom[2] += self->mins[2];
			VectorSubtract(trace->ent->s.origin, bottom, dir);
			VectorNormalize(dir);
		}

		if(AI_IsMovable(trace->ent) || trace->ent->takedamage)
		{
			if(throwthem)
				VectorScale(dir, 200, trace->ent->velocity);
		}
		else if(Vec3NotZero(self->velocity) && trace->fraction < 0.7)
		{
			if(AI_IsInfrontOf(self, trace->ent))
			{
				if(trace->ent->targetname && !stricmp(trace->ent->targetname, "pillar"))
				{//FIXME: In higher skills, less chance of breaking it?  Or debounce time?

					if(IsVisibleToClient(self))
					{
//						gi.dprintf("Beast hit pillar!\n");
						G_UseTargets (trace->ent, self);
						trace->ent->targetname = "";//so we don't hit them again
						trace->ent->target = "stop";//so it doesn't fire the target when it's broken later
						self->monsterinfo.attack_finished = level.time + 3;

						self->velocity[0] = self->velocity[1] = 0;
						self->sounds++;
					
						self->red_rain_count++;

						if(self->red_rain_count >= 2)//got both pillars, now die
						{
							//self->clipmask = 0;
							self->solid = SOLID_NOT;
							self->takedamage = DAMAGE_NO;
						}
					}
					gi.CreateEffect(&self->s,
						FX_QUAKE,
						0,
						vec3_origin,
						"bbb",
						4,
						3,
						7);

					if(self->sounds!=2 && irand(0, 1))
						SetAnim(self, ANIM_STUN);
				}
			}
		}
		
		if(trace->ent->touch&&trace->ent->solid!=SOLID_NOT)
			trace->ent->touch (trace->ent, self, &trace->plane, trace->surface);

		if(trace->ent->isBlocked&&trace->ent->solid!=SOLID_NOT)
		{
			tr = *trace;
			tr.ent = self;
			trace->ent->isBlocked(trace->ent, &tr);
		}

		if(throwthem && trace->ent->takedamage)
		{
			float	damage;

			if(Vec3NotZero(self->velocity))
			{
				if(trace->ent->client)
				{
					if(trace->ent->health > 30)
						DoImpactDamage(self, trace);

					if(trace->ent->groundentity && trace->ent->health)
					{
						if(trace->ent->client->playerinfo.lowerseq != ASEQ_KNOCKDOWN)
						{
							P_KnockDownPlayer(&trace->ent->client->playerinfo);
						}
					}
				}
				else
					DoImpactDamage(self, trace);
			}
			else
			{
				if(trace->ent->client)
				{
					if(crush)
						damage = flrand(20, 100);
					else if(trace->ent->health > 30)
						damage = flrand(10, 30) * skill->value/2;
					else
						damage = 0;

					if(!irand(0, 5) || (crush && !irand(0,1)))
					{
						if(trace->ent->client->playerinfo.lowerseq != ASEQ_KNOCKDOWN)
							P_KnockDownPlayer(&trace->ent->client->playerinfo);
					}
					if(damage)
						T_Damage(trace->ent, self, self, dir, trace->endpos, dir, 
								flrand(TB_DMG_IMPACT_MIN, TB_DMG_IMPACT_MAX), TB_DMG_IMPACT_KB, 0,MOD_DIED);
				}
				else
				{
					if(crush)
						damage = flrand(1000, 3000);
					else
						damage = flrand(20, 100);
					T_Damage(trace->ent, self, self, dir, trace->endpos, dir, 1000, 250, 0,MOD_DIED);
				}
			}
		}
		else
		{
			if(trace->ent->client)
			{
				if(trace->ent->groundentity && trace->ent->health)
				{
					if(trace->ent->client->playerinfo.lowerseq != ASEQ_KNOCKDOWN)
					{
						P_KnockDownPlayer(&trace->ent->client->playerinfo);
					}
				}
			}
		}
	}
}

qboolean boxes_overlap(vec3_t mins1, vec3_t maxs1, vec3_t mins2, vec3_t maxs2)
{
	if(mins1[0]>maxs2[0])
		return false;

	if(mins1[1]>maxs2[1])
		return false;

	if(mins1[2]>maxs2[2])
		return false;

	if(maxs1[0]<mins2[0])
		return false;

	if(maxs1[1]<mins2[1])
		return false;

	if(maxs1[2]<mins2[2])
		return false;

	return true;
}

void tbeast_check_impacts(edict_t *self)
{//Used by Trial Beast for Simulated Complex Bounding Box Collision

	vec3_t		forward, right, up, start, end, mins, maxs, lfootoffset, rfootoffset;
	vec3_t		lstart, lend, rstart, rend, lfootmins, lfootmaxs, rfootmins, rfootmaxs, fmins, fmaxs;
	int			leg_check_index;
	qboolean	hitme = true;
	qboolean	hitother = false;
	trace_t		trace;

	AngleVectors(self->s.angles, forward, right, up);

	//set up body check
	VectorCopy(self->s.origin, start);
	start[2] += 64 + TB_UP_OFFSET;//bottom of torso
	start[2] += 61;//halfway up to the top of torso
	VectorMA(start, 150 + TB_FWD_OFFSET, forward, end);
	VectorMA(start, -120, forward, start);// + TB_FWD_OFFSET???
	VectorSet(mins, -50, -50, -61);
	VectorSet(maxs, 50, 50, 70);

	leg_check_index = tbeast_inwalkframes(self);

	if(leg_check_index > -1)
	{//set up leg checks - FIXME: trace from last footpos to current one
		VectorSet(fmins, -8, -8, 0);
		VectorSet(fmaxs, 8, 8, 1);

	//left leg
		VectorCopy(left_foot_offset_for_frame_index[leg_check_index], lfootoffset);
		VectorMA(self->s.origin, lfootoffset[0] + TB_FWD_OFFSET, forward, lend);
		VectorMA(lend, lfootoffset[1] + TB_RT_OFFSET, right, lend);
		VectorMA(lend, lfootoffset[2] + TB_UP_OFFSET, up, lend);
		VectorCopy(lend, lstart);
		lstart[2]+=63;
		
		VectorAdd(lend, fmins, lfootmins);
		VectorCopy(lfootmins, lfootmaxs);
		lfootmaxs[2] += 64;

	//right leg
		VectorCopy(right_foot_offset_for_frame_index[leg_check_index], rfootoffset);
		VectorMA(self->s.origin, rfootoffset[0] + TB_FWD_OFFSET, forward, rend);
		VectorMA(rend, rfootoffset[1] + TB_RT_OFFSET, right, rend);
		VectorMA(rend, rfootoffset[2] + TB_UP_OFFSET, up, rend);
		VectorCopy(rend, rstart);
		rstart[2]+=63;

		VectorAdd(rend, fmins, rfootmins);
		VectorCopy(rfootmins, rfootmaxs);
		rfootmaxs[2] += 64;
	}

//BODY
	//Fix me: continue the trace if less than 1.0 or save for next touch?
	gi.trace(start, mins, maxs, end, self, MASK_MONSTERSOLID,&trace);
	//Hey!  Check and see if they're close to my mouth and chomp 'em!
	tbeast_fake_impact(self, &trace, false);

	if(leg_check_index == -1)
	{
		VectorCopy(self->s.origin, end);
		end[2] += self->mins[2];
		VectorCopy(end, start);
		start[2] += 63;
		VectorSet(mins, -32, -32, 0);
		VectorSet(maxs, 32, 32, 1);

		gi.trace(start, mins, maxs, end, self, MASK_MONSTERSOLID,&trace);
		tbeast_fake_impact(self, &trace, false);
		return;
	}

//Do leg checks
//left leg
	//Fix me: continue the trace if less than 1.0 or save for next touch?
	gi.trace(lstart, fmins, fmaxs, lend, self, MASK_MONSTERSOLID,&trace);
	tbeast_fake_impact(self, &trace, true);

//right leg
	//Fix me: continue the trace if less than 1.0 or save for next touch?
	gi.trace(rstart, fmins, fmaxs, rend, self, MASK_MONSTERSOLID,&trace);
	tbeast_fake_impact(self, &trace, true);
}

void tbeast_fake_touch(edict_t *self)
{//Used by Trial Beast for Simulated Complex Bounding Box Collision

	vec3_t		forward, right, up, start, end, dir, mins, maxs;
	vec3_t		lfootoffset, rfootoffset, omins, omaxs;
	vec3_t		lstart, lend, rstart, rend, lfootmins, lfootmaxs, rfootmins, rfootmaxs, fmins, fmaxs;
	vec3_t		melee_point;
	int			leg_check_index;
	qboolean	hitme = true;
	qboolean	hitother = false;
	trace_t		trace;
	int			osolid, ocm;
	int			i, num;
	edict_t		*touch[MAX_EDICTS], *other;

	num = gi.BoxEdicts (self->absmin, self->absmax, touch, MAX_EDICTS, AREA_SOLID);
	// be careful, it is possible to have an entity in this
	// list removed before we get to it (killtriggered)

	if(!touch[0])
		goto finish;

	AngleVectors(self->s.angles, forward, right, up);

	leg_check_index = tbeast_inwalkframes(self);

	if(leg_check_index > -1)
	{//set up leg checks - FIXME: trace from last footpos to current one
		
		//Walking, Check melee point in front
		VectorMA(self->s.origin, self->maxs[2]*0.5 + TB_UP_OFFSET, up, melee_point);
		VectorMA(melee_point, 150 + TB_FWD_OFFSET, forward, melee_point);

		VectorSet(fmins, -8, -8, 0);
		VectorSet(fmaxs, 8, 8, 1);

	//left leg
		VectorCopy(left_foot_offset_for_frame_index[leg_check_index], lfootoffset);
		VectorMA(self->s.origin, lfootoffset[0] + TB_FWD_OFFSET, forward, lend);
		VectorMA(lend, lfootoffset[1] + TB_RT_OFFSET, right, lend);
		VectorMA(lend, lfootoffset[2] + TB_UP_OFFSET, up, lend);
		VectorCopy(lend, lstart);
		lstart[2]+=63;
		
		VectorAdd(lend, fmins, lfootmins);
		VectorCopy(lfootmins, lfootmaxs);
		lfootmaxs[2] += 64;

	//right leg
		VectorCopy(right_foot_offset_for_frame_index[leg_check_index], rfootoffset);
		VectorMA(self->s.origin, rfootoffset[0] + TB_FWD_OFFSET, forward, rend);
		VectorMA(rend, rfootoffset[1] + TB_RT_OFFSET, right, rend);
		VectorMA(rend, rfootoffset[2] + TB_UP_OFFSET, up, rend);
		VectorCopy(rend, rstart);
		rstart[2]+=63;

		VectorAdd(rend, fmins, rfootmins);
		VectorCopy(rfootmins, rfootmaxs);
		rfootmaxs[2] += 64;
	}

	//set up body check
	VectorCopy(self->s.origin, start);
	start[2] += 64 + TB_UP_OFFSET;//bottom of torso
	start[2] += 35;//halfway up to the top of torso
	VectorMA(start, 150 + TB_FWD_OFFSET, forward, end);
	VectorMA(start, -120, forward, start);// + TB_FWD_OFFSET???
	VectorSet(mins, -50, -50, -61);
	VectorSet(maxs, 50, 50, 70);

	for (i=0 ; i<num ; i++)
	{
		other = touch[i];
		if (!other->inuse)
			continue;
		
		if(other==self)
			continue;

		if(!stricmp(other->classname, "worldspawn"))
			continue;

		if(other == self->targetEnt)
			continue;

		if(self->curAnimID != ANIM_CHARGE && self->curAnimID != ANIM_QUICK_CHARGE)
		{
			if(leg_check_index > -1 && other->takedamage && AI_IsMovable(other))
			{//Hey!  Check and see if they're close to my mouth and chomp 'em!
				if(vhlen (other->s.origin, melee_point) < 100)
				{
					self->oldenemy = self->enemy;
					self->enemy = other;
					QPostMessage(self, MSG_MELEE, PRI_DIRECTIVE, NULL);
					goto finish;
				}
			}
		}

		if(other->classID == CID_TCHECKRIK)
			continue;//we want to pick up and eat insects

		//make other solid and size temp for trace
		ocm = other->clipmask;
		osolid = other->solid;
		if(!Vec3IsZero(other->mins))
			VectorCopy(other->mins, omins);
		else
		{
			VectorCopy(other->mins, omins);
			VectorSet(other->mins, -1, -1, -1);
		}

		if(!Vec3IsZero(other->maxs))
			VectorCopy(other->maxs, omaxs);
		else
		{
			VectorCopy(other->maxs, omaxs);
			VectorSet(other->maxs, 1, 1, 1);
		}

		other->solid = SOLID_BBOX;
		other->clipmask = MASK_ALL;
//BODY
		//Fix me: continue the trace if less than 1.0 or save for next touch?
		gi.trace(start, mins, maxs, end, self, MASK_MONSTERSOLID,&trace);
		//put other back to normal
		other->solid = osolid;
		other->clipmask = ocm;
		VectorCopy(omins, other->mins);
		VectorCopy(omaxs, other->maxs);
		
		if(trace.ent==other)//hit something with BODY , touch it
			hitother = true;//hit other!

		if(!hitother && other->absmin[2]>self->absmax[2] - 10)
		{
			if(!irand(0,10))
			{
//				gi.dprintf("Jump to throw off something\n");
				SetAnim(self, ANIM_JUMP);
				VectorSubtract(other->s.origin, self->s.origin, dir);
				VectorNormalize(dir);
				VectorScale(dir, 500, other->velocity);
//				other->groundentity = NULL;
			}
		}
		else
		{
			if(hitother)
				hitme = true;
			else
			{
				hitme = false;
				if(leg_check_index > -1)
				{
					VectorAdd(other->s.origin, other->mins, omins);
					VectorAdd(other->s.origin, other->maxs, omaxs);
					if(boxes_overlap(omins, omaxs, lfootmins, lfootmaxs))
						hitme = true;
					else if(boxes_overlap(omins, omaxs, rfootmins, rfootmaxs))
						hitme = true;
				}
			}

			if(hitme)
			{
				if(other->isBlocked&&other->solid!=SOLID_NOT)
				{
					gi.trace(other->s.origin, vec3_origin, vec3_origin, self->s.origin, other, MASK_ALL,&trace);
					trace.ent = self;
					VectorCopy(other->s.origin, trace.endpos);
					other->isBlocked(other, &trace);
				}
				if(other->touch&&other->solid!=SOLID_NOT)
				{
					gi.trace(other->s.origin, vec3_origin, vec3_origin, self->s.origin, other, MASK_ALL,&trace);
					trace.ent = self;
					VectorCopy(other->s.origin, trace.endpos);
					other->touch (other, self, &trace.plane, trace.surface);
				}
				if(other && other == trace.ent)
				{//if other still valid, do my impact with it
					tbeast_fake_impact(self, &trace, false);
					other->svflags |= SVF_TOUCHED_BEAST;//so check_impacts doesn't do anything with it
				}
			}
		}
	}

finish:
	tbeast_check_impacts(self);

	for (i=0 ; i<num ; i++)
	{
		touch[i]->svflags &= ~SVF_TOUCHED_BEAST;
	}

}

void tbeast_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	tbeast_fake_touch(self);	
}

void tbeast_post_think (edict_t *self)
{
#define TBEAST_SBAR_SIZE	3500

	trace_t		trace;
	vec3_t		end, mins, maxs;
	float		omins2;
	qboolean	go_jump = false;

	if(self->monsterinfo.awake)
	{
		if(self->volume < (float)(self->max_health))
		{
			M_ShowLifeMeter(self, (int)(ceil(self->volume/self->max_health*TBEAST_SBAR_SIZE)), (int)(ceil(self->volume/self->max_health*TBEAST_SBAR_SIZE)));
			self->volume += (float)(self->max_health) / 10;
		}
		else if(self->health > 0)
		{
			M_ShowLifeMeter(self, (int)(ceil((float)(self->health)/(float)(self->max_health)*TBEAST_SBAR_SIZE)), TBEAST_SBAR_SIZE);
		}
	}

	if(self->s.origin[0] != self->s.old_origin[0] || self->s.origin[1] != self->s.old_origin[1])
		LevelToGround(self, 0.5, 0.25, true);

	if(Q_fabs(self->s.angles[PITCH])>45 || Q_fabs(self->s.angles[ROLL])>45)
		go_jump = true;
	else
	{
		//raise him up if on flat ground, lower is on slope - to keep feet on ground!
		//FIXME - use checkbottom plane instead?
		
		if(self->s.origin[0] != self->s.old_origin[0] || self->s.origin[1] != self->s.old_origin[1])
		{
			omins2 = self->mins[2];
			self->mins[2] = ((Q_fabs(self->s.angles[PITCH]) + Q_fabs(self->s.angles[ROLL]))*0.5)/45 * 144 - 6 + TB_UP_OFFSET;
			omins2 -= self->mins[2];
			if(omins2)
			{
				VectorCopy(self->s.origin, end);
				end[2] += omins2;
				gi.trace(self->s.origin, self->mins, self->maxs, end, self, MASK_SOLID,&trace);
				VectorCopy(trace.endpos, self->s.origin);
			}
			gi.linkentity(self);
		}
	}

	VectorCopy(self->s.origin, self->s.old_origin);
//	if(!TB_CheckBottom(self))
//	{
//		gi.dprintf("Beast not on ground jump!\n");
//		SetAnim(self, ANIM_JUMP);
//	}

	if(!irand(0, 10))
	{
		if(self->curAnimID == ANIM_WALK ||
			self->curAnimID == ANIM_WALKLEFT ||
			self->curAnimID == ANIM_WALKRT ||
			self->curAnimID == ANIM_WALKATK)
		{
			VectorCopy(self->s.origin, end);
			end[2] -= 64;

			VectorSet(mins, -8, -8, 0);
			VectorSet(maxs, 8, 8, 2);

			gi.trace(self->s.origin, mins, maxs, end, self, MASK_SOLID,&trace);
			if(trace.fraction == 1.0 && !trace.startsolid && !trace.allsolid)
				go_jump = true;
		}
	}
	
	if(go_jump)
	{
//		gi.dprintf("Beast not on ground jump!\n");
		TB_CheckJump (self);
		//SetAnim(self, ANIM_JUMP);
	}

	tbeast_fake_touch(self);

	self->next_post_think = level.time + 0.1;
}

edict_t *TB_CheckHit(const vec3_t start, vec3_t end) //mxd. Named 'check_hit_beast' in original logic.
{
	edict_t	*found = NULL;
	int		i = 0;
	vec3_t	shot_dir, beast_dir, checkpos, diffvec;
	float	diff1, diff2;

	VectorSubtract(end, start, shot_dir);
	diff1 = VectorNormalize(shot_dir);

	while(found = G_Find(found, FOFS(classname), "monster_trial_beast"))
	{
		VectorSubtract(found->s.origin, start, beast_dir);
		diff2 = VectorLength(beast_dir) - 128;

		if(diff2 > diff1)
			continue;

		//beast closer than trace endpos, let's do an incremental check

		VectorCopy(start, checkpos);
		
		for(i = 16; i < diff1; i+=16)
		{
			VectorMA(checkpos, 16, shot_dir, checkpos);
			VectorSubtract(checkpos, found->s.origin, diffvec);
			if(VectorLengthSquared(diffvec) < 16384)//128 squared
			{//this spot is within 128 of beast origin, so you hit him, ok?
				VectorCopy(checkpos, end);
				return found;
			}
		}
	}
	
	return NULL;
}

void tbeast_go_charge (edict_t *self, edict_t *other, edict_t *activator)
{
	self->enemy = activator;//are we certain activator is client?
	//do a FoundTarget(self, false);?
	self->dmg = true;
	SetAnim(self, ANIM_CHARGE);
	self->use = TBeastDie_UseCallback;
}

void TBeastStaticsInit(void)
{
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

	sounds[SND_ROAR]  = gi.soundindex ("monsters/tbeast/roar.wav");	
	sounds[SND_ROAR2]  = gi.soundindex ("monsters/tbeast/roar2.wav");	
	sounds[SND_SNORT1]  = gi.soundindex ("monsters/tbeast/snort1.wav");	
	sounds[SND_SNORT2]  = gi.soundindex ("monsters/tbeast/snort2.wav");	

	sounds[SND_STEP1]  = gi.soundindex ("monsters/tbeast/step1.wav");	
	sounds[SND_STEP2] = gi.soundindex ("monsters/tbeast/step2.wav");	
	sounds[SND_LAND]     = gi.soundindex ("monsters/tbeast/land.wav");	

	sounds[SND_GROWL1]     = gi.soundindex ("monsters/tbeast/growl1.wav");	
	sounds[SND_GROWL2]     = gi.soundindex ("monsters/tbeast/growl2.wav");	
	sounds[SND_GROWL3]     = gi.soundindex ("monsters/tbeast/growl3.wav");	

//	sounds[SND_SWIPE]= gi.soundindex ("monsters/tbeast/swipe.wav");	
	sounds[SND_SLAM]= gi.soundindex ("monsters/tbeast/slam.wav");	
	sounds[SND_SNATCH]= gi.soundindex ("monsters/tbeast/snatch.wav");	
	sounds[SND_CHOMP]= gi.soundindex ("monsters/tbeast/chomp.wav");	
	sounds[SND_TEAR1]= gi.soundindex ("monsters/tbeast/tear1.wav");	
	sounds[SND_TEAR2]= gi.soundindex ("monsters/tbeast/tear2.wav");	
	sounds[SND_THROW]= gi.soundindex ("monsters/tbeast/throw.wav");	
	sounds[SND_CATCH]= gi.soundindex ("monsters/tbeast/catch.wav");	
//	sounds[SND_SWALLOW]= gi.soundindex ("monsters/tbeast/swallow.wav");

	sounds[SND_PAIN1]= gi.soundindex ("monsters/tbeast/pain1.wav");	
	sounds[SND_PAIN2]= gi.soundindex ("monsters/tbeast/pain2.wav");	
	sounds[SND_DIE]= gi.soundindex ("monsters/tbeast/die.wav");	

	sounds[SND_CORVUS_SCREAM1] = gi.soundindex ("corvus/bdeath1.wav");
	sounds[SND_CORVUS_SCREAM2] = gi.soundindex ("corvus/bdeath2.wav");
	sounds[SND_CORVUS_SCREAM3] = gi.soundindex ("corvus/bdeath3.wav");
	sounds[SND_CORVUS_DIE] = gi.soundindex ("player/falldeath1.wav");

	res_info.numSounds = NUM_SOUNDS;
	res_info.sounds = sounds;

	classStatics[CID_TBEAST].resInfo = &res_info;
}


/*QUAKED monster_trial_beast (1 .5 0) (-100 -100 -36) (100 100 150) ?

The Trial Beastie

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
mintel					= 100
melee_range				= 400 (bite)
missile_range			= 1500 (charge)
min_missile_range		= 100
bypass_missile_chance	= 77
jump_chance				= 100
wakeup_distance			= 3000

NOTE: A value of zero will result in defaults, if you actually want zero as the value, use -1
*/
void SP_monster_trial_beast (edict_t *self)
{
	// Generic Monster Initialization
	if ((deathmatch->value == 1) && !((int)sv_cheats->value & self_spawn))
	{
		G_FreeEdict(self);
		return;
	}

	if (!M_WalkmonsterStart (self))		// Incomplete initialization
		return;

	self->msgHandler = DefaultMsgHandler;
	self->monsterinfo.aiflags |= AI_BRUTAL|AI_AGRESSIVE|AI_SHOVE;
	self->monsterinfo.otherenemyname = "monster_tcheckrik_male";

	self->health = TB_HEALTH * (skill->value + 1) / 3;

	self->mass = TB_MASS;
	self->yaw_speed = 10;
	self->isBlocked = TBeastBlocked;
	self->bounced = TBeastBlocked;

	self->movetype=PHYSICSTYPE_STEP;
	VectorClear(self->knockbackvel);

	//problem- staff won't work on him!
	self->solid=SOLID_TRIGGER;//BBOX;
	self->materialtype = MAT_FLESH;

//	VectorSet(self->mins, -116, -116, -4);
//	VectorSet(self->maxs, 116, 116, 182);
	VectorSet(self->mins, -100, -100, -36);
	VectorSet(self->maxs, 100, 100, 150);

	self->viewheight = 104 + TB_UP_OFFSET;

	self->s.modelindex = classStatics[CID_TBEAST].resInfo->modelIndex;

	//Big guy can be stood on top of perhaps?
	//self->touch = M_Touch;

	if(!self->wakeup_distance)
		self->wakeup_distance = 3000;

	MG_InitMoods(self);

	QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
	
	self->dmg = false;

	self->svflags|=(SVF_BOSS|SVF_NO_AUTOTARGET);

//	self->monsterinfo.aiflags &= ~AI_USING_BUOYS;

	if(!irand(0,1))
		self->ai_mood_flags |= AI_MOOD_FLAG_PREDICT;

	self->monsterinfo.aiflags |= AI_NIGHTVISION;

	self->touch = tbeast_touch;
	self->post_think = tbeast_post_think;
	self->next_post_think = level.time + 0.1;
	self->elasticity = ELASTICITY_SLIDE;
	self->count = self->sounds = 0;
	self->clipmask = CONTENTS_SOLID;
	self->solid = SOLID_TRIGGER;//WHY IS HE BEING PUSHED BY BSP entities now???!
	self->red_rain_count = 0;//pillar init
	self->use = tbeast_go_charge;
	self->delay = true;
	
	self->max_health = self->health;
	self->volume = 0;
	self->wait = 0;

	level.fighting_beast = true;//sorry, only one beast per level
}
