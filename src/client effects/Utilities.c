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
#include "q_Sprite.h" //mxd
#include "g_playstats.h"

// Setup for circular list.
#define MAX_ENTRIES_IN_CIRCULAR_LIST	70
static client_entity_t* circular_list[MAX_ENTRIES_IN_CIRCULAR_LIST + 1];
static int circular_list_size;
static int circular_list_index = 0;

#pragma region ========================== Update functions ==========================

qboolean RemoveSelfAI(client_entity_t* self, centity_t* owner)
{
	return false; // Removed after one think (nextThinkTime is lifetime).
}

qboolean KeepSelfAI(client_entity_t* self, centity_t* owner)
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

qboolean LinkedEntityUpdatePlacement(client_entity_t* self, centity_t* owner)
{
	if (self->r.flags & RF_FIXED)
	{
		matrix3_t rotation;
		Matrix3FromAngles(owner->lerp_angles, rotation);

		vec3_t direction;
		Matrix3MultByVec3(rotation, self->direction, direction);

		vec3_t up;
		Matrix3MultByVec3(rotation, self->up, up);

		AnglesFromDirAndUp(direction, up, self->r.angles);
	}

	VectorCopy(owner->origin, self->r.origin);

	return true;
}

qboolean OffsetLinkedEntityUpdatePlacement(client_entity_t* self, centity_t* owner)
{
	matrix3_t rotation;
	Matrix3FromAngles(owner->lerp_angles, rotation);

	vec3_t origin;
	Matrix3MultByVec3(rotation, self->origin, origin);

	if (self->r.flags & RF_FIXED)
	{
		vec3_t direction;
		VectorAdd(self->origin, self->direction, direction);

		vec3_t up;
		VectorAdd(self->origin, self->up, up);

		vec3_t direction2;
		Matrix3MultByVec3(rotation, direction, direction2);

		vec3_t up2;
		Matrix3MultByVec3(rotation, up, up2);

		Vec3SubtractAssign(origin, direction2);
		Vec3SubtractAssign(origin, up2);

		AnglesFromDirAndUp(direction2, up2, self->r.angles);
	}

	VectorAdd(owner->origin, origin, self->r.origin);

	return true;
}

qboolean ReferenceLinkedEntityUpdatePlacement(client_entity_t* self, centity_t* owner)
{
	// This tells if we are wasting our time, because the reference points are culled.
	if (!RefPointsValid(owner))
		return true;

	VectorCopy(owner->referenceInfo->references[self->refPoint].placement.origin, self->origin);

	return OffsetLinkedEntityUpdatePlacement(self, owner); //mxd
}

#pragma endregion

#pragma region ========================== Physics functions ==========================

// Returns vertical distance to solid world / water surface or max_dist if no solid.
// If max_dist < 0 returns distance to floor.
// If max_dist > 0 returns distance to ceiling.
float GetSolidDist(const vec3_t origin, const float radius, const float max_dist, const qboolean check_water) //mxd. 'int' return type in original logic, distance was returned via 'float* dist' arg.
{
	const vec3_t end = VEC3_INITA(origin, 0.0f, 0.0f, max_dist);
	const vec3_t mins = { -radius, -radius, -1.0f };
	const vec3_t maxs = {  radius,  radius,  1.0f };

	const int brushmask = (MASK_SOLID | (check_water ? CONTENTS_WATER : 0)); //mxd. Check for water specifically, not slime or lava.

	trace_t trace;
	fxi.Trace(origin, mins, maxs, end, brushmask, CTF_CLIP_TO_WORLD, &trace); //mxd. Original logic uses MASK_DRIP (resulted in trace.startsolid when origin was underwater).

	//mxd. Bbox stuck in a floor (or ceiling).
	if (trace.startsolid)
		return 0.0f;

	if (trace.fraction == 1.0f)
		return max_dist;

	return trace.endpos[2] - origin[2] - radius + 1.0f; //mxd. Subtract radius (fixes RedRain cloud sprites half-clipping into ceiling when fired at ceiling).
}

// Gets time for a client_entity_t to fall to the ground.
int GetFallTime(vec3_t origin, const float velocity, const float acceleration, const float radius, const float maxtime, trace_t* trace)
{
	const vec3_t mins = { -radius, -radius, -1.0f };
	const vec3_t maxs = {  radius,  radius,  1.0f };

	vec3_t end = VEC3_INIT(origin);
	end[2] += (velocity * maxtime) + acceleration * (maxtime * maxtime) * 0.5f; // From s = ut + 0.5at^2

	fxi.Trace(origin, mins, maxs, end, MASK_DRIP, CTF_CLIP_TO_WORLD, trace);

	//mxd. If origin is underwater, retry with MASK_SOLID. Otherwise, trace will stop when hitting adjacent water brushes (which is never what we expect).
	if (trace->startsolid && (trace->contents & CONTENTS_WATER))
		fxi.Trace(origin, mins, maxs, end, MASK_SOLID, CTF_CLIP_TO_WORLD, trace);

	return (int)(GetTimeToReachDistance(velocity, acceleration, trace->endpos[2] - origin[2])); // In ms.
}

// Returns false if no plane found before maxdist traveled, or non-water plane hit.
// Returns true, the plane normal of plane hit and the distance to the plane if a plane hit.
qboolean GetWaterNormal(const vec3_t origin, const float radius, const float maxdist, vec3_t normal, float* dist)
{
	const vec3_t mins = { -radius, -radius, -1.0f };
	const vec3_t maxs = {  radius,  radius,  1.0f };

	const vec3_t end = VEC3_INITA(origin, 0.0f, 0.0f, -maxdist);

	trace_t trace;
	fxi.Trace(origin, mins, maxs, end, MASK_DRIP, CTF_CLIP_TO_WORLD, &trace);

	if (trace.fraction == 1.0f || (trace.contents & MASK_SOLID))
		return false;

	VectorCopy(trace.plane.normal, normal);
	*dist = (end[2] - origin[2]) * trace.fraction;

	return true;
}

static void FizzleEffect(const client_entity_t* self, const vec3_t surface_top, const vec3_t normal)
{
	if (self != NULL && self->dlight != NULL)
		self->dlight->intensity = 0; // Lights out.

	const char* snd_name = va("ambient/lavadrop%i.wav", irand(1, 3)); //mxd. Original logic also plays "misc/lavaburn.wav" here.
	fxi.S_StartSound(surface_top, -1, CHAN_AUTO, fxi.S_RegisterSound(snd_name), flrand(0.25f, 0.75f), ATTN_IDLE, 0.0f);

	const int num_puffs = GetScaledCount(irand(2, 5), 0.3f);
	for (int i = 0; i < num_puffs; i++)
	{
		const vec3_t spot = VEC3_INITA(surface_top, flrand(-3.0f, 3.0f), flrand(-3.0f, 3.0f), flrand(0.0f, 3.0f));
		FXDarkSmoke(spot, flrand(0.2f, 0.5f), flrand(30.0f, 50.0f));
	}

	FireSparks(NULL, FX_SPARKS, 0, surface_top, normal);
}

qboolean Physics_MoveEnt(client_entity_t* self, float d_time, float d_time2, trace_t* trace, const qboolean update_velocity) //mxd. +update_velocity arg.
{
	entity_t* r = &self->r;

	if (Vec3IsZero(self->velocity) || Vec3IsZero(self->acceleration))
		return false;

	// Make more debris leave particle trails, like dust for wood, pebbles for rock, etc.
	if (self->SpawnInfo & SIF_INWATER)
	{
		// Leave several bubbles?
		d_time *= 0.2f; // H2: 0.5
		d_time2 *= 0.2f; // H2: 0.5
	}
	else if (self->SpawnInfo & SIF_INLAVA)
	{
		d_time *= 0.03f; // H2: 0.01
		d_time2 *= 0.03f; // H2: 0.01
	}
	else if (self->SpawnInfo & SIF_INMUCK)
	{
		// Leave a ripple or two?
		d_time *= 0.05f;
		d_time2 *= 0.05f;
	}

	vec3_t attempt;
	for (int i = 0; i < 3; i++)
		attempt[i] = self->velocity[i] * d_time + self->acceleration[i] * d_time2;

	vec3_t end;
	VectorAdd(r->origin, attempt, end);

	// May need an individual mins and maxs, probably want to put into an optional physics_info struct.
	const vec3_t mins = { -self->radius, -self->radius, -self->radius };
	const vec3_t maxs = {  self->radius,  self->radius,  self->radius };

	//mxd. Don't use MASK_WATER flag (because it results in startsolid trace when we are in liquid).
	//mxd. trace.contents holds accurate contents regardless of trace results because of CTF_ALWAYS_CHECK_CONTENTS flag.
	fxi.Trace(r->origin, mins, maxs, end, MASK_SHOT, self->clip_flags, trace); //mxd. Use clip_flags.

	if ((trace->surface->flags & SURF_SKY))
	{
		// Remove it.
		self->nextThinkTime = fx_time; //BUGFIX. mxd. updateTime = fxi.cl->time + 0.1f; in original logic (makes no sense: updateTime is ADDED to fx_time in UpdateEffects()).
		self->Update = RemoveSelfAI; //mxd. FXDebris_Remove() in original logic.

		return false;
	}

	//mxd. Update velocity regardless of trace results (but only when called from UpdateEffects()).
	if (update_velocity)
		VectorMA(self->velocity, d_time, self->acceleration, self->velocity);

	//mxd. UGH! Do extra contents check. There are some VERY thin slime/lava brushes used in some H2 maps (like 2 mu. high slime brush near [-1304 -416 88] in hivepriestess.bsp).
	vec3_t move_dir;
	VectorNormalize2(attempt, move_dir);

	vec3_t surf_end;
	VectorMA(end, self->radius, move_dir, surf_end);

	trace_t surf_trace;
	fxi.Trace(r->origin, vec3_origin, vec3_origin, surf_end, MASK_SHOT | MASK_WATER, self->clip_flags, &surf_trace);

	if (surf_trace.fraction < 1.0f)
		trace->contents |= surf_trace.contents;

	const qboolean do_splash_effect = (irand(DETAIL_LOW, DETAIL_UBERHIGH) <= R_DETAIL); //mxd. 'r_detail->value < DETAIL_UBERHIGH || irand(0, 1)' in original logic.

	// When entered water.
	if ((trace->contents & CONTENTS_WATER) && !(self->SpawnInfo & SIF_INWATER))
	{
		// Process intersection with water surface.
		if (surf_trace.fraction < 1.0f)
		{
			if (self->flags & CEF_FLAG6)
			{
				self->flags &= ~CEF_FLAG6;
				FizzleEffect(self, surf_trace.endpos, surf_trace.plane.normal);
			}

			// Spawn ripples, splash.
			if (do_splash_effect)
				DoWaterEntrySplash(FX_WATER_ENTRYSPLASH, 0, surf_trace.endpos, 64, surf_trace.plane.normal);

			// Bounce off water surface?
			const float hit_angle = DotProduct(move_dir, surf_trace.plane.normal);

			if (flrand(-0.5f, 0.0f) < hit_angle)
			{
				// Splash sound.
				if (do_splash_effect)
				{
					const char* snd_name = va("misc/splish%i.wav", irand(2, 3));
					fxi.S_StartSound(r->origin, -1, CHAN_AUTO, fxi.S_RegisterSound(snd_name), flrand(0.5f, 1.0f), ATTN_IDLE, 0.0f);
				}

				// Move to collision point.
				vec3_t collision_point = VEC3_INITS(move_dir, -self->radius);
				Vec3AddAssign(surf_trace.endpos, collision_point);
				VectorCopy(collision_point, r->origin);

				CE_PostMessage(self, MSG_COLLISION, "g", trace); // This will be processed next.

				return true;
			}

			//mxd. Done only for MAT_WOOD in original logic. Changed because of added underwater handling logic in FXDebris_Vanish...
			self->d_alpha = -0.05f;
			self->Update = FXDebris_Vanish;
			self->nextThinkTime = fx_time; //BUGFIX. mxd. updateTime = fxi.cl->time + 0.1f; in original logic (makes no sense: updateTime is ADDED to fx_time in UpdateEffects()).

			const int material = (self->SpawnInfo & SIF_FLAG_MASK);

			if (material != MAT_WOOD) // Wood floats, everything else can keep sinking.
			{
				// Spawn bubbles and blurp sound when water is deep enough to sink.
				if (do_splash_effect && trace->fraction == 1.0f)
				{
					FXBubble(NULL, FX_BUBBLE, 0, surf_trace.endpos);

					const char* snd_name = va("misc/splish%i.wav", irand(1, 3));
					fxi.S_StartSound(r->origin, -1, CHAN_AUTO, fxi.S_RegisterSound(snd_name), flrand(0.5f, 1.0f), ATTN_IDLE, flrand(0.0f, 0.25f));
				}
			}
			else // Sit on surface.
			{
				// Splash sound.
				if (do_splash_effect)
				{
					const char* snd_name = va("player/waterrun%i.wav", irand(1, 2));
					fxi.S_StartSound(r->origin, -1, CHAN_AUTO, fxi.S_RegisterSound(snd_name), flrand(0.5f, 1.0f), ATTN_IDLE, 0.0f);
				}

				//mxd. Sit slightly above water surface.
				vec3_t collision_point = VEC3_INITS(move_dir, -1.0f);
				Vec3AddAssign(surf_trace.endpos, collision_point);
				VectorCopy(collision_point, r->origin);

				// This will abort further Physics_MoveEnt() calls, among other things --mxd.
				VectorClear(self->velocity);
				VectorClear(self->acceleration);

				return false; // No need to update trace counter if not sending collision.
			}
		}

		self->SpawnInfo |= SIF_INWATER; // In water now, sink a little slower. //mxd. Done above "Bounce off water surface" block in original logic.
	}
	else if (!(trace->contents & CONTENTS_WATER) && (self->SpawnInfo & SIF_INWATER)) //mxd. When left water.
	{
		self->SpawnInfo &= ~SIF_INWATER; //TODO: spawn splash FX?
	}

	// When entered slime.
	if ((trace->contents & CONTENTS_SLIME) && !(self->SpawnInfo & SIF_INMUCK))
	{
		self->SpawnInfo |= SIF_INMUCK; // In muck, sink really really slowly.

		self->d_alpha = -0.2f;
		self->Update = FXDebris_Vanish;
		self->nextThinkTime = fx_time; //BUGFIX. mxd. updateTime = fxi.cl->time + 0.1f; in original logic (makes no sense: updateTime is ADDED to fx_time in UpdateEffects()).

		// Process intersection with slime surface.
		if (surf_trace.fraction < 1.0f)
		{
			if (self->flags & CEF_FLAG6)
			{
				self->flags &= ~CEF_FLAG6;

				if (do_splash_effect)
					FizzleEffect(self, surf_trace.endpos, surf_trace.plane.normal);
			}

			//mxd. Skip bouncing off slime surface logic (slime is supposed to be viscous, right?).

			if (do_splash_effect)
			{
				// Spawn ripples, splash.
				DoWaterEntrySplash(FX_WATER_ENTRYSPLASH, 0, surf_trace.endpos, 64, surf_trace.plane.normal);

				if (irand(0, 9) < 4)
					fxi.S_StartSound(r->origin, -1, CHAN_AUTO, fxi.S_RegisterSound("objects/submerge.wav"), flrand(0.05f, 0.5f), ATTN_IDLE, flrand(0.0f, 2.0f)); //mxd. Skip bubbles FX - slime is opaque.
			}
		}
	}

	// When entered lava.
	if ((trace->contents & CONTENTS_LAVA) && !(self->SpawnInfo & SIF_INLAVA))
	{
		self->flags &= ~CEF_FLAG6;
		self->SpawnInfo |= SIF_INLAVA; // In lava now, continue to burn.

		self->d_scale = -0.4f; // H2: -0.2.
		self->Update = FXDebris_Vanish;
		self->nextThinkTime = fx_time; //BUGFIX. mxd. updateTime = fxi.cl->time + 0.1f; in original logic (makes no sense: updateTime is ADDED to fx_time in UpdateEffects()).

		// Process intersection with lava surface.
		if (do_splash_effect && surf_trace.fraction < 1.0f)
		{
			FizzleEffect(self, surf_trace.endpos, surf_trace.plane.normal);

			if (irand(0, 9) < 2) //mxd
				fxi.S_StartSound(r->origin, -1, CHAN_AUTO, fxi.S_RegisterSound("objects/fallinlava.wav"), flrand(0.25f, 0.75f), ATTN_IDLE, flrand(0.0f, 0.25f));
		}
	}

	// Move to end-point.
	VectorCopy(trace->endpos, r->origin);

	// Trace hit nothing. Can move full distance.
	if (trace->fraction == 1.0f)
		return false;

	// Trace hit something.

	//mxd. GROSS HACK: sink into extra thin lava/slime surfaces...
	if (self->SpawnInfo & (SIF_INLAVA | SIF_INMUCK))
	{
		vec3_t diff;
		VectorSubtract(trace->endpos, surf_trace.endpos, diff);

		if (VectorLength(diff) < self->radius)
		{
			VectorCopy(end, r->origin);
			return false;
		}
	}

	// Send collision event.
	CE_PostMessage(self, MSG_COLLISION, "g", trace); // This will be processed next.

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

	if (R_DETAIL == DETAIL_NORMAL)
		work *= 0.75f;
	else if (R_DETAIL == DETAIL_LOW)
		work *= 0.5f;

	return (int)max(work, 1.0f);
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

#pragma region ========================== r_entity sprite setup functions ==========================

//mxd. Expects square texture. Roll is in degrees.
void RE_SetupRollSprite(entity_t* ent, const float size, const float roll)
{
	ent->spriteType = SPRITE_DYNAMIC;

	const float radius = size * 0.5f;
	const float roll_sin = radius * sinf(roll * ANGLE_TO_RAD);
	const float roll_cos = radius * cosf(roll * ANGLE_TO_RAD);

	SVERTEX_SET(ent->verts[0], -roll_cos + roll_sin, -roll_sin - roll_cos, 0.0f, 1.0f); // Top-left.
	SVERTEX_SET(ent->verts[1], -roll_cos - roll_sin, -roll_sin + roll_cos, 0.0f, 0.0f); // Bottom-left.
	SVERTEX_SET(ent->verts[2],  roll_cos - roll_sin,  roll_sin + roll_cos, 1.0f, 0.0f); // Bottom-right.
	SVERTEX_SET(ent->verts[3],  roll_cos + roll_sin,  roll_sin - roll_cos, 1.0f, 1.0f); // Top-right.
}

//mxd
void RE_SetupFlipSprite(entity_t* ent, const float origin_x, const float origin_y, const float width, const float height, const qboolean flip_x, const qboolean flip_y)
{
	if (!flip_x && !flip_y)
		return;

	ent->spriteType = SPRITE_DYNAMIC;

	const float xl = -origin_x;
	const float xr = width - origin_x;
	const float yt = -origin_y;
	const float yb = height - origin_y;

	const float sl = (flip_x ? 1.0f : 0.0f);
	const float sr = 1.0f - sl;
	const float tt = (flip_y ? 1.0f : 0.0f);
	const float tb = 1.0f - tt;

	SVERTEX_SET(ent->verts[0], xl, yt, sl, tb); // Top-left.
	SVERTEX_SET(ent->verts[1], xl, yb, sl, tt); // Bottom-left.
	SVERTEX_SET(ent->verts[2], xr, yb, sr, tt); // Bottom-right.
	SVERTEX_SET(ent->verts[3], xr, yt, sr, tb); // Top-right.
}

#pragma endregion

// Tells if we have not rendered this reference point for a while.
qboolean RefPointsValid(const centity_t* owner)
{
	// RF_IGNORE_REFS check is necessary in case we're a chicken.
	return (owner->referenceInfo != NULL && !(owner->current.renderfx & RF_IGNORE_REFS) && owner->referenceInfo->lastUpdate - (float)fx_time <= REF_MINCULLTIME);
}

qboolean ReferencesInitialized(const centity_t* owner) //TODO: no longer needed?
{
	return (owner->referenceInfo != NULL);
}

// Add a blood splat or a scorchmark to the circular list - removing an entity that's next in the list if there is one.
void InsertInCircularList(client_entity_t* self)
{
	client_entity_t** root = &clientEnts;

	// If we have an entry already - delete it.
	if (circular_list[circular_list_index] != NULL)
	{
		// Search for this client entities entry in the client entity list.
		client_entity_t** current_p = root;
		for (const client_entity_t* current = *root; current != NULL; current = current->next)
		{
			if (current == circular_list[circular_list_index])
			{
				RemoveEffectFromList(current_p);
				break;
			}

			current_p = &(*current_p)->next;
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

	switch (R_DETAIL)
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

//mxd. Returns value based on pickup position (simplified version of undulate logic from R_EmitWaterPolys()).
float GetPickupBobPhase(const vec3_t origin)
{
	const float bob_phase = (origin[0] + origin[1]) * 0.0495f + (float)fx_time * 3.0f; // 0.0495 == 2.3 * 0.015.
	return fmodf(bob_phase, ANGLE_360); // Use remainder. Otherwise returned value will eventually cause floating point precision errors...
}

qboolean GetTruePlane(vec3_t origin, vec3_t direction, const float direction_scale, const float offset_scale) //mxd. 2 similar same-named functions (in fx_blood.c and fx_scorchmark.c) in original logic.
{
	VectorNormalize(direction); //mxd. Make sure it's normalized (can be non-normalized because of network transmission imprecision)...

	vec3_t end;
	VectorMA(origin, direction_scale, direction, end);

	trace_t trace;
	fxi.Trace(origin, vec3_origin, vec3_origin, end, MASK_DRIP, CTF_CLIP_TO_WORLD, &trace);

	if (trace.fraction != 1.0f)
	{
		// Set the new endpos and plane (should be exact).
		VectorCopy(trace.endpos, origin);
		VectorCopy(trace.plane.normal, direction);

		// Raise the decal slightly off the target wall.
		VectorMA(origin, offset_scale, direction, origin);

		return true;
	}

	return false;
}