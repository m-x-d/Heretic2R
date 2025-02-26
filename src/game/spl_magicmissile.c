//
// spl_magicmissile.c
//
// Copyright 1998 Raven Software
//

#include "spl_magicmissile.h" //mxd
#include "g_ai.h" //mxd
#include "g_combat.h" //mxd
#include "g_playstats.h"
#include "Decals.h"
#include "FX.h"
#include "Random.h"
#include "Utilities.h"
#include "Vector.h"
#include "g_local.h"

#define MISSILE_RADIUS	2.0f //mxd. ARROW_RADIUS in original version.

static void MagicMissileTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surface)
{
	if (surface != NULL && (surface->flags & SURF_SKY))
	{
		SkyFly(self);
		return;
	}

	// Has the target got reflection turned on?
	if (self->reflect_debounce_time > 0 && EntReflecting(other, true, true))
	{
		Create_rand_relect_vect(self->velocity, self->velocity);

		// Scale speed down.
		Vec3ScaleAssign(MAGICMISSILE_SPEED / 2.0f, self->velocity);
		MagicMissileReflect(self, other, self->velocity);

		return;
	}

	if (other == self->owner || strcmp(self->classname, other->classname) == 0) // Don't collide with owner or other magic missiles.
		return;

	vec3_t scorch_origin;
	VectorCopy(self->s.origin, scorch_origin);

	// Calculate the position for the explosion entity.
	vec3_t origin;
	VectorMA(self->s.origin, -0.02f, self->velocity, origin);

	AlertMonsters(self, self->owner, 1.0f, false);

	if (other->takedamage != DAMAGE_NO)
	{
		T_Damage(other, self, self->owner, self->movedir, self->s.origin, plane->normal, self->dmg, self->dmg, DAMAGE_SPELL, MOD_MMISSILE);
	}
	else
	{
		// Back off the origin for the damage a bit.
		// We are a point and this will help fix hitting base of a stair and not hurting a guy on next step up.
		VectorMA(self->s.origin, -8.0f, self->movedir, self->s.origin);
	}

	// Do some blast damage when in deathmatch (too wimpy without it).
	if (DEATHMATCH)
	{
		T_DamageRadius(self, self->owner, self->owner, MAGICMISSILE_RADIUS, MAGICMISSILE_DAMAGE_RAD,
			MAGICMISSILE_DAMAGE_RAD * 0.25f, DAMAGE_SPELL | DAMAGE_EXTRA_KNOCKBACK, MOD_MMISSILE);
	}

	// Attempt to apply a scorchmark decal to the thing I hit.
	int make_scorch = 0;
	if (IsDecalApplicable(other, self->s.origin, surface, plane, NULL))
		make_scorch = CEF_FLAG6;

	gi.CreateEffect(&self->s, FX_WEAPON_MAGICMISSILEEXPLODE, make_scorch | CEF_OWNERS_ORIGIN, self->s.origin, "d", self->movedir);
	gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/MagicMissileHit.wav"), 2.0f, ATTN_NORM, 0.0f); //TODO: why 2.0 volume?

	G_SetToFree(self);
}

static void MagicMissileThink(edict_t* self)
{
	// Prevent any further transmission of this entity to clients.
	self->svflags |= SVF_NOCLIENT;
	self->think = NULL;
}

// Create guts of magic missile
static void CreateMagicMissile(edict_t* missile) //mxd. Named 'create_magic' in original version.
{
	missile->s.effects = (EF_NODRAW_ALWAYS_SEND | EF_ALWAYS_ADD_EFFECTS);
	missile->movetype = MOVETYPE_FLYMISSILE;

	missile->touch = MagicMissileTouch;
	missile->classname = "Spell_MagicMissile";
	missile->nextthink = level.time + 0.1f;
	VectorSet(missile->mins, -MISSILE_RADIUS, -MISSILE_RADIUS, -MISSILE_RADIUS);
	VectorSet(missile->maxs,  MISSILE_RADIUS,  MISSILE_RADIUS,  MISSILE_RADIUS);

	missile->dmg = irand(MAGICMISSILE_DAMAGE_MIN, MAGICMISSILE_DAMAGE_MAX); //30 - 40
	if (DEATHMATCH)
		missile->dmg /= 2; //15 - 20

	missile->solid = SOLID_BBOX;
	missile->clipmask = MASK_SHOT;
}

edict_t *MagicMissileReflect(edict_t *self, edict_t *other, vec3_t vel)
{
	edict_t	*magicmissile;
	short shortyaw, shortpitch;

	// create a new missile to replace the old one - this is necessary cos physics will do nasty things
	// with the existing one,since we hit something. Hence, we create a new one totally.
	magicmissile = G_Spawn();

	// copy everything across
	VectorCopy(self->s.origin, magicmissile->s.origin);
	CreateMagicMissile(magicmissile);
	VectorCopy(vel, magicmissile->velocity);
	VectorNormalize2(vel, magicmissile->movedir);
	AnglesFromDir(magicmissile->movedir, magicmissile->s.angles);
	magicmissile->owner = other;
	magicmissile->think = MagicMissileThink;
	magicmissile->health = self->health;
	magicmissile->enemy = self->owner;
	magicmissile->flags |= (self->flags & FL_NO_KNOCKBACK);
	magicmissile->reflect_debounce_time = self->reflect_debounce_time -1; //so it doesn't infinitely reflect in one frame somehow
	magicmissile->reflected_time=self->reflected_time;
	G_LinkMissile(magicmissile); 

	// create new trails for the new missile
	shortyaw = (short)(magicmissile->s.angles[YAW]*(65536.0/360.0));
	shortpitch = (short)(magicmissile->s.angles[PITCH]*(65536.0/360.0));

	gi.CreateEffect(&magicmissile->s,
				FX_WEAPON_MAGICMISSILE,
				CEF_OWNERS_ORIGIN|CEF_FLAG6,
				0,		
				"ss",
				shortyaw, shortpitch);

	// kill the existing missile, since its a pain in the ass to modify it so the physics won't screw it. 
	G_SetToFree(self);

	// Do a nasty looking blast at the impact point
	gi.CreateEffect(&magicmissile->s, FX_LIGHTNING_HIT, CEF_OWNERS_ORIGIN, NULL, "t", magicmissile->velocity);

	return(magicmissile);
}

// ****************************************************************************
// SpellCastMagicMissile
// ****************************************************************************

void SpellCastMagicMissile(edict_t *Caster,vec3_t StartPos,vec3_t AimAngles,vec3_t AimDir)
{
	edict_t	*MagicMissile;
	trace_t trace;
	vec3_t	TempVec;
	short	shortyaw, shortpitch;

	// Spawn the magic-missile.

	MagicMissile=G_Spawn();

	VectorNormalize2(AimDir, MagicMissile->movedir);
	VectorMA(StartPos,1.0,AimDir,MagicMissile->s.origin);

	CreateMagicMissile(MagicMissile);
	MagicMissile->owner=Caster;
	MagicMissile->reflect_debounce_time = MAX_REFLECT;
	G_LinkMissile(MagicMissile); 

	gi.trace(Caster->s.origin, MagicMissile->mins, MagicMissile->maxs, MagicMissile->s.origin, Caster, MASK_PLAYERSOLID,&trace);
	if (trace.startsolid)
	{
		MagicMissileTouch(MagicMissile, trace.ent, &trace.plane, trace.surface);
		return;
	}

	// Handle autotargeting by looking for the nearest monster that:
	// a) Lies in a 45 degree degree horizontal, 180 degree vertical cone from my facing.
	// b) Lies within 0 to 1000 meters of myself.
	// c) Is visible (i.e. LOS exists from the missile to myself).
	
	if(MagicMissile->enemy=FindNearestVisibleActorInFrustum(MagicMissile,
													AimAngles,
													0.0,
													1000.0,
													ANGLE_30,
													ANGLE_180,
													MagicMissile->s.origin,
													NULL,NULL))
	{
		VectorCopy(MagicMissile->s.origin,TempVec);
		VectorSubtract(MagicMissile->enemy->s.origin,TempVec,TempVec);

		TempVec[0]+=(MagicMissile->enemy->mins[0]+MagicMissile->enemy->maxs[0])/2.0;
		TempVec[1]+=(MagicMissile->enemy->mins[1]+MagicMissile->enemy->maxs[1])/2.0;
		TempVec[2]+=(MagicMissile->enemy->mins[2]+MagicMissile->enemy->maxs[2])/2.0;
		
		VectorNormalize(TempVec);
		vectoangles(TempVec,MagicMissile->s.angles);
		// The pitch is flipped in these?
		MagicMissile->s.angles[PITCH] = -MagicMissile->s.angles[PITCH];
		VectorScale(TempVec,MAGICMISSILE_SPEED,MagicMissile->velocity);
	}
	else
	{
		VectorScale(AimDir,MAGICMISSILE_SPEED,MagicMissile->velocity);
		VectorCopy(AimAngles,MagicMissile->s.angles);
	}

	shortyaw = (short)(MagicMissile->s.angles[YAW]*(65536.0/360.0));
	shortpitch = (short)(MagicMissile->s.angles[PITCH]*(65536.0/360.0));

	gi.CreateEffect(&MagicMissile->s,
				FX_WEAPON_MAGICMISSILE,
				CEF_OWNERS_ORIGIN,
				0,		
				"ss",
				shortyaw, shortpitch);


	MagicMissile->think=MagicMissileThink;
	MagicMissile->nextthink=level.time+0.1;
}


/*
// ****************************************************************************
// MagicMissileThink1
// ****************************************************************************

static void MagicMissileThink1(edict_t *self)
{
	vec3_t	TempVec;
	short	shortyaw, shortpitch;

	// Handle autotargeting by looking for the nearest monster that:
	// a) Lies in a 45 degree degree horizontal, 180 degree vertical cone from my facing.
	// b) Lies within 0 to 1000 meters of myself.
	// c) Is visible (i.e. LOS exists from the missile to myself).
	
	if(self->enemy=FindNearestVisibleActorInFrustum(self,
													self->s.angles,
													0.0,
													1000.0,
													ANGLE_45,
													ANGLE_180,
													SVF_MONSTER,
													self->s.origin,
													NULL,NULL))
	{
		VectorCopy(self->s.origin,TempVec);
		VectorSubtract(self->enemy->s.origin,TempVec,TempVec);

		TempVec[0]+=(self->enemy->mins[0]+self->enemy->maxs[0])/2.0;
		TempVec[1]+=(self->enemy->mins[1]+self->enemy->maxs[1])/2.0;
		TempVec[2]+=(self->enemy->mins[2]+self->enemy->maxs[2])/2.0;
		
		VectorNormalize(TempVec);
		vectoangles(TempVec,self->s.angles);
		VectorScale(TempVec,MAGICMISSILE_SPEED,self->velocity);
	}
	else
	{
		VectorScale(self->velocity,MAGICMISSILE_SPEED,self->velocity);
	}

	shortyaw = (short)(self->s.angles[YAW]*(65536.0/360.0));
	shortpitch = (short)(self->s.angles[PITCH]*(65536.0/360.0));

	gi.CreateEffect(&self->s,
				FX_WEAPON_MAGICMISSILE,
				CEF_OWNERS_ORIGIN,
				0,		
				"ss",
				shortyaw, shortpitch);


	self->think=MagicMissileThink2;
	self->nextthink=level.time+0.1;
}
*/
