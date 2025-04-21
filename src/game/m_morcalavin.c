//
// m_morcalavin.c
//
// Copyright 1998 Raven Software
//

#include "m_morcalavin.h"
#include "m_morcalavin_shared.h"
#include "m_morcalavin_anim.h"
#include "g_DefaultMessageHandler.h"
#include "g_monster.h"
#include "g_playstats.h"
#include "mg_ai.h" //mxd
#include "mg_guide.h" //mxd
#include "m_stats.h"
#include "p_main.h"
#include "Random.h"
#include "Utilities.h"
#include "Vector.h"
#include "g_local.h"

#define MORCALAVIN_GRAVITY	0.3f //mxd. Named 'MORK_GRAV' in original logic.

static void MorcalavinProjectileInit(edict_t* self, edict_t* proj); //TODO: remove.
static void MorcalavinProjectile1Blocked(edict_t* self, trace_t* trace); //TODO: remove.
static void MorcalavinProjectile3Blocked(edict_t* self, trace_t* trace); //TODO: remove.
static void morcalavin_init_phase_out(edict_t* self); //TODO: remove.
static void morcalavin_attack_fade_out(edict_t* self); //TODO: remove.

#pragma region ========================== Morcalavin base info ==========================

static const animmove_t* animations[NUM_ANIMS] =
{
	&morcalavin_move_float,
	&morcalavin_move_hurtidle,
	&morcalavin_move_attack1,
	&morcalavin_move_attack2,
	&morcalavin_move_attack2b,
	&morcalavin_move_attack3,
	&morcalavin_move_def1,
	&morcalavin_move_def2,
	&morcalavin_move_walk,
	&morcalavin_move_fly,
	&morcalavin_move_getup,
	&morcalavin_move_retort,
	&morcalavin_move_fall,
	&morcalavin_move_glide,
	&morcalavin_move_ground_attack,
	&morcalavin_move_tracking_attack1,
	&morcalavin_move_attack4,
};

static int sounds[NUM_SOUNDS];

#pragma endregion

void morcalavin_end_retort(edict_t* self)
{
	SetAnim(self, ANIM_WALK);
}

static void MorcalavinLightning2Think(edict_t* self) //mxd. Named 'morcalavin_check_lightning2' in original logic.
{
	if (self->owner->enemy == NULL)
	{
		self->nextthink = level.time + FRAMETIME; //mxd. Use define.
		return;
	}

	vec3_t dir;
	VectorSubtract(self->s.origin, self->owner->enemy->s.origin, dir);
	const float dist = VectorNormalize(dir);

	if (dist < 72.0f)
	{
		T_Damage(self->owner->enemy, self, self->owner, dir, vec3_origin, vec3_origin, 2, 0, DAMAGE_SPELL, MOD_SHIELD);

		vec3_t vel;
		VectorNormalize2(self->velocity, vel);

		vec3_t lightning_start;
		VectorMA(self->s.origin, 600.0f * FRAMETIME, vel, lightning_start);

		vec3_t lightning_end;
		VectorCopy(self->owner->enemy->s.origin, lightning_end);
		lightning_end[2] += flrand(-16.0f, 32.0f); //mxd. irand() in original logic.

		gi.CreateEffect(NULL, FX_LIGHTNING, 0, lightning_start, "vbb", lightning_end, irand(2, 4), 0); //TODO: play SND_LGHTNGHIT sound?
	}

	self->nextthink = level.time + FRAMETIME; //mxd. Use define.
}

void morcalavin_big_shot(edict_t* self)
{
	edict_t* proj = G_Spawn();

	VectorSet(proj->mins, -4.0f, -4.0f, -4.0f);
	VectorSet(proj->maxs,  4.0f,  4.0f,  4.0f);

	proj->solid = SOLID_BBOX;
	proj->movetype = PHYSICSTYPE_FLY;
	proj->gravity = 0.0f;
	proj->clipmask = MASK_SHOT;

	proj->owner = self;
	self->target_ent = proj;

	proj->isBlocked = MorcalavinProjectile3Blocked;
	proj->isBlocking = MorcalavinProjectile3Blocked;
	proj->bounced = MorcalavinProjectile3Blocked;

	proj->think = MorcalavinLightning2Think;
	proj->nextthink = level.time + FRAMETIME; //mxd. Use define.

	vec3_t forward;
	vec3_t right;
	AngleVectors(self->s.angles, forward, right, NULL);
	VectorMA(self->s.origin, 24.0f, forward, proj->s.origin);
	VectorMA(proj->s.origin, 12.0f, right, proj->s.origin);
	proj->s.origin[2] += 32.0f;

	vec3_t dir;
	VectorSubtract(self->enemy->s.origin, proj->s.origin, dir);
	VectorNormalize(dir);
	VectorScale(dir, 600.0f, proj->velocity);

	proj->s.effects = (EF_MARCUS_FLAG1 | EF_CAMERA_NO_CLIP);

	gi.linkentity(proj);
	gi.CreateEffect(&proj->s, FX_M_EFFECTS, CEF_OWNERS_ORIGIN, proj->s.origin, "bv", FX_MORK_MISSILE, proj->s.origin);
	gi.sound(self, CHAN_AUTO, sounds[SND_PPCHARGE], 1.0f, ATTN_NORM, 0.0f);
}

static void MorcalavinTrackingProjectileThink(edict_t* self) //mxd. Named 'morcalavin_proj_track' in original logic.
{
	// No enemy or enemy is dead, stop tracking.
	if (self->enemy == NULL || self->enemy->health <= 0)
	{
		self->think = NULL;
		return;
	}

	// Timed out?
	if (self->monsterinfo.attack_finished < level.time)
	{
		gi.CreateEffect(&self->s, FX_HP_MISSILE, CEF_OWNERS_ORIGIN, self->s.origin, "vb", vec3_origin, HPMISSILE1_EXPLODE); //TODO: play SND_HOMEHIT sound?

		self->think = G_FreeEdict;
		self->nextthink = level.time + FRAMETIME; //mxd. Use define.

		return;
	}

	vec3_t old_dir;
	VectorCopy(self->velocity, old_dir);
	VectorNormalize(old_dir);

	vec3_t hunt_dir;
	VectorSubtract(self->enemy->s.origin, self->s.origin, hunt_dir);
	VectorNormalize(hunt_dir);

	if (self->delay + 0.05f < 4.0f)
		self->delay += 0.05f;

	if (self->ideal_yaw + 10.0f < 1000.0f)
		self->ideal_yaw += 10.0f;

	const float old_vel_mult = self->delay;
	float new_vel_div = 1.0f / (old_vel_mult + 1.0f);

	VectorScale(old_dir, old_vel_mult, old_dir);

	vec3_t new_dir;
	VectorAdd(old_dir, hunt_dir, new_dir);
	VectorScale(new_dir, new_vel_div, new_dir);

	float speed_mod = DotProduct(old_dir, new_dir);
	speed_mod = max(0.05f, speed_mod);

	new_vel_div *= self->ideal_yaw * speed_mod;

	VectorScale(old_dir, old_vel_mult, old_dir);
	VectorAdd(old_dir, hunt_dir, new_dir);
	VectorScale(new_dir, new_vel_div, new_dir);

	VectorCopy(new_dir, self->velocity);
	self->nextthink = level.time + FRAMETIME; //mxd. Use define.
}

void morcalavin_tracking_projectile(edict_t* self, float pitch, float yaw, float roll)
{
	edict_t* proj = G_Spawn();

	proj->monsterinfo.attack_finished = level.time + 10.0f;
	proj->svflags |= SVF_ALWAYS_SEND;
	proj->movetype = PHYSICSTYPE_FLY;
	proj->gravity = 0.0f;
	proj->solid = SOLID_BBOX;
	proj->classname = "Morcalavin_Tracking_Missile";
	proj->dmg = 1;
	proj->clipmask = MASK_SHOT;
	proj->delay = 1.0f;
	proj->s.scale = 1.0f;
	proj->s.effects = (EF_MARCUS_FLAG1 | EF_CAMERA_NO_CLIP);

	proj->owner = self;
	proj->enemy = self->enemy;

	VectorSet(proj->mins, -2.0f, -2.0f, -2.0f);
	VectorSet(proj->maxs, 2.0f, 2.0f, 2.0f);
	VectorCopy(self->s.origin, proj->s.origin);

	// Determine the starting velocity of the ball.
	vec3_t angles;
	VectorCopy(self->s.angles, angles);

	angles[PITCH] += pitch;
	angles[YAW] += yaw;
	angles[ROLL] += roll;

	vec3_t forward;
	vec3_t right;
	vec3_t up;
	AngleVectors(angles, forward, right, up);

	VectorMA(self->s.origin, 24.0f, forward, proj->s.origin);
	VectorMA(proj->s.origin, -16.0f, right, proj->s.origin);
	VectorMA(proj->s.origin, 52.0f, up, proj->s.origin);

	VectorScale(up, 16.0f, proj->velocity);
	proj->ideal_yaw = 300.0f;

	proj->bounced = MorcalavinProjectile1Blocked;
	proj->isBlocking = MorcalavinProjectile1Blocked;
	proj->isBlocked = MorcalavinProjectile1Blocked;

	proj->think = MorcalavinTrackingProjectileThink;

	if (SKILL < SKILL_MEDIUM)
		proj->nextthink = level.time + 2.0f;
	else
		proj->nextthink = level.time + flrand(1.5f, 3.0f);

	gi.sound(self, CHAN_AUTO, sounds[SND_HOMING], 1.0f, ATTN_NORM, 0.0f);
	gi.CreateEffect(&proj->s, FX_M_EFFECTS, CEF_OWNERS_ORIGIN, NULL, "bv", FX_MORK_TRACKING_MISSILE, proj->velocity);
}

static void MorcalavinProjectile2Blocked(edict_t* self, trace_t* trace) //mxd. Named 'morcalavin_proj2_blocked' in original logic.
{
	if (trace->ent == self->owner)
		return;

	// Reflection stuff.
	if (EntReflecting(trace->ent, true, true))
	{
		edict_t* proj = G_Spawn();

		MorcalavinProjectileInit(self, proj);
		proj->owner = self->owner;
		proj->ideal_yaw = self->ideal_yaw;

		Create_rand_relect_vect(self->velocity, proj->velocity);
		Vec3ScaleAssign(proj->ideal_yaw, proj->velocity);
		vectoangles(proj->velocity, proj->s.angles);

		gi.CreateEffect(&self->s, FX_HP_MISSILE, CEF_OWNERS_ORIGIN, self->s.origin, "vb", vec3_origin, HPMISSILE1_EXPLODE); //TODO: play SND_HOMEHIT sound?
		gi.linkentity(proj);

		G_SetToFree(self);

		return;
	}

	if (trace->ent->takedamage != DAMAGE_NO)
	{
		vec3_t hit_dir;
		VectorCopy(self->velocity, hit_dir);
		VectorNormalize(hit_dir);

		T_Damage(trace->ent, self, self->owner, hit_dir, self->s.origin, trace->plane.normal, 40, 0, DAMAGE_SPELL | DAMAGE_NO_KNOCKBACK, MOD_DIED);
	}

	gi.CreateEffect(NULL, FX_M_EFFECTS, 0, self->s.origin, "bv", FX_MORK_MISSILE_HIT, trace->plane.normal); //TODO: play SND_HOMEHIT sound?

	self->think = G_FreeEdict;
	self->nextthink = level.time + FRAMETIME; //mxd. Use define.
}

static void MorcalavinProjectile3Blocked(edict_t* self, trace_t* trace) //mxd. Named 'morcalavin_proj3_blocked' in original logic.
{
	if (trace->ent == self->owner)
		return;

	// Reflection stuff.
	if (EntReflecting(trace->ent, true, true))
	{
		edict_t* proj = G_Spawn();

		MorcalavinProjectileInit(self, proj);
		proj->owner = self->owner;
		proj->ideal_yaw = self->ideal_yaw;

		Create_rand_relect_vect(self->velocity, proj->velocity);
		Vec3ScaleAssign(proj->ideal_yaw, proj->velocity);
		vectoangles(proj->velocity, proj->s.angles);

		gi.CreateEffect(&self->s, FX_HP_MISSILE, CEF_OWNERS_ORIGIN, self->s.origin, "vb", vec3_origin, HPMISSILE1_EXPLODE); //TODO: play SND_HOMEHIT sound?
		gi.linkentity(proj);

		G_SetToFree(self);

		return;
	}

	if (trace->ent->takedamage != DAMAGE_NO)
	{
		vec3_t hit_dir;
		VectorCopy(self->velocity, hit_dir);
		VectorNormalize(hit_dir);

		T_Damage(trace->ent, self, self->owner, hit_dir, self->s.origin, trace->plane.normal, irand(3, 5), 0, DAMAGE_SPELL | DAMAGE_NO_KNOCKBACK, MOD_DIED); //TODO: 'damage' arg value is the only difference between this and MorcalavinProjectile2Blocked().
	}

	gi.CreateEffect(NULL, FX_M_EFFECTS, 0, self->s.origin, "bv", FX_MORK_MISSILE_HIT, trace->plane.normal); //TODO: play SND_HOMEHIT sound?

	self->think = G_FreeEdict;
	self->nextthink = level.time + FRAMETIME; //mxd. Use define.
}

static void MorcalavinLightningThink(edict_t* self) //mxd. Named 'morcalavin_check_lightning' in original logic.
{
	if (self->owner == NULL || self->owner->enemy == NULL)
	{
		self->nextthink = level.time + FRAMETIME; //mxd. Use define.
		return;
	}

	vec3_t dir;
	VectorSubtract(self->s.origin, self->owner->enemy->s.origin, dir);
	const float dist = VectorNormalize(dir);

	if (dist < 150.0f)
	{
		T_Damage(self->owner->enemy, self, self->owner, dir, vec3_origin, vec3_origin, irand(2, 4), 0, DAMAGE_SPELL, MOD_SHIELD);

		vec3_t vel;
		VectorNormalize2(self->velocity, vel);

		vec3_t lightning_start;
		VectorMA(self->s.origin, 400.0f * FRAMETIME, vel, lightning_start);

		vec3_t lightning_end;
		VectorCopy(self->owner->enemy->s.origin, lightning_end);
		lightning_end[2] += flrand(-16.0f, 32.0f); //mxd. irand() in original logic.

		gi.CreateEffect(NULL, FX_LIGHTNING, 0, lightning_start, "vbb", lightning_end, irand(2, 4), 0); //TODO: play SND_LGHTNGHIT sound?

		if (SKILL < SKILL_MEDIUM)
		{
			VectorNormalize(self->velocity);
			Vec3AddAssign(vel, self->velocity);
			Vec3ScaleAssign(400.0f, self->velocity);
		}
	}

	self->nextthink = level.time + FRAMETIME; //mxd. Use define.
}

static void MorcalavinMissileThink(edict_t* self) //mxd. Named 'morcalavin_missile_update' in original logic.
{
	vec3_t forward;
	vec3_t right;
	AngleVectors(self->owner->s.angles, forward, right, NULL);

	switch (self->owner->s.frame)
	{
		case FRAME_atakc3:
			VectorMA(self->owner->s.origin, 16.0f, forward, self->s.origin);
			VectorMA(self->s.origin, 10.0f, right, self->s.origin);
			self->s.origin[2] += 24.0f;
			break;

		case FRAME_atakc4:
			VectorMA(self->owner->s.origin, 16.0f, forward, self->s.origin);
			VectorMA(self->s.origin, 12.0f, right, self->s.origin);
			self->s.origin[2] += 26.0f;
			break;

		case FRAME_atakc5:
			VectorMA(self->owner->s.origin, 16.0f, forward, self->s.origin);
			VectorMA(self->s.origin, 14.0f, right, self->s.origin);
			self->s.origin[2] += 28.0f;
			break;

		case FRAME_atakc6:
			VectorMA(self->owner->s.origin, 15.0f, forward, self->s.origin);
			VectorMA(self->s.origin, 14.0f, right, self->s.origin);
			self->s.origin[2] += 32.0f;
			break;

		case FRAME_atakc7:
			VectorMA(self->owner->s.origin, 15.0f, forward, self->s.origin);
			VectorMA(self->s.origin, 14.0f, right, self->s.origin);
			self->s.origin[2] += 36.0f;
			break;

		case FRAME_atakc8:
			VectorMA(self->owner->s.origin, 14.0f, forward, self->s.origin);
			VectorMA(self->s.origin, 14.0f, right, self->s.origin);
			self->s.origin[2] += 37.0f;
			break;

		case FRAME_atakc9:
			VectorMA(self->owner->s.origin, 14.0f, forward, self->s.origin);
			VectorMA(self->s.origin, 14.0f, right, self->s.origin);
			self->s.origin[2] += 38.0f;
			break;

		case FRAME_atakc10:
			VectorMA(self->owner->s.origin, 14.0f, forward, self->s.origin);
			VectorMA(self->s.origin, 14.0f, right, self->s.origin);
			self->s.origin[2] += 39.0f;
			break;

		case FRAME_atakc11:
			VectorMA(self->owner->s.origin, 14.0f, forward, self->s.origin);
			VectorMA(self->s.origin, 14.0f, right, self->s.origin);
			self->s.origin[2] += 40.0f;
			break;

		case FRAME_atakc12:
			VectorMA(self->owner->s.origin, 14.0f, forward, self->s.origin);
			VectorMA(self->s.origin, 14.0f, right, self->s.origin);
			self->s.origin[2] += 41.0f;
			break;

		case FRAME_atakc13:
		case FRAME_atakc14:
			VectorMA(self->owner->s.origin, 14.0f, forward, self->s.origin);
			VectorMA(self->s.origin, 14.0f, right, self->s.origin);
			self->s.origin[2] += 42.0f;
			break;

		case FRAME_atakc15:
		{
			VectorMA(self->owner->s.origin, 16.0f, forward, self->s.origin);
			VectorMA(self->s.origin, 14.0f, right, self->s.origin);
			self->s.origin[2] += 42.0f;

			const vec3_t end_pos =
			{
				self->owner->enemy->s.origin[0],
				self->owner->enemy->s.origin[1],
				self->owner->enemy->s.origin[2] + (float)self->owner->enemy->viewheight
			};

			vec3_t diff;
			VectorSubtract(end_pos, self->s.origin, diff);
			VectorNormalize(diff);
			VectorScale(diff, 400.0f, self->velocity);

			self->think = MorcalavinLightningThink; //TODO: play SND_LIGHTNING sound?
			self->nextthink = level.time + FRAMETIME; //mxd. Use define.
		} return;
	}

	gi.linkentity(self);

	self->nextthink = level.time + FRAMETIME; //mxd. Use define.
}

void morcalavin_start_missile(edict_t* self)
{
	edict_t* proj = G_Spawn();

	proj->solid = SOLID_BBOX;
	proj->movetype = PHYSICSTYPE_FLY;
	proj->gravity = 0.0f;
	proj->clipmask = MASK_SHOT;
	proj->s.effects = (EF_MARCUS_FLAG1 | EF_CAMERA_NO_CLIP);

	proj->think = MorcalavinMissileThink;
	proj->nextthink = level.time + FRAMETIME; //mxd. Use define.

	proj->isBlocked = MorcalavinProjectile2Blocked;
	proj->isBlocking = MorcalavinProjectile2Blocked;
	proj->bounced = MorcalavinProjectile2Blocked;

	proj->owner = self;
	self->target_ent = proj;

	vec3_t forward;
	vec3_t right;
	AngleVectors(self->s.angles, forward, right, NULL);
	VectorMA(self->s.origin, 16.0f, forward, proj->s.origin);
	VectorMA(proj->s.origin, 10.0f, right, proj->s.origin);
	proj->s.origin[2] += 24.0f;

	VectorSet(proj->mins, -4.0f, -4.0f, -4.0f);
	VectorSet(proj->maxs,  4.0f,  4.0f,  4.0f);

	gi.linkentity(proj);

	// Create the effect.
	gi.CreateEffect(&proj->s, FX_M_EFFECTS, CEF_OWNERS_ORIGIN, proj->s.origin, "bv", FX_MORK_MISSILE, proj->s.origin);
	gi.sound(self, CHAN_AUTO, sounds[SND_PPCHARGE], 1.0f, ATTN_NORM, 0.0f);
}

void morcalavin_release_missile(edict_t* self)
{
	gi.sound(self, CHAN_AUTO, sounds[SND_PPFIRE], 1.0f, ATTN_NORM, 0.0f);
}

static void MorcalavinProjectile1Blocked(edict_t* self, trace_t* trace) //mxd. Named 'morcalavin_proj1_blocked' in original logic.
{
	if (trace->ent == self->owner || Q_stricmp(trace->ent->classname, "Morcalavin_Missile") == 0) //mxd. stricmp -> Q_stricmp
		return;

	// Reflection stuff.
	if (EntReflecting(trace->ent, true, true))
	{
		edict_t* proj = G_Spawn();

		MorcalavinProjectileInit(self, proj);
		proj->owner = self->owner;
		proj->ideal_yaw = self->ideal_yaw;

		Create_rand_relect_vect(self->velocity, proj->velocity);
		Vec3ScaleAssign(proj->ideal_yaw, proj->velocity);
		vectoangles(proj->velocity, proj->s.angles);

		gi.CreateEffect(&self->s, FX_HP_MISSILE, CEF_OWNERS_ORIGIN, self->s.origin, "vb", vec3_origin, HPMISSILE1_EXPLODE); //TODO: play SND_HOMEHIT sound?

		gi.linkentity(proj);
		G_SetToFree(self);

		return;
	}

	// Do the rest of the stuff.
	if (trace->ent->takedamage != DAMAGE_NO)
	{
		vec3_t hit_dir;
		VectorNormalize2(self->velocity, hit_dir);

		T_Damage(trace->ent, self, self->owner, hit_dir, self->s.origin, trace->plane.normal, irand(4, 8), 0, DAMAGE_SPELL | DAMAGE_NO_KNOCKBACK, MOD_DIED);
	}

	gi.CreateEffect(&self->s, FX_HP_MISSILE, CEF_OWNERS_ORIGIN, self->s.origin, "vb", vec3_origin, HPMISSILE1_EXPLODE); //TODO: play SND_HOMEHIT sound?

	self->think = G_FreeEdict;
	self->nextthink = level.time + FRAMETIME; //mxd. Use define.
}

// Create the guts of morcalavin's projectile.
static void MorcalavinProjectileInit(edict_t* self, edict_t* proj) //mxd. Named 'create_morcalavin_proj' in original logic.
{
	proj->svflags |= SVF_ALWAYS_SEND;
	proj->movetype = PHYSICSTYPE_FLY;
	proj->gravity = 0.0f;
	proj->solid = SOLID_BBOX;
	proj->classname = "Morcalavin_Missile";
	proj->dmg = 1;
	proj->s.scale = 1.0f;
	proj->clipmask = MASK_SHOT;
	proj->nextthink = level.time + FRAMETIME; //mxd. Use define.

	proj->isBlocked = MorcalavinProjectile1Blocked;
	proj->isBlocking = MorcalavinProjectile1Blocked;
	proj->bounced = MorcalavinProjectile1Blocked;

	proj->s.effects = (EF_MARCUS_FLAG1 | EF_CAMERA_NO_CLIP);
	proj->enemy = self->enemy;

	VectorSet(proj->mins, -2.0f, -2.0f, -2.0f);
	VectorSet(proj->maxs,  2.0f,  2.0f,  2.0f);
	VectorCopy(self->s.origin, proj->s.origin);
}

void morcalavin_taunt_shot(edict_t* self)
{
	if (self->enemy == NULL)
		return;

	// Only predict once for all the missiles.
	vec3_t pred_pos;
	M_PredictTargetPosition(self->enemy, self->enemy->velocity, 5.0f, pred_pos);

	vec3_t forward;
	vec3_t right;
	AngleVectors(self->s.angles, forward, right, NULL);

	vec3_t start_pos;
	VectorMA(self->s.origin, -8.0f, forward, start_pos);
	VectorMA(start_pos, -16.0f, right, start_pos);
	start_pos[2] += 8.0f;

	vec3_t diff;
	VectorSubtract(pred_pos, start_pos, diff);
	VectorNormalize(diff);

	vec3_t angles;
	vectoangles(diff, angles);

	angles[PITCH] = -angles[PITCH] + flrand(-4.0f, 4.0f);
	angles[YAW] += flrand(-4.0f, 4.0f);

	// Spawn the projectile.
	edict_t* proj = G_Spawn();

	MorcalavinProjectileInit(self, proj);
	proj->owner = self;

	VectorCopy(start_pos, proj->s.origin);

	vec3_t vel;
	AngleVectors(angles, vel, NULL, NULL);

	VectorScale(vel, flrand(700.0f, 800.0f) + (skill->value * 100.0f), proj->velocity); //mxd. irand() in original logic.
	vectoangles(proj->velocity, proj->s.angles);

	gi.sound(self, CHAN_AUTO, sounds[SND_HOMING], 1.0f, ATTN_NORM, 0.0f);
	gi.CreateEffect(&proj->s, FX_M_EFFECTS, CEF_OWNERS_ORIGIN, NULL, "bv", FX_MORK_TRACKING_MISSILE, proj->s.origin);
	gi.linkentity(proj);
}

void morcalavin_phase_out (edict_t *self)
{
	int	interval = 40;

	if(self->s.color.a > interval)
	{
		self->s.color.a -= irand(interval/2, interval);
		self->pre_think = morcalavin_phase_out;
		self->next_pre_think = level.time + 0.05;
	}
	else 
	{
		self->s.color.a = 0;
		self->pre_think = NULL;
		self->next_pre_think = -1;
	}
}

void morcalavin_phase_in (edict_t *self)
{
	int	interval = 12;
	
	if(self->s.color.a < 255 - interval)
	{
		self->s.color.a += irand(interval/2, interval);
		self->pre_think = morcalavin_phase_in;
		self->next_pre_think = level.time + 0.05;
	}
	else 
	{
		self->svflags &= ~SVF_NO_AUTOTARGET;
		self->s.color.c = 0xffffffff;
		
		if(self->health <= 0 || self->monsterinfo.lefty >= 6)
		{
			self->pre_think = NULL;
			self->next_pre_think = -1;
		}
		else
			morcalavin_init_phase_out(self);
	}
}

static void morcalavin_init_phase_out (edict_t *self)
{
	//Become tangible once more
	self->solid = SOLID_NOT;
	self->pre_think = morcalavin_phase_out;
	self->next_pre_think = level.time + FRAMETIME;
	self->svflags |= SVF_NO_AUTOTARGET;
}

void morcalavin_init_phase_in (edict_t *self)
{
	self->pre_think = morcalavin_phase_in;
	self->next_pre_think = level.time + FRAMETIME;
}

/*

	morcalavin Shield Functions

*/

void mork_laugh (edict_t *self)
{
	gi.sound(self, CHAN_VOICE, sounds[SND_LAUGH], 1, ATTN_NONE, 0);
	self->post_think = NULL;
	self->next_post_think = -1;
}

void mork_check_killed_enemy(edict_t *attacker)
{
	if(attacker->enemy && attacker->enemy->health<=0)
	{
		attacker->post_think = mork_laugh;
		attacker->next_post_think = level.time + flrand(1.3, 2.3);
	}
}

void projectile_veer(edict_t *self, float amount)
{
//useful code for making projectiles wander randomly to a specified degree
	vec3_t	veerdir;
	float	speed;

	speed = VectorLength(self->velocity);
	VectorSet(veerdir,
		flrand(-amount, amount),
		flrand(-amount, amount),
		flrand(-amount, amount));
	VectorAdd(veerdir, self->velocity, self->velocity);
	VectorNormalize(self->velocity);
	Vec3ScaleAssign(speed, self->velocity);

	//check to see if this is needed
	//	self.angles=vectoangles(self.velocity);
}

void projectile_homethink (edict_t *self)
{
	vec3_t olddir, newdir, huntdir;
	float oldvelmult , newveldiv, speed_mod;
	float turnspeed;

	if(self->delay)
		turnspeed = self->delay;
	else
		turnspeed = 1.3;

	VectorCopy(self->velocity, olddir);
	VectorNormalize(olddir);

	VectorSubtract(self->enemy->s.origin, self->s.origin, huntdir);
	VectorNormalize(huntdir);

	oldvelmult = turnspeed;
	newveldiv = 1/(oldvelmult + 1);
	
	VectorScale(olddir, oldvelmult, olddir);
	VectorAdd(olddir, huntdir, newdir);
	VectorScale(newdir, newveldiv, newdir);

	speed_mod = DotProduct( olddir , newdir );

	if (speed_mod < 0.05)
		speed_mod = 0.05;

	newveldiv *= self->ideal_yaw * speed_mod;

	VectorScale(olddir, oldvelmult, olddir);
	VectorAdd(olddir, huntdir, newdir);
	VectorScale(newdir, newveldiv, newdir);

	VectorCopy(newdir, self->velocity);

	if(self->random)
		projectile_veer(self, self->random);
}

/*

	morcalavin Projectile Functions

*/

/*-----------------------------------------------
			morcalavin_proj1_think
-----------------------------------------------*/

void morcalavin_proj1_think( edict_t *self )
{
	if(AI_IsClearlyVisible(self, self->enemy))
		projectile_homethink(self);

	self->nextthink = level.time + 0.1;
}

/*-----------------------------------------------
			morcalavin_proj1_blocked
-----------------------------------------------*/

void beam_blocked( edict_t *self, trace_t *trace )
{	
//	edict_t	*proj;

	/*
	if(EntReflecting(trace->ent, true, true) && self->reflect_debounce_time)
	{
		proj = G_Spawn();

		create_morcalavin_proj(self,proj);
		proj->isBlocked = beam_blocked;
		proj->classname = "M_ref_HMissile";
		proj->owner = self->owner;
		proj->ideal_yaw = self->ideal_yaw;
		proj->classID = self->classID;

		Create_rand_relect_vect(self->velocity, proj->velocity);
		Vec3ScaleAssign(proj->ideal_yaw,proj->velocity);
		vectoangles(proj->velocity, proj->s.angles);

		gi.CreateEffect(&proj->s,
					FX_M_EFFECTS,
					CEF_OWNERS_ORIGIN,
					NULL,
					"bv",
					proj->classID,
					proj->velocity);

		proj->reflect_debounce_time = self->reflect_debounce_time -1;
		gi.linkentity(proj); 

		G_SetToFree(self);

		return;
	}*/

	if (trace->ent->takedamage )
	{
		vec3_t	hitDir;
		float	damage = flrand(MORK_DMG_PROJ1_MIN, MORK_DMG_PROJ1_MAX);
	
		if(self->dmg)
			damage += self->dmg;
		VectorCopy( self->velocity, hitDir );
		VectorNormalize( hitDir );

		T_Damage(trace->ent, self, self->owner, hitDir, self->s.origin, trace->plane.normal, damage, 0, DAMAGE_SPELL | DAMAGE_NO_KNOCKBACK,MOD_DIED);
	}

	gi.sound(self, CHAN_WEAPON, gi.soundindex("monsters/seraph/guard/spellhit.wav"), 1, ATTN_NORM, 0);

	gi.CreateEffect(&self->s,
				FX_M_EFFECTS,
				CEF_OWNERS_ORIGIN,
				self->s.origin,
				"bv",
				FX_M_MISC_EXPLODE,
				vec3_origin);

	G_SetToFree(self);
}

void beam_think (edict_t *self)
{
	self->think = NULL;
	self->nextthink = -1;
}

void morcalavin_beam( edict_t *self)
{
	edict_t	*proj;
	vec3_t	Forward, vf, vr, endpos;

	// Spawn the projectile

	proj = G_Spawn();

	MorcalavinProjectileInit(self,proj);

	proj->reflect_debounce_time = MAX_REFLECT;
	proj->classname = "M_Beam";

	proj->isBlocked = beam_blocked;

	proj->owner = self;
	
	AngleVectors(self->s.angles, vf, vr, NULL);
	VectorMA(proj->s.origin, 48,  vf, proj->s.origin);
	VectorMA(proj->s.origin, -20, vr, proj->s.origin);
	proj->s.origin[2] += 16;

	VectorSet(endpos, self->enemy->s.origin[0], self->enemy->s.origin[1], self->enemy->s.origin[2] + self->enemy->viewheight);
	VectorSubtract(endpos, proj->s.origin, Forward);
	VectorNormalize(Forward);
	
	VectorScale(Forward, 1250, proj->velocity);

	vectoangles(proj->velocity, proj->s.angles);

	proj->dmg = flrand(MORK_DMG_BEAM_MIN, MORK_DMG_BEAM_MAX);

	proj->think=beam_think;
	proj->nextthink = level.time + 1;

	gi.sound(self, CHAN_WEAPON, gi.soundindex("monsters/seraph/guard/spell.wav"), 1, ATTN_NORM, 0);

	//TODO: Spawn a muzzle flash
	gi.CreateEffect(&proj->s,
				FX_M_EFFECTS,
				CEF_OWNERS_ORIGIN,
				vec3_origin,
				"bv",
				FX_M_BEAM,
				proj->s.angles);


	gi.linkentity(proj); 
}


void morcalavin_beam2( edict_t *self)
{
	edict_t	*proj;
	vec3_t	Forward, vf, vr;

	// Spawn the projectile

	proj = G_Spawn();

	MorcalavinProjectileInit(self,proj);

	proj->reflect_debounce_time = MAX_REFLECT;
	proj->classname = "M_Beam";

	proj->isBlocked = beam_blocked;

	proj->owner = self;
	
	AngleVectors(self->s.angles, vf, vr, NULL);
	VectorMA(proj->s.origin, 48,  vf, proj->s.origin);
	VectorMA(proj->s.origin, -20, vr, proj->s.origin);
	proj->s.origin[2] += 16;

	VectorSubtract(self->enemy->s.origin, proj->s.origin, Forward);
	VectorNormalize(Forward);
	
	VectorScale(Forward, 1250, proj->velocity);

	vectoangles(proj->velocity, proj->s.angles);

	proj->dmg = flrand(MORK_DMG_BEAM_MIN, MORK_DMG_BEAM_MAX);

	proj->think=beam_think;
	proj->nextthink = level.time + 1;

	gi.sound(self, CHAN_WEAPON, gi.soundindex("monsters/seraph/guard/spell.wav"), 1, ATTN_NORM, 0);

	//TODO: Spawn a muzzle flash
	gi.CreateEffect(&proj->s,
				FX_M_EFFECTS,
				CEF_OWNERS_ORIGIN,
				vec3_origin,
				"bv",
				FX_M_BEAM,
				proj->s.angles);


	gi.linkentity(proj); 
}

/*-----------------------------------------------
		morcalavin_ground_attack
-----------------------------------------------*/

void morcalavin_ground_attack( edict_t *self )
{
	gi.CreateEffect(NULL, 
					FX_M_EFFECTS, 
					0, 
					self->s.origin, 
					"bv", 
					FX_GROUND_ATTACK, 
					self->enemy->s.origin);
}

/*-----------------------------------------------
				morcalavin_quake_pause
-----------------------------------------------*/

void morcalavin_quake_pause( edict_t *self )
{
	if (self->monsterinfo.flee_finished)
	{
		self->monsterinfo.flee_finished = false;
		SetAnim(self, ANIM_GROUND_ATTACK);
	}
}

/*-----------------------------------------------
				morcalavin_quake
-----------------------------------------------*/
void morcalavin_quake(edict_t *self, float pitch_ofs, float yaw_ofs, float roll_ofs)
{
	vec3_t	org, vf, vr;

	//Create the effect
	AngleVectors(self->s.angles, vf, vr, NULL);
	
	VectorMA(self->s.origin, 44, vf, org);
	VectorMA(org, -14, vr, org);
	org[2] += self->mins[2];

	gi.CreateEffect(	NULL,
						FX_M_EFFECTS,
						0,
						self->s.origin,
						"bv",
						FX_QUAKE_RING,
						org);

	//Check to see if the player is on the ground, and if he is, then knock him down
	if (self->enemy && self->enemy->groundentity && self->enemy->client)
	{
		//Knock the player down
		P_KnockDownPlayer(&self->enemy->client->playerinfo);

		//Denote we've done so to follow it with an attack		
		self->monsterinfo.flee_finished = true;
	}
}

/*

	Morcalavin Helper Functions

*/


/*-----------------------------------------------
				morcalavin_move
-----------------------------------------------*/

void morcalavin_move( edict_t *self, float vf, float vr, float vu )
{
}

void morcalavin_rush_sound (edict_t *self)
{
	vec3_t forward;
	gi.sound(self, CHAN_BODY, sounds[SND_RUSH], 1, ATTN_NORM, 0);
	AngleVectors(self->s.angles, forward, NULL, NULL);
	VectorScale(forward, 250, self->velocity);
	self->velocity[2] = 150;
}

/*-----------------------------------------------
	morcalavin_pause
-----------------------------------------------*/

void morcalavin_pause( edict_t *self )
{
	if (self->monsterinfo.lefty < 6 && self->health > 0)
	{
		SetAnim(self, ANIM_FLOAT);
		return;
	}

	self->takedamage = DAMAGE_YES;
	self->mood_think(self);
	
	switch (self->ai_mood)
	{
		case AI_MOOD_ATTACK:
			if(self->ai_mood_flags & AI_MOOD_FLAG_MISSILE)
				QPostMessage(self, MSG_MISSILE, PRI_DIRECTIVE, NULL);
			else
				QPostMessage(self, MSG_MELEE, PRI_DIRECTIVE, NULL);
			break;
		case AI_MOOD_PURSUE:
		case AI_MOOD_NAVIGATE:
			QPostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_DELAY:
		case AI_MOOD_STAND:
			QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_WANDER:
			SetAnim(self, ANIM_WALK);
			break;

		default :
#ifdef _DEVEL
			gi.dprintf("mork: Unusable mood %d!\n", self->ai_mood);
#endif
			break;
	}
}

/*-----------------------------------------------
	morcalavin_idle
-----------------------------------------------*/

void morcalavin_idle(edict_t *self)
{
}

/*

	morcalavin Message Functions

*/

/*-----------------------------------------------
	morcalavin_death
-----------------------------------------------*/

void morcalavin_death( edict_t *self, G_Message_t *msg )
{
	self->monsterinfo.stepState++;
}

void morcalavin_retort( edict_t *self)
{
	self->msgHandler = DefaultMsgHandler;
	SetAnim(self, ANIM_RETORT);
}

void morcalavin_getup( edict_t *self)
{
	if (self->monsterinfo.lefty==7)
	{
		if (self->monsterinfo.attack_finished > 0 && self->monsterinfo.attack_finished < level.time)
		{
			self->monsterinfo.attack_finished = -1;
			gi.sound(self, CHAN_VOICE, sounds[SND_REVIVE], 1, ATTN_NORM, 0);
			SetAnim(self, ANIM_GETUP);
		}
	}
}

void morcalavin_hurtidle( edict_t *self)
{
	SetAnim(self, ANIM_HURTIDLE);
}

/*-----------------------------------------------
				morcalavin_evade
-----------------------------------------------*/


void morcalavin_evade( edict_t *self, G_Message_t *msg )
{
	//FIXME: Make him do something smart here...
}

/*-----------------------------------------------
				morcalavin_stand
-----------------------------------------------*/

void morcalavin_stand(edict_t *self, G_Message_t *msg)
{
	if (self->health <= 0)
		return;

	//SetAnim(self, ANIM_ATTACK1);
	SetAnim(self, ANIM_FLOAT);
}

/*-----------------------------------------------
				morcalavin_run
-----------------------------------------------*/

void mork_ai_hover (edict_t *self, float dist)
{
	vec3_t		bottom;
	trace_t		trace;
	float		desired_vel;

	if (self->health <= 0)
		return;

	if(self->enemy)
		ai_charge(self, 0);
	else
		ai_stand(self, 0);

	if(dist)
	{
		VectorCopy(self->s.origin, bottom);
		bottom[2] -= dist;
		gi.trace(self->s.origin, self->mins, self->maxs, bottom, self, MASK_SOLID, &trace);

		if(trace.fraction<1.0)
		{
			desired_vel = (1 - trace.fraction) * dist;
			
			if(self->velocity[2] < desired_vel)
				self->velocity[2] = desired_vel;

			return;
		}
	}
}

void mork_ai_run (edict_t *self, float dist)
{
	vec3_t	forward;

	if (self->health <= 0)
		return;

	if(self->curAnimID!=ANIM_FLY)
	{
		MG_AI_Run(self, dist);
		if(!self->groundentity)
		{
			if(self->curAnimID == ANIM_WALK)
			{
				AngleVectors(self->s.angles, forward, NULL, NULL);
				VectorScale(forward, 250, self->velocity);
				self->velocity[2] = 150;
			}
			SetAnim(self, ANIM_GLIDE);
		}
		else
			SetAnim(self, ANIM_WALK);
	}
	else
	{
		/*
		gi.CreateEffect(&self->s,
					FX_M_EFFECTS,
					0,
					vec3_origin,
					"bv",
					FX_M_MOBLUR,
					self->s.angles);
		*/

		ai_charge(self, dist);
	}
}

void morcalavin_run(edict_t *self, G_Message_t *msg)
{//if can't move, go into a float for a bit
	
	if (self->health <= 0)
		return;

	if(!self->enemy)
	{
		SetAnim(self, ANIM_FLOAT);
		return;
	}

	if(self->enemy->health<=0)
	{
		mork_check_killed_enemy(self->enemy);
		SetAnim(self, ANIM_FLOAT);
		return;
	}

	if (self->monsterinfo.lefty == 0)
	{
		morcalavin_init_phase_out(self);
		SetAnim(self, ANIM_FLOAT);
		gi.sound(self, CHAN_VOICE, sounds[SND_REVIVE], 1, ATTN_NORM, 0);
		self->solid = SOLID_NOT;
		self->monsterinfo.sound_start = level.time + 2.5;
		self->monsterinfo.lefty++;

		return;
	}

	if(!self->groundentity)
		SetAnim(self, ANIM_GLIDE);
	else
		SetAnim(self, ANIM_WALK);
}

void morcalavin_rush(edict_t *self, G_Message_t *msg)
{
//	self->gravity = 0.0f;
	SetAnim(self, ANIM_FLY);
}

enum
{
	MORK_ATTACK_FADE,
	MORK_ATTACK_TRACKING,
	MORK_ATTACK_SPHERE,
	MORK_ATTACK_BEAM,
	MORK_ATTACK_5SPHERE
}; morcalavin_attackID_t;

void morcalavin_missile( edict_t *self, G_Message_t *msg)
{
	int chance = irand(0, 100);

	if (chance < 5 && self->wait != MORK_ATTACK_FADE)
	{
		morcalavin_attack_fade_out(self);
		self->wait = MORK_ATTACK_FADE;
	}
	else if (chance < 25 && self->wait != MORK_ATTACK_TRACKING)
	{
		SetAnim(self, ANIM_TRACKING1);
		self->wait = MORK_ATTACK_TRACKING;
	}
	else if (chance < 50 && self->wait != MORK_ATTACK_SPHERE)
	{
		SetAnim(self, ANIM_ATTACK2B);
		self->wait = MORK_ATTACK_SPHERE;
	}
	else if (chance < 75 && self->wait != MORK_ATTACK_BEAM)
	{
		SetAnim(self, ANIM_ATTACK3);
		self->wait = MORK_ATTACK_BEAM;
	}
	else if  (self->monsterinfo.stepState > 1 && self->wait != MORK_ATTACK_5SPHERE)
	{
		SetAnim(self, ANIM_ATTACK4);
		self->wait = MORK_ATTACK_5SPHERE;
	}
}

void morcalavin_melee( edict_t *self, G_Message_t *msg)
{
	SetAnim(self, ANIM_ATTACK1);
}

/*

	morcalavin Spawn Functions

*/

void MorcalavinStaticsInit(void)
{
	static ClassResourceInfo_t resInfo; //mxd. Made local static.

	classStatics[CID_MORK].msgReceivers[MSG_STAND]	= morcalavin_stand;
	classStatics[CID_MORK].msgReceivers[MSG_MELEE] = morcalavin_melee;
	classStatics[CID_MORK].msgReceivers[MSG_MISSILE] = morcalavin_missile;
	classStatics[CID_MORK].msgReceivers[MSG_RUN] = morcalavin_run;
	classStatics[CID_MORK].msgReceivers[MSG_EVADE] = morcalavin_evade;
	classStatics[CID_MORK].msgReceivers[MSG_DEATH] = morcalavin_death;
	
	resInfo.numAnims = NUM_ANIMS;
	resInfo.animations = animations;
	resInfo.modelIndex = gi.modelindex("models/monsters/morcalavin/tris.fm");
	resInfo.numSounds = NUM_SOUNDS;
	resInfo.sounds = sounds;

//quake attack
	sounds[SND_QUAKE]=gi.soundindex("monsters/mork/quake.wav");	
//straightt-fire beam
	sounds[SND_BEAM]=gi.soundindex("monsters/mork/beam.wav");	
	sounds[SND_BEAMHIT]=gi.soundindex("monsters/mork/beamhit.wav");	
//homing balls
	sounds[SND_HOMING]=gi.soundindex("monsters/mork/homing.wav");	
	sounds[SND_HOMEHIT]=gi.soundindex("monsters/mork/homehit.wav");	
//power Puff
	sounds[SND_PPCHARGE]=gi.soundindex("monsters/mork/ppcharge.wav");	
	sounds[SND_PPFIRE]=gi.soundindex("monsters/mork/ppfire.wav");	
	sounds[SND_PPEXPLODE]=gi.soundindex("monsters/mork/ppexplode.wav");	
//Lightning from eyes
	sounds[SND_LIGHTNING]=gi.soundindex("monsters/mork/lightning.wav");	
	sounds[SND_LGHTNGHIT]=gi.soundindex("monsters/mork/lghtnghit.wav");	
//Shove
	sounds[SND_FORCEWALL]=gi.soundindex("monsters/mork/forcewall.wav");	
//Shield
	sounds[SND_MAKESHIELD]=gi.soundindex("monsters/mork/makeshield.wav");	
	sounds[SND_SHIELDHIT]=gi.soundindex("monsters/mork/shieldhit.wav");	
	sounds[SND_SHIELDPULSE]=gi.soundindex("monsters/mork/shieldpulse.wav");	
	sounds[SND_SHIELDGONE]=gi.soundindex("monsters/mork/shieldgone.wav");	
	sounds[SND_SHIELDBREAK]=gi.soundindex("monsters/mork/shieldbreak.wav");	
//Fly forward
	sounds[SND_RUSH]=gi.soundindex("monsters/mork/rush.wav");	
//hurt and get up	
	sounds[SND_FALL]=gi.soundindex("monsters/mork/fall.wav");	
	sounds[SND_REVIVE]=gi.soundindex("monsters/mork/revive.wav");	
//strafing beams attack
	sounds[SND_STRAFEON]=gi.soundindex("monsters/mork/strafeon.wav");	
	sounds[SND_STRFSWNG]=gi.soundindex("monsters/mork/strfswng.wav");	
	sounds[SND_STRAFEOFF]=gi.soundindex("monsters/mork/strafeoff.wav");	
//hurt/kill player laugh
	sounds[SND_LAUGH]=gi.soundindex("monsters/mork/laugh.wav");	
	
//Taunts
	sounds[TAUNT_LAUGH1] =gi.soundindex("monsters/mork/laugh1.wav");	
	sounds[TAUNT_LAUGH2] =gi.soundindex("monsters/mork/laugh2.wav");	
	sounds[TAUNT_LAUGH3] =gi.soundindex("monsters/mork/laugh3.wav");	
	sounds[TAUNT_LAUGH4] =gi.soundindex("monsters/mork/laugh4.wav");	
	
	sounds[TAUNT_BELLY1] =gi.soundindex("monsters/mork/belly.wav");	
	sounds[TAUNT_BELLY2] =gi.soundindex("monsters/mork/belly2.wav");	
	sounds[TAUNT_BELLY3] =gi.soundindex("monsters/mork/digest.wav");	

	classStatics[CID_MORK].resInfo = &resInfo;
}

static void morcalavin_attack_fade_out(edict_t *self)
{
	gi.sound(self, CHAN_VOICE, sounds[SND_REVIVE], 1, ATTN_NORM, 0);
	self->monsterinfo.sound_start = level.time + 2.0;
	morcalavin_init_phase_out(self);
	SetAnim(self, ANIM_FLOAT);
	self->monsterinfo.lefty = 8;
}

void morcalavin_fade_out(edict_t *self)
{
	gi.sound(self, CHAN_VOICE, sounds[SND_REVIVE], 1, ATTN_NORM, 0);
	self->monsterinfo.sound_start = level.time + 2.0;
	morcalavin_init_phase_out(self);
	SetAnim(self, ANIM_FLOAT);
}

qboolean morcalavin_choose_teleport_destination(edict_t *self)
{
	vec3_t	teleport_angles, forward, endpos, startpos;
	trace_t trace;
	int	num_tries, i;
	edict_t	*noblockent;
	float	tracedist;

	//Instead of chance, do around self if evade, around other if ambush
	if(!self->enemy)
	{
		//Phase in and become tangible again
		morcalavin_init_phase_in(self);
		self->takedamage = DAMAGE_YES;
		self->monsterinfo.lefty = 10;
		return false;
	}

	num_tries = 10;

	for(i = 0; i < num_tries; i++)
	{
		VectorSet(teleport_angles, 0, anglemod(flrand(0, 360)), 0);
		AngleVectors(teleport_angles, forward, NULL, NULL);
		VectorCopy(self->enemy->s.origin, startpos);
		startpos[2]+=self->enemy->mins[2];
		startpos[2]-=self->mins[2];
		tracedist = irand(self->min_missile_range, self->missile_range);
		VectorMA(startpos, -tracedist, forward, endpos);
		noblockent = self->enemy;
		
		gi.trace(startpos, self->mins, self->maxs, endpos, noblockent, MASK_MONSTERSOLID, &trace);
		
		if(trace.fraction*tracedist < 100)//min origin lerp dist
			continue;

		if(trace.allsolid || trace.startsolid)
			continue;
		
		if(vhlen(trace.endpos, self->enemy->s.origin)>=128)
		{
			VectorCopy(trace.endpos, startpos);
			VectorCopy(trace.endpos, endpos);
			endpos[2] -=64;
			gi.trace(startpos, self->mins, self->maxs, endpos, noblockent, MASK_MONSTERSOLID,&trace);
			if(trace.fraction<1.0 && !trace.allsolid && !trace.startsolid)//the last two should be false if trace.fraction is < 1.0 but doesn't hurt to check
			{
				VectorCopy(trace.endpos, self->s.origin);
				gi.linkentity(self);
				return true;
			}
		}
	}
	return false;
}

//Teleport in and attack the player quickly, before fading out again
void morcalavin_teleport_attack(edict_t *self)
{
	int chance;

	//Find a valid point away from the player
	morcalavin_choose_teleport_destination(self);
	
	//Start the animation for the attack
	if (self->monsterinfo.lefty == 8)
		SetAnim(self, ANIM_ATTACK4);
	else
		SetAnim(self, ANIM_ATTACK2);

	//Play the teleport in sound fx
	gi.sound(self, CHAN_AUTO, sounds[SND_MAKESHIELD], 1, ATTN_NORM, 0);	

	//Start phasing back in
	morcalavin_init_phase_in(self);

	//Become tangible once more
	self->solid = SOLID_BBOX;

	if (self->monsterinfo.stepState)
	{
		chance = irand(0,6);
		switch (chance)
		{
		case 0:
			gi.sound(self, CHAN_AUTO, sounds[TAUNT_LAUGH2], 1, ATTN_NONE, 0);
			break;

		case 1:
			gi.sound(self, CHAN_AUTO, sounds[TAUNT_LAUGH3], 1, ATTN_NONE, 0);
			break;

		case 2:
			gi.sound(self, CHAN_AUTO, sounds[TAUNT_LAUGH4], 1, ATTN_NONE, 0);
			break;

		default:
			break;
		}
	}
}

void morcalavin_postthink(edict_t *self)
{
	int chance;
	
	if (!self->monsterinfo.lefty)
		MG_CheckEvade(self);

	if (self->enemy && self->monsterinfo.stepState)
	{
		if (self->dmg < self->max_health)
		{
			M_ShowLifeMeter(self->dmg, self->dmg);
			self->dmg+=50;
		}
		else
		{
			M_ShowLifeMeter(self->health, self->max_health);
			mork_check_killed_enemy(self->enemy);
		}
	}

	//Check for a teleport razzing
	if (self->monsterinfo.jump_time > 0 && self->monsterinfo.jump_time < level.time)
	{
		if (self->monsterinfo.stepState)
		{
			morcalavin_init_phase_in(self);
			self->monsterinfo.jump_time = -1;
			self->monsterinfo.sound_start = -1;
			morcalavin_teleport_attack(self);
			return;
		}
		else
		{
			morcalavin_teleport_attack(self);
			self->monsterinfo.jump_time = -1;
			return;
		}
	}

	//Check for a pending taunt
	if (self->monsterinfo.sound_start > 0 && self->monsterinfo.sound_start < level.time)
	{
		switch (self->monsterinfo.lefty)
		{
		case 1:
			gi.sound(self, CHAN_AUTO, sounds[TAUNT_LAUGH1], 1, ATTN_NONE, 0);
			self->monsterinfo.sound_start = -1;
			self->monsterinfo.jump_time = level.time + 1.0;
			self->monsterinfo.lefty++;
			break;

		case 2:
			gi.sound(self, CHAN_AUTO, sounds[TAUNT_BELLY1], 1, ATTN_NONE, 0);
			self->monsterinfo.sound_start = -1;
			self->monsterinfo.jump_time = level.time + 8.0;
			self->monsterinfo.lefty++;
			break;
		
		case 3:
			gi.sound(self, CHAN_AUTO, sounds[TAUNT_BELLY2], 1, ATTN_NONE, 0);
			self->monsterinfo.sound_start = -1;
			self->monsterinfo.jump_time = level.time + 5.0;
			self->monsterinfo.lefty++;
			break;
		
		case 4:
			gi.sound(self, CHAN_AUTO, sounds[TAUNT_BELLY3], 1, ATTN_NONE, 0);
			self->monsterinfo.sound_start = -1;
			self->monsterinfo.jump_time = level.time + 7.0;
			self->monsterinfo.lefty++;
			break;
		
		case 5:
			chance = irand(0,6);
			switch (chance)
			{
			case 0:
				gi.sound(self, CHAN_AUTO, sounds[TAUNT_LAUGH2], 1, ATTN_NONE, 0);
				break;

			case 1:
				gi.sound(self, CHAN_AUTO, sounds[TAUNT_LAUGH3], 1, ATTN_NONE, 0);
				break;

			case 2:
				gi.sound(self, CHAN_AUTO, sounds[TAUNT_LAUGH4], 1, ATTN_NONE, 0);
				break;

			default:
				break;
			}

			self->monsterinfo.sound_start = -1;
			self->monsterinfo.jump_time = level.time + 1.0;
			break;
		
		case 6:
			self->monsterinfo.jump_time = -1;
			gi.sound(self, CHAN_AUTO, sounds[TAUNT_LAUGH1], 1, ATTN_NONE, 0);
			
			if (!self->targetEnt)
			{
#ifdef _DEVEL
				if (level.time < 5.0)		// Otherwise it will print forever after Morcalavin is dead.
					gi.dprintf("GDE Fault: YOU NEED TO PUT A BARRIER IN THIS ROOM FOR THE GAME TO WORK!!!n");
#endif
				return;
			}

			self->svflags &= ~SVF_NO_AUTOTARGET;
		
			if (self->delay)
			{
				self->monsterinfo.sound_start = self->monsterinfo.attack_finished = level.time + ( self->delay );
				self->monsterinfo.sound_start += 1.5;
				self->targetEnt->monsterinfo.attack_finished = self->monsterinfo.attack_finished;
				self->delay *= 2;
			}
			else
			{
				self->monsterinfo.sound_start = level.time + 3.5;
				self->monsterinfo.attack_finished = level.time + 2.0;
				self->delay = 2;
			}
			
			self->monsterinfo.lefty++;
			break;

		case 7:
			self->monsterinfo.sound_start = -1;
			gi.sound(self, CHAN_AUTO, sounds[SND_LAUGH], 1, ATTN_NONE, 0);
			self->monsterinfo.jump_time = -1;
			break;

		case 8:
			/*
			chance = irand(0,2);
			switch (chance)
			{
			case 0:
				gi.sound(self, CHAN_AUTO, sounds[TAUNT_LAUGH2], 1, ATTN_NONE, 0);
				break;

			case 1:
				gi.sound(self, CHAN_AUTO, sounds[TAUNT_LAUGH3], 1, ATTN_NONE, 0);
				break;

			case 2:
				gi.sound(self, CHAN_AUTO, sounds[TAUNT_LAUGH4], 1, ATTN_NONE, 0);
				break;

			default:
				break;
			}*/

			self->monsterinfo.sound_start = -1;
			self->monsterinfo.jump_time = level.time + 1.0;
			break;
		}
	}

	self->next_post_think = level.time + 0.05;
}

static void morcalavin_resist_death (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	vec3_t	vf, vr, temp;

	self->msgHandler = DeadMsgHandler;

	gi.sound(self, CHAN_VOICE, sounds[SND_FALL], 1, ATTN_NORM, 0);
	SetAnim(self, ANIM_FALL);
	
	self->s.color.a = 0xFFFFFFFF;
	self->pre_think = NULL;
	self->next_pre_think = -1;

	self->takedamage = DAMAGE_NO;
	self->dmg = 1;
	self->health = self->max_health = MonsterHealth(MORK_HEALTH);
	self->monsterinfo.sound_start = level.time + 2.5;
	self->solid = SOLID_BBOX;

	self->monsterinfo.stepState++;
	self->monsterinfo.lefty = 6;

	//Check to release a charging weapon
	if (self->target_ent)
	{
		AngleVectors(self->s.angles, vf, vr, NULL);
		VectorMA(self->s.origin, 10, vf, self->target_ent->s.origin);
		VectorMA(self->target_ent->s.origin, 14, vr, self->target_ent->s.origin);
		self->target_ent->s.origin[2] += 42;

		VectorScale(vf, 400, self->target_ent->velocity);

		self->target_ent->think = MorcalavinLightningThink;
		self->target_ent->nextthink = level.time + 0.1;
	}

	//TODO: Create an effect around him
	VectorClear(temp);
	temp[0] = self->delay;

	/*
	gi.CreateEffect( &self->s,
					 FX_M_EFFECTS,
					 CEF_OWNERS_ORIGIN,
					 self->s.origin,
					 "bv",
					 FX_MORK_RECHARGE,
					 temp);
	*/

	//TODO: Create a barrier around him so the player cannot get close to him
}

/*QUAKED monster_morcalavin(1 .5 0) (-24 -24 -50) (24 24 40)

Morky

"wakeup_target" - monsters will fire this target the first time it wakes up (only once)

"pain_target" - monsters will fire this target the first time it gets hurt (only once)

*/
void SP_monster_morcalavin (edict_t *self)
{
	if (!M_WalkmonsterStart(self))		// Failed initialization
		return;

	self->msgHandler = DefaultMsgHandler;
	self->classname = "monster_morcalavin";

	if (!self->health)
		self->health = MORK_HEALTH1;

	//Apply to the end result (whether designer set or not)
	self->max_health = self->health = MonsterHealth(self->health);

	self->mass = MORK_MASS;
	self->yaw_speed = 24;

	self->movetype = PHYSICSTYPE_STEP;
	self->solid=SOLID_BBOX;

	//This is the number of times he's died (used to calculate window of opportunity for the player)
	self->delay = 0.0;

	self->s.origin[2] += 50;
	VectorSet(self->mins, -24, -24, -48);
	VectorSet(self->maxs, 24, 24, 40);

	self->viewheight = 36;

	self->materialtype = MAT_FLESH;

	self->s.modelindex = classStatics[CID_MORK].resInfo->modelIndex;
	self->s.skinnum=0;

	self->monsterinfo.otherenemyname = "player";	

	self->post_think = morcalavin_postthink;
	self->next_post_think = level.time + 0.1;

	if (self->monsterinfo.scale)
	{
		self->s.scale = self->monsterinfo.scale = MODEL_SCALE;
	}

	MG_InitMoods( self );

	QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);

	self->s.color.c = 0xFFFFFFFF;

	self->s.renderfx |= RF_GLOW;

	self->monsterinfo.stepState = 0;
	self->svflags |= SVF_BOSS | SVF_FLOAT;
	self->count = self->s.modelindex;	// For Cinematic purposes
	self->gravity = MORCALAVIN_GRAVITY;

	self->die = morcalavin_resist_death;

	gi.linkentity(self);
}

/*QUAKED obj_morcalavin_barrier (1 .5 0) ? ?

The magical barrier that prevents the player from entering the tome area and defeating 
Morcalavin

*/

void morcalavin_barrier_think(edict_t *self)
{
	edict_t *owner = NULL;

	//If we haven't found an owner yet, find one
	if (!self->owner)
	{
		owner = G_Find(NULL, FOFS(classname), "monster_morcalavin");

		if (!owner)
		{
//			gi.dprintf("Unable to bind barrier to Morcalavin!\n");
		}
		else
		{
			self->owner = owner;
			owner->targetEnt = self;
		}
	}

	if (self->monsterinfo.attack_finished > level.time)
	{
		self->count = false;
		self->svflags |= SVF_NOCLIENT;
	}
	else
	{
		self->count = true;
		self->svflags &= ~SVF_NOCLIENT;
	}

	self->nextthink = level.time + 0.1;
}

void morcalavin_barrier_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t	vel;

	if (!strcmp(other->classname, "player"))
	{
		if (self->count)
		{
			VectorSubtract(self->s.origin, other->s.origin, vel);
			VectorNormalize(vel);
			VectorScale(vel, -1, vel);
			VectorScale(vel, 512, other->velocity);
			other->velocity[2] = 128;
			other->client->playerinfo.flags |= PLAYER_FLAG_USE_ENT_POS;

			//NOTENOTE: We should always have an owner.. but this is for safety
			if (self->owner)
				T_Damage(other, self, self->owner, vel, other->s.origin, vel, irand(5, 10), 250, DAMAGE_AVOID_ARMOR, MOD_DIED);
			else
				T_Damage(other, self, self, vel, other->s.origin, vel, irand(5, 10), 250, DAMAGE_AVOID_ARMOR, MOD_DIED);

			if (self->delay < level.time)
			{
				self->delay = level.time + 0.5;
				
				gi.CreateEffect( NULL,
								 FX_WEAPON_STAFF_STRIKE,
								 0,
								 other->s.origin,
								 "db",
								 vel,
								 2);
			}
		}
	}
}

void morcalavin_barrier_use (edict_t *self, edict_t *other, edict_t *activator)
{
	//Become visible again
	self->svflags &= ~SVF_NOCLIENT;
	
	//Never do this again
	self->use = NULL;

	//Start blocking
	self->think = morcalavin_barrier_think;
	self->nextthink = level.time + 0.1;
}

void SP_obj_morcalavin_barrier (edict_t *self)
{
	gi.setmodel(self, self->model);

	self->solid = SOLID_TRIGGER;
	self->movetype = PHYSICSTYPE_NONE;

	self->touch = morcalavin_barrier_touch;
	self->use   = morcalavin_barrier_use;

	self->s.color.c = 0xFFFFFFFF;
	self->count = 1;
	self->health = true;

	gi.linkentity(self);

	//Be invisible until used
	self->svflags |= SVF_NOCLIENT;
}

