//
// g_Physics.c
//
// Copyright 1998 Raven Software
//

#include "g_Physics.h"
#include "g_PhysicsLocal.h" //mxd
#include "g_local.h"
#include "g_combat.h" //mxd
#include "q_Physics.h"
#include "p_main.h"
#include "p_anims.h"
#include "FX.h"
#include "Random.h"
#include "SinglyLinkedList.h"
#include "Vector.h"
#include "Utilities.h"

#define FRICTION_STOPSPEED	100.0f
#define FRICTION_SURFACE	6.0f

//FIXME: a high detail option? Or just not in netplay?
// - maybe a flag for client to take care of disabling for now since it might cause too much net traffic.
void PhysicsCheckWaterTransition(edict_t* self)
{
	if (DEATHMATCH || COOP)
		return;

	// Check for water transition.
	const qboolean wasinwater = (self->watertype & MASK_WATER);

	self->watertype = gi.pointcontents(self->s.origin);
	const qboolean isinwater = (self->watertype & MASK_WATER);

	trace_t trace;
	if (!wasinwater && isinwater)
		gi.trace(self->s.old_origin, vec3_origin, vec3_origin, self->s.origin, self, MASK_WATER, &trace);
	else if (wasinwater && !isinwater)
		gi.trace(self->s.origin, vec3_origin, vec3_origin, self->s.old_origin, self, MASK_WATER, &trace);
	else // wasinwater == isinwater.
		return;

	if (trace.fraction == 1.0f)
		return;

	//FIXME: just put a flag on them and do the effect on the other side?
	int size = (int)(ceilf(VectorLength(self->size) + VectorLength(self->velocity) / 10.0f));
	size = ClampI(size, 10, 255);

	gi.CreateEffect(NULL, FX_WATER_ENTRYSPLASH, CEF_FLAG6 | CEF_FLAG7, trace.endpos, "bd", size, trace.plane.normal);
}

static void Physics_NoclipMove(edict_t* self)
{
	VectorMA(self->s.angles, FRAMETIME, self->avelocity, self->s.angles);
	VectorMA(self->s.origin, FRAMETIME, self->velocity, self->s.origin);

	gi.linkentity(self);
}

static void Physics_FlyMove(edict_t* self)
{
	FormMove_t form_move;

	if (self->physicsFlags & PF_RESIZE)
	{
		if (!gi.ResizeBoundingForm(self, &form_move))
			return; // If an ent can't be resized, then it probably can't be moved either.

		self->physicsFlags &= ~PF_RESIZE;
	}

	// Update angles.
	VectorMA(self->s.angles, FRAMETIME, self->avelocity, self->s.angles);

	if (!BoundVelocity(self->velocity) && self->gravity <= 0.0f)
		return;

	if (self->velocity[2] > 0.0f)
		self->groundentity = NULL;
	else if (self->groundentity != NULL)
		return; // On ground, return without moving.

	VectorCopy(self->mins, form_move.mins);
	VectorCopy(self->maxs, form_move.maxs);

	form_move.pass_entity = self;
	form_move.clipmask = self->clipmask;

	MoveEntity_Bounce(self, &form_move);
	PhysicsCheckWaterTransition(self);

	gi.linkentity(self);
	ActivateTriggers(self);
}

// Monsters freefall when they don't have a ground entity, otherwise all movement is done with discrete steps.
// This is also used for objects that have become still on the ground, but will fall if the floor is pulled out from under them.
static void Physics_StepMove(edict_t* self)
{
	assert(self->gravity >= 0.0f);

	const qboolean has_velocity = Vec3NotZero(self->velocity);

	// Apply rotation friction if desired.
	if (self->physicsFlags & PF_ROTATIONAL_FRICTION)
	{
		if (has_velocity)
			ApplyRotationalFriction(self);
	}
	else
	{
		VectorMA(self->s.angles, FRAMETIME, self->avelocity, self->s.angles);
	}

	FormMove_t form_move;
	if ((self->physicsFlags & PF_RESIZE) && gi.ResizeBoundingForm(self, &form_move))
		self->physicsFlags &= ~PF_RESIZE;

	const float gravity = self->gravity * sv_gravity->value;

	// Check for submersion or nograv.
	if (self->waterlevel < 2 && gravity > 0.0f)
	{
		if (self->groundentity != NULL)
		{
			const float friction = self->friction * sv_friction->value;

			if (!has_velocity && self->groundNormal[2] >= GROUND_NORMAL && self->groundNormal[2] >= gravity / (friction + gravity))
				return; // Not going anywhere without velocity on ground whose slope this ent won't slide on.
		}

		MoveEntity_Slide(self);
	}
	else
	{
		if (!has_velocity)
			return; // Not going anywhere without velocity.

		VectorCopy(self->mins, form_move.mins);
		VectorCopy(self->maxs, form_move.maxs);

		form_move.pass_entity = self;
		form_move.clipmask = self->clipmask;

		MoveEntity_Bounce(self, &form_move);
	}

	if (!BoundVelocity(self->velocity))
	{
		if (has_velocity)
			QPostMessage(self, G_MSG_RESTSTATE, PRI_PHYSICS, "i", has_velocity); // Stopped moving.
	}
	else if (!has_velocity)
	{
		QPostMessage(self, G_MSG_RESTSTATE, PRI_PHYSICS, "i", has_velocity); // Started moving.
	}

	PhysicsCheckWaterTransition(self);

	gi.linkentity(self);
	ActivateTriggers(self);
}

void EntityPhysics(edict_t* self)
{
	assert(self->inuse);

	switch (self->movetype)
	{
		case PHYSICSTYPE_NONE:
		case PHYSICSTYPE_STATIC:
			break;

		case PHYSICSTYPE_NOCLIP:
			Physics_NoclipMove(self);
			break;

		case PHYSICSTYPE_FLY:
		case MOVETYPE_FLYMISSILE:
			Physics_FlyMove(self);
			break;

		case PHYSICSTYPE_STEP:
			Physics_StepMove(self);
			break;

		case PHYSICSTYPE_PUSH:
		case PHYSICSTYPE_STOP:
			Physics_Push(self);
			break;

		case PHYSICSTYPE_SCRIPT_ANGULAR:
			Physics_ScriptAngular(self);
			break;

		default:
			gi.error("SV_Physics: bad movetype %i", self->movetype);
			break;
	}
}

// Determines which entity if any the self is on.
// Also zeros z vel and moves onto ground if it was in the air.
void CheckEntityOn(edict_t* self)
{
	assert(self);

	if (self->velocity[2] > Z_VEL_NOT_ONGROUND)
	{
		self->groundentity = NULL;
		return;
	}

	// If the hull point one-quarter unit down is solid, the entity is on ground.
	vec3_t point;
	VectorCopy(self->s.origin, point);
	point[2] -= PHYSICS_Z_FUDGE + CHECK_BELOW_DIST;

	FormMove_t form_move;
	VectorCopy(self->mins, form_move.mins);
	VectorCopy(self->maxs, form_move.maxs);

	form_move.start = self->s.origin;
	form_move.end = point;
	form_move.pass_entity = self;
	form_move.clipmask = MASK_MONSTERSOLID;

	gi.TraceBoundingForm(&form_move);

	// Check steepness.
	if (form_move.trace.ent == NULL || (form_move.trace.plane.normal[2] < GROUND_NORMAL && !form_move.trace.startsolid))
	{
		self->groundentity = NULL;
		return;
	}

	if (!form_move.trace.startsolid && !form_move.trace.allsolid)
	{
		VectorCopy(form_move.trace.endpos, self->s.origin);
		SetGroundEntFromTrace(self, &form_move.trace);
	}
}

// Set move to be based on gravity and velocity, and adjust velocity for gravity.
static void ApplyGravity(edict_t* self, vec3_t move)
{
	assert(self);

	if (move != NULL)
		move[2] -= self->gravity * sv_gravity->value * (FRAMETIME * FRAMETIME * 0.5f);

	self->velocity[2] -= self->gravity * sv_gravity->value * FRAMETIME;
}

// Make things that hit each other do some damage.
void DoImpactDamage(edict_t* self, trace_t* trace)
{
	if (self->impact_debounce_time > level.time || (self->svflags & SVF_DO_NO_IMPACT_DMG))
		return;

	if (self->client != NULL && (self->client->playerinfo.edictflags & FL_CHICKEN)) // Chicken is not very impactful...
		return;

	// Skip when not a player or monster, and has no mass.
	if (self->client == NULL && !(self->svflags & SVF_MONSTER) && self->mass <= 0)
		return;

	const float speed = VectorLength(self->velocity);

	if (speed < 50.0f || (speed < 500.0f && self->watertype > 0))
		return;

	float impact_dmg = sqrtf(speed / 10.0f);

	// Monsters don't do impact damage to their own type.
	if (self->classID != CID_NONE && self->classID == trace->ent->classID)
		return;

	vec3_t normal;
	if (Vec3NotZero(trace->plane.normal)) //BUGFIX: mxd. 2 always true checks in original version.
		VectorCopy(trace->plane.normal, normal);
	else
		VectorCopy(vec3_up, normal);

	vec3_t move_dir;
	if (Vec3NotZero(self->velocity))
	{
		VectorCopy(self->velocity, move_dir);
		VectorNormalize(move_dir);
	}
	else
	{
		VectorCopy(vec3_up, move_dir);
	}

	float other_health;

	if (trace->ent->solid == SOLID_BSP)
	{
		if (self->health > 0)
			impact_dmg = impact_dmg * (float)self->health / 100.0f;
		else if (speed < 300.0f)
			return;

		if ((trace->ent->takedamage == DAMAGE_NO && self->health > 100) || self->health <= 0)
			other_health = (float)self->health * 10.0f; //TODO: err, self->health CAN be negative!
		else
			other_health = 1000.0f;
	}
	else if (trace->ent->health > 0)
	{
		other_health = (float)trace->ent->health * 0.5f;
	}
	else
	{
		other_health = 1.0f;
	}

	float self_health = 2.0f;
	if (self->health > 0)
		self_health *= (float)self->health;

	const float total_health = self_health + other_health;

	float other_damage; //mxd. int in original version.
	if (trace->ent->solid == SOLID_BSP && trace->ent->takedamage == DAMAGE_NO)
		other_damage = 0.0f;
	else
		other_damage = floorf(impact_dmg * self_health / total_health);

	float self_damage = floorf(impact_dmg - other_damage); //mxd. int in original version.

	// Damage other.
	if (other_damage >= 1.0f && trace->ent->takedamage != DAMAGE_NO && !(trace->ent->svflags & SVF_TAKE_NO_IMPACT_DMG) && !(trace->ent->svflags & SVF_BOSS))
	{
		if (SKILL < SKILL_HARD && (self->svflags & SVF_MONSTER) && trace->ent->client != NULL)
			other_damage = ceilf(other_damage * 0.5f); // Monsters do a bit less damage to player on normal and easy skill.

		if (other_damage >= 1.0f)
		{
			T_Damage(trace->ent, self, self, move_dir, trace->endpos, normal, (int)other_damage, (int)other_damage, 0, MOD_CRUSH);

			// Knokdown player?
			if (trace->ent->health > 0 && trace->ent->client != NULL && other_damage > flrand(25.0f, 40.0f) - (5.0f * skill->value))
			{
				if (trace->ent->client->playerinfo.lowerseq != ASEQ_KNOCKDOWN)
					P_KnockDownPlayer(&trace->ent->client->playerinfo);
			}
		}
	}

	// Damage self.
	if (self_damage >= 1.0f && self->takedamage != DAMAGE_NO && (speed > 600.0f || self->health <= 0) && !(self->svflags & SVF_TAKE_NO_IMPACT_DMG) && !(self->svflags & SVF_BOSS) && self->jump_time < level.time)
	{
		if (SKILL > SKILL_EASY && self->client != NULL && trace->ent->solid == SOLID_BSP)
			self_damage = floorf((float)self->dmg * 1.5f); // More damage to player from falls. //TODO: why self->dmg?.. Should be self_damage * 1.5 instead?

		if ((self_damage >= 3.0f && (self->classID != CID_ASSASSIN && self->classID != CID_SSITHRA)) || self->health <= 0) // But what about ring of repulsion?
		{
			int flags = DAMAGE_AVOID_ARMOR; //mxd
			if (self_damage < 5.0f)
				flags |= DAMAGE_NO_BLOOD;

			T_Damage(self, self, self, move_dir, trace->endpos, normal, (int)self_damage, 0, flags, MOD_FALLING);
			self->impact_debounce_time = level.time + 0.3f; // Don't collide again too soon.
		}
	}
}

// Calls correct collision handling functions for ents involved.
static void HandleCollision(edict_t* self, trace_t* trace, const vec3_t move, const qboolean forceful, const int flags) //mxd. int forceful -> qboolean forceful.
{
	edict_t* other = trace->ent;

	if (IMPACT_DAMAGE && (flags & CH_ISBLOCKED || flags & CH_BOUNCED))
		DoImpactDamage(self, trace);

	if (forceful)
		HandleForcefulCollision(self, other, move, forceful);

	if ((flags & CH_ISBLOCKED) && self->isBlocked)
		self->isBlocked(self, trace);

	if ((flags & CH_BOUNCED) && self->bounced)
		self->bounced(self, trace);

	if ((flags & CH_ISBLOCKING) && other->isBlocking)
	{
		trace_t temp = *trace;

		temp.ent = self;
		VectorInverse(temp.plane.normal);

		other->isBlocking(other, &temp);
	}
}

static void HandleForcefulCollision(edict_t* forcer, edict_t* forcee, const vec3_t move, const qboolean forceful) //mxd. int forceful -> qboolean forceful.
{
	assert(forcee);

	const qboolean hit_world = (forcee == world);
	const float vel_scaler = FRAMES_PER_SECOND * (hit_world ? -0.8f : 0.5f); //mxd

	vec3_t vel;
	VectorScale(move, vel_scaler, vel);

	vec3_t dir;
	float knockback = VectorNormalize2(vel, dir);
	knockback *= (float)forcer->mass / KNOCK_BACK_MULTIPLIER;

	// Knock other entity back?
	if (!hit_world)
	{
		PostKnockBack(forcee, dir, knockback, 0);
		Vec3ScaleAssign(-1.0f, dir);
	}

	// Knock back running ent?
	if (forceful)
		PostKnockBack(forcer, dir, knockback, 0);
}

// Takes into account gravity, and bounces an ent away from impacts based on elasticity. Fiction is ignored.
static void MoveEntity_Bounce(edict_t* self, FormMove_t* form_move)
{
	vec3_t move;
	VectorScale(self->velocity, FRAMETIME, move);

	// Create the delta move.
	if (self->gravity > 0.0f)
		ApplyGravity(self, move);

	vec3_t end;
	VectorAdd(self->s.origin, move, end);

	form_move->start = self->s.origin;
	form_move->end = end;

	gi.TraceBoundingForm(form_move);

	VectorCopy(form_move->trace.endpos, self->s.origin);

	// Handle bouncing and sliding.
	if (form_move->trace.fraction < 1.0f)
	{
		BounceVelocity(self->velocity, form_move->trace.plane.normal, self->velocity, self->elasticity);

		if (self->elasticity > ELASTICITY_SLIDE && self->velocity[2] < 60.0f && form_move->trace.plane.normal[2] > GROUND_NORMAL)
		{
			SetGroundEntFromTrace(self, &form_move->trace);

			VectorClear(self->velocity);
			VectorClear(self->avelocity);
		}

		HandleCollision(self, &form_move->trace, move, (self->physicsFlags & PF_FORCEFUL_COLLISIONS), CH_STANDARD);
	}
}

// Moves an entity sliding or bouncing of off any planes collided with.
static void MoveEntity_Slide(edict_t* self)
{
#define MAX_CLIP_PLANES					5
#define MAX_BUMPS						4
#define STEEP_SLOPE_FRICTION_MODIFIER	0.1f

	static vec3_t planes[MAX_CLIP_PLANES];

	vec3_t dir;
	vec3_t gravity_dir;
	vec3_t new_velocity;
	vec3_t delta;

	assert(self);
	assert(self->clipmask);

	const float gravity = self->gravity * sv_gravity->value;
	const float base_friction = self->friction * sv_friction->value;

	vec3_t original_velocity;
	VectorCopy(self->velocity, original_velocity);

	vec3_t primal_velocity;
	VectorCopy(self->velocity, primal_velocity);

	float* ground_normal = self->groundNormal;

	if (self->groundentity == NULL)
		ground_normal[2] = 0.0f;

	FormMove_t form_move;
	VectorCopy(self->mins, form_move.mins);
	VectorCopy(self->maxs, form_move.maxs);

	form_move.start = self->s.origin;
	form_move.pass_entity = self;
	form_move.clipmask = self->clipmask;

	int cur_plane = 0; //mxd. Initialize.
	int num_planes = 0;
	int fudge_index = -1;
	float time_remaining = FRAMETIME;
	float dist = 0.0f; //mxd. Initialize.
	float fwd_accel = 0.0f; //mxd. Initialize.
	float grav_accel = 0.0f; //mxd. Initialize.

	for (int bumpcount = 0; bumpcount < MAX_BUMPS; bumpcount++)
	{
		const float friction = base_friction;
		const float time_remaining_sq = time_remaining * time_remaining;
		qboolean slide = false;

		VectorScale(self->velocity, time_remaining, delta);

		// On some type of upward facing slope (otherwise fall straight down, no surface friction needed).
		if (ground_normal[2] > 0.0f)
		{
			// No velocity.
			if (Vec3IsZero(self->velocity))
			{
				// Can't slide -> not going anywhere.
				if (ground_normal[2] >= GROUND_NORMAL && (ground_normal[2] >= gravity / (friction + gravity)) && bumpcount > 0)
					return;

				// (|gravity| * ground_normal) * ground_normal yields the vector in the direction of gravity applied to the the slope of ground_normal.
				gravity_dir[0] = ground_normal[0] * ground_normal[2];
				gravity_dir[1] = ground_normal[1] * ground_normal[2];
				gravity_dir[2] = -ground_normal[0] * ground_normal[0] - ground_normal[1] * ground_normal[1];

				VectorNormalize(gravity_dir);

				const float dot = DotProduct(gravity_dir, ground_normal);

				if (dot < -FLOAT_ZERO_EPSILON)
				{
					// Floating point error, fudge it away from the plane a bit.
					fudge_index = cur_plane;
					VectorMA(self->s.origin, PHYSICS_Z_FUDGE, planes[cur_plane], self->s.origin);
				}

				dist = 0.0f;
				slide = true;
			}
			else
			{
				dist = VectorNormalize2(delta, dir);

				// dot < -0.05f: the trace will fail, try to restructure in order to skip it.
				// dot <  0.05f: parallel to ground.
				// dot >= 0.05f: pulling away from ground.
				const float dot = DotProduct(dir, ground_normal);
				slide = (fabsf(dot) < 0.05f); // Parallel to ground.
			}
		}

		if (slide)
		{
			// Moving along ground, apply gravity and friction.
			gravity_dir[0] = ground_normal[0] * ground_normal[2];
			gravity_dir[1] = ground_normal[1] * ground_normal[2];
			gravity_dir[2] = -ground_normal[0] * ground_normal[0] - ground_normal[1] * ground_normal[1];

			VectorNormalize(gravity_dir);

			fwd_accel = -friction * ground_normal[2];

			// Turn down friction on a steep slope, the theory being that something wouldn't be able to maintain good surface contact on such a slope.
			if (ground_normal[2] < GROUND_NORMAL)
				fwd_accel *= STEEP_SLOPE_FRICTION_MODIFIER;

			dist += 0.5f * fwd_accel * time_remaining_sq;

			if (dist < 0.0f)
			{
				dist = 0.0f;
				fwd_accel = 0.0f;
			}

			VectorScale(dir, dist, delta);

			const float dot = DotProduct(gravity_dir, ground_normal);

			if (dot < -FLOAT_ZERO_EPSILON)
			{
				// Floating point error, fudge it away from the plane a bit.
				fudge_index = cur_plane;
				VectorMA(self->s.origin, PHYSICS_Z_FUDGE, planes[cur_plane], self->s.origin);
			}

			grav_accel = gravity * (1 - ground_normal[2]) - friction * ground_normal[2];

			// Turn down friction on a steep slope, the theory being that something wouldn't be able to maintain good surface contact on such a slope.
			if (ground_normal[2] < GROUND_NORMAL)
				grav_accel *= STEEP_SLOPE_FRICTION_MODIFIER;

			grav_accel = max(0.0f, grav_accel);
			VectorMA(delta, grav_accel * 0.5f * time_remaining_sq, gravity_dir, delta);

			if (dist + fwd_accel + grav_accel == 0.0f)
			{
				VectorClear(self->velocity);
				return;
			}
		}
		else
		{
			// Not sliding, apply gravity straight down, no friction.
			delta[2] -= 0.5f * gravity * time_remaining_sq;
		}

		vec3_t end;
		VectorCopy(self->s.origin, end);
		Vec3AddAssign(delta, end);

		form_move.end = end;

		gi.TraceBoundingForm(&form_move);

		if (form_move.trace.startsolid)
		{
			if (fudge_index != -1)
			{
				// Undo fudge and let it try that again.
				VectorMA(self->s.origin, -PHYSICS_Z_FUDGE, planes[fudge_index], self->s.origin);
				continue;
			}

			VectorClear(self->velocity);
			return;
		}

		if (form_move.trace.allsolid)
		{
			// Entity is trapped in another solid.
			VectorClear(self->velocity);
			self->s.origin[2] += 20.0f;

			return;
		}

		const float time_moved = time_remaining * form_move.trace.fraction;

		if (form_move.trace.fraction > 0.0f)
		{
			// Actually covered some distance.
			VectorCopy(form_move.trace.endpos, self->s.origin);

			if (slide)
			{
				float speed = VectorNormalize2(self->velocity, dir);
				speed += fwd_accel * time_moved;

				VectorScale(dir, speed, self->velocity);
				VectorMA(self->velocity, grav_accel * time_moved, gravity_dir, self->velocity);
			}
			else
			{
				self->velocity[2] -= gravity * time_moved;
			}

			if (form_move.trace.fraction == 1.0f)
				break; // Moved the entire distance.

			VectorCopy(self->velocity, original_velocity);

			num_planes = 0;
			fudge_index = -1;
		}
		else if (Vec3IsZero(self->velocity) && self->groundNormal[2] >= gravity / (friction + gravity))
		{
			break; // No velocity and the trace failed, not going anywhere on ground the ent can't slide on.
		}

		// Results in isBlocked being called on the last bounced.
		const int flags = CH_BOUNCED | ((bumpcount == MAX_BUMPS - 1) ? CH_STANDARD : CH_ISBLOCKING); //mxd
		HandleCollision(self, &form_move.trace, delta, false, flags);

		VectorCopy(form_move.trace.plane.normal, ground_normal);

		if (ground_normal[2] > 0.0f && form_move.trace.ent->solid == SOLID_BSP) // Hit the floor.
		{
			self->groundentity = form_move.trace.ent;
		}
		else
		{
			ground_normal[2] = max(0.0f, ground_normal[2]);
			self->groundentity = NULL;
		}

		time_remaining -= time_moved;

		// Clipped to another plane.
		assert(num_planes < MAX_CLIP_PLANES);

		if (num_planes == 0 || !VectorCompare(form_move.trace.plane.normal, planes[num_planes - 1]))
		{
			VectorCopy(form_move.trace.plane.normal, planes[num_planes]);
			num_planes++;
		}
		else
		{
			fudge_index = num_planes - 1;
			VectorMA(self->s.origin, PHYSICS_Z_FUDGE, planes[fudge_index], self->s.origin);
		}

		// Modify original_velocity so it parallels all of the clip planes.
		for (cur_plane = 0; cur_plane < num_planes; cur_plane++)
		{
			assert(Vec3NotZero(planes[cur_plane]));
			BounceVelocity(original_velocity, planes[cur_plane], new_velocity, self->elasticity);
			const float dir_mag = VectorNormalize2(new_velocity, dir);

			if (FloatIsZeroEpsilon(dir_mag))
			{
				// Smacked into something exactly head on.
				if (planes[cur_plane][2] > 0.0f)
				{
					// Slide down slope.
					dir[0] = -planes[cur_plane][0] * planes[cur_plane][2];
					dir[1] = -planes[cur_plane][1] * planes[cur_plane][2];
					dir[2] = planes[cur_plane][0] * planes[cur_plane][0] + planes[cur_plane][1] * planes[cur_plane][1];

					VectorNormalize(dir);
				}
				else
				{
					// Drop straight down.
					dir[2] = -1.0f;
				}
			}

			const float dot = DotProduct(dir, planes[cur_plane]);

			if (dot < -FLOAT_ZERO_EPSILON)
			{
				// Floating point error, fudge it away from the plane a bit.
				fudge_index = cur_plane;
				VectorMA(self->s.origin, PHYSICS_Z_FUDGE, planes[cur_plane], self->s.origin);
			}

			slide = false;

			if (planes[cur_plane][2] > 0.0f)
			{
				// dot < -0.01f: the trace will fail, try to restructure in order to skip it.
				// dot <  0.01f: parallel to surface.
				// dot >= 0.01f: pulling away from surface.
				slide = (fabsf(dot) < 0.01f); // Parallel to surface.
			} // Otherwise, fall straight down, no surface friction needed.

			int p;
			for (p = 0; p < num_planes; p++)
				if (p != cur_plane && DotProduct(new_velocity, planes[p]) <= 0.0f)
					break; // Unacceptable slide.

			if (p == num_planes)
				break; // Acceptable slide.
		}

		if (cur_plane != num_planes)
		{
			VectorCopy(new_velocity, self->velocity); // Good slide.
		}
		else
		{
			// Go along the crease.
			assert(num_planes);

			if (num_planes != 2)
			{
				VectorClear(self->velocity);
				return;
			}

			CrossProduct(planes[0], planes[1], dir);
			VectorScale(dir, VectorLength(self->velocity), self->velocity);

			if (dir[2] <= 0.0f)
				slide = true;
		}

		float speed = VectorNormalize2(self->velocity, dir);

		if (slide)
		{
			// Moving along ground, apply gravity and friction.
			const float accel = -friction * (1.0f - dir[2]) - gravity * dir[2];
			speed += accel * time_remaining;
			VectorScale(dir, speed, new_velocity);
		}
		else
		{
			// Apply gravity straight down, no friction.
			VectorScale(dir, speed, new_velocity);
			new_velocity[2] -= gravity * time_remaining;
		}
	}

	if (form_move.trace.fraction < 1.0f)
	{
		HandleCollision(self, &form_move.trace, delta, false, CH_STANDARD);
		VectorClear(self->velocity);

		if (form_move.trace.plane.normal[2] > GROUND_NORMAL) // Hit the floor.
		{
			SetGroundEntFromTrace(self, &form_move.trace);
			return;
		}
	}

	CheckEntityOn(self);
}

// Searches for any triggers in the entities bounding box and calls their touch function.
static void ActivateTriggers(edict_t* self)
{
	// Dead things don't activate triggers.
	if (self->dead_state != DEAD_NO)
		return;

	SinglyLinkedList_t list;
	SLList_DefaultCon(&list); // This should be global, initialized at startup.
	gi.FindEntitiesInBounds(self->mins, self->maxs, &list, AREA_TRIGGERS);

	while (!SLList_IsEmpty(&list))
	{
		const GenericUnion4_t found = SLList_Pop(&list);
		edict_t* hit = found.t_edict_p;

		if (hit->inuse && hit->touch != NULL)
			hit->touch(hit, self, NULL, NULL);
	}

	SLList_Des(&list); // Kill on shut down.
}

static void ApplyRotationalFriction(edict_t* self)
{
	VectorMA(self->s.angles, FRAMETIME, self->avelocity, self->s.angles);
	const float adjustment = FRAMETIME * FRICTION_STOPSPEED * FRICTION_SURFACE;

	for (int i = 0; i < 3; ++i)
	{
		if (self->avelocity[i] > 0.0f)
			self->avelocity[i] = max(0.0f, self->avelocity[i] - adjustment);
		else
			self->avelocity[i] = min(0.0f, self->avelocity[i] + adjustment);
	}
}

// Sets the groundentity info contained in self based on trace.
static void SetGroundEntFromTrace(edict_t* self, const trace_t* trace)
{
	assert(self);
	assert(trace->plane.normal[2] > GROUND_NORMAL);

	self->groundentity = trace->ent;
	self->groundentity_linkcount = trace->ent->linkcount;
	VectorCopy(trace->plane.normal, self->groundNormal);
}

static edict_t* TestEntityPosition(const edict_t* self)
{
	FormMove_t form_move;

	VectorCopy(self->mins, form_move.mins);
	VectorCopy(self->maxs, form_move.maxs);

	form_move.start = self->s.origin;
	form_move.end = self->s.origin;
	form_move.pass_entity = self;
	form_move.clipmask = self->clipmask;

	gi.TraceBoundingForm(&form_move);

	if (form_move.trace.startsolid)
		return world;

	return NULL;
}

typedef struct
{
	edict_t* ent;
	vec3_t origin;
	vec3_t angles;
	float delta_yaw;
} pushed_t;

static pushed_t pushed[MAX_EDICTS];
static pushed_t* pushed_p;
static edict_t* obstacle;

#define MAX_MATRIX	3

static void BackSub(float a[][MAX_MATRIX], const int* indx, float* b, const int sz)
{
	int ii = -1;

	for (int i = 0; i < sz; i++)
	{
		const int ip = indx[i];

		float sum = b[ip];
		b[ip] = b[i];

		if (ii >= 0)
		{
			for (int j = ii; j < i; j++)
				sum -= a[i][j] * b[j];
		}
		else if (sum != 0.0f)
		{
			ii = i;
		}

		b[i] = sum;
	}

	for (int i = sz - 1; i >= 0; i--)
	{
		float sum = b[i];
		for (int j = i + 1; j < sz; j++)
			sum -= a[i][j] * b[j];

		b[i] = sum / a[i][i];
	}
}

#define EPS_MATRIX	(1E-15f) //TODO: outside of floating point precision, evaluates to 0.0f. Use FLT_EPSILON instead?

static qboolean LUDecomposition(float a[][MAX_MATRIX], int indx[], const int sz, float* d)
{
	float vv[MAX_MATRIX];

	*d = 1;

	for (int i = 0; i < sz; i++)
	{
		float big = 0.0f;

		for (int j = 0; j < sz; j++)
		{
			const float temp = fabsf(a[i][j]);
			big = max(big, temp);
		}

		if (big < EPS_MATRIX)
			return false;

		vv[i] = 1.0f / big;
	}

	int i_max = 0; // Should have a default value just to make sure.

	for (int i = 0; i < sz; i++)
	{
		for (int j = 0; j < i; j++)
		{
			float sum = a[j][i];

			for (int k = 0; k < j; k++)
				sum -= a[j][k] * a[k][i];

			a[j][i] = sum;
		}

		float big = 0.0f;

		for (int j = i; j < sz; j++)
		{
			float sum = a[j][i];

			for (int k = 0; k < i; k++)
				sum -= a[j][k] * a[k][i];

			a[j][i] = sum;

			const float dum = vv[j] * fabsf(sum);
			if (dum >= big)
			{
				big = dum;
				i_max = j;
			}
		}

		if (i != i_max)
		{
			for (int k = 0; k < sz; k++)
			{
				const float dum = a[i_max][k];
				a[i_max][k] = a[i][k];
				a[i][k] = dum;
			}

			vv[i_max] = vv[i];
			*d *= -1.0f;
		}

		indx[i] = i_max;

		if (fabsf(a[i][i]) < EPS_MATRIX)
			return false;

		if (i != sz - 1)
		{
			const float dum = 1.0f / a[i][i];
			for (int j = i + 1; j < sz; j++)
				a[j][i] *= dum;
		}
	}

	return true;
}

// Objects need to be moved back on a failed push, otherwise riders would continue to slide.
static qboolean PushEntities(edict_t* pusher, const vec3_t move, const vec3_t amove)
{
	vec3_t mins;
	vec3_t maxs;

	// Find the bounding box.
	for (int i = 0; i < 3; i++)
	{
		if (move[i] < 0.0f)
		{
			mins[i] = pusher->absmin[i] + move[i];
			maxs[i] = pusher->absmax[i];
		}
		else
		{
			mins[i] = pusher->absmin[i];
			maxs[i] = pusher->absmax[i] + move[i];
		}
	}

	// We need this for pushing things later.
	vec3_t org;
	VectorCopy(pusher->s.angles, org);
	Vec3AddAssign(amove, org);

	vec3_t forward;
	vec3_t right;
	vec3_t up;
	AngleVectors(org, forward, right, up);

	float a[3][3];
	a[0][0] = forward[0];
	a[1][0] = forward[1];
	a[2][0] = forward[2];
	a[0][1] = -right[0];
	a[1][1] = -right[1];
	a[2][1] = -right[2];
	a[0][2] = up[0];
	a[1][2] = up[1];
	a[2][2] = up[2];

	int indx[3];
	float d;

	vec3_t forward_inv;
	vec3_t right_inv;
	vec3_t up_inv;

	if (LUDecomposition(a, indx, 3, &d))
	{
		VectorSet(forward_inv, 1.0f, 0.0f, 0.0f);
		BackSub(a, indx, forward_inv, 3);

		VectorSet(right_inv, 0.0f, 1.0f, 0.0f);
		BackSub(a, indx, right_inv, 3);

		VectorSet(up_inv, 0.0f, 0.0f, 1.0f);
		BackSub(a, indx, up_inv, 3);
	}
	else
	{
		VectorClear(forward_inv);
		VectorClear(right_inv);
		VectorClear(up_inv);
	}

	AngleVectors(pusher->s.angles, forward, right, up);

	// Save the pusher's original position.
	pushed_p->ent = pusher;

	VectorCopy(pusher->s.origin, pushed_p->origin);
	VectorCopy(pusher->s.angles, pushed_p->angles);

	// If pusher rotates, rotate the player.
	if (pusher->client != NULL)
		pushed_p->delta_yaw = pusher->client->ps.pmove.delta_angles[YAW];

	pushed_p++;

	// Move the pusher to it's final position.
	vec3_t pusher_org;
	VectorCopy(pusher->s.origin, pusher_org);
	Vec3AddAssign(move, pusher->s.origin);
	Vec3AddAssign(amove, pusher->s.angles);

	gi.linkentity(pusher);

	// See if any solid entities are inside the final position.
	edict_t* check = &g_edicts[1];

	for (int i = 1; i < globals.num_edicts; i++, check++)
	{
		if (!check->inuse)
			continue;

		if (check->movetype == PHYSICSTYPE_NONE || check->movetype == PHYSICSTYPE_PUSH ||
			check->movetype == PHYSICSTYPE_STOP || check->movetype == PHYSICSTYPE_NOCLIP)
			continue;

		// If the entity is standing on the pusher, it will definitely be moved.
		if (check->groundentity != pusher)
		{
			// See if the self needs to be tested.
			if (check->absmin[0] >= maxs[0] || check->absmin[1] >= maxs[1] || check->absmin[2] >= maxs[2] ||
				check->absmax[0] <= mins[0] || check->absmax[1] <= mins[1] || check->absmax[2] <= mins[2])
				continue;

			// See if the self's bbox is inside the pusher's final position.
			if (!TestEntityPosition(check))
				continue;
		}

		if (pusher->movetype == PHYSICSTYPE_PUSH || check->groundentity == pusher)
		{
			// Move this entity.
			pushed_p->ent = check;

			VectorCopy(check->s.origin, pushed_p->origin);
			VectorCopy(check->s.angles, pushed_p->angles);

			vec3_t hold_org;
			VectorCopy(check->s.origin, hold_org);

			pushed_p++;

			// Try moving the contacted entity.
			if (check->client != NULL)
				check->client->ps.pmove.delta_angles[YAW] += (short)amove[YAW]; // FIXME: doesn't rotate monsters?

			const edict_t* block = NULL;

			// Figure movement due to the pusher's amove.
			for (int test = 0; test < 4; test++)
			{
				vec3_t test_point;
				VectorCopy(hold_org, test_point);
				test_point[2] += check->mins[2];

				VectorCopy(hold_org, check->s.origin);

				if (test & 1)
					test_point[0] += check->mins[0];
				else
					test_point[0] += check->maxs[0];

				if (test & 2)
					test_point[1] += check->mins[1];
				else
					test_point[1] += check->maxs[1];

				VectorSubtract(test_point, pusher_org, org);
				Vec3AddAssign(move, check->s.origin);

				vec3_t org2;
				org2[0] = DotProduct(org, forward);
				org2[1] = -DotProduct(org, right);
				org2[2] = DotProduct(org, up);

				vec3_t move2;
				move2[0] = DotProduct(org2, forward_inv) - org[0];
				move2[1] = DotProduct(org2, right_inv) - org[1];
				move2[2] = DotProduct(org2, up_inv) - org[2];

				Vec3AddAssign(move2, check->s.origin);

				// May have pushed them off an edge.
				if (check->groundentity != pusher)
					check->groundentity = NULL;

				block = TestEntityPosition(check);
				if (block == NULL)
					break;
			}

			if (block == NULL)
			{
				// Pushed ok.
				gi.linkentity(check);

				if (check->client != NULL)
					check->client->playerinfo.flags |= PLAYER_FLAG_USE_ENT_POS;

				// Impact?
				continue;
			}

			// If it is ok to leave in the old position, do it.	This is only relevant for riding entities, not pushed.
			VectorCopy(hold_org, check->s.origin);
			block = TestEntityPosition(check);

			if (block == NULL)
			{
				pushed_p--;
				continue;
			}
		}

		// Save off the obstacle so we can call the block function.
		obstacle = check;

		// Move back any entities we already moved.
		// Go backwards, so if the same entity was pushed twice, it goes back to the original position.
		for (const pushed_t* p = pushed_p - 1; p >= &pushed[0]; p--)
		{
			VectorCopy(p->origin, p->ent->s.origin);
			VectorCopy(p->angles, p->ent->s.angles);

			if (p->ent->client != NULL)
				p->ent->client->ps.pmove.delta_angles[YAW] = (short)p->delta_yaw;

			gi.linkentity(p->ent);
		}

		return false;
	}

	// See if anything we moved has touched a trigger.
	for (const pushed_t* p = pushed_p - 1; p >= &pushed[0]; p--)
		ActivateTriggers(p->ent);

	return true;
}

// Bmodel objects don't interact with each other, but push all box objects.
static void Physics_Push(edict_t* self)
{
	// Not a team captain, so movement will be handled elsewhere.
	if (self->flags & FL_TEAMSLAVE)
		return;

	// Make sure all team slaves can move before commiting any moves or calling any think functions.
	// If the move is blocked, all moved objects will be backed out.
	pushed_p = &pushed[0];

	edict_t* pusher;
	for (pusher = self; pusher != NULL; pusher = pusher->teamchain)
	{
		if (Vec3NotZero(pusher->velocity) || Vec3NotZero(pusher->avelocity))
		{
			// Object is moving.
			vec3_t move;
			VectorScale(pusher->velocity, FRAMETIME, move);

			vec3_t amove;
			VectorScale(pusher->avelocity, FRAMETIME, amove);

			if (!PushEntities(pusher, move, amove))
				break; // Move was blocked.
		}
	}

	if (pushed_p > &pushed[MAX_EDICTS])
		gi.error(ERR_FATAL, "pushed_p > &pushed[MAX_EDICTS], memory corrupted");

	if (pusher != NULL)
	{
		// The move failed, bump all nextthink times and back out moves.
		for (edict_t* moved = self; moved != NULL; moved = moved->teamchain)
			if (moved->nextthink > 0.0f)
				moved->nextthink += FRAMETIME;

		// If the pusher has a "blocked" function, call it.	Otherwise, just stay in place until the obstacle is gone.
		if (pusher->blocked != NULL)
			pusher->blocked(pusher, obstacle);
	}
	else
	{
		// The move succeeded, so call all think functions.
		for (edict_t* moved = self; moved != NULL; moved = moved->teamchain)
			if (ThinkTime(moved))
				moved->think(moved);
	}
}

static void Physics_ScriptAngular(edict_t* self)
{
	if (self->owner != NULL)
	{
		vec3_t angle;
		VectorAdd(self->owner->s.angles, self->moveinfo.start_angles, angle);

		vec3_t forward;
		if (self->moveinfo.state & 1)
			AngleVectors(angle, NULL, forward, NULL);
		else
			AngleVectors(angle, NULL, NULL, forward);

		VectorInverse(forward);

		vec3_t dest;
		VectorMA(self->moveinfo.start_origin, self->moveinfo.distance, forward, dest);

		Vec3SubtractAssign(self->s.origin, dest); // Distance moved this frame.
		VectorScale(dest, (1.0f / FRAMETIME), self->velocity); // Hence velocity.

		if (self->moveinfo.state & 1)
			VectorCopy(angle, self->s.angles);
	}

	Physics_Push(self);
}