//
// m_gorgon.c
//
// Copyright 1998 Raven Software
//

#include "m_gorgon.h"
#include "m_gorgon_shared.h"
#include "m_gorgon_anim.h"
#include "m_gorgon_moves.h"
#include "g_DefaultMessageHandler.h"
#include "g_monster.h"
#include "g_debris.h" //mxd
#include "m_stats.h"
#include "mg_ai.h" //mxd
#include "mg_guide.h" //mxd
#include "Random.h"
#include "Utilities.h"
#include "Vector.h"
#include "g_local.h"

#define GORGON_MELEE_RANGE		48.0f //mxd. Named 'GORGON_STD_MELEE_RNG' in original logic.
#define GORGON_MAX_HOP_RANGE	200.0f //mxd. Named 'GORGON_STD_MAXHOP_RNG' in original logic.

#pragma region ========================== Gorgon base info ==========================

static const animmove_t* animations[NUM_ANIMS] =
{
	&gorgon_move_stand1,
	&gorgon_move_stand2,
	&gorgon_move_stand3,
	&gorgon_move_stand4,
	&gorgon_move_walk,
	&gorgon_move_walk2,
	&gorgon_move_walk3,
	&gorgon_move_melee1,
	&gorgon_move_melee2,
	&gorgon_move_melee3,
	&gorgon_move_melee4,
	&gorgon_move_melee5,
	&gorgon_move_melee6,
	&gorgon_move_melee7,
	&gorgon_move_melee8,
	&gorgon_move_melee9,
	&gorgon_move_melee10,
	&gorgon_move_fjump,
	&gorgon_move_run1,
	&gorgon_move_run2,
	&gorgon_move_run3,
	&gorgon_move_pain1,
	&gorgon_move_pain2,
	&gorgon_move_pain3,
	&gorgon_move_die1,
	&gorgon_move_die2,
	&gorgon_move_snatch,
	&gorgon_move_catch,
	&gorgon_move_miss,
	&gorgon_move_readycatch,
	&gorgon_move_snatchhi,
	&gorgon_move_snatchlow,
	&gorgon_move_slip,
	&gorgon_move_slip_pain,
	&gorgon_move_delay,
	&gorgon_move_roar,
	&gorgon_move_roar2,
	&gorgon_move_land2,
	&gorgon_move_land,
	&gorgon_move_inair,
	&gorgon_move_to_swim,
	&gorgon_move_swim,
	&gorgon_move_swim_bite_a,
	&gorgon_move_swim_bite_a, //TODO: use gorgon_move_swim_bite_b?
	&gorgon_move_outwater,
	&gorgon_move_eat_down,
	&gorgon_move_eat_up,
	&gorgon_move_eat_loop,
	&gorgon_move_eat_tear,
	&gorgon_move_eat_pullback,
	&gorgon_move_look_around,
	&gorgon_move_eat_left,
	&gorgon_move_eat_right,
	&gorgon_move_eat_snap,
	&gorgon_move_eat_react
};

static int sounds[NUM_SOUNDS];

#pragma endregion

#pragma region ========================== Utility functions =========================

// Checks if there are any gorgons in range that aren't awake.
static qboolean GorgonFindAsleepGorgons(const edict_t* self) //mxd. Named 'gorgonFindAsleepGorgons' in original logic.
{
	edict_t* e = NULL;

	while ((e = FindInRadius(e, self->s.origin, GORGON_ALERT_DIST)) != NULL)
		if (e != self && e->health > 0 && (e->svflags & SVF_MONSTER) && e->enemy == NULL && e->classID == CID_GORGON && !e->monsterinfo.roared)
			return true;

	return false;
}

static qboolean GorgonCanAttack(edict_t* self) //mxd. Named 'gorgon_check_attack' in original logic.
{
	if (!M_ValidTarget(self, self->enemy) || !AI_IsClearlyVisible(self, self->enemy))
		return false;

	vec3_t diff;
	VectorSubtract(self->enemy->s.origin, self->s.origin, diff);
	const float dist = VectorNormalize(diff);

	if (dist < 200.0f)
	{
		self->wakeup_time = level.time + 1.0f; // Wake up other monsters.
		G_PostMessage(self, MSG_MELEE, PRI_DIRECTIVE, NULL);

		return true;
	}

	if (dist < 400.0f && irand(0, 3) == 0)
	{
		gorgon_growl(self);
		return true;
	}

	if (irand(0, 3) == 0)
	{
		vec3_t forward;
		vec3_t right;
		AngleVectors(self->s.angles, forward, right, NULL);
		const float dot = DotProduct(forward, diff);

		if (dot > -0.85f && dot < -0.25f) // Behind and aesthetically correct.
		{
			gorgon_growl(self);
			SetAnim(self, (DotProduct(right, diff) > 0.0f ? ANIM_STAND3 : ANIM_STAND2)); // dot > 0 -> to the right, otherwise to the left.

			return true;
		}

		return false;
	}

	if (self->monsterinfo.aiflags & AI_EATING)
	{
		const int chance = irand(0, 100);

		if (chance < 10)
			SetAnim(self, ANIM_EAT_TEAR);
		else if (chance < 20)
			SetAnim(self, ANIM_EAT_PULLBACK);
		else
			SetAnim(self, ANIM_EAT_LOOP);

		return true;
	}

	return false;
}

static qboolean GorgonSetupJumpArc(edict_t* self, const vec3_t angles, const vec3_t landing_spot) //mxd. Added to reduce code duplication.
{
	// JUMPING
	// Calculate landing spot behind enemy to jump to.
	// Calculate arc spot to jump at which will arc the monster to the landing spot.
	// Calculate velocity to make monster jump to hit arc spot.

	// Choose landing spot behind enemy.
	vec3_t forward;
	AngleVectors(angles, forward, NULL, NULL);

	vec3_t landing_pos;
	VectorMA(landing_spot, 60.0f, forward, landing_pos);

	trace_t trace;
	const vec3_t test_spot = VEC3_INITA(landing_pos, 0.0f, 0.0f, -1024.0f);
	gi.trace(landing_pos, self->mins, self->maxs, test_spot, self, MASK_MONSTERSOLID | MASK_WATER, &trace);

	if (trace.fraction == 1.0f || (!(trace.contents & CONTENTS_SOLID) && !(trace.contents & CONTENTS_WATER)))
		return false;

	// Calculate arc spot (the top of his jump arc) which will land monster at landing spot.
	vec3_t landing_dir;
	VectorSubtract(self->s.origin, landing_pos, landing_dir);

	const vec3_t landing_spot_angles = VEC3_SET(0.0f, VectorYaw(landing_dir), 0.0f);

	vec3_t up;
	AngleVectors(landing_spot_angles, forward, NULL, up);

	vec3_t arc_spot;
	VectorMA(landing_pos, 20.0f, forward, arc_spot);
	VectorMA(landing_pos, 180.0f, up, arc_spot);

	// Calculate velocity to make monster jump to hit arc spot.
	vec3_t arc_dir;
	VectorSubtract(arc_spot, self->s.origin, arc_dir); // Face monster to arc spot.

	vec3_t arc_angles;
	vectoangles(arc_dir, arc_angles);
	self->best_move_yaw = arc_angles[YAW];

	const float hold_time = VectorLength(arc_dir) / 200.0f;

	AngleVectors(arc_angles, forward, NULL, NULL);
	VectorScale(forward, hold_time * 300.0f, self->movedir); // Store calculated jump velocity in movedir.
	self->movedir[2] = hold_time * 200.0f;

	//mxd. Scale by s.scale (scaling directly by s.scale results in SMOL gorgons greatly under-shooting the jump (tested with .5 scale)).
	const float mass_scaler = LerpFloat(self->s.scale, 1.0f, 0.5f);
	Vec3ScaleAssign(mass_scaler, self->movedir);

	self->jump_time = level.time + 0.5f;
	self->monsterinfo.jump_time = level.time + 3.0f;

	return true;
}

static qboolean GorgonSetupJump(edict_t* self) //mxd. Named 'gorgon_check_jump' in original logic.
{
	vec3_t landing_spot;

	if (self->jump_chance < irand(0, 100) || !MG_TryGetTargetOrigin(self, landing_spot) || !MG_IsInforntPos(self, landing_spot))
		return false;

	vec3_t diff;
	VectorSubtract(self->s.origin, landing_spot, diff);

	if (VectorLength(diff) > 400.0f)
		return false;

	vec3_t angles;

	if (self->enemy != NULL)
		VectorSet(angles, 0.0f, anglemod(-self->enemy->s.angles[YAW]), 0.0f);
	else
		VectorCopy(self->s.angles, angles);

	return GorgonSetupJumpArc(self, angles, landing_spot);
}

static qboolean GorgonStartSlipAnimation(edict_t* self, const qboolean from_pain) //mxd. Named 'gorgonCheckSlipGo' in original logic.
{
	if (self->enemy == NULL)
		return false;

	vec3_t dir;
	VectorSubtract(self->enemy->s.origin, self->s.origin, dir);
	VectorNormalize(dir);

	vec3_t right;
	AngleVectors(self->s.angles, NULL, right, NULL);

	if (DotProduct(right, dir) > 0.3f && irand(0, 1) == 1)
	{
		// Fall down, go boom.
		if (from_pain)
		{
			SetAnim(self, ANIM_SLIP_PAIN);
			return true;
		}

		if (self->monsterinfo.misc_debounce_time < level.time && irand(0, 4) == 0)
		{
			self->monsterinfo.misc_debounce_time = level.time + 7.0f;
			SetAnim(self, ANIM_SLIP);

			return true;
		}
	}

	return false;
}

#pragma endregion

#pragma region ========================== Edict callbacks ===========================

void GorgonRoarResponsePreThink(edict_t* self) //mxd. Named 'gorgon_roar_response_go' in original logic.
{
	self->pre_think = NULL;
	self->next_pre_think = -1.0f;

	if (self->ai_mood == AI_MOOD_EAT)
		self->ai_mood = AI_MOOD_PURSUE;

	SetAnim(self, ANIM_ROAR2);

	self->nextthink = level.time + FRAMETIME; //mxd. Use define. //TODO: no think callbacks assigned. Not needed? 
}

void GorgonPreThink(edict_t* self) //mxd. Named 'gorgon_prethink' in original logic.
{
	// Also make wake on surface of water?
	if (self->flags & FL_INWATER)
	{
		self->gravity = 0.0f;
		self->svflags |= (SVF_TAKE_NO_IMPACT_DMG | SVF_DO_NO_IMPACT_DMG);

		if (!self->gorgon_is_underwater)
		{
			gi.CreateEffect(NULL, FX_WATER_ENTRYSPLASH, CEF_FLAG7, self->s.origin, "bd", 128 | 96, vec3_up);
			self->gorgon_is_underwater = true;
		}

		if (self->curAnimID == ANIM_INAIR)
			SetAnim(self, ANIM_TO_SWIM);
	}
	else
	{
		self->gravity = 1.0f;
		self->svflags &= ~SVF_TAKE_NO_IMPACT_DMG;

		if (self->s.scale > 0.5f)
			self->svflags &= ~SVF_DO_NO_IMPACT_DMG;

		if (self->gorgon_is_underwater)
		{
			gi.RemoveEffects(&self->s, FX_M_EFFECTS); // Remove FX_UNDER_WATER_WAKE effect. Done outside of gorgon_is_underwater check in original logic --mxd.
			gi.CreateEffect(NULL, FX_WATER_ENTRYSPLASH, 0, self->s.origin, "bd", 128 | 96, vec3_up);
			self->gorgon_is_underwater = false;
		}

		if (self->curAnimID == ANIM_SWIM || self->curAnimID == ANIM_SWIM_BITE_A || self->curAnimID == ANIM_SWIM_BITE_B)
			SetAnim(self, ANIM_RUN1);

		gorgon_fix_pitch(self);
	}

	self->next_pre_think = level.time + FRAMETIME; //mxd. Use define.
}

#pragma endregion

#pragma region ========================== Message handlers ==========================

// Respond to call.
static void GorgonVoicePollMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'gorgon_roar_response' in original logic.
{
	if (irand(0, 3) > 0) // 25% chance to not roar back.
	{
		self->pre_think = GorgonRoarResponsePreThink;
		self->next_pre_think = level.time + flrand(0.5f, 2.0f);
		self->nextthink = level.time + 10.0f; //TODO: GorgonRoarResponsePreThink will always override this. No think callbacks assigned. Not needed?
	}
}

// Gorgon Eat - decide which eating animations to use.
static void GorgonEatMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'gorgon_eat' in original logic.
{
	const int chance = irand(0, 100);

	if (chance < 80)
		SetAnim(self, ANIM_EAT_LOOP);
	else if (chance < 90)
		SetAnim(self, ANIM_EAT_PULLBACK);
	else
		SetAnim(self, ANIM_EAT_TEAR);

	self->monsterinfo.misc_debounce_time = level.time + 5.0f;
}

// Gorgon Stand - decide which standing animations to use.
static void GorgonStandMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'gorgon_stand' in original logic.
{
	if (self->ai_mood == AI_MOOD_DELAY)
	{
		SetAnim(self, ANIM_DELAY);
		return;
	}

	if (GorgonCanAttack(self))
		return;

	const int chance = irand(0, 100);

	if (self->monsterinfo.aiflags & AI_EATING)
	{
		if (chance < 10)
			SetAnim(self, ANIM_EAT_TEAR);
		else if (chance < 20)
			SetAnim(self, ANIM_EAT_PULLBACK);
		else
			SetAnim(self, ANIM_EAT_LOOP);
	}
	else
	{
		if (chance < 10)
			SetAnim(self, ANIM_STAND2);
		else if (chance < 20)
			SetAnim(self, ANIM_STAND3);
		else
			SetAnim(self, ANIM_STAND1);
	}

	self->monsterinfo.misc_debounce_time = level.time + 5.0f;
}

// Gorgon Walk - decide which walk animations to use.
static void GorgonWalkMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'gorgon_walk' in original logic.
{
	vec3_t target_origin;

	if (!MG_TryGetTargetOrigin(self, target_origin))
		return;

	if (self->enemy == NULL) // ?goal?
	{
		SetAnim(self, ANIM_WALK1);
		return;
	}

	if (self->spawnflags & MSF_COWARD)
	{
		vec3_t diff;
		VectorSubtract(self->s.origin, target_origin, diff);
		const float dist = VectorLength(diff);
		self->ideal_yaw = VectorYaw(diff);

		M_ChangeYaw(self);

		if (dist < 200.0f)
		{
			self->monsterinfo.aiflags |= AI_FLEE;
			self->monsterinfo.flee_finished = level.time + flrand(4.0f, 7.0f);
			SetAnim(self, ANIM_RUN1);

			return;
		}
	}

	if (AI_IsClearlyVisible(self, self->enemy) && AI_IsInfrontOf(self, self->enemy))
	{
		vec3_t diff;
		VectorSubtract(self->s.origin, target_origin, diff);
		const float dist = VectorLength(diff);

		// target_origin is within range and far enough above or below to warrant a jump.
		if (dist > 40.0f && dist < 600.0f && (self->s.origin[2] < target_origin[2] - 18.0f || self->s.origin[2] > target_origin[2] + 18.0f) && GorgonSetupJump(self))
		{
			SetAnim(self, ANIM_FJUMP);
			return;
		}
	}

	const float delta = anglemod(self->s.angles[YAW] - self->ideal_yaw);

	if (delta > 25.0f && delta <= 180.0f)
		SetAnim(self, ANIM_WALK3);
	else if (delta > 180.0f && delta < 335.0f)
		SetAnim(self, ANIM_WALK2);
	else
		SetAnim(self, ANIM_WALK1);
}

// Gorgon Melee - decide which melee animations to use.
static void GorgonMeleeMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'gorgon_melee' in original logic.
{
	if (self->ai_mood == AI_MOOD_NAVIGATE || !AI_HaveEnemy(self))
		return;

	// Too soon to attack again?
	if (self->monsterinfo.attack_finished > level.time)
	{
		const float chance = flrand(0.0f, 1.0f);

		if (chance < 0.6f)
			SetAnim(self, ANIM_STAND4);
		else if (chance < 0.7f)
			SetAnim(self, ANIM_MELEE6); // Hop left.
		else if (chance < 0.8f)
			SetAnim(self, ANIM_MELEE7); // Hop right.
		else
			SetAnim(self, ANIM_MELEE9); // Hop backward.

		return;
	}

	vec3_t forward;
	vec3_t up;
	AngleVectors(self->s.angles, forward, NULL, up);

	vec3_t melee_point;
	VectorMA(self->s.origin, self->maxs[2] * 0.5f, up, melee_point);
	VectorMA(melee_point, self->maxs[0], forward, melee_point);

	vec3_t diff;
	VectorSubtract(self->enemy->s.origin, melee_point, diff);
	const float dist = VectorLength(diff);

	const float separation = self->enemy->maxs[0];

	// Ok to do a melee attack?
	if (dist - separation < self->melee_range)
	{
		const float chance = flrand(0.0f, 1.0f);

		if (Q_stricmp(self->enemy->classname, "monster_rat") == 0) //mxd. stricmp -> Q_stricmp.
			SetAnim(self, (self->enemy->s.origin[2] > self->s.origin[2] ? ANIM_SNATCHHI : ANIM_SNATCHLOW));
		else if (chance < 0.25f)
			SetAnim(self, ANIM_MELEE1); // Attack left.
		else if (chance < 0.5f)
			SetAnim(self, ANIM_MELEE2); // Attack right.
		else if (chance < 0.75f)
			SetAnim(self, ANIM_MELEE3); // Attack up.
		else
			SetAnim(self, ANIM_MELEE4); // Pull back.

		return;
	}

	// Out of melee range?
	if (dist < 150.0f)
	{
		SetAnim(self, ANIM_MELEE5);
		return;
	}

	// Hop forward?
	if (dist < self->s.scale * GORGON_MAX_HOP_RANGE)
	{
		// Checks ahead to see if can hop at it.
		vec3_t hop_pos;
		VectorCopy(self->s.origin, hop_pos);
		VectorMA(hop_pos, self->s.scale * 64.0f, forward, hop_pos);

		trace_t	trace;
		gi.trace(self->s.origin, self->mins, self->maxs, hop_pos, self, MASK_SHOT, &trace);

		if (trace.ent == self->enemy || trace.fraction == 1.0f)
		{
			SetAnim(self, ANIM_MELEE8);
			return;
		}

		// Try closer position...
		VectorCopy(self->s.origin, hop_pos);
		VectorMA(hop_pos, self->s.scale * 32.0f, forward, hop_pos);

		gi.trace(self->s.origin, self->mins, self->maxs, hop_pos, self, MASK_SHOT, &trace);

		SetAnim(self, (trace.fraction == 1.0f ? ANIM_MELEE7 : ANIM_MELEE6));
	}
}

// Gorgon Run - decide which run animations to use.
static void GorgonRunMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'gorgon_run' in original logic.
{
	vec3_t target_origin;

	if (!AI_HaveEnemy(self) || !MG_TryGetTargetOrigin(self, target_origin))
		return;

	if (self->flags & FL_INWATER)
	{
		gorgon_swim_go(self);
		return;
	}

	qboolean enemy_visible;

	if (self->ai_mood == AI_MOOD_PURSUE)
		enemy_visible = AI_IsClearlyVisible(self, self->enemy);
	else
		enemy_visible = MG_IsClearlyVisiblePos(self, self->monsterinfo.nav_goal);

	if (enemy_visible)
	{
		// Roar?
		if (self->enemy != NULL && irand(0, 4) > 0 && self->damage_debounce_time < level.time && !self->monsterinfo.roared && AI_IsInfrontOf(self, self->enemy))
		{
			// Should we do this the first time we see player?
			if (GorgonFindAsleepGorgons(self))
			{
				self->damage_debounce_time = level.time + 10.0f;
				SetAnim(self, ANIM_ROAR); // Threaten, brings other monsters.

				return;
			}

			// Make a wakeup roar?
			if (!self->gorgon_wakeup_roar)
			{
				self->gorgon_wakeup_roar = true;
				SetAnim(self, ANIM_ROAR2);

				return; //mxd. Original logic doesn't return here, so this behaviour is always overridden by either jump or walk anim below.
			}
		}

		// Enemy is within range and far enough above or below to warrant a jump.
		if (MG_IsInforntPos(self, target_origin))
		{
			vec3_t diff;
			VectorSubtract(self->s.origin, target_origin, diff);
			const float dist = VectorLength(diff);

			if (dist > 40.0f && dist < 600.0f && (self->s.origin[2] < target_origin[2] - 24.0f || self->s.origin[2] > target_origin[2] + 24.0f))
			{
				if (fabsf(self->s.origin[2] - target_origin[2] - 24.0f) < 200.0f) // Can't jump more than 200 high. //mxd. abs() -> fabsf().
				{
					if (irand(0, 2) == 0 && (self->ai_mood == AI_MOOD_PURSUE || irand(0, 4) == 0) && GorgonSetupJump(self)) // 20% chance to jump at a buoy.
					{
						SetAnim(self, ANIM_FJUMP);
						return;
					}
				}
			}
		}
	}

	const float delta = anglemod(self->s.angles[YAW] - self->ideal_yaw);

	if (delta > 45.0f && delta <= 180.0f)
		SetAnim(self, ANIM_RUN3); // Turn right.
	else if (delta > 180.0f && delta < 315.0f)
		SetAnim(self, ANIM_RUN2); // Turn left.
	else
		SetAnim(self, ANIM_RUN1); // Run on.
}

// Gorgon Pain - make the decision between pains 1, 2, or 3 or slip.
static void GorgonPainMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'gorgon_pain' in original logic.
{
	edict_t* temp_ent;
	int temp;
	int damage;
	qboolean force_pain;
	G_ParseMsgParms(msg, "eeiii", &temp_ent, &temp_ent, &force_pain, &damage, &temp);

	if (!force_pain && (irand(0, 2) == 0 || self->groundentity == NULL || self->pain_debounce_time > level.time))
		return;

	self->pain_debounce_time = level.time + 0.5f;
	gi.sound(self, CHAN_VOICE, sounds[irand(SND_PAIN1, SND_PAIN2)], 1.0f, ATTN_NORM, 0.0f);

	if (irand(0, 4) == 0)
		self->s.skinnum = GORGON_PAIN_SKIN;

	if (SKILL > SKILL_MEDIUM || !GorgonStartSlipAnimation(self, true)) //TODO: Strange skill check: always do pain anims only on Hard+? Should be inverted (or easy-only) instead?
		SetAnim(self, irand(ANIM_PAIN1, ANIM_PAIN3));
}

static void GorgonDeathPainMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'gorgon_death_pain' in original logic.
{
	if (self->health <= -80)
	{
		gi.sound(self, CHAN_BODY, sounds[SND_GIB], 1.0f, ATTN_NORM, 0.0f);
		self->dead_state = DEAD_DEAD;
		BecomeDebris(self);
	}
}

// Gorgon Die - choose  decide which death animation to use.
static void GorgonDeathMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'gorgon_death' in original logic.
{
	if (self->monsterinfo.aiflags & AI_DONT_THINK)
	{
		gi.sound(self, CHAN_VOICE, sounds[SND_DIE], 1.0f, ATTN_NORM, 0.0f);
		SetAnim(self, irand(ANIM_DIE1, ANIM_DIE2));

		return;
	}

	self->msgHandler = DeadMsgHandler;

	// Check for gib.
	if (self->health <= -80)
	{
		GorgonDeathPainMsgHandler(self, msg); //mxd. Reuse function.
		return;
	}

	if (self->dead_state == DEAD_DEAD) // Dead but still being hit.
		return;

	// Regular death.
	self->pre_think = NULL;
	self->next_pre_think = -1.0f;

	self->gravity = 1.0f;
	self->svflags |= (SVF_TAKE_NO_IMPACT_DMG | SVF_DO_NO_IMPACT_DMG);

	self->s.skinnum = GORGON_PAIN_SKIN;
	gi.sound(self, CHAN_VOICE, sounds[SND_DIE], 1.0f, ATTN_NORM, 0.0f);
	self->dead_state = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	SetAnim(self, (self->health <= -10 ? ANIM_DIE2 : ANIM_DIE1)); // Big enough death to be thrown back?
}

static void GorgonCheckMoodMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'gorgon_check_mood' in original logic.
{
	G_ParseMsgParms(msg, "i", &self->ai_mood);
	gorgon_check_mood(self);
}

// Gorgon Evasion!
static void GorgonEvadeMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'gorgon_evade' in original logic.
{
	static const EvadeChance_t evade_chances[] = //mxd. Use struct.
	{
		{.duck_chance = 5,  .dodgeleft_chance = 10, .dodgeright_chance = 10, .jump_chance = 10, .backflip_chance = 10, .frontflip_chance = 10 }, // hl_NoneSpecific
		{.duck_chance = 30, .dodgeleft_chance = 50, .dodgeright_chance = 50, .jump_chance = 0,  .backflip_chance = 20, .frontflip_chance = 20 }, // hl_Head
		{.duck_chance = 20, .dodgeleft_chance = 40, .dodgeright_chance = 40, .jump_chance = 0,  .backflip_chance = 80, .frontflip_chance = 0  }, // hl_TorsoFront
		{.duck_chance = 20, .dodgeleft_chance = 40, .dodgeright_chance = 40, .jump_chance = 0,  .backflip_chance = 0,  .frontflip_chance = 80 }, // hl_TorsoBack
		{.duck_chance = 10, .dodgeleft_chance = 0,  .dodgeright_chance = 90, .jump_chance = 0,  .backflip_chance = 20, .frontflip_chance = 20 }, // hl_ArmUpperLeft
		{.duck_chance = 0,  .dodgeleft_chance = 0,  .dodgeright_chance = 80, .jump_chance = 30, .backflip_chance = 20, .frontflip_chance = 20 }, // hl_ArmLowerLeft
		{.duck_chance = 20, .dodgeleft_chance = 90, .dodgeright_chance = 0,  .jump_chance = 0,  .backflip_chance = 20, .frontflip_chance = 20 }, // hl_ArmUpperRight
		{.duck_chance = 0,  .dodgeleft_chance = 80, .dodgeright_chance = 0,  .jump_chance = 30, .backflip_chance = 20, .frontflip_chance = 20 }, // hl_ArmLowerRight
		{.duck_chance = 0,  .dodgeleft_chance = 0,  .dodgeright_chance = 60, .jump_chance = 50, .backflip_chance = 30, .frontflip_chance = 30 }, // hl_LegUpperLeft
		{.duck_chance = 0,  .dodgeleft_chance = 0,  .dodgeright_chance = 30, .jump_chance = 90, .backflip_chance = 60, .frontflip_chance = 60 }, // hl_LegLowerLeft
		{.duck_chance = 0,  .dodgeleft_chance = 60, .dodgeright_chance = 0,  .jump_chance = 50, .backflip_chance = 30, .frontflip_chance = 30 }, // hl_LegUpperRight
		{.duck_chance = 0,  .dodgeleft_chance = 30, .dodgeright_chance = 0,  .jump_chance = 90, .backflip_chance = 30, .frontflip_chance = 30 }, // hl_LegLowerRight
	};

	edict_t* projectile;
	HitLocation_t hl;
	float eta;
	G_ParseMsgParms(msg, "eif", &projectile, &hl, &eta);

	if (eta < 0.3f)
		return; // Needs at least 0.3 seconds to respond.

	//mxd. Get evade info.
	if (hl < hl_NoneSpecific || hl > hl_LegLowerRight)
		hl = hl_NoneSpecific;

	const EvadeChance_t* ec = &evade_chances[hl];

	if (irand(0, 100) < ec->frontflip_chance)
	{
		SetAnim(self, ANIM_MELEE8); // Hop forward.
		return;
	}

	if (irand(0, 100) < ec->backflip_chance)
	{
		if (self->curAnimID == ANIM_RUN1 && irand(0, 10) < 8) // Running, do the front jump.
			SetAnim(self, ANIM_MELEE10); // Jump forward.
		else
			SetAnim(self, ANIM_MELEE9); // Hop forward.

		return;
	}

	if (irand(0, 100) < ec->dodgeleft_chance)
	{
		SetAnim(self, ANIM_MELEE6); // Hop left.
		return;
	}

	if (irand(0, 100) < ec->dodgeright_chance)
	{
		SetAnim(self, ANIM_MELEE7); // Hop right.
		return;
	}

	if (self->jump_chance >= 0 && irand(0, 100) < ec->jump_chance)
	{
		SetAnim(self, ANIM_MELEE10); // Jump forward.
		return;
	}

	if (irand(0, 100) < ec->duck_chance)
		SetAnim(self, ANIM_PAIN1); // Jump forward.
}

static void GorgonJumpMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'gorgon_jump_msg' in original logic.
{
	if (self->jump_chance >= irand(0, 100))
		SetAnim(self, ANIM_FJUMP);
}

#pragma endregion

#pragma region ========================== Action functions ==========================

void gorgon_roar_sound(edict_t* self)
{
	const int chance = irand(0, 100);

	if (chance < 20)
		gi.sound(self, CHAN_VOICE, sounds[SND_PAIN1], 1.0f, ATTN_NORM, 0.0f);
	else if (chance < 40)
		gi.sound(self, CHAN_VOICE, sounds[SND_PAIN2], 1.0f, ATTN_NORM, 0.0f);
	else if (chance < 60)
		gi.sound(self, CHAN_VOICE, sounds[SND_DIE], 1.0f, ATTN_NORM, 0.0f);
	else
		gorgon_growl(self);
}

// Finds gorgons in immediate vicinity and wakes them up.
void gorgon_roar(edict_t* self) //mxd. Named 'gorgonRoar' in original logic.
{
	if (self->enemy == NULL)
		return;

	edict_t* e = NULL;

	while ((e = FindInRadius(e, self->s.origin, GORGON_ALERT_DIST)) != NULL)
	{
		if (e->health > 0 && e->enemy == NULL && (e->svflags & SVF_MONSTER) && e->classID == CID_GORGON && !e->monsterinfo.roared)
		{
			// Make sure they can hear me.
			if (gi.inPHS(self->s.origin, e->s.origin) && AI_OkToWake(e, true, true))
			{
				e->monsterinfo.roared = true;
				e->enemy = self->enemy;
				AI_FoundTarget(e, false);
				G_PostMessage(e, MSG_VOICE_POLL, PRI_DIRECTIVE, "");
			}
		}
	}
}

void gorgon_footstep(edict_t* self)
{
	gi.sound(self, CHAN_BODY, sounds[irand(SND_STEP1, SND_STEP4)], 1.0f, ATTN_NORM, 0.0f);
}

void gorgon_growl(edict_t* self)
{
	const int chance = irand(0, 100);

	if (chance < 10)
		gi.sound(self, CHAN_WEAPON, sounds[SND_GROWL1], 1.0f, ATTN_NORM, 0.0f);
	else if (chance < 20)
		gi.sound(self, CHAN_WEAPON, sounds[SND_GROWL2], 1.0f, ATTN_NORM, 0.0f);
	else if (chance < 30)
		gi.sound(self, CHAN_WEAPON, sounds[SND_GROWL3], 1.0f, ATTN_NORM, 0.0f);
}

void gorgon_bite(edict_t* self) //mxd. Named 'gorgonbite' in original logic.
{
	if (self->ai_mood == AI_MOOD_NAVIGATE || !AI_HaveEnemy(self))
		return;

	vec3_t forward;
	vec3_t up;
	AngleVectors(self->s.angles, forward, NULL, up);

	vec3_t melee_point;
	VectorMA(self->s.origin, self->maxs[2] * 0.5f, up, melee_point);
	VectorMA(melee_point, self->maxs[0], forward, melee_point);

	const float melee_range = self->s.scale * GORGON_MELEE_RANGE * 1.25f; // Give extra range.

	vec3_t bite_end_pos;
	VectorMA(melee_point, melee_range, forward, bite_end_pos);

	// Let's do this the right way.
	trace_t trace;
	gi.trace(melee_point, vec3_origin, vec3_origin, bite_end_pos, self, MASK_SHOT, &trace);

	if (trace.fraction < 1.0f && !trace.startsolid && !trace.allsolid && trace.ent->takedamage != DAMAGE_NO) // A hit.
	{
		gi.sound(self, CHAN_WEAPON, sounds[irand(SND_MELEEHIT1, SND_MELEEHIT2)], 1.0f, ATTN_NORM, 0.0f);

		vec3_t normal;
		VectorSubtract(self->enemy->s.origin, self->s.origin, normal);
		VectorNormalize(normal);

		const int damage = (int)((float)(irand(GORGON_DMG_MIN, GORGON_DMG_MAX)) * self->monsterinfo.scale);
		T_Damage(self->enemy, self, self, forward, trace.endpos, normal, damage, damage / 2, DAMAGE_EXTRA_BLOOD, MOD_DIED);

		//mxd. Set only when we actually damaged our target (original logic sets this when setting attack animation, which can result in gorgon approaching its target and just standing still if it happens too soon after previous attack animation (even unsuccessful one)).
		self->monsterinfo.attack_finished = level.time + flrand(0.0f, 3.0f - skill->value);
	}
	else // A miss.
	{
		gi.sound(self, CHAN_WEAPON, sounds[irand(SND_MELEEMISS1, SND_MELEEMISS2)], 1.0f, ATTN_NORM, 0.0f);
	}
}

void gorgon_dead(edict_t* self)
{
	self->pre_think = NULL;
	self->next_pre_think = -1.0f;
	self->dead_state = DEAD_DEAD;
	self->svflags |= SVF_DEADMONSTER;

	M_EndDeath(self);
}

void gorgon_death1_fall(edict_t* self) //mxd. Originally defined in m_gorgon_anim.c. Named 'gorgondeath1_fall' in original logic.
{
	if (self->s.frame == FRAME_deatha13)
		gorgon_land(self);

	M_walkmove(self, flrand(180.0f, 345.0f), flrand(-8.0f, 0.0f)); //mxd. M_movetoside() in original logic.
}

void gorgon_death2(edict_t* self) //mxd. Originally defined in m_gorgon_anim.c.
{
	self->velocity[2] = -120.0f;
}

void gorgon_death2_throw(edict_t* self) //mxd. Originally defined in m_gorgon_anim.c. Named 'gorgon_death2throw' in original logic. 
{
	vec3_t forward;
	AngleVectors(self->s.angles, forward, NULL, NULL);

	VectorScale(forward, -375.0f, self->velocity);
	self->velocity[2] = 200.0f;
}

void gorgon_death2_slide(edict_t* self) //mxd. Originally defined in m_gorgon_anim.c.
{
	self->monsterinfo.currentmove = &gorgon_move_death2slide; //TODO: add to animations[], set via SetAnim() instead?
	self->monsterinfo.nextframeindex = 0;
}

void gorgon_start_twitch(edict_t* self) //mxd. Originally defined in m_gorgon_anim.c. Named 'gorgon_starttwitch' in original logic.
{
	self->monsterinfo.thinkinc = MONSTER_THINK_INC;
}

void gorgon_next_twitch(edict_t* self) //mxd. Originally defined in m_gorgon_anim.c. Named 'gorgon_nexttwitch' in original logic.
{
	self->monsterinfo.thinkinc = FRAMETIME * 4.0f + flrand(0.0f, 4.0f);
}

void gorgon_land(edict_t* self)
{
	gi.sound(self, CHAN_WEAPON, sounds[SND_LAND], 1.0f, ATTN_NORM, 0.0f);
}

void gorgon_hop(edict_t* self)
{
#define JUMP_SCALE 2.5f //mxd. 1.5 in original logic.

	if (self->s.frame == FRAME_hop8)
	{
		self->velocity[2] = -10.0f;
		return;
	}

	if (self->monsterinfo.currentmove == &gorgon_move_melee6) // Hop left.
	{
		vec3_t angles = VEC3_INIT(self->s.angles);
		angles[YAW] -= 15.0f;

		vec3_t right;
		AngleVectors(angles, NULL, right, NULL);
		VectorScale(right, -50.0f * JUMP_SCALE, self->velocity); //mxd. Increase scaler (from -40).
	}
	else if (self->monsterinfo.currentmove == &gorgon_move_melee7) // Hop right.
	{
		vec3_t angles = VEC3_INIT(self->s.angles);
		angles[YAW] += 15.0f;

		vec3_t right;
		AngleVectors(angles, NULL, right, NULL);
		VectorScale(right, 50.0f * JUMP_SCALE, self->velocity); //mxd. Increase scaler (from 40).
	}
	else if (self->monsterinfo.currentmove == &gorgon_move_melee8) // Hop forward.
	{
		vec3_t forward;
		AngleVectors(self->s.angles, forward, NULL, NULL);
		VectorScale(forward, 75.0f * JUMP_SCALE, self->velocity); //mxd. Increase scaler (from 50), so it doesn't look like we are jumping in place.
	}
	else if (self->monsterinfo.currentmove == &gorgon_move_melee9) // Hop backward.
	{
		vec3_t forward;
		AngleVectors(self->s.angles, forward, NULL, NULL);
		VectorScale(forward, -50.0f * JUMP_SCALE, self->velocity);
	}

	self->velocity[2] += 175.0f;

	//mxd. Scale by s.scale.
	const float mass_scaler = LerpFloat(self->s.scale, 1.0f, 0.5f);
	Vec3ScaleAssign(mass_scaler, self->velocity);
}

void gorgon_apply_jump(edict_t* self) //mxd. Named 'gorgonApplyJump' in original logic.
{
	if (Vec3IsZero(self->movedir))
	{
		vec3_t forward;
		AngleVectors(self->s.angles, forward, NULL, NULL);

		VectorMA(self->velocity, flrand(200.0f, 400.0f), forward, self->velocity);
		self->velocity[2] = flrand(100.0f, 200.0f);
	}
	else
	{
		VectorCopy(self->movedir, self->velocity);
		VectorClear(self->movedir);
	}

	self->jump_time = level.time + 0.5f; //mxd. Done only in 'else' cause in original logic. 
}

void gorgon_jump_out_of_water(edict_t* self) //mxd. Named 'gorgonJumpOutWater' in original logic.
{
	vec3_t end_pos;

	if (self->enemy != NULL) //BUGFIX: mxd. 'if(!self->enemy)' in original logic.
	{
		VectorCopy(self->enemy->s.origin, end_pos);
	}
	else
	{
		vec3_t forward;
		AngleVectors(self->s.angles, forward, NULL, NULL);
		VectorMA(self->s.origin, 100.0f, forward, end_pos);
	}

	end_pos[2] += 30.0f;
	VectorSubtract(end_pos, self->s.origin, self->velocity);
	self->velocity[2] += 200.0f;
}

void gorgon_forward(edict_t* self, float dist) //mxd. Named 'gorgonForward' in original logic.
{
	vec3_t forward;
	AngleVectors(self->s.angles, forward, NULL, NULL);

	vec3_t fwd_dir;
	VectorNormalize2(self->velocity, fwd_dir);

	dist *= 1.0f - DotProduct(fwd_dir, forward);

	if (dist != 0.0f)
		VectorMA(self->velocity, dist, forward, self->velocity);
}

void gorgon_fix_pitch(edict_t* self) //mxd. Named 'gorgonFixPitch' in original logic.
{
	MG_ChangePitch(self, 0.0f, 10.0f);
}

void gorgon_reset_pitch(edict_t* self) //mxd. Named 'gorgonZeroPitch' in original logic.
{
	self->s.angles[PITCH] = 0.0f;
}

void gorgon_swim_go(edict_t* self) //mxd. Named 'gorgonGoSwim' in original logic.
{
	SetAnim(self, ANIM_SWIM);
}

void gorgon_check_in_water(edict_t* self) //mxd. Named 'gorgonCheckInWater' in original logic.
{
	trace_t trace;
	const vec3_t end_pos = VEC3_INITA(self->s.origin, 0.0f, 0.0f, -32.0f);
	gi.trace(self->s.origin, self->mins, self->maxs, end_pos, self, MASK_MONSTERSOLID, &trace);

	if (trace.fraction < 1.0f && ((trace.contents & CONTENTS_SOLID) || (trace.contents & CONTENTS_MONSTER)))
		SetAnim(self, ANIM_INAIR);
}

void gorgon_check_landed(edict_t* self)
{
	const float save_yaw_speed = self->yaw_speed;
	self->yaw_speed *= 0.33f;
	self->best_move_yaw = VectorYaw(self->velocity);
	MG_ChangeWhichYaw(self, false); // Turn toward best_move_yaw, 1/3 as fast as if on ground.
	self->yaw_speed = save_yaw_speed;

	if (self->groundentity != NULL)
	{
		SetAnim(self, irand(ANIM_LAND2, ANIM_LAND));
	}
	else if (self->velocity[2] < 0.0f)
	{
		vec3_t pos = VEC3_INITA(self->s.origin, 0.0f, 0.0f, self->mins[2]);
		VectorMA(pos, 0.5f, self->velocity, pos);

		const int contents = gi.pointcontents(pos);

		if (contents & CONTENTS_SOLID)
			SetAnim(self, irand(ANIM_LAND2, ANIM_LAND));
		else if (contents & CONTENTS_WATER)
			SetAnim(self, ANIM_TO_SWIM);
	}
}

void gorgon_inair_go(edict_t* self) //mxd. Named 'gorgon_go_inair' in original logic.
{
	SetAnim(self, ANIM_INAIR);
}

void gorgon_jump(edict_t* self)
{
	vec3_t landing_spot;

	if (!MG_TryGetTargetOrigin(self, landing_spot))
	{
		SetAnim(self, ((irand(0, 3) == 0) ? ANIM_ROAR2 : ANIM_RUN1));
		return;
	}

	vec3_t diff;
	VectorSubtract(self->s.origin, landing_spot, diff);

	if (VectorLength(diff) > 400.0f)
	{
		SetAnim(self, ((irand(0, 3) == 0) ? ANIM_ROAR2 : ANIM_RUN1));
		return;
	}

	vec3_t angles;

	if (self->enemy != NULL)
		VectorSet(angles, 0.0f, anglemod(-self->enemy->s.angles[YAW]), 0.0f);
	else
		VectorCopy(self->s.angles, angles);

	GorgonSetupJumpArc(self, angles, landing_spot);
}

// Gorgon picks up and gores something.
void gorgon_ready_catch(edict_t* self)
{
	if (!AI_HaveEnemy(self))
	{
		SetAnim(self, ANIM_CATCH);
		return;
	}

	float max_catch_zdist = 128.0f * (self->s.scale * 0.5f / 2.5f);
	max_catch_zdist = max(48.0f, max_catch_zdist);

	const float enemy_zdist = self->enemy->absmin[2] - self->absmax[2];

	if (enemy_zdist <= 0.0f || (enemy_zdist <= max_catch_zdist && self->enemy->velocity[2] <= -60.0f))
		SetAnim(self, ANIM_CATCH);
	else
		SetAnim(self, ANIM_READY_CATCH);
}

//mxd. Similar to tbeast_throw_toy().
void gorgon_throw_toy(edict_t* self)
{
	if (self->enemy == NULL)
		return;

	self->enemy->flags &= ~FL_FLY;
	VectorSet(self->enemy->velocity, 0.0f, 0.0f, 500.0f);

	if (self->enemy->movetype > NUM_PHYSICSTYPES) //TODO: Eh? Should check for PHYSICSTYPE_FLY instead?..
		self->enemy->movetype = PHYSICSTYPE_STEP;

	VectorRandomCopy(vec3_origin, self->enemy->avelocity, 300.0f);

	if (Q_stricmp(self->enemy->classname, "player") != 0) //TODO: strange way to check for non-players. Should check self->targetEnt->client instead?..
		G_PostMessage(self->enemy, MSG_DEATH, PRI_DIRECTIVE, NULL);

	//TODO: play throw sound?
}

//mxd. Similar to tbeast_shake_toy().
void gorgon_shake_toy(edict_t* self, float forward_offset, float right_offset, float up_offset) //mxd. Named 'gorgon_toy_ofs' in original logic.
{
	if (self->enemy == NULL)
		return;

	// Adjust for scale.
	forward_offset *= self->s.scale * 0.5f / 2.5f;
	right_offset *= self->s.scale * 0.5f / 2.5f;
	up_offset += self->mins[2]; // Because origin moved up since those were calculated.
	up_offset *= self->s.scale * 0.25f / 2.5f;

	vec3_t forward;
	vec3_t right;
	vec3_t up;
	AngleVectors(self->s.angles, forward, right, up);

	vec3_t enemy_offset;
	VectorMA(self->s.origin, forward_offset, forward, enemy_offset);
	VectorMA(enemy_offset, right_offset, right, enemy_offset);
	VectorMA(enemy_offset, up_offset, up, self->enemy->s.origin);

	vec3_t blood_dir;
	VectorSubtract(self->enemy->s.origin, self->s.origin, blood_dir);

	vec3_t enemy_dir;
	VectorScale(blood_dir, -1.0f, enemy_dir);
	enemy_dir[2] /= 10.0f;
	vectoangles(enemy_dir, self->enemy->s.angles);

	switch (self->enemy->gorgon_grabbed_toy_shake_mode)
	{
		case 1:
			self->enemy->s.angles[PITCH] = anglemod(self->enemy->s.angles[PITCH] + 90.0f); // Can't do roll?
			break;

		case 2:
			self->enemy->s.angles[PITCH] = anglemod(self->enemy->s.angles[PITCH] - 90.0f); // Can't do roll?
			break;

		case 3:
			self->enemy->s.angles[ROLL] = anglemod(self->enemy->s.angles[ROLL] + 90.0f); // Can't do roll?
			break;

		case 4:
			self->enemy->s.angles[ROLL] = anglemod(self->enemy->s.angles[ROLL] - 90.0f); // Can't do roll?
			break;

		default:
			break;
	}

	VectorClear(self->enemy->velocity);
	VectorClear(self->enemy->avelocity);

	if (flrand(0.0f, 1.0f) < 0.5f)
	{
		const int fx_flags = ((self->enemy->materialtype == MAT_INSECT) ? CEF_FLAG8 : 0); //mxd
		gi.CreateEffect(&self->enemy->s, FX_BLOOD, fx_flags, self->enemy->s.origin, "ub", blood_dir, 200);
	}
}

//mxd. Similar to tbeast_check_snatch().
void gorgon_check_snatch(edict_t* self, float forward_offset, float right_offset, float up_offset)
{
	// Adjust for scale.
	float max_snatch_zdist = 64.0f * (self->s.scale * 0.5f / 2.5f);
	max_snatch_zdist = max(24.0f, max_snatch_zdist);

	vec3_t forward;
	vec3_t right;
	vec3_t up;
	AngleVectors(self->s.angles, forward, right, up);

	vec3_t start_pos;
	VectorMA(self->s.origin, self->maxs[0], forward, start_pos);
	VectorMA(start_pos, self->maxs[2] * 0.5f, up, start_pos);

	vec3_t end_pos;
	VectorCopy(start_pos, end_pos);
	VectorMA(end_pos, forward_offset, forward, end_pos);
	VectorMA(end_pos, right_offset, right, end_pos);
	VectorMA(end_pos, up_offset, up, end_pos);

	trace_t trace;
	gi.trace(start_pos, vec3_origin, vec3_origin, end_pos, self, MASK_SHOT, &trace);
	VectorCopy(trace.endpos, end_pos);

	vec3_t diff;
	VectorSubtract(self->enemy->s.origin, end_pos, diff);

	if (VectorLength(diff) > max_snatch_zdist)
	{
		self->msgHandler = DefaultMsgHandler;

		if (Q_stricmp(self->enemy->classname, "player") == 0 && self->oldenemy != NULL && self->oldenemy->health > 0) //TODO: check for self->enemy->client instead of Q_stricmp?
		{
			self->oldenemy = NULL;
			self->enemy = self->oldenemy; //TODO: Ehhhhh? Shouldn't we clear self->oldenemy AFTER we assigned it to self->enemy?..
		}

		if (self->curAnimID == ANIM_SNATCHLOW)
			SetAnim(self, ANIM_MISS);
		else
			SetAnim(self, ANIM_MELEE2); //?

		return;
	}

	//FIXME: if health is low, just chomp it now.
	self->enemy->flags |= FL_FLY;

	if (self->enemy->movetype > NUM_PHYSICSTYPES) //TODO: why this check? tbeast_check_snatch() just assigns movetype.
		self->enemy->movetype = PHYSICSTYPE_FLY;

	if (Q_stricmp(self->enemy->classname, "player") != 0) //TODO: check for !self->enemy->client instead?
	{
		self->enemy->monsterinfo.aiflags |= AI_DONT_THINK;
		self->enemy->gorgon_grabbed_toy_shake_mode = irand(1, 4); //BUGFIX: mxd. irand(1, 5) in original logic. Case 5 is not handled.
	}
	else
	{
		self->enemy->nextthink = level.time + 10.0f; // Stuck for 10 seconds.
	}

	VectorClear(self->enemy->velocity);
	VectorClear(self->enemy->avelocity);
}

//mxd. Similar to tbeast_gore_toy().
void gorgon_gore_toy(edict_t* self, float jump_height)
{
	const qboolean last_frame = (jump_height == -1.0f); //mxd

	if (!last_frame)
	{
		// Not getting here.
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
		self->gorgon_grabbed_toy_shake_mode = 0;
	}

	if (self->enemy == NULL || self->enemy->health < 0 || self->gorgon_grabbed_toy_shake_mode != 0)
		return;

	float max_zdist = 56.0f * (self->s.scale * 0.5f / 2.5f);
	max_zdist = max(36.0f, max_zdist);

	const float enemy_zdist = self->enemy->s.origin[2] - self->s.origin[2];

	if (last_frame || enemy_zdist <= self->maxs[2] + max_zdist)
	{
		gi.sound(self, CHAN_WEAPON, sounds[SND_MELEEMISS2], 1.0f, ATTN_NORM, 0.0f);

		if (!last_frame)
			self->gorgon_grabbed_toy_shake_mode = 1;

		vec3_t dir;
		VectorCopy(self->velocity, dir);
		VectorNormalize(dir);

		self->enemy->materialtype = MAT_FLESH;

		const int num_chunks = min(15, self->enemy->health / 4);
		SprayDebris(self->enemy, self->enemy->s.origin, num_chunks, (float)self->enemy->health * 4.0f); //self->enemy is thingtype wood?!

		if (Q_stricmp(self->enemy->classname, "player") != 0) //mxd. stricmp -> Q_stricmp //TODO: check for enemy->client instead?
		{
			gi.sound(self->enemy, CHAN_BODY, sounds[SND_GIB], 1.0f, ATTN_NORM, 0.0f);
			BecomeDebris(self->enemy);
		}
		else
		{
			self->enemy->nextthink = level.time + FRAMETIME; //mxd. Add FRAMETIME.
			T_Damage(self->enemy, self, self, self->velocity, self->enemy->s.origin, dir, 2000, 300, 0, MOD_DIED);
		}
	}
}

void gorgon_miss_sound(edict_t* self)
{
	gi.sound(self, CHAN_WEAPON, sounds[irand(SND_MELEEMISS1, SND_MELEEMISS2)], 1.0f, ATTN_NORM, 0.0f);
}

void gorgon_anger_sound(edict_t* self)
{
	const int chance = irand(0, 100);

	if (chance < 10)
		gi.sound(self, CHAN_VOICE, sounds[SND_PAIN1], 1.0f, ATTN_NORM, 0.0f);
	else if (chance < 20)
		gi.sound(self, CHAN_VOICE, sounds[SND_PAIN2], 1.0f, ATTN_NORM, 0.0f);
	else if (chance < 30)
		gi.sound(self, CHAN_WEAPON, sounds[SND_MELEEHIT1], 1.0f, ATTN_NORM, 0.0f);
	else if (chance < 40)
		gi.sound(self, CHAN_WEAPON, sounds[SND_MELEEHIT2], 1.0f, ATTN_NORM, 0.0f);
	else if (chance < 50)
		gi.sound(self, CHAN_VOICE, sounds[SND_DIE], 1.0f, ATTN_NORM, 0.0f);
	else if (chance < 60)
		gorgon_growl(self);

	if (self->enemy != NULL)
	{
		G_PostMessage(self->enemy, MSG_DISMEMBER, PRI_DIRECTIVE, "ii", self->enemy->health / 2, irand(1, 13)); // Do I need last three if not sending them?
		G_PostMessage(self->enemy, MSG_PAIN, PRI_DIRECTIVE, "eeiii", self, self, true, GORGON_DMG_MAX, 0); //BUGFIX. mxd. Original logic sends 'ii' args here. //TODO: check damage amount. Original logic doesn't specify any...
	}
}

void gorgon_snatch_go(edict_t* self) //mxd. Named 'gorgon_go_snatch' in original logic.
{
	SetAnim(self, ANIM_SNATCH);
}

void gorgon_done_gore(edict_t* self)
{
	self->msgHandler = DefaultMsgHandler;
	self->gorgon_grabbed_toy_shake_mode = 0;

	if (self->oldenemy != NULL && self->oldenemy->health > 0)
	{
		self->enemy = self->oldenemy;
		G_PostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
	}
	else
	{
		SetAnim(self, ANIM_EAT_LOOP);
	}
}

// Trip and fall if making too tight a turn, won't work until origin is in center.
void gorgon_set_roll(edict_t* self, float roll_angle) //mxd. Named 'gorgonRoll' in original logic.
{
	self->s.angles[ROLL] = anglemod(roll_angle); //FIXME: cannot interrupt!!!
}

void gorgon_lerp_off(edict_t* self) //mxd. Named 'gorgonLerpOff' in original logic.
{
	self->s.renderfx &= ~RF_FRAMELERP;
}

void gorgon_lerp_on(edict_t* self) //mxd. Named 'gorgonLerpOn' in original logic.
{
	self->s.renderfx |= RF_FRAMELERP;
}

void gorgon_check_slip(edict_t* self) //mxd. Named 'gorgonCheckSlip' in original logic.
{
	if ((!(self->spawnflags & MSF_GORGON_SPEEDY) && self->s.scale > 0.75f) || !GorgonStartSlipAnimation(self, false))
		gorgon_check_mood(self);
}

void gorgon_slide(edict_t* self, float force) //mxd. Named 'gorgonSlide' in original logic.
{
	if (self->groundentity == NULL)
		return; // Already in air.

	if (force == 0.0f)
	{
		self->friction = 1.0f;
		return;
	}

	vec3_t right;
	AngleVectors(self->s.angles, NULL, right, NULL);
	VectorMA(self->velocity, force, right, self->velocity);
	self->velocity[2] = 50.0f;

	self->friction = 0.2f;
}

void gorgon_ai_run(edict_t* self, float distance) //mxd. Originally defined in m_gorgon_anim.c.
{
#define GORGON_SWERVE		20 // Degree of swerve.
#define GORGON_SWERVE_INT1	(GORGON_SWERVE / 4)
#define GORGON_SWERVE_INT2	(GORGON_SWERVE / 2)
#define GORGON_SWERVE_INT3	(GORGON_SWERVE - GORGON_SWERVE_INT1)
#define GORGON_SWERVE_INT4	GORGON_SWERVE

#define GORGON_SWERVE_MULT	(GORGON_SWERVE / GORGON_SWERVE_INT1)

	MG_FaceGoal(self, false);

	if (self->monsterinfo.idle_time != -1.0f && self->monsterinfo.idle_time < level.time)
	{
		self->gorgon_swerve_step++;

		float offset;

		if (self->gorgon_swerve_step < GORGON_SWERVE_INT1)
		{
			offset = (float)self->gorgon_swerve_step * GORGON_SWERVE_MULT;
		}
		else if (self->gorgon_swerve_step < GORGON_SWERVE_INT2)
		{
			offset = GORGON_SWERVE - (((float)self->gorgon_swerve_step - GORGON_SWERVE_INT1) * GORGON_SWERVE_MULT);
		}
		else if (self->gorgon_swerve_step < GORGON_SWERVE_INT3)
		{
			offset = -(((float)self->gorgon_swerve_step - GORGON_SWERVE_INT2) * GORGON_SWERVE_MULT);
		}
		else if (self->gorgon_swerve_step < GORGON_SWERVE_INT4)
		{
			offset = -(GORGON_SWERVE - ((float)self->gorgon_swerve_step - GORGON_SWERVE_INT3) * GORGON_SWERVE_MULT);
		}
		else
		{
			offset = 0.0f; //mxd. Initialize to avoid compiler warning.
			self->gorgon_swerve_step = 0;
		}

		if (self->gorgon_swerve_step != 0)
			self->ideal_yaw = anglemod(self->ideal_yaw + offset);
	}

	const float scaler = ((self->spawnflags & MSF_GORGON_SPEEDY) ? 1.25f : 0.75f); //mxd
	MG_AI_Run(self, distance * scaler);
}

void gorgon_ai_swim(edict_t* self, float distance)
{
	const qboolean do_charge = (distance == -1.0f); //mxd

	GorgonPreThink(self);
	self->pre_think = GorgonPreThink;
	self->next_pre_think = level.time + FRAMETIME; //mxd. Use define.

	self->mood_think(self);

	vec3_t dir;
	MG_SetNormalizedVelToGoal(self, dir);

	if (Vec3IsZero(dir))
	{
		Vec3ScaleAssign(0.8f, self->velocity);
		return;
	}

	self->ideal_yaw = VectorYaw(dir);
	MG_ChangeYaw(self);

	if (do_charge)
		Vec3ScaleAssign(150.0f, dir);
	else
		Vec3ScaleAssign(distance * 3.0f, dir);

	VectorAdd(self->velocity, dir, self->velocity);
	VectorNormalize(self->velocity);
	Vec3ScaleAssign(200.0f, self->velocity);

	if (self->enemy == NULL)
		return;

	vec3_t diff;
	VectorSubtract(self->enemy->s.origin, self->s.origin, diff);

	vec3_t angles;
	vectoangles(diff, angles);

	MG_ChangePitch(self, angles[PITCH], 10.0f);

	if (do_charge)
		return;

	if (self->monsterinfo.attack_finished < level.time && M_ValidTarget(self, self->enemy) && AI_IsClearlyVisible(self, self->enemy) && AI_IsInfrontOf(self, self->enemy))
	{
		const float dist = VectorLength(diff);

		if (dist < self->melee_range + VectorLength(self->velocity) * 0.1f)
			SetAnim(self, irand(ANIM_SWIM_BITE_A, ANIM_SWIM_BITE_B));
		else if (self->monsterinfo.jump_time < level.time && !(self->enemy->flags & FL_INWATER) && dist < GORGON_MAX_HOP_RANGE * 2.0f)
			SetAnim(self, ANIM_OUT_WATER);
	}
}

void gorgon_ai_eat(edict_t* self, float switch_animation)
{
	//FIXME: 'switch_animation' will be a yaw mod for view_ofs looking around.
	if (self->enemy != NULL)
	{
		vec3_t diff;
		VectorSubtract(self->enemy->s.origin, self->s.origin, diff);

		if (VectorNormalize(diff) < self->wakeup_distance)
		{
			if (AI_IsVisible(self, self->enemy))
			{
				self->spawnflags &= ~MSF_EATING;
				self->monsterinfo.aiflags &= ~AI_EATING;
				AI_FoundTarget(self, true);

				return;
			}
		}
		else if (self->curAnimID == ANIM_EAT_LOOP && irand(0, 5) == 0)
		{
			if (AI_IsVisible(self, self->enemy))
			{
				vec3_t forward;
				vec3_t right;
				AngleVectors(self->s.angles, forward, right, NULL);
				const float fwd_dot = DotProduct(diff, forward);

				if (fwd_dot < 0.0f)
				{
					const float right_dot = DotProduct(diff, right);

					if (right_dot > 0.3f)
						SetAnim(self, ANIM_EAT_RIGHT);
					if (right_dot < -0.3f)
						SetAnim(self, ANIM_EAT_LEFT);
					else
						SetAnim(self, ANIM_EAT_UP);

					return;
				}
			}
		}
	}
	else
	{
		FindTarget(self);
	}

	if (switch_animation != -1.0f)
		return;

	switch (self->curAnimID)
	{
		case ANIM_EAT_DOWN:
		case ANIM_EAT_TEAR:
		case ANIM_EAT_PULLBACK:
		case ANIM_EAT_LEFT:
		case ANIM_EAT_RIGHT:
		case ANIM_EAT_SNAP:
		case ANIM_EAT_REACT:
			SetAnim(self, ANIM_EAT_LOOP);
			break;

		case ANIM_EAT_UP:
			SetAnim(self, ANIM_LOOK_AROUND);
			break;

		case ANIM_LOOK_AROUND:
			SetAnim(self, ANIM_EAT_DOWN);
			break;

		case ANIM_EAT_LOOP:
			if (irand(0, 1) == 1)
				SetAnim(self, ANIM_EAT_LOOP);
			else if (irand(0, 1) == 1)
				SetAnim(self, ANIM_EAT_PULLBACK);
			else if (irand(0, 1) == 1)
				SetAnim(self, ANIM_EAT_TEAR);
			else if (irand(0, 1) == 1) //FIXME: check gorgon to right.
				SetAnim(self, ANIM_EAT_SNAP);
			else if (irand(0, 1) == 1) //FIXME: check if gorgon to left snapped.
				SetAnim(self, ANIM_EAT_REACT);
			else if (irand(0, 1) == 1) //FIXME: check enemy.
				SetAnim(self, ANIM_EAT_LEFT);
			else //FIXME: check enemy.
				SetAnim(self, ANIM_EAT_RIGHT);
			break;
	}
}

void gorgon_ai_charge2(edict_t* self, float distance) //mxd. Originally defined in m_gorgon_anim.c.
{
	if (AI_IsVisible(self, self->enemy))
		ai_charge2(self, distance);
}

void gorgon_check_mood(edict_t* self) //mxd. Named 'gorgonCheckMood' in original logic.
{
	self->pre_think = GorgonPreThink;
	self->next_pre_think = level.time + FRAMETIME; //mxd. Use define.

	self->mood_think(self);

	if (self->ai_mood == AI_MOOD_NORMAL)
		return;

	switch (self->ai_mood)
	{
		case AI_MOOD_ATTACK: // Melee and missile handlers are the same.
			G_PostMessage(self, MSG_MELEE, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_PURSUE:
			G_PostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_WALK:
			G_PostMessage(self, MSG_WALK, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_STAND:
			G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_DELAY:
			SetAnim(self, ANIM_DELAY);
			break;

		case AI_MOOD_NAVIGATE:
		case AI_MOOD_WANDER:
		case AI_MOOD_FLEE:
			if (self->flags & FL_INWATER)
				gorgon_swim_go(self);
			else if (self->curAnimID != ANIM_RUN1 && self->curAnimID != ANIM_RUN2 && self->curAnimID != ANIM_RUN3)
				SetAnim(self, ANIM_RUN1);
			break;

		case AI_MOOD_JUMP:
			SetAnim(self, ((self->jump_chance < irand(0, 100)) ? ANIM_DELAY : ANIM_FJUMP));
			break;

		case AI_MOOD_EAT: //FIXME: this is not necessary?
			gorgon_ai_eat(self, 0);
			break;

		default:
			break;
	}
}

void gorgon_under_water_wake(edict_t* self) //mxd
{
	gi.CreateEffect(&self->s, FX_M_EFFECTS, CEF_OWNERS_ORIGIN, self->s.origin, "bv", FX_UNDER_WATER_WAKE, vec3_origin);
}

// Gorgon melee while running.
void gorgon_melee5check(edict_t* self) //mxd. Originally defined in m_gorgon_anim.c.
{
	if (self->monsterinfo.currframeindex == FRAME_atka1 || self->monsterinfo.currframeindex == FRAME_atkb1) //TODO: never executed? Called on FRAME_runatk1 and FRAME_runatk5. Replace with gorgon_footstep()?
		gorgon_footstep(self);
}

#pragma endregion

void GorgonStaticsInit(void)
{
	static ClassResourceInfo_t res_info; //mxd. Made local static.

	classStatics[CID_GORGON].msgReceivers[MSG_STAND] = GorgonStandMsgHandler;
	classStatics[CID_GORGON].msgReceivers[MSG_WALK] = GorgonWalkMsgHandler;
	classStatics[CID_GORGON].msgReceivers[MSG_RUN] = GorgonRunMsgHandler;
	classStatics[CID_GORGON].msgReceivers[MSG_EAT] = GorgonEatMsgHandler;
	classStatics[CID_GORGON].msgReceivers[MSG_MELEE] = GorgonMeleeMsgHandler;
	classStatics[CID_GORGON].msgReceivers[MSG_MISSILE] = GorgonMeleeMsgHandler;
	classStatics[CID_GORGON].msgReceivers[MSG_WATCH] = GorgonWalkMsgHandler;
	classStatics[CID_GORGON].msgReceivers[MSG_PAIN] = GorgonPainMsgHandler;
	classStatics[CID_GORGON].msgReceivers[MSG_DEATH] = GorgonDeathMsgHandler;
	classStatics[CID_GORGON].msgReceivers[MSG_JUMP] = GorgonJumpMsgHandler;
	classStatics[CID_GORGON].msgReceivers[MSG_DEATH_PAIN] = GorgonDeathPainMsgHandler;
	classStatics[CID_GORGON].msgReceivers[MSG_CHECK_MOOD] = GorgonCheckMoodMsgHandler;
	classStatics[CID_GORGON].msgReceivers[MSG_VOICE_POLL] = GorgonVoicePollMsgHandler;
	classStatics[CID_GORGON].msgReceivers[MSG_EVADE] = GorgonEvadeMsgHandler;

	res_info.numAnims = NUM_ANIMS;
	res_info.animations = animations;
	res_info.modelIndex = gi.modelindex("models/monsters/gorgon/tris.fm");

	sounds[SND_PAIN1] = gi.soundindex("monsters/gorgon/pain1.wav");
	sounds[SND_PAIN2] = gi.soundindex("monsters/gorgon/pain2.wav");
	sounds[SND_DIE] = gi.soundindex("monsters/gorgon/death1.wav");
	//sounds[SND_GURGLE] = gi.soundindex("monsters/gorgon/gurgle.wav");	// What is this for?
	sounds[SND_GIB] = gi.soundindex("monsters/gorgon/gib.wav");

	sounds[SND_MELEEHIT1] = gi.soundindex("monsters/gorgon/meleehit1.wav");
	sounds[SND_MELEEHIT2] = gi.soundindex("monsters/gorgon/meleehit2.wav");
	sounds[SND_MELEEMISS1] = gi.soundindex("monsters/gorgon/meleemiss1.wav");
	sounds[SND_MELEEMISS2] = gi.soundindex("monsters/gorgon/meleemiss2.wav");

	sounds[SND_STEP1] = gi.soundindex("monsters/gorgon/footstep1.wav");
	sounds[SND_STEP2] = gi.soundindex("monsters/gorgon/footstep2.wav");
	sounds[SND_STEP3] = gi.soundindex("monsters/gorgon/footstep3.wav");
	sounds[SND_STEP4] = gi.soundindex("monsters/gorgon/footstep4.wav");

	sounds[SND_GROWL1] = gi.soundindex("monsters/gorgon/growl1.wav");
	sounds[SND_GROWL2] = gi.soundindex("monsters/gorgon/growl2.wav");
	sounds[SND_GROWL3] = gi.soundindex("monsters/gorgon/growl3.wav");

	sounds[SND_LAND] = gi.soundindex("monsters/gorgon/land.wav");

	res_info.numSounds = NUM_SOUNDS;
	res_info.sounds = sounds;

	classStatics[CID_GORGON].resInfo = &res_info;
}

// QUAKED monster_gorgon_leader (1 .5 0) (-16 -16 -0) (16 16 32) AMBUSH ASLEEP EATING 8 16 32 64 128
// The gorgon leader (not implemented).
void SP_monster_gorgon_leader(edict_t* self)
{
	G_SetToFree(self);
}

// QUAKED monster_gorgon (1 .5 0) (-16 -16 0) (16 16 32) AMBUSH ASLEEP EATING SPEEDY 16 32 64 128 WANDER MELEE_LEAD STALK COWARD EXTRA1 EXTRA2 EXTRA3 EXTRA4
// The gorgon.

// Spawnflags:
// AMBUSH		- Will not be woken up by other monsters or shots from player.
// ASLEEP		- Will not appear until triggered.
// EATING		- Chomp chomp... chewie chomp (wakeup_distance will default to 300).
// SPEEDY		- Generally faster gorgon.
// WANDER		- Monster will wander around aimlessly (but follows buoys).
// MELEE_LEAD	- Monster will try to cut you off when you're running and fighting him, works well if there are a few monsters in a group, half doing this, half not.
// STALK		- Monster will only approach and attack from behind. If you're facing the monster it will just stand there.
//				  Once the monster takes pain, however, it will stop this behaviour and attack normally.
// COWARD		- Monster starts off in flee mode (runs away from you when woken up).

// Variables:
// homebuoy					- Monsters will head to this buoy if they don't have an enemy ("homebuoy" should be targetname of the buoy you want them to go to).
// wakeup_target			- Monsters will fire this target the first time it wakes up (only once).
// pain_target				- Monsters will fire this target the first time it gets hurt (only once).
// mintel					- Monster intelligence - this basically tells a monster how many buoys away an enemy has to be for it to give up (default 20).
// melee_range				- How close the player has to be for the monster to go into melee. If this is zero, the monster will never melee.
//							  If it is negative, the monster will try to keep this distance from the player.
//							  If the monster has a backup, he'll use it if too close, otherwise, a negative value here means the monster will just stop
//							  running at the player at this distance (default 48).
//							 Examples:
//								melee_range = 60 - monster will start swinging it player is closer than 60.
//								melee_range = 0 - monster will never do a melee attack.
//								melee_range = -100 - monster will never do a melee attack and will back away (if it has that ability) when player gets too close.
// missile_range			- Maximum distance the player can be from the monster to be allowed to use it's ranged attack (default 0).
// min_missile_range		- Minimum distance the player can be from the monster to be allowed to use it's ranged attack (default 0).
// bypass_missile_chance	- Chance that a monster will NOT fire it's ranged attack, even when it has a clear shot. This, in effect, will make the monster
//							  come in more often than hang back and fire. A percentage (0 = always fire/never close in, 100 = never fire/always close in) - must be whole number (default 0).
// jump_chance				- Every time the monster has the opportunity to jump, what is the chance (out of 100) that he will... (100 = jump every time) - must be whole number (default 80).
// wakeup_distance			- How far (max) the player can be away from the monster before it wakes up. This means that if the monster can see the player,
//							  at what distance should the monster actually notice him and go for him (default 1024).
// NOTE: A value of zero will result in defaults, if you actually want zero as the value, use -1.
void SP_monster_gorgon(edict_t* self)
{
	// Fix some spawnflags.
	if (self->spawnflags & MSF_GORGON_COWARD)
	{
		self->spawnflags &= ~MSF_GORGON_COWARD;
		self->spawnflags |= MSF_COWARD;
	}

	// Generic Monster Initialization.
	if (!M_WalkmonsterStart(self)) // Failed initialization.
		return;

	self->msgHandler = DefaultMsgHandler;
	self->materialtype = MAT_FLESH;

	self->mass = GORGON_MASS;
	self->yaw_speed = ((self->spawnflags & MSF_GORGON_SPEEDY) ? 30.0f : 15.0f);
	self->gorgon_swerve_step = 0; // Used for slight turn during run.
	self->gorgon_grabbed_toy_shake_mode = 0; //mxd. Initialize.
	self->gorgon_is_underwater = false; //mxd. Initialize. //TODO: check if spawned underwater & set to true?

	self->movetype = PHYSICSTYPE_STEP;
	VectorClear(self->knockbackvel);
	self->solid = SOLID_BBOX;

	if (irand(0, 1) == 1)
		self->ai_mood_flags |= AI_MOOD_FLAG_PREDICT;

	VectorCopy(STDMinsForClass[self->classID], self->mins);
	VectorCopy(STDMaxsForClass[self->classID], self->maxs);

	self->s.modelindex = (byte)classStatics[CID_GORGON].resInfo->modelIndex;
	self->s.skinnum = GORGON_SKIN;
	self->monsterinfo.otherenemyname = "monster_rat";

	if (self->spawnflags & MSF_COWARD)
	{
		if (self->health == 0)
			self->health = GORGON_HEALTH / 2;

		self->monsterinfo.aiflags |= AI_COWARD;
		self->s.scale = 0.5f;
	}
	else
	{
		if (self->health == 0)
			self->health = GORGON_HEALTH;

		if (self->s.scale == 0.0f)
			self->s.scale = flrand(GORGON_SCALE_MIN, GORGON_SCALE_MAX);
	}

	self->monsterinfo.scale = self->s.scale;

	self->health = MonsterHealth(self->health);
	self->max_health = self->health;

	if (self->spawnflags & MSF_EATING)
	{
		self->monsterinfo.aiflags |= AI_EATING;
		G_PostMessage(self, MSG_EAT, PRI_DIRECTIVE, NULL);

		if (self->wakeup_distance == 0.0f)
			self->wakeup_distance = 300.0f;
	}
	else
	{
		G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
	}

	MG_InitMoods(self);

	self->svflags |= SVF_WAIT_NOTSOLID;
	self->flags |= FL_AMPHIBIAN;
	self->monsterinfo.aiflags |= AI_SWIM_OK;
	self->monsterinfo.roared = false;
	self->gorgon_wakeup_roar = (irand(0, 2) == 0); // 33% chance of not making a wakeup roar.

	self->pre_think = GorgonPreThink;
	self->next_pre_think = level.time + FRAMETIME; //mxd. Use define.
}