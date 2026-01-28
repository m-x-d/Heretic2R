//
// m_spreader.c
//
// Copyright 1998 Raven Software
//

#include "m_spreader.h"
#include "m_spreader_shared.h"
#include "m_spreader_anim.h"
#include "m_spreader_moves.h"
#include "m_spreadermist.h"
#include "g_DefaultMessageHandler.h"
#include "g_debris.h" //mxd
#include "mg_ai.h" //mxd
#include "mg_guide.h" //mxd
#include "m_stats.h"
#include "p_anims.h"
#include "Random.h"
#include "Utilities.h"
#include "Vector.h"
#include "g_monster.h"

#pragma region ========================== Spreader Base Info ==========================

static const animmove_t* animations[NUM_ANIMS] =
{
	&spreader_move_attack1,
	&spreader_move_attack2,
	&spreader_move_backup1,
	&spreader_move_backattack1,
	&spreader_move_death1_go,
	&spreader_move_death1_loop,
	&spreader_move_death1_end,
	&spreader_move_duck1,
	&spreader_move_dkatck1,
	&spreader_move_duckdown,
	&spreader_move_duckstill,
	&spreader_move_duckup,
	&spreader_move_idle1,
	&spreader_move_pain1,
	&spreader_move_pvtlt1,
	&spreader_move_pvtrt1,
	&spreader_move_rnatck1,
	&spreader_move_run1,
	&spreader_move_land,
	&spreader_move_inair,
	&spreader_move_fjump,
	&spreader_move_walk1,
	&spreader_move_walk2,
	&spreader_move_death2,
	&spreader_move_fly,
	&spreader_move_flyloop,
	&spreader_move_fdie,
	&spreader_move_dead,
	&spreader_move_delay
};

static int sounds[NUM_SOUNDS];

#pragma endregion

#pragma region ========================== Utility functions =========================

static void SpreaderCrouch(edict_t* self) //mxd. Named 'spreader_crouch' in original logic. //TODO: inline?
{
	VectorSet(self->maxs, 16.0f, 16.0f, 0.0f);
	self->viewheight = 0;
	SetAnim(self, ANIM_DUCKDOWN);
}

static qboolean SpreaderCheckUncrouch(edict_t* self) //mxd. Named 'spreader_check_uncrouch' in original logic.
{
	//FIXME: Need to ultimately make sure this is ok!
	const vec3_t mins = { self->mins[0], self->mins[1], 0.0f };
	const vec3_t maxs = { self->maxs[0], self->maxs[1], 1.0f };
	const float desired_height = STDMaxsForClass[CID_SPREADER][2] * self->s.scale;

	trace_t trace;
	const vec3_t end_pos = VEC3_INITA(self->s.origin, 0.0f, 0.0f, desired_height);
	gi.trace(self->s.origin, mins, maxs, end_pos, self, MASK_MONSTERSOLID, &trace);

	if (trace.fraction < 1.0f)
		return false;

	self->maxs[2] = desired_height;
	self->viewheight = (int)(self->maxs[2] - self->s.scale * 8.0f);

	return true;
}

static qboolean SpreaderCanThrowNode(edict_t* self, const int node_id, int* throw_nodes) //mxd. Named 'canthrownode_ps' in original logic.
{
	static const int bit_for_mesh_node[NUM_MESH_NODES] = //mxd. Made local static.
	{
		BIT_PARENT,
		BIT_CHILD,
		BIT_BODY,
		BIT_BOMB,
		BIT_RITLEG,
		BIT_LFTARM,
		BIT_LFTLEG,
		BIT_HEAD,
		BIT_RITARM,
		BIT_TANK3,
		BIT_TANK2,
		BIT_TANK1,
		BIT_HOSE
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

static void SpreaderDropWeapon(edict_t* self) //mxd. Named 'spreader_dropweapon' in original logic.
{
	if (self->s.fmnodeinfo[BIT_BOMB].flags & FMNI_NO_DRAW)
		return;

	vec3_t right;
	AngleVectors(self->s.angles, NULL, right, NULL);

	vec3_t hand_pos;
	VectorScale(right, -12.0f, hand_pos);

	ThrowWeapon(self, &hand_pos, BIT_BOMB, 0, 0);
	self->s.fmnodeinfo[MESH__BOMB].flags |= FMNI_NO_DRAW;
}

static void SpreaderTakeOff(edict_t* self) //mxd. Named 'spreaderTakeOff' in original logic.
{
	self->msgHandler = DeadMsgHandler;
	self->isBlocked = SpreaderIsBlocked;
	self->bounced = SpreaderIsBlocked;

	vec3_t forward;
	AngleVectors(self->s.angles, forward, NULL, NULL);
	VectorMA(self->s.origin, -12.0f, forward, self->spreader_mist_origin);

	self->spreader_mist_origin[2] += self->maxs[2] * 0.8f;

	// Create the volume effect for the damage.
	edict_t* gas = CreateRadiusDamageEnt(self, self, 1, 0, 150.0f, 0.0f, DAMAGE_NO_BLOOD | DAMAGE_ALIVE_ONLY, 4.5f, 0.1f, NULL, self->spreader_mist_origin, true);

	gas->svflags |= SVF_ALWAYS_SEND;
	gas->s.effects = EF_MARCUS_FLAG1;

	gi.CreateEffect(&gas->s, FX_PLAGUEMISTEXPLODE, CEF_OWNERS_ORIGIN, self->spreader_mist_origin, "b", 70);

	gi.sound(self, CHAN_VOICE, sounds[SND_PAIN], 1.0f, ATTN_NORM, 0.0f);
	gi.sound(self, CHAN_WEAPON, sounds[SND_SPRAYSTART], 1.0f, ATTN_NORM, 0.0f);
	gi.sound(self, CHAN_AUTO, sounds[SND_BOMB], 1.0f, ATTN_NORM, 0.0f);

	self->spreader_spray_sound_time = level.time + flrand(0.4f, 0.8f); // For sound loop.

	self->velocity[0] = 0.0f; // Not realistic, but funnier.
	self->velocity[1] = 0.0f;
	self->velocity[2] += 150.0f;

	self->avelocity[YAW] = flrand(200.0f, 600.0f) * (float)(Q_sign(irand(-1, 0)));
	self->movetype = PHYSICSTYPE_FLY;
	self->svflags |= (SVF_ALWAYS_SEND | SVF_TAKE_NO_IMPACT_DMG);

	SpreaderDropWeapon(self);
	SetAnim(self, ANIM_FLY);
}

#pragma endregion

#pragma region ========================== Message handlers ==========================

static void SpreaderCheckMoodMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'spreader_check_mood' in original logic.
{
	G_ParseMsgParms(msg, "i", &self->ai_mood);
	spreader_pause(self);
}

static void SpreaderPainMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'spreader_pain' in original logic.
{
	int temp;
	int damage;
	qboolean force_pain;
	G_ParseMsgParms(msg, "eeiii", &temp, &temp, &force_pain, &damage, &temp);

	// Weighted random based on health compared to the maximum it was at.
	if (force_pain || (irand(0, self->max_health + 50) > self->health && irand(0, 2) != 0)) //mxd. flrand() in original logic.
	{
		gi.sound(self, CHAN_BODY, sounds[SND_PAIN], 1.0f, ATTN_NORM, 0.0f);
		SetAnim(self, ANIM_PAIN1);
	}
}

static void SpreaderStandMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'spreader_stand' in original logic.
{
	SetAnim(self, ANIM_IDLE1);
}

static void SpreaderRunMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'spreader_run' in original logic.
{
	if (!M_ValidTarget(self, self->enemy))
	{
		// If our enemy is dead, we need to stand.
		G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
		return;
	}

	SetAnim(self, ((self->spawnflags & MSF_FIXED) ? ANIM_DELAY : ANIM_RUN1));
}

static void SpreaderWalkMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'spreader_walk' in original logic.
{
	SetAnim(self, ((self->spawnflags & MSF_FIXED) ? ANIM_DELAY : ANIM_WALK1));
}

static void SpreaderMeleeMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'spreader_melee' in original logic.
{
	if (!M_ValidTarget(self, self->enemy))
	{
		SetAnim(self, ANIM_IDLE1);
		return;
	}

	const int chance = irand(0, 100);

	if (self->spawnflags & MSF_FIXED)
	{
		SetAnim(self, ((chance < 50) ? ANIM_DUCKDOWN : ANIM_DELAY));
		return;
	}

	if (M_DistanceToTarget(self, self->enemy) < 64.0f)
	{
		// Bumped into player? Knock him down.
		if (self->curAnimID == ANIM_RUNATTACK && self->enemy->health > 0 && self->enemy->client != NULL && self->enemy->client->playerinfo.lowerseq != ASEQ_KNOCKDOWN)
			if (irand(0, 2) == 0 && AI_IsInfrontOf(self->enemy, self))
				P_KnockDownPlayer(&self->enemy->client->playerinfo);

		if (self->curAnimID == ANIM_RUNATTACK || (self->curAnimID == ANIM_RUN1 && self->s.frame > FRAME_run1))
		{
			if (chance < 40)
				SetAnim(self, ANIM_DUCKDOWN);
			else
				SetAnim(self, ANIM_ATTACK2);
		}
		else
		{
			if (irand(0, 1) == 1 && !(self->monsterinfo.aiflags & AI_NO_MELEE))
				SetAnim(self, ANIM_BACKATTACK);
			else
				SetAnim(self, ANIM_BACKUP);
		}

		return;
	}

	if (chance < 20)
		SetAnim(self, ANIM_BACKUP);
	else if (chance < 40)
		SetAnim(self, ANIM_DUCKDOWN);
	else if (chance < 60 || (self->monsterinfo.aiflags & AI_NO_MELEE))
		SetAnim(self, ANIM_RUN1);
	else
		SetAnim(self, ANIM_ATTACK2);
}

static void SpreaderMissileMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'spreader_missile' in original logic.
{
	if (!M_ValidTarget(self, self->enemy))
	{
		SetAnim(self, ANIM_IDLE1);
		return;
	}

	const float dist = M_DistanceToTarget(self, self->enemy);

	if (dist < 64.0f)
	{
		if (!(self->monsterinfo.aiflags & AI_NO_MELEE) && irand(0, 5) != 0)
			SetAnim(self, ANIM_BACKATTACK);
		else
			SetAnim(self, ANIM_BACKUP);

		return;
	}

	const int chance = irand(0, 100);

	if (dist < 128.0f)
	{
		if (chance < 20)
			SetAnim(self, ANIM_BACKUP);
		else if (chance < 40)
			SetAnim(self, ANIM_DUCKDOWN);
		else if (chance < 60 || (self->monsterinfo.aiflags & AI_NO_MELEE))
			SetAnim(self, ANIM_RUN1);
		else
			SetAnim(self, ANIM_ATTACK2);

		return;
	}

	vec3_t forward;
	AngleVectors(self->s.angles, forward, NULL, NULL);

	vec3_t attack_vel;
	VectorScale(forward, 200.0f, attack_vel);
	const qboolean in_range = M_PredictTargetEvasion(self, self->enemy, attack_vel, self->enemy->velocity, 150.0f, 5.0f);

	// See what the predicted outcome is.
	trace_t trace;
	if (!(self->monsterinfo.aiflags & AI_NO_MELEE) && in_range && chance < 25 && M_CheckMeleeHit(self, 200.0f, &trace) == self->enemy)
		SetAnim(self, ANIM_RUNATTACK);
	else if (self->monsterinfo.aiflags & AI_NO_MISSILE)
		SetAnim(self, ANIM_RUN1);
	else
		SetAnim(self, ANIM_ATTACK1);
}

static void SpreaderFallbackMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'spreader_fallback' in original logic.
{
	if (self->spawnflags & MSF_FIXED)
		SetAnim(self, ANIM_DELAY);
	else if (!(self->monsterinfo.aiflags & AI_NO_MELEE))
		SetAnim(self, ANIM_BACKATTACK);
	else
		SetAnim(self, ANIM_BACKUP);
}

static void SpreaderEvadeMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'spreader_evade' in original logic.
{
	edict_t* projectile;
	HitLocation_t hl;
	float eta;
	G_ParseMsgParms(msg, "eif", &projectile, &hl, &eta);

	if (irand(0, 1) == 0 || self->curAnimID == ANIM_DUCKDOWN || self->curAnimID == ANIM_DUCKUP || self->curAnimID == ANIM_DUCKSTILL)
		return;

	int duck_chance;

	switch (hl)
	{
		case hl_Head:
			duck_chance = 100;
			break;

		case hl_TorsoFront:
		case hl_TorsoBack:
		case hl_ArmUpperLeft:
		case hl_ArmUpperRight:
			duck_chance = 75;
			break;

		case hl_ArmLowerLeft:
		case hl_ArmLowerRight:
			duck_chance = 35;
			break;

		case hl_LegUpperLeft:
		case hl_LegLowerLeft:
		case hl_LegUpperRight:
		case hl_LegLowerRight:
			duck_chance = 0;
			break;

		default:
			duck_chance = 10;
			break;
	}

	if (irand(0, 100) < duck_chance)
	{
		self->evade_debounce_time = level.time + eta;
		SpreaderCrouch(self);
	}
}

static void SpreaderDeathMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'spreader_death' in original logic.
{
	edict_t* target = NULL; //mxd. Pre-initialize.

	if (msg != NULL)
	{
		edict_t* inflictor;
		edict_t* attacker;
		float damage;
		G_ParseMsgParms(msg, "eeei", &target, &inflictor, &attacker, &damage);
	}

	self->s.fmnodeinfo[MESH__BOMB].flags |= FMNI_NO_DRAW; //mxd. Hide grenade. Original logic calls spreader_hidegrenade() here, which also plays SND_THROW.
	M_StartDeath(self, ANIM_DEATH1_GO);

	if (self->health < -80) // To be gibbed.
		return;

	if (self->health < -10)
	{
		SetAnim(self, ANIM_DEATH1_GO);
		VectorClear(self->knockbackvel);

		if (target != NULL) //BUGFIX: mxd. No NULL check in original logic.
		{
			vec3_t dir;
			VectorNormalize2(target->velocity, dir);

			vec3_t yaw_dir;
			VectorScale(dir, -1.0f, yaw_dir);

			self->ideal_yaw = VectorYaw(yaw_dir);
			self->yaw_speed = 16.0f;

			VectorScale(dir, 300.0f, self->velocity);
			self->velocity[2] = flrand(150.0f, 250.0f); //mxd. irand() in original logic.
		}
	}
	else
	{
		SetAnim(self, ANIM_DEATH2);
	}

	gi.sound(self, CHAN_BODY, sounds[SND_DEATH], 1.0f, ATTN_NORM, 0.0f);
}

static void SpreaderDeathPainMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'spreader_dead_pain' in original logic.
{
	if (msg != NULL && !(self->svflags & SVF_PARTS_GIBBED))
		DismemberMsgHandler(self, msg);
}

static void SpreaderJumpMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'spreader_jump' in original logic.
{
	SetAnim(self, ANIM_FJUMP);
}

#pragma endregion

#pragma region ========================== Edict callbacks ===========================

static qboolean SpreaderThrowHead(edict_t* self, float damage, const qboolean dismember_ok) //mxd. Added to simplify logic.
{
	if (self->s.fmnodeinfo[MESH__HEAD].flags & FMNI_NO_DRAW)
		return false;

	if (self->s.fmnodeinfo[MESH__HEAD].flags & FMNI_USE_SKIN)
		damage *= 1.5f; // Greater chance to cut off if previously damaged.

	if (dismember_ok && flrand(0, (float)self->health) < damage * 0.3f)
	{
		int throw_nodes = 0;
		SpreaderCanThrowNode(self, MESH__HEAD, &throw_nodes);

		vec3_t gore_spot = { 0.0f, 0.0f, 18.0f };
		ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);

		Vec3AddAssign(self->s.origin, gore_spot);
		SprayDebris(self, gore_spot, 8, damage);

		if (self->health > 0)
		{
			self->health = 1;
			T_Damage(self, self, self, vec3_origin, vec3_origin, vec3_origin, 10, 20, 0, MOD_DIED);
		}

		return true;
	}

	self->s.fmnodeinfo[MESH__HEAD].flags |= FMNI_USE_SKIN;
	self->s.fmnodeinfo[MESH__HEAD].skin = self->s.skinnum + 1;

	return false;
}

static qboolean SpreaderThrowTorso(edict_t* self, float damage, const float damage_scaler, const int chance) //mxd. Added to simplify logic.
{
	if (self->s.fmnodeinfo[MESH__BODY].flags & FMNI_USE_SKIN)
		damage *= 1.5f; // Greater chance to cut off if previously damaged.

	if (flrand(0, (float)self->health) < damage * damage_scaler && irand(0, chance) == 0) // One in 10 chance of the takeoff even if reqs met.
	{
		// Fly straight up, hit ceiling, head squish, otherwise go though sky.
		int throw_nodes = 0;
		SpreaderCanThrowNode(self, MESH__TANK3, &throw_nodes);
		SpreaderCanThrowNode(self, MESH__TANK2, &throw_nodes);
		SpreaderCanThrowNode(self, MESH__TANK1, &throw_nodes);
		SpreaderCanThrowNode(self, MESH__HOSE, &throw_nodes);

		const vec3_t gore_spot = { 0.0f, 0.0f, 12.0f };
		ThrowWeapon(self, &gore_spot, throw_nodes, 0, FRAME_death17);

		if (self->health > 0)
			SpreaderTakeOff(self);

		return true;
	}

	self->s.fmnodeinfo[MESH__BODY].flags |= FMNI_USE_SKIN;
	self->s.fmnodeinfo[MESH__BODY].skin = self->s.skinnum + 1;

	return false;
}

static void SpreaderThrowArm(edict_t* self, float damage, const int mesh_part, const qboolean dismember_ok) //mxd. Added to simplify logic.
{
	if (self->s.fmnodeinfo[mesh_part].flags & FMNI_NO_DRAW)
		return;

	const qboolean is_left_arm = (mesh_part == MESH__LFTARM); //mxd

	if (is_left_arm && (self->s.fmnodeinfo[mesh_part].flags & FMNI_USE_SKIN))
		damage *= 1.5f; // Greater chance to cut off if previously damaged.

	if (dismember_ok && flrand(0, (float)self->health) < damage * 0.75f)
	{
		int throw_nodes = 0;

		if (SpreaderCanThrowNode(self, mesh_part, &throw_nodes))
		{
			if (!is_left_arm)
				SpreaderDropWeapon(self);

			vec3_t right;
			AngleVectors(self->s.angles, NULL, right, NULL);

			vec3_t gore_spot = { 0.0f, 0.0f, self->maxs[2] * 0.3f };
			const float side = (is_left_arm ? -1.0f : 1.0f);
			VectorMA(gore_spot, 10.0f * side, right, gore_spot);

			ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);
		}
	}
	else
	{
		self->s.fmnodeinfo[mesh_part].flags |= FMNI_USE_SKIN;
		self->s.fmnodeinfo[mesh_part].skin = self->s.skinnum + 1;
	}
}

static void SpreaderThrowLeg(edict_t* self, const int mesh_part) //mxd. Added to simplify logic.
{
	if (!(self->s.fmnodeinfo[mesh_part].flags & FMNI_NO_DRAW))
	{
		self->s.fmnodeinfo[mesh_part].flags |= FMNI_USE_SKIN;
		self->s.fmnodeinfo[mesh_part].skin = self->s.skinnum + 1;
	}
}

void SpreaderDismember(edict_t* self, int damage, HitLocation_t hl) //mxd. Named 'spreader_dismember' in original logic.
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
			if (SpreaderThrowHead(self, (float)damage, dismember_ok))
				return;
			break;

		case hl_TorsoFront: // Split in half?
			if (SpreaderThrowTorso(self, (float)damage, 0.3f, 9)) // One in 10 chance of the takeoff even if requirements are met.
				return;
			break;

		case hl_TorsoBack: // Split in half?
			if (SpreaderThrowTorso(self, (float)damage, 0.7f, 3)) // 25% chance of the takeoff even if requirements are met.
				return;
			break;

		case hl_ArmUpperLeft: // Left arm. //FIXME: what about hose?
		case hl_ArmLowerLeft:
			SpreaderThrowArm(self, (float)damage, MESH__LFTARM, dismember_ok); //mxd
			break;

		case hl_ArmUpperRight: // Right arm. //FIXME: what about grenade?
		case hl_ArmLowerRight:
			SpreaderThrowArm(self, (float)damage, MESH__RITARM, dismember_ok); //mxd
			break;

		case hl_LegUpperLeft: // Left leg.
		case hl_LegLowerLeft:
			SpreaderThrowLeg(self, MESH__LFTLEG); //mxd
			break;

		case hl_LegUpperRight: // Right leg.
		case hl_LegLowerRight:
			SpreaderThrowLeg(self, MESH__RITLEG); //mxd
			break;

		default:
			if (flrand(0, (float)self->health) < (float)damage * 0.25f)
				SpreaderDropWeapon(self);
			break;
	}

	if ((self->s.fmnodeinfo[MESH__LFTARM].flags & FMNI_NO_DRAW) && (self->s.fmnodeinfo[MESH__RITARM].flags & FMNI_NO_DRAW))
	{
		self->monsterinfo.aiflags |= AI_COWARD;
		return;
	}

	if (self->s.fmnodeinfo[MESH__LFTARM].flags & FMNI_NO_DRAW)
		self->monsterinfo.aiflags |= AI_NO_MELEE;

	if (self->s.fmnodeinfo[MESH__RITARM].flags & FMNI_NO_DRAW)
		self->monsterinfo.aiflags |= AI_NO_MISSILE;
}

void SpreaderStopWhenBlocked(edict_t* self, trace_t* trace) //mxd. Named 'spreader_stop' in original logic.
{
	// Apparently being on ground no longer causes you to lose avelocity so I do it manually.
	VectorClear(self->avelocity);

	self->svflags &= ~SVF_TAKE_NO_IMPACT_DMG;
	self->isBlocked = NULL;
	self->bounced = NULL;
}

void SpreaderIsBlocked(edict_t* self, trace_t* trace) //mxd. Named 'spreader_isblocked' in original logic.
{
	if (trace->surface != NULL && (trace->surface->flags & SURF_SKY))
	{
		self->movetype = PHYSICSTYPE_NOCLIP;
		self->solid = SOLID_NOT;
		self->isBlocked = NULL;
		self->bounced = NULL;

		return;
	}

	edict_t* other = trace->ent;

	if (other->movetype != PHYSICSTYPE_NONE && other->movetype != PHYSICSTYPE_PUSH)
	{
		if (other == self->enemy && self->touch_debounce_time > level.time)
			return;

		self->enemy = other;
		VectorAdd(other->velocity, self->velocity, other->velocity);

		if (other->takedamage != DAMAGE_NO)
			T_Damage(other, self, self, vec3_origin, vec3_origin, vec3_origin, 10, 20, 0, MOD_DIED);

		self->touch_debounce_time = level.time + 0.3f;

		return;
	}

	// When crushed, enter headless mode...
	self->s.fmnodeinfo[MESH__HEAD].flags |= FMNI_NO_DRAW;

	const vec3_t gore_spot = VEC3_INITA(self->s.origin, 0.0f, 0.0f, self->maxs[2] - 8.0f);
	SprayDebris(self, gore_spot, 8, 100);

	self->health = 1;
	T_Damage(self, self, self, vec3_origin, vec3_origin, vec3_origin, 10, 20, 0, MOD_DIED);

	self->isBlocked = SpreaderStopWhenBlocked;
	self->bounced = SpreaderStopWhenBlocked;
	SpreaderDeathMsgHandler(self, NULL);

	self->avelocity[YAW] = 0.0f;
	self->elasticity = 1.3f;
	self->friction = 0.8f;
}

void SpreaderSplatWhenBlocked(edict_t* self, trace_t* trace) //mxd. Named 'spreaderSplat' in original logic.
{
	if (trace->ent != NULL && trace->ent->takedamage != DAMAGE_NO)
	{
		vec3_t dir;
		int damage;

		if (Vec3IsZero(self->velocity))
		{
			VectorCopy(vec3_up, dir);
			damage = 0; //mxd. Not initialized in original logic.
		}
		else
		{
			VectorCopy(self->velocity, dir);
			damage = (int)(VectorNormalize(dir));
		}

		if (damage < 50)
			damage = irand(50, 200);

		T_Damage(trace->ent, self, self, dir, trace->endpos, dir, damage, 0, 0, MOD_DIED);

		if (trace->ent->health > 0 && trace->ent->client != NULL) // Else don't gib? //mxd. classname -> client check.
			P_KnockDownPlayer(&trace->ent->client->playerinfo);
	}

	self->dead_state = DEAD_DEAD;
	self->health = -1000;
	self->mass = 0; //mxd. '0.01' in original logic.
	self->think = BecomeDebris;
	self->nextthink = level.time + FRAMETIME; //mxd. '0.01' in original logic.
}

void SpreaderDropDownThink(edict_t* self) //mxd. Named 'spreaderDropDown' in original logic.
{
	self->movetype = PHYSICSTYPE_NOCLIP;
	self->solid = SOLID_NOT;
	self->velocity[2] = -200.0f;
	VectorRandomSet(self->avelocity, 300.0f);

	SetAnim(self, ANIM_FDIE);

	self->think = M_Think;
	self->nextthink = level.time + FRAMETIME; //mxd. Use define.
}

#pragma endregion

#pragma region ========================== Action functions ==========================

void spreader_show_grenade(edict_t* self) //mxd. Named 'spreader_showgrenade' in original logic.
{
	if (!(self->monsterinfo.aiflags & AI_NO_MISSILE)) //FIXME: actually prevent these anims.
		self->s.fmnodeinfo[MESH__BOMB].flags &= ~FMNI_NO_DRAW;
}

void spreader_hide_grenade(edict_t* self) //mxd. Named 'spreader_hidegrenade' in original logic.
{
	self->s.fmnodeinfo[MESH__BOMB].flags |= FMNI_NO_DRAW;
	gi.sound(self, CHAN_AUTO, sounds[SND_THROW], 1.0f, ATTN_IDLE, 0.0f); //TODO: move to spreader_toss_grenade()?
}

void spreader_pain_sound(edict_t* self)
{
	gi.sound(self, CHAN_WEAPON, sounds[SND_PAIN], 1.0f, ATTN_NORM, 0.0f);
}

void spreader_mist_start_sound(edict_t* self) //mxd. Named 'spreader_miststartsound' in original logic.
{
	if (!(self->monsterinfo.aiflags & AI_NO_MELEE)) //FIXME: actually prevent these anims.
		gi.sound(self, CHAN_WEAPON, sounds[SND_SPRAYSTART], 1.0f, ATTN_IDLE, 0.0f);
}

void spreader_mist_stop_sound(edict_t* self) //mxd. Named 'spreader_miststopsound' in original logic. //TODO: remove?
{
}

void spreader_idle_sound(edict_t* self) //mxd. Named 'spreader_idlenoise' in original logic.
{
	static int sound_delay = 0;

	if (irand(0, 9) < 7 && sound_delay > 0)
		return;

	if (++sound_delay >= 50)
		sound_delay = 0;

	gi.sound(self, CHAN_AUTO, sounds[irand(SND_VOICE1, SND_VOICE2)], 1.0f, ATTN_IDLE, 0.0f);
}

void spreader_flyback_loop(edict_t* self)
{
	SetAnim(self, ANIM_DEATH1_LOOP);
}

void spreader_flyback_move(edict_t* self)
{
	M_ChangeYaw(self);

	trace_t trace;
	const vec3_t end_pos = VEC3_INITA(self->s.origin, 0.0f, 0.0f, -48.0f);
	gi.trace(self->s.origin, self->mins, self->maxs, end_pos, self, MASK_MONSTERSOLID, &trace);

	if (trace.fraction < 1.0f || trace.startsolid || trace.allsolid)
	{
		if (trace.fraction < 1.0f)
		{
			const int fx_flags = (irand(0, 1) == 1 ? CEF_FLAG6 : 0);
			const vec3_t bottom = VEC3_INITA(trace.endpos, flrand(-4.0f, 4.0f), flrand(-4.0f, 4.0f), self->mins[2]);
			gi.CreateEffect(NULL, FX_BLOOD_TRAIL, fx_flags, bottom, "d", trace.plane.normal);
		}

		if (self->curAnimID != ANIM_DEATH1_END && self->curAnimID != ANIM_DEATH1_GO)
		{
			self->elasticity = 1.1f;
			self->friction = 0.5f;
			SetAnim(self, ANIM_DEATH1_END);
		}
	}
}

void spreader_dead(edict_t* self)
{
	if (irand(0, 1) == 1 && self->curAnimID == ANIM_DEATH1_END)
	{
		const vec3_t spray_dir = { flrand(-10.0f, 10.0f), flrand(-10.0f, 10.0f), flrand(10.0f, 100.0f) };
		const vec3_t offset = { 0.0f, 0.0f, -24.0f };

		//vec3_t spray_origin;
		//VectorAdd(self->s.origin, offset, spray_origin);

		// Create the volume effect for the damage.
		edict_t* gas = CreateRadiusDamageEnt(self, self, 1, 0, 30, 1.0f, DAMAGE_NO_BLOOD | DAMAGE_ALIVE_ONLY, 4.5f, 0.2f, NULL, offset, true);

		gas->svflags |= SVF_ALWAYS_SEND;
		gas->s.effects = EF_MARCUS_FLAG1;

		gi.CreateEffect(&gas->s, FX_PLAGUEMIST, CEF_OWNERS_ORIGIN, offset, "vb", spray_dir, 100); //FIXME: pass spray_origin, not offset? //TODO: test this.
	}

	M_EndDeath(self);
}

void spreader_duck_pause(edict_t* self) //mxd. Named 'spreader_duckpause' in original logic.
{
	if (M_ValidTarget(self, self->enemy))
	{
		if (M_DistanceToTarget(self, self->enemy) < 128.0f)
		{
			SetAnim(self, ANIM_DUCKATTACK);
			return;
		}

		if (self->evade_debounce_time > level.time || irand(0, 10) == 0)
		{
			SetAnim(self, ANIM_DUCKSTILL);
			return;
		}
	}

	SetAnim(self, (SpreaderCheckUncrouch(self) ? ANIM_DUCKUP : ANIM_DUCKSTILL));
}

void spreader_pause(edict_t* self)
{
	const qboolean is_fixed = (self->spawnflags & MSF_FIXED); //mxd

	if (is_fixed && self->curAnimID == ANIM_DELAY && self->enemy != NULL)
	{
		self->monsterinfo.searchType = SEARCH_COMMON;
		MG_FaceGoal(self, true);
	}

	self->mood_think(self);

	switch (self->ai_mood)
	{
		case AI_MOOD_ATTACK:
			G_PostMessage(self, MSG_MISSILE, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_NAVIGATE:
		case AI_MOOD_PURSUE:
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
		case AI_MOOD_WALK:
			G_PostMessage(self, MSG_WALK, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_JUMP:
			SetAnim(self, (is_fixed ? ANIM_DELAY : ANIM_FJUMP));
			break;

		case AI_MOOD_BACKUP:
			G_PostMessage(self, MSG_FALLBACK, PRI_DIRECTIVE, NULL);
			break;

		default:
			break;
	}
}

void spreader_deadloop_go(edict_t* self) //mxd. Named 'spreader_go_deadloop' in original logic.
{
	SetAnim(self, ANIM_DEAD);
}

void spreader_become_solid(edict_t* self) //mxd. Named 'spreaderSolidAgain' in original logic.
{
	self->nextthink = level.time + FRAMETIME; //mxd. Use define.
	const vec3_t org = VEC3_INITA(self->s.origin, 0.0f, 0.0f, self->maxs[2]);

	if (gi.pointcontents(org) == CONTENTS_EMPTY)
	{
		if (self->movetype == PHYSICSTYPE_STEP)
			return;

		self->svflags &= ~(SVF_ALWAYS_SEND | SVF_TAKE_NO_IMPACT_DMG);
		self->movetype = PHYSICSTYPE_STEP;
		self->solid = SOLID_BBOX;

		self->isBlocked = SpreaderSplatWhenBlocked;
		self->bounced = SpreaderSplatWhenBlocked;
	}
	else
	{
		if (self->velocity[2] > -600.0f)
		{
			if (VectorLength(self->movedir) > 500.0f)
				VectorCopy(self->movedir, self->velocity);
			else
				self->velocity[2] -= 50.0f;
		}
		else
		{
			VectorCopy(self->velocity, self->movedir);
		}
	}
}

void spreader_fly(edict_t* self) //mxd. Named 'spreaderFly' in original logic.
{
	if (self->spreader_spray_sound_time <= level.time)
	{
		gi.sound(self, CHAN_BODY, sounds[SND_SPRAYLOOP], 1.0f, ATTN_NORM, 0.0f);
		self->spreader_spray_sound_time = level.time + flrand(0.4f, 0.8f);
	}

	if (self->velocity[2] < 800.0f)
		self->velocity[2] += 100.0f;

	self->velocity[2] = min(800.0f, self->velocity[2]);

	vec3_t forward;
	AngleVectors(self->s.angles, forward, NULL, NULL);
	VectorMA(self->s.origin, -12.0f, forward, self->spreader_mist_origin);
	self->spreader_mist_origin[2] += self->maxs[2] * 0.8f;

	vec3_t spray_dir = { flrand(-100.0f, 100.0f), flrand(-100.0f, 100.0f), -self->velocity[2] };
	gi.CreateEffect(NULL, FX_PLAGUEMIST, 0, self->spreader_mist_origin, "vb", spray_dir, 41);

	if (self->s.origin[2] > 3900.0f) //TODO: either add a delay, or track traveled distance instead?..
	{
		self->movetype = PHYSICSTYPE_NONE;
		VectorClear(self->velocity);

		self->think = SpreaderDropDownThink;
		self->nextthink = level.time + flrand(1.5f, 3.0f);
	}
	else if (self->health <= 0)
	{
		VectorClear(self->avelocity);
	}
}

void spreader_fly_loop(edict_t* self) //mxd. Named 'spreaderFlyLoop' in original logic.
{
	SetAnim(self, ANIM_FLYLOOP);
}

void spreader_land(edict_t* self)
{
	gi.sound(self, CHAN_BODY, gi.soundindex("misc/land.wav"), 1.0f, ATTN_NORM, 0.0f);
	gi.CreateEffect(&self->s, FX_DUST_PUFF, CEF_OWNERS_ORIGIN, self->s.origin, NULL);
}

void spreader_jump(edict_t* self) //mxd. Named 'spreaderApplyJump' in original logic.
{
	self->jump_time = level.time + 0.5f;
	VectorCopy(self->movedir, self->velocity);
	VectorNormalize(self->movedir);
}

void spreader_inair_go(edict_t* self) //mxd. Named 'spreader_go_inair' in original logic.
{
	SetAnim(self, ANIM_INAIR);
}

#pragma endregion

void SpreaderStaticsInit(void)
{
	static ClassResourceInfo_t res_info; //mxd. Made local static.

	res_info.numAnims = NUM_ANIMS;
	res_info.animations = animations;
	res_info.modelIndex = gi.modelindex("models/monsters/spreader/tris.fm");

	classStatics[CID_SPREADER].msgReceivers[MSG_STAND] = SpreaderStandMsgHandler;
	classStatics[CID_SPREADER].msgReceivers[MSG_RUN] = SpreaderRunMsgHandler;
	classStatics[CID_SPREADER].msgReceivers[MSG_WALK] = SpreaderWalkMsgHandler;
	classStatics[CID_SPREADER].msgReceivers[MSG_MELEE] = SpreaderMeleeMsgHandler;
	classStatics[CID_SPREADER].msgReceivers[MSG_MISSILE] = SpreaderMissileMsgHandler;
	classStatics[CID_SPREADER].msgReceivers[MSG_DISMEMBER] = DismemberMsgHandler;
	classStatics[CID_SPREADER].msgReceivers[MSG_JUMP] = SpreaderJumpMsgHandler;
	classStatics[CID_SPREADER].msgReceivers[MSG_EVADE] = SpreaderEvadeMsgHandler;
	classStatics[CID_SPREADER].msgReceivers[MSG_FALLBACK] = SpreaderFallbackMsgHandler;
	classStatics[CID_SPREADER].msgReceivers[MSG_DEATH] = SpreaderDeathMsgHandler;
	classStatics[CID_SPREADER].msgReceivers[MSG_PAIN] = SpreaderPainMsgHandler;
	classStatics[CID_SPREADER].msgReceivers[MSG_DEATH_PAIN] = SpreaderDeathPainMsgHandler;
	classStatics[CID_SPREADER].msgReceivers[MSG_CHECK_MOOD] = SpreaderCheckMoodMsgHandler;

	sounds[SND_SPRAYSTART] = gi.soundindex("monsters/spreader/spraystart.wav");
	sounds[SND_SPRAYLOOP] = gi.soundindex("monsters/spreader/sprayloop.wav");
	sounds[SND_PAIN] = gi.soundindex("monsters/spreader/pain1.wav");
	sounds[SND_VOICE1] = gi.soundindex("monsters/spreader/voice1.wav");
	sounds[SND_VOICE2] = gi.soundindex("monsters/spreader/voice2.wav");
	sounds[SND_THROW] = gi.soundindex("monsters/spreader/throw.wav");
	sounds[SND_DEATH] = gi.soundindex("monsters/spreader/death.wav");
	sounds[SND_BOMB] = gi.soundindex("monsters/spreader/gasbomb.wav");

	res_info.numSounds = NUM_SOUNDS;
	res_info.sounds = sounds;

	classStatics[CID_SPREADER].resInfo = &res_info;
}

// QUAKED monster_spreader (1 .5 0) (-16 -16 -0) (16 16 32) AMBUSH ASLEEP WALKING 8 16 32 64 FIXED WANDER MELEE_LEAD STALK COWARD EXTRA1 EXTRA2 EXTRA3 EXTRA4
// The spreader.

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
// missile_range			- Maximum distance the player can be from the monster to be allowed to use it's ranged attack (default 512).
// min_missile_range		- Minimum distance the player can be from the monster to be allowed to use it's ranged attack (default 200).
// bypass_missile_chance	- Chance that a monster will NOT fire it's ranged attack, even when it has a clear shot. This, in effect, will make the monster
//							  come in more often than hang back and fire. A percentage (0 = always fire/never close in, 100 = never fire/always close in) - must be whole number (default 50).
// jump_chance				- Every time the monster has the opportunity to jump, what is the chance (out of 100) that he will... (100 = jump every time) - must be whole number (default 30).
// wakeup_distance			- How far (max) the player can be away from the monster before it wakes up. This means that if the monster can see the player,
//							  at what distance should the monster actually notice him and go for him (default 1024).
// NOTE: A value of zero will result in defaults, if you actually want zero as the value, use -1.
void SP_monster_spreader(edict_t* self)
{
	if (self->spawnflags & MSF_WALKING)
	{
		self->spawnflags |= MSF_WANDER;
		self->spawnflags &= ~MSF_WALKING;
	}

	if (!M_WalkmonsterStart(self)) //mxd. M_Start -> M_WalkmonsterStart.
		return; // Failed initialization.

	self->msgHandler = DefaultMsgHandler;
	self->monsterinfo.dismember = SpreaderDismember;

	if (self->health == 0)
		self->health = SPREADER_HEALTH;

	self->health = MonsterHealth(self->health);
	self->max_health = self->health;

	self->mass = SPREADER_MASS;
	self->materialtype = MAT_FLESH;
	self->yaw_speed = 20.0f;

	self->s.origin[2] += 40.0f; //TODO: known to break stuff. Adjust in SpawnEntities() instead (like monster_plagueElf)?..
	self->movetype = PHYSICSTYPE_STEP;

	self->solid = SOLID_BBOX;

	VectorCopy(STDMinsForClass[self->classID], self->mins);
	VectorCopy(STDMaxsForClass[self->classID], self->maxs);
	self->viewheight = 36; //TODO: set to '(int)(self->maxs[2] - 8.0f * self->s.scale)' in SpreaderCheckUncrouch().

	self->s.modelindex = (byte)classStatics[CID_SPREADER].resInfo->modelIndex;

	self->s.fmnodeinfo[MESH__BOMB].flags |= FMNI_NO_DRAW; // Hide the bomb.
	self->s.skinnum = 0;

	self->ai_mood_flags |= AI_MOOD_FLAG_PREDICT;
	self->monsterinfo.otherenemyname = "monster_box"; //TODO: 'monster_box' is not defined anywhere.

	if (self->s.scale == 0.0f) //mxd. 's.scale' not checked/set in original logic.
	{
		self->s.scale = MODEL_SCALE;
		self->monsterinfo.scale = self->s.scale;
	}

	MG_InitMoods(self);
	self->min_melee_range = 24.0f;

	//FIXME: what else should he spawn doing?
	const G_MsgID_t msg_id = ((self->spawnflags & MSF_WANDER) ? MSG_WALK : MSG_STAND); //mxd
	G_PostMessage(self, msg_id, PRI_DIRECTIVE, NULL);
}