//
// m_spreadermist.c
//
// Copyright 1998 Raven Software
//

#include "m_spreadermist.h"
#include "m_spreader_local.h" //mxd. For SND_BOMB.
#include "m_stats.h" //mxd. For SPREADER_GRENADE_DAMAGE, SPREADER_GRENADE_RADIUS, SPREADER_GRENADE_TIME.
#include "Matrix.h"
#include "Random.h"
#include "Vector.h"
#include "g_monster.h"

static void RadiusDamageEntThink(edict_t* self) //mxd. Named 'GenericRadiusDamageEntThink' in original logic.
{
	if (self->air_finished < level.time)
	{
		G_SetToFree(self);
		return;
	}

	// Apply my offset?
	if (self->yaw_speed && self->activator != NULL && Vec3NotZero(self->activator->s.origin))
	{
		vec3_t forward;
		vec3_t right;
		vec3_t up;
		AngleVectors(self->activator->s.angles, forward, right, up);

		VectorMA(self->activator->s.origin, self->v_angle_ofs[0], forward, self->s.origin);
		VectorMA(self->s.origin, self->v_angle_ofs[1], right, self->s.origin);
		VectorMA(self->s.origin, self->v_angle_ofs[2], up, self->s.origin);
	}

	T_DamageRadius(self, self->owner, self->owner, self->dmg_radius, (float)self->dmg, 1.0f, self->bloodType, MOD_DIED);

	self->dmg_radius -= self->speed;

	if (self->dmg_radius <= 0.0f)
	{
		G_SetToFree(self);
		return;
	}

	self->dmg -= (int)self->damage_debounce_time;

	if (self->dmg <= 0)
	{
		G_SetToFree(self);
		return;
	}

	self->nextthink = level.time + self->wait;
}

edict_t* RadiusDamageEnt(edict_t* position_owner, edict_t* damage_owner, const int damage, const float delta_damage, const float radius, const float delta_radius, const int dflags, const float lifetime, const float think_increment, const vec3_t origin, const vec3_t offset, const qboolean attach) //TODO: rename to CreateRadiusDamageEnt.
{
	assert(damage_owner != NULL);

	edict_t* self = G_Spawn();

	self->classname = "plague_mist";
	self->owner = damage_owner; // For damage.
	self->activator = position_owner;// For offsetting. //TODO: add spreadermist_attach_ent name.
	self->dmg = damage; // Starting damage.
	self->damage_debounce_time = delta_damage; // Damage amount to decrease by each think. //TODO: add int spreadermist_damage_delta name.
	self->dmg_radius = radius; // Radius of damage.
	self->speed = delta_radius; // Amount to change radius each think. //TODO: speed -> add spreadermist_dmg_radius_delta name.
	self->bloodType = dflags; // Damage flags. //TODO: add spreadermist_dflags name.
	self->air_finished = level.time + lifetime; // When to die out. //TODO: add float spreadermist_expire_time name.
	self->yaw_speed = attach; // Whether to keep that offset from the owner or just sit here. //TODO: add qboolean spreadermist_attach name.
	self->wait = max(FRAMETIME, think_increment); // How often to think (default to 10 fps).

	self->think = RadiusDamageEntThink;
	self->nextthink = level.time + self->wait;

	if (attach)
	{
		vec3_t forward;
		vec3_t right;
		vec3_t up;
		AngleVectors(self->activator->s.angles, forward, right, up);

		VectorCopy(offset, self->v_angle_ofs); // Where to keep me - offset in {f, r, u}

		VectorMA(self->activator->s.origin, self->v_angle_ofs[0], forward, self->s.origin);
		VectorMA(self->s.origin, self->v_angle_ofs[1], right, self->s.origin);
		VectorMA(self->s.origin, self->v_angle_ofs[2], up, self->s.origin);
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
	self->monsterinfo.thinkinc = 0.2f;

	self->bounced = NULL;
	self->isBlocked = NULL;

	self->think = SpreaderGrenadeThink;
	self->nextthink = level.time + 0.2f;
}

static void SpreaderGrenadeDieThink(edict_t* self) //mxd. Named 'spreader_grenade_die' in original logic.
{
	G_FreeEdict(self);
}

static void SpreaderGrenadeThink(edict_t* self) //mxd. Named 'spreader_grenade_think' in original logic.
{
	self->movetype = PHYSICSTYPE_NONE;
	self->solid = SOLID_NOT;

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
				vec3_t attack_dir;
				VectorCopy(ent->s.origin, attack_dir);
				attack_dir[2] += 5.0f;

				T_Damage(ent, self, self->owner, attack_dir, self->s.origin, vec3_origin, self->dmg, 0, DAMAGE_NO_BLOOD | DAMAGE_NO_KNOCKBACK | DAMAGE_ALIVE_ONLY | DAMAGE_AVOID_ARMOR, MOD_DIED);
			}
		}
	}

	self->nextthink = level.time + FRAMETIME;
}

static void SpreaderGrenadeBounced(edict_t* self, trace_t* trace) //mxd. Named 'spreader_grenade_bounce' in original logic.
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

static void SpreaderMistInit(edict_t* self, float x, float y, float z, float velocity_scaler) //mxd. Added to reduce code duplication.
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
	vec3_t velocity = { cosf(yaw_rad) * velocity_scaler, sinf(yaw_rad) * velocity_scaler, 0.0f };
	gi.CreateEffect(NULL, FX_PLAGUEMIST, 0, rotated_offset, "vb", velocity, 41);

	// Create the volume effect for the damage.
	const int dflags = (DAMAGE_NO_BLOOD | DAMAGE_NO_KNOCKBACK | DAMAGE_ALIVE_ONLY | DAMAGE_AVOID_ARMOR); //mxd
	RadiusDamageEnt(self, self, 1, 0.0f, 60.0f, 1.0f, dflags, 2.0f, 0.25f, rotated_offset, velocity, false); //TODO: modify damage by skill?
}

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

/*-------------------------------------------------------------------------
	spreader_toss_grenade -- this is where the grenade actually gets to 
	come to life and become; sorry about the confusion between this and 
	spreader_throw().  This is a "think func" for the 
	spreader_move_attack1 animmove_t 
-------------------------------------------------------------------------*/
void spreader_toss_grenade(edict_t *self) //self is the tosser
{
	edict_t	*grenade;
	vec3_t	start;
	vec3_t	forward, right, up;
	vec3_t	aim;
	vec3_t	offset = {12, 10, 68};
	vec3_t	dir;
	vec3_t	v;
	vec3_t	predPos;
	float	distance;
	
	if(self->monsterinfo.aiflags & AI_NO_MISSILE)
		return;//fixme: actually prevent these anims

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, offset, forward, right, start);

	grenade = G_Spawn();
	VectorCopy (start, grenade->s.origin);

	M_PredictTargetPosition( self->enemy, self->enemy->velocity, 15, predPos);

	VectorSubtract(self->s.origin, predPos, v);
	distance = VectorLength (v);
	distance *= 1.25;

	VectorCopy (forward, aim);
	vectoangles (aim, dir);
	AngleVectors (dir, forward, right, up);

	VectorScale (aim, distance, grenade->velocity);
	VectorMA (grenade->velocity, flrand(100.0F, 125.0F), up, grenade->velocity);
	
	//FIXME: Difficulty modifier here
	VectorMA (grenade->velocity, flrand(-10.0F, 10.0F), right, grenade->velocity);
	
	VectorSet (grenade->avelocity, flrand(300,600), flrand(300,600), flrand(300,600));

	grenade->movetype = PHYSICSTYPE_STEP;
	grenade->elasticity = 1;
	grenade->friction = 1;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	VectorSet (grenade->mins, -1, -1, -1);
	VectorSet (grenade->maxs, 1, 1, 1);

	grenade->s.modelindex = gi.modelindex ("models/monsters/spreader/bomb/tris.fm");
	grenade->owner = self;
	//grenade->touch = spreader_grenade_touch;
	grenade->bounced = SpreaderGrenadeBounced;
	grenade->isBlocked = SpreaderGrenadeBounced;
	self->delay = 5.0;
	//grenade->isBlocked = spreader_grenade_blocked;
	//grenade->think = spreader_grenade_explode;
	grenade->dmg = SPREADER_GRENADE_DAMAGE;
	
	//FIXME: difficulty modifier here
	grenade->dmg_radius = SPREADER_GRENADE_RADIUS;
	grenade->classname = "spreader_grenade";

	grenade->s.effects |= EF_CAMERA_NO_CLIP;
	gi.linkentity (grenade);	
}

