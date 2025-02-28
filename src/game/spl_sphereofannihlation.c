//
// spl_sphereofannihilation.c
//
// Copyright 1998 Raven Software
//

#include "spl_sphereofannihlation.h" //mxd
#include "g_ai.h" //mxd
#include "g_combat.h" //mxd
#include "g_playstats.h"
#include "m_beast.h"
#include "p_main.h" // For PLAYER_FLAG_KNOCKDOWN.
#include "Decals.h"
#include "FX.h"
#include "Random.h"
#include "Vector.h"
#include "Utilities.h"
#include "g_local.h"

#define SPHERE_INIT_SCALE			0.7f
#define SPHERE_MAX_SCALE			2.0f
#define SPHERE_SCALE_RANGE			(SPHERE_MAX_SCALE - SPHERE_INIT_SCALE)
#define SPHERE_SCALE_PER_CHARGE		(SPHERE_SCALE_RANGE / SPHERE_MAX_CHARGES)
#define SPHERE_SCALE_PULSE			0.5f

#define SPHERE_RADIUS_DIFF			(SPHERE_RADIUS_MAX - SPHERE_RADIUS_MIN)
#define SPHERE_RADIUS_PER_CHARGE	(SPHERE_RADIUS_DIFF / SPHERE_MAX_CHARGES)
#define SPHERE_GROW_MIN_TIME		2
#define SPHERE_GROW_SPEED			(SPHERE_RADIUS_DIFF / SPHERE_MAX_CHARGES)
#define SPHERE_GROW_START			(SPHERE_RADIUS_MIN - (SPHERE_GROW_SPEED * SPHERE_GROW_MIN_TIME))

#define SPHERE_COUNT_MIN			3
#define SPHERE_RADIUS				2.0f

// For Celestial Watcher.
#define SPHERE_WATCHER_DAMAGE_MIN			50
#define SPHERE_WATCHER_DAMAGE_RANGE			150
#define SPHERE_WATCHER_EXPLOSION_RADIUS_MIN	50.0f
#define SPHERE_WATCHER_EXPLOSION_RADIUS_MAX	200.0f

static void SphereWatcherGrowThink(edict_t *Self);

static void SphereExplodeThink(edict_t* self)
{
	edict_t* ent = NULL;

	while ((ent = FindInRadius(ent, self->s.origin, self->dmg_radius)) != NULL)
	{
		if (ent->takedamage == DAMAGE_NO || ent == self->owner || ent->fire_timestamp >= self->fire_timestamp)
			continue;

		T_Damage(ent, self, self->owner, self->velocity, ent->s.origin, vec3_origin, self->dmg, 0, 0, MOD_SPHERE);
		ent->fire_timestamp = self->fire_timestamp;
		gi.CreateEffect(&ent->s, FX_LIGHTNING_HIT, CEF_OWNERS_ORIGIN, NULL, "t", vec3_origin);
	}

	self->count--;
	self->dmg_radius += SPHERE_GROW_SPEED;
	self->nextthink = level.time + 0.1f;

	if (self->count < 0)
		G_SetToFree(self);
}

static void SphereOfAnnihilationTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surface)
{
	// Has the target got reflection turned on?
	if (self->reflect_debounce_time > 0 && EntReflecting(other, true, true))
	{
		Create_rand_relect_vect(self->velocity, self->velocity);
		Vec3ScaleAssign(SPHERE_FLY_SPEED / 2.0f, self->velocity);
		SphereReflect(self, other, self->velocity);

		return;
	}

	if (surface != NULL && (surface->flags & SURF_SKY))
	{
		SkyFly(self);
		return;
	}

	edict_t* explosion = G_Spawn();

	VectorCopy(self->s.origin, explosion->s.origin);
	explosion->solid = SOLID_NOT;
	explosion->dmg_radius = SPHERE_GROW_START;
	explosion->count = self->count + SPHERE_GROW_MIN_TIME;
	explosion->fire_timestamp = level.time;
	explosion->think = SphereExplodeThink;
	explosion->owner = self->owner;
	explosion->classname = "sphere_damager";
	explosion->dmg = self->dmg;

	gi.linkentity(explosion);

	AlertMonsters(self, self->owner, 3.0f, false);

	// Back off the origin for the damage a bit.
	// We are a point and this will help fix hitting base of a stair and not hurting a guy on next step up.
	VectorMA(self->s.origin, -8.0f, self->movedir, self->s.origin);

	gi.CreateEffect(&self->s, FX_WEAPON_SPHEREPLAYEREXPLODE, CEF_OWNERS_ORIGIN, NULL, "db", self->movedir, (byte)self->count);
	gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/SphereImpact.wav"), 2.0f, ATTN_NORM, 0.0f); //TODO: why 2.0 volume?

	G_SetToFree(self);

	// Do damage directly to the thing you hit. This is mainly for big creatures, like the trial beast.
	// The sphere will not damage others after this initial impact.
	if (other != NULL && other->takedamage != DAMAGE_NO)
	{
		T_Damage(other, self, self->owner, self->velocity, self->s.origin, vec3_origin, self->dmg, 0, 0, MOD_SPHERE);
		other->fire_timestamp = level.time;

		gi.CreateEffect(&other->s, FX_LIGHTNING_HIT, CEF_OWNERS_ORIGIN, NULL, "t", vec3_origin);
	}

	SphereExplodeThink(explosion);
}

static void SphereOfAnnihilationGrowThink(edict_t* self)
{
	vec3_t forward;
	vec3_t up;

	const gclient_t* cl = self->owner->client; //mxd

	if (cl != NULL)
		AngleVectors(cl->aimangles, forward, NULL, up);
	else
		AngleVectors(self->owner->s.angles, forward, NULL, up);

	// NOTE: 'edict_t'->combattarget is used as a pointer to a 'qboolean' which flags whether or not I have been released.
	// Would like a dedicated value in the 'edict_t' but this is unlikely to happen, sooooo...

	// If we have released, or we are dead, or a chicken, release the sphere.
	if (*(qboolean*)self->combattarget && !(self->owner->deadflag & (DEAD_DYING | DEAD_DEAD))
		&& (cl != NULL && !(cl->playerinfo.edictflags & FL_CHICKEN)) && !(cl->playerinfo.flags & PLAYER_FLAG_KNOCKDOWN))
	{
		if (self->count < SPHERE_MAX_CHARGES)
		{
			self->count++;
			self->s.scale = SPHERE_INIT_SCALE + (SPHERE_SCALE_PER_CHARGE * (float)self->count);
		}
		else
		{
			self->s.scale = SPHERE_MAX_SCALE + flrand(0, SPHERE_SCALE_PULSE); // If at max size, pulse like crazy!
		}

		// Detect if we have teleported, need to move with the player if that's so.
		if (cl != NULL)
		{
			VectorCopy(self->owner->s.origin, self->s.origin);

			self->s.origin[0] += forward[0] * 20.0f;
			self->s.origin[1] += forward[1] * 20.0f;
			self->s.origin[2] += (float)self->owner->viewheight - 5.0f;
		}

		self->nextthink = level.time + 0.1f;
	}
	else
	{
		// My caster has released me, so I am now a missile and I will fly like the wind.
		self->svflags &= ~SVF_NOCLIENT;
		self->s.effects &= ~EF_MARCUS_FLAG1;

		// Check ahead first to see if it's going to hit anything at this angle.
		vec3_t end_pos;
		VectorMA(self->s.origin, SPHERE_FLY_SPEED, forward, end_pos);

		trace_t trace;
		gi.trace(self->s.origin, vec3_origin, vec3_origin, end_pos, self->owner, MASK_MONSTERSOLID, &trace);

		if (trace.ent != NULL && OkToAutotarget(self->owner, trace.ent))
			VectorScale(forward, SPHERE_FLY_SPEED, self->velocity); // Already going to hit a valid target at this angle - so don't auto-target.
		else
			GetAimVelocity(self->owner->enemy, self->s.origin, SPHERE_FLY_SPEED, cl->aimangles, self->velocity); // Auto-target current enemy.

		VectorNormalize2(self->velocity, self->movedir);

		self->movetype = MOVETYPE_FLYMISSILE;
		self->solid = SOLID_BBOX;
		self->health = 0;
		self->dmg = SPHERE_DAMAGE;
		self->dmg_radius = SPHERE_RADIUS_MIN + (SPHERE_RADIUS_PER_CHARGE * (float)self->count);
		self->touch = SphereOfAnnihilationTouch;
		self->think = NULL;

		VectorSet(self->mins, -SPHERE_RADIUS, -SPHERE_RADIUS, -SPHERE_RADIUS);
		VectorSet(self->maxs, SPHERE_RADIUS, SPHERE_RADIUS, SPHERE_RADIUS);

		self->s.sound = 0;
		gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/SphereFire.wav"), 1.0f, ATTN_NORM, 0.0f);

		gi.trace(self->s.origin, vec3_origin, vec3_origin, self->s.origin, self->owner, MASK_PLAYERSOLID, &trace);

		if (trace.startsolid)
			SphereOfAnnihilationTouch(self, trace.ent, &trace.plane, trace.surface);
	}
}

static void SpherePowerLaserThink(edict_t* self)
{
	static const vec3_t min = { -16.0f, -16.0f, -16.0f }; //mxd. Made static.
	static const vec3_t max = {  16.0f,  16.0f,  16.0f }; //mxd. Made static.

	vec3_t start_pos;
	VectorCopy(self->s.origin, start_pos);

	const edict_t* trace_buddy = self;
	float sphere_dist = 2048.0f;

	vec3_t aim_angles;
	VectorCopy(self->s.angles, aim_angles);

	vec3_t fwd;
	AngleVectors(aim_angles, fwd, NULL, NULL);
	VectorMA(start_pos, 12.0f, fwd, start_pos);

	trace_t tr;
	vec3_t shoot_dir;
	int num_hit = 0; // Can't hit more than 8 guys...

	do
	{
		AngleVectors(aim_angles, shoot_dir, NULL, NULL);

		vec3_t end_pos;
		VectorMA(start_pos, sphere_dist, shoot_dir, end_pos);

		gi.trace(start_pos, min, max, end_pos, trace_buddy, MASK_SHOT, &tr);

		if (level.fighting_beast)
		{
			edict_t* ent = TB_CheckHit(start_pos, tr.endpos);

			if (ent != NULL)
				tr.ent = ent;
		}

		if (tr.ent->takedamage == DAMAGE_NO)
			break;

		if ((tr.startsolid || tr.fraction < 0.99f) && tr.ent != NULL)
		{
			// Reflect it off into space?
			if (EntReflecting(tr.ent, true, true))
			{
				// Draw line to this point.
				vec3_t diff;
				VectorSubtract(tr.endpos, start_pos, diff);
				const float trace_dist = VectorLength(diff);

				gi.CreateEffect(NULL, FX_WEAPON_SPHEREPOWER, 0, start_pos, "xbb", shoot_dir, (byte)(self->s.scale * 7.5f), (byte)(trace_dist / 8.0f));

				// Re-constitute aim_angles.
				aim_angles[1] += flrand(160.0f, 200.0f);
				aim_angles[0] += flrand(-20.0f, 20.0f);

				break;
			}

			if (tr.ent->fire_timestamp < self->fire_timestamp)
			{
				T_Damage(tr.ent, self, self->owner, shoot_dir, tr.endpos, shoot_dir, SPHERE_DAMAGE, SPHERE_DAMAGE, DAMAGE_SPELL, MOD_P_SPHERE);
				tr.ent->fire_timestamp = self->fire_timestamp;
			}
		}

		vec3_t diff;
		VectorSubtract(tr.endpos, start_pos, diff);
		sphere_dist -= VectorLength(diff);

		VectorCopy(tr.endpos, start_pos);

		// This seems to alleviate the problem of a trace hitting the same ent multiple times...
		VectorSubtract(end_pos, start_pos, diff);
		if (VectorLength(diff) > 16.0f)
			VectorMA(start_pos, 16.0f, shoot_dir, start_pos);

		trace_buddy = tr.ent;
		num_hit++;
	} while (tr.fraction < 0.99f && tr.contents != MASK_SOLID && num_hit < MAX_REFLECT);

	vec3_t temp;
	VectorSubtract(tr.endpos, start_pos, temp);
	const float trace_dist = VectorLength(temp);

	// Set 'is powered' flag.
	int fx_flags = CEF_FLAG7;

	// When CEF_FLAG8 is set, move to the left. Otherwise to the right.
	if (self->health == 2)
		fx_flags |= CEF_FLAG8;

	// Decide if a decal is appropriate or not.
	vec3_t plane_dir;
	if (IsDecalApplicable(tr.ent, self->s.origin, tr.surface, &tr.plane, plane_dir))
		fx_flags |= CEF_FLAG6;

	gi.CreateEffect(NULL, FX_WEAPON_SPHEREPOWER, fx_flags, start_pos, "xbb", shoot_dir, (byte)(self->s.scale * 7.5f), (byte)(trace_dist / 8.0f));

	if (--self->count <= 0)
		G_SetToFree(self);
	else
		self->nextthink = level.time + 0.1f;
}

static void SpherePowerLaserTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surface)
{
	G_SetToFree(self);
}

static void SphereOfAnnihilationGrowThinkPower(edict_t* self)
{
	vec3_t forward;
	vec3_t right;

	const gclient_t* cl = self->owner->client; //mxd

	if (cl != NULL)
		AngleVectors(cl->aimangles, forward, right, NULL);
	else
		AngleVectors(self->owner->s.angles, forward, right, NULL);

	// NOTE: 'edict_t'->combattarget is used as a pointer to a 'qboolean' which flags whether or not I have been released.
	// Would like a dedicated value in the 'edict_t' but this is unlikely to happen, sooooo...

	// If we have released, or we are dead, or a chicken, release the sphere.
	if (*(qboolean*)self->combattarget && !(self->owner->deadflag & (DEAD_DYING | DEAD_DEAD))
		&& (cl != NULL && !(cl->playerinfo.edictflags & FL_CHICKEN)) && !(cl->playerinfo.flags & PLAYER_FLAG_KNOCKDOWN))
	{
		if (self->count < SPHERE_MAX_CHARGES)
		{
			self->count++;
			self->s.scale = SPHERE_INIT_SCALE + (SPHERE_SCALE_PER_CHARGE * (float)self->count);
		}
		else
		{
			self->s.scale = SPHERE_MAX_SCALE + flrand(0, SPHERE_SCALE_PULSE); // If at max size, pulse like crazy!
		}

		// Detect if we have teleported, need to move with the player if that's so.
		if (cl != NULL)
		{
			VectorCopy(self->owner->s.origin, self->s.origin);

			self->s.origin[0] += forward[0] * 20.0f;
			self->s.origin[1] += forward[1] * 20.0f;
			self->s.origin[2] += (float)self->owner->viewheight - 5.0f;
		}

		self->nextthink = level.time + 0.1f;
	}
	else
	{
		for (int i = 1; i < 3; i++)
		{
			const float direction = (i == 1 ? 1.0f : -1.0f); //mxd. Left / right.

			edict_t* laser = G_Spawn();

			laser->owner = self->owner;
			laser->count = self->count - SPHERE_COUNT_MIN;
			VectorMA(self->s.origin, 10.0f * direction, right, laser->s.origin);

			if (cl != NULL)
				VectorCopy(cl->aimangles, laser->s.angles);
			else
				VectorCopy(self->owner->s.angles, laser->s.angles);

			VectorScale(right, SPHERE_LASER_SPEED * direction, laser->velocity);
			laser->health = i;
			laser->movetype = MOVETYPE_FLYMISSILE;
			laser->solid = SOLID_BBOX;
			laser->clipmask = MASK_SOLID;
			laser->think = SpherePowerLaserThink;
			laser->touch = SpherePowerLaserTouch;
			laser->fire_timestamp = level.time;

			G_LinkMissile(laser);
			SpherePowerLaserThink(laser);
		}

		gi.sound(self->owner, CHAN_WEAPON, gi.soundindex("weapons/SpherePowerFire.wav"), 1.0f, ATTN_NORM, 0.0f);
		G_SetToFree(self);
	}
}

// Guts of create sphere.
static void CreateSphere(edict_t* sphere)
{
	sphere->svflags |= SVF_ALWAYS_SEND;
	sphere->s.effects |= EF_ALWAYS_ADD_EFFECTS | EF_MARCUS_FLAG1;
	sphere->classname = "Spell_SphereOfAnnihilation";
	sphere->clipmask = MASK_SHOT;
	sphere->movetype = MOVETYPE_FLYMISSILE;
	sphere->nextthink = level.time + 0.1f;
}

edict_t *SphereReflect(edict_t *self, edict_t *other, vec3_t vel)
{
	edict_t	*Sphere;
   	Sphere = G_Spawn();
   	CreateSphere(Sphere);
   	Sphere->owner = other;
   	Sphere->enemy = self->enemy;
   	Sphere->reflect_debounce_time = self->reflect_debounce_time -1;
	Sphere->reflected_time=self->reflected_time;

   	Sphere->count=self->count;
   	Sphere->solid=self->solid;
   	Sphere->dmg=self->dmg;
   	Sphere->dmg_radius=self->dmg_radius;
   	Sphere->s.scale=self->s.scale;

   	VectorCopy(vel, Sphere->velocity);

   	Sphere->touch=SphereOfAnnihilationTouch;
   	Sphere->nextthink=level.time+0.1;

   	VectorCopy(self->mins, Sphere->mins);
   	VectorCopy(self->maxs, Sphere->maxs);

   	VectorCopy(self->s.origin, Sphere->s.origin);
   	G_LinkMissile(Sphere); 

   	gi.CreateEffect(&Sphere->s,
   				FX_WEAPON_SPHERE,
   				CEF_OWNERS_ORIGIN,
   				NULL,
   				"s",
   				(short)Sphere->owner->s.number);

   	gi.CreateEffect(&Sphere->s,
   				FX_WEAPON_SPHEREGLOWBALLS,
   				CEF_OWNERS_ORIGIN,
   				NULL,
   				"s",
   				-1);


   	// kill the existing missile, since its a pain in the ass to modify it so the physics won't screw it. 
   	G_SetToFree(self);

   	// Do a nasty looking blast at the impact point
   	gi.CreateEffect(&Sphere->s, FX_LIGHTNING_HIT, CEF_OWNERS_ORIGIN, NULL, "t", Sphere->velocity);

   	return(Sphere);
}

// ****************************************************************************
// SpellCastSphereOfAnnihilation
// ****************************************************************************

void SpellCastSphereOfAnnihilation(edict_t *Caster,vec3_t StartPos,vec3_t AimAngles,vec3_t AimDir,
								  float Value,qboolean *ReleaseFlagsPtr)
{
	edict_t	*Sphere;
	int	flags;

	// Spawn the sphere of annihilation as an invisible entity (i.e. modelindex=0).

	Sphere=G_Spawn();

	if(Caster->client)
	{
		VectorCopy(Caster->s.origin, Sphere->s.origin);	
		Sphere->s.origin[0] += AimDir[0]*20.0;
		Sphere->s.origin[1] += AimDir[1]*20.0;
		Sphere->s.origin[2] += Caster->viewheight-5.0;
	}
	else
		VectorCopy(StartPos, Sphere->s.origin);	
	
	VectorCopy(AimAngles,Sphere->s.angles);

	Sphere->avelocity[YAW]=100.0;
	Sphere->avelocity[ROLL]=100.0;

	// NOTE: 'edict_t'->combattarget is used as a pointer to a 'qboolean' which flags
	// whether or not I have been released. Would like a dedicated value in the 'edict_t' but this
	// is unlikely to happen, sooooo...


	Sphere->combattarget=(char *)ReleaseFlagsPtr;

	Sphere->count = 0;
	Sphere->solid = SOLID_NOT;
	Sphere->dmg = 0;
	Sphere->s.scale = SPHERE_INIT_SCALE;
	Sphere->owner = Caster;
	Sphere->enemy = Caster->enemy;
	CreateSphere(Sphere);
	Sphere->reflect_debounce_time = MAX_REFLECT;

	if (Caster->client && Caster->client->playerinfo.powerup_timer > level.time)
	{
		Sphere->think=SphereOfAnnihilationGrowThinkPower;
	}
	else
	{
		if (Caster->client)
			Sphere->think=SphereOfAnnihilationGrowThink;
		else	// The celestial watcher can also cast a sphere, but a different kind of one.
			Sphere->think=SphereWatcherGrowThink;
	}

	gi.linkentity(Sphere); 

	if(!Caster->client)
		flags = 0;
	else
		flags = CEF_OWNERS_ORIGIN;

	gi.CreateEffect(&Sphere->s,
					FX_WEAPON_SPHERE,
					flags,
					StartPos,
					"s",
					(short)Caster->s.number);

	if(!Caster->client)
		flags = CEF_FLAG6;
	else
		flags = CEF_OWNERS_ORIGIN;

	gi.CreateEffect(&Sphere->s,
					FX_WEAPON_SPHEREGLOWBALLS,
					flags,
					StartPos,
					"s",
					(short)Caster->s.number);

	Sphere->s.sound = gi.soundindex("weapons/SphereGrow.wav");
	Sphere->s.sound_data = (255 & ENT_VOL_MASK) | ATTN_NORM;
}

// ****************************************************************************
// SphereWatcherFlyThink
// ****************************************************************************

void SphereWatcherFlyThink(edict_t *Self)
{
	Self->count++;

	if(Self->count > 20)
	{	
		// End the circling...
		G_SetToFree(Self);
	}
	else
	{
		Self->nextthink=level.time+0.2;
	}
}


static void SphereWatcherGrowThink(edict_t *Self)
{
	vec3_t	Forward, Up, endpos;
	trace_t trace;


	if (Self->owner->client)
		AngleVectors(Self->owner->client->aimangles,Forward,NULL,Up);
	else
		AngleVectors(Self->owner->s.angles,Forward,NULL,Up);

	// NOTE: 'edict_t'->combattarget is used as a pointer to a 'qboolean' which flags
	// whether or not I have been released. Would like a dedicated value in the 'edict_t' but this
	// is unlikely to happen, sooooo...

	// if we have released, or we are dead, or a chicken, release the sphere 
	if(*(qboolean *)Self->combattarget && !(Self->owner->deadflag & (DEAD_DYING|DEAD_DEAD)))
	{

		Self->count+=irand(1,2);
		
		if((Self->count>10)&&(Self->s.scale<SPHERE_MAX_SCALE))
		{
			if(Self->count>20)
			{
				Self->s.scale-=0.01;
			}
			else
			{
				Self->s.scale+=0.1;
			}

			if(Self->count>25)
			{
				Self->count&=3;
			}

		}

		Self->nextthink=level.time+0.1;
	}
	else
	{
		vec3_t	angles;
		// My caster has released me, so I am now a missile and I will fly like the wind.

		Self->svflags &= ~SVF_NOCLIENT;

		Self->s.effects&=~EF_MARCUS_FLAG1;

		VectorCopy(Self->owner->movedir,Self->movedir);

		//Check ahead first to see if it's going to hit anything at this angle
		VectorCopy(Self->owner->s.angles, angles);
		AngleVectors(angles, Forward, NULL, NULL);
		VectorMA(Self->s.origin, SPHERE_FLY_SPEED, Forward, endpos);
		gi.trace(Self->s.origin, vec3_origin, vec3_origin, endpos, Self->owner, MASK_MONSTERSOLID,&trace);
		if(trace.ent && OkToAutotarget(Self->owner, trace.ent))
		{//already going to hit a valid target at this angle- so don't autotarget
			VectorScale(Forward, SPHERE_FLY_SPEED, Self->velocity);
		}
		else
		{//autotarget current enemy
			GetAimVelocity(Self->owner->enemy, Self->s.origin, SPHERE_FLY_SPEED, Self->s.angles, Self->velocity);
		}
		VectorNormalize2(Self->velocity, Self->movedir);
		
		Self->movetype=MOVETYPE_FLYMISSILE;
		Self->solid=SOLID_BBOX;
		Self->health=0;
		Self->count=0;
		Self->dmg=	SPHERE_WATCHER_DAMAGE_MIN + 
					(SPHERE_WATCHER_DAMAGE_RANGE*((Self->s.scale-SPHERE_INIT_SCALE)/SPHERE_SCALE_RANGE));
		Self->dmg_radius = 
					SPHERE_WATCHER_EXPLOSION_RADIUS_MIN + 
					(SPHERE_WATCHER_EXPLOSION_RADIUS_MAX - SPHERE_WATCHER_EXPLOSION_RADIUS_MIN) * 
							(Self->s.scale-SPHERE_INIT_SCALE)/SPHERE_SCALE_RANGE;
		Self->touch=SphereWatcherTouch;
		Self->think=SphereWatcherFlyThink;
		Self->nextthink=level.time+0.1;
			
	   	VectorSet(Self->mins, -SPHERE_RADIUS, -SPHERE_RADIUS, -SPHERE_RADIUS);
		VectorSet(Self->maxs, SPHERE_RADIUS, SPHERE_RADIUS, SPHERE_RADIUS);

		Self->s.sound = 0;
		gi.sound(Self,CHAN_WEAPON,gi.soundindex("weapons/SphereFire.wav"),1,ATTN_NORM,0);

		gi.trace(Self->s.origin, vec3_origin, vec3_origin, Self->s.origin, Self->owner, MASK_PLAYERSOLID,&trace);
		if (trace.startsolid)
		{
			SphereWatcherTouch(Self, trace.ent, &trace.plane, trace.surface);
			return;
		}
	}
}



edict_t *SphereWatcherReflect(edict_t *self, edict_t *other, vec3_t vel)
{
	edict_t	*Sphere;
   	Sphere = G_Spawn();
   	CreateSphere(Sphere);
   	Sphere->owner = other;
   	Sphere->enemy = self->enemy;
   	Sphere->reflect_debounce_time = self->reflect_debounce_time -1;
	Sphere->reflected_time=self->reflected_time;

   	Sphere->count=self->count;
   	Sphere->solid=self->solid;
   	Sphere->dmg=self->dmg;
   	Sphere->dmg_radius=self->dmg_radius;
   	Sphere->s.scale=self->s.scale;

   	VectorCopy(vel, Sphere->velocity);

   	Sphere->touch=SphereWatcherTouch;
   	Sphere->think=SphereWatcherFlyThink;
   	Sphere->nextthink=level.time+0.1;

   	VectorCopy(self->mins, Sphere->mins);
   	VectorCopy(self->maxs, Sphere->maxs);

   	VectorCopy(self->s.origin, Sphere->s.origin);
   	G_LinkMissile(Sphere); 

   	gi.CreateEffect(&Sphere->s,
   				FX_WEAPON_SPHERE,
   				CEF_OWNERS_ORIGIN,
   				NULL,
   				"s",
   				(short)Sphere->owner->s.number);

   	gi.CreateEffect(&Sphere->s,
   				FX_WEAPON_SPHEREGLOWBALLS,
   				CEF_OWNERS_ORIGIN,
   				NULL,
   				"s",
   				-1);


   	// kill the existing missile, since its a pain in the ass to modify it so the physics won't screw it. 
   	G_SetToFree(self);

   	// Do a nasty looking blast at the impact point
   	gi.CreateEffect(&Sphere->s, FX_LIGHTNING_HIT, CEF_OWNERS_ORIGIN, NULL, "t", Sphere->velocity);

   	return(Sphere);
}



static void SphereWatcherTouch(edict_t *self, edict_t *Other, cplane_t *Plane, csurface_t *surface)
{
	int			makeScorch;

	// has the target got reflection turned on ?
	if(EntReflecting(Other, true, true) && self->reflect_debounce_time)
	{
		Create_rand_relect_vect(self->velocity, self->velocity);
		Vec3ScaleAssign(SPHERE_FLY_SPEED/2, self->velocity);
		SphereWatcherReflect(self, Other, self->velocity);
		return;
	}


	if(surface && (surface->flags & SURF_SKY))
	{
		SkyFly(self);
		return;
	}

	AlertMonsters (self, self->owner, 3, false);
	if(Other->takedamage)
	{
		T_Damage(Other, self, self->owner, self->movedir, self->s.origin, Plane->normal, self->dmg, self->dmg, DAMAGE_SPELL,MOD_SPHERE);
	}
	else
	{
		// Back off the origin for the damage a bit. We are a point and this will
		// help fix hitting base of a stair and not hurting a guy on next step up.
		VectorMA(self->s.origin, -8.0, self->movedir, self->s.origin);
	}

	T_DamageRadius(self, self->owner, self, self->dmg_radius, self->dmg, self->dmg / 8, DAMAGE_ATTACKER_KNOCKBACK,MOD_SPHERE);
	makeScorch = 0;
	if(IsDecalApplicable(Other, self->s.origin, surface, Plane, NULL))
	{
		makeScorch = CEF_FLAG6;
	}
	gi.CreateEffect(&self->s, FX_WEAPON_SPHEREEXPLODE, CEF_OWNERS_ORIGIN | makeScorch, NULL,
					"db", self->movedir, (byte)(self->s.scale * 7.5));

	gi.sound(self,CHAN_WEAPON,gi.soundindex("weapons/SphereImpact.wav"),2,ATTN_NORM,0);

	G_SetToFree(self);
}
