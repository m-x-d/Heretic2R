//
// m_elflord.c
//
// Copyright 1998 Raven Software
//

#include "m_elflord.h"
#include "m_elflord_anim.h" //mxd
#include "m_elflord_moves.h" //mxd
#include "m_elflord_shared.h"
#include "g_DefaultMessageHandler.h"
#include "g_monster.h"
#include "mg_ai.h" //mxd
#include "m_stats.h"
#include "spl_sphereofannihlation.h" //mxd
#include "Random.h"
#include "Utilities.h"
#include "Vector.h"
#include "g_local.h"

#pragma region ========================== Elf Lord base info ==========================

static const animmove_t* animations[] =
{
	&elflord_move_idle,
	&elflord_move_run,
	&elflord_move_charge,
	&elflord_move_charge_trans,
	&elflord_move_floatback,
	&elflord_move_dodgeright,
	&elflord_move_dodgeleft,
	&elflord_move_soa_begin,
	&elflord_move_soa_loop,
	&elflord_move_soa_end,
	&elflord_move_ls,
	&elflord_move_pain,
	&elflord_move_death_btrans,
	&elflord_move_death_loop,
	&elflord_move_shield,
	&elflord_move_attack,
	&elflord_move_move,
	&elflord_move_wait,
	&elflord_move_come_to_life
};

static int sounds[NUM_SOUNDS];

static const vec3_t projectile_mins = { -2.0f, -2.0f, -2.0f }; //mxd
static const vec3_t projectile_maxs = {  2.0f,  2.0f,  2.0f }; //mxd

#pragma endregion

#pragma region ========================== Utility functions =========================

static void FindMoveTarget(edict_t* self) //mxd. Named 'elflord_FindMoveTarget' in original logic.
{
	const edict_t* move_target = NULL;

	edict_t* e = NULL;
	while ((e = FindInRadius_Old(e, self->s.origin, 640.0f)) != NULL)
	{
		// Must be a path_corner.
		if (strcmp(e->classname, "path_corner") != 0)
			continue;

		// Must be a specified path_corner too.
		if (e->targetname == NULL || strcmp(e->targetname, "elflord") != 0) //BUGFIX: mxd. 'e->targetname && strcmp(...)' in original logic. 
			continue;

		if (vhlen(e->s.origin, self->s.origin) < 64.0f)
			continue;

		move_target = e;

		if (irand(0, 1) == 0)
			break;
	}

	if (move_target != NULL)
	{
		//FIXME: Determine a velocity to get us here.
		vec3_t target = VEC3_INIT(move_target->s.origin);
		target[2] = self->s.origin[2];

		vec3_t vel;
		VectorSubtract(target, self->s.origin, vel);
		float len = VectorNormalize(vel);

		len /= 10.0f / FRAMETIME * 2.0f;

		VectorScale(vel, len, self->velocity);
	}
}

static void MoveToFinalPosition(edict_t* self) //mxd. Named 'elflord_MoveToFinalPosition' in original logic.
{
	edict_t* e = NULL;
	while ((e = FindInRadius_Old(e, self->s.origin, 640.0f)) != NULL)
	{
		// Must be a path_corner.
		if (strcmp(e->classname, "path_corner") != 0)
			continue;

		// Must be a specified path_corner too.
		if (e->targetname == NULL || strcmp(e->targetname, "elflord_final") != 0) //BUGFIX: mxd. 'e->targetname && strcmp(...)' in original logic.
			continue;

		vec3_t target = VEC3_INIT(e->s.origin);
		target[2] = self->s.origin[2];

		vec3_t vel;
		VectorSubtract(target, self->s.origin, vel);
		float len = VectorNormalize(vel);

		len /= 10.0f / FRAMETIME * 2.0f;

		VectorScale(vel, len, self->velocity);

		return;
	}
}

static qboolean CheckAttack(edict_t* self) //mxd. Named 'elfLordCheckAttack' in original logic. //TODO: always returns false!
{
	if (!M_ValidTarget(self, self->enemy))
	{
		SetAnim(self, ANIM_HOVER);
		return false;
	}

	elflord_decelerate(self, 0.8f);

	if (self->elflord_charge_meter < self->max_health)
	{
		VectorClear(self->velocity);
		SetAnim(self, ANIM_COME_TO_LIFE);

		return false;
	}

	int projectile_chance;
	int soa_chance;
	int beam_chance;

	if (self->health < self->max_health / 3)
	{
		// Last stage.
		if (!self->elflord_last_stage)
		{
			MoveToFinalPosition(self);
			SetAnim(self, ANIM_MOVE);
			self->elflord_last_stage = true;

			return false;
		}

		if (COOP)
		{
			projectile_chance = 50;
			soa_chance = 50;
			beam_chance = 0;
		}
		else
		{
			projectile_chance = 5;
			soa_chance = 5;
			beam_chance = 90;
		}
	}
	else if (self->health < (int)((float)self->max_health / 1.5f))
	{
		// Second stage.
		projectile_chance = 25;
		soa_chance = 75;
		beam_chance = 0;
	}
	else
	{
		// First stage.
		projectile_chance = 90;
		soa_chance = 0;
		beam_chance = 0;
	}

	if (irand(0, 100) < projectile_chance)
	{
		SetAnim(self, ANIM_ATTACK);
		return false; //TODO: return true?
	}

	if (irand(0, 100) < beam_chance)
	{
		SetAnim(self, ANIM_ATTACK_LS);
		return false; //TODO: return true?
	}

	if (irand(0, 100) < soa_chance)
	{
		SetAnim(self, ANIM_ATTACK_SOA_BTRANS);
		return false; //TODO: return true?
	}

	if (!self->elflord_last_stage)
	{
		FindMoveTarget(self);
		SetAnim(self, ANIM_MOVE);
	}

	return false;
}

#pragma endregion

#pragma region ========================== Message handlers ==========================

static void ElfLordStandMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'elflord_stand' in original logic.
{
	SetAnim(self, ANIM_HOVER);
}

static void ElfLordRunMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'elflord_run' in original logic.
{
	SetAnim(self, ANIM_FLOAT_FORWARD);
}

static void ElfLordDeathMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'elflord_death_start' in original logic.
{
	// Turn off a beam if it's on.
	if (self->elflord_beam != NULL)
		G_FreeEdict(self->elflord_beam);

	self->health = 0;
	self->max_health = 0;
	M_ShowLifeMeter(0, 0);

	self->think = G_FreeEdict;
	self->nextthink = level.time + FRAMETIME; //mxd. Use define.
}

static void ElfLordMissileMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'elflord_soa_start' in original logic.
{
	if (!M_ValidTarget(self, self->enemy))
		return;

	gi.sound(self, CHAN_VOICE, sounds[SND_SACHARGE], 1.0f, ATTN_NORM, 0.0f);
	self->sphere_of_annihilation_charging = true;

	vec3_t forward;
	AngleVectors(self->s.angles, forward, NULL, NULL);

	SpellCastSphereOfAnnihilation(self, self->s.origin, self->s.angles, forward);
	SetAnim(self, ANIM_ATTACK_SOA_BTRANS);
}

static void ElfLordPainMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'elflord_pain' in original logic.
{
	if (irand(0, 9) == 0)
		gi.sound(self, CHAN_VOICE, sounds[irand(SND_PAIN1, SND_PAIN2)], 1.0f, ATTN_NORM, 0.0f);
}

static void ElfLordSightMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'elfLordWakeUp' in original logic.
{
	SetAnim(self, ANIM_COME_TO_LIFE);
}

#pragma endregion

#pragma region ========================== Edict callbacks ==========================

void ElfLordProjectileBlocked(edict_t* self, trace_t* trace) //mxd. Named 'elflord_projectile_blocked' in original logic.
{
	if (trace->ent != NULL) //mxd. Done below Q_stricmp()/owner checks in original logic.
	{
		if (Q_stricmp(trace->ent->classname, "elflord_projectile") == 0 || trace->ent == self->owner) //mxd. stricmp -> Q_stricmp.
			return;

		if (trace->ent->takedamage != DAMAGE_NO)
		{
			vec3_t dir;
			VectorNormalize2(self->velocity, dir);

			T_Damage(trace->ent, self->owner, self->owner, dir, trace->endpos, trace->plane.normal, irand(ELFLORD_STAR_MIN_DAMAGE, ELFLORD_STAR_MAX_DAMAGE), 0, DAMAGE_NORMAL, MOD_DIED);
		}
	}

	// Create the star explosion.
	gi.CreateEffect(NULL, FX_CWATCHER, 0, trace->endpos, "bv", CW_STAR_HIT, trace->plane.normal);

	self->think = G_FreeEdict;
	self->nextthink = level.time + FRAMETIME; //mxd. Use define.
}

void ElfLordPreThink(edict_t* self) //mxd. Named 'elflord_PreThink' in original logic.
{
	if (self->enemy != NULL && self->elflord_charge_meter >= self->max_health)
		M_ShowLifeMeter(self->health, self->max_health);

	self->next_pre_think = level.time + FRAMETIME; //mxd. Use define.
}

#pragma endregion

#pragma region ========================== Action functions ==========================

void elflord_attack(edict_t* self)
{
	if (!M_ValidTarget(self, self->enemy))
		return;

	float yaw_offset = -20.0f;

	for (int i = 0; i < 3; i++)
	{
		edict_t* projectile = G_Spawn();

		projectile->classname = "elflord_projectile";
		projectile->solid = SOLID_BBOX;
		projectile->movetype = PHYSICSTYPE_FLY;
		projectile->clipmask = MASK_SHOT;
		projectile->gravity = 0.0f;

		vec3_t forward;
		vec3_t right;
		AngleVectors(self->s.angles, forward, right, NULL);

		VectorCopy(self->s.origin, projectile->s.origin);
		VectorMA(projectile->s.origin, 48.0f, forward, projectile->s.origin);
		VectorMA(projectile->s.origin, 16.0f, right, projectile->s.origin);
		projectile->s.origin[2] += 8.0f;

		VectorCopy(projectile_mins, projectile->mins);
		VectorCopy(projectile_maxs, projectile->maxs);

		projectile->owner = self;
		projectile->svflags |= SVF_ALWAYS_SEND;

		vec3_t origin = VEC3_INIT(self->enemy->s.origin);
		M_PredictTargetPosition(self->enemy, self->enemy->velocity, skill->value * 2.0f, origin);
		origin[2] += (float)self->enemy->viewheight;

		vec3_t dir;
		VectorSubtract(origin, projectile->s.origin, dir);
		VectorNormalize(dir);

		vec3_t angles;
		vectoangles(dir, angles);
		angles[PITCH] *= -1.0f;
		angles[YAW] += yaw_offset;

		vec3_t velocity;
		AngleVectors(angles, velocity, NULL, NULL);
		VectorScale(velocity, 600.0f + skill->value * 100.0f, projectile->velocity);

		projectile->isBlocking = ElfLordProjectileBlocked;
		projectile->isBlocked = ElfLordProjectileBlocked;
		projectile->bounced = ElfLordProjectileBlocked;

		gi.linkentity(projectile);
		gi.CreateEffect(&projectile->s, FX_CWATCHER, CEF_OWNERS_ORIGIN, projectile->s.origin, "bv", CW_STAR, self->s.origin);

		yaw_offset += 20.0f;
	}

	gi.sound(self, CHAN_WEAPON, sounds[SND_PROJ1], 1.0f, ATTN_NORM, 0.0f);
}

void elflord_start_beam(edict_t* self)
{
	if (!M_ValidTarget(self, self->enemy))
		return;

	edict_t* beam = G_Spawn();

	vec3_t angles = VEC3_INIT(self->s.angles);
	angles[PITCH] *= -1.0f;

	vec3_t right;
	AngleVectors(angles, self->elflord_beam_direction, right, NULL);

	VectorMA(self->s.origin, 32.0f, self->elflord_beam_direction, self->elflord_beam_start);
	self->elflord_beam_start[2] -= 32.0f;

	beam->classname = "elflord_Beam";
	beam->solid = SOLID_NOT;
	beam->movetype = PHYSICSTYPE_NONE;
	beam->owner = self;
	beam->svflags |= SVF_ALWAYS_SEND;
	beam->pain_debounce_time = level.time + 5.0f;

	vec3_t end_pos;
	VectorMA(self->s.origin, 640.0f, self->elflord_beam_direction, end_pos);

	trace_t	trace;
	gi.trace(self->s.origin, projectile_mins, projectile_maxs, end_pos, self, MASK_SHOT, &trace);
	VectorCopy(trace.endpos, beam->s.origin);

	gi.linkentity(beam);

	gi.CreateEffect(&beam->s, FX_CWATCHER, CEF_OWNERS_ORIGIN, beam->s.origin, "bv", CW_BEAM, self->elflord_beam_start);
	gi.sound(self, CHAN_VOICE, sounds[SND_BEAM], 0.5f, ATTN_NONE, 0.0f);

	self->elflord_beam = beam;
}

void elflord_end_beam(edict_t* self)
{
	self->elflord_beam->think = G_FreeEdict;
	self->elflord_beam->nextthink = level.time + FRAMETIME; //mxd. Use define.
}

void elflord_decelerate(edict_t* self, float scaler)
{
	if (Vec3NotZero(self->velocity))
	{
		Vec3ScaleAssign(scaler, self->velocity);

		for (int i = 0; i < 3; i++)
			if (fabsf(self->velocity[i]) < 1.0f)
				self->velocity[i] = 0.0f;
	}
}

void elflord_finish_death(edict_t* self)
{
	SetAnim(self, ANIM_DIE_LOOP);
}

void elflord_charge(edict_t* self)
{
	SetAnim(self, ANIM_CHARGE);
}

void elflord_soa_end(edict_t* self)
{
	self->sphere_of_annihilation_charging = false;
	gi.sound(self, CHAN_WEAPON, sounds[SND_SAFIRE], 1.0f, ATTN_NORM, 0.0f);
	SetAnim(self, ANIM_ATTACK_SOA_END);
}

void elflord_flymove(edict_t* self, float dist)
{
	if (!M_ValidTarget(self, self->enemy))
		return;

	vec3_t dir;
	VectorSubtract(self->enemy->s.origin, self->s.origin, dir);
	self->ideal_yaw = VectorYaw(dir);

	M_ChangeYaw(self);

	vec3_t forward;
	AngleVectors(self->s.angles, forward, NULL, NULL);

	VectorMA(self->velocity, dist, forward, self->velocity);
	self->velocity[2] = self->enemy->s.origin[2] + 100.0f - self->absmin[2];

	if (!CheckAttack(self))
		MG_CheckEvade(self);
}

void elflord_soa_charge(edict_t* self)
{
	gi.sound(self, CHAN_VOICE, sounds[SND_SACHARGE], 1.0f, ATTN_NORM, 0.0f);
}

void elflord_soa_go(edict_t* self)
{
	gi.sound(self, CHAN_VOICE, sounds[SND_SAFIRE], 1.0f, ATTN_NORM, 0.0f);
	self->sphere_of_annihilation_charging = false;

	vec3_t forward;
	AngleVectors(self->s.angles, forward, NULL, NULL);

	SpellCastSphereOfAnnihilation(self, self->s.origin, self->s.angles, forward);
}

void elflord_track(edict_t* self)
{
	if (!M_ValidTarget(self, self->enemy))
	{
		// Remove the beam.
		self->elflord_beam->think = G_FreeEdict;
		self->elflord_beam->nextthink = level.time + FRAMETIME; //mxd. Use define.

		// Don't finish what we were doing.
		SetAnim(self, ANIM_HOVER);

		return;
	}

	vec3_t dir;
	VectorSubtract(self->enemy->s.origin, self->elflord_beam_start, dir);
	VectorNormalize(dir);

	vec3_t new_dir;
	VectorScale(self->elflord_beam_direction, 3.0f - skill->value * 0.5f, new_dir);
	Vec3AddAssign(dir, new_dir);
	Vec3ScaleAssign(1.0f / ((3.0f - skill->value * 0.5f) + 1.0f), new_dir);
	VectorNormalize(new_dir);

	vec3_t end_pos;
	VectorMA(self->s.origin, 640.0f, new_dir, end_pos);

	trace_t trace;
	gi.trace(self->s.origin, projectile_mins, projectile_maxs, end_pos, self, MASK_SHOT, &trace);

	if (trace.ent != NULL && trace.ent->takedamage != DAMAGE_NO)
		T_Damage(trace.ent, self, self, new_dir, trace.endpos, trace.plane.normal, irand(ELFLORD_BEAM_MIN_DAMAGE, ELFLORD_BEAM_MAX_DAMAGE), 0, DAMAGE_NORMAL, MOD_DIED);

	VectorCopy(trace.endpos, self->elflord_beam->s.origin);
	vectoangles(new_dir, self->s.angles);

	ai_charge2(self, 0.0f);

	VectorCopy(new_dir, self->elflord_beam_direction);
}

void elflord_reset_pitch(edict_t* self)
{
	self->s.angles[PITCH] = 0.0f;
}

void elflord_check_attack(edict_t* self)
{
	CheckAttack(self);
}

void elflord_try_charge(edict_t* self)
{
	if (M_ValidTarget(self, self->enemy))
		ai_charge2(self, 0.0f);
}

void elflord_update_charge_meter(edict_t* self)
{
	self->velocity[2] = 32.0f;

	if (self->elflord_charge_meter < self->max_health)
	{
		M_ShowLifeMeter(self->elflord_charge_meter, self->elflord_charge_meter);
		self->elflord_charge_meter += self->max_health / 20;
	}
}

#pragma endregion

void ElflordStaticsInit(void)
{
	static ClassResourceInfo_t res_info; //mxd. Made local static.

	classStatics[CID_ELFLORD].msgReceivers[MSG_STAND] = ElfLordStandMsgHandler;
	classStatics[CID_ELFLORD].msgReceivers[MSG_RUN] = ElfLordRunMsgHandler;
	classStatics[CID_ELFLORD].msgReceivers[MSG_FLY] = ElfLordRunMsgHandler;
	classStatics[CID_ELFLORD].msgReceivers[MSG_DEATH] = ElfLordDeathMsgHandler;
	classStatics[CID_ELFLORD].msgReceivers[MSG_MISSILE] = ElfLordMissileMsgHandler;
	classStatics[CID_ELFLORD].msgReceivers[MSG_PAIN] = ElfLordPainMsgHandler;
	classStatics[CID_ELFLORD].msgReceivers[MSG_SIGHT] = ElfLordSightMsgHandler;

	res_info.numAnims = NUM_ANIMS;
	res_info.animations = animations;
	res_info.modelIndex = gi.modelindex("models/monsters/elflord/tris.fm");

	sounds[SND_PAIN1] = gi.soundindex("monsters/elflord/pain1.wav");
	sounds[SND_PAIN2] = gi.soundindex("monsters/elflord/pain2.wav");
	sounds[SND_DIE] = gi.soundindex("monsters/elflord/death1.wav");

	// Use sphere sounds.
	sounds[SND_SACHARGE] = gi.soundindex("weapons/SphereGrow.wav");
	sounds[SND_SAFIRE] = gi.soundindex("weapons/SphereFire.wav");
	sounds[SND_SAHIT] = gi.soundindex("weapons/SphereImpact.wav");

	sounds[SND_PROJ1] = gi.soundindex("monsters/elflord/shoot.wav");
	sounds[SND_BEAM] = gi.soundindex("monsters/elflord/beam.wav");

	res_info.numSounds = NUM_SOUNDS;
	res_info.sounds = sounds;

	classStatics[CID_ELFLORD].resInfo = &res_info;
}

// QUAKED SP_monster_elflord (0.5 0.5 1) (-24 -24 -64) (24 24 16)
// Celestial Watcher.
// wakeup_target	- Monsters will fire this target the first time it wakes up (only once).
// pain_target		- Monsters will fire this target the first time it gets hurt (only once).
void SP_monster_elflord(edict_t* self)
{
	// Generic Monster Initialization.
	if (!M_FlymonsterStart(self))
		return; // Failed initialization.

	self->msgHandler = DefaultMsgHandler;

	if (self->health == 0)
		self->health = ELFLORD_HEALTH;

	self->health = MonsterHealth(self->health);
	self->max_health = self->health;

	self->mass = ELFLORD_MASS;
	self->yaw_speed = 20.0f;

	self->movetype = PHYSICSTYPE_STEP;
	self->flags |= FL_FLY;
	self->gravity = 0.0f;
	self->clipmask = MASK_MONSTERSOLID;
	self->svflags |= (SVF_ALWAYS_SEND | SVF_BOSS | SVF_TAKE_NO_IMPACT_DMG);
	self->materialtype = MAT_FLESH;
	self->solid = SOLID_BBOX;

	VectorSet(self->mins, -24.0f, -24.0f, -64.0f);
	VectorSet(self->maxs, 24.0f, 24.0f, 16.0f);

	VectorClear(self->velocity);

	self->s.modelindex = (byte)classStatics[CID_ELFLORD].resInfo->modelIndex;

	self->elflord_last_stage = false;
	self->s.skinnum = 0;
	self->s.renderfx |= RF_GLOW;
	self->s.scale = MODEL_SCALE;
	self->monsterinfo.scale = MODEL_SCALE;

	self->elflord_charge_meter = 1;
	self->monsterinfo.otherenemyname = "player";
	self->pre_think = ElfLordPreThink;
	self->next_pre_think = level.time + FRAMETIME; //mxd. Use define.

	G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);

	gi.linkentity(self);
}