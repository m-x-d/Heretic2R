#include "fx.h"
#include "q_typedef.h"
#include "m_spreadermist.h"
#include "m_spreaderbomb_anim.h"
#include "g_local.h"
#include "vector.h"
#include "angles.h"
#include "random.h"
#include "matrix.h"
#include "g_volume_effect.h"
#include "g_ClassStatics.h"
#include "g_monster.h"

#define SPREADER_GRENADE_DAMAGE	(40.0)
#define SPREADER_GRENADE_RADIUS	(100.0)
#define SPREADER_GRENADE_TIME	(2.9F)


//NOTE:.this would be wrong if the sounds array in
//spreader.h changes
#define SND_BOMB 7

/*-------------------------------------------------------------------------
	VolumeEffectThink
-------------------------------------------------------------------------*/
void GenericRadiusDamageEntThink(edict_t *self)
{
	if(self->air_finished<level.time)
	{
		G_SetToFree(self);
		return;
	}

	if(self->yaw_speed)
	{//apply my offset
		if(self->activator)
		{
			if(Vec3NotZero(self->activator->s.origin))
			{
				vec3_t	forward, right, up;

				AngleVectors(self->activator->s.angles, forward, right, up);

				VectorMA(self->activator->s.origin, self->v_angle_ofs[0], forward, self->s.origin);
				VectorMA(self->s.origin, self->v_angle_ofs[1], right, self->s.origin);
				VectorMA(self->s.origin, self->v_angle_ofs[2], up, self->s.origin);
			}
		}
	}
	
	T_DamageRadius(self, self->owner, self->owner, self->dmg_radius, self->dmg, 1, self->bloodType,MOD_DIED);

	self->dmg_radius -= self->speed;
	if(self->dmg_radius<=0)
	{
		G_SetToFree(self);
		return;
	}

	self->dmg -= self->damage_debounce_time;
	if(self->dmg<=0)
	{
		G_SetToFree(self);
		return;
	}

	self->nextthink = level.time + self->wait;
}

edict_t *RadiusDamageEnt(	edict_t *posowner,//for position
							edict_t *dmgowner,//for damage credit
							int damage,//damage
							float d_damage,//d_damage
							float radius,//radius
							float d_radius,//d_radius
							int dflags,//dflags
							float lifetime,//lifetime
							float thinkIncrement,//thinktime
							vec3_t origin,//start origin
							vec3_t offset,//velocity it not attached
							float attach)//attach to posowner as offset?
{
	edict_t		*self;

	assert(dmgowner);

	self = G_Spawn();

	self->owner = dmgowner;//for damage
	self->dmg = damage;//starting damage - (int)
	self->damage_debounce_time = d_damage;//damage amount to decrease by each think
	self->dmg_radius = radius;//radius of damage
	self->speed = d_radius;//amount to change radius each think
	self->bloodType	= dflags;//damage flags
	self->classname = "plague_mist";

	self->air_finished = level.time + lifetime;//when to die out
	
	if(thinkIncrement<=0)
		self->wait = 0.1;//default to 10 fps
	else
		self->wait = thinkIncrement;//how oftern to think
	
	self->think = GenericRadiusDamageEntThink;//what to do
	self->nextthink = level.time + self->wait;//next think
	
	self->activator = posowner;//for offsetting
	if(attach)
	{
		vec3_t	forward, right, up;

		AngleVectors(self->activator->s.angles, forward, right, up);
		
		VectorCopy(offset, self->v_angle_ofs);// where to keep me- offset in {f, r, u}
		
		VectorMA(self->activator->s.origin, self->v_angle_ofs[0], forward, self->s.origin);
		VectorMA(self->s.origin, self->v_angle_ofs[1], right, self->s.origin);
		VectorMA(self->s.origin, self->v_angle_ofs[2], up, self->s.origin);
		self->yaw_speed = attach;//whether to keep that offset from the owner or just sit here
	}
	else
	{
		self->movetype = PHYSICSTYPE_FLY;
		self->gravity = 0;

		VectorCopy(offset, self->velocity);
		VectorCopy(origin, self->s.origin);
	}

	return self;
}

/*-------------------------------------------------------------------------
	spreader_grenade_explode
-------------------------------------------------------------------------*/	

static void spreader_grenade_explode (edict_t *self)
{
	vec3_t		origin;
//	self->s.fmnodeinfo[MESH_BOMBASE].flags |= FMNI_NO_DRAW;
	self->s.modelindex = 0;

	self->dmg = 1;
	VectorMA (self->s.origin, -0.02, self->velocity, origin);
	gi.CreateEffect(NULL, FX_PLAGUEMISTEXPLODE, 0, origin, "b", 3500 / 50);	
	self->nextthink = level.time + 0.2;
	self->monsterinfo.thinkinc = 0.2;
	self->think = spreader_grenade_think;
	gi.sound(self, CHAN_AUTO, classStatics[CID_SPREADER].resInfo->sounds[SND_BOMB],
		1, ATTN_IDLE, 0);
	PauseTime(self, SPREADER_GRENADE_TIME);
	
	self->bounced = NULL;
	self->isBlocked = NULL;
//	self->think = G_FreeEdict;
//	self->nextthink = level.time + self->delay;
}

/*-------------------------------------------------------------------------
	spreader_grenade_die
-------------------------------------------------------------------------*/
void spreader_grenade_die(edict_t *self)
{
	G_FreeEdict(self);
}

/*-------------------------------------------------------------------------
	spreader_grenade_think
-------------------------------------------------------------------------*/
static void spreader_grenade_think(edict_t *self)
{
	edict_t *ent;
	vec3_t temp;
	
	self->movetype = PHYSICSTYPE_NONE;
	self->solid = SOLID_NOT;

	if(self->monsterinfo.pausetime < level.time)
	{
		self->think = spreader_grenade_die;
	}
	else
	{
		ent = NULL;
		while ((ent = findradius(ent, self->s.origin, self->dmg_radius)) != NULL)
		{			
			VectorCopy(ent->s.origin, temp);
			temp[2] += 5;
			if(!Q_stricmp(ent->classname, "monster_spreader"))
				continue;
			
			if(!gi.inPVS(self->s.origin, ent->s.origin))
				continue;

			T_Damage(ent,self, self->owner, temp, self->s.origin, vec3_origin,self->dmg,
					(int)0, DAMAGE_NO_BLOOD|DAMAGE_NO_KNOCKBACK|DAMAGE_ALIVE_ONLY|DAMAGE_AVOID_ARMOR,MOD_DIED);
		}	
	}

	self->nextthink = level.time + FRAMETIME;
}

static void spreader_grenade_bounce ( edict_t *self, struct trace_s *trace )
{
	vec3_t		vel;

	if (trace->plane.normal[2] > 0.1)
	{
		VectorCopy(self->velocity, vel);
		VectorNormalize(vel);
		VectorScale(vel, 100, vel);

		vel[0] += flrand(-100, 100); 
		vel[1] += flrand(-100, 100);
		vel[2] = flrand(10, 100);

		// create the volume effect for the damage
/*
		gas = RadiusDamageEnt(self,//position
						self->owner,//damage credit
						5,//damage
						0,//d_damage
						60,//radius
						1.0,//d_radius
						DAMAGE_NO_BLOOD|DAMAGE_NO_KNOCKBACK|DAMAGE_ALIVE_ONLY|DAMAGE_AVOID_ARMOR,//dflags
						self->delay,//lifetime
						0.2,//thinktime
						self->s.origin,//startpoint
						vel,//velocity
						false);//attach to owner until gone

		gas->svflags |= SVF_ALWAYS_SEND;
		gas->s.effects=EF_MARCUS_FLAG1;
*/
		gi.CreateEffect(&self->s, FX_PLAGUEMIST, CEF_OWNERS_ORIGIN, self->s.origin, "vb", vel, 50);

		spreader_grenade_explode (self);
	}
}

/*-------------------------------------------------------------------------
	spreader_grenade_touch
-------------------------------------------------------------------------*/
static void spreader_grenade_touch (edict_t *self, edict_t *other, cplane_t *plane, 
							csurface_t *surf)
{
	vec3_t	vel;

	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}		

	VectorCopy(self->velocity, vel);
	VectorNormalize(vel);
	VectorScale(vel, 100, vel);

	vel[0] += flrand(-100, 100); 
	vel[1] += flrand(-100, 100);
	vel[2] = flrand(10, 100);

	// create the volume effect for the damage
/*	gas = RadiusDamageEnt(self,//position
					self->owner,//damage credit
					5,//damage
					0,//d_damage
					60,//radius
					1.0,//d_radius
					DAMAGE_NO_BLOOD|DAMAGE_NO_KNOCKBACK|DAMAGE_ALIVE_ONLY|DAMAGE_AVOID_ARMOR,//dflags
					self->delay,//lifetime
					0.2,//thinktime
					self->s.origin,//startpoint
					vel,//velocity
					false);//attach to owner until gone

	gas->svflags |= SVF_ALWAYS_SEND;
	gas->s.effects=EF_MARCUS_FLAG1;
*/
	gi.CreateEffect(&self->s, FX_PLAGUEMIST, CEF_OWNERS_ORIGIN, self->s.origin, "vb", vel, 50);

	spreader_grenade_explode (self);
	
	self->touch = NULL;
	self->bounced = NULL;
	self->isBlocked = NULL;
}


/*-------------------------------------------------------------------------
	spreader_mist
-------------------------------------------------------------------------*/
void spreader_mist(edict_t *self, float x, float y, float z)
{
	vec3_t		offset;
	vec3_t		rotoffset;
	vec3_t		normalized;
	vec3_t		velocity;
	float		yaw;
	matrix3_t	mat;

	if(self->monsterinfo.aiflags & AI_NO_MELEE)
		return;//fixme: actually prevent these anims

	// Converts degrees to radians for use with trig and matrix functions
	yaw = self->s.angles[YAW] * ANGLE_TO_RAD;	

	// Sets offset presuming yaw of zero
	VectorSet(offset, x, y, z);		

	// Creates a rotation matrix to rotate the point about the z axis
	CreateYawMatrix(mat, yaw);		

	// Rotates point about local z axis
	Matrix3MultByVec3(mat, offset, rotoffset);	

	// Get normalized offset
	VectorCopy(rotoffset, normalized);	
	normalized[2] = 0.0F;
	VectorNormalize(normalized);

	// Add offset to owners origin
	Vec3AddAssign(self->s.origin, rotoffset);	

	// Get direction vector scaled by speed
	VectorSet(velocity, cos(yaw) * 200.0F, sin(yaw) * 200.0F, 0);	
		
	gi.CreateEffect(NULL, FX_PLAGUEMIST, 0, rotoffset, "vb", velocity, 2050 / 50);

	// create the volume effect for the damage
	RadiusDamageEnt(self,//owner
					self,//damage-owner
					1,//damage
					0,//d_damage
					60,//radius
					1.0,//d_radius
					DAMAGE_NO_BLOOD|DAMAGE_NO_KNOCKBACK|DAMAGE_ALIVE_ONLY|DAMAGE_AVOID_ARMOR,//dflags
					2.0,//lifetime
					0.25,//thinkincr
					rotoffset,//origin
					velocity,//velocity or offset
					false);//offset from owner?

	self->monsterinfo.attack_finished = level.time + (3 - skill->value) + flrand(0.5, 1);
}


void spreader_mist_fast(edict_t *self, float x, float y, float z)
{
	vec3_t		offset;
	vec3_t		rotoffset;
	vec3_t		normalized;
	vec3_t		velocity;
	float		yaw;
	matrix3_t	mat;

	// Converts degrees to radians for use with trig and matrix functions
	yaw = self->s.angles[YAW] * ANGLE_TO_RAD;	

	// Sets offset presuming yaw of zero
	VectorSet(offset, x, y, z);		

	// Creates a rotation matrix to rotate the point about the z axis
	CreateYawMatrix(mat, yaw);		

	// Rotates point about local z axis
	Matrix3MultByVec3(mat, offset, rotoffset);	

	// Get normalized offset
	VectorCopy(rotoffset, normalized);	
	normalized[2] = 0.0F;
	VectorNormalize(normalized);

	// Add offset to owners origin
	Vec3AddAssign(self->s.origin, rotoffset);	

	// Get direction vector scaled by speed
	VectorSet(velocity, cos(yaw) * 300.0F, sin(yaw) * 300.0F, 0);	
		
	gi.CreateEffect(NULL, FX_PLAGUEMIST, 0, rotoffset, "vb", velocity, 2050 / 50);

	// create the volume effect for the damage
	
	//FIXME: Skill modifier here
	RadiusDamageEnt(self,//owner
					self,//damage-owner
					1,//damage
					0,//d_damage
					60,//radius
					1.0,//d_radius
					DAMAGE_NO_BLOOD|DAMAGE_NO_KNOCKBACK|DAMAGE_ALIVE_ONLY|DAMAGE_AVOID_ARMOR,//dflags
					2.0,//lifetime
					0.25,//thinkincr
					rotoffset,//origin
					velocity,//velocity or offset
					false);//offset from owner?
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
	grenade->bounced = spreader_grenade_bounce;
	grenade->isBlocked = spreader_grenade_bounce;
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

