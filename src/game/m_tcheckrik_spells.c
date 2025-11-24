//
// m_tcheckrik_spells.c
//
// Copyright 1998 Raven Software
//

#include "m_tcheckrik_spells.h" //mxd
#include "g_combat.h" //mxd
#include "g_playstats.h"
#include "m_stats.h"
#include "Decals.h"
#include "Vector.h"
#include "Random.h"
#include "Utilities.h"

#define INSECT_GLOBE_MAX_SCALE			1.8f //mxd. Named 'GLOBE_MAX_SCALE' in original logic.
#define INSECT_SPEAR_PROJECTILE_SPEED	600.0f //mxd. Named 'SPEARPROJ_SPEED' in original logic.

#pragma region ========================== Insect staff bolt spell ==========================

void InsectStaffBoltThink(edict_t* self) //mxd. Named 'InsectStaffThink' in original logic.
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

void InsectStaffBoltTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surface) //mxd. Named 'InsectStaffTouch' in original logic.
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

	if (self->insect_staff_bolt_powered)
	{
		gi.sound(self, CHAN_BODY, gi.soundindex("monsters/imp/fbfire.wav"), 1.0f, ATTN_NORM, 0.0f);
		gi.CreateEffect(&self->s, FX_M_EFFECTS, CEF_OWNERS_ORIGIN, self->s.origin, "bv", FX_IMP_FBEXPL, vec3_origin);
	}
	else
	{
		// Attempt to apply a scorchmark decal to the thing I hit.
		const byte fx_flags = (IsDecalApplicable(other, self->s.origin, surface, plane, NULL) ? CEF_FLAG6 : 0);
		gi.CreateEffect(NULL, FX_I_EFFECTS, fx_flags, self->s.origin, "bv", FX_I_ST_MSL_HIT, self->movedir);
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
	bolt->insect_staff_bolt_powered = power;

	G_LinkMissile(bolt);

	if (power)
	{
		Vec3ScaleAssign(2.0f, bolt->velocity);
		bolt->dmg *= 2;

		gi.CreateEffect(&bolt->s, FX_M_EFFECTS, CEF_OWNERS_ORIGIN, NULL, "bv", FX_IMP_FIRE, bolt->velocity);
	}
	else
	{
		gi.CreateEffect(&bolt->s, FX_I_EFFECTS, CEF_OWNERS_ORIGIN, vec3_origin, "bv", FX_I_STAFF, vec3_origin);
	}
}

#pragma endregion

#pragma region ========================== Insect globe of ouchiness spell ==========================

void InsectGlobeOfOuchinessGrowThink(edict_t* self) //mxd. Named 'GlobeOfOuchinessGrowThink' in original logic.
{
	if (self->owner->s.effects & EF_DISABLE_EXTRA_FX)
	{
		gi.RemoveEffects(&self->s, FX_REMOVE_EFFECTS);
		G_FreeEdict(self);

		return;
	}

	if (!self->owner->tcheckrik_globe_spell_released)
	{
		self->insect_globe_grow_counter += irand(1, 2);

		if (self->insect_globe_grow_counter > 10 && self->s.scale < INSECT_GLOBE_MAX_SCALE)
		{
			if (self->insect_globe_grow_counter > 20)
				self->s.scale -= 0.01f;
			else
				self->s.scale += 0.1f;

			if (self->insect_globe_grow_counter > 25)
				self->insect_globe_grow_counter &= 3;
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
	globe->insect_globe_grow_counter = 0;
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

	if (proj->insect_tracking_projectile_track_chance > 0) // Tracking projectile.
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

	proj->is_insect_tracking_projectile = self->is_insect_tracking_projectile;
	proj->owner = other;
	proj->enemy = NULL;
	proj->ideal_yaw = self->ideal_yaw;
	proj->insect_tracking_projectile_veer_amount = self->insect_tracking_projectile_veer_amount;
	proj->insect_tracking_projectile_turn_speed = self->insect_tracking_projectile_turn_speed;
	proj->insect_tracking_projectile_track_chance = self->insect_tracking_projectile_track_chance;
	proj->red_rain_count = self->red_rain_count;
	proj->think = self->think;
	proj->nextthink = self->nextthink;

	if (proj->is_insect_tracking_projectile)
		gi.CreateEffect(&proj->s, FX_I_EFFECTS, CEF_OWNERS_ORIGIN, NULL, "bv", FX_I_SPEAR2, vec3_origin);
	else
		gi.CreateEffect(&proj->s, FX_I_EFFECTS, CEF_OWNERS_ORIGIN, vec3_origin, "bv", FX_I_SPEAR, proj->velocity);

	// Kill the existing missile, since its a pain in the ass to modify it so the physics won't screw it. 
	G_SetToFree(self);

	return proj;
}

void InsectSpearProjectileTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surface) //mxd. Named 'SpearProjTouch' in original logic.
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
	const int fx_type = (self->insect_tracking_projectile_track_chance > 0 ? FX_I_SP_MSL_HIT2 : FX_I_SP_MSL_HIT); //mxd
	gi.CreateEffect(&self->s, FX_I_EFFECTS, fx_flags, vec3_origin, "bv", fx_type, self->movedir);

	G_SetToFree(self);
}

static void InsectTrackingSpearProjectileHomeIn(edict_t* self) //mxd. Named 'projectile_homethink' in original logic, defined in m_morcalavin.c.
{
	vec3_t old_dir;
	VectorNormalize2(self->velocity, old_dir);

	vec3_t hunt_dir;
	VectorSubtract(self->enemy->s.origin, self->s.origin, hunt_dir);
	VectorNormalize(hunt_dir);

	const float old_vel_mult = ((self->insect_tracking_projectile_turn_speed != 0.0f) ? self->insect_tracking_projectile_turn_speed : 1.3f);
	Vec3ScaleAssign(old_vel_mult, old_dir);

	vec3_t new_dir;
	VectorAdd(old_dir, hunt_dir, new_dir);

	float new_vel_div = 1.0f / (old_vel_mult + 1.0f);
	Vec3ScaleAssign(new_vel_div, new_dir);

	float speed_mod = DotProduct(old_dir, new_dir);
	speed_mod = max(0.05f, speed_mod);

	new_vel_div *= self->ideal_yaw * speed_mod;

	Vec3ScaleAssign(old_vel_mult, old_dir);
	VectorAdd(old_dir, hunt_dir, new_dir);

	VectorScale(new_dir, new_vel_div, self->velocity);

	if (self->insect_tracking_projectile_veer_amount != 0.0f) //mxd. Inline projectile_veer().
	{
		// Useful code for making projectiles wander randomly to a specified degree.
		const float speed = VectorLength(self->velocity);

		vec3_t veer_dir;
		VectorRandomSet(veer_dir, self->insect_tracking_projectile_veer_amount);

		Vec3AddAssign(veer_dir, self->velocity);
		VectorNormalize(self->velocity);

		Vec3ScaleAssign(speed, self->velocity);
	}
}

// This function will make a projectile wander from it's course in a random manner.
void InsectTrackingSpearProjectileThink(edict_t* self) //mxd. Named 'yellowjacket_proj_think' in original logic.
{
	// No enemy, stop tracking.
	if (self->enemy == NULL)
	{
		self->think = NULL;
		return;
	}

	vec3_t dir;
	VectorCopy(self->velocity, dir);
	VectorNormalize(dir);

	vec3_t enemy_dir;
	VectorSubtract(self->enemy->s.origin, self->s.origin, enemy_dir);
	VectorNormalize(enemy_dir);

	if (DotProduct(enemy_dir, dir) > 0.0f && irand(2, 24) > self->insect_tracking_projectile_track_chance)
		InsectTrackingSpearProjectileHomeIn(self);

	self->insect_tracking_projectile_track_chance++;

	if (self->insect_tracking_projectile_veer_amount < 100.0f)
		self->insect_tracking_projectile_veer_amount += 10.0f;

	self->nextthink = level.time + FRAMETIME; //mxd. Use define.
}

void SpellCastInsectSpear(edict_t* caster, const vec3_t start_pos, const vec3_t aim_angles, const int offset)
{
	// Spawn the magic-missile.
	if (caster->enemy == NULL)
		return;

	edict_t* proj = G_Spawn();

	VectorCopy(start_pos, proj->s.origin);

	vec3_t diff;
	VectorSubtract(caster->enemy->s.origin, start_pos, diff);

	if (offset > 0 && VectorLength(diff) > 128.0f)
	{
		vec3_t forward;
		vec3_t right;
		vec3_t up;
		AngleVectors(aim_angles, forward, right, up);

		switch (offset)
		{
			default:
			case 1:
				VectorAverage(forward, right, forward);
				break;

			case 2:
				VectorInverse(right);
				VectorAverage(forward, right, forward);
				break;

			case 3:
				VectorAverage(forward, up, forward);
				break;
		}

		VectorScale(forward, INSECT_SPEAR_PROJECTILE_SPEED, proj->velocity);
	}
	else
	{
		// Check ahead first to see if it's going to hit anything at this angle.
		vec3_t forward;
		AngleVectors(aim_angles, forward, NULL, NULL);

		vec3_t end_pos;
		VectorMA(start_pos, INSECT_SPEAR_PROJECTILE_SPEED, forward, end_pos);

		trace_t trace;
		gi.trace(start_pos, vec3_origin, vec3_origin, end_pos, caster, MASK_MONSTERSOLID, &trace);

		if (trace.ent != NULL && OkToAutotarget(caster, trace.ent))
			VectorScale(forward, INSECT_SPEAR_PROJECTILE_SPEED, proj->velocity); // Already going to hit a valid target at this angle, so don't autotarget.
		else
			GetAimVelocity(caster->enemy, proj->s.origin, INSECT_SPEAR_PROJECTILE_SPEED, aim_angles, proj->velocity); // Autotarget current enemy.
	}

	proj->owner = caster;
	VectorNormalize2(proj->velocity, proj->movedir);
	InsectSpearProjectileInit(proj);
	proj->reflect_debounce_time = MAX_REFLECT;

	G_LinkMissile(proj);

	trace_t trace;
	gi.trace(proj->s.origin, vec3_origin, vec3_origin, proj->s.origin, caster, MASK_PLAYERSOLID, &trace);

	if (trace.startsolid)
	{
		InsectSpearProjectileTouch(proj, trace.ent, &trace.plane, trace.surface);
		return;
	}

	if (caster->spawnflags & MSF_INSECT_YELLOWJACKET)
	{
		proj->enemy = caster->enemy;
		Vec3ScaleAssign(0.5f, proj->velocity);
		proj->ideal_yaw = INSECT_SPEAR_PROJECTILE_SPEED / 2.0f;
		proj->insect_tracking_projectile_veer_amount = 30.0f;
		proj->insect_tracking_projectile_turn_speed = 1.5f;
		proj->insect_tracking_projectile_track_chance = 1;
		proj->is_insect_tracking_projectile = true; // To indicate the homing projectile (affects FX only -- mxd).
		proj->red_rain_count = 1; //TODO: unused?

		proj->think = InsectTrackingSpearProjectileThink;
		proj->nextthink = level.time + FRAMETIME; //mxd. Use define.

		gi.CreateEffect(&proj->s, FX_I_EFFECTS, CEF_OWNERS_ORIGIN, NULL, "bv", FX_I_SPEAR2, vec3_origin);
	}
	else
	{
		proj->insect_tracking_projectile_track_chance = 0;
		gi.CreateEffect(&proj->s, FX_I_EFFECTS, CEF_OWNERS_ORIGIN, vec3_origin, "bv", FX_I_SPEAR, proj->velocity);
	}
}

#pragma endregion