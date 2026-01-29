//
// m_seraph_guard.c
//
// Copyright 1998 Raven Software
//

#include "m_seraph_guard.h"
#include "m_seraph_guard_shared.h"
#include "m_seraph_guard_anim.h"
#include "m_seraph_guard_moves.h"
#include "g_debris.h" //mxd
#include "g_DefaultMessageHandler.h"
#include "g_playstats.h"
#include "mg_ai.h" //mxd
#include "mg_guide.h" //mxd
#include "m_stats.h"
#include "p_anims.h"
#include "Random.h"
#include "Utilities.h"
#include "Vector.h"
#include "g_monster.h"

#define SGUARD_NUM_PREDICTED_FRAMES	5.0f //mxd. Named 'NUM_PREDFRAMES' in original logic.
#define SF_SGUARD_GOLEM				4 //mxd. Named 'SERAPH_FLAG_GOLEM' in original logic.

#pragma region ========================== Seraph Guard Base Info ==========================

static const animmove_t* animations[NUM_ANIMS] =
{
	&seraph_guard_move_stand,
	&seraph_guard_move_run,
	&seraph_guard_move_fjump,
	&seraph_guard_move_runmelee,
	&seraph_guard_move_walk,
	&seraph_guard_move_pain,
	&seraph_guard_move_melee,
	&seraph_guard_move_melee2,
	&seraph_guard_move_melee3,
	&seraph_guard_move_death1,
	&seraph_guard_move_death2_go,
	&seraph_guard_move_death2_loop,
	&seraph_guard_move_death2_end,
	&seraph_guard_move_backup,
	&seraph_guard_move_missile,
	&seraph_guard_move_delay,
};

static int sounds[NUM_SOUNDS];

#pragma endregion

#pragma region ========================== Utility functions =========================

// Create the guts of seraph guard's projectile.
static void SeraphGuardProjectileInit(const edict_t* self, edict_t* proj) //mxd. Named 'create_guard_proj' in original logic.
{
	proj->svflags |= SVF_ALWAYS_SEND;
	proj->movetype = PHYSICSTYPE_FLY;
	proj->gravity = 0.0f;
	proj->solid = SOLID_BBOX;
	proj->classname = "Guard_Missile";
	proj->dmg = irand(SGUARD_DMG_SPELL_MIN, SGUARD_DMG_SPELL_MAX); //mxd. flrand() in original logic.
	proj->s.scale = 1.0f;
	proj->clipmask = MASK_SHOT;
	proj->nextthink = level.time + FRAMETIME; //mxd. Use define.

	proj->isBlocked = SeraphGuardProjectileBlocked;
	proj->isBlocking = SeraphGuardProjectileBlocked;
	proj->bounced = SeraphGuardProjectileBlocked;

	proj->s.effects = (EF_MARCUS_FLAG1 | EF_CAMERA_NO_CLIP);
	proj->enemy = self->enemy;

	VectorSet(proj->mins, -4.0f, -4.0f, -4.0f);
	VectorSet(proj->maxs,  4.0f,  4.0f,  4.0f);
	VectorCopy(self->s.origin, proj->s.origin);
}

static void SeraphGuardProjectile(edict_t* self) //mxd. Named 'guard_beam' in original logic.
{
	// Spawn the projectile.
	edict_t* proj = G_Spawn();

	SeraphGuardProjectileInit(self, proj);

	proj->reflect_debounce_time = MAX_REFLECT;
	proj->owner = self;

	vec3_t forward;
	vec3_t right;
	AngleVectors(self->s.angles, forward, right, NULL);

	VectorMA(proj->s.origin, 24.0f, forward, proj->s.origin);
	VectorMA(proj->s.origin, 16.0f, right, proj->s.origin);
	proj->s.origin[2] += 10.0f;

	vec3_t dir;

	if (MG_IsAheadOf(self, self->enemy))
	{
		VectorSubtract(self->enemy->s.origin, self->s.origin, dir);
		VectorNormalize(dir);
	}
	else
	{
		VectorCopy(forward, dir);
	}

	VectorScale(dir, 500.0f, proj->velocity);
	vectoangles(proj->velocity, proj->s.angles);

	proj->think = SeraphGuardProjectileThink;
	proj->nextthink = level.time + 1.0f;

	gi.sound(self, CHAN_WEAPON, sounds[SND_MISSILE], 1.0f, ATTN_NORM, 0.0f); //mxd. Use precached sound index.
	gi.CreateEffect(&proj->s, FX_M_EFFECTS, CEF_OWNERS_ORIGIN, vec3_origin, "bv", FX_M_BEAM, proj->s.angles);
	gi.linkentity(proj);
}

static qboolean SeraphGuardCanThrowNode(edict_t* self, const int node_id, int* throw_nodes) //mxd. Named 'canthrownode_sg' in original logic.
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
static void SeraphGuardDropWeapon(edict_t* self) //mxd. Named 'seraph_guard_dropweapon' in original logic.
{
	if (self->s.fmnodeinfo[MESH__AXE].flags & FMNI_NO_DRAW)
		return; // Already dropped.

	vec3_t forward;
	vec3_t right;
	vec3_t up;
	AngleVectors(self->s.angles, forward, right, up);

	vec3_t hand_pos = { 0 };
	VectorMA(hand_pos, 8.0f, forward, hand_pos);
	VectorMA(hand_pos, 5.0f, right, hand_pos);
	VectorMA(hand_pos, 12.0f, up, hand_pos);

	ThrowWeapon(self, &hand_pos, BIT_AXE, 0, FRAME_partfly);
	self->s.fmnodeinfo[MESH__AXE].flags |= FMNI_NO_DRAW;
}

#pragma endregion

#pragma region ========================== Message handlers ==========================

static void SeraphGuardCheckMoodMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'seraph_guard_check_mood' in original logic.
{
	G_ParseMsgParms(msg, "i", &self->ai_mood);
	seraph_guard_pause(self);
}

static void SeraphGuardPainMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'seraph_guard_pain' in original logic.
{
	int temp;
	int force_damage;
	int damage;
	G_ParseMsgParms(msg, "eeiii", &temp, &temp, &force_damage, &damage, &temp);

	// Weighted random based on health compared to the maximum it was at.
	if (force_damage || (irand(0, self->max_health + 50) > self->health && irand(0, 2) != 0))
	{
		gi.sound(self, CHAN_WEAPON, sounds[irand(SND_PAIN1, SND_PAIN4)], 1.0f, ATTN_NORM, 0.0f);
		SetAnim(self, ANIM_PAIN);
	}
}

static void SeraphGuardStandMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'seraph_guard_stand' in original logic.
{
	SetAnim(self, ANIM_STAND);
}

static void SeraphGuardMeleeMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'seraph_guard_melee' in original logic.
{
	// Don't interrupt a current animation with another melee call inside of it.
	if (self->curAnimID == ANIM_MELEE1 || self->curAnimID == ANIM_MELEE2)
		return;

	if (!M_ValidTarget(self, self->enemy))
	{
		// If our enemy is dead, we need to stand.
		G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
		return;
	}

	if (self->ai_mood == AI_MOOD_FLEE)
	{
		SetAnim(self, ANIM_BACKUP);
		return;
	}

	const float dist = M_DistanceToTarget(self, self->enemy);

	if (dist >= 120.0f)
	{
		SetAnim(self, ANIM_RUN);
		return;
	}

	if (self->s.fmnodeinfo[MESH__AXE].flags & FMNI_NO_DRAW)
	{
		SetAnim(self, ANIM_MISSILE);
		return;
	}

	vec3_t forward;
	AngleVectors(self->s.angles, forward, NULL, NULL);

	if (!M_PredictTargetEvasion(self, self->enemy, forward, self->enemy->velocity, self->melee_range, SGUARD_NUM_PREDICTED_FRAMES))
	{
		SetAnim(self, ANIM_RUN_MELEE);
		return;
	}

	if (dist < 88.0f && irand(0, 3) == 0)
		SetAnim(self, ANIM_MISSILE);
	else if (irand(0, 4) != 0)
		SetAnim(self, irand(0, 10) != 0 ? ANIM_MELEE1 : ANIM_MISSILE);
	else
		SetAnim(self, ANIM_MELEE2);
}

static void SeraphGuardMissileMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'seraph_guard_missile' in original logic.
{
	if (!M_ValidTarget(self, self->enemy))
	{
		// If our enemy is dead, we need to stand.
		G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
		return;
	}

	if (self->ai_mood == AI_MOOD_FLEE)
	{
		SetAnim(self, ANIM_BACKUP);
		return;
	}

	if (M_DistanceToTarget(self, self->enemy) < self->min_missile_range)
	{
		if (irand(0, 4) != 0)
			SetAnim(self, ((irand(0, 10) != 0) ? ANIM_MELEE1 : ANIM_MISSILE));
		else
			SetAnim(self, ANIM_MELEE2);
	}
	else if (MG_IsAheadOf(self, self->enemy))
	{
		SetAnim(self, ANIM_MISSILE);
	}
}

static void SeraphGuardDeathPainMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'seraph_guard_death_pain' in original logic.
{
	if (self->health < -80) // Become gibbed?
		BecomeDebris(self);
}

static void SeraphGuardDeathMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'seraph_guard_death' in original logic.
{
	edict_t* target;
	edict_t* inflictor;
	edict_t* attacker;
	float damage;
	G_ParseMsgParms(msg, "eeei", &target, &inflictor, &attacker, &damage);

	M_StartDeath(self, ANIM_DEATH1);

	if (self->health < -80) // To be gibbed.
		return;

	if (self->health < -10)
	{
		SeraphGuardDropWeapon(self);
		SetAnim(self, ANIM_DEATH2_GO);

		vec3_t dir;
		VectorNormalize2(target->velocity, dir);

		vec3_t yaw_dir;
		VectorScale(dir, -1.0f, yaw_dir);

		self->ideal_yaw = VectorYaw(yaw_dir);
		self->yaw_speed = 24.0f;

		VectorScale(dir, 300.0f, self->velocity);
		self->velocity[2] = flrand(150.0f, 200.0f); //mxd. irand() in original logic.
	}
	else
	{
		SetAnim(self, ANIM_DEATH1);
	}

	gi.sound(self, CHAN_BODY, sounds[irand(SND_DEATH1, SND_DEATH4)], 1.0f, ATTN_NORM, 0.0f);
}

static void SeraphGuardRunMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'seraph_guard_run' in original logic.
{
	if (!M_ValidTarget(self, self->enemy))
	{
		// If our enemy is dead, we need to stand.
		G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
		return;
	}

	if (self->ai_mood == AI_MOOD_FLEE)
	{
		SetAnim(self, ANIM_RUN);
		return;
	}

	if (self->monsterinfo.attack_finished > level.time || (self->monsterinfo.aiflags & AI_NO_MELEE))
	{
		SetAnim(self, ANIM_RUN);
		return;
	}

	// Set this for any uses below.
	vec3_t pursue_vel;
	AngleVectors(self->s.angles, pursue_vel, NULL, NULL);

	trace_t trace;
	const float dist = M_DistanceToTarget(self, self->enemy);

	if (dist < 100.0f)
	{
		const qboolean in_range = M_PredictTargetEvasion(self, self->enemy, pursue_vel, self->enemy->velocity, self->melee_range, SGUARD_NUM_PREDICTED_FRAMES);

		// See what the predicted outcome is.
		if (in_range && M_CheckMeleeHit(self, self->melee_range, &trace) == self->enemy)
			SetAnim(self, ANIM_MELEE1);
		else
			SetAnim(self, ANIM_RUN_MELEE);
	}
	else if (dist < 200.0f)
	{
		Vec3ScaleAssign(150.0f, pursue_vel);
		const qboolean in_range = M_PredictTargetEvasion(self, self->enemy, pursue_vel, self->enemy->velocity, 150.0f, SGUARD_NUM_PREDICTED_FRAMES);

		// See what the predicted outcome is.
		if (in_range && M_CheckMeleeHit(self, 150.0f, &trace) == self->enemy)
			SetAnim(self, ANIM_RUN_MELEE);
		else
			SetAnim(self, ANIM_RUN);
	}
	else
	{
		SetAnim(self, ANIM_RUN);
	}
}

static void SeraphGuardVoiceSightMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'ser_grd_SightSound' in original logic.
{
	gi.sound(self, CHAN_VOICE, sounds[irand(SND_SIGHT1, SND_SIGHT4)], 1.0f, ATTN_NORM, 0.0f);
}

#pragma endregion

#pragma region ========================== Edict callbacks ===========================

void SeraphGuardProjectileBlocked(edict_t* self, trace_t* trace) //mxd. Named 'guard_beam_blocked' in original logic.
{
	//TODO: re-implement projectile reflecting logic?

	if (trace->ent->takedamage != DAMAGE_NO)
	{
		vec3_t hit_dir;
		VectorNormalize2(self->velocity, hit_dir);

		T_Damage(trace->ent, self, self->owner, hit_dir, self->s.origin, trace->plane.normal, self->dmg, 0, DAMAGE_SPELL | DAMAGE_NO_KNOCKBACK, MOD_DIED);
	}

	gi.sound(self, CHAN_WEAPON, sounds[SND_MISSHIT], 1.0f, ATTN_NORM, 0.0f); //mxd. Use precached sound index.
	gi.CreateEffect(&self->s, FX_M_EFFECTS, CEF_OWNERS_ORIGIN, self->s.origin, "bv", FX_M_MISC_EXPLODE, vec3_origin);

	G_SetToFree(self);
}

void SeraphGuardProjectileThink(edict_t* self) //mxd. Named 'guard_beam_think' in original logic. //TODO: is this needed?..
{
	self->think = NULL;
	self->nextthink = -1.0f;
}

static qboolean SeraphGuardThrowHead(edict_t* self, float damage, const qboolean dismember_ok) //mxd. Added to simplify logic.
{
	if (self->s.fmnodeinfo[MESH__GUARDHEAD].flags & FMNI_NO_DRAW)
		return false;

	if (self->s.fmnodeinfo[MESH__GUARDHEAD].flags & FMNI_USE_SKIN)
		damage *= 1.5f; // Greater chance to cut off if previously damaged.

	if (dismember_ok && flrand(0, (float)self->health) < damage * 0.3f)
	{
		SeraphGuardDropWeapon(self);

		int throw_nodes = 0;
		SeraphGuardCanThrowNode(self, MESH__GUARDHEAD, &throw_nodes);

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

	self->s.fmnodeinfo[MESH__GUARDHEAD].flags |= FMNI_USE_SKIN;
	self->s.fmnodeinfo[MESH__GUARDHEAD].skin = self->s.skinnum + 1;

	return false;
}

static void SeraphThrowTorso(edict_t* self, float damage, const int mesh_part) //mxd. Added to simplify logic.
{
	if (flrand(0, (float)self->health) < damage)
	{
		self->s.fmnodeinfo[mesh_part].flags |= FMNI_USE_SKIN;
		self->s.fmnodeinfo[mesh_part].skin = self->s.skinnum + 1;
	}
}

static void SeraphGuardThrowArmUpper(edict_t* self, const float damage, const int mesh_part, const qboolean dismember_ok) //mxd. Added to simplify logic.
{
	if (flrand(0, (float)self->health) < damage)
	{
		self->s.fmnodeinfo[mesh_part].flags |= FMNI_USE_SKIN;
		self->s.fmnodeinfo[mesh_part].skin = self->s.skinnum + 1;
	}

	if (dismember_ok && flrand(0, (float)self->health) < damage * 0.75f)
	{
		int throw_nodes = 0;
		const qboolean is_left_arm = (mesh_part == MESH__LFTUPARM);
		const int hand_mesh_part = (is_left_arm ? MESH__LHANDGRD : MESH__RHAND);

		if (SeraphGuardCanThrowNode(self, hand_mesh_part, &throw_nodes))
		{
			if (is_left_arm)
				SeraphGuardCanThrowNode(self, MESH__ARMSPIKES, &throw_nodes);

			vec3_t right;
			AngleVectors(self->s.angles, NULL, right, NULL);

			vec3_t gore_spot = { 0.0f, 0.0f, self->maxs[2] * 0.3f };
			const float side = (is_left_arm ? -1.0f : 1.0f);
			VectorMA(gore_spot, 10 * side, right, gore_spot);

			SeraphGuardDropWeapon(self);
			ThrowBodyPart(self, gore_spot, throw_nodes, (int)damage, FRAME_partfly);
		}
	}
}

static void SeraphGuardThrowArmLower(edict_t* self, float damage, const int mesh_part, const qboolean dismember_ok) //mxd. Added to simplify logic.
{
	if (self->s.fmnodeinfo[mesh_part].flags & FMNI_NO_DRAW)
		return;

	if (self->s.fmnodeinfo[mesh_part].flags & FMNI_USE_SKIN)
		damage *= 1.5f; // Greater chance to cut off if previously damaged.

	if (dismember_ok && flrand(0, (float)self->health) < damage)
	{
		int throw_nodes = 0;

		if (SeraphGuardCanThrowNode(self, mesh_part, &throw_nodes))
		{
			const qboolean is_left_arm = (mesh_part == MESH__LHANDGRD);

			if (is_left_arm)
				SeraphGuardCanThrowNode(self, MESH__ARMSPIKES, &throw_nodes);

			vec3_t right;
			AngleVectors(self->s.angles, NULL, right, NULL);

			vec3_t gore_spot = { 0.0f, 0.0f, self->maxs[2] * 0.3f };
			const float side = (is_left_arm ? -1.0f : 1.0f);
			VectorMA(gore_spot, 10 * side, right, gore_spot);

			SeraphGuardDropWeapon(self);
			ThrowBodyPart(self, gore_spot, throw_nodes, (int)damage, FRAME_partfly);
		}
	}
	else
	{
		self->s.fmnodeinfo[mesh_part].flags |= FMNI_USE_SKIN;
		self->s.fmnodeinfo[mesh_part].skin = self->s.skinnum + 1;
	}
}

static void SeraphGuardThrowLeg(edict_t* self, const int mesh_part) //mxd. Added to simplify logic.
{
	if (!(self->s.fmnodeinfo[mesh_part].flags & FMNI_USE_SKIN))
	{
		self->s.fmnodeinfo[mesh_part].flags |= FMNI_USE_SKIN;
		self->s.fmnodeinfo[mesh_part].skin = self->s.skinnum + 1;
	}
}

void SeraphGuardDismember(edict_t* self, int damage, HitLocation_t hl) //mxd. Named 'seraph_guard_dismember' in original logic.
{
	qboolean dismember_ok = false;

	if (hl & hl_MeleeHit)
	{
		dismember_ok = true;
		hl &= ~hl_MeleeHit;
	}

	if (hl <= hl_NoneSpecific || hl >= hl_Max) //mxd. '> hl_Max' in original logic.
		return;

	if (hl == hl_TorsoFront && dismember_ok)
	{
		// Melee swing at chest during my melee swing.
		if (self->curAnimID == ANIM_MELEE1 || self->curAnimID == ANIM_MELEE2 || self->curAnimID == ANIM_MELEE3 || self->curAnimID == ANIM_RUN_MELEE)
			hl = ((irand(0, 2) == 0) ? hl_ArmLowerLeft : hl_ArmLowerRight);
	}

	switch (hl)
	{
		case hl_Head:
			if (SeraphGuardThrowHead(self, (float)damage, dismember_ok))
				return;
			break;

		case hl_TorsoFront: // Split in half?
			SeraphThrowTorso(self, (float)damage, MESH__FRTORSO);
			break;

		case hl_TorsoBack: // Split in half?
			SeraphThrowTorso(self, (float)damage, MESH__BKTORSO);
			break;

		case hl_ArmUpperLeft: // Left arm.
			SeraphGuardThrowArmUpper(self, (float)damage, MESH__LFTUPARM, dismember_ok);
			break;

		case hl_ArmLowerLeft:
			SeraphGuardThrowArmLower(self, (float)damage, MESH__LHANDGRD, dismember_ok);
			break;

		case hl_ArmUpperRight: // Right arm.
			SeraphGuardThrowArmUpper(self, (float)damage, MESH__RTARM, dismember_ok);
			break;

		case hl_ArmLowerRight:
			SeraphGuardThrowArmLower(self, (float)damage, MESH__RHAND, dismember_ok);
			break;

		case hl_LegUpperLeft: // Left leg.
		case hl_LegLowerLeft:
			SeraphGuardThrowLeg(self, MESH__LFTLEG);
			break;

		case hl_LegUpperRight: // Right leg.
		case hl_LegLowerRight:
			SeraphGuardThrowLeg(self, MESH__RTLEG);
			break;

		default:
			break;
	}

	if (self->s.fmnodeinfo[MESH__RHAND].flags & FMNI_NO_DRAW)
	{
		self->monsterinfo.aiflags |= AI_NO_MISSILE;

		if (self->s.fmnodeinfo[MESH__AXE].flags & FMNI_NO_DRAW)
			self->monsterinfo.aiflags |= AI_NO_MELEE;
	}

	if ((self->monsterinfo.aiflags & AI_NO_MELEE) && (self->monsterinfo.aiflags & AI_NO_MISSILE))
	{
		self->monsterinfo.aiflags |= AI_COWARD;
		SetAnim(self, ANIM_BACKUP);
	}
}

#pragma endregion

#pragma region ========================== Action functions ==========================

void seraph_guard_check_poke(edict_t* self) //mxd. Named 'seraph_guard_checkpoke' in original logic.
{
	// Really, this is a given, but it could fail...
	if (!M_ValidTarget(self, self->enemy))
	{
		G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
		return;
	}

	if (M_DistanceToTarget(self, self->enemy) >= 120.0f)
		return;

	// Set this for any uses below.
	vec3_t forward;
	AngleVectors(self->s.angles, forward, NULL, NULL);

	vec3_t attack_vel;
	VectorScale(forward, 2.0f, attack_vel);

	if (M_PredictTargetEvasion(self, self->enemy, attack_vel, self->enemy->velocity, 150.0f, SGUARD_NUM_PREDICTED_FRAMES))
	{
		const int chance = irand(0, 100);

		if (chance < 40)
			SetAnim(self, ANIM_MELEE3);
		else if (chance < 60)
			SetAnim(self, ANIM_MELEE2);
	}
	else
	{
		SetAnim(self, ((irand(0, 1) == 1) ? ANIM_RUN_MELEE : ANIM_MELEE2));
	}
}

void seraph_guard_death_loop(edict_t* self)
{
	SetAnim(self, ANIM_DEATH2_LOOP);
}

void seraph_guard_check_land(edict_t* self)
{
	M_ChangeYaw(self);

	vec3_t end_pos;
	VectorCopy(self->s.origin, end_pos);
	end_pos[2] -= 48.0f;

	trace_t trace;
	gi.trace(self->s.origin, self->mins, self->maxs, end_pos, self, MASK_MONSTERSOLID, &trace);

	if ((trace.fraction < 1.0f || trace.startsolid || trace.allsolid) && self->curAnimID != ANIM_DEATH2_END && self->curAnimID != ANIM_DEATH2_GO)
	{
		self->elasticity = 1.25f;
		self->friction = 0.5f;
		SetAnim(self, ANIM_DEATH2_END);
	}
}

void seraph_guard_dead(edict_t* self)
{
	M_EndDeath(self);
}

void seraph_guard_strike(edict_t* self, float damage, float forward, float up)
{
	if (self->monsterinfo.aiflags & AI_NO_MELEE)
		return;

	damage *= self->s.scale;
	self->monsterinfo.attack_finished = level.time + (3.0f - skill->value) * 2.0f + flrand(0.0f, 1.0f);

	qboolean knockback = true;
	vec3_t start_offset;
	vec3_t end_offset;

	switch (self->curAnimID)
	{
		case ANIM_MELEE2:
			VectorSet(start_offset, 16.0f, -16.0f, 24.0f);
			VectorSet(end_offset, 124.0f, -16.0f, 16.0f);
			break;

		case ANIM_MELEE3:
			VectorSet(start_offset, 32.0f, -48.0f, 34.0f);
			VectorSet(end_offset, 64.0f, 64.0f, -8.0f);
			break;

		default:
			knockback = false;
			VectorSet(start_offset, 32.0f, -16.0f, 64.0f);
			VectorSet(end_offset, 72.0f, 16.0f, -8.0f);
			break;
	}

	Vec3ScaleAssign(self->s.scale, start_offset);
	Vec3ScaleAssign(self->s.scale, end_offset);

	const vec3_t mins = { -4.0f, -4.0f, -4.0f };
	const vec3_t maxs = {  4.0f,  4.0f,  4.0f };

	trace_t trace;
	vec3_t direction;
	edict_t* victim = M_CheckMeleeLineHit(self, start_offset, end_offset, mins, maxs, &trace, direction);

	if (victim == NULL) // Missed.
	{
		// Play swoosh sound.
		gi.sound(self, CHAN_WEAPON, sounds[SND_ATTACK_MISS], 1.0f, ATTN_NORM, 0.0f);
		return;
	}

	if (victim == self) // Hit wall.
	{
		// Create a spark effect.
		gi.CreateEffect(NULL, FX_SPARKS, CEF_FLAG6, trace.endpos, "d", direction);
		gi.sound(self, CHAN_WEAPON, sounds[SND_HIT_WALL], 1, ATTN_NORM, 0);

		return;
	}

	// Hurt whatever we were whacking away at.
	if (self->curAnimID == ANIM_MELEE3)
	{
		gi.CreateEffect(NULL, FX_WEAPON_STAFF_STRIKE, 0, trace.endpos, "db", trace.plane.normal, 2);
		gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/staffhit_2.wav"), 1.0f, ATTN_NORM, 0.0f);
	}
	else
	{
		gi.sound(self, CHAN_WEAPON, sounds[SND_ATTACK], 1.0f, ATTN_NORM, 0.0f);
	}

	damage *= (skill->value + 1.0f) / 3.0f * flrand(0.85f, 1.15f); // Add some variance to the hit, since it passes a constant (skill 0 = 1/3, skill 3 = 1 1/3).

	vec3_t blood_dir;
	VectorSubtract(end_offset, start_offset, blood_dir);
	VectorNormalize(blood_dir);

	if (knockback)
		T_Damage(victim, self, self, direction, trace.endpos, blood_dir, (int)damage, (int)damage * 20, DAMAGE_EXTRA_KNOCKBACK | DAMAGE_DISMEMBER | DAMAGE_DOUBLE_DISMEMBER | DAMAGE_EXTRA_BLOOD, MOD_DIED);
	else
		T_Damage(victim, self, self, direction, trace.endpos, blood_dir, (int)damage, 0, DAMAGE_NO_KNOCKBACK | DAMAGE_DISMEMBER | DAMAGE_DOUBLE_DISMEMBER | DAMAGE_EXTRA_BLOOD, MOD_DIED);

	// Knockdown player?
	if (self->curAnimID == ANIM_MELEE3 && victim->client != NULL && victim->health > 0)
		if (victim->client->playerinfo.lowerseq != ASEQ_KNOCKDOWN && AI_IsInfrontOf(self, victim))
			P_KnockDownPlayer(&victim->client->playerinfo);
}

void seraph_guard_jump(edict_t* self) //mxd. Named 'seraphGuardApplyJump' in original logic.
{
	self->jump_time = level.time + 0.75f;
	VectorCopy(self->movedir, self->velocity);
	VectorNormalize(self->movedir);
}

void seraph_guard_pause(edict_t* self)
{
	if ((self->spawnflags & MSF_FIXED) && self->curAnimID == ANIM_DELAY && self->enemy != NULL)
	{
		self->monsterinfo.searchType = SEARCH_COMMON;
		MG_FaceGoal(self, true);
	}

	self->mood_think(self);

	switch (self->ai_mood)
	{
	case AI_MOOD_ATTACK:
			G_PostMessage(self, ((self->ai_mood_flags & AI_MOOD_FLAG_MISSILE) ? MSG_MISSILE : MSG_MELEE), PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_PURSUE:
		case AI_MOOD_NAVIGATE:
		case AI_MOOD_WALK:
		case AI_MOOD_FLEE:
			G_PostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_STAND:
			if (self->enemy == NULL) //TODO: else what?
				G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_DELAY:
			SetAnim(self, ANIM_DELAY);
			break;

		case AI_MOOD_WANDER:
			SetAnim(self, ANIM_WALK);
			break;

		case AI_MOOD_JUMP:
			SetAnim(self, ANIM_FJUMP);
			break;

		default:
			break;
	}
}

void seraph_guard_fire(edict_t* self)
{
	if (self->enemy == NULL)
		return;

	if (M_DistanceToTarget(self, self->enemy) >= self->min_missile_range)
	{
		SeraphGuardProjectile(self);
		return;
	}

	vec3_t start_offset = { 12.0f, -18.0f, 24.0f };
	vec3_t end_offset = { 88.0f, -4.0f, -16.0f };

	Vec3ScaleAssign(self->s.scale, start_offset);
	Vec3ScaleAssign(self->s.scale, end_offset);

	const vec3_t mins = { -4.0f, -4.0f, -4.0f };
	const vec3_t maxs = {  4.0f,  4.0f,  4.0f };

	trace_t trace;
	vec3_t direction;
	edict_t* victim = M_CheckMeleeLineHit(self, start_offset, end_offset, mins, maxs, &trace, direction);

	if (victim == NULL) // Missed.
	{
		// Play swoosh sound.
		gi.sound(self, CHAN_WEAPON, sounds[SND_ATTACK_MISS], 1.0f, ATTN_NORM, 0.0f);
		return;
	}

	if (victim == self) // Hit wall.
	{
		gi.sound(self, CHAN_WEAPON, sounds[SND_FIST_HIT_WALL], 1.0f, ATTN_NORM, 0.0f);
		return;
	}

	// Hurt whatever we were whacking away at.
	vec3_t blood_dir;
	VectorSubtract(end_offset, start_offset, blood_dir);
	VectorNormalize(blood_dir);

	const float damage = self->s.scale * 15.0f * ((skill->value + 1.0f) / 3.0f); // skill 0 = 1/3, skill 3 = 1 1/3.
	T_Damage(victim, self, self, direction, trace.endpos, blood_dir, (int)damage, (int)damage * 20, DAMAGE_EXTRA_KNOCKBACK, MOD_DIED);

	gi.sound(self, CHAN_WEAPON, sounds[SND_HIT_WALL], 1.0f, ATTN_NORM, 0.0f);

	// Knockdown player?
	if (victim->client != NULL && victim->health > 0 && victim->client->playerinfo.lowerseq != ASEQ_KNOCKDOWN && AI_IsInfrontOf(self, victim))
		P_KnockDownPlayer(&victim->client->playerinfo);
}

void seraph_guard_back(edict_t* self, float distance)
{
	if (!MG_TryWalkMove(self, self->s.angles[YAW] + 180.0f, distance, true) && irand(0, 1000) == 0) // 0.0009% chance to run away when MG_TryWalkMove() fails. //TODO: chance seems way too low. Scale with difficulty?
	{
		self->monsterinfo.aiflags |= AI_COWARD;
		SetAnim(self, ANIM_RUN);
	}
}

#pragma endregion

void SeraphGuardStaticsInit(void)
{
	static ClassResourceInfo_t res_info; //mxd. Made local static.

	classStatics[CID_SERAPH_GUARD].msgReceivers[MSG_STAND] = SeraphGuardStandMsgHandler;
	classStatics[CID_SERAPH_GUARD].msgReceivers[MSG_RUN] = SeraphGuardRunMsgHandler;
	classStatics[CID_SERAPH_GUARD].msgReceivers[MSG_MELEE] = SeraphGuardMeleeMsgHandler;
	classStatics[CID_SERAPH_GUARD].msgReceivers[MSG_MISSILE] = SeraphGuardMissileMsgHandler;
	classStatics[CID_SERAPH_GUARD].msgReceivers[MSG_PAIN] = SeraphGuardPainMsgHandler;
	classStatics[CID_SERAPH_GUARD].msgReceivers[MSG_DEATH] = SeraphGuardDeathMsgHandler;
	classStatics[CID_SERAPH_GUARD].msgReceivers[MSG_DEATH_PAIN] = SeraphGuardDeathPainMsgHandler;
	classStatics[CID_SERAPH_GUARD].msgReceivers[MSG_CHECK_MOOD] = SeraphGuardCheckMoodMsgHandler;
	classStatics[CID_SERAPH_GUARD].msgReceivers[MSG_DISMEMBER] = DismemberMsgHandler;
	classStatics[CID_SERAPH_GUARD].msgReceivers[MSG_VOICE_SIGHT] = SeraphGuardVoiceSightMsgHandler;

	res_info.numAnims = NUM_ANIMS;
	res_info.animations = animations;
	res_info.modelIndex = gi.modelindex("models/monsters/guard/tris.fm");

	sounds[SND_ATTACK] = gi.soundindex("monsters/seraph/guard/attack.wav");
	sounds[SND_ATTACK_MISS] = gi.soundindex("monsters/seraph/guard/attack_miss.wav");
	sounds[SND_HIT_WALL] = gi.soundindex("weapons/staffhitwall.wav");
	sounds[SND_MISSILE] = gi.soundindex("monsters/seraph/guard/spell.wav");
	sounds[SND_MISSHIT] = gi.soundindex("monsters/seraph/guard/spellhit.wav");
	sounds[SND_FIST_HIT_WALL] = gi.soundindex("objects/bam1.wav");

	sounds[SND_DEATH1] = gi.soundindex("monsters/seraph/death1.wav");
	sounds[SND_DEATH2] = gi.soundindex("monsters/seraph/death2.wav");
	sounds[SND_DEATH3] = gi.soundindex("monsters/seraph/wimpdeath1.wav");
	sounds[SND_DEATH4] = gi.soundindex("monsters/seraph/wimpdeath2.wav");

	sounds[SND_PAIN1] = gi.soundindex("monsters/seraph/pain1.wav");
	sounds[SND_PAIN2] = gi.soundindex("monsters/seraph/pain2.wav");
	sounds[SND_PAIN3] = gi.soundindex("monsters/seraph/pain3.wav");
	sounds[SND_PAIN4] = gi.soundindex("monsters/seraph/pain4.wav");

	sounds[SND_SIGHT1] = gi.soundindex("monsters/seraph/guard/sight1.wav");
	sounds[SND_SIGHT2] = gi.soundindex("monsters/seraph/guard/sight2.wav");
	sounds[SND_SIGHT3] = gi.soundindex("monsters/seraph/guard/sight3.wav");
	sounds[SND_SIGHT4] = gi.soundindex("monsters/seraph/guard/sight4.wav");

	res_info.numSounds = NUM_SOUNDS;
	res_info.sounds = sounds;

	classStatics[CID_SERAPH_GUARD].resInfo = &res_info;
}

// QUAKED monster_seraph_guard(1 .5 0) (-24 -24 -34) (24 24 34) AMBUSH ASLEEP GOLEM 8 16 32 64 FIXED WANDER MELEE_LEAD STALK COWARD EXTRA1 EXTRA2 EXTRA3 EXTRA4
// The big, ugly, brutal seraph guard.

// Spawnflags:
// AMBUSH		- Will not be woken up by other monsters or shots from player.
// ASLEEP		- Will not appear until triggered.
// GOLEM		- Engage statue mode.
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
// mintel					- Monster intelligence - this basically tells a monster how many buoys away an enemy has to be for it to give up (default 20).
// melee_range				- How close the player has to be for the monster to go into melee. If this is zero, the monster will never melee.
//							  If it is negative, the monster will try to keep this distance from the player.
//							  If the monster has a backup, he'll use it if too close, otherwise, a negative value here means the monster will just stop
//							  running at the player at this distance (default 100).
//							 Examples:
//								melee_range = 60 - monster will start swinging it player is closer than 60.
//								melee_range = 0 - monster will never do a melee attack.
//								melee_range = -100 - monster will never do a melee attack and will back away (if it has that ability) when player gets too close.
// missile_range			- Maximum distance the player can be from the monster to be allowed to use it's ranged attack (default 1024).
// min_missile_range		- Minimum distance the player can be from the monster to be allowed to use it's ranged attack (default 64).
// bypass_missile_chance	- Chance that a monster will NOT fire it's ranged attack, even when it has a clear shot. This, in effect, will make the monster
//							  come in more often than hang back and fire. A percentage (0 = always fire/never close in, 100 = never fire/always close in) - must be whole number (default 0).
// jump_chance				- Every time the monster has the opportunity to jump, what is the chance (out of 100) that he will... (100 = jump every time) - must be whole number (default 20).
// wakeup_distance			- How far (max) the player can be away from the monster before it wakes up. This means that if the monster can see the player,
//							  at what distance should the monster actually notice him and go for him (default 1024).
// NOTE: A value of zero will result in defaults, if you actually want zero as the value, use -1.
void SP_monster_seraph_guard(edict_t* self)
{
	if (self->spawnflags & SF_SGUARD_GOLEM)
	{
		self->clipmask = MASK_MONSTERSOLID;

		if (self->health == 0)
			self->health = SGUARD_HEALTH * 2;

		self->health = MonsterHealth(self->health);
		self->max_health = self->health;

		self->mass = SGUARD_MASS * 10;
		self->yaw_speed = 12.0f;

		self->movetype = PHYSICSTYPE_STEP;
		self->solid = SOLID_BBOX;

		if (self->s.scale == 0.0f)
		{
			self->s.scale = 2.5f;
			self->monsterinfo.scale = self->s.scale;
		}

		VectorCopy(STDMinsForClass[self->classID], self->mins);
		VectorCopy(STDMaxsForClass[self->classID], self->maxs);

		self->materialtype = MAT_STONE;

		self->s.modelindex = (byte)classStatics[CID_SERAPH_GUARD].resInfo->modelIndex;
		self->s.skinnum = 2; //BUGFIX: mxd. Actually use stone skin. Original logic sets this to 3, then re-assigns to 0.

		self->monsterinfo.otherenemyname = "monster_rat";

		// Turn off the Overlord pieces.
		self->s.fmnodeinfo[MESH__WHIP].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__LHANDBOSS].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__PITHEAD].flags |= FMNI_NO_DRAW;

		self->s.frame = FRAME_idle; //TODO: randomly pick between FRAME_idle and FRAME_idle29?

		//TODO: enable golem_awaken() logic? Would require either stone golem pain skin or disabling skin switching in SeraphGuardDismember()...
	}
	else
	{
		if (!M_WalkmonsterStart(self)) //mxd. M_Start -> M_WalkmonsterStart.
			return; // Failed initialization.

		self->msgHandler = DefaultMsgHandler;

		if (self->health == 0)
			self->health = SGUARD_HEALTH;

		// Apply to the end result (whether designer set or not).
		self->health = MonsterHealth(self->health);
		self->max_health = self->health;

		self->mass = SGUARD_MASS;
		self->yaw_speed = 24.0f;

		if (irand(0, 2) != 0)
			self->ai_mood_flags |= AI_MOOD_FLAG_PREDICT;

		self->movetype = PHYSICSTYPE_STEP;
		self->solid = SOLID_BBOX;
		self->monsterinfo.dismember = SeraphGuardDismember;

		if (self->s.scale == 0.0f)
		{
			self->s.scale = MODEL_SCALE;
			self->monsterinfo.scale = self->s.scale;
		}

		VectorCopy(STDMinsForClass[self->classID], self->mins);
		VectorCopy(STDMaxsForClass[self->classID], self->maxs);

		self->materialtype = MAT_FLESH;

		self->s.modelindex = (byte)classStatics[CID_SERAPH_GUARD].resInfo->modelIndex;
		self->s.skinnum = 0;

		self->monsterinfo.otherenemyname = "monster_rat";

		// Turn off the Overlord pieces.
		self->s.fmnodeinfo[MESH__WHIP].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__LHANDBOSS].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__PITHEAD].flags |= FMNI_NO_DRAW;

		MG_InitMoods(self);
		G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);

		self->melee_range *= self->s.scale;
	}
}