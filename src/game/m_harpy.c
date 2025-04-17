//
// m_harpy.c
//
// Copyright 1998 Raven Software
//

#include "m_harpy.h"
#include "m_harpy_shared.h"
#include "m_harpy_anim.h"
#include "g_debris.h" //mxd
#include "g_DefaultMessageHandler.h"
#include "g_monster.h"
#include "m_stats.h"
#include "p_anims.h"
#include "p_client.h"
#include "Random.h"
#include "Utilities.h"
#include "Vector.h"
#include "g_local.h"

#define HARPY_CHECK_DIST		128.0f
#define	HARPY_COLLISION_DIST	148.0f
#define HARPY_MIN_SWOOP_DIST	128.0f //mxd. Named 'HARPY_MIN_SSWOOP_DIST' in original logic.
#define HARPY_MIN_HOVER_DIST	128.0f
#define HARPY_MAX_HOVER_DIST	512.0f
#define HARPY_MAX_DIVE_DIST		108.0f //mxd. Named 'HARPY_MIN_SWOOP_DIST' in original logic.

#define HARPY_DRIFT_AMOUNT_X	128.0f
#define HARPY_DRIFT_AMOUNT_Y	128.0f
#define HARPY_DRIFT_AMOUNT_Z	64.0f

#define HARPY_SWOOP_INCREMENT	2 //mxd. Named 'HARPY_SWOOP_INCR' in original logic.
#define HARPY_MAX_SWOOP_SPEED	512.0f //mxd. Named 'HARPY_SWOOP_SPEED_MAX' in original logic.

#define HARPY_PROJECTILE_SEARCH_RADIUS	1024.0f //mxd. Named 'HARPY_PROJECTILE_RADIUS' in original logic.

//TODO: what happens when several harpies try to carry a head?..
edict_t* give_head_to_harpy = NULL; // Harpy, which carries the head. //mxd. Not SUS at all :) //TODO: rename to harpy_head_carrier?
edict_t* take_head_from = NULL; // Player or monster, who's head harpy is carrying. //TODO: rename to harpy_head_source?

#pragma region ========================== Gorgon base info ==========================

static const animmove_t* animations[NUM_ANIMS] =
{
	&harpy_move_die1,
	&harpy_move_fly1,
	&harpy_move_flyback1,
	&harpy_move_hover1,
	&harpy_move_hoverscream,
	&harpy_move_dive_go,
	&harpy_move_dive_loop,
	&harpy_move_dive_end,
	&harpy_move_pain1,
	&harpy_move_glide,
	&harpy_move_dive_trans,
	&harpy_move_dive_hit_loop,
	&harpy_move_tumble,
	&harpy_move_pirch1_idle,
	&harpy_move_pirch2_idle,
	&harpy_move_pirch3_idle,
	&harpy_move_pirch4_idle,
	&harpy_move_pirch5_idle,
	&harpy_move_pirch6_idle,
	&harpy_move_pirch7_idle,
	&harpy_move_pirch8_idle,
	&harpy_move_pirch9_idle,
	&harpy_move_takeoff,
	&harpy_move_circle,
	&harpy_move_circle_flap
};

static int sounds[NUM_SOUNDS];

static const vec3_t dead_harpy_mins = { -16.0f, -16.0f, 0.0f }; //mxd
static const vec3_t dead_harpy_maxs = { 16.0f,  16.0f, 12.0f }; //mxd

#pragma endregion

static int HarpyHeadDie(edict_t* self, edict_t* inflictor, edict_t* attacker, int damage, vec3_t point) //mxd. Named 'head_die' in original logic.
{
	BecomeDebris(self);
	return 1;
}

static void HarpyHeadThink(edict_t* self) //mxd. Named 'harpy_head_think' in original logic.
{
	if (self->owner == NULL || self->owner->health <= 0)
	{
		self->movetype = PHYSICSTYPE_STEP;
		self->elasticity = 0.8f;
		self->gravity = 1.0f;
		self->solid = SOLID_BBOX;
		self->takedamage = DAMAGE_YES;
		self->clipmask = MASK_MONSTERSOLID;
		self->nextthink = -1.0f;
		self->svflags |= SVF_DEADMONSTER;
		self->health = 25;
		self->die = HarpyHeadDie;

		vec3_t forward;
		AngleVectors(self->s.angles, forward, NULL, NULL);
		VectorScale(forward, 100.0f, self->velocity);

		VectorSet(self->mins, -4.0f, -4.0f, -4.0f);
		VectorSet(self->maxs,  4.0f,  4.0f,  4.0f);

		gi.linkentity(self);
	}
	else
	{
		VectorCopy(self->owner->s.angles, self->s.angles);
		VectorCopy(self->owner->s.origin, self->s.origin);

		vec3_t down;
		AngleVectors(self->s.angles, NULL, NULL, down);
		Vec3ScaleAssign(-1.0f, down);

		VectorMA(self->s.origin, self->count, down, self->s.origin);

		self->nextthink = level.time + FRAMETIME; //mxd. Use define.
	}
}

void harpy_take_head(edict_t* self, edict_t* victim, const int bodypart_node_id, const int frame, const int flags) //TODO: rename to HarpyTakeHead.
{
	edict_t* head = G_Spawn();

	head->s.effects |= EF_CAMERA_NO_CLIP;
	head->svflags |= SVF_ALWAYS_SEND;
	head->solid = SOLID_NOT;
	head->movetype = PHYSICSTYPE_NOCLIP;
	head->gravity = 0.0f;
	head->clipmask = 0;
	head->materialtype = victim->materialtype;

	head->owner = self;
	head->activator = victim;

	VectorCopy(self->s.angles, head->s.angles);
	VectorCopy(self->s.origin, head->s.origin);

	vec3_t forward;
	vec3_t down;
	AngleVectors(head->s.angles, forward, NULL, down);
	Vec3ScaleAssign(-1.0f, down);

	head->count = 8; //TODO: add float harpy_head_offset name.
	VectorMA(head->s.origin, head->count, down, head->s.origin);

	head->s.origin[2] += 100.0f;

	gi.CreateEffect(&head->s, FX_BODYPART, flags, head->s.origin, "ssbbb", (short)frame, (short)bodypart_node_id, 0, victim->s.modelindex, victim->s.number);

	head->think = HarpyHeadThink;
	head->nextthink = level.time + FRAMETIME; //mxd. Use define.

	gi.linkentity(head);

	give_head_to_harpy = NULL;
	take_head_from = NULL;

	VectorScale(forward, 200.0f, self->velocity);
	self->velocity[2] = 20.0f; //FIXME: fix angles?
	self->enemy = NULL;

	QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL); //FIXME: go into a circle?
}

static void HarpyIsBlocked(edict_t* self, trace_t* trace) //mxd. Named 'harpy_blocked' in original logic.
{
	if (self->enemy == NULL && (self->spawnflags & MSF_SPECIAL1))
	{
		SetAnim(self, ANIM_CIRCLING_FLAP);
		return;
	}

	if (self->health <= 0 || trace->ent == NULL)
		return;

	vec3_t forward;
	AngleVectors(self->s.angles, forward, NULL, NULL);

	vec3_t dir;
	VectorCopy(self->velocity, dir);
	VectorNormalize(dir);

	const float dot = DotProduct(dir, forward);

	if (trace->ent->takedamage != DAMAGE_NO && (self->curAnimID == ANIM_DIVE_GO || self->curAnimID == ANIM_DIVE_LOOP || self->curAnimID == ANIM_DIVE_END || self->curAnimID == ANIM_HIT_LOOP))
	{
		if (trace->ent->client != NULL || classStatics[trace->ent->classID].msgReceivers[MSG_DISMEMBER] != NULL)
		{
			if (trace->ent->health < HARPY_DMG_MAX && trace->ent->s.origin[2] < self->s.origin[2])
			{
				// Also make this skill-dependant.
				give_head_to_harpy = self;
				take_head_from = trace->ent;

				if (trace->ent->client != NULL)
				{
					trace->ent->health = 1;
					player_decap(trace->ent, self);
				}
				else
				{
					QPostMessage(trace->ent, MSG_DISMEMBER, PRI_DIRECTIVE, "ii", 9999999, hl_Head | hl_MeleeHit);
				}

				return;
			}
		}

		const int damage = irand(HARPY_DMG_MIN, HARPY_DMG_MAX);
		T_Damage(trace->ent, self, self, dir, trace->ent->s.origin, trace->plane.normal, damage, damage * 2, 0, MOD_DIED);

		// 16% chance to knock down player.
		if (trace->ent->health > 0 && trace->ent->client != NULL && irand(0, 5) == 0 && trace->ent->client->playerinfo.lowerseq != ASEQ_KNOCKDOWN)
			P_KnockDownPlayer(&trace->ent->client->playerinfo);

		SetAnim(self, ANIM_FLYBACK1);
	}
	else if (self->damage_debounce_time < level.time || dot > 0.0f)
	{
		// Only back up from a block once every 2 seconds.
		self->damage_debounce_time = level.time + 2.0f;
		SetAnim(self, ANIM_FLYBACK1);
	}
	else
	{
		SetAnim(self, ANIM_FLY1);
	}
}

//TODO: replace these with generic harpy_sound() (copy gkrokon_sound())?
void harpy_flap_noise(edict_t* self)
{
	gi.sound(self, CHAN_BODY, sounds[SND_FLAP], 1.0f, ATTN_NORM, 0.0f);
}

void harpy_flap_fast_noise(edict_t* self)
{
	gi.sound(self, CHAN_BODY, sounds[SND_FLAP_FAST], 1.0f, ATTN_NORM, 0.0f);
}

void harpy_death_noise(edict_t* self)
{
	gi.sound(self, CHAN_BODY, sounds[SND_DEATH], 1.0f, ATTN_NORM, 0.0f);
}

void harpy_pain1_noise(edict_t* self)
{
	gi.sound(self, CHAN_BODY, sounds[SND_PAIN1], 1.0f, ATTN_NORM, 0.0f);
}

void harpy_pain2_noise(edict_t* self)
{
	gi.sound(self, CHAN_BODY, sounds[SND_PAIN2], 1.0f, ATTN_NORM, 0.0f);
}

void harpy_attack_noise(edict_t* self)
{
	gi.sound(self, CHAN_BODY, sounds[SND_ATTACK], 1.0f, ATTN_NORM, 0.0f);
}

void harpy_dive_noise(edict_t* self)
{
	gi.sound(self, CHAN_BODY, sounds[SND_DIVE], 1.0f, ATTN_NORM, 0.0f);
}

static qboolean HarpyCanMove(const edict_t* self, const float distance) //mxd. Named 'harpy_check_move' in original logic.
{
	vec3_t end_pos;
	VectorCopy(self->s.origin, end_pos);

	vec3_t forward;
	AngleVectors(self->s.angles, forward, NULL, NULL);
	VectorMA(end_pos, distance, forward, end_pos);

	trace_t trace;
	gi.trace(self->s.origin, self->mins, self->maxs, end_pos, self, MASK_SHOT | MASK_WATER, &trace);

	if ((trace.fraction < 1.0f || trace.allsolid || trace.startsolid) && trace.ent != self->enemy)
		return false;

	return true;
}

void harpy_ai_circle(edict_t* self, float forward_offset, float right_offset, float up_offset)
{
#define HARPY_CIRCLE_AMOUNT	4.0f
#define HARPY_CIRCLE_SPEED  64.0f

	self->s.angles[ROLL] += flrand(-1.25f, 1.0f);
	self->s.angles[ROLL] = Clamp(self->s.angles[ROLL], -45.0f, 0.0f);

	self->s.angles[YAW] = anglemod(self->s.angles[YAW] - (HARPY_CIRCLE_AMOUNT + (forward_offset - 32.0f) / 4.0f));

	vec3_t forward;
	AngleVectors(self->s.angles, forward, NULL, NULL);
	VectorMA(self->velocity, HARPY_CIRCLE_SPEED + forward_offset, forward, self->velocity);
	Vec3ScaleAssign(0.5f, self->velocity);

	if (irand(0, 150) == 0)
		gi.sound(self, CHAN_VOICE, sounds[SND_SCREAM], 1.0f, ATTN_NORM, 0.0f);
}

// Replaces ai_walk and ai_run for harpy.
void harpy_ai_glide(edict_t* self, float forward_offset, float right_offset, float up_offset)
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

void harpy_ai_fly(edict_t* self, float forward_offset, float right_offset, float up_offset)
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

	if (!HarpyCanMove(self, forward_offset / 10.0f))
	{
		SetAnim(self, ANIM_HOVER1);
		return;
	}

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

// Replaces ai_stand for harpy.
void harpy_ai_hover(edict_t* self, float distance)
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

	harpy_ai_glide(self, 0.0f, 0.0f, 0.0f);
}

void harpy_flyback(edict_t* self)
{
	SetAnim(self, ANIM_FLYBACK1);
}

void harpy_ai_pirch(edict_t* self) //TODO: rename to harpy_ai_perch.
{
	if (!M_ValidTarget(self, self->enemy) || !AI_IsVisible(self, self->enemy))
		return;

	vec3_t diff;
	VectorSubtract(self->enemy->s.origin, self->s.origin, diff);
	const float dist = VectorNormalize(diff);

	if (dist < 150.0f)
	{
		SetAnim(self, ANIM_TAKEOFF);
		return;
	}

	if (irand(0, 100) < 10 && self->monsterinfo.attack_finished < level.time)
	{
		self->monsterinfo.attack_finished = level.time + 5.0f;
		gi.sound(self, CHAN_WEAPON, sounds[irand(SND_IDLE1, SND_IDLE2)], 1.0f, ATTN_NORM, 0.0f);
	}

	vec3_t forward;
	vec3_t right;
	AngleVectors(self->s.angles, forward, right, NULL);

	if (DotProduct(diff, forward) < 0.0f)
	{
		SetAnim(self, ANIM_TAKEOFF);
		return;
	}

	const float right_dot = DotProduct(diff, right);

	if (right_dot < 0.0f) // Left.
	{
		if (right_dot < -0.8f)
			SetAnim(self, ANIM_PIRCH9);
		else if (right_dot < -0.6f)
			SetAnim(self, ANIM_PIRCH8);
		else if (right_dot < -0.4f)
			SetAnim(self, ANIM_PIRCH7);
		else if (right_dot < -0.2f)
			SetAnim(self, ANIM_PIRCH6);
		else
			SetAnim(self, ANIM_PIRCH5);
	}
	else // Right.
	{
		if (right_dot > 0.8f)
			SetAnim(self, ANIM_PIRCH1);
		else if (right_dot > 0.6f)
			SetAnim(self, ANIM_PIRCH2);
		else if (right_dot > 0.4f)
			SetAnim(self, ANIM_PIRCH3);
		else if (right_dot > 0.2f)
			SetAnim(self, ANIM_PIRCH4);
		else
			SetAnim(self, ANIM_PIRCH5);
	}
}

void move_harpy_tumble(edict_t* self) //TODO: rename to harpy_tumble_move.
{
	self->movetype = PHYSICSTYPE_STEP;
	self->gravity = 1.0f;

	VectorCopy(dead_harpy_mins, self->mins); //mxd
	VectorCopy(dead_harpy_maxs, self->maxs); //mxd

	vec3_t end_pos;
	VectorCopy(self->s.origin, end_pos);
	end_pos[2] -= 32.0f;

	trace_t trace;
	gi.trace(self->s.origin, self->mins, self->maxs, end_pos, self, MASK_MONSTERSOLID, &trace);

	if (self->groundentity != NULL || trace.fraction < 1.0f || trace.startsolid || trace.allsolid || self->monsterinfo.jump_time < level.time)
	{
		gi.CreateEffect(&self->s, FX_DUST_PUFF, CEF_OWNERS_ORIGIN, self->s.origin, NULL);

		VectorCopy(self->s.angles, self->movedir);
		harpy_death_noise(self);

		SetAnim(self, ANIM_DIE);
	}
}

void harpy_fix_angles(edict_t* self)
{
	// Pitch.
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

	// Roll.
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

static void HarpyDeathPainMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'harpy_dead_pain' in original logic.
{
	if (self->health <= -40) // Gib death.
	{
		BecomeDebris(self);
		self->think = NULL;
		self->nextthink = 0.0f;

		gi.linkentity(self);
	}
	else if (msg != NULL)
	{
		DismemberMsgHandler(self, msg);
	}
}

static void HarpyDeathMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'harpy_die' in original logic.
{
	if (self->monsterinfo.aiflags & AI_DONT_THINK)
	{
		SetAnim(self, ANIM_DIE);
		return;
	}

	self->movetype = PHYSICSTYPE_STEP;
	self->gravity = 1.0f;
	self->elasticity = 1.1f;

	VectorCopy(dead_harpy_mins, self->mins); //mxd
	VectorCopy(dead_harpy_maxs, self->maxs); //mxd

	if (self->health <= -40) // Gib death.
	{
		gi.sound(self, CHAN_BODY, sounds[SND_GIB], 1.0f, ATTN_NORM, 0.0f);

		BecomeDebris(self);
		gi.linkentity(self);
	}
	else
	{
		self->msgHandler = DeadMsgHandler;

		if (irand(0, 1) == 1)
			self->svflags &= ~SVF_TAKE_NO_IMPACT_DMG;

		SetAnim(self, ANIM_DIE);
	}
}

static void HarpyDismember(edict_t* self, int damage, HitLocation_t hl) //mxd. Named 'harpy_dismember' in original logic.
{
	static const int bones_per_node_for_hitloc[hl_harpy_max] = //mxd. Made local static.
	{
		0,
		BPN_BACKSPIKES,						// hl_backspikes
		BPN_HEAD | BPN_HORNS | BPN_HORN | BPN_NECKSPIKES, // hl_head
		BPN_STINGER,						// hl_stinger
		BPN_LWING,							// hl_lwing
		BPN_LHAND,							// hl_lefthand
		BPN_RWING,							// hl_rwing
		BPN_RHAND,							// hl_righthand
		BPN_LUARM | BPN_LLARM | BPN_LHAND,	// hl_leftupperleg
		BPN_LLARM | BPN_LHAND,				// hl_leftlowerleg
		BPN_RUARM | BPN_RLARM | BPN_RHAND,	// hl_rightupperleg
		BPN_RLARM | BPN_RHAND				// hl_rightlowerleg
	};

	static const int mesh_for_hitloc[hl_harpy_max] = //mxd. Made local static.
	{
		0,
		MESH_BACKSPIKES,	// hl_backspikes
		MESH_HEAD,			// hl_head
		MESH_STINGER,		// hl_stinger
		MESH_LWING,			// hl_lwing
		MESH_LHAND,			// hl_lefthand
		MESH_RWING,			// hl_rwing
		MESH_RHAND,			// hl_righthand
		MESH_LUARM,			// hl_leftupperleg
		MESH_LLARM,			// hl_leftlowerleg
		MESH_RUARM,			// hl_rightupperleg
		MESH_RLARM			// hl_rightlowerleg
	};

	qboolean dismember_ok = false;

	if (hl & hl_MeleeHit)
	{
		dismember_ok = true;
		hl &= ~hl_MeleeHit;
	}

	if (hl < hl_NoneSpecific || hl >= hl_WingedPoints) //mxd. Added lower bound check. //BUGFIX: mxd. 'if (hl > hl_WingedPoints)' in original logic. Will cause mesh_for_hitloc[] overrun when hl == hl_WingedPoints.
		return;

	if (hl == hl_backspikes)
		hl = irand(hl_lwing, hl_rwing);

	switch (hl)
	{
		case hl_head:
			if ((self->s.fmnodeinfo[MESH_HEAD].flags & FMNI_NO_DRAW) || irand(0, 10) > 2)
				dismember_ok = false;
			break;

		case hl_rightlowerleg:
		case hl_rightupperleg:
			if ((self->s.fmnodeinfo[MESH_RUARM].flags & FMNI_NO_DRAW) || irand(0, 10) > 4)
				dismember_ok = false;
			break;

		case hl_leftlowerleg:
		case hl_leftupperleg:
			if ((self->s.fmnodeinfo[MESH_LUARM].flags & FMNI_NO_DRAW) || irand(0, 10) > 4)
				dismember_ok = false;
			break;

		case hl_rwing:
			if ((self->s.fmnodeinfo[MESH_RWING].flags & FMNI_NO_DRAW) || irand(0, 10) > 6)
				dismember_ok = false;
			break;

		case hl_lwing:
			if ((self->s.fmnodeinfo[MESH_LWING].flags & FMNI_NO_DRAW) || irand(0, 10) > 6)
				dismember_ok = false;
			break;

		default:
			dismember_ok = false;
			break;
	}

	const int mesh_loc = mesh_for_hitloc[hl];

	if (dismember_ok)
	{
		const vec3_t gore_spot = { 0.0f, 0.0f, 10.0f };
		const int throw_nodes = bones_per_node_for_hitloc[hl];

		ThrowBodyPart(self, &gore_spot, throw_nodes, (float)damage, FRAME_partfly1);

		switch (mesh_loc)
		{
			case hl_head:
				self->s.fmnodeinfo[MESH_HEAD].flags |= FMNI_NO_DRAW;
				self->s.fmnodeinfo[MESH_HORN].flags |= FMNI_NO_DRAW;
				self->s.fmnodeinfo[MESH_HORNS].flags |= FMNI_NO_DRAW;
				break;

			case hl_leftlowerleg:
			case hl_leftupperleg:
				self->s.fmnodeinfo[MESH_LUARM].flags |= FMNI_NO_DRAW;
				self->s.fmnodeinfo[MESH_LLARM].flags |= FMNI_NO_DRAW;
				self->s.fmnodeinfo[MESH_LHAND].flags |= FMNI_NO_DRAW;
				break;

			case hl_rightlowerleg:
			case hl_rightupperleg:
				self->s.fmnodeinfo[MESH_RUARM].flags |= FMNI_NO_DRAW;
				self->s.fmnodeinfo[MESH_RLARM].flags |= FMNI_NO_DRAW;
				self->s.fmnodeinfo[MESH_RHAND].flags |= FMNI_NO_DRAW;
				break;

			default:
				self->s.fmnodeinfo[mesh_loc].flags |= FMNI_NO_DRAW;
				break;
		}

		if (hl == hl_rwing || hl == hl_lwing || hl == hl_head)
		{
			self->monsterinfo.jump_time = level.time + 2.0f;

			if (self->health > 0)
			{
				self->health = -1;
				harpy_death_noise(self);
				SetAnim(self, ANIM_TUMBLE);
				self->msgHandler = DeadMsgHandler;
			}
		}
	}
	else if (irand(0, 10) == 0 && !(self->s.fmnodeinfo[mesh_loc].flags & FMNI_USE_SKIN))
	{
		self->s.fmnodeinfo[mesh_loc].flags |= FMNI_USE_SKIN;
		self->s.fmnodeinfo[mesh_loc].skin = self->s.skinnum + 1;
	}
}

static void HarpyPainMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'harpy_pain' in original logic.
{
	int temp;
	int damage;
	qboolean force_pain;
	ParseMsgParms(msg, "eeiii", &temp, &temp, &force_pain, &damage, &temp);

	if (self->curAnimID >= ANIM_PIRCH1 && self->curAnimID <= ANIM_PIRCH9)
	{
		SetAnim(self, ANIM_TAKEOFF);
	}
	else if (force_pain || (irand(0, 10) < 2 && self->pain_debounce_time < level.time))
	{
		if (irand(0, 1) == 1)
			harpy_pain1_noise(self);
		else
			harpy_pain2_noise(self);

		self->pain_debounce_time = level.time + 2.0f;
		SetAnim(self, ANIM_PAIN1);
	}
}

// Receiver for MSG_STAND, MSG_RUN and MSG_FLY. 
static void HarpyFlyMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'harpy_hover' in original logic.
{
	if (self->spawnflags & (MSF_PERCHING | MSF_SPECIAL1))
		return;

	if (irand(1, 10) > 3)
	{
		SetAnim(self, ANIM_HOVER1);
	}
	else
	{
		gi.sound(self, CHAN_BODY, sounds[SND_SCREAM], 1.0f, ATTN_NORM, 0.0f);
		SetAnim(self, ANIM_HOVERSCREAM);
	}
}

static void HarpyEvadeMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'harpy_evade' in original logic.
{
	if (self->curAnimID > ANIM_PIRCH1 && self->curAnimID < ANIM_PIRCH9)
	{
		self->mins[2] -= 4.0f;
		SetAnim(self, ANIM_TAKEOFF);
	}
}

static void HarpyWatchMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'harpy_perch' in original logic.
{
	SetAnim(self, ANIM_PIRCH5);
}

void harpy_hit(edict_t *self)
{
	trace_t	trace;
	edict_t	*victim;
	vec3_t	vf;//, hitPos, mins, maxs;
	float	movedist, damage;

	AngleVectors(self->s.angles, vf, NULL, NULL);
	movedist = VectorLength(self->velocity);

	victim = M_CheckMeleeHit( self, movedist, &trace);

	if (victim)
	{
		if (victim == self)
		{
			SetAnim(self, ANIM_FLYBACK1);
		}
		else
		{
			damage = irand(HARPY_DMG_MIN, HARPY_DMG_MAX);
			T_Damage (victim, self, self, vf, self->enemy->s.origin, trace.plane.normal, damage, damage*2, 0,MOD_DIED);
		}
	}
}

void harpy_pause (edict_t *self)
{
	if (M_ValidTarget(self, self->enemy))
		QPostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
	else if(self->curAnimID == ANIM_CIRCLING)
	{
		if(!irand(0, 6))
			SetAnim(self, ANIM_CIRCLING_FLAP);
	}
	else if(self->curAnimID == ANIM_CIRCLING_FLAP && irand(0, 1))
		SetAnim(self, ANIM_CIRCLING);
}


//end of anim func for death anim
void harpy_dead(edict_t *self)
{
	VectorSet(self->mins, -16, -16, 0);
	VectorSet(self->maxs, 16, 16, 12);

	M_EndDeath(self);
}

qboolean harpy_check_directions(edict_t *self, vec3_t goal, vec3_t vf, vec3_t vr, vec3_t vu, float checkdist, vec3_t ret)
{
	trace_t	trace;
	vec3_t	goalpos;

	//Don't always check one direction first (looks mechanical)
	if (irand(0,1))
		VectorScale(vr, -1, vr);

	VectorMA(self->s.origin, checkdist, vr, goalpos);

	gi.trace(goalpos, self->mins, self->maxs, goal, self, MASK_SHOT|MASK_WATER,&trace);

	//We've found somewhere to go
	if (trace.ent == self->enemy)
	{
		VectorCopy(vr, ret);
		return true;
	}
	else  //Check the other directions
	{
		VectorScale(vr, -1, vr);
		VectorMA(goalpos, checkdist, vr, goalpos);

		gi.trace(goalpos, self->mins, self->maxs, goal, self, MASK_SHOT|MASK_WATER,&trace);

		if (trace.ent == self->enemy)
		{
			VectorCopy(vr, ret);
			return true;
		}
	}
	
	//Check up and down
	VectorCopy(self->s.origin, goalpos);

	//Don't always check one direction first (looks mechanical)
	if (irand(0,1))
		VectorScale(vu, -1, vu);

	VectorMA(goalpos, checkdist, vu, goalpos);

	gi.trace(goalpos, self->mins, self->maxs, goal, self, MASK_SHOT|MASK_WATER,&trace);

	//We've found somewhere to go
	if (trace.ent == self->enemy)
	{
		VectorCopy(vu, ret);
		return true;
	}
	else  //Check the other directions
	{
		VectorScale(vu, -1, vu);
		VectorMA(goalpos, checkdist, vu, goalpos);

		gi.trace(goalpos, self->mins, self->maxs, goal, self, MASK_SHOT|MASK_WATER,&trace);

		if (trace.ent == self->enemy)
		{
			VectorCopy(vu, ret);
			return true;
		}
	}
		
	//Check forward and back
	VectorCopy(self->s.origin, goalpos);

	//Don't always check one direction first (looks mechanical)
	if (irand(0,1))
		VectorScale(vf, -1, vf);

	VectorMA(goalpos, checkdist, vf, goalpos);

	gi.trace(goalpos, self->mins, self->maxs, goal, self, MASK_SHOT|MASK_WATER,&trace);

	//We've found somewhere to go
	if (trace.ent == self->enemy)
	{
		VectorCopy(vf, ret);
		return true;
	}
	else  //Check the other directions
	{
		VectorScale(vf, -1, vf);
		VectorMA(goalpos, checkdist, vf, goalpos);

		gi.trace(goalpos, self->mins, self->maxs, goal, self, MASK_SHOT|MASK_WATER,&trace);

		if (trace.ent == self->enemy)
		{
			VectorCopy(vf, ret);
			return true;
		}
	}

	return false;
}

qboolean harpy_check_swoop(edict_t *self, vec3_t goal)
{
	trace_t	trace;
	vec3_t	checkpos;
	float	zd;

	//Find the difference in the target's height and the creature's height
	zd = Q_fabs(self->enemy->s.origin[2] - self->s.origin[2]);
	
	if (zd < HARPY_MIN_SWOOP_DIST)
		return false;

	zd -= zd/4;

	VectorCopy(self->s.origin, checkpos);
	checkpos[2] -= zd;

	//Trace down about that far and about one forth the distance to the target
	gi.trace(self->s.origin, self->mins, self->maxs, checkpos, self, MASK_SHOT|MASK_WATER,&trace);

	if (trace.fraction < 1 || trace.startsolid || trace.allsolid)
	{
		//gi.dprintf("harpy_check_swoop: failed down check\n");
		return false;
	}

	//Trace straight to the target

	gi.trace(checkpos, self->mins, self->maxs, goal, self, MASK_SHOT|MASK_WATER,&trace);

	if (trace.ent != self->enemy)
	{
		//gi.dprintf("harpy_check_swoop: failed out check\n");
		return false;
	}

	//There's a clear path
	return true;
}

void move_harpy_dive(edict_t *self)
{
	vec3_t	vec, vf, enemy_pos;
	float	dist, zd, hd, forward;

	VectorSet(enemy_pos, self->enemy->s.origin[0], self->enemy->s.origin[1], self->enemy->s.origin[2] + flrand(self->maxs[2], self->enemy->maxs[2]));
	//Find out the Z and Horizontal deltas to target
	zd = Q_fabs(self->s.origin[2] - enemy_pos[2]);
	
	AngleVectors(self->s.angles, vf, NULL, NULL);

	VectorCopy(self->s.origin, vec);
	vec[2] = enemy_pos[2];

	VectorSubtract(enemy_pos, vec, vec);
	hd = VectorLength(vec);

	if ((self->groundentity != NULL) || (!HarpyCanMove(self, 64)))
	{
		if (self->groundentity == self->enemy)
			SetAnim(self, ANIM_DIVE_END);

		SetAnim(self, ANIM_FLYBACK1);
		return;
	}

	dist = Q_fabs(self->s.origin[2] - enemy_pos[2]);

	forward = (256 - (dist*0.85));

	if (forward > 256)
		forward = 256;
	else if (forward < 0)
		forward = 0;

	if (dist > HARPY_MAX_DIVE_DIST)
	{
		VectorMA(vf, forward, vf, self->velocity);
		self->velocity[2] = -dist*2.25;
		if (self->velocity[2] < -300)
			self->velocity[2] = -300;
	}
	else
	{
		SetAnim(self, ANIM_DIVE_TRANS);
		return;
	}

	harpy_ai_glide(self, 0, 0, 0);
}

void move_harpy_dive_end(edict_t *self)
{
	vec3_t	vec, vf, vr, vu, nvec, enemy_pos;
	float	dist, hd, fd, dot;
	
	VectorSet(enemy_pos, self->enemy->s.origin[0], self->enemy->s.origin[1], self->enemy->s.origin[2] + flrand(self->maxs[2], self->enemy->maxs[2]));

	VectorCopy(self->s.origin, vec);
	vec[2] = enemy_pos[2];

	VectorSubtract(enemy_pos, vec, vec);
	hd = VectorLength(vec);
	self->ideal_yaw = VectorYaw(vec);

	M_ChangeYaw(self);

	AngleVectors(self->s.angles, vf, vr, vu);
	
	self->velocity[2] *= 0.75;

	self->monsterinfo.jump_time *= HARPY_SWOOP_INCREMENT;

	fd = self->monsterinfo.jump_time;

	if (fd > HARPY_MAX_SWOOP_SPEED)
		fd = HARPY_MAX_SWOOP_SPEED;

	if ((self->groundentity != NULL) || (!HarpyCanMove(self, 128)))
	{
		if (self->groundentity == self->enemy)
			SetAnim(self, ANIM_DIVE_END);

		SetAnim(self, ANIM_FLYBACK1);
		return;
	}

	VectorSubtract(enemy_pos, self->s.origin, vec);
	VectorCopy(vec, nvec);
	VectorNormalize(nvec);

	AngleVectors(self->s.angles, vf, vr, NULL);

	dot  = DotProduct(vf, nvec);

	if (dot < -0.5)
	{
		SetAnim(self, ANIM_FLYBACK1);
		return;
	}

	VectorMA(self->velocity, fd, vf, self->velocity);
	
	//Are we about to hit the target?
	VectorSubtract(enemy_pos, self->s.origin, vec);
	dist = VectorLength(vec);

	if (dist < HARPY_COLLISION_DIST)
	{
		SetAnim(self, ANIM_DIVE_END);
		return;
	}	

	harpy_ai_glide(self, 0, 0, 0);
}

void harpy_dive_loop(edict_t *self)
{
	SetAnim(self, ANIM_DIVE_LOOP);
}

void harpy_hit_loop(edict_t *self)
{
	SetAnim(self, ANIM_HIT_LOOP);
}

void harpy_check_dodge(edict_t *self)
{
	qboolean	dodge = false;
	trace_t		trace;
	edict_t		*ent = NULL;
	vec3_t		vec, vr, projvec, dodgedir, goalpos;
	float		dodgedot;

	if (!self->enemy)
		return;

	VectorSubtract(self->enemy->s.origin, self->s.origin, vec);
	VectorNormalize(vec);

	while ((ent = FindInRadius(ent, self->s.origin, HARPY_PROJECTILE_SEARCH_RADIUS)) != NULL)
	{
		//We're only interested in his projectiles
		if (ent->owner != self->enemy)
			continue;
		
		//VectorCopy(ent->velocity, projvec);
		VectorNormalize2(ent->velocity, projvec);

		dodgedot = DotProduct(projvec, vec);

		//gi.dprintf("Found projectile with dot %f\n", dodgedot);

		if (dodgedot < -0.85 && irand(0,1))
		{
			//gi.dprintf("Dodge it!\n");

			dodge = true;
			AngleVectors(self->s.angles, NULL, vr, NULL);

			if (irand(0,1))
				VectorScale(vr, -1, vr);

			VectorMA(self->s.origin, 100, vr, goalpos);

			gi.trace(self->s.origin, self->mins, self->maxs, goalpos, self, MASK_SHOT|MASK_WATER,&trace);

			if (trace.fraction < 1 || trace.startsolid || trace.allsolid)
				VectorScale(vr, -1, dodgedir);
			else
				VectorCopy(vr, dodgedir);
		}
	}

	if (dodge)
	{
		//If he is, dodge!
		if (self->monsterinfo.misc_debounce_time < level.time)
		{
			Vec3ScaleAssign(flrand(300,500), dodgedir);
			VectorAdd(dodgedir, self->velocity, self->velocity);
			self->monsterinfo.misc_debounce_time = level.time + irand(2,4);
		}
	}	
	
	harpy_ai_glide(self, 0, 0, 0);
}

void move_harpy_hover(edict_t *self)
{
	qboolean	canmove = false, dodge = false;
	trace_t		trace;
	edict_t		*ent = NULL;
	vec3_t		goal, dodgedir, mins, maxs, vf, vr, vu, vec, projvec, goalpos;
	float		dist, zd, dodgedot;
	
	//gi.dprintf("move_harpy_hover: entered function\n");

	if (!self->enemy)
	{
		if (!FindTarget(self))
		{
			//gi.dprintf("move_harpy_hover: Enemy lost\n");
			return;
		}
	}

	//First check to see that the player is at least 128 units away in (discounting z height)
	VectorCopy(self->enemy->s.origin, goal);
	goal[2] = self->s.origin[2];

	VectorSubtract(goal, self->s.origin, goal);
	dist = VectorLength(goal);
	
	//Face target
	self->ideal_yaw = VectorYaw(goal);
	M_ChangeYaw(self);

	//If he is...
	if (dist > HARPY_MIN_HOVER_DIST && dist < HARPY_MAX_HOVER_DIST)
	{
		//gi.dprintf("move_harpy_hover: valid player distance\n");

		//Make sure we've got line of sight
		VectorSet(mins, -1, -1, -1);
		VectorSet(maxs, 1, 1, 1);

		gi.trace(self->s.origin, mins, maxs, self->enemy->s.origin, self, MASK_SHOT|MASK_WATER,&trace);

		//If not, try looking from a bit to the side in all six directions
		if (trace.ent != self->enemy)
		{
			//gi.dprintf("move_harpy_hover: lost line of sight to player\n");
			
			//Setup the directions
			AngleVectors(self->s.angles, vf, vr, vu);

			canmove = harpy_check_directions(self, self->enemy->s.origin, vf, vr, vu, HARPY_CHECK_DIST, goal);
			
			//If we can see him from one of these, go there
			if (canmove)
			{
				//gi.dprintf("move_harpy_hover: new position found, moving...\n");
				VectorMA(self->velocity, flrand(300.0F, 400.0F), goal, self->velocity);
				return;
			}

			//gi.dprintf("move_harpy_hover: no new direction found, bumping about\n");
			
			//Otherwise just flap around and wait, perhaps lower yourself a bit if high up
			self->velocity[0] = flrand(-HARPY_DRIFT_AMOUNT_X, HARPY_DRIFT_AMOUNT_X);
			self->velocity[1] = flrand(-HARPY_DRIFT_AMOUNT_Y, HARPY_DRIFT_AMOUNT_Y);
			self->velocity[2] = flrand(-HARPY_DRIFT_AMOUNT_Z, HARPY_DRIFT_AMOUNT_Z);

			return;
		}
		else
		{
			//Check to make sure the player isn't shooting anything

			//This won't change over the calculations
			VectorSubtract(self->enemy->s.origin, self->s.origin, vec);
			VectorNormalize(vec);

			while ((ent = FindInRadius(ent, self->s.origin, HARPY_PROJECTILE_SEARCH_RADIUS)) != NULL)
			{
				//We're only interested in his projectiles
				if (ent->owner != self->enemy)
					continue;
				
				VectorCopy(ent->velocity, projvec);
				VectorNormalize(projvec);

				dodgedot = DotProduct(projvec, vec);

				//gi.dprintf("Found projectile with dot %f\n", dodgedot);

				if (dodgedot < -0.6)
				{
					//gi.dprintf("Dodge it!\n");

					dodge = true;
					AngleVectors(self->s.angles, NULL, vr, NULL);

					if (irand(0,1))
						VectorScale(vr, -1, vr);

					VectorMA(self->s.origin, 100, vr, goalpos);

					gi.trace(self->s.origin, self->mins, self->maxs, goalpos, self, MASK_SHOT|MASK_WATER,&trace);

					if (trace.fraction < 1 || trace.startsolid || trace.allsolid)
						VectorScale(vr, -1, vr);

					VectorCopy(vr, dodgedir);
				}
			}

			if (dodge)
			{
				//If he is, dodge!
				VectorMA(self->velocity, irand(300, 500), dodgedir, self->velocity);
				return;
			}

			//If nothing is happening, check to swoop
			canmove = harpy_check_swoop(self, self->enemy->s.origin);

			//If you can--nail um
			if (canmove)
			{
				//gi.dprintf("move_harpy_hover: valid swoop\n");
				self->monsterinfo.jump_time = 2;
				SetAnim(self, ANIM_DIVE_GO);

				return;
			}

			//If not, check to see if there's somewhere that you can get to that will allow it
			//FIXME: Too many checks.. just try something simple

			//If all else fails, then just pick a random direction to nudge yourself to
			else
			{
				//gi.dprintf("move_harpy_hover: swoop worthless\n");
				
				//Find the difference in the target's height and the creature's height
				zd = Q_fabs(self->enemy->s.origin[2] - self->s.origin[2]);
		
				//We can't swoop because we're too low, so fly upwards if possible
				if (zd < HARPY_MIN_SWOOP_DIST)
				{
					if (!HarpyCanMove(self, -64))
					{
						SetAnim(self, ANIM_FLY1);
						return;
					}
					else
					{
						//gi.dprintf("Moveback ok\n");
						SetAnim(self, ANIM_FLYBACK1);
						return;
					}
				}
				else
				{
					//Otherwise just flap around and wait, perhaps lower yourself a bit if high up					
					self->velocity[0] = flrand(-HARPY_DRIFT_AMOUNT_X, HARPY_DRIFT_AMOUNT_X);
					self->velocity[1] = flrand(-HARPY_DRIFT_AMOUNT_Y, HARPY_DRIFT_AMOUNT_Y);
					self->velocity[2] = flrand(-HARPY_DRIFT_AMOUNT_Z, HARPY_DRIFT_AMOUNT_Z);

					AngleVectors(self->s.angles, vec, NULL, NULL);
					VectorMA(self->velocity, irand(200,300), vec, self->velocity);
				}

				return;
			}

		}

		//If he's too far away trace a line (expanded) to see if you can move at him
	}
	else if (dist < HARPY_MIN_HOVER_DIST)
	{
		//gi.dprintf("move_harpy_hover: backing away\n");
		if (!HarpyCanMove(self, -64))
		{
			SetAnim(self, ANIM_FLY1);
		}
		else
		{
			SetAnim(self, ANIM_FLYBACK1);
		}
	}
	else
	{
		//gi.dprintf("move_harpy_hover: covering ground\n");
		if (!HarpyCanMove(self, 64))
		{
			SetAnim(self, ANIM_FLYBACK1);
		}
		else
		{
			SetAnim(self, ANIM_FLY1);
		}
	}

	return;
} 

//New physics call that modifies the harpy's velocity and angles based on aerodynamics
void harpy_flight_model(edict_t *self)
{
}

void move_harpy_fly(edict_t *self)
{	
	edict_t *dummy;
	dummy = self;
	
	return;
}

void move_harpy_die(edict_t *self)
{
	//fall to the floor
	return;
}

void harpy_hover_anim(edict_t *self)
{
	SetAnim(self, ANIM_HOVER1);
}

/*===============================================================

	Harpy Spawn Functions

===============================================================*/

void HarpyStaticsInit(void)
{
	static ClassResourceInfo_t resInfo;

	classStatics[CID_HARPY].msgReceivers[MSG_DEATH] = HarpyDeathMsgHandler;
	classStatics[CID_HARPY].msgReceivers[MSG_FLY] = HarpyFlyMsgHandler;
	classStatics[CID_HARPY].msgReceivers[MSG_STAND] = HarpyFlyMsgHandler;
	classStatics[CID_HARPY].msgReceivers[MSG_RUN] = HarpyFlyMsgHandler;
	classStatics[CID_HARPY].msgReceivers[MSG_PAIN] = HarpyPainMsgHandler;
	classStatics[CID_HARPY].msgReceivers[MSG_DISMEMBER] = DismemberMsgHandler;
	classStatics[CID_HARPY].msgReceivers[MSG_WATCH] = HarpyWatchMsgHandler;
	classStatics[CID_HARPY].msgReceivers[MSG_DEATH_PAIN] = HarpyDeathPainMsgHandler;
	classStatics[CID_HARPY].msgReceivers[MSG_EVADE] = HarpyEvadeMsgHandler;

	resInfo.numAnims = NUM_ANIMS;
	resInfo.animations = animations;
	resInfo.modelIndex = gi.modelindex("models/monsters/harpy/tris.fm");
	resInfo.numSounds = NUM_SOUNDS;
	resInfo.sounds = sounds;

	sounds[SND_GIB]=gi.soundindex("misc/fleshbreak.wav");	
	sounds[SND_FLAP]=gi.soundindex("monsters/harpy/flap.wav");	
	sounds[SND_SCREAM]=gi.soundindex("monsters/harpy/scream.wav");	
	sounds[SND_FLAP_FAST]=gi.soundindex("monsters/harpy/flap_quick.wav");	
	sounds[SND_DIVE]=gi.soundindex("monsters/harpy/dive.wav");	
	sounds[SND_DEATH]=gi.soundindex("monsters/harpy/death.wav");	
	sounds[SND_PAIN1]=gi.soundindex("monsters/harpy/pain1.wav");	
	sounds[SND_PAIN2]=gi.soundindex("monsters/harpy/pain2.wav");	
	sounds[SND_ATTACK]=gi.soundindex("monsters/harpy/attack.wav");	
	
	sounds[SND_IDLE1]=gi.soundindex("monsters/harpy/pain1.wav");	
	sounds[SND_IDLE2]=gi.soundindex("monsters/harpy/pain2.wav");	

	classStatics[CID_HARPY].resInfo = &resInfo;
}

/*QUAKED monster_harpy(1 .5 0) (-16 -16 -12) (16 16 12) AMBUSH ASLEEP PERCHING CIRCLING

The harpy

AMBUSH - Will not be woken up by other monsters or shots from player

ASLEEP - will not appear until triggered

PERCHING - Will watch player until get too close or get behind the harpy

CIRCLING - harpy circles around in the air

"wakeup_target" - monsters will fire this target the first time it wakes up (only once)

"pain_target" - monsters will fire this target the first time it gets hurt (only once)

*/
void SP_monster_harpy(edict_t *self)
{
	if (!M_FlymonsterStart(self))
		return;				// Failed initialization

	self->msgHandler = DefaultMsgHandler;
	self->monsterinfo.dismember = HarpyDismember;

	if (!self->health)
		self->health = HARPY_HEALTH;

	self->max_health = self->health = MonsterHealth(self->health);

	self->mass = HARPY_MASS;
	self->yaw_speed = 14;

	self->movetype = PHYSICSTYPE_FLY;
	self->gravity = 0;
	self->flags |= FL_FLY;
	self->solid = SOLID_BBOX;
	self->clipmask = MASK_MONSTERSOLID;

	VectorCopy(STDMinsForClass[self->classID], self->mins);
	VectorCopy(STDMaxsForClass[self->classID], self->maxs);	

	self->svflags |= SVF_TAKE_NO_IMPACT_DMG;
	self->svflags |= SVF_DO_NO_IMPACT_DMG;

	self->materialtype = MAT_FLESH;

	self->s.modelindex = classStatics[CID_HARPY].resInfo->modelIndex;
	self->s.skinnum = 0;

	self->isBlocked = HarpyIsBlocked;
	self->bounced = HarpyIsBlocked;

	if (!self->s.scale)
	{
		self->monsterinfo.scale = self->s.scale = flrand(1.25, 1.75);
	}

	self->monsterinfo.otherenemyname = "monster_rat";	

	self->monsterinfo.aiflags |= AI_NO_ALERT;//pay no attention to alert ents

	if (self->spawnflags & MSF_PERCHING)
	{
		
		self->s.origin[2] += 4;
		SetAnim(self, ANIM_PIRCH5);
	}
	else if (self->spawnflags & MSF_SPECIAL1)
	{
		SetAnim(self, ANIM_CIRCLING);
	}
	else
	{
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
	}

	if(irand(0,1))
		self->s.fmnodeinfo[MESH_HORNS].flags |= FMNI_NO_DRAW;

	if(irand(0,1))
		self->s.fmnodeinfo[MESH_HORN].flags |= FMNI_NO_DRAW;

	if(irand(0,1))
		self->s.fmnodeinfo[MESH_BACKSPIKES].flags |= FMNI_NO_DRAW;

	if(irand(0,4))
		self->s.fmnodeinfo[MESH_NECKSPIKES].flags |= FMNI_NO_DRAW;

	if(irand(0,2))
		self->s.fmnodeinfo[MESH_TAILSPIKES].flags |= FMNI_NO_DRAW;

	gi.linkentity(self);
}
