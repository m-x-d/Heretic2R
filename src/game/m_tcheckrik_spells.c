//
// m_tcheckrik_spells.c
//
// Copyright 1998 Raven Software
//

#include "m_tcheckrik_spells.h" //mxd
#include "g_combat.h" //mxd
#include "g_playstats.h"
#include "m_stats.h"
#include "m_morcalavin.h" //mxd
#include "Decals.h"
#include "Vector.h"
#include "Random.h"
#include "Utilities.h"

#define INSECT_GLOBE_MAX_SCALE	1.8f //mxd. Named 'GLOBE_MAX_SCALE' in original logic.
#define INSECT_SPEAR_PROJECTILE_SPEED	600.0f //mxd. Named 'SPEARPROJ_SPEED' in original logic.

#pragma region ========================== Insect staff bolt spell ==========================

static void InsectStaffBoltInit(edict_t* bolt); //TODO: remove.

static void InsectStaffBoltThink(edict_t* self) //mxd. Named 'InsectStaffThink' in original logic.
{
	vec3_t forward;

	// Grow myself a bit.
	self->s.scale = 1.0f;

	// Do auto-targeting.
	if (self->enemy != NULL)
	{
		// I have a target (pointed at by self->enemy) so aim myself at it.
		VectorSubtract(self->enemy->s.origin, self->s.origin, forward);

		for (int i = 0; i < 3; i++)
			forward[i] += (self->enemy->mins[i] + self->enemy->maxs[i]) / 2.0f;

		VectorNormalize(forward);
	}
	else
	{
		// No target, fly straight ahead.
		AngleVectors(self->s.angles, forward, NULL, NULL);
	}

	// Give myself a velocity of 750 in my forward direction.
	VectorScale(forward, INSECT_STAFF_AIMED_SPEED, self->velocity);

	self->think = NULL;
}

static void InsectStaffBoltTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surface) //mxd. Named 'InsectStaffTouch' in original logic.
{
	if (surface != NULL && (surface->flags & SURF_SKY))
	{
		SkyFly(self);
		return;
	}

	if (EntReflecting(other, true, true))
	{
		edict_t* bolt = G_Spawn();

		InsectStaffBoltInit(bolt);

		bolt->owner = self->owner;
		bolt->enemy = NULL;
		bolt->s.scale = self->s.scale;

		VectorCopy(self->s.origin, bolt->s.origin);
		Create_rand_relect_vect(self->velocity, bolt->velocity);
		VectorCopy(bolt->velocity, bolt->movedir);
		Vec3ScaleAssign(INSECT_STAFF_SPEED, bolt->velocity);
		vectoangles(bolt->velocity, bolt->s.angles);

		G_LinkMissile(bolt);
		gi.CreateEffect(&bolt->s, FX_I_EFFECTS, CEF_OWNERS_ORIGIN, vec3_origin, "bv", FX_I_SP_MSL_HIT, vec3_origin); //TODO: 'origin' arg should be bolt->s.origin; last arg should be bolt->movedir?
		G_SetToFree(self);

		return;
	}

	if (other->takedamage != DAMAGE_NO)
	{
		T_Damage(other, self, self->owner, self->movedir, self->s.origin, plane->normal, self->dmg, 0, DAMAGE_SPELL, MOD_DIED);
	}
	else
	{
		// Back off the origin for the damage a bit. We are a point and this will help fix hitting base of a stair and not hurting a guy on next step up.
		VectorMA(self->s.origin, -8.0f, self->movedir, self->s.origin);
	}

	if (self->count == 0)
	{
		// Attempt to apply a scorchmark decal to the thing I hit.
		const byte fx_flags = (IsDecalApplicable(other, self->s.origin, surface, plane, NULL) ? CEF_FLAG6 : 0);
		gi.CreateEffect(NULL, FX_I_EFFECTS, fx_flags, self->s.origin, "bv", FX_I_ST_MSL_HIT, self->movedir);

	}
	else
	{
		gi.sound(self, CHAN_BODY, gi.soundindex("monsters/imp/fbfire.wav"), 1.0f, ATTN_NORM, 0.0f);
		gi.CreateEffect(&self->s, FX_M_EFFECTS, CEF_OWNERS_ORIGIN, self->s.origin, "bv", FX_IMP_FBEXPL, vec3_origin);

	}

	G_SetToFree(self);
}

// Create the guts of the insect staff bolt.
static void InsectStaffBoltInit(edict_t* bolt) //mxd. Named 'create_insect_staff_bolt' in original logic.
{
	bolt->s.effects = (EF_NODRAW_ALWAYS_SEND | EF_CAMERA_NO_CLIP);
	bolt->movetype = MOVETYPE_FLYMISSILE;
	bolt->solid = SOLID_BBOX;
	bolt->classname = "Spell_InsectStaff";
	bolt->dmg = irand(TC_FEMALE_DMG_HACK_MIN, TC_FEMALE_DMG_HACK_MAX) * (SKILL + 1) / 3;
	bolt->clipmask = (MASK_MONSTERSOLID | MASK_SHOT);

	// Radius of zero seems to prevent collision between bolts.
	VectorClear(bolt->mins);
	VectorClear(bolt->maxs);

	bolt->touch = InsectStaffBoltTouch;
	bolt->think = InsectStaffBoltThink;
	bolt->nextthink = level.time + FRAMETIME; //mxd. Use define.
}

void SpellCastInsectStaff(edict_t* caster, const vec3_t start_pos, const vec3_t aim_angles, const vec3_t aim_dir, const qboolean power) //TODO: rename to SpellCastInsectStaffBolt.
{
	edict_t* bolt = G_Spawn();

	InsectStaffBoltInit(bolt);

	VectorCopy(start_pos, bolt->s.origin);
	VectorCopy(aim_angles, bolt->s.angles);
	VectorScale(aim_dir, INSECT_STAFF_SPEED, bolt->velocity);

	AngleVectors(aim_angles, bolt->movedir, NULL, NULL);

	bolt->s.scale = 0.1f;
	bolt->owner = caster;
	bolt->enemy = caster->enemy;

	G_LinkMissile(bolt);

	if (power)
	{
		Vec3ScaleAssign(2.0f, bolt->velocity);
		bolt->dmg *= 2;
		bolt->count = 1; //TODO: add qboolean insect_staff_bolt_powered name.

		gi.CreateEffect(&bolt->s, FX_M_EFFECTS, CEF_OWNERS_ORIGIN, NULL, "bv", FX_IMP_FIRE, bolt->velocity);
	}
	else
	{
		bolt->count = 0;
		gi.CreateEffect(&bolt->s, FX_I_EFFECTS, CEF_OWNERS_ORIGIN, vec3_origin, "bv", FX_I_STAFF, vec3_origin);
	}
}

#pragma endregion

#pragma region ========================== Insect globe of ouchiness spell ==========================

static void InsectGlobeOfOuchinessGrowThink(edict_t* self) //mxd. Named 'GlobeOfOuchinessGrowThink' in original logic.
{
	if (self->owner->s.effects & EF_DISABLE_EXTRA_FX)
	{
		gi.RemoveEffects(&self->s, 0);
		G_FreeEdict(self);

		return;
	}

	if (!self->owner->damage_debounce_time) //TODO: add qboolean insect_globe_released name.
	{
		self->count += irand(1, 2);

		if (self->count > 10 && self->s.scale < INSECT_GLOBE_MAX_SCALE)
		{
			if (self->count > 20)
				self->s.scale -= 0.01f;
			else
				self->s.scale += 0.1f;

			if (self->count > 25)
				self->count &= 3;
		}

		vec3_t forward;
		vec3_t up;
		AngleVectors(self->owner->s.angles, forward, NULL, up);

		self->velocity[0] = 8.0f * ((self->owner->s.origin[0] + forward[0] * 22.0f + flrand(-2.0f, 2.0f)) - self->s.origin[0]);
		self->velocity[1] = 8.0f * ((self->owner->s.origin[1] + forward[1] * 22.0f + flrand(-2.0f, 2.0f)) - self->s.origin[1]);
		self->velocity[2] = 8.0f * ((self->owner->s.origin[2] + up[2] * 10.0f) - self->s.origin[2]);

		self->nextthink = level.time + FRAMETIME; //mxd. Use define.
	}
	else
	{
		self->owner->damage_debounce_time = true; //TODO: not needed?
		G_FreeEdict(self);
	}
}

void SpellCastGlobeOfOuchiness(edict_t* caster, const vec3_t start_pos, const vec3_t aim_angles, const vec3_t aim_dir) //TODO: remove 'start_pos' arg, rename to SpellCastInsectGlobeOfOuchiness.
{
	// Spawn the globe of annihilation as an invisible entity (i.e. modelindex = 0).
	edict_t* globe = G_Spawn();

	VectorCopy(caster->s.origin, globe->s.origin);

	globe->s.origin[0] += aim_dir[0] * 20.0f;
	globe->s.origin[1] += aim_dir[1] * 20.0f;
	globe->s.origin[2] += (float)caster->viewheight - 5.0f;

	vectoangles(aim_angles, globe->s.angles);

	globe->avelocity[YAW] = 100.0f;
	globe->avelocity[ROLL] = 100.0f;

	globe->svflags |= SVF_ALWAYS_SEND;
	globe->s.effects |= (EF_ALWAYS_ADD_EFFECTS | EF_MARCUS_FLAG1 | EF_CAMERA_NO_CLIP);
	globe->s.scale = 1.0f;
	globe->owner = caster;
	globe->enemy = caster->enemy;
	globe->classname = "Spell_GlobeOfOuchiness";
	globe->dmg = 0;
	globe->count = 0;
	globe->clipmask = MASK_MONSTERSOLID;
	globe->movetype = PHYSICSTYPE_FLY;
	globe->solid = SOLID_NOT;

	globe->think = InsectGlobeOfOuchinessGrowThink;
	globe->nextthink = level.time + FRAMETIME; //mxd. Use define.

	G_LinkMissile(globe);

	vec3_t temp_vec = { caster->s.number, 0.0f, 0.0f }; // Caster entnum for FX_I_GLOW.
	gi.CreateEffect(&globe->s, FX_I_EFFECTS, CEF_OWNERS_ORIGIN, vec3_origin, "bv", FX_I_GLOBE, temp_vec);
	gi.CreateEffect(&globe->s, FX_I_EFFECTS, CEF_OWNERS_ORIGIN, vec3_origin, "bv", FX_I_GLOW, temp_vec);
}

#pragma endregion

#pragma region ========================== Insect spear projectile spell ==========================

static void InsectSpearProjectileTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surface);

// Guts of creating a spear projectile.
static void InsectSpearProjectileInit(edict_t* proj) //mxd. Named 'create_spearproj' in original logic.
{
	proj->classname = "Spell_SpearProj";
	proj->s.effects |= (EF_ALWAYS_ADD_EFFECTS | EF_CAMERA_NO_CLIP);
	proj->svflags |= SVF_ALWAYS_SEND;
	proj->movetype = MOVETYPE_FLYMISSILE;

	// Radius of zero seems to prevent collision between bolts.
	VectorClear(proj->mins);
	VectorClear(proj->maxs);

	proj->solid = SOLID_BBOX;
	proj->clipmask = MASK_SHOT;
	proj->touch = InsectSpearProjectileTouch;

	if (proj->count) // Powered projectile.
		proj->dmg = irand(TC_DMG_YSPEAR_MIN, TC_DMG_YSPEAR_MAX);
	else if (SKILL == SKILL_EASY)
		proj->dmg = TC_DMG_SPEAR_MIN;
	else if (SKILL == SKILL_MEDIUM)
		proj->dmg = irand(TC_DMG_SPEAR_MIN, TC_DMG_SPEAR_MAX);
	else // HARD, HARD+
		proj->dmg = TC_DMG_SPEAR_MAX;
}

edict_t* SpearProjReflect(edict_t* self, edict_t* other, vec3_t vel) //TODO: rename to InsectSpearProjectileReflect.
{
	edict_t* proj = G_Spawn();

	InsectSpearProjectileInit(proj);

	VectorCopy(self->s.origin, proj->s.origin);
	VectorCopy(vel, proj->velocity);
	VectorNormalize2(proj->velocity, proj->movedir);
	AnglesFromDir(proj->movedir, proj->s.angles);

	proj->reflect_debounce_time = self->reflect_debounce_time - 1;
	proj->reflected_time = self->reflected_time;

	G_LinkMissile(proj);

	proj->health = self->health;
	proj->owner = other;
	proj->enemy = NULL;
	proj->ideal_yaw = self->ideal_yaw;
	proj->random = self->random;
	proj->delay = self->delay;
	proj->count = self->count;
	proj->red_rain_count = self->red_rain_count;
	proj->think = self->think;
	proj->nextthink = self->nextthink;

	if (proj->health == 1)
		gi.CreateEffect(&proj->s, FX_I_EFFECTS, CEF_OWNERS_ORIGIN, NULL, "bv", FX_I_SPEAR2, vec3_origin);
	else
		gi.CreateEffect(&proj->s, FX_I_EFFECTS, CEF_OWNERS_ORIGIN, vec3_origin, "bv", FX_I_SPEAR, proj->velocity);

	// Kill the existing missile, since its a pain in the ass to modify it so the physics won't screw it. 
	G_SetToFree(self);

	return proj;
}

static void InsectSpearProjectileTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surface) //mxd. Named 'SpearProjTouch' in original logic.
{
	// Did we hit the sky ? 
	if (surface != NULL && (surface->flags & SURF_SKY))
	{
		SkyFly(self);
		return;
	}

	// Did we hit someone where reflection is functional?
	if (self->reflect_debounce_time > 0 && EntReflecting(other, true, true))
	{
		Create_rand_relect_vect(self->velocity, self->velocity);
		Vec3ScaleAssign(INSECT_SPEAR_PROJECTILE_SPEED / 2.0f, self->velocity);
		SpearProjReflect(self, other, self->velocity);

		return;
	}

	if (other->takedamage != DAMAGE_NO)
	{
		if (level.fighting_beast)
		{
			if (other->classID == CID_TBEAST)
			{
				other->enemy = self->owner;
			}
			else if (other->classID == CID_BBRUSH)
			{
				self->dmg = 0;
				VectorMA(self->s.origin, -4.0f, self->movedir, self->s.origin);
			}
		}

		if (self->dmg > 0) //HACK = so can't collapse trial beast bridge.
			T_Damage(other, self, self->owner, self->movedir, self->s.origin, plane->normal, self->dmg, 0, DAMAGE_SPELL, MOD_SPEAR);
	}
	else
	{
		// Back off the origin for the damage a bit. We are a point and this will help fix hitting base of a stair and not hurting a guy on next step up.
		VectorMA(self->s.origin, -4.0f, self->movedir, self->s.origin);
	}

	const byte fx_flags = (IsDecalApplicable(other, self->s.origin, surface, plane, NULL) ? CEF_FLAG6 : 0);
	const int fx_type = (self->count ? FX_I_SP_MSL_HIT2 : FX_I_SP_MSL_HIT); //mxd
	gi.CreateEffect(&self->s, FX_I_EFFECTS, fx_flags, vec3_origin, "bv", fx_type, self->movedir);

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
		MorcalavinProjectileHomeIn(self);

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
		VectorScale(forward, INSECT_SPEAR_PROJECTILE_SPEED, spearproj->velocity);
	}
	else
	{
		//Check ahead first to see if it's going to hit anything at this angle
		AngleVectors(AimAngles, forward, NULL, NULL);
		VectorMA(StartPos, INSECT_SPEAR_PROJECTILE_SPEED, forward, endpos);
		gi.trace(StartPos, vec3_origin, vec3_origin, endpos, caster, MASK_MONSTERSOLID,&trace);
		if(trace.ent && OkToAutotarget(caster, trace.ent))
		{//already going to hit a valid target at this angle- so don't autotarget
			VectorScale(forward, INSECT_SPEAR_PROJECTILE_SPEED, spearproj->velocity);
		}
		else
		{//autotarget current enemy
			GetAimVelocity(caster->enemy, spearproj->s.origin, INSECT_SPEAR_PROJECTILE_SPEED, AimAngles, spearproj->velocity);
		}
	}

	spearproj->owner = caster;
	VectorNormalize2(spearproj->velocity, spearproj->movedir);
	InsectSpearProjectileInit(spearproj);
	spearproj->reflect_debounce_time = MAX_REFLECT;

	G_LinkMissile(spearproj); 

	gi.trace(spearproj->s.origin, vec3_origin, vec3_origin, spearproj->s.origin, caster, MASK_PLAYERSOLID,&trace);
	if (trace.startsolid)
	{
		InsectSpearProjectileTouch(spearproj, trace.ent, &trace.plane, trace.surface);
		return;
	}

	if(caster->spawnflags & MSF_INSECT_YELLOWJACKET)
	{
		spearproj->think = yellowjacket_proj_think;
		spearproj->nextthink = level.time + 0.1;
		spearproj->enemy = caster->enemy;
		Vec3ScaleAssign(0.5, spearproj->velocity);
		spearproj->ideal_yaw = INSECT_SPEAR_PROJECTILE_SPEED/2;
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

#pragma endregion