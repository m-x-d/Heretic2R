//
// Utilities.c
//
// Copyright 1998 Raven Software
//

#include "Utilities.h"
#include "Client Effects.h"
#include "Random.h"
#include "Matrix.h"
#include "Vector.h"
#include "Reference.h"
#include "Motion.h"
#include "Particle.h"
#include "ce_DLight.h"
#include "fx_debris.h" //mxd
#include "fx_smoke.h" //mxd
#include "fx_sparks.h" //mxd
#include "g_playstats.h"

// Setup for circular list.
#define MAX_ENTRIES_IN_CIRCULAR_LIST	70
static client_entity_t* circular_list[MAX_ENTRIES_IN_CIRCULAR_LIST + 1];
static int circular_list_size;
static int circular_list_index = 0;

#pragma region ========================== Update functions ==========================

qboolean RemoveSelfAI(client_entity_t* this, centity_t* owner)
{
	return false; // Removed after one think (nextThinkTime is lifetime).
}

qboolean KeepSelfAI(client_entity_t* this, centity_t* owner)
{
	return true; // Remain alive forever.
}

qboolean AttemptRemoveSelf(client_entity_t* self, centity_t* owner)
{
	if (self->flags & (CEF_NO_DRAW | CEF_DISAPPEARED | CEF_DROPPED))
		return false;

	const float max_dist = ((self->flags & CEF_CULLED) ? CFX_CULLING_DIST : r_farclipdist->value);
	if (self->r.depth > max_dist)
		return false;

	self->updateTime = 400;
	return true;
}

#pragma endregion

#pragma region ========================== AddToView functions ==========================

qboolean LinkedEntityUpdatePlacement(client_entity_t* current, centity_t* owner)
{
	if (current->r.flags & RF_FIXED)
	{
		matrix3_t rotation;
		Matrix3FromAngles(owner->lerp_angles, rotation);

		vec3_t direction;
		Matrix3MultByVec3(rotation, current->direction, direction);

		vec3_t up;
		Matrix3MultByVec3(rotation, current->up, up);

		AnglesFromDirAndUp(direction, up, current->r.angles);
	}

	VectorCopy(owner->origin, current->r.origin);

	return true;
}

qboolean OffsetLinkedEntityUpdatePlacement(client_entity_t* current, centity_t* owner)
{
	matrix3_t rotation;
	vec3_t up;
	vec3_t direction;
	vec3_t up2;
	vec3_t direction2;
	vec3_t origin;

	Matrix3FromAngles(owner->lerp_angles, rotation);
	Matrix3MultByVec3(rotation, current->origin, origin);

	if (current->r.flags & RF_FIXED)
	{
		VectorAdd(current->origin, current->direction, direction);
		VectorAdd(current->origin, current->up, up);

		Matrix3MultByVec3(rotation, direction, direction2);
		Matrix3MultByVec3(rotation, up, up2);

		Vec3SubtractAssign(origin, direction2);
		Vec3SubtractAssign(origin, up2);

		AnglesFromDirAndUp(direction2, up2, current->r.angles);
	}

	VectorAdd(owner->origin, origin, current->r.origin);

	return true;
}

qboolean ReferenceLinkedEntityUpdatePlacement(struct client_entity_s* self, centity_t* owner)
{
	// This tells if we are wasting our time, because the reference points are culled.
	if (!RefPointsValid(owner))
		return true;

	VectorCopy(owner->referenceInfo->references[self->refPoint].placement.origin, self->origin);

	return OffsetLinkedEntityUpdatePlacement(self, owner); //mxd
}

#pragma endregion

#pragma region ========================== Message response helper functions ==========================

void BecomeStatic(client_entity_t* self)
{
	VectorClear(self->velocity);
	VectorClear(self->acceleration);

	self->flags &= ~CEF_CLIP_TO_WORLD;
}

#pragma endregion

#pragma region ========================== Physics functions ==========================

// Returns whether solid world is there.
// Sets dist to distance to solid or maxdist if no solid.
// If maxdist < 0 returns distance to floor.
// If maxdist > 0 returns distance to ceiling.
int GetSolidDist(const vec3_t origin, const float radius, const float maxdist, float* dist)
{
	vec3_t mins;
	vec3_t maxs;
	vec3_t end;
	trace_t trace;

	VectorSet(mins, -radius, -radius, -1.0f);
	VectorSet(maxs, radius, radius, 1.0f);
	VectorCopy(origin, end);
	end[2] += maxdist;

	fxi.Trace(origin, mins, maxs, end, MASK_DRIP, CEF_CLIP_TO_WORLD, &trace);

	if (trace.fraction == 1.0f)
	{
		*dist = maxdist;
		return false;
	}

	*dist = trace.endpos[2] - origin[2];
	return true;
}

// Gets time for a client_entity_t to fall to the ground.
int GetFallTime(vec3_t origin, const float velocity, const float acceleration, const float radius, const float maxtime, trace_t* trace)
{
	vec3_t mins;
	vec3_t maxs;
	vec3_t end;

	VectorSet(mins, -radius, -radius, -1.0f);
	VectorSet(maxs, radius, radius, 1.0f);
	VectorCopy(origin, end);
	end[2] += (velocity * maxtime) + acceleration * (maxtime * maxtime) * 0.5f; // From s = ut + 0.5at^2

	fxi.Trace(origin, mins, maxs, end, MASK_DRIP, CEF_CLIP_TO_WORLD, trace);

	return (int)(GetTimeToReachDistance(velocity, acceleration, trace->endpos[2] - origin[2])); // In ms.
}

// Returns false if no plane found before maxdist traveled, or non-water plane hit.
// Returns true, the plane normal of plane hit and the distance to the plane if a plane hit.
int GetWaterNormal(const vec3_t origin, const float radius, const float maxdist, vec3_t normal, float* dist)
{
	vec3_t mins;
	vec3_t maxs;
	vec3_t start;
	vec3_t end;
	trace_t trace;

	VectorSet(mins, -radius, -radius, -1.0f);
	VectorSet(maxs, radius, radius, 1.0f);
	VectorCopy(origin, end);
	VectorCopy(origin, start);
	start[2] += maxdist;
	end[2] -= maxdist;

	fxi.Trace(origin, mins, maxs, end, MASK_DRIP, CEF_CLIP_TO_WORLD, &trace);

	if (trace.fraction == 1.0f || (trace.contents & MASK_SOLID))
		return false;

	VectorCopy(trace.plane.normal, normal);
	*dist = (end[2] - origin[2]) * trace.fraction;

	return true;
}

static void FizzleEffect(const client_entity_t* self, vec3_t surface_top, vec3_t normal)
{
	vec3_t spot;

	if (self != NULL && self->dlight != NULL)
		self->dlight->intensity = 0; // Lights out.

	char* snd_name;
	if (irand(0, 3))
		snd_name = va("ambient/lavadrop%i.wav", irand(1, 3));
	else
		snd_name = "misc/lavaburn.wav";

	fxi.S_StartSound(surface_top, -1, CHAN_AUTO, fxi.S_RegisterSound(snd_name), 1.0f, ATTN_STATIC, 0.0f);

	const int num_puffs = GetScaledCount(irand(2, 5), 0.3f);
	for (int i = 0; i < num_puffs; i++)
	{
		spot[0] = surface_top[0] + flrand(-3.0f, 3.0f);
		spot[1] = surface_top[1] + flrand(-3.0f, 3.0f);
		spot[2] = surface_top[2] + flrand(0.0f, 3.0f);

		FXDarkSmoke(spot, flrand(0.2f, 0.5f), flrand(30.0f, 50.0f));
	}

	FireSparks(NULL, FX_SPARKS, 0, surface_top, normal);
}

qboolean Physics_MoveEnt(client_entity_t* self, float d_time, float d_time2, trace_t* trace)
{
	vec3_t end;
	vec3_t move;
	vec3_t attempt;
	vec3_t mins;
	vec3_t maxs;
	vec3_t dir;
	vec3_t surface_top;

	entity_t* r = &self->r;

	if (Vec3IsZero(self->velocity) || Vec3IsZero(self->acceleration))
		return false;

	// Make more debris leave particle trails, like dust for wood, pebbles for rock, etc.
	if (self->SpawnInfo & SIF_INWATER)
	{
		// Leave several bubbles?
		d_time *= 0.5f;
		d_time2 *= 0.5f;
	}
	else if (self->SpawnInfo & SIF_INLAVA)
	{
		d_time *= 0.01f;
		d_time2 *= 0.01f;
	}
	else if (self->SpawnInfo & SIF_INMUCK)
	{
		// Leave a ripple or two?
		d_time *= 0.05f;
		d_time2 *= 0.05f;
	}

	for (int i = 0; i < 3; i++)
		attempt[i] = self->velocity[i] * d_time + self->acceleration[i] * d_time2;

	// May need an individual mins and maxs, probably want to put into an optional physics_info struct.
	VectorSet(mins, -self->radius, -self->radius, -self->radius);
	VectorSet(maxs, self->radius, self->radius, self->radius);
	VectorAdd(r->origin, attempt, end);

	fxi.Trace(r->origin, mins, maxs, end, MASK_SHOT | MASK_WATER, self->flags, trace);

	if (trace->fraction == 1.0f || trace->allsolid || trace->startsolid || Vec3IsZeroEpsilon(trace->plane.normal))
	{
		Vec3AddAssign(attempt, r->origin);

		for (int i = 0; i < 3; i++)
			self->velocity[i] += self->acceleration[i] * d_time;

		return false;
	}

	if (trace->surface->flags & SURF_SKY)
	{
		// Remove it.
		self->nextThinkTime = fxi.cl->time; //BUGFIX. mxd. updateTime = fxi.cl->time + 0.1f; in original logic (makes no sense: updateTime is ADDED to fxi.cl->time in UpdateEffects()).
		self->Update = RemoveSelfAI; //mxd. FXDebris_Remove() in original logic.

		return false;
	}

	d_time *= trace->fraction;

	VectorScale(attempt, d_time, move);
	Vec3AddAssign(move, r->origin);

	for (int i = 0; i < 3; i++)
		self->velocity[i] += self->acceleration[i] * d_time;

	VectorCopy(attempt, dir);
	VectorNormalize(dir);

	VectorMA(r->origin, self->radius * 0.5f, dir, surface_top);

	const float hit_angle = DotProduct(dir, trace->plane.normal);
	const qboolean do_splash_effect = (r_detail->value < DETAIL_UBERHIGH || irand(0, 1)); //TODO: 'r_detail->value < DETAIL_UBERHIGH' check seems strange. Show 50% LESS often on uberhigh?..

	// When in water.
	if (trace->contents & CONTENTS_WATER && !(self->SpawnInfo & SIF_INWATER))
	{
		if (self->flags & CEF_FLAG6)
		{
			self->flags &= ~CEF_FLAG6;
			FizzleEffect(self, surface_top, trace->plane.normal);
		}

		self->SpawnInfo |= SIF_INWATER; // In water now, sink a little slower.

		// Spawn ripples, splash.
		if (do_splash_effect)
			DoWaterEntrySplash(FX_WATER_ENTRYSPLASH, 0, surface_top, 64, trace->plane.normal);

		if (flrand(-0.5f, 0.0f) < hit_angle)
		{
			// Splash sound.
			if (do_splash_effect)
				fxi.S_StartSound(r->origin, -1, CHAN_AUTO, fxi.S_RegisterSound(va("misc/splish%i.wav", irand(2, 3))), 1.0f, ATTN_STATIC, 0.0f);

			QPostMessage(self, MSG_COLLISION, "g", trace); // This will be processed next.

			return true;
		}

		const int material = (self->SpawnInfo & SIF_FLAG_MASK);

		if (material != MAT_WOOD) // Wood floats, everything else can keep sinking.
		{
			// Bubbles and blurp sound.
			if (do_splash_effect)
			{
				FXBubble(NULL, FX_BUBBLE, 0, surface_top);
				fxi.S_StartSound(r->origin, -1, CHAN_AUTO, fxi.S_RegisterSound("misc/splish1.wav"), 1.0f, ATTN_STATIC, 0.0f);
			}

			fxi.Trace(trace->endpos, mins, maxs, end, MASK_SHOT, self->flags, trace);

			if (trace->fraction < 1.0f)
			{
				d_time *= trace->fraction;

				VectorScale(attempt, d_time, move);
				Vec3AddAssign(move, r->origin);
			}
			else
			{
				Vec3AddAssign(attempt, r->origin);
			}

			for (int i = 0; i < 3; i++)
				self->velocity[i] += self->acceleration[i] * d_time;
		}
		else // Sit on surface.
		{
			// Splash sound.
			if (do_splash_effect)
				fxi.S_StartSound(r->origin, -1, CHAN_AUTO, fxi.S_RegisterSound(va("player/waterrun%i.wav", irand(1, 2))), 1.0f, ATTN_STATIC, 0.0f);

			VectorCopy(surface_top, r->origin);
			VectorClear(self->velocity);
			VectorClear(self->acceleration);
			self->d_alpha = -0.2f;
			self->Update = FXDebris_Vanish;
			self->nextThinkTime = fxi.cl->time; //BUGFIX. mxd. updateTime = fxi.cl->time + 0.1f; in original logic (makes no sense: updateTime is ADDED to fxi.cl->time in UpdateEffects()).
		}

		return false; // No need to update trace counter if not sending collision.
	}

	// When in slime
	if (trace->contents & CONTENTS_SLIME && !(self->SpawnInfo & SIF_INMUCK))
	{
		if (self->flags & CEF_FLAG6)
		{
			self->flags &= ~CEF_FLAG6;

			if (do_splash_effect)
				FizzleEffect(self, surface_top, trace->plane.normal);
		}

		self->SpawnInfo |= SIF_INMUCK; // In muck, sink really really slowly.

		// Spawn ripples, splash.
		if (do_splash_effect)
			DoWaterEntrySplash(FX_WATER_ENTRYSPLASH, 0, surface_top, 64, trace->plane.normal);

		if (flrand(-0.75f, 0) < hit_angle)
		{
			// Splash sound.
			if (do_splash_effect)
				fxi.S_StartSound(r->origin, -1, CHAN_AUTO, fxi.S_RegisterSound(va("player/waterrun%i.wav", irand(1, 2))), 1.0f, ATTN_STATIC, 0.0f);

			QPostMessage(self, MSG_COLLISION, "g", trace); // This will be processed next.

			return true;
		}

		// Bubbles and blurp sound.
		if (do_splash_effect)
		{
			FXBubble(NULL, FX_BUBBLE, 0, surface_top);
			fxi.S_StartSound(r->origin, -1, CHAN_AUTO, fxi.S_RegisterSound("objects/submerge.wav"), 1.0f, ATTN_STATIC, 0.0f);
		}

		VectorCopy(surface_top, r->origin);
		self->d_alpha = -0.01f;
		self->Update = FXDebris_Vanish;
		self->nextThinkTime = fxi.cl->time; //BUGFIX. mxd. updateTime = fxi.cl->time + 0.1f; in original logic (makes no sense: updateTime is ADDED to fxi.cl->time in UpdateEffects()).

		return false; // No need to update trace counter if not sending collision.
	}

	// When in lava.
	if (trace->contents & CONTENTS_LAVA && !(self->SpawnInfo & SIF_INLAVA))
	{
		self->flags &= ~CEF_FLAG6;
		self->SpawnInfo |= SIF_INLAVA; // In lava now, continue to burn.

		// Smoke puffs and sizzle here.
		if (do_splash_effect)
			FizzleEffect(self, surface_top, trace->plane.normal);

		VectorCopy(surface_top, r->origin);
		self->d_scale = -0.2f;
		self->Update = FXDebris_Vanish;
		self->nextThinkTime = fxi.cl->time; //BUGFIX. mxd. updateTime = fxi.cl->time + 0.1f; in original logic (makes no sense: updateTime is ADDED to fxi.cl->time in UpdateEffects()).

		return false;
	}

	QPostMessage(self, MSG_COLLISION, "g", trace); // This will be processed next.

	// Works just like a recursive call.
	return true;
}

// Takes desired count and scales it by the desired frame rates relationship with actual framerate.
int GetScaledCount(const int count, float refdepend) //TODO: remove 2-nd arg?
{
	float work;

	// If we are doing a time demo, we don't want scalability.
	if ((int)cl_timedemo->value)
		work = (float)count;
	else
		work = (float)count * fxi.cls->framemodifier;

	if ((int)r_detail->value == DETAIL_NORMAL)
		work *= 0.75f;
	else if ((int)r_detail->value == DETAIL_LOW)
		work *= 0.5f;

	return Q_ftol(max(work, 1.0f));
}

void AdvanceParticle(client_particle_t* p, const int ms)
{
	const float time = (float)ms * 0.001f;
	GetPositionOverTime(p->origin, p->velocity, p->acceleration, time, p->origin);
	GetVelocityOverTime(p->velocity, p->acceleration, time, p->velocity);
}

// We don`t have access to "sv_gravity" so we keep a local client fx copy. It is set to same default as on the server.
float GetGravity(void)
{
	return -clfx_gravity->value;
}

#pragma endregion

// Tells if we have not rendered this reference point for a while.
qboolean RefPointsValid(const centity_t* owner)
{
	// RF_IGNORE_REFS check is necessary in case we're a chicken.
	return (owner->referenceInfo != NULL && !(owner->current.renderfx & RF_IGNORE_REFS) && owner->referenceInfo->lastUpdate - (float)fxi.cl->time <= REF_MINCULLTIME);
}

qboolean ReferencesInitialized(const centity_t* owner) //TODO: no longer needed?
{
	return (owner->referenceInfo != NULL);
}

// Add a blood splat or a scorchmark to the circular list - removing an entity that's next in the list if there is one.
void InsertInCircularList(client_entity_t* self)
{
	client_entity_t** prev;
	client_entity_t* current;

	client_entity_t** root = &clientEnts;

	// If we have an entry already - delete it.
	if (circular_list[circular_list_index] != NULL)
	{
		// Search for this client entities entry in the client entity list.
		for (prev = root, current = *root; current != NULL; current = current->next)
		{
			if (current == circular_list[circular_list_index])
			{
				RemoveEffectFromList(prev, NULL);
				break;
			}

			prev = &(*prev)->next;
		}
	}

	// Add in new one.
	circular_list[circular_list_index] = self;
	circular_list_index++;

	// Delimit the pointer.
	if (circular_list_index >= circular_list_size)
		circular_list_index = 0;
}

void ClearCircularList(void) //mxd
{
	memset(&circular_list[0], 0, sizeof(circular_list));

	switch ((int)r_detail->value)
	{
		case DETAIL_LOW:
			circular_list_size = 30;
			break;

		case DETAIL_NORMAL:
			circular_list_size = 50;
			break;

		default: // DETAIL_HIGH / DETAIL_UBERHIGH
			circular_list_size = MAX_ENTRIES_IN_CIRCULAR_LIST;
			break;
	}
}