//
// m_priestess.c
//
// Copyright 1998 Raven Software
//

#include <float.h> //mxd
#include "m_priestess.h"
#include "m_priestess_shared.h"
#include "m_priestess_anim.h"
#include "g_DefaultMessageHandler.h"
#include "g_monster.h"
#include "mg_guide.h" //mxd
#include "m_stats.h"
#include "Random.h"
#include "Utilities.h"
#include "Vector.h"
#include "g_local.h"

// Number of frames the priestess is in the air.
#define PRIESTESS_JUMP_FRAMES	10.0f //mxd. Named 'PRIESTESS_JUMPFRAMES' in original logic.
#define PRIESTESS_HOP_DISTANCE	0.0f //mxd. Named 'PRIESTESS_HOPDIST' in original logic.

#pragma region ========================== High Priestess Base Info ==========================

static const animmove_t* animations[NUM_ANIMS] =
{
	&priestess_move_stand1,
	&priestess_move_attack1_go,
	&priestess_move_attack1_loop,
	&priestess_move_attack1_end,
	&priestess_move_attack2,
	&priestess_move_backup,
	&priestess_move_death,
	&priestess_move_idle,
	&priestess_move_jump,
	&priestess_move_pain,
	&priestess_move_idle_pose,
	&priestess_move_pose_trans,
	&priestess_move_shield_go,
	&priestess_move_shield_end,
	&priestess_move_dodge_left,
	&priestess_move_dodge_right,
	&priestess_move_walk,
	&priestess_move_jump_forward,
	&priestess_move_jump_back,
	&priestess_move_jump_right,
	&priestess_move_jump_left,
	&priestess_move_jump_pounce,
	&priestess_move_pounce_attack,
	&priestess_move_attack3_go,
	&priestess_move_attack3_loop,
	&priestess_move_attack3_end,
	&priestess_move_jump_attack
};

static int sounds[NUM_SOUNDS];

enum HighPriestessAttackStates_e
{
	AS_QUEENS_FURY,
	AS_BROODS_SACRIFICE,
	AS_HEAVENS_RAIN,
	AS_LIGHT_MISSILE,
	AS_POUNCE,
	AS_JUMP_RIGHT,
	AS_JUMP_LEFT,
};

#pragma endregion

#pragma region ========================== Utility functions =========================

// Create the guts of the high priestess projectile.
static void PriestessProjectileInit(const edict_t* self, edict_t* proj) //mxd. Named 'create_priestess_proj' in original logic.
{
	proj->svflags |= SVF_ALWAYS_SEND;
	proj->movetype = PHYSICSTYPE_FLY;
	proj->gravity = 0.0f;
	proj->solid = SOLID_BBOX;
	proj->classname = "HPriestess_Missile";
	proj->dmg = 1; //TODO: not needed?
	proj->s.scale = 1.0f;
	proj->clipmask = MASK_SHOT;
	proj->nextthink = level.time + FRAMETIME; //mxd. Use define.

	proj->bounced = PriestessProjectile1Blocked;
	proj->isBlocking = PriestessProjectile1Blocked;
	proj->isBlocked = PriestessProjectile1Blocked;

	proj->s.effects = (EF_MARCUS_FLAG1 | EF_CAMERA_NO_CLIP);
	proj->enemy = self->enemy;

	VectorSet(proj->mins, -2.0f, -2.0f, -2.0f);
	VectorSet(proj->maxs, 2.0f, 2.0f, 2.0f);
	VectorCopy(self->s.origin, proj->s.origin);
}

// Tracking, anime style missiles.
static void PriestessFire2(edict_t* self) //mxd. Named 'priestess_fire2' in original logic.
{
	// Spawn the projectile.
	edict_t* proj = G_Spawn();

	PriestessProjectileInit(self, proj);

	proj->monsterinfo.attack_state = AS_QUEENS_FURY;
	proj->monsterinfo.attack_finished = level.time + 2.0f;
	proj->owner = self;

	vec3_t forward;
	vec3_t right;
	AngleVectors(self->s.angles, forward, right, NULL);

	VectorCopy(self->s.origin, proj->s.origin);

	VectorMA(self->s.origin, 30.0f, forward, proj->s.origin);
	VectorMA(proj->s.origin, 8.0f, right, proj->s.origin);
	proj->s.origin[2] += 56.0f;

	proj->ideal_yaw = 400.0f;

	VectorScale(forward, proj->ideal_yaw, proj->velocity);
	vectoangles(proj->velocity, proj->s.angles);

	if (irand(0, 15) == 0) // 6.25% chance to fire drunken missile.
		proj->think = PriestessProjectile1DrunkenThink;
	else
		proj->think = PriestessProjectile1Think;

	gi.sound(self, CHAN_AUTO, sounds[SND_HOMINGATK], 1.0f, ATTN_NORM, 0.0f);
	gi.CreateEffect(&proj->s, FX_HP_MISSILE, CEF_OWNERS_ORIGIN, proj->s.origin, "vb", proj->s.origin, HPMISSILE1);
	gi.linkentity(proj);
}

// The light bugs.
static void PriestessFire3(edict_t* self) //mxd. Named 'priestess_fire3' in original logic.
{
	// Spawn the projectile.
	edict_t* proj = G_Spawn();

	PriestessProjectileInit(self, proj);

	proj->takedamage = DAMAGE_YES;
	proj->monsterinfo.attack_state = AS_BROODS_SACRIFICE;
	proj->monsterinfo.attack_finished = level.time + 5.0f;
	proj->owner = self;

	vec3_t forward;
	vec3_t right;
	AngleVectors(self->s.angles, forward, right, NULL);
	VectorNormalize(forward);

	VectorCopy(self->s.origin, proj->s.origin);

	VectorMA(self->s.origin, 30.0f, forward, proj->s.origin);
	VectorMA(proj->s.origin, 8.0f, right, proj->s.origin);
	proj->s.origin[2] += 56.0f;

	const float dist = M_DistanceToTarget(self, self->enemy) / 200.0f;

	proj->ideal_yaw = flrand(dist * 500.0f, dist * 750.0f); //mxd. irand() in original logic.
	proj->missile_range = flrand(0.65f, 0.75f);

	vec3_t dir;
	VectorSubtract(self->enemy->s.origin, proj->s.origin, dir);
	VectorNormalize(dir);

	vec3_t angles;
	vectoangles(dir, angles);

	angles[PITCH] *= -1.0f;
	angles[PITCH] += flrand(-10.0f, 5.0f); //mxd. irand() in original logic.
	angles[YAW] += flrand(-35.0f, 35.0f); //mxd. irand() in original logic.

	AngleVectors(angles, forward, NULL, NULL);
	VectorScale(forward, proj->ideal_yaw, proj->velocity);
	vectoangles(proj->velocity, proj->s.angles);

	proj->die = PriestessProjectile2Die;
	proj->think = PriestessProjectile2Think;

	gi.sound(self, CHAN_AUTO, sounds[SND_BUGS], 1.0f, ATTN_NORM, 0.0f);
	gi.CreateEffect(&proj->s, FX_HP_MISSILE, CEF_OWNERS_ORIGIN, proj->s.origin, "vb", proj->velocity, HPMISSILE3);
	gi.linkentity(proj);
}

// Big special light show of doom and chaos and destruction... or something...
static void PriestessFire4(edict_t* self) //mxd. Named 'priestess_fire4' in original logic.
{
	if (self->monsterinfo.sound_finished < level.time)
	{
		gi.sound(self, CHAN_AUTO, sounds[SND_ZAP], 1.0f, ATTN_NORM, 0.0f);
		self->monsterinfo.sound_finished = level.time + 5.0f;
	}

	vec3_t forward;
	vec3_t right;
	AngleVectors(self->s.angles, forward, right, NULL);

	vec3_t start_pos;
	VectorCopy(self->s.origin, start_pos);

	VectorMA(self->s.origin, 30.0f, forward, start_pos);
	VectorMA(start_pos, 8.0f, right, start_pos);
	start_pos[2] += 56.0f;

	// The 5 to 8 effects are spawn on the other side, no reason to send each one.
	gi.CreateEffect(NULL, FX_HP_MISSILE, 0, start_pos, "vb", start_pos, HPMISSILE4);

	if (self->monsterinfo.misc_debounce_time < level.time)
	{
		vec3_t dir;
		VectorSubtract(self->enemy->s.origin, start_pos, dir);
		const float dist = VectorNormalize(dir);

		vec3_t end_pos;
		VectorMA(start_pos, dist, dir, end_pos);

		const vec3_t mins = { -1.0f, -1.0f, -1.0f };
		const vec3_t maxs = { 1.0f,  1.0f,  1.0f };

		trace_t trace;
		gi.trace(start_pos, mins, maxs, end_pos, self, MASK_SHOT, &trace);

		if (trace.ent == self->enemy)
		{
			const int damage = irand(HP_DMG_FIRE_MIN, HP_DMG_FIRE_MAX);
			T_Damage(trace.ent, self, self, dir, trace.endpos, trace.plane.normal, damage, 0, DAMAGE_DISMEMBER, MOD_DIED);

			gi.sound(self, CHAN_AUTO, sounds[SND_ZAPHIT], 1.0f, ATTN_NORM, 0.0f);
		}

		gi.CreateEffect(NULL, FX_HP_MISSILE, 0, start_pos, "vb", trace.endpos, HPMISSILE5);
		self->monsterinfo.misc_debounce_time = level.time + flrand(0.2f, 0.4f);
	}
}

#pragma endregion

#pragma region ========================== Message handlers ==========================

static void PriestessDeathMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'priestess_death' in original logic.
{
	self->msgHandler = DeadMsgHandler;

	if (self->dead_state == DEAD_DEAD)
		return;

	self->dead_state = DEAD_DEAD;
	self->takedamage = DAMAGE_NO;

	self->priestess_healthbar_buildup = 0;
	self->health = 0;
	self->max_health = 0;

	M_ShowLifeMeter(0, 0);
	SetAnim(self, ANIM_DEATH);
}

static void PriestessEvadeMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'priestess_evade' in original logic.
{
	SetAnim(self, irand(ANIM_DODGE_LEFT, ANIM_DODGE_RIGHT));
}

static void PriestessStandMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'priestess_stand' in original logic.
{
	SetAnim(self, ANIM_STAND1);
}

static void PriestessMissileMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'priestess_missile' in original logic.
{
	SetAnim(self, ANIM_ATTACK2);
}

static void PriestessRunMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'priestess_run' in original logic.
{
	SetAnim(self, ANIM_WALK);
}

static void PriestessPainMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'priestess_pain' in original logic.
{
	int	temp;
	int damage;
	int force_pain;
	ParseMsgParms(msg, "eeiii", &temp, &temp, &force_pain, &damage, &temp);

	if (self->curAnimID == ANIM_ATTACK3_GO || self->curAnimID == ANIM_ATTACK3_LOOP || self->curAnimID == ANIM_SHIELD_GO)
		return;

	// Weighted random based on health compared to the maximum it was at.
	if (force_pain || ((irand(0, self->max_health + 50) > self->health) && irand(0, 2) == 0))
	{
		gi.sound(self, CHAN_AUTO, sounds[irand(SND_PAIN1, SND_PAIN2)], 1.0f, ATTN_NORM, 0.0f);
		SetAnim(self, ANIM_PAIN);
	}
}

#pragma endregion

#pragma region ========================== Edict callbacks ===========================

static void PriestessProjectile1DrunkenThink(edict_t* self) //mxd. Named 'priestess_proj1_drunken' in original logic.
{
	VectorRandomCopy(self->velocity, self->velocity, 64.0f);
	self->nextthink = level.time + FRAMETIME; //mxd. Use define.
}

static void PriestessProjectile1Think(edict_t* self) //mxd. Named 'priestess_proj1_think' in original logic.
{
#define PROJ1_VELOCITY_MULTIPLIER	1.2f //mxd

	// No enemy / enemy is dead, stop tracking.
	if (self->enemy == NULL || self->enemy->health <= 0)
	{
		self->think = NULL;
		return;
	}

	// Timeout?
	if (self->monsterinfo.attack_finished < level.time)
	{
		gi.sound(self, CHAN_BODY, sounds[SND_BALLHIT], 1.0f, ATTN_NORM, 0.0f);
		gi.CreateEffect(&self->s, FX_HP_MISSILE, CEF_OWNERS_ORIGIN, self->s.origin, "vb", vec3_origin, HPMISSILE1_EXPLODE);

		self->think = G_FreeEdict;
		self->nextthink = level.time + FRAMETIME; //mxd. Use define.

		return;
	}

	vec3_t old_dir;
	VectorNormalize2(self->velocity, old_dir);
	Vec3ScaleAssign(PROJ1_VELOCITY_MULTIPLIER, old_dir);

	vec3_t hunt_dir;
	VectorSubtract(self->enemy->s.origin, self->s.origin, hunt_dir);
	VectorNormalize(hunt_dir);

	vec3_t new_dir;
	VectorAdd(old_dir, hunt_dir, new_dir);

	float new_vel_div = 1.0f / (PROJ1_VELOCITY_MULTIPLIER + 1.0f);
	Vec3ScaleAssign(new_vel_div, new_dir);

	float speed_mod = DotProduct(old_dir, new_dir);
	speed_mod = max(0.05f, speed_mod);

	new_vel_div *= self->ideal_yaw * speed_mod;

	Vec3ScaleAssign(PROJ1_VELOCITY_MULTIPLIER, old_dir);

	VectorAdd(old_dir, hunt_dir, new_dir);
	Vec3ScaleAssign(new_vel_div, new_dir);

	VectorCopy(new_dir, self->velocity);
	self->nextthink = level.time + FRAMETIME; //mxd. Use define.
}

static void PriestessProjectile1Blocked(edict_t* self, trace_t* trace) //mxd. Named 'priestess_proj1_blocked' in original logic.
{
	if (trace->ent == self->owner || Q_stricmp(trace->ent->classname, "HPriestess_Missile") == 0) //mxd. stricmp -> Q_stricmp.
		return;

	byte fx_type;

	// Reflection stuff.
	if (EntReflecting(trace->ent, true, true))
	{
		edict_t* proj = G_Spawn();

		PriestessProjectileInit(self, proj);
		proj->owner = self->owner;
		proj->ideal_yaw = self->ideal_yaw;

		Create_rand_relect_vect(self->velocity, proj->velocity);
		Vec3ScaleAssign(proj->ideal_yaw, proj->velocity);
		vectoangles(proj->velocity, proj->s.angles);

		switch (self->monsterinfo.attack_state)
		{
			case AS_QUEENS_FURY:
			case AS_LIGHT_MISSILE:
				fx_type = HPMISSILE1_EXPLODE;
				break;

			case AS_BROODS_SACRIFICE:
				fx_type = HPMISSILE3_EXPLODE;
				break;

			case AS_HEAVENS_RAIN:
				fx_type = HPMISSILE1_EXPLODE; //TODO: same as case 1. Use HPMISSILE4_EXPLODE or HPMISSILE5_EXPLODE instead?..
				break;

			default: //mxd
				assert(0);
				return;
		}

		gi.CreateEffect(&self->s, FX_HP_MISSILE, CEF_OWNERS_ORIGIN, self->s.origin, "vb", vec3_origin, fx_type);
		gi.linkentity(proj);
		G_SetToFree(self);

		return;
	}

	int damage;

	// Do the rest of the stuff.
	switch (self->monsterinfo.attack_state)
	{
		case AS_QUEENS_FURY:
			fx_type = HPMISSILE1_EXPLODE;
			damage = irand(HP_DMG_FURY_MIN, HP_DMG_FURY_MAX);
			gi.sound(self, CHAN_AUTO, sounds[SND_HOMINGHIT], 1.0f, ATTN_NORM, 0.0f);
			break;

		case AS_BROODS_SACRIFICE:
			fx_type = HPMISSILE3_EXPLODE;
			damage = irand(HP_DMG_BROOD_MIN, HP_DMG_BROOD_MAX);
			gi.sound(self, CHAN_AUTO, sounds[SND_BUGHIT], 1.0f, ATTN_NORM, 0.0f);
			break;

		case AS_HEAVENS_RAIN:
			fx_type = HPMISSILE2_EXPLODE;
			damage = HP_DMG_RAIN;
			gi.sound(self, CHAN_AUTO, sounds[SND_ZAPHIT], 1.0f, ATTN_NORM, 0.0f);
			break;

		case AS_LIGHT_MISSILE:
			fx_type = HPMISSILE1_EXPLODE;
			damage = irand(HP_DMG_MISSILE_MIN, HP_DMG_MISSILE_MAX);
			gi.sound(self, CHAN_AUTO, sounds[SND_BALLHIT], 1.0f, ATTN_NORM, 0.0f);
			break;

		default:
			assert(0);
			return; //mxd. break -> return.
	}

	if (trace->ent->takedamage != DAMAGE_NO)
	{
		vec3_t hit_dir;
		VectorNormalize2(self->velocity, hit_dir);

		T_Damage(trace->ent, self, self->owner, hit_dir, self->s.origin, trace->plane.normal, damage, 0, DAMAGE_SPELL | DAMAGE_NO_KNOCKBACK, MOD_DIED);
	}

	gi.CreateEffect(&self->s, FX_HP_MISSILE, CEF_OWNERS_ORIGIN, self->s.origin, "vb", vec3_origin, fx_type);

	self->think = G_FreeEdict;
	self->nextthink = level.time + FRAMETIME; //mxd. Use define.
}

static void PriestessProjectile2Die(edict_t* self, edict_t* inflictor, edict_t* attacker, int damage, const vec3_t point) //mxd. Named 'priestess_proj2_die' in original logic.
{
	gi.sound(self, CHAN_AUTO, sounds[SND_BUGHIT], 1.0f, ATTN_NORM, 0.0f);
	gi.CreateEffect(&self->s, FX_HP_MISSILE, CEF_OWNERS_ORIGIN, self->s.origin, "vb", vec3_origin, HPMISSILE3_EXPLODE);

	self->think = G_FreeEdict;
	self->nextthink = level.time + FRAMETIME; //mxd. Use define.
}

static void PriestessProjectile2Think(edict_t* self) //mxd. Named 'priestess_proj2_think' in original logic.
{
	// Timeout?
	if (self->monsterinfo.attack_finished < level.time)
	{
		gi.sound(self, CHAN_AUTO, sounds[SND_BUGHIT], 1.0f, ATTN_NORM, 0.0f);
		gi.CreateEffect(&self->s, FX_HP_MISSILE, CEF_OWNERS_ORIGIN, self->s.origin, "vb", vec3_origin, HPMISSILE3_EXPLODE);

		self->think = G_FreeEdict;
	}
	else
	{
		Vec3ScaleAssign(self->missile_range, self->velocity);
		VectorRandomCopy(self->velocity, self->velocity, 8.0f);
	}

	self->nextthink = level.time + FRAMETIME; //mxd. Use define.
}

static void PriestessPostThink(edict_t* self) //mxd. Named 'priestess_postthink' in original logic.
{
	// Only display a lifemeter if we have an enemy.
	if (self->enemy != NULL)
	{
		if (self->priestess_healthbar_buildup < self->max_health)
		{
			M_ShowLifeMeter(self->priestess_healthbar_buildup, self->priestess_healthbar_buildup);
			self->priestess_healthbar_buildup += 50;
		}
		else
		{
			M_ShowLifeMeter(self->health, self->max_health);
		}
	}

	self->next_post_think = level.time + 0.05f;
}

#pragma endregion

#pragma region ========================== Action functions ==========================

void priestess_teleport_go(edict_t* self)
{
	self->takedamage = DAMAGE_NO;
	gi.sound(self, CHAN_AUTO, sounds[SND_TPORT_OUT], 1.0f, ATTN_NORM, 0.0f);
	gi.CreateEffect(NULL, FX_HP_MISSILE, 0, self->s.origin, "vb", self->s.origin, HPTELEPORT_START);
}

void priestess_teleport_end(edict_t* self)
{
	gi.sound(self, CHAN_AUTO, sounds[SND_TPORT_IN], 1.0f, ATTN_NORM, 0.0f);
	gi.CreateEffect(NULL, FX_HP_MISSILE, 0, self->s.origin, "vb", self->s.origin, HPTELEPORT_END);
}

void priestess_teleport_move(edict_t* self)
{
	const vec3_t mins = { -24.0f, -24.0f, -36.0f };
	const vec3_t maxs = { 24.0f, 24.0f, 36.0f }; //BUGFIX: mxd. Same as mins in original logic.

	float best_dist = FLT_MAX; //mxd. 9999999 in original logic.

	edict_t* path_corner = NULL;
	const edict_t* best_corner = NULL;
	while ((path_corner = G_Find(path_corner, FOFS(classname), "path_corner")) != NULL)
	{
		if (Q_stricmp(path_corner->targetname, "priestess") != 0) //mxd. stricmp -> Q_stricmp.
			continue;

		const float enemy_dist = vhlen(self->enemy->s.origin, path_corner->s.origin);
		const float start_dist = vhlen(self->s.origin, path_corner->s.origin);

		if (enemy_dist < 64.0f || start_dist < 64.0f || enemy_dist >= best_dist || !AI_IsVisible(path_corner, self->enemy))
			continue;

		vec3_t test_pos;
		VectorCopy(path_corner->s.origin, test_pos);
		test_pos[2] += maxs[2];

		trace_t	trace;
		gi.trace(test_pos, mins, maxs, test_pos, self, MASK_MONSTERSOLID, &trace);

		if (trace.startsolid || trace.allsolid)
			continue;

		if (trace.ent != NULL && Q_stricmp(trace.ent->classname, "player") == 0) //mxd. stricmp -> Q_stricmp.
			continue;

		best_dist = enemy_dist;
		best_corner = path_corner;
	}

	if (best_corner != NULL)
	{
		// ULTRA HACK!
		VectorCopy(best_corner->s.origin, self->monsterinfo.nav_goal);
		self->s.origin[0] += 2000.0f; //TODO: is there better way to hide her?..
		gi.linkentity(self);

		// Spawn a fake entity to sit where the priestess will teleport to assure there's no telefragging.
		edict_t* blocker = G_Spawn();

		VectorCopy(mins, blocker->mins);
		VectorCopy(maxs, blocker->maxs);

		blocker->solid = SOLID_BBOX;
		blocker->movetype = PHYSICSTYPE_NONE;

		//TODO: if the player touches this entity somehow, he's thrown back.
		self->priestess_teleport_blocker = blocker;

		gi.linkentity(blocker);
	}
	else
	{
		SetAnim(self, ANIM_SHIELD_END);
	}
}

void priestess_teleport_self_effects(edict_t* self)
{
	self->s.renderfx |= RF_ALPHA_TEXTURE;
	self->s.color.c = 0xffffffff;
}

void priestess_delta_alpha(edict_t* self, float amount)
{
	const int alpha = self->s.color.a + (int)amount;
	self->s.color.a = (byte)ClampI(alpha, 0, 255);
}

void priestess_stop_alpha(edict_t* self)
{
	self->takedamage = DAMAGE_YES;
	self->s.renderfx &= ~RF_ALPHA_TEXTURE;
	self->s.color.c = 0xffffffff;
}

void priestess_teleport_return(edict_t* self)
{
	if (self->priestess_teleport_blocker != NULL && self->movetarget != self->enemy) // Free the teleport blocker entity.
		G_FreeEdict(self->priestess_teleport_blocker);

	vec3_t start;
	VectorCopy(self->monsterinfo.nav_goal, start);
	start[2] += 36.0f;

	vec3_t end;
	VectorCopy(self->monsterinfo.nav_goal, end);
	end[2] -= 128.0f;

	trace_t trace;
	gi.trace(start, self->mins, self->maxs, end, self, MASK_MONSTERSOLID, &trace);

	if (trace.allsolid || trace.startsolid)
	{
		// The priestess has become lodged in something!
		assert(0); //TODO: handle this... somehow. Try picking different path corner?
		return;
	}

	VectorCopy(trace.endpos, self->s.origin);
	gi.linkentity(self);

	SetAnim(self, ANIM_SHIELD_END);
}

// Hand thrown light missiles.
void priestess_fire1(edict_t* self, float pitch_offset, float yaw_offset, float roll_offset)
{
	if (self->enemy == NULL)
		return;

	// Only predict once for all the missiles.
	vec3_t predicted_pos;
	M_PredictTargetPosition(self->enemy, self->enemy->velocity, 1.0f, predicted_pos);

	vec3_t forward;
	vec3_t right;
	AngleVectors(self->s.angles, forward, right, NULL);

	vec3_t proj_pos;
	VectorMA(self->s.origin, -8.0f, forward, proj_pos);
	VectorMA(proj_pos, -16.0f, right, proj_pos);
	proj_pos[2] += 32.0f;

	vec3_t diff;
	VectorSubtract(predicted_pos, proj_pos, diff);
	VectorNormalize(diff);

	vec3_t start_angles;
	vectoangles(diff, start_angles);

	for (int i = 0; i < irand(2, 3); i++)
	{
		// Spawn the projectile.
		edict_t* proj = G_Spawn();

		PriestessProjectileInit(self, proj);

		proj->monsterinfo.attack_state = AS_LIGHT_MISSILE;
		proj->owner = self;

		VectorCopy(proj_pos, proj->s.origin);

		vec3_t angles;
		VectorCopy(start_angles, angles);

		angles[PITCH] += flrand(-4.0f, 4.0f);
		angles[YAW] += flrand(-4.0f, 4.0f);

		vec3_t vel;
		AngleVectors(angles, vel, NULL, NULL);
		VectorScale(vel, flrand(500.0f, 600.0f), proj->velocity); //mxd. irand() in original logic.

		vectoangles(proj->velocity, proj->s.angles);

		// One in ten wander off drunkenly.
		if (irand(0, 9) == 0) //mxd. irand(0, 10) in original logic (which is one in eleven).
			proj->think = PriestessProjectile1DrunkenThink;

		gi.sound(self, CHAN_AUTO, sounds[SND_3BALLATK], 1.0f, ATTN_NORM, 0.0f);
		gi.CreateEffect(&proj->s, FX_HP_MISSILE, CEF_OWNERS_ORIGIN, NULL, "vb", proj->velocity, HPMISSILE2);
		gi.linkentity(proj);
	}
}

void priestess_attack1_pause(edict_t* self)
{
	self->monsterinfo.priestess_attack_delay -= 1.0f;

	if (self->monsterinfo.priestess_attack_delay <= 0.0f)
		priestess_pause(self);
}

void priestess_attack3_loop(edict_t* self)
{
	SetAnim(self, ANIM_ATTACK3_LOOP);
	self->monsterinfo.attack_finished = level.time + 4.0f;

	vec3_t forward;
	vec3_t right;
	AngleVectors(self->s.angles, forward, right, NULL);

	vec3_t spawn_pos;
	VectorCopy(self->s.origin, spawn_pos);

	VectorMA(self->s.origin, 30.0f, forward, spawn_pos);
	VectorMA(spawn_pos, 8.0f, right, spawn_pos);
	spawn_pos[2] += 56.0f;

	// RIGHT HERE!
	self->monsterinfo.jump_time = level.time + 10.0f;

	// Don't repeat an attack (people want to see them all!).
	const int prev_attack_state = self->monsterinfo.attack_state;
	
	do
	{
		self->monsterinfo.attack_state = irand(AS_QUEENS_FURY, AS_HEAVENS_RAIN);
	} while (prev_attack_state == self->monsterinfo.attack_state);

	switch (self->monsterinfo.attack_state)
	{
		case AS_QUEENS_FURY:
			gi.CreateEffect(NULL, FX_HP_MISSILE, 0, spawn_pos, "vb", vec3_origin, HPMISSILE1_LIGHT);
			break;

		case AS_BROODS_SACRIFICE:
			self->monsterinfo.attack_finished = level.time + 2.0f;
			gi.CreateEffect(NULL, FX_HP_MISSILE, 0, spawn_pos, "vb", vec3_origin, HPMISSILE3_LIGHT);
			break;

		case AS_HEAVENS_RAIN:
			gi.CreateEffect(NULL, FX_HP_MISSILE, 0, spawn_pos, "vb", vec3_origin, HPMISSILE4_LIGHT);
			gi.CreateEffect(NULL, FX_LENSFLARE, CEF_FLAG8, spawn_pos, "bbbf", 128, 128, 128, 0.9f);
			break;

		default:
			break;
	}
}

void priestess_attack3_loop_fire(edict_t* self)
{
	if (self->monsterinfo.attack_finished < level.time)
	{
		SetAnim(self, ANIM_ATTACK3_END);
		return;
	}

	//NOTE: These effects are not necessarily called each frame (hence the irands).
	switch (self->monsterinfo.attack_state)
	{
		case AS_QUEENS_FURY:
			if (self->monsterinfo.priestess_attack_delay < level.time)
			{
				PriestessFire2(self);
				self->monsterinfo.priestess_attack_delay = level.time + 0.25f;
			}
			break;

		case AS_BROODS_SACRIFICE:
			if (self->monsterinfo.priestess_attack_delay < level.time)
			{
				PriestessFire3(self);
				self->monsterinfo.priestess_attack_delay = level.time + 0.15f;
			}
			break;

		case AS_HEAVENS_RAIN:
			PriestessFire4(self);
			break;

		default:
			break;
	}
}

void priestess_pounce_attack(edict_t* self)
{
	if (!M_ValidTarget(self, self->enemy))
		return;

	const float dist = M_DistanceToTarget(self, self->enemy);

	if (dist < 64.0f)
		SetAnim(self, ANIM_POUNCE_ATTACK);
	else if (dist < 128.0f)
		SetAnim(self, ANIM_ATTACK2);
	else
		priestess_pause(self);
}

void priestess_jump_attack(edict_t* self)
{
	// Find out where the player will be when we would probably land.
	vec3_t predicted_pos;
	M_PredictTargetPosition(self->enemy, self->enemy->velocity, PRIESTESS_JUMP_FRAMES + 2.0f, predicted_pos);

	// Find the vector to that spot and the length.
	vec3_t jump_vel;
	VectorSubtract(predicted_pos, self->s.origin, jump_vel);
	const float move_dist = VectorNormalize(jump_vel);

	// Velocity is applied per tenth of a frame, so take the distance, divide by the number of frames in the air, and FRAMETIME.
	const float jump_dist = move_dist * PRIESTESS_JUMP_FRAMES * FRAMETIME;

	// Now get the height to keep her in the air long enough to complete this jump.
	const float hop_dist = (PRIESTESS_HOP_DISTANCE + (sv_gravity->value * PRIESTESS_JUMP_FRAMES / 4.0f)) * FRAMETIME;

	// Setup the vector for the jump.
	Vec3ScaleAssign(jump_dist, jump_vel);
	jump_vel[2] = hop_dist;

	// Set the priestess in motion.
	VectorCopy(jump_vel, self->velocity);
}

void priestess_pounce(edict_t* self)
{
	if (self->enemy != NULL)
		priestess_jump_attack(self); //mxd. Reuse existing logic.
}

void priestess_strike(edict_t* self, float damage)
{
	vec3_t start_offset;
	vec3_t end_offset;

	//FIXME: Take out the mults here, done this way to speed up tweaking (sue me).
	switch (self->s.frame)
	{
		case FRAME_attackB8:
			VectorSet(start_offset, 16.0f * 4.0f, -16.0f * 5.0f, 16.0f * 3.0f);
			VectorSet(end_offset, 16.0f * 3.0f, 16.0f * 5.0f, -8.0f);
			break;

		case FRAME_attackB14:
			VectorSet(start_offset, 16.0f * 2.0f, 16.0f * 5.0f, 16.0f * 4.0f);
			VectorSet(end_offset, 16.0f * 5.0f, -16.0f * 5.0f, -16.0f * 2.0f);
			break;

		case FRAME_jumpatt12:
			VectorSet(start_offset, 16.0f * 2.0f, 0.0f, 16.0f * 5.0f);
			VectorSet(end_offset, 16.0f * 5.0f, 4.0f, 4.0f);
			break;
	}

	const vec3_t mins = { -4.0f, -4.0f, -4.0f };
	const vec3_t maxs = {  4.0f,  4.0f,  4.0f };

	trace_t	trace;
	vec3_t direction;
	edict_t* victim = M_CheckMeleeLineHit(self, start_offset, end_offset, mins, maxs, &trace, direction);

	// Did something get hit?
	if (victim == NULL)
	{
		// Play swoosh sound.
		gi.sound(self, CHAN_AUTO, sounds[SND_SWIPEMISS], 1.0f, ATTN_NORM, 0.0f);
		return;
	}

	if (victim == self)
	{
		// Create a spark effect.
		gi.CreateEffect(NULL, FX_SPARKS, CEF_FLAG6, trace.endpos, "d", direction);
		gi.sound(self, CHAN_WEAPON, sounds[SND_SWIPEWALL], 1.0f, ATTN_NORM, 0.0f);

		return;
	}

	// Hurt whatever we were whacking away at.
	vec3_t blood_dir;
	VectorSubtract(start_offset, end_offset, blood_dir);
	VectorNormalize(blood_dir);

	T_Damage(victim, self, self, direction, trace.endpos, blood_dir, (int)damage, (int)damage * 2, DAMAGE_DISMEMBER, MOD_DIED);
	gi.sound(self, CHAN_WEAPON, sounds[SND_SWIPE], 1.0f, ATTN_NORM, 0.0f);
}

void priestess_move(edict_t* self, float vf, float vr, float vu) //TODO: implement (and add to priestess_frames_dodge_right)?..
{
}

void priestess_jump_right(edict_t* self)
{
	vec3_t right;
	AngleVectors(self->s.angles, NULL, right, NULL);
	Vec3ScaleAssign(300.0f, right);
	right[2] = 150.0f;

	VectorCopy(right, self->velocity);
}

void priestess_jump_left(edict_t* self)
{
	vec3_t right;
	AngleVectors(self->s.angles, NULL, right, NULL);
	Vec3ScaleAssign(-300.0f, right);
	right[2] = 150.0f;

	VectorCopy(right, self->velocity);
}

void priestess_jump_forward(edict_t* self)
{
	vec3_t forward;
	AngleVectors(self->s.angles, forward, NULL, NULL);
	Vec3ScaleAssign(300.0f, forward);
	forward[2] = 150.0f;

	VectorCopy(forward, self->velocity);
}

void priestess_jump_back(edict_t* self)
{
	vec3_t forward;
	AngleVectors(self->s.angles, forward, NULL, NULL);
	Vec3ScaleAssign(-300.0f, forward);
	forward[2] = 150.0f;

	VectorCopy(forward, self->velocity);
}

void priestess_pause(edict_t* self)
{
	if (!M_ValidTarget(self, self->enemy))
	{
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
		return;
	}

	if (!AI_IsVisible(self, self->enemy) && irand(0, 100) < 75)
	{
		SetAnim(self, ANIM_SHIELD_GO);
		return;
	}

	const float dist = M_DistanceToTarget(self, self->enemy);
	const int chance = irand(0, 100);

	if (dist < 64.0f)
	{
		if (chance < 20)
			SetAnim(self, ANIM_ATTACK2);
		else if (chance < 40)
			SetAnim(self, ANIM_BACKUP);
		else
			SetAnim(self, ANIM_JUMP_BACK);

		return;
	}

	if (chance < 40 && self->monsterinfo.jump_time < level.time)
	{
		SetAnim(self, ANIM_ATTACK3_GO);
	}
	else if (chance < 40 && self->monsterinfo.attack_state != AS_LIGHT_MISSILE)
	{
		self->monsterinfo.priestess_attack_delay = 2.0f; //TODO: same var is used for different purpose in priestess_attack3_loop_fire(). Use different var?
		self->monsterinfo.attack_state = AS_LIGHT_MISSILE;
		SetAnim(self, ANIM_ATTACK1_GO);
	}
	else if (chance < 80 && self->monsterinfo.attack_state != AS_POUNCE)
	{
		self->monsterinfo.attack_state = AS_POUNCE;
		SetAnim(self, ((dist > 256.0f) ? ANIM_JUMP_POUNCE : ANIM_JUMP_ATTACK));
	}
	else if (chance < 90 && self->monsterinfo.attack_finished < level.time)
	{
		SetAnim(self, ANIM_SHIELD_GO);
		self->monsterinfo.attack_finished = level.time + 5.0f;
	}
	else if (self->monsterinfo.attack_state != AS_JUMP_RIGHT)
	{
		self->monsterinfo.attack_state = AS_JUMP_RIGHT;
		SetAnim(self, ANIM_JUMP_RIGHT);
	}
	else
	{
		self->monsterinfo.attack_state = AS_JUMP_LEFT;
		SetAnim(self, ANIM_JUMP_LEFT);
	}
}

void priestess_dead(edict_t* self)
{
	self->mood_nextthink = -1.0f; // Never mood_think again.
	self->maxs[2] = self->mins[2] + 16.0f;

	if (self->PersistantCFX > 0)
	{
		gi.RemovePersistantEffect(self->PersistantCFX, REMOVE_PRIESTESS);
		self->PersistantCFX = 0;
	}

	gi.RemoveEffects(&self->s, FX_REMOVE_EFFECTS);
	gi.linkentity(self);

	self->think = G_FreeEdict;
	self->nextthink = level.time + FRAMETIME; //mxd. Use define.
}

#pragma endregion

void HighPriestessStaticsInit(void)
{
	static ClassResourceInfo_t res_info; //mxd. Made local static.

	classStatics[CID_HIGHPRIESTESS].msgReceivers[MSG_STAND] = PriestessStandMsgHandler;
	classStatics[CID_HIGHPRIESTESS].msgReceivers[MSG_MISSILE] = PriestessMissileMsgHandler;
	classStatics[CID_HIGHPRIESTESS].msgReceivers[MSG_RUN] = PriestessRunMsgHandler;
	classStatics[CID_HIGHPRIESTESS].msgReceivers[MSG_EVADE] = PriestessEvadeMsgHandler;
	classStatics[CID_HIGHPRIESTESS].msgReceivers[MSG_DEATH] = PriestessDeathMsgHandler;
	classStatics[CID_HIGHPRIESTESS].msgReceivers[MSG_PAIN] = PriestessPainMsgHandler;

	res_info.numAnims = NUM_ANIMS;
	res_info.animations = animations;
	res_info.modelIndex = gi.modelindex("models/monsters/highpriestess/tris.fm");

	sounds[SND_PAIN1] = gi.soundindex("monsters/highpriestess/pain1.wav");
	sounds[SND_PAIN2] = gi.soundindex("monsters/highpriestess/pain2.wav");
	//sounds[SND_FALL] = gi.soundindex("monsters/highpriestess/fall.wav");
	sounds[SND_3BALLATK] = gi.soundindex("monsters/highpriestess/3ballatk.wav");
	sounds[SND_BALLHIT] = gi.soundindex("monsters/highpriestess/ballhit.wav");
	//sounds[SND_WHIRL] = gi.soundindex("weapons/stafftwirl_2.wav");
	sounds[SND_BUGS] = gi.soundindex("monsters/highpriestess/bugs.wav");
	//sounds[SND_BUGBUZZ] = gi.soundindex("monsters/highpriestess/bugbuzz.wav");
	sounds[SND_BUGHIT] = gi.soundindex("monsters/highpriestess/bughit.wav");
	sounds[SND_ZAP] = gi.soundindex("monsters/highpriestess/zap.wav");
	sounds[SND_ZAPHIT] = gi.soundindex("monsters/highpriestess/zaphit.wav");
	sounds[SND_HOMINGATK] = gi.soundindex("monsters/highpriestess/homatk.wav");
	sounds[SND_HOMINGHIT] = gi.soundindex("monsters/highpriestess/homhit.wav");
	sounds[SND_TPORT_IN] = gi.soundindex("monsters/highpriestess/tportin.wav");
	sounds[SND_TPORT_OUT] = gi.soundindex("monsters/highpriestess/tpotout.wav");
	sounds[SND_SWIPE] = gi.soundindex("weapons/staffswing_2.wav");
	//sounds[SND_SWIPEHIT] = gi.soundindex("weapons/staffhit_2.wav");
	sounds[SND_SWIPEMISS] = gi.soundindex("monsters/seraph/guard/attack_miss.wav");
	sounds[SND_SWIPEWALL] = gi.soundindex("weapons/staffhitwall.wav");

	res_info.sounds = sounds;
	res_info.numSounds = NUM_SOUNDS;

	classStatics[CID_HIGHPRIESTESS].resInfo = &res_info;
}

// QUAKED monster_high_priestess (1 .5 0) (-24 -24 0) (24 24 72)
// The High Priestess.
// wakeup_target	- Monsters will fire this target the first time it wakes up (only once).
// pain_target		- Monsters will fire this target the first time it gets hurt (only once).
void SP_monster_high_priestess(edict_t* self)
{
	if (DEATHMATCH && !(SV_CHEATS & self_spawn))
		return;

	if (!M_WalkmonsterStart(self)) // Failed initialization.
		return;

	self->msgHandler = DefaultMsgHandler;

	if (self->health == 0)
		self->health = HP_HEALTH;

	// Apply to the end result (whether designer set or not).
	self->health = MonsterHealth(self->health);
	self->max_health = self->health;

	self->mass = HP_MASS;
	self->yaw_speed = 24.0f;

	self->movetype = PHYSICSTYPE_STEP;
	self->solid = SOLID_BBOX;
	self->clipmask = MASK_MONSTERSOLID;
	self->svflags |= SVF_BOSS;

	self->s.origin[2] += 36.0f;
	VectorSet(self->mins, -24.0f, -24.0f, -36.0f); //TODO: init via STDMinsForClass?
	VectorSet(self->maxs,  24.0f,  24.0f,  36.0f); //TODO: init via STDMaxsForClass?

	self->s.modelindex = (byte)classStatics[CID_HIGHPRIESTESS].resInfo->modelIndex;
	self->s.skinnum = 0;
	self->materialtype = MAT_INSECT;

	self->monsterinfo.jump_time = level.time + 15.0f;
	self->monsterinfo.otherenemyname = "monster_rat";

	if (self->s.scale == 0.0f) //BUGFIX: mxd. 'if (self->monsterinfo.scale)' in original logic.
	{
		self->s.scale = MODEL_SCALE;
		self->monsterinfo.scale = self->s.scale;
	}

	MG_InitMoods(self);

	// Setup her reference points.
	self->PersistantCFX = gi.CreatePersistantEffect(&self->s, FX_HP_STAFF, CEF_OWNERS_ORIGIN | CEF_BROADCAST, vec3_origin, "bs", HP_STAFF_INIT, self->s.number);

	QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);

	self->post_think = PriestessPostThink;
	self->next_post_think = level.time + FRAMETIME; //mxd. Use define.
}