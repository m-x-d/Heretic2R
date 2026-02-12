//
// m_spreadermist.c
//
// Copyright 1998 Raven Software
//

#include "m_spreadermist.h"
#include "m_spreader_shared.h" //mxd. For SND_BOMB.
#include "m_stats.h" //mxd. For SPREADER_GRENADE_DAMAGE, SPREADER_GRENADE_RADIUS, SPREADER_GRENADE_TIME.
#include "Matrix.h"
#include "Random.h"
#include "Vector.h"
#include "g_monster.h"

#pragma region ========================== Utility functions =========================

static void RadiusDamageEntUpdateAttachPosition(edict_t* self) //mxd. Added to reduce code duplication.
{
	vec3_t forward;
	vec3_t right;
	vec3_t up;
	AngleVectors(self->spreadermist_attach_ent->s.angles, forward, right, up);

	VectorMA(self->spreadermist_attach_ent->s.origin, self->v_angle_ofs[0], forward, self->s.origin);
	VectorMA(self->s.origin, self->v_angle_ofs[1], right, self->s.origin);
	VectorMA(self->s.origin, self->v_angle_ofs[2], up, self->s.origin);
}

edict_t* CreateRadiusDamageEnt(edict_t* position_owner, edict_t* damage_owner, const int damage, const int delta_damage, const float radius, const float delta_radius, const int dflags, const float lifetime, const float think_increment, const vec3_t origin, const vec3_t offset, const qboolean attach) //mxd. Named 'RadiusDamageEnt' in original logic.
{
	assert(damage_owner != NULL);

	edict_t* self = G_Spawn();

	self->classname = "plague_mist";
	self->owner = damage_owner; // For damage.
	self->spreadermist_attach_ent = position_owner;// For offsetting.
	self->dmg = damage; // Starting damage.
	self->spreadermist_damage_delta = delta_damage; // Damage amount to decrease by each think.
	self->dmg_radius = radius; // Radius of damage.
	self->spreadermist_dmg_radius_delta = delta_radius; // Amount to change radius each think.
	self->spreadermist_dflags = dflags; // Damage flags.
	self->spreadermist_expire_time = level.time + lifetime; // When to die out.
	self->spreadermist_attach = attach; // Whether to keep that offset from the owner or just sit here.
	self->wait = max(FRAMETIME, think_increment); // How often to think (default to 10 fps).

	self->think = RadiusDamageEntThink;
	self->nextthink = level.time + self->wait;

	if (attach)
	{
		VectorCopy(offset, self->v_angle_ofs); // Where to keep me - offset in {f, r, u}
		RadiusDamageEntUpdateAttachPosition(self); //mxd
	}
	else
	{
		self->movetype = PHYSICSTYPE_FLY;
		self->gravity = 0.0f;

		VectorCopy(offset, self->velocity);
		VectorCopy(origin, self->s.origin);
	}

	return self;
}

static void SpreaderGrenadeExplode(edict_t* self) //mxd. Named 'spreader_grenade_explode' in original logic.
{
	self->s.modelindex = 0;
	self->dmg = 1;

	vec3_t origin;
	VectorMA(self->s.origin, -0.02f, self->velocity, origin);

	gi.CreateEffect(NULL, FX_PLAGUEMISTEXPLODE, 0, origin, "b", 70);
	gi.sound(self, CHAN_AUTO, classStatics[CID_SPREADER].resInfo->sounds[SND_BOMB], 1.0f, ATTN_IDLE, 0.0f);

	self->monsterinfo.pausetime = level.time + SPREADER_GRENADE_TIME; //mxd. Inlined PauseTime() logic.
	self->monsterinfo.thinkinc = 0.2f; //TODO: unused?

	self->bounced = NULL;
	self->isBlocked = NULL;

	self->think = SpreaderGrenadeExplodeThink;
	self->nextthink = level.time + 0.2f;
}

static void SpreaderMistInit(edict_t* self, const float x, const float y, const float z, const float velocity_scaler) //mxd. Added to reduce code duplication.
{
	// Converts degrees to radians for use with trig and matrix functions.
	const float yaw_rad = self->s.angles[YAW] * ANGLE_TO_RAD;

	// Creates a rotation matrix to rotate the point about the z axis.
	matrix3_t mat;
	CreateYawMatrix(mat, yaw_rad);

	// Rotates point about local z axis.
	const vec3_t offset = { x, y, z }; // Set offset presuming yaw of zero.
	vec3_t rotated_offset;
	Matrix3MultByVec3(mat, offset, rotated_offset);

	// Add offset to owners origin.
	Vec3AddAssign(self->s.origin, rotated_offset);

	// Get direction vector scaled by speed.
	const vec3_t velocity = { cosf(yaw_rad) * velocity_scaler, sinf(yaw_rad) * velocity_scaler, 0.0f };
	gi.CreateEffect(NULL, FX_PLAGUEMIST, 0, rotated_offset, "vb", velocity, 41);

	// Create the volume effect for the damage.
	const int dflags = (DAMAGE_NO_BLOOD | DAMAGE_NO_KNOCKBACK | DAMAGE_ALIVE_ONLY | DAMAGE_AVOID_ARMOR); //mxd
	CreateRadiusDamageEnt(self, self, 1, 0, 60.0f, 1.0f, dflags, 2.0f, 0.25f, rotated_offset, velocity, false); //TODO: modify damage by skill?
}

#pragma endregion

#pragma region ========================== Edict callbacks ===========================

void RadiusDamageEntThink(edict_t* self) //mxd. Named 'GenericRadiusDamageEntThink' in original logic.
{
	if (self->spreadermist_expire_time < level.time)
	{
		G_SetToFree(self);
		return;
	}

	// Apply my offset?
	if (self->spreadermist_attach && self->spreadermist_attach_ent != NULL && Vec3NotZero(self->spreadermist_attach_ent->s.origin))
		RadiusDamageEntUpdateAttachPosition(self); //mxd

	T_DamageRadius(self, self->owner, self->owner, self->dmg_radius, (float)self->dmg, 1.0f, self->spreadermist_dflags, MOD_DIED);

	self->dmg_radius -= self->spreadermist_dmg_radius_delta;

	if (self->dmg_radius <= 0.0f)
	{
		G_SetToFree(self);
		return;
	}

	self->dmg -= self->spreadermist_damage_delta;

	if (self->dmg <= 0)
	{
		G_SetToFree(self);
		return;
	}

	self->nextthink = level.time + self->wait;
}

void SpreaderGrenadeAppearThink(edict_t* self) //mxd
{
	self->s.scale = self->owner->s.scale;
	self->think = NULL;
}

void SpreaderGrenadeDieThink(edict_t* self) //mxd. Named 'spreader_grenade_die' in original logic. //TODO: replace with G_FreeEdict()?
{
	G_FreeEdict(self);
}

void SpreaderGrenadeExplodeThink(edict_t* self) //mxd. Named 'spreader_grenade_think' in original logic.
{
	if (self->solid != SOLID_NOT)
	{
		self->movetype = PHYSICSTYPE_NONE;
		self->solid = SOLID_NOT;

		gi.linkentity(self); //mxd. Not called in original logic (fixes grenade staying solid client-side).
	}

	if (self->monsterinfo.pausetime < level.time)
	{
		self->think = SpreaderGrenadeDieThink;
	}
	else
	{
		edict_t* ent = NULL;
		while ((ent = FindInRadius(ent, self->s.origin, self->dmg_radius)) != NULL)
		{
			if (ent->classID != CID_SPREADER && gi.inPVS(self->s.origin, ent->s.origin)) //mxd. classname -> classID check.
			{
				const vec3_t attack_dir = VEC3_INITA(ent->s.origin, 0.0f, 0.0f, 5.0f);
				T_Damage(ent, self, self->owner, attack_dir, self->s.origin, vec3_origin, self->dmg, 0, DAMAGE_NO_BLOOD | DAMAGE_NO_KNOCKBACK | DAMAGE_ALIVE_ONLY | DAMAGE_AVOID_ARMOR, MOD_DIED);
			}
		}
	}

	self->nextthink = level.time + FRAMETIME;
}

void SpreaderGrenadeBounced(edict_t* self, trace_t* trace) //mxd. Named 'spreader_grenade_bounce' in original logic.
{
	if (trace->plane.normal[2] <= 0.1f)
		return;

	vec3_t vel;
	VectorNormalize2(self->velocity, vel);
	Vec3ScaleAssign(100.0f, vel);

	vel[0] += flrand(-100.0f, 100.0f);
	vel[1] += flrand(-100.0f, 100.0f);
	vel[2] = flrand(10.0f, 100.0f);

	// Create the volume effect for the damage.
	gi.CreateEffect(&self->s, FX_PLAGUEMIST, CEF_OWNERS_ORIGIN, self->s.origin, "vb", vel, 50);
	SpreaderGrenadeExplode(self);
}

#pragma endregion

#pragma region ========================== Action functions ==========================

void spreader_mist(edict_t* self, float x, float y, float z)
{
	if (!(self->monsterinfo.aiflags & AI_NO_MELEE)) //FIXME: actually prevent these anims.
	{
		SpreaderMistInit(self, x, y, z, 200.0f); //mxd
		self->monsterinfo.attack_finished = level.time + (3.0f - skill->value) + flrand(0.5f, 1.0f);
	}
}

void spreader_mist_fast(edict_t* self, float x, float y, float z)
{
	SpreaderMistInit(self, x, y, z, 300.0f); //mxd
}

// This is where the grenade actually gets to come to life and become.
// Sorry about the confusion between this and spreader_throw().
// This is a "think func" for the spreader_move_attack1 animmove_t.
void spreader_toss_grenade(edict_t* self) // Self is the tosser.
{
	if (self->monsterinfo.aiflags & AI_NO_MISSILE)
		return; //FIXME: actually prevent these anims.

	vec3_t forward;
	vec3_t right;
	vec3_t up;
	AngleVectors(self->s.angles, forward, right, up);

	vec3_t start;
	vec3_t offset = { 10.0f, 6.0f, 27.0f }; //mxd. Adjusted offset to better match with spreader's model.
	Vec3ScaleAssign(self->s.scale, offset); //mxd. Apply self scale.
	G_ProjectSource(self->s.origin, offset, forward, right, start);

	edict_t* grenade = G_Spawn();
	VectorCopy(start, grenade->s.origin);

	vec3_t pred_pos;
	M_PredictTargetPosition(self->enemy, self->enemy->velocity, 15.0f, pred_pos);

	vec3_t diff;
	VectorSubtract(self->s.origin, pred_pos, diff);
	const float distance = VectorLength(diff) * 1.25f;

	VectorScale(forward, distance, grenade->velocity);
	VectorMA(grenade->velocity, flrand(100.0f, 125.0f), up, grenade->velocity);
	VectorMA(grenade->velocity, flrand(-10.0f, 10.0f), right, grenade->velocity); //TODO: scale velocity by difficulty?

	for (int i = 0; i < 3; i++)
		grenade->avelocity[i] = flrand(300.0f, 600.0f) * (float)(Q_sign(irand(-1, 0))); //mxd. Randomly pick negative value.

	grenade->owner = self;
	grenade->classname = "spreader_grenade";
	grenade->s.modelindex = (byte)gi.modelindex("models/monsters/spreader/bomb/tris.fm");
	grenade->s.scale = 0.01f; //mxd. Hide 1-st frame (so grenade is already in motion when it becomes visible next frame).
	grenade->s.effects |= EF_CAMERA_NO_CLIP;
	grenade->movetype = PHYSICSTYPE_STEP;
	grenade->elasticity = 1.0f;
	grenade->friction = 1.0f;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	grenade->dmg = SPREADER_GRENADE_DAMAGE; //TODO: difficulty modifier here.
	grenade->dmg_radius = SPREADER_GRENADE_RADIUS; //TODO: difficulty modifier here.

	VectorSet(grenade->mins, -1.0f, -1.0f, -1.0f);
	VectorSet(grenade->maxs, 1.0f, 1.0f, 1.0f);

	grenade->bounced = SpreaderGrenadeBounced;
	grenade->isBlocked = SpreaderGrenadeBounced;

	//mxd. Make grenade appear next frame.
	grenade->think = SpreaderGrenadeAppearThink;
	grenade->nextthink = level.time + FRAMETIME;

	gi.linkentity(grenade);

	self->delay = 5.0f;
}

#pragma endregion