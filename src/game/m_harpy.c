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
edict_t* harpy_head_carrier = NULL; // Harpy, which carries the head. //mxd. Named 'give_head_to_harpy' in original logic. Not SUS at all :)
edict_t* harpy_head_source = NULL; // Player or monster, who's head harpy is carrying. //mxd. Named 'take_head_from' in original logic.

#pragma region ========================== Harpy base info ==========================

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
	//&harpy_move_glide, //mxd. Unused.
	&harpy_move_dive_trans,
	&harpy_move_dive_hit_loop,
	&harpy_move_tumble,
	&harpy_move_perch1_idle,
	&harpy_move_perch2_idle,
	&harpy_move_perch3_idle,
	&harpy_move_perch4_idle,
	&harpy_move_perch5_idle,
	&harpy_move_perch6_idle,
	&harpy_move_perch7_idle,
	&harpy_move_perch8_idle,
	&harpy_move_perch9_idle,
	&harpy_move_takeoff,
	&harpy_move_circle,
	&harpy_move_circle_flap
};

static int sounds[NUM_SOUNDS];

static const vec3_t dead_harpy_mins = { -16.0f, -16.0f, 0.0f }; //mxd
static const vec3_t dead_harpy_maxs = {  16.0f,  16.0f, 12.0f }; //mxd

#pragma endregion

#pragma region ========================== Head grabbing functions =========================

static void HarpyHeadDie(edict_t* self, edict_t* inflictor, edict_t* attacker, int damage, const vec3_t point) //mxd. Named 'head_die' in original logic.
{
	BecomeDebris(self);
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
		self->nextthink = THINK_NEVER; //mxd. Use define.
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

		VectorMA(self->s.origin, self->harpy_head_offset, down, self->s.origin);

		self->nextthink = level.time + FRAMETIME; //mxd. Use define.
	}
}

void HarpyTakeHead(edict_t* self, edict_t* victim, const int bodypart_node_id, const int frame, const int flags) //mxd. Named 'harpy_take_head' in original logic.
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

	head->harpy_head_offset = 8.0f;
	VectorMA(head->s.origin, head->harpy_head_offset, down, head->s.origin);

	head->s.origin[2] += 100.0f;

	gi.CreateEffect(&head->s, FX_BODYPART, flags, head->s.origin, "ssbbb", (short)frame, (short)bodypart_node_id, 0, victim->s.modelindex, victim->s.number);

	head->think = HarpyHeadThink;
	head->nextthink = level.time + FRAMETIME; //mxd. Use define.

	gi.linkentity(head);

	harpy_head_carrier = NULL;
	harpy_head_source = NULL;

	VectorScale(forward, 200.0f, self->velocity);
	self->velocity[2] = 20.0f; //FIXME: fix angles?
	self->enemy = NULL;

	QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL); //FIXME: go into a circle?
}

#pragma endregion

#pragma region ========================== Utility functions =========================

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

static qboolean HarpyCheckDirections(const edict_t* self, const vec3_t goal, const vec3_t forward, const vec3_t right, const vec3_t up, const float check_dist, vec3_t goal_direction) //mxd. Named 'harpy_check_directions' in original logic.
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

static qboolean HarpyCheckSwoop(const edict_t* self, const vec3_t goal_pos) //mxd. Named 'harpy_check_swoop' in original logic.
{
	// Find the difference in the target's height and the creature's height.
	float z_diff = fabsf(self->enemy->s.origin[2] - self->s.origin[2]);

	if (z_diff < HARPY_MIN_SWOOP_DIST)
		return false;

	z_diff -= z_diff / 4.0f;

	vec3_t check_pos;
	VectorCopy(self->s.origin, check_pos);
	check_pos[2] -= z_diff;

	// Trace down about that far and about one forth the distance to the target.
	trace_t trace;
	gi.trace(self->s.origin, self->mins, self->maxs, check_pos, self, MASK_SHOT | MASK_WATER, &trace);

	if (trace.fraction < 1.0f || trace.startsolid || trace.allsolid)
		return false;

	// Trace straight to the target.
	gi.trace(check_pos, self->mins, self->maxs, goal_pos, self, MASK_SHOT | MASK_WATER, &trace);

	// If we hit our enemy, there's a clear path.
	return (trace.ent == self->enemy);
}

#pragma endregion

#pragma region ========================== Message handlers ==========================

static void HarpyDeathPainMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'harpy_dead_pain' in original logic.
{
	if (self->health <= -40) // Gib death.
	{
		BecomeDebris(self);
		self->think = NULL;
		self->nextthink = THINK_NEVER; //mxd. '0' in original logic. Changed for consistency sake.

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

static void HarpyPainMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'harpy_pain' in original logic.
{
	int temp;
	int damage;
	qboolean force_pain;
	ParseMsgParms(msg, "eeiii", &temp, &temp, &force_pain, &damage, &temp);

	if (self->curAnimID >= ANIM_PERCH1 && self->curAnimID <= ANIM_PERCH9)
	{
		SetAnim(self, ANIM_TAKEOFF);
	}
	else if (force_pain || (irand(0, 10) < 2 && self->pain_debounce_time < level.time))
	{
		gi.sound(self, CHAN_BODY, sounds[irand(SND_PAIN1, SND_PAIN2)], 1.0f, ATTN_NORM, 0.0f); //mxd. Inline harpy_pain1_noise() and harpy_pain2_noise().
		self->pain_debounce_time = level.time + 2.0f;

		SetAnim(self, ANIM_PAIN1);
	}
}

// Receiver for MSG_STAND, MSG_RUN and MSG_FLY. 
static void HarpyFlyMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'harpy_hover' in original logic.
{
	if (self->spawnflags & (MSF_PERCHING | MSF_SPECIAL1))
		return;

	if (irand(1, 10) > 1) //mxd. 70% -> 90% chance to avoid screaming.
	{
		SetAnim(self, ANIM_HOVER1);
	}
	else
	{
		gi.sound(self, CHAN_VOICE, sounds[SND_SCREAM], 1.0f, ATTN_NORM, 0.0f); //mxd. CHAN_BODY in original logic. Changed, so this and wing flap sounds can overlap.
		SetAnim(self, ANIM_HOVERSCREAM);
	}
}

static void HarpyEvadeMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'harpy_evade' in original logic.
{
	if (self->curAnimID >= ANIM_PERCH1 && self->curAnimID <= ANIM_PERCH9) //mxd. '> ANIM_PERCH1 && < ANIM_PERCH9' in original logic.
	{
		self->mins[2] -= 4.0f;
		SetAnim(self, ANIM_TAKEOFF);
	}
}

static void HarpyWatchMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'harpy_perch' in original logic.
{
	SetAnim(self, ANIM_PERCH5);
}

#pragma endregion

#pragma region ========================== Edict callbacks ===========================

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
				harpy_head_carrier = self;
				harpy_head_source = trace->ent;

				if (trace->ent->client != NULL)
				{
					trace->ent->health = 1;
					PlayerDecapitate(trace->ent, self);
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

		//mxd. Add attack sound.
		gi.sound(self, CHAN_WEAPON, sounds[irand(SND_ATTACK1, SND_ATTACK2)], 1.0f, ATTN_NORM, 0.0f);

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
				gi.sound(self, CHAN_BODY, sounds[SND_DEATH], 1.0f, ATTN_NORM, 0.0f); //mxd. Inline harpy_death_noise().
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

#pragma endregion

#pragma region ========================== Action functions ==========================

void harpy_flap_noise(edict_t* self)
{
	gi.sound(self, CHAN_BODY, sounds[SND_FLAP], 1.0f, ATTN_NORM, 0.0f);
}

void harpy_flap_fast_noise(edict_t* self)
{
	gi.sound(self, CHAN_BODY, sounds[SND_FLAP_FAST], 1.0f, ATTN_NORM, 0.0f);
}

void harpy_dive_noise(edict_t* self)
{
	gi.sound(self, CHAN_VOICE, sounds[SND_DIVE], 1.0f, ATTN_NORM, 0.0f); //mxd. CHAN_BODY in original logic. Changed, so this and wing flap sounds can overlap.
}

void harpy_ai_circle(edict_t* self, float forward_offset, float right_offset, float up_offset)
{
#define HARPY_CIRCLE_AMOUNT	4.0f
#define HARPY_CIRCLE_SPEED	64.0f

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
	if (fabsf(yaw_delta) > self->yaw_speed)
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

void harpy_ai_perch(edict_t* self) //mxd. Named 'harpy_ai_pirch' in original logic.
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
			SetAnim(self, ANIM_PERCH9);
		else if (right_dot < -0.6f)
			SetAnim(self, ANIM_PERCH8);
		else if (right_dot < -0.4f)
			SetAnim(self, ANIM_PERCH7);
		else if (right_dot < -0.2f)
			SetAnim(self, ANIM_PERCH6);
		else
			SetAnim(self, ANIM_PERCH5);
	}
	else // Right.
	{
		if (right_dot > 0.8f)
			SetAnim(self, ANIM_PERCH1);
		else if (right_dot > 0.6f)
			SetAnim(self, ANIM_PERCH2);
		else if (right_dot > 0.4f)
			SetAnim(self, ANIM_PERCH3);
		else if (right_dot > 0.2f)
			SetAnim(self, ANIM_PERCH4);
		else
			SetAnim(self, ANIM_PERCH5);
	}
}

void harpy_tumble_move(edict_t* self) //mxd. Named 'move_harpy_tumble' in original logic.
{
	if (self->monsterinfo.currframeindex == FRAME_hover1) //mxd
		harpy_flap_noise(self);

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
		gi.sound(self, CHAN_BODY, sounds[SND_DEATH], 1.0f, ATTN_NORM, 0.0f); //mxd. Inline harpy_death_noise().

		VectorCopy(self->s.angles, self->movedir);
		SetAnim(self, ANIM_DIE);
	}
}

void harpy_fix_angles(edict_t* self)
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

void harpy_pause(edict_t* self)
{
	if (M_ValidTarget(self, self->enemy))
	{
		QPostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
	}
	else if (self->curAnimID == ANIM_CIRCLING)
	{
		if (irand(0, 6) == 0)
			SetAnim(self, ANIM_CIRCLING_FLAP);
	}
	else if (self->curAnimID == ANIM_CIRCLING_FLAP && irand(0, 1) == 1)
	{
		SetAnim(self, ANIM_CIRCLING);
	}
}

// End of animation func for death animation.
void harpy_dead(edict_t* self)
{
	VectorCopy(dead_harpy_mins, self->mins); //mxd
	VectorCopy(dead_harpy_maxs, self->maxs); //mxd

	M_EndDeath(self);
}

void harpy_dive_move(edict_t* self) //mxd. Named 'move_harpy_dive' in original logic.
{
	if (self->groundentity != NULL || !HarpyCanMove(self, 64.0f)) //TODO: doesn't seem to be ever triggered. Will be handled by HarpyIsBlocked() anyway?
	{
		SetAnim(self, (self->groundentity == self->enemy ? ANIM_DIVE_END : ANIM_FLYBACK1)); //mxd. ANIM_DIVE_END case ignored in original logic.
		return;
	}

	// Find out the Z and Horizontal deltas to target.
	const float enemy_z = self->enemy->s.origin[2] + flrand(self->maxs[2], self->enemy->maxs[2]);
	const float z_dist = fabsf(self->s.origin[2] - enemy_z);

	if (z_dist > HARPY_MAX_DIVE_DIST)
	{
		vec3_t forward;
		AngleVectors(self->s.angles, forward, NULL, NULL);

		float forward_dist = 256.0f - z_dist * 0.85f;
		forward_dist = Clamp(forward_dist, 0.0f, 256.0f);

		VectorMA(forward, forward_dist, forward, self->velocity);

		self->velocity[2] = -z_dist * 2.25f;
		self->velocity[2] = max(-300.0f, self->velocity[2]);

		harpy_ai_glide(self, 0.0f, 0.0f, 0.0f);
	}
	else
	{
		SetAnim(self, ANIM_DIVE_TRANS);
	}
}

void harpy_dive_end_move(edict_t* self) //mxd. Named 'move_harpy_dive_end' in original logic.
{
	const vec3_t enemy_pos =
	{
		self->enemy->s.origin[0],
		self->enemy->s.origin[1],
		self->enemy->s.origin[2] + flrand(self->maxs[2], self->enemy->maxs[2])
	};

	vec3_t dir;
	VectorSubtract(enemy_pos, self->s.origin, dir);

	self->ideal_yaw = VectorYaw(dir);
	M_ChangeYaw(self);

	self->velocity[2] *= 0.75f;
	self->monsterinfo.jump_time *= HARPY_SWOOP_INCREMENT;

	const float forward_dist = min(HARPY_MAX_SWOOP_SPEED, self->monsterinfo.jump_time);

	if (self->groundentity != NULL || !HarpyCanMove(self, 128.0f)) //TODO: doesn't seem to be ever triggered. Will be handled by HarpyIsBlocked() anyway?
	{
		SetAnim(self, (self->groundentity == self->enemy ? ANIM_DIVE_END : ANIM_FLYBACK1)); //mxd. ANIM_DIVE_END case ignored in original logic.
		return;
	}

	const float dist = VectorLength(dir);
	VectorNormalize(dir);

	vec3_t forward;
	AngleVectors(self->s.angles, forward, NULL, NULL);

	if (DotProduct(forward, dir) < -0.5f)
	{
		SetAnim(self, ANIM_FLYBACK1);
		return;
	}

	VectorMA(self->velocity, forward_dist, forward, self->velocity);

	// Are we about to hit the target?
	if (dist < max(HARPY_COLLISION_DIST, VectorLength(self->velocity) * 0.5f)) //mxd. 'dist < HARPY_COLLISION_DIST' in original logic. May never happen when swooping really fast...
		SetAnim(self, ANIM_DIVE_END);
	else
		harpy_ai_glide(self, 0.0f, 0.0f, 0.0f);
}

void harpy_dive_loop(edict_t* self)
{
	SetAnim(self, ANIM_DIVE_LOOP);
}

void harpy_hit_loop(edict_t* self)
{
	SetAnim(self, ANIM_HIT_LOOP);
}

void harpy_flyback(edict_t* self)
{
	SetAnim(self, ANIM_FLYBACK1);
}

void harpy_check_dodge(edict_t* self)
{
	if (self->enemy == NULL)
		return;

	vec3_t enemy_dir;
	VectorSubtract(self->enemy->s.origin, self->s.origin, enemy_dir);
	VectorNormalize(enemy_dir);

	vec3_t dodge_dir;
	qboolean dodge = false;

	edict_t* ent = NULL;
	while ((ent = FindInRadius(ent, self->s.origin, HARPY_PROJECTILE_SEARCH_RADIUS)) != NULL)
	{
		// We're only interested in his projectiles.
		if (ent->owner != self->enemy)
			continue;

		vec3_t proj_dir;
		VectorNormalize2(ent->velocity, proj_dir);

		if (DotProduct(proj_dir, enemy_dir) < -0.85f && irand(0, 1) == 1)
		{
			dodge = true;

			vec3_t right;
			AngleVectors(self->s.angles, NULL, right, NULL);

			if (irand(0, 1) == 1)
				Vec3ScaleAssign(-1.0f, right);

			vec3_t goal_pos;
			VectorMA(self->s.origin, 100.0f, right, goal_pos);

			trace_t trace;
			gi.trace(self->s.origin, self->mins, self->maxs, goal_pos, self, MASK_SHOT | MASK_WATER, &trace);

			if (trace.fraction < 1.0f || trace.startsolid || trace.allsolid)
				Vec3ScaleAssign(-1.0f, right);

			VectorCopy(right, dodge_dir);

			//TODO: we found projectile to dodge. Shouldn't we either break here, or compare distances with previous match to find the closest one?
		}
	}

	if (dodge && self->monsterinfo.misc_debounce_time < level.time) // If he is, dodge!
	{
		VectorMA(self->velocity, flrand(300.0f, 500.0f), dodge_dir, self->velocity);
		self->monsterinfo.misc_debounce_time = level.time + flrand(2.0f, 4.0f); //mxd. irand() in original logic.
	}

	harpy_ai_glide(self, 0.0f, 0.0f, 0.0f);
}

void harpy_hover_move(edict_t* self) //mxd. Named 'move_harpy_hover' in original logic.
{
	if (self->monsterinfo.currframeindex == FRAME_pain1) //mxd
		harpy_flap_noise(self);

	if (self->enemy == NULL && !FindTarget(self))
		return;

	// First check to see that the player is at least 128 units away (discounting z height).
	vec3_t goal_dir;
	VectorSubtract(self->enemy->s.origin, self->s.origin, goal_dir);
	goal_dir[2] = 0.0f;

	// Face target.
	self->ideal_yaw = VectorYaw(goal_dir);
	M_ChangeYaw(self);

	const float goal_dist = VectorLength(goal_dir);

	// If he is...
	if (goal_dist > HARPY_MIN_HOVER_DIST && goal_dist < HARPY_MAX_HOVER_DIST)
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
			if (HarpyCheckDirections(self, self->enemy->s.origin, forward, right, up, HARPY_CHECK_DIST, goal_dir))
			{
				VectorMA(self->velocity, flrand(300.0f, 400.0f), goal_dir, self->velocity);
				return;
			}

			// Otherwise just flap around and wait, perhaps lower yourself a bit if high up.
			self->velocity[0] = flrand(-HARPY_DRIFT_AMOUNT_X, HARPY_DRIFT_AMOUNT_X);
			self->velocity[1] = flrand(-HARPY_DRIFT_AMOUNT_Y, HARPY_DRIFT_AMOUNT_Y);
			self->velocity[2] = flrand(-HARPY_DRIFT_AMOUNT_Z, HARPY_DRIFT_AMOUNT_Z);

			return;
		}

		vec3_t dodge_dir;
		// Check to make sure the player isn't shooting anything.

		// This won't change over the calculations.
		vec3_t enemy_dir;
		VectorSubtract(self->enemy->s.origin, self->s.origin, enemy_dir);
		VectorNormalize(enemy_dir);

		qboolean dodge = false;
		edict_t* ent = NULL;

		while ((ent = FindInRadius(ent, self->s.origin, HARPY_PROJECTILE_SEARCH_RADIUS)) != NULL)
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

				if (trace.fraction < 1.0f || trace.startsolid || trace.allsolid)
					Vec3ScaleAssign(-1.0f, right);

				//TODO: should probably check if this dir is blocked as well. Could also try dodging up/down/diagonally.
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

		// If nothing is happening, check to swoop. If you can - nail'em.
		if (HarpyCheckSwoop(self, self->enemy->s.origin))
		{
			self->monsterinfo.jump_time = 2.0f;
			SetAnim(self, ANIM_DIVE_GO);

			return;
		}

		// Find the difference in the target's height and the creature's height.
		const float z_dist = fabsf(self->enemy->s.origin[2] - self->s.origin[2]);

		// We can't swoop because we're too low, so fly upwards if possible.
		if (z_dist < HARPY_MIN_SWOOP_DIST)
		{
			if (!HarpyCanMove(self, -64.0f))
				SetAnim(self, ANIM_FLY1);
			else
				SetAnim(self, ANIM_FLYBACK1);
		}
		else
		{
			// Otherwise just flap around and wait, perhaps lower yourself a bit if high up.			
			self->velocity[0] = flrand(-HARPY_DRIFT_AMOUNT_X, HARPY_DRIFT_AMOUNT_X);
			self->velocity[1] = flrand(-HARPY_DRIFT_AMOUNT_Y, HARPY_DRIFT_AMOUNT_Y);
			self->velocity[2] = flrand(-HARPY_DRIFT_AMOUNT_Z, HARPY_DRIFT_AMOUNT_Z);

			vec3_t forward;
			AngleVectors(self->s.angles, forward, NULL, NULL);
			VectorMA(self->velocity, flrand(200.0f, 300.0f), forward, self->velocity); //mxd. irand() in original logic.
		}

		//FIXME: If he's too far away trace a line (expanded) to see if you can move at him.
		return;
	}

	if (goal_dist < HARPY_MIN_HOVER_DIST)
	{
		if (!HarpyCanMove(self, -64.0f))
			SetAnim(self, ANIM_FLY1);
		else
			SetAnim(self, ANIM_FLYBACK1);
	}
	else
	{
		if (!HarpyCanMove(self, 64.0f))
			SetAnim(self, ANIM_FLYBACK1);
		else
			SetAnim(self, ANIM_FLY1);
	}
}

#pragma endregion

void HarpyStaticsInit(void)
{
	static ClassResourceInfo_t res_info; //mxd. Made local static.

	classStatics[CID_HARPY].msgReceivers[MSG_DEATH] = HarpyDeathMsgHandler;
	classStatics[CID_HARPY].msgReceivers[MSG_FLY] = HarpyFlyMsgHandler;
	classStatics[CID_HARPY].msgReceivers[MSG_STAND] = HarpyFlyMsgHandler;
	classStatics[CID_HARPY].msgReceivers[MSG_RUN] = HarpyFlyMsgHandler;
	classStatics[CID_HARPY].msgReceivers[MSG_PAIN] = HarpyPainMsgHandler;
	classStatics[CID_HARPY].msgReceivers[MSG_DISMEMBER] = DismemberMsgHandler;
	classStatics[CID_HARPY].msgReceivers[MSG_WATCH] = HarpyWatchMsgHandler;
	classStatics[CID_HARPY].msgReceivers[MSG_DEATH_PAIN] = HarpyDeathPainMsgHandler;
	classStatics[CID_HARPY].msgReceivers[MSG_EVADE] = HarpyEvadeMsgHandler;

	res_info.numAnims = NUM_ANIMS;
	res_info.animations = animations;
	res_info.modelIndex = gi.modelindex("models/monsters/harpy/tris.fm");

	sounds[SND_GIB] = gi.soundindex("misc/fleshbreak.wav");
	sounds[SND_FLAP] = gi.soundindex("monsters/harpy/flap.wav");
	sounds[SND_SCREAM] = gi.soundindex("monsters/harpy/scream.wav");
	sounds[SND_FLAP_FAST] = gi.soundindex("monsters/harpy/flap_quick.wav");
	sounds[SND_DIVE] = gi.soundindex("monsters/harpy/dive.wav");
	sounds[SND_DEATH] = gi.soundindex("monsters/harpy/death.wav");
	sounds[SND_PAIN1] = gi.soundindex("monsters/harpy/pain1.wav");
	sounds[SND_PAIN2] = gi.soundindex("monsters/harpy/pain2.wav");
	//sounds[SND_ATTACK] = gi.soundindex("monsters/harpy/attack.wav");
	sounds[SND_ATTACK1] = gi.soundindex("monsters/beetle/meleehit1.wav"); //mxd. Reuse beetle attack sounds...
	sounds[SND_ATTACK2] = gi.soundindex("monsters/beetle/meleehit2.wav"); //mxd. Reuse beetle attack sounds...
	sounds[SND_IDLE1] = gi.soundindex("monsters/harpy/pain1.wav");
	sounds[SND_IDLE2] = gi.soundindex("monsters/harpy/pain2.wav");

	res_info.numSounds = NUM_SOUNDS;
	res_info.sounds = sounds;

	classStatics[CID_HARPY].resInfo = &res_info;
}

// QUAKED monster_harpy(1 .5 0) (-16 -16 -12) (16 16 12) AMBUSH ASLEEP PERCHING CIRCLING
// The harpy.
// Spawnflags:
// AMBUSH	- Will not be woken up by other monsters or shots from player.
// ASLEEP	- will not appear until triggered.
// PERCHING	- Will watch player until get too close or get behind the harpy.
// CIRCLING	- Harpy circles around in the air.
// Variables:
// wakeup_target	- Monsters will fire this target the first time it wakes up (only once).
// pain_target		- Monsters will fire this target the first time it gets hurt (only once).
void SP_monster_harpy(edict_t* self)
{
	if (!M_FlymonsterStart(self))
		return; // Failed initialization.

	if (self->health == 0)
		self->health = HARPY_HEALTH;

	self->health = MonsterHealth(self->health);
	self->max_health = self->health;

	self->mass = HARPY_MASS;
	self->yaw_speed = 14.0f;

	self->movetype = PHYSICSTYPE_FLY;
	self->gravity = 0.0f;
	self->flags |= FL_FLY;
	self->solid = SOLID_BBOX;
	self->clipmask = MASK_MONSTERSOLID;
	self->s.effects |= EF_FAST_MOVER; //mxd

	VectorCopy(STDMinsForClass[self->classID], self->mins);
	VectorCopy(STDMaxsForClass[self->classID], self->maxs);

	self->svflags |= (SVF_TAKE_NO_IMPACT_DMG | SVF_DO_NO_IMPACT_DMG);
	self->materialtype = MAT_FLESH;

	self->s.modelindex = (byte)classStatics[CID_HARPY].resInfo->modelIndex;

	self->msgHandler = DefaultMsgHandler;
	self->monsterinfo.dismember = HarpyDismember;
	self->isBlocked = HarpyIsBlocked;
	self->bounced = HarpyIsBlocked;

	if (self->s.scale == 0.0f)
		self->s.scale = flrand(1.25f, 1.75f);

	self->monsterinfo.scale = self->s.scale;
	self->monsterinfo.otherenemyname = "monster_rat";
	self->monsterinfo.aiflags |= AI_NO_ALERT; // Pay no attention to alert ents.

	if (self->spawnflags & MSF_PERCHING)
	{
		self->s.origin[2] += 4.0f;
		SetAnim(self, ANIM_PERCH5);
	}
	else if (self->spawnflags & MSF_SPECIAL1)
	{
		SetAnim(self, ANIM_CIRCLING);
	}
	else
	{
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
	}

	if (irand(0, 1) != 0)
		self->s.fmnodeinfo[MESH_HORNS].flags |= FMNI_NO_DRAW;

	if (irand(0, 1) != 0)
		self->s.fmnodeinfo[MESH_HORN].flags |= FMNI_NO_DRAW;

	if (irand(0, 1) != 0)
		self->s.fmnodeinfo[MESH_BACKSPIKES].flags |= FMNI_NO_DRAW;

	if (irand(0, 4) != 0)
		self->s.fmnodeinfo[MESH_NECKSPIKES].flags |= FMNI_NO_DRAW;

	if (irand(0, 2) != 0)
		self->s.fmnodeinfo[MESH_TAILSPIKES].flags |= FMNI_NO_DRAW;

	gi.linkentity(self);
}