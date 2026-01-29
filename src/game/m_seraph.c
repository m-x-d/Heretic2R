//
// m_seraph.c
//
// Copyright 1998 Raven Software
//

#include "m_seraph.h"
#include "m_seraph_shared.h"
#include "m_seraph_anim.h"
#include "m_seraph_moves.h"
#include "g_debris.h" //mxd
#include "g_DefaultMessageHandler.h"
#include "mg_ai.h" //mxd
#include "mg_guide.h" //mxd
#include "m_stats.h"
#include "Random.h"
#include "Utilities.h"
#include "Vector.h"
#include "g_monster.h"
#include "g_local.h"

#define OVERLORD_ENFORCE_RADIUS	1000.0f //FIXME: Tweak out. //mxd. Named 'OVERLORD_RADIUS' in original logic.

#pragma region ========================== Seraph Base Info ==========================

static const animmove_t* animations[NUM_ANIMS] =
{
	&seraph_move_walk1,
	&seraph_move_walk2,
	&seraph_move_whip1,
	&seraph_move_whip1_loop,
	&seraph_move_whip1_end,
	&seraph_move_stand1,
	&seraph_move_stand1_tr,
	&seraph_move_stand1_r,
	&seraph_move_stand1_trc,
	&seraph_move_stand1_tl,
	&seraph_move_stand1_l,
	&seraph_move_stand1_tlc,
	&seraph_move_point1,
	&seraph_move_run1,
	&seraph_move_fjump,
	&seraph_move_run1_whip,
	&seraph_move_pain,
	&seraph_move_swipe,
	&seraph_move_get2work,
	&seraph_move_get2work2,
	&seraph_move_startle,
	&seraph_move_ready2idle,
	&seraph_move_backup,
	&seraph_move_death1,
	&seraph_move_death2_go,
	&seraph_move_death2_loop,
	&seraph_move_death2_end,
	&seraph_move_backup2,
};

static int sounds[NUM_SOUNDS];

#pragma endregion

#pragma region ========================== Utility functions =========================

// Check to see if you can make it to an idle ogle and scare them.
static qboolean SeraphCheckScare(edict_t* self, edict_t* ogle) //mxd. Named 'seraph_checkscare' in original logic.
{
	//FIXME: refers to goalentity.
	vec3_t self_fwd;
	AngleVectors(self->s.angles, self_fwd, NULL, NULL);

	vec3_t ogle_fwd;
	AngleVectors(ogle->s.angles, ogle_fwd, NULL, NULL);

	// Only do it if you're in back of them, and they're facing away.
	if (DotProduct(self_fwd, ogle_fwd) <= 0.0f)
		return false;

	vec3_t mins;
	VectorCopy(self->mins, mins);
	mins[2] += 18.0f; // Account for step ability.

	trace_t trace;
	gi.trace(self->s.origin, mins, self->maxs, ogle->s.origin, self, MASK_MONSTERSOLID, &trace);

	if (trace.ent != ogle)
		return false;

	// Necessary info for the AI_MOOD_POINT_NAVIGATE stuff.
	VectorCopy(self->s.origin, self->monsterinfo.nav_goal);
	self->old_yaw = self->s.angles[YAW]; //TODO: set, but never used.
	self->ai_mood = AI_MOOD_POINT_NAVIGATE;
	self->ai_mood_flags |= AI_MOOD_FLAG_IGNORE;

	self->movetarget = ogle;
	self->goalentity = ogle;
	self->enemy = ogle;

	// Tells the other Seraphs not to try and get this one too.
	ogle->targeted = true;

	return true;
}

// Check the ogles and make sure their noses are to the grind stone.
static void SeraphOversee(edict_t* self) //mxd. Named 'seraph_oversee' in original logic.
{
	edict_t* ogle = NULL;
	while ((ogle = FindInRadius(ogle, self->s.origin, OVERLORD_ENFORCE_RADIUS)) != NULL)
	{
		if (ogle->ai_mood != AI_MOOD_REST || ogle->classID != CID_OGLE)
			continue;

		if (ogle->targeted || ogle->targetEnt != self || SeraphCheckScare(self, ogle)) // See if we can scare this one.
			return;

		self->ai_mood = AI_MOOD_ATTACK;
		self->ai_mood_flags |= AI_MOOD_FLAG_WHIP;
	}
}

static qboolean SeraphCanThrowNode(edict_t* self, int node_id, int* throw_nodes) //mxd. Named 'canthrownode_so' in original logic.
{
	static const int bit_for_mesh_node[NUM_MESH_NODES] = //mxd. Made local static.
	{
		BIT_BASEBIN,
		BIT_PITHEAD, // Overlord head.
		BIT_SHOULDPAD,
		BIT_GUARDHEAD, // Guard head.
		BIT_LHANDGRD, // Left hand guard.
		BIT_LHANDBOSS, // Left hand overlord.
		BIT_RHAND, // Right hand.
		BIT_FRTORSO,
		BIT_ARMSPIKES,
		BIT_LFTUPARM,
		BIT_RTLEG,
		BIT_RTARM,
		BIT_LFTLEG,
		BIT_BKTORSO,
		BIT_AXE,
		BIT_WHIP
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

// Throws weapon, turns off those nodes, sets that weapon as gone.
static void SeraphDropWeapon(edict_t* self) //mxd. Named 'seraph_dropweapon' in original logic.
{
	if (self->s.fmnodeinfo[MESH__WHIP].flags & FMNI_NO_DRAW)
		return; // Already dropped.

	vec3_t forward;
	vec3_t right;
	vec3_t up;
	AngleVectors(self->s.angles, forward, right, up);

	vec3_t hand_pos = { 0 };
	VectorMA(hand_pos, 8.0f, forward, hand_pos);
	VectorMA(hand_pos, 5.0f, right, hand_pos);
	VectorMA(hand_pos, 12.0f, up, hand_pos);

	ThrowWeapon(self, hand_pos, BIT_WHIP, 0, FRAME_partfly);
	self->s.fmnodeinfo[MESH__WHIP].flags |= FMNI_NO_DRAW;
}

#pragma endregion

#pragma region ========================== Message handlers ==========================

static void SeraphDeathPainMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'seraph_death_pain' in original logic.
{
	if (self->health < -80) // Become gibbed.
		BecomeDebris(self);
}

static void SeraphDeathMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'seraph_death' in original logic.
{
	edict_t* target;
	edict_t* inflictor;
	edict_t* attacker;
	float damage;
	G_ParseMsgParms(msg, "eeei", &target, &inflictor, &attacker, &damage);

	M_StartDeath(self, ANIM_DEATH1);

	if (attacker->classID == CID_OGLE) //mxd. classname -> classID check.
	{
		self->health = 1;
		self->takedamage = DAMAGE_NO;
		self->solid = SOLID_BBOX;

		SetAnim(self, ANIM_DEATH1);
		gi.sound(self, CHAN_BODY, sounds[irand(SND_DEATH1, SND_DEATH4)], 1.0f, ATTN_NORM, 0.0f);

		return;
	}

	if (self->health < -80) // To be gibbed.
		return;

	if (self->health < -10)
	{
		SeraphDropWeapon(self);
		SetAnim(self, ANIM_DEATH2_GO);

		vec3_t dir;
		VectorNormalize2(target->velocity, dir);

		vec3_t yaw_dir;
		VectorScale(dir, -1.0f, yaw_dir);

		self->ideal_yaw = VectorYaw(yaw_dir);
		self->yaw_speed = 24.0f;

		self->elasticity = 1.2f;
		self->friction = 0.8f;

		VectorScale(dir, 300.0f, self->velocity);
		self->velocity[2] = flrand(150.0f, 200.0f); //mxd. irand() in original logic.
	}
	else
	{
		SetAnim(self, ANIM_DEATH1);
	}

	gi.sound(self, CHAN_BODY, sounds[irand(SND_DEATH1, SND_DEATH4)], 1.0f, ATTN_NORM, 0.0f);
}

// Check to see if the Seraph is already standing, if not, transition into it.
static void SeraphStandMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'seraph_stand' in original logic.
{
	SetAnim(self, ((self->curAnimID == ANIM_READY2IDLE) ? ANIM_STAND1 : ANIM_READY2IDLE));
}

// Classic run-attack function.
static void SeraphRunMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'seraph_run' in original logic.
{
	if (M_ValidTarget(self, self->enemy))
	{
		SetAnim(self, ANIM_RUN1);
		return;
	}

	// If our enemy is dead, we need to stand.
	G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
}

// Classic melee attack function.
static void SeraphMeleeMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'seraph_melee' in original logic.
{
	// Don't interrupt a current animation with another melee call inside of it.
	if (self->curAnimID == ANIM_ATTACK1_LOOP)
		return;

	if (self->enemy != NULL && self->enemy->classID == CID_OGLE && (self->ai_mood_flags & AI_MOOD_FLAG_IGNORE)) //mxd. classname -> classID check.
		return;

	if (!M_ValidTarget(self, self->enemy))
	{
		// If our enemy is dead, we need to stand.
		G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
		return;
	}

	if (self->ai_mood == AI_MOOD_FLEE)
	{
		SetAnim(self, ANIM_BACKUP2);
		return;
	}

	if (M_DistanceToTarget(self, self->enemy) < 100.0f)
	{
		vec3_t forward;
		AngleVectors(self->s.angles, forward, NULL, NULL);

		const qboolean in_range = M_PredictTargetEvasion(self, self->enemy, forward, self->enemy->velocity, self->melee_range, 5.0f);
		SetAnim(self, (in_range ? ANIM_ATTACK1_LOOP : ANIM_RUN1_WHIP));
	}
	else
	{
		SetAnim(self, ANIM_RUN1);
	}
}

static void SeraphPainMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'seraph_pain' in original logic.
{
	int	temp;
	edict_t* attacker;
	int force_pain;
	int damage;
	G_ParseMsgParms(msg, "eeiii", &temp, &attacker, &force_pain, &damage, &temp);

	if (self->monsterinfo.awake)
		self->ai_mood_flags &= ~AI_MOOD_FLAG_IGNORE;

	// Weighted random based on health compared to the maximum it was at.
	if (force_pain || (irand(0, self->max_health + 50) > self->health && irand(0, 2) == 0))
	{
		gi.sound(self, CHAN_BODY, sounds[irand(SND_PAIN1, SND_PAIN4)], 1.0f, ATTN_NORM, 0.0f);
		SetAnim(self, ANIM_PAIN);
	}

	if (attacker == NULL || attacker == self->enemy)
		return;

	if (self->enemy == NULL)
	{
		self->enemy = attacker;
		return;
	}

	if (M_DistanceToTarget(self, self->enemy) > self->melee_range)
	{
		if (self->enemy->client != NULL)
			self->oldenemy = self->enemy;

		self->enemy = attacker;
	}
}

static void SeraphVoiceSightMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'ser_ovl_SightSound' in original logic.
{
	gi.sound(self, CHAN_VOICE, sounds[irand(SND_SIGHT1, SND_SIGHT3)], 1.0f, ATTN_NORM, 0.0f);
}

static void SeraphCheckMoodMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'seraph_check_mood' in original logic.
{
	G_ParseMsgParms(msg, "i", &self->ai_mood);
	seraph_pause(self);
}

#pragma endregion

#pragma region ========================== Edict callbacks ===========================

qboolean SeraphAlert(edict_t* self, alertent_t* alerter, edict_t* enemy) //mxd. Named 'seraphAlerted' in original logic.
{
	if (self->alert_time < level.time)
	{
		// Not startled already.
		if (!(alerter->alert_svflags & SVF_ALERT_NO_SHADE) && SKILL < SKILL_VERYHARD && !(self->monsterinfo.aiflags & AI_NIGHTVISION))
			if (enemy->light_level < irand(6, 77)) //mxd. flrand() in original logic.
				return false; // Too dark, can't see enemy.

		if (!AI_IsInfrontOf(self, enemy))
		{
			if (alerter->lifetime < level.time + 2.0f)
				self->alert_time = level.time + 2.0f; // Be ready for 2 seconds to wake up if alerted again.
			else
				self->alert_time = alerter->lifetime; // Be alert as long as the alert sticks around.

			SetAnim(self, ANIM_STARTLE); //mxd. Inline seraph_startle().

			return false;
		}
	}

	self->enemy = ((enemy->svflags & SVF_MONSTER) ? alerter->enemy : enemy);
	AI_FoundTarget(self, true);

	return true;
}

static qboolean SeraphThrowHead(edict_t* self, float damage, const qboolean dismember_ok) //mxd. Added to simplify logic.
{
	if (self->s.fmnodeinfo[MESH__PITHEAD].flags & FMNI_NO_DRAW)
		return false;

	if (self->s.fmnodeinfo[MESH__PITHEAD].flags & FMNI_USE_SKIN)
		damage *= 1.5f; // Greater chance to cut off if previously damaged.

	if (dismember_ok && flrand(0, (float)self->health) < damage * 0.3f)
	{
		SeraphDropWeapon(self);

		int throw_nodes = 0;
		SeraphCanThrowNode(self, MESH__PITHEAD, &throw_nodes);

		vec3_t gore_spot = { 0.0f, 0.0f, 18.0f };
		ThrowBodyPart(self, gore_spot, throw_nodes, (int)damage, FRAME_partfly);

		Vec3AddAssign(self->s.origin, gore_spot);
		SprayDebris(self, gore_spot, 8);

		if (self->health > 0)
		{
			self->health = 1;
			T_Damage(self, self, self, vec3_origin, vec3_origin, vec3_origin, 10, 20, 0, MOD_DIED);
		}

		return true;
	}

	self->s.fmnodeinfo[MESH__PITHEAD].flags |= FMNI_USE_SKIN;
	self->s.fmnodeinfo[MESH__PITHEAD].skin = self->s.skinnum + 1;

	return false;
}

static void SeraphThrowBodyPart(edict_t* self, const float damage, const int mesh_part) //mxd. Added to simplify logic.
{
	if (flrand(0, (float)self->health) < damage)
	{
		self->s.fmnodeinfo[mesh_part].flags |= FMNI_USE_SKIN;
		self->s.fmnodeinfo[mesh_part].skin = self->s.skinnum + 1;
	}
}

static void SeraphThrowArmLowerLeft(edict_t* self, float damage, const qboolean dismember_ok) //mxd. Added to simplify logic.
{
	if (self->s.fmnodeinfo[MESH__LHANDBOSS].flags & FMNI_NO_DRAW)
		return;

	if (self->s.fmnodeinfo[MESH__LHANDBOSS].flags & FMNI_USE_SKIN)
		damage *= 1.5f; // Greater chance to cut off if previously damaged.

	if (dismember_ok && flrand(0, (float)self->health) < damage * 0.75f)
	{
		int throw_nodes = 0;

		if (SeraphCanThrowNode(self, MESH__LHANDBOSS, &throw_nodes))
		{
			vec3_t right;
			AngleVectors(self->s.angles, NULL, right, NULL);

			vec3_t gore_spot = { 0.0f, 0.0f, self->maxs[2] * 0.3f };
			VectorMA(gore_spot, -10.0f, right, gore_spot);

			ThrowBodyPart(self, gore_spot, throw_nodes, (int)damage, FRAME_partfly);
		}
	}
	else
	{
		self->s.fmnodeinfo[MESH__LHANDBOSS].flags |= FMNI_USE_SKIN;
		self->s.fmnodeinfo[MESH__LHANDBOSS].skin = self->s.skinnum + 1;
	}
}

static void SeraphThrowArmLowerRight(edict_t* self, float damage, const qboolean dismember_ok) //mxd. Added to simplify logic.
{
	if (self->s.fmnodeinfo[MESH__RHAND].flags & FMNI_NO_DRAW)
		return;

	if (dismember_ok && flrand(0, (float)self->health) < damage * 0.75f)
	{
		int throw_nodes = 0;

		if (SeraphCanThrowNode(self, MESH__RHAND, &throw_nodes))
		{
			vec3_t right;
			AngleVectors(self->s.angles, NULL, right, NULL);

			vec3_t gore_spot = { 0.0f, 0.0f, self->maxs[2] * 0.3f };
			VectorMA(gore_spot, 10.0f, right, gore_spot);

			SeraphDropWeapon(self);
			ThrowBodyPart(self, gore_spot, throw_nodes, (int)damage, FRAME_partfly);
		}
	}
	else
	{
		self->s.fmnodeinfo[MESH__RHAND].flags |= FMNI_USE_SKIN;
		self->s.fmnodeinfo[MESH__RHAND].skin = self->s.skinnum + 1;
	}
}

static void SeraphThrowLeg(edict_t* self, const int mesh_part) //mxd. Added to simplify logic.
{
	if (!(self->s.fmnodeinfo[mesh_part].flags & FMNI_USE_SKIN))
	{
		self->s.fmnodeinfo[mesh_part].flags |= FMNI_USE_SKIN;
		self->s.fmnodeinfo[mesh_part].skin = self->s.skinnum + 1;
	}
}

void SeraphDismember(edict_t* self, int damage, HitLocation_t hl) //mxd. Named 'seraph_dismember' in original logic.
{
	qboolean dismember_ok = false;

	if (hl & hl_MeleeHit)
	{
		dismember_ok = true;
		hl &= ~hl_MeleeHit;
	}

	if (hl <= hl_NoneSpecific || hl >= hl_Max) //mxd. '> hl_Max' in original logic.
		return;

	switch (hl)
	{
		case hl_Head:
			if (SeraphThrowHead(self, (float)damage, dismember_ok))
				return;
			break;

		case hl_TorsoFront: // Split in half?
			SeraphThrowBodyPart(self, (float)damage, MESH__FRTORSO);
			break;

		case hl_TorsoBack: // Split in half?
			SeraphThrowBodyPart(self, (float)damage, MESH__BKTORSO);
			break;

		case hl_ArmUpperLeft: // Left arm.
			SeraphThrowBodyPart(self, (float)damage, MESH__LFTUPARM);
			break;

		case hl_ArmLowerLeft:
			SeraphThrowArmLowerLeft(self, (float)damage, dismember_ok);
			break;

		case hl_ArmUpperRight: // Right arm.
			SeraphThrowBodyPart(self, (float)damage, MESH__RTARM);
			break;

		case hl_ArmLowerRight:
			SeraphThrowArmLowerRight(self, (float)damage, dismember_ok);
			break;

		case hl_LegUpperLeft: // Left leg.
		case hl_LegLowerLeft:
			SeraphThrowLeg(self, MESH__LFTLEG);
			break;

		case hl_LegUpperRight: // Right leg.
		case hl_LegLowerRight:
			SeraphThrowLeg(self, MESH__RTLEG);
			break;

		default:
			break;
	}

	if (self->s.fmnodeinfo[MESH__RHAND].flags & FMNI_NO_DRAW)
	{
		self->monsterinfo.aiflags |= AI_NO_MELEE;
		SetAnim(self, ANIM_BACKUP);
	}
}

#pragma endregion

#pragma region ========================== Action functions ==========================

void seraph_jump(edict_t* self) //mxd. Named 'seraphApplyJump' in original logic.
{
	self->jump_time = level.time + 0.75f;
	VectorCopy(self->movedir, self->velocity);
	VectorNormalize(self->movedir);
}

void seraph_dead(edict_t* self)
{
	self->health = 0;
	self->solid = SOLID_NOT;

	M_EndDeath(self);
}

void seraph_death_loop(edict_t* self)
{
	SetAnim(self, ANIM_DEATH2_LOOP);
}

void seraph_check_land(edict_t* self)
{
	M_ChangeYaw(self);

	vec3_t end_pos;
	VectorCopy(self->s.origin, end_pos);
	end_pos[2] -= 48.0f;

	trace_t trace;
	gi.trace(self->s.origin, self->mins, self->maxs, end_pos, self, MASK_MONSTERSOLID, &trace);

	if ((trace.fraction < 1.0f || trace.allsolid || trace.startsolid) && self->curAnimID != ANIM_DEATH2_END && self->curAnimID != ANIM_DEATH2_GO)
	{
		self->elasticity = 1.25f;
		self->friction = 0.5f;
		SetAnim(self, ANIM_DEATH2_END);
	}
}

void seraph_sound_startle(edict_t* self)
{
	gi.sound(self, CHAN_VOICE, sounds[SND_STARTLE], 1.0f, ATTN_NORM, 0.0f);
}

void seraph_sound_slap(edict_t* self)
{
	gi.sound(self, CHAN_WEAPON, sounds[SND_SLAP], 1.0f, ATTN_NORM, 0.0f);
}

void seraph_sound_scold(edict_t* self)
{
	gi.sound(self, CHAN_VOICE, sounds[SND_SCOLD3], 1.0f, ATTN_NORM, 0.0f);
}

void seraph_sound_scold2(edict_t* self)
{
	gi.sound(self, CHAN_VOICE, sounds[irand(SND_SCOLD1, SND_SCOLD2)], 1.0f, ATTN_NORM, 0.0f);
}

void seraph_sound_yell(edict_t* self)
{
	gi.sound(self, CHAN_VOICE, sounds[SND_SCARE], 1.0f, ATTN_NORM, 0.0f);
}

void seraph_sound_whip(edict_t* self)
{
	gi.sound(self, CHAN_WEAPON, sounds[SND_ATTACK], 1.0f, ATTN_NORM, 0.0f);
}

// Seraph has finished his startle, either track down the enemy, or go back to normal.
void seraph_done_startle(edict_t* self)
{
	if (!FindTarget(self))
		G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
}

// Seraph has finished ANIM_GET2WORK, and must reset its enemy.
void seraph_done_get2work(edict_t* self)
{
	if (self->enemy != NULL) //TODO: also check if self->enemy is CID_OGLE?
		self->enemy->targeted = false;

	self->enemy = NULL;
	self->ai_mood = AI_MOOD_STAND;
	self->ai_mood_flags &= ~AI_MOOD_FLAG_IGNORE;

	seraph_pause(self);
}

// Special walk code for going to yell at an ogle.
void seraph_ai_walk(edict_t* self, float distance)
{
	if (self->enemy != NULL && M_DistanceToTarget(self, self->enemy) < 72.0f) //TODO: also check if self->enemy is CID_OGLE?
	{
		self->ai_mood = AI_MOOD_STAND;
		SetAnim(self, irand(ANIM_GET2WORK, ANIM_GET2WORK2));
	}
	else
	{
		MG_MoveToGoal(self, distance);
	}
}

void seraph_stand(edict_t* self) //mxd. Named 'seraph2idle' in original logic.
{
	SetAnim(self, ANIM_STAND1);
}

// Upper level AI interfacing.
void seraph_pause(edict_t* self)
{
	self->mood_think(self);

	if (self->enemy != NULL && self->enemy->classID == CID_OGLE && (self->ai_mood_flags & AI_MOOD_FLAG_IGNORE))
		return;

	switch (self->ai_mood)
	{
		case AI_MOOD_ATTACK:
			G_PostMessage(self, MSG_MELEE, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_PURSUE:
		case AI_MOOD_NAVIGATE:
		case AI_MOOD_WALK:
			G_PostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_STAND:
			if (self->enemy == NULL) //TODO: else what?
				G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_WANDER:
			SetAnim(self, ANIM_WALK1);
			break;

		case AI_MOOD_POINT_NAVIGATE:
			SetAnim(self, ANIM_WALK2);
			break;

		case AI_MOOD_JUMP:
			SetAnim(self, ANIM_FJUMP);
			break;

		default:
			break;
	}
}

// Targets a specific ogle and puts it back to work.
void seraph_enforce_ogle(edict_t* self)
{
	if (self->enemy != NULL) //TODO: also check if self->enemy is CID_OGLE?
	{
		self->enemy->use(self->enemy, self, self);
		gi.sound(self, CHAN_VOICE, sounds[irand(SND_SCOLD1, SND_SCOLD2)], 1.0f, ATTN_NORM, 0.0f);
	}
}

// Targets all idle ogles and puts them back to work.
void seraph_enforce(edict_t* self)
{
	edict_t* ogle = NULL;
	while ((ogle = FindInRadius(ogle, self->s.origin, OVERLORD_ENFORCE_RADIUS)) != NULL)
		if (ogle->classID == CID_OGLE && ogle->ai_mood == AI_MOOD_REST && ogle->targetEnt == self)
			ogle->use(ogle, self, self); // Setup within the ogle code.
}

// Cycles through the various idle animations.
void seraph_idle(edict_t* self)
{
	SeraphOversee(self);

	// Check to see if we were supposed to point at an ogle.
	if (self->ai_mood == AI_MOOD_ATTACK && (self->ai_mood_flags & AI_MOOD_FLAG_WHIP) && self->curAnimID != ANIM_POINT1)
	{
		SetAnim(self, ANIM_POINT1);
		self->ai_mood = AI_MOOD_STAND;
		self->ai_mood_flags &= ~AI_MOOD_FLAG_WHIP;

		return;
	}

	// See if we're going to go scare an ogle.
	if (self->ai_mood == AI_MOOD_POINT_NAVIGATE)
	{
		SetAnim(self, ANIM_WALK2);
		return;
	}

	const int chance = irand(0, 100);

	switch (self->curAnimID)
	{
		case ANIM_STAND1:
			if (chance < 20)
				SetAnim(self, ANIM_STAND1_TR);
			else if (chance < 40)
				SetAnim(self, ANIM_STAND1_TL);
			break;

			// Right.
		case ANIM_POINT1:
		case ANIM_STAND1_TR:
			SetAnim(self, ANIM_STAND1_R);
			break;

		case ANIM_STAND1_R:
			if (chance < 50)
				SetAnim(self, ANIM_STAND1_TRC);
			break;

		case ANIM_STAND1_TLC:
		case ANIM_STAND1_TRC:
			SetAnim(self, ANIM_STAND1);
			break;

			// Left.
		case ANIM_STAND1_TL:
			SetAnim(self, ANIM_STAND1_L);
			break;

		case ANIM_STAND1_L:
			if (chance < 50)
				SetAnim(self, ANIM_STAND1_TLC);
			break;

		default:
			break;
	}
}

// Check to do damage with a whip.
void seraph_strike(edict_t* self, float damage, float a, float b)
{
	const vec3_t start_offset = { 16.0f, 16.0f, 32.0f };
	const vec3_t end_offset = { 80.0f, 16.0f, 8.0f };

	const vec3_t mins = { -2.0f, -2.0f, -2.0f };
	const vec3_t maxs = {  2.0f,  2.0f,  2.0f };

	trace_t	trace;
	vec3_t direction;
	edict_t* victim = M_CheckMeleeLineHit(self, start_offset, end_offset, mins, maxs, &trace, direction);

	gi.sound(self, CHAN_WEAPON, sounds[SND_ATTACK], 1.0f, ATTN_NORM, 0.0f);

	if (victim == NULL) // Missed.
		return; //TODO: play swoosh sound instead?

	if (victim == self) // Hit wall.
		return; //TODO: Create a puff effect.

	// Hurt whatever we were whacking away at.
	vec3_t blood_dir;
	VectorSubtract(start_offset, end_offset, blood_dir);
	VectorNormalize(blood_dir);

	T_Damage(victim, self, self, direction, trace.endpos, blood_dir, (int)damage, (int)damage, DAMAGE_EXTRA_BLOOD | DAMAGE_EXTRA_KNOCKBACK, MOD_DIED);
}

void seraph_back(edict_t* self, float distance)
{
	if (!MG_TryWalkMove(self, self->s.angles[YAW] + 180.0f, distance, true) && irand(0, 1000) == 0) // 0.0009% chance to run away when MG_TryWalkMove() fails. //TODO: chance seems way too low. Scale with difficulty?
	{
		self->monsterinfo.aiflags |= AI_COWARD;
		SetAnim(self, ANIM_RUN1);
	}
}

#pragma endregion

void SeraphOverlordStaticsInit(void)
{
	static ClassResourceInfo_t res_info; //mxd. Made local static.

	classStatics[CID_SERAPH_OVERLORD].msgReceivers[MSG_STAND] = SeraphStandMsgHandler;
	classStatics[CID_SERAPH_OVERLORD].msgReceivers[MSG_RUN] = SeraphRunMsgHandler;
	classStatics[CID_SERAPH_OVERLORD].msgReceivers[MSG_MELEE] = SeraphMeleeMsgHandler;
	classStatics[CID_SERAPH_OVERLORD].msgReceivers[MSG_PAIN] = SeraphPainMsgHandler;
	classStatics[CID_SERAPH_OVERLORD].msgReceivers[MSG_DEATH] = SeraphDeathMsgHandler;
	classStatics[CID_SERAPH_OVERLORD].msgReceivers[MSG_DISMEMBER] = DismemberMsgHandler;
	classStatics[CID_SERAPH_OVERLORD].msgReceivers[MSG_DEATH_PAIN] = SeraphDeathPainMsgHandler;
	classStatics[CID_SERAPH_OVERLORD].msgReceivers[MSG_CHECK_MOOD] = SeraphCheckMoodMsgHandler;
	classStatics[CID_SERAPH_OVERLORD].msgReceivers[MSG_VOICE_SIGHT] = SeraphVoiceSightMsgHandler;

	res_info.numAnims = NUM_ANIMS;
	res_info.animations = animations;
	res_info.modelIndex = gi.modelindex("models/monsters/overlord/tris.fm");

	sounds[SND_ATTACK] = gi.soundindex("monsters/seraph/overlord/attack.wav");

	sounds[SND_SCOLD1] = gi.soundindex("monsters/seraph/overlord/scold1.wav"); // Back to work.
	sounds[SND_SCOLD2] = gi.soundindex("monsters/seraph/overlord/scold2.wav"); // Get to work.
	sounds[SND_SCOLD3] = gi.soundindex("monsters/seraph/overlord/scold3.wav"); // No talking.

	sounds[SND_SCARE] = gi.soundindex("monsters/seraph/overlord/scare.wav"); // Hey!

	sounds[SND_STARTLE] = gi.soundindex("monsters/seraph/overlord/startle.wav");
	sounds[SND_SLAP] = gi.soundindex("monsters/seraph/overlord/slap.wav");

	sounds[SND_DEATH1] = gi.soundindex("monsters/seraph/death1.wav");
	sounds[SND_DEATH2] = gi.soundindex("monsters/seraph/death2.wav");
	sounds[SND_DEATH3] = gi.soundindex("monsters/seraph/wimpdeath1.wav");
	sounds[SND_DEATH4] = gi.soundindex("monsters/seraph/wimpdeath2.wav");

	sounds[SND_PAIN1] = gi.soundindex("monsters/seraph/pain1.wav");
	sounds[SND_PAIN2] = gi.soundindex("monsters/seraph/pain2.wav");
	sounds[SND_PAIN3] = gi.soundindex("monsters/seraph/pain3.wav");
	sounds[SND_PAIN4] = gi.soundindex("monsters/seraph/pain4.wav");

	sounds[SND_SIGHT1] = gi.soundindex("monsters/seraph/overlord/sight1.wav");
	sounds[SND_SIGHT2] = gi.soundindex("monsters/seraph/overlord/sight2.wav");
	sounds[SND_SIGHT3] = gi.soundindex("monsters/seraph/overlord/scare.wav");

	res_info.numSounds = NUM_SOUNDS;
	res_info.sounds = sounds;

	classStatics[CID_SERAPH_OVERLORD].resInfo = &res_info;
}

// QUAKED monster_seraph_overlord(1 .5 0) (-24 -24 -34) (24 24 34) AMBUSH ASLEEP 4 8 16 32 64 FIXED WANDER MELEE_LEAD STALK COWARD EXTRA1 EXTRA2 EXTRA3 EXTRA4
// The big, nasty, tyranical Overlords.

// Spawnflags:
// AMBUSH		- Will not be woken up by other monsters or shots from player.
// ASLEEP		- Will not appear until triggered.
// WALKING		- Use WANDER instead.
// WANDER		- Monster will wander around aimlessly (but follows buoys).
// MELEE_LEAD	- Monster will try to cut you off when you're running and fighting him, works well if there are a few monsters in a group, half doing this, half not.
// STALK		- Monster will only approach and attack from behind. If you're facing the monster it will just stand there.
//				  Once the monster takes pain, however, it will stop this behaviour and attack normally.
// COWARD		- Monster starts off in flee mode (runs away from you when woken up).

// Variables:
// homebuoy					- Monsters will head to this buoy if they don't have an enemy ("homebuoy" should be targetname of the buoy you want them to go to).
// wakeup_target			- Monsters will fire this target the first time it wakes up (only once).
// pain_target				- Monsters will fire this target the first time it gets hurt (only once).
// mintel					- Monster intelligence - this basically tells a monster how many buoys away an enemy has to be for it to give up (default 24).
// melee_range				- How close the player has to be for the monster to go into melee. If this is zero, the monster will never melee.
//							  If it is negative, the monster will try to keep this distance from the player.
//							  If the monster has a backup, he'll use it if too close, otherwise, a negative value here means the monster will just stop
//							  running at the player at this distance (default 100).
//							 Examples:
//								melee_range = 60 - monster will start swinging it player is closer than 60.
//								melee_range = 0 - monster will never do a melee attack.
//								melee_range = -100 - monster will never do a melee attack and will back away (if it has that ability) when player gets too close.
// missile_range			- Maximum distance the player can be from the monster to be allowed to use it's ranged attack (default 0).
// min_missile_range		- Minimum distance the player can be from the monster to be allowed to use it's ranged attack (default 0).
// bypass_missile_chance	- Chance that a monster will NOT fire it's ranged attack, even when it has a clear shot. This, in effect, will make the monster
//							  come in more often than hang back and fire. A percentage (0 = always fire/never close in, 100 = never fire/always close in) - must be whole number (default 0).
// jump_chance				- Every time the monster has the opportunity to jump, what is the chance (out of 100) that he will... (100 = jump every time) - must be whole number (default 30).
// wakeup_distance			- How far (max) the player can be away from the monster before it wakes up. This means that if the monster can see the player,
//							  at what distance should the monster actually notice him and go for him (default 1024).
// NOTE: A value of zero will result in defaults, if you actually want zero as the value, use -1.
void SP_monster_seraph_overlord(edict_t* self)
{
	// Generic monster initialization.
	if (!M_WalkmonsterStart(self)) //mxd. M_Start -> M_WalkmonsterStart.
		return; // Failed initialization.

	self->msgHandler = DefaultMsgHandler;
	self->monsterinfo.alert = SeraphAlert;
	self->monsterinfo.dismember = SeraphDismember;

	if (self->health == 0)
		self->health = SERAPH_HEALTH;

	// Apply to the end result (whether designer set or not).
	self->health = MonsterHealth(self->health);
	self->max_health = self->health;

	self->mass = SERAPH_MASS;
	self->yaw_speed = 18.0f;

	self->movetype = PHYSICSTYPE_STEP;
	self->solid = SOLID_BBOX;

	VectorCopy(STDMinsForClass[self->classID], self->mins);
	VectorCopy(STDMaxsForClass[self->classID], self->maxs);

	self->s.modelindex = (byte)classStatics[CID_SERAPH_OVERLORD].resInfo->modelIndex;
	self->s.skinnum = 0;
	self->materialtype = MAT_FLESH;
	self->ai_mood_flags |= AI_MOOD_FLAG_PREDICT;
	self->monsterinfo.otherenemyname = "monster_rat";

	if (self->s.scale == 0.0f)
	{
		self->s.scale = MODEL_SCALE;
		self->monsterinfo.scale = self->s.scale;
	}

	// Turn off the Guard pieces!
	self->s.fmnodeinfo[MESH__AXE].flags |= FMNI_NO_DRAW;
	self->s.fmnodeinfo[MESH__GUARDHEAD].flags |= FMNI_NO_DRAW; //TODO: randomly enable?
	self->s.fmnodeinfo[MESH__LHANDGRD].flags |= FMNI_NO_DRAW; //mxd. Unfinished texture.
	self->s.fmnodeinfo[MESH__ARMSPIKES].flags |= FMNI_NO_DRAW; //mxd. Unfinished texture.
	self->s.fmnodeinfo[MESH__SHOULDPAD].flags |= FMNI_NO_DRAW; //TODO: randomly enable?

	MG_InitMoods(self);
	G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);

	self->melee_range *= self->s.scale;
}