//
// spl_flyingfist.c
//
// Heretic II
// Copyright 1998 Raven Software
//

#include "g_local.h"
#include "fx.h"
#include "vector.h"
#include "random.h"
#include "decals.h"
#include "m_tcheckrik.h"
#include "Utilities.h"
#include "m_stats.h"
#include "g_playstats.h"


extern void AlertMonsters (edict_t *self, edict_t *enemy, float lifetime, qboolean sound_alert);
static void InsectStaffThink(edict_t *self);
static void InsectStaffTouch(edict_t *self,edict_t *Other,cplane_t *Plane,csurface_t *Surface);

void create_insect_staff_bolt(edict_t *InsectStaff);

// ************************************************************************************************
// InsectStaffThink
// ************************************************************************************************

static void InsectStaffThink(edict_t *self)
{
	vec3_t	Forward;

	// Grow myself a bit.

	self->s.scale=1.0;

	// Do autotargeting.

	if(self->enemy)
	{
		// I have a target (pointed at by self->enemy) so aim myself at it.

		VectorCopy(self->s.origin,Forward);

		VectorSubtract(self->enemy->s.origin,Forward,Forward);

		Forward[0]+=(self->enemy->mins[0]+self->enemy->maxs[0])/2.0;
		Forward[1]+=(self->enemy->mins[1]+self->enemy->maxs[1])/2.0;
		Forward[2]+=(self->enemy->mins[2]+self->enemy->maxs[2])/2.0;

		VectorNormalize(Forward);
	}
	else
	{
		// I don't have a target so just I'll just fly straight forward.

		AngleVectors(self->s.angles,Forward,0,0);
	}

	// Give myself a velocity of 500 in my forward direction.
	
	VectorScale(Forward,INSECT_STAFF_AIMED_SPEED,self->velocity);

	self->think = NULL;
}

// ************************************************************************************************
// InsectStaffTouch
// ************************************************************************************************

static void InsectStaffTouch(edict_t *self,edict_t *Other,cplane_t *Plane,csurface_t *Surface)
{
	vec3_t		Origin;
	byte		makescorch;
	edict_t	*InsectStaff;

	if(Surface&&(Surface->flags&SURF_SKY))
	{
		SkyFly(self);
		return;
	}

	if(EntReflecting(Other, true, true))
	{
		InsectStaff=G_Spawn();

		create_insect_staff_bolt(InsectStaff);

		InsectStaff->owner = self->owner;
		InsectStaff->enemy = NULL;
		InsectStaff->s.scale= self->s.scale;

		VectorCopy(self->s.origin, InsectStaff->s.origin);
		Create_rand_relect_vect(self->velocity, InsectStaff->velocity);
		VectorCopy(InsectStaff->velocity, InsectStaff->movedir);
		Vec3ScaleAssign(INSECT_STAFF_SPEED,InsectStaff->velocity);
		vectoangles(InsectStaff->velocity, InsectStaff->s.angles);

		G_LinkMissile(InsectStaff); 

		gi.CreateEffect(&InsectStaff->s,
					FX_I_EFFECTS,
					CEF_OWNERS_ORIGIN,
					vec3_origin,
					"bv",
					FX_I_SP_MSL_HIT,
					vec3_origin);

		G_SetToFree(self);

		return;
	}

	// Calculate the position for the explosion entity.

	VectorMA(self->s.origin,-0.02,self->velocity,Origin);


	if(Other->takedamage)
	{
		T_Damage(Other, self, self->owner, self->movedir, self->s.origin, Plane->normal, self->dmg, 0, DAMAGE_SPELL, MOD_DIED);
	}
	else
	{
		// Back off the origin for the damage a bit. We are a point and this will
		// help fix hitting base of a stair and not hurting a guy on next step up.
		VectorMA(self->s.origin,-8.0,self->movedir,self->s.origin);
	}

	// Attempt to apply a scorchmark decal to the thing I hit.
	makescorch = 0;
	if(IsDecalApplicable(self,Other,self->s.origin,Surface,Plane,NULL))
	{
		makescorch = CEF_FLAG6;
	}

	if(!self->count)
	{
		gi.CreateEffect(NULL,
					FX_I_EFFECTS,
					makescorch,
					self->s.origin,
					"bv",
					FX_I_ST_MSL_HIT,
					self->movedir);

	}
	else
	{
		gi.sound(self, CHAN_BODY, gi.soundindex("monsters/imp/fbfire.wav"), 1, ATTN_NORM, 0);

		gi.CreateEffect(&self->s,
					FX_M_EFFECTS,
					CEF_OWNERS_ORIGIN,
					self->s.origin,
					"bv",
					FX_IMP_FBEXPL,
					vec3_origin);

	}

	G_SetToFree(self);
}

// create the guts of the insect staff bolt
void create_insect_staff_bolt(edict_t *InsectStaff)
{
	InsectStaff->s.effects = EF_NODRAW_ALWAYS_SEND|EF_CAMERA_NO_CLIP;
	InsectStaff->movetype = MOVETYPE_FLYMISSILE;
	InsectStaff->solid = SOLID_BBOX;
	InsectStaff->classname = "Spell_InsectStaff";
	InsectStaff->touch = InsectStaffTouch;
 	InsectStaff->dmg = irand(TC_FEMALE_DMG_HACK_MIN, TC_FEMALE_DMG_HACK_MAX) * (skill->value + 1)/3;
	InsectStaff->clipmask = MASK_MONSTERSOLID|MASK_SHOT;
	VectorClear(InsectStaff->mins);
	VectorClear(InsectStaff->maxs);
	InsectStaff->think = InsectStaffThink;
	InsectStaff->nextthink = level.time+0.1;
}

// ************************************************************************************************
// SpellCastInsectStaff
// ************************************************************************************************

void SpellCastInsectStaff(edict_t *Caster,vec3_t StartPos,vec3_t AimAngles,vec3_t AimDir,qboolean power)
{
	vec3_t	forward;
	edict_t	*InsectStaff;

	InsectStaff = G_Spawn();

	VectorCopy(StartPos, InsectStaff->s.origin);	
	VectorCopy(AimAngles, InsectStaff->s.angles);
	VectorScale(AimDir, INSECT_STAFF_SPEED, InsectStaff->velocity);
	AngleVectors(AimAngles, forward, NULL,NULL);
	VectorCopy(forward, InsectStaff->movedir);

	create_insect_staff_bolt(InsectStaff);

	InsectStaff->s.scale = 0.1;
	InsectStaff->owner = Caster;
	InsectStaff->enemy = Caster->enemy;

	G_LinkMissile(InsectStaff); 

	if(power)
	{
		Vec3ScaleAssign(2, InsectStaff->velocity);
		InsectStaff->dmg *= 2;
		InsectStaff->count = 1;
		gi.CreateEffect(&InsectStaff->s,
					FX_M_EFFECTS,//just so I don't have to make a new FX_ id
					CEF_OWNERS_ORIGIN,
					NULL,
					"bv",
					FX_IMP_FIRE,
					InsectStaff->velocity);
	}
	else
	{
		InsectStaff->count = 0;
		gi.CreateEffect(&InsectStaff->s,
						FX_I_EFFECTS,
						CEF_OWNERS_ORIGIN,
						vec3_origin,
						"bv",
						FX_I_STAFF,
						vec3_origin);
	}
}



#define GLOBE_MAX_SCALE				1.8
#define GLOBE_SCALE_RANGE				0.8
#define GLOBE_FLY_SPEED				600.0

static void GlobeOfOuchinessGrowThink(edict_t *self);

// ****************************************************************************
// GlobeOfOuchinessGrowThink
// ****************************************************************************

static void GlobeOfOuchinessGrowThink(edict_t *self)
{
	vec3_t	Forward,Up;

	if(self->owner->s.effects&EF_DISABLE_EXTRA_FX)
	{
		gi.RemoveEffects(&self->s,0);
		G_FreeEdict(self);
		return;
	}
		
	if (self->owner->client)
		AngleVectors(self->owner->client->aimangles,Forward,NULL,Up);
	else
		AngleVectors(self->owner->s.angles,Forward,NULL,Up);

	// whether or not I have been released. Would like a dedicated value in the 'edict_t' but this
	// is unlikely to happen, sooooo...

	if(!self->owner->damage_debounce_time)
	{
//		self->svflags |= SVF_NOCLIENT;

		self->count+=irand(1,2);
		
		if((self->count>10)&&(self->s.scale<GLOBE_MAX_SCALE))
		{
			if(self->count>20)
			{
				self->s.scale-=0.01;
			}
			else
			{
				self->s.scale+=0.1;
			}

			if(self->count>25)
			{
				self->count&=3;
			}
		}

		self->velocity[0]=8.0*((self->owner->s.origin[0]+Forward[0]*22.0+flrand(-2.0F,2.0F))-self->s.origin[0]);
		self->velocity[1]=8.0*((self->owner->s.origin[1]+Forward[1]*22.0+flrand(-2.0F,2.0F))-self->s.origin[1]);
		self->velocity[2]=8.0*((self->owner->s.origin[2]+Up[2]*10.0)-self->s.origin[2]);

		self->nextthink=level.time+0.1;
	}
	else
	{
		self->owner->damage_debounce_time = true;

		G_FreeEdict(self);
		return;

	}
}

// ****************************************************************************
// SpellCastGlobeOfOuchiness
// ****************************************************************************

void SpellCastGlobeOfOuchiness(edict_t *Caster,vec3_t StartPos,vec3_t AimAngles,vec3_t AimDir)
{
	edict_t	*Globe;
	vec3_t	tempvec;

	// Spawn the globe of annihilation as an invisible entity (i.e. modelindex=0).

	Globe=G_Spawn();

	VectorCopy(Caster->s.origin,Globe->s.origin);	
	Globe->s.origin[0]+=AimDir[0]*20.0;
	Globe->s.origin[1]+=AimDir[1]*20.0;
	Globe->s.origin[2]+=Caster->viewheight-5.0;
	
	vectoangles(AimAngles,Globe->s.angles);

	Globe->avelocity[YAW]=100.0;
	Globe->avelocity[ROLL]=100.0;

	// whether or not I have been released. Would like a dedicated value in the 'edict_t' but this
	// is unlikely to happen, sooooo...

	Globe->svflags |= SVF_ALWAYS_SEND;
	Globe->s.effects |= EF_ALWAYS_ADD_EFFECTS|EF_MARCUS_FLAG1|EF_CAMERA_NO_CLIP;
	Globe->owner=Caster;
	Globe->classname="Spell_GlobeOfOuchiness";
	Globe->dmg=0;
	Globe->s.scale=1.0;
	Globe->enemy=Caster->enemy;
	Globe->count=0;
	Globe->clipmask=MASK_MONSTERSOLID;
	Globe->movetype = PHYSICSTYPE_FLY;
	Globe->solid=SOLID_NOT;
	Globe->nextthink=level.time+0.1;
	Globe->think=GlobeOfOuchinessGrowThink;

	G_LinkMissile(Globe); 

	VectorSet(tempvec, (float)(Caster->s.number), 0, 0);

	gi.CreateEffect(&Globe->s,
					FX_I_EFFECTS,
					CEF_OWNERS_ORIGIN,
					vec3_origin,
					"bv",
					FX_I_GLOBE,
					tempvec);

	gi.CreateEffect(&Globe->s,
					FX_I_EFFECTS,
					CEF_OWNERS_ORIGIN,
					vec3_origin,
					"bv",
					FX_I_GLOW,
					tempvec);

//	gi.sound(Globe,CHAN_WEAPON,gi.soundindex("weapons/GlobeOfOuchinessGrow.wav"),1,ATTN_NORM,0);
}

//Spear Projectiles
static void SpearProjTouch(edict_t *self,edict_t *Other,cplane_t *Plane,csurface_t *Surface);

// Radius of zero seems to prevent collision between bolts

#define SPEARPROJ_RADIUS		0.0
#define SPEARPROJ_SPEED			600.0

// guts of creating a spearproj
void create_spearproj(edict_t *spearproj)
{

	spearproj->s.effects |= EF_ALWAYS_ADD_EFFECTS|EF_CAMERA_NO_CLIP;
	spearproj->svflags |= SVF_ALWAYS_SEND;
	spearproj->movetype = MOVETYPE_FLYMISSILE;

	VectorSet(spearproj->mins, -SPEARPROJ_RADIUS, -SPEARPROJ_RADIUS, -SPEARPROJ_RADIUS);
	VectorSet(spearproj->maxs, SPEARPROJ_RADIUS, SPEARPROJ_RADIUS, SPEARPROJ_RADIUS);

	spearproj->solid = SOLID_BBOX;
	spearproj->clipmask = MASK_SHOT;
	spearproj->touch = SpearProjTouch;

	if(spearproj->count)
		spearproj->dmg = irand(TC_DMG_YSPEAR_MIN, TC_DMG_YSPEAR_MAX);
	else if(skill->value > 1)
		spearproj->dmg = TC_DMG_SPEAR_MAX;
	else if(!skill->value)
		spearproj->dmg = TC_DMG_SPEAR_MIN;
	else
		spearproj->dmg = irand(TC_DMG_SPEAR_MIN, TC_DMG_SPEAR_MAX);

	spearproj->classname = "Spell_SpearProj";
}


edict_t *SpearProjReflect(edict_t *self, edict_t *other, vec3_t vel)
{
	edict_t *spearproj;

	spearproj = G_Spawn();

	VectorCopy(self->s.origin, spearproj->s.origin);
	VectorCopy(vel, spearproj->velocity);
	create_spearproj(spearproj);
	VectorNormalize2(spearproj->velocity, spearproj->movedir);
	AnglesFromDir(spearproj->movedir, spearproj->s.angles);
	spearproj->reflect_debounce_time = self->reflect_debounce_time -1;
	spearproj->reflected_time=self->reflected_time;
	G_LinkMissile(spearproj); 

	spearproj->health = self->health;
	spearproj->think = self->think;
	spearproj->owner = other;
	spearproj->enemy = NULL;
	spearproj->nextthink = self->nextthink;
	spearproj->ideal_yaw = self->ideal_yaw;
	spearproj->random = self->random;
	spearproj->delay = self->delay;
	spearproj->count = self->count;
	spearproj->red_rain_count = self->red_rain_count;
	if (spearproj->health == 1)
	{
		gi.CreateEffect(&spearproj->s,
			FX_I_EFFECTS,
			CEF_OWNERS_ORIGIN,
			NULL,
			"bv",
			FX_I_SPEAR2,
			vec3_origin);
	}
	else
	{
		gi.CreateEffect(&spearproj->s,
			FX_I_EFFECTS,
			CEF_OWNERS_ORIGIN,
			vec3_origin,
			"bv",
			FX_I_SPEAR,
			spearproj->velocity);
	}

	// kill the existing missile, since its a pain in the ass to modify it so the physics won't screw it. 
	G_SetToFree(self);

	return(spearproj);
}


// ****************************************************************************
// SpearProjTouch
// ****************************************************************************

static void SpearProjTouch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surface)
{
	byte	makeScorch = 0;

	// did we hit the sky ? 
	if(surface && (surface->flags & SURF_SKY))
	{
		SkyFly(self);
		return;
	}

	// did we hit someone where reflection is functional ?
	if (self->reflect_debounce_time)
	{
		if(EntReflecting(other, true, true))
		{
			Create_rand_relect_vect(self->velocity, self->velocity);
			Vec3ScaleAssign(SPEARPROJ_SPEED/2, self->velocity);
			SpearProjReflect(self, other, self->velocity);

			return;
		}
	}

	if(other->takedamage)
	{
		if(level.fighting_beast)
		{
			if(other->classID == CID_TBEAST)
			{	
				if(other->enemy != self->owner)
				{
					other->enemy = self->owner;
				}
			}
			else if(other->classID == CID_BBRUSH)
			{
				self->dmg = 0;
				VectorMA(self->s.origin, -4.0, self->movedir, self->s.origin);
			}
		}

		if(self->dmg)//HACK = so can't collapse trial beast bridge
			T_Damage(other, self, self->owner, self->movedir, self->s.origin, plane->normal, self->dmg, 0, DAMAGE_SPELL,MOD_SPEAR);
	}
	else
	{
		// Back off the origin for the damage a bit. We are a point and this will
		// help fix hitting base of a stair and not hurting a guy on next step up.
		VectorMA(self->s.origin, -4.0, self->movedir, self->s.origin);
	}

	makeScorch = 0;
	if(IsDecalApplicable(self, other, self->s.origin, surface, plane, NULL))
	{
		makeScorch = CEF_FLAG6;
	}

	if(self->count)
	{
		gi.CreateEffect(&self->s,
			FX_I_EFFECTS,
			makeScorch,
			vec3_origin,
			"bv",
			FX_I_SP_MSL_HIT2,
			self->movedir);
	}
	else
	{
		gi.CreateEffect(&self->s,
			FX_I_EFFECTS,
			makeScorch,
			vec3_origin,
			"bv",
			FX_I_SP_MSL_HIT,
			self->movedir);
	}

	G_SetToFree(self);
}

/*
====================================================
void Veer(float amount)
MG
This function will make a projectile
wander from it's course in a random
manner.  It does not actually directly
use the .veer value, you must send the
veer amount value to the function as
a parameter.  But this allows it to
be used in other ways (call it once,
etc.)  So you can call it by using
Veer(self.veer) or Veer(random()*300)
or Veer([any number]), etc.
=====================================================
*/
void projectile_veer(edict_t *self, float amount);
void projectile_homethink (edict_t *self);

void yellowjacket_proj_think (edict_t *self)
{
	vec3_t		vdir, edir;
	//No enemy, stop tracking
	if (!self->enemy)
	{
		self->think = NULL;
		return;
	}

	VectorCopy(self->velocity, vdir);
	VectorNormalize(vdir);
	VectorSubtract(self->enemy->s.origin, self->s.origin, edir);
	VectorNormalize(edir);

	if(DotProduct(edir, vdir) > 0 && irand(2, 24) > self->count)
		projectile_homethink(self);

	self->count++;

	if(self->random < 100)
		self->random += 10;

	self->nextthink = level.time + 0.1;
}
// ****************************************************************************
// SpellCastSpearProj
// ****************************************************************************


void SpellCastInsectSpear(edict_t *caster, vec3_t StartPos, vec3_t AimAngles, int offset)
{
	edict_t	*spearproj;
	trace_t trace;
	vec3_t	endpos, forward, right, up, dir;
	float	dist;

	// Spawn the magic-missile.

	if(!caster->enemy)
		return;

	spearproj = G_Spawn();

	VectorCopy(StartPos, spearproj->s.origin);
	VectorSubtract(caster->enemy->s.origin, StartPos, dir);
	dist = VectorLength(dir);

	if(offset && dist > 128)
	{
		AngleVectors(AimAngles, forward, right, up);
		switch(offset)
		{
		default:
		case 1:
			VectorAverage(forward, right, forward);
			break;
		
		case 2:
			Vec3ScaleAssign(-1, right);
			VectorAverage(forward, right, forward);
			break;
		
		case 3:
			VectorAverage(forward, up, forward);
			break;
		}
		VectorScale(forward, SPEARPROJ_SPEED, spearproj->velocity);
	}
	else
	{
		//Check ahead first to see if it's going to hit anything at this angle
		AngleVectors(AimAngles, forward, NULL, NULL);
		VectorMA(StartPos, SPEARPROJ_SPEED, forward, endpos);
		gi.trace(StartPos, vec3_origin, vec3_origin, endpos, caster, MASK_MONSTERSOLID,&trace);
		if(trace.ent && ok_to_autotarget(caster, trace.ent))
		{//already going to hit a valid target at this angle- so don't autotarget
			VectorScale(forward, SPEARPROJ_SPEED, spearproj->velocity);
		}
		else
		{//autotarget current enemy
			GetAimVelocity(caster->enemy, spearproj->s.origin, SPEARPROJ_SPEED, AimAngles, spearproj->velocity);
		}
	}

	spearproj->owner = caster;
	VectorNormalize2(spearproj->velocity, spearproj->movedir);
	create_spearproj(spearproj);
	spearproj->reflect_debounce_time = MAX_REFLECT;

	G_LinkMissile(spearproj); 

	gi.trace(spearproj->s.origin, vec3_origin, vec3_origin, spearproj->s.origin, caster, MASK_PLAYERSOLID,&trace);
	if (trace.startsolid)
	{
		SpearProjTouch(spearproj, trace.ent, &trace.plane, trace.surface);
		return;
	}

	if(caster->spawnflags & MSF_INSECT_YELLOWJACKET)
	{
		spearproj->think = yellowjacket_proj_think;
		spearproj->nextthink = level.time + 0.1;
		spearproj->enemy = caster->enemy;
		Vec3ScaleAssign(0.5, spearproj->velocity);
		spearproj->ideal_yaw = SPEARPROJ_SPEED/2;
		spearproj->random = 30;
		spearproj->delay = 1.5;
		spearproj->count = 1;
		spearproj->health = 1;			// To indicate the homing projectile
		spearproj->red_rain_count = 1;

		gi.CreateEffect(&spearproj->s,
			FX_I_EFFECTS,
			CEF_OWNERS_ORIGIN,
			NULL,
			"bv",
			FX_I_SPEAR2,
			vec3_origin);
	}
	else
	{
		spearproj->count = 0;
		gi.CreateEffect(&spearproj->s,
			FX_I_EFFECTS,
			CEF_OWNERS_ORIGIN,
			vec3_origin,
			"bv",
			FX_I_SPEAR,
			spearproj->velocity);
	}
}