#include "Utilities.h"

#include "Client Effects.h"
#include "FX.h"
#include "random.h"
#include "Matrix.h"
#include "Vector.h"
#include "Reference.h"
#include "motion.h"
#include "Particle.h"
#include "ce_DLight.h"
#include "g_playstats.h"
#include "Skeletons.h"


// setup for circular list
client_entity_t *CircularList[MAX_ENTRIES_IN_CIRCLE_LIST + 1];
int				CurrentCirclePointer = 0;
int				total_circle_entries;
extern void RemoveEffectFromList(client_entity_t **root, centity_t *owner);


//------------------------------------------------------------------
// Update funcs
//------------------------------------------------------------------

qboolean RemoveSelfAI(client_entity_t *this, centity_t *owner)
{
	return(false);	// removed after one think (nextThinkTime is lifetime)
}
qboolean KeepSelfAI(client_entity_t *this, centity_t *owner)
{
	return(true);	// Remain alive forever
}

qboolean AttemptRemoveSelf(client_entity_t *self, centity_t *owner)
{
	if(self->flags & (CEF_NO_DRAW|CEF_DISAPPEARED|CEF_DROPPED))
	{
		return false;
	}

	if(self->flags & CEF_CULLED)
	{
		if(self->r.depth > CFX_CULLING_DIST)
		{
			return false;
		}
	}
	else
	{
		if(self->r.depth > r_farclipdist->value)
		{
			return false;
		}		
	}

	self->updateTime = 400;

	return true;
}

//------------------------------------------------------------------
// AddToView funcs
//------------------------------------------------------------------

qboolean LinkedEntityUpdatePlacement(client_entity_t *current, centity_t *owner)
{
	matrix3_t rotation;
	vec3_t up, direction;

	if(current->r.flags & RF_FIXED)
	{
		Matrix3FromAngles(owner->lerp_angles, rotation);

		Matrix3MultByVec3(rotation, current->direction, direction);
		Matrix3MultByVec3(rotation, current->up, up);

		AnglesFromDirAndUp(direction, up, current->r.angles);
	}

	current->r.origin[0] = owner->origin[0];
	current->r.origin[1] = owner->origin[1];
	current->r.origin[2] = owner->origin[2];

	return true;
}

qboolean OffsetLinkedEntityUpdatePlacement(client_entity_t *current, centity_t *owner)
{
	matrix3_t rotation;
	vec3_t up, direction;
	vec3_t up2, direction2;
	vec3_t origin;

	Matrix3FromAngles(owner->lerp_angles, rotation);

	Matrix3MultByVec3(rotation, current->origin, origin);

	if(current->r.flags & RF_FIXED)
	{
		VectorAdd(current->origin, current->direction, direction);
		VectorAdd(current->origin, current->up, up);

		Matrix3MultByVec3(rotation, direction, direction2);
		Matrix3MultByVec3(rotation, up, up2);

		Vec3SubtractAssign(origin, direction2);
		Vec3SubtractAssign(origin, up2);

		AnglesFromDirAndUp(direction2, up2, current->r.angles);
	}

	current->r.origin[0] = owner->origin[0] + origin[0];
	current->r.origin[1] = owner->origin[1] + origin[1];
	current->r.origin[2] = owner->origin[2] + origin[2];

	return true;
}

qboolean ReferenceLinkedEntityUpdatePlacement(struct client_entity_s *self, centity_t *owner)
{
	matrix3_t rotation;
	vec3_t up, direction;
	vec3_t up2, direction2;
	vec3_t origin;

	// This tells if we are wasting our time, because the reference points are culled.
	if (!RefPointsValid(owner))
		return true;

	VectorCopy(owner->referenceInfo->references[self->refPoint].placement.origin, self->origin);

	Matrix3FromAngles(owner->lerp_angles, rotation);

	Matrix3MultByVec3(rotation, self->origin, origin);

	if(self->r.flags & RF_FIXED)
	{
		VectorAdd(self->origin, self->direction, direction);
		VectorAdd(self->origin, self->up, up);

		Matrix3MultByVec3(rotation, direction, direction2);
		Matrix3MultByVec3(rotation, up, up2);

		Vec3SubtractAssign(origin, direction2);
		Vec3SubtractAssign(origin, up2);

		AnglesFromDirAndUp(direction2, up2, self->r.angles);
	}

	self->r.origin[0] = owner->origin[0] + origin[0];
	self->r.origin[1] = owner->origin[1] + origin[1];
	self->r.origin[2] = owner->origin[2] + origin[2];

	return true;
}

//------------------------------------------------------------------
// Message Response Helper Funcs
//------------------------------------------------------------------

void BecomeStatic(client_entity_t *self)
{
	VectorClear(self->velocity);
	VectorClear(self->acceleration);

	self->flags &= ~CEF_CLIP_TO_WORLD;
}

//------------------------------------------------------------------
// Physics Funcs
//------------------------------------------------------------------

// Returns whether solid world is there
// Sets dist to distance to solid or maxdist if no solid
// If maxdist < 0 - returns distance to floor
// If maxdist > 0 - returns distance to ceiling

int GetSolidDist(vec3_t origin, vec_t radius, float maxdist, vec_t *dist)
{
	vec3_t		mins, maxs, end;
	trace_t		trace;

	VectorSet(mins, -radius, -radius, -1.0F);
	VectorSet(maxs, radius, radius, 1.0F);
	VectorCopy(origin, end);
	end[2] += maxdist;
	fxi.Trace(origin, mins, maxs, end, MASK_DRIP, CEF_CLIP_TO_WORLD, &trace);

	if(trace.fraction == 1.0F)
	{
		*dist = maxdist;
		return(false);
	}
	*dist = trace.endpos[2] - origin[2];
	return(true);
}

// Gets time for a ce to fall to the ground

int GetFallTime(vec3_t origin, vec_t veloc, vec_t accel, vec_t radius, float maxtime, trace_t *trace)
{
	vec3_t		mins, maxs, end;
	float		time;

	VectorSet(mins, -radius, -radius, -1.0F);
	VectorSet(maxs, radius, radius, 1.0F);
	VectorCopy(origin, end);
	end[2] += (veloc * maxtime) + accel * (maxtime * maxtime) * 0.5;		// from s = ut + 0.5at^2

	fxi.Trace(origin, mins, maxs, end, MASK_DRIP, CEF_CLIP_TO_WORLD, trace);
	time = GetTimeToReachDistance(veloc, accel, trace->endpos[2] - origin[2]);
	return(time);													// in ms
}

// Returns false if no plane found b4 maxdist travelled, or non water plane hit
// Returns true, the plane normal of plane hit and the distance to the plane if a plane hit

int GetWaterNormal(vec3_t origin, float radius, float maxdist, vec3_t normal, vec_t *dist)
{
	vec3_t		mins, maxs, start, end;
	trace_t		trace;

	VectorSet(mins, -radius, -radius, -1.0F);
	VectorSet(maxs, radius, radius, 1.0F);
	VectorCopy(origin, end);
	VectorCopy(origin, start);
	start[2] += maxdist;
	end[2] -= maxdist;

	fxi.Trace(origin, mins, maxs, end, MASK_DRIP, CEF_CLIP_TO_WORLD, &trace);
	if((trace.fraction == 1.0F) || (trace.contents & MASK_SOLID))
		return(false);

	VectorCopy(trace.plane.normal, normal);
	*dist = (end[2] - origin[2]) * trace.fraction;
	return(true);
}

void FXDoWaterEntrySplash(centity_t *Owner,int Type,int Flags,vec3_t Origin, byte SplashSize, vec3_t Dir);
void FXWaterRipples(centity_t *Owner, int Type, int Flags, vec3_t Origin);
void FXBubble(centity_t *Owner, int Type, int Flags, vec3_t Origin);
void FireSparks(centity_t *owner, int type, int flags, vec3_t origin, vec3_t dir);
void FXDarkSmoke(vec3_t origin, float scale, float range);
qboolean FXDebris_Vanish(struct client_entity_s *self,centity_t *owner);
qboolean FXDebris_Remove(struct client_entity_s *self,centity_t *owner);

void FizzleEffect (client_entity_t *self, vec3_t surface_top, vec3_t normal)
{
	vec3_t spot;
	int	num_puffs, i;

	if(self)
	{
		if(self->dlight)
			self->dlight->intensity = 0;//lights out
	}
	if(irand(0, 3))
	{
		fxi.S_StartSound(surface_top, -1, CHAN_AUTO,
			fxi.S_RegisterSound(va("ambient/lavadrop%c.wav", irand('1', '3'))), 1, ATTN_STATIC, 0);
	}
	else
	{
		fxi.S_StartSound(surface_top, -1, CHAN_AUTO,
			fxi.S_RegisterSound("misc/lavaburn.wav"), 1, ATTN_STATIC, 0);
	}

	num_puffs = GetScaledCount(irand(2, 5), 0.3);
	for(i = 0; i<num_puffs; i++)
	{
		spot[0] = surface_top[0] + flrand(-3, 3);
		spot[1] = surface_top[1] + flrand(-3, 3);
		spot[2] = surface_top[2] + flrand(0, 3);
		FXDarkSmoke(spot, flrand(0.2, 0.5), flrand(30, 50));
	}
	FireSparks(NULL, FX_SPARKS, 0, surface_top, normal);
}

qboolean Physics_MoveEnt(client_entity_t *self, float d_time, float d_time2, trace_t *trace)
{
	vec3_t		end, move, attempt;
	vec3_t		mins, maxs, dir, surface_top;
	float		hit_angle;
	int			material;
	entity_t	*r = &self->r;
	qboolean	do_effect = false;

	if(Vec3IsZero(self->velocity))
		return false;

	if(Vec3IsZero(self->acceleration))
		return false;

	//BTW, make more debris leave particle trails, like dust for wood, pebbles for rock, etc.
	if(self->SpawnInfo&SIF_INWATER)
	{
		//leave several bubbles?
		d_time*=0.5;
		d_time2*=0.5;
	}
	else if(self->SpawnInfo&SIF_INLAVA)
	{
		d_time*=0.01;
		d_time2*=0.01;
	}
	else if(self->SpawnInfo&SIF_INMUCK)
	{
		//leave a ripple or two?
		d_time*=0.05;
		d_time2*=0.05;
	}

	attempt[0] = self->velocity[0] * d_time + self->acceleration[0] * d_time2;
	attempt[1] = self->velocity[1] * d_time + self->acceleration[1] * d_time2;
	attempt[2] = self->velocity[2] * d_time + self->acceleration[2] * d_time2;

	// may need an individual mins and maxs, probably want to put into an optional
	// physics_info struct
	mins[0] = mins[1] = mins[2] = -(maxs[0] = maxs[1] = maxs[2] = self->radius);

	VectorAdd(r->origin, attempt, end);

	fxi.Trace(r->origin, mins, maxs, end, MASK_SHOT|MASK_WATER, self->flags, trace);

	if((trace->fraction < 1.0) && !trace->allsolid && !trace->startsolid && !Vec3IsZeroEpsilon(trace->plane.normal))
	{
		if(trace->surface->flags&SURF_SKY)
		{//remove it
			self->Update = FXDebris_Remove;
			self->updateTime = fxi.cl->time + 0.1;
			return false;
		}

		d_time *= trace->fraction;

		VectorScale(attempt, d_time, move);

		Vec3AddAssign(move, r->origin);

		self->velocity[0] += self->acceleration[0] * d_time;
		self->velocity[1] += self->acceleration[1] * d_time;
		self->velocity[2] += self->acceleration[2] * d_time;

		VectorCopy(attempt, dir);
		VectorNormalize(dir);

		VectorMA(r->origin, self->radius*0.5, dir, surface_top);

		hit_angle = DotProduct(dir, trace->plane.normal);

		if(r_detail->value < DETAIL_UBERHIGH || !irand(0, 1))
			do_effect = true;

		if(trace->contents & CONTENTS_WATER && !(self->SpawnInfo&SIF_INWATER))
		{
			if(self->flags&CEF_FLAG6)
			{
				self->flags &= ~CEF_FLAG6;
				FizzleEffect(self, surface_top, trace->plane.normal);
			}

			self->SpawnInfo |= SIF_INWATER;//in water now, sink a little slower
			//spawn ripples, splash
			if(do_effect)
			{
				FXDoWaterEntrySplash(NULL, FX_WATER_ENTRYSPLASH, 0, surface_top, 64, trace->plane.normal);
			}


			if(flrand(-0.5, 0) < hit_angle)
			{//splash sound
				if(do_effect)
				{
					fxi.S_StartSound(r->origin, -1, CHAN_AUTO,
							fxi.S_RegisterSound(va("misc/splish%c.wav", irand('2', '3'))), 1, ATTN_STATIC, 0);
				}
				QPostMessage(self, MSG_COLLISION, "g", trace);	// this will be processed next
				return true;
			}
			
			material = self->SpawnInfo & SIF_FLAG_MASK;
			if(material!=MAT_WOOD)//wood floats, everything else can keep sinking
			{//bubbles and blurp sound
				if(do_effect)
				{
					FXBubble(NULL, FX_BUBBLE, 0, surface_top);
					fxi.S_StartSound(r->origin, -1, CHAN_AUTO,
							fxi.S_RegisterSound("misc/splish1.wav"), 1, ATTN_STATIC, 0);
				}
				fxi.Trace(trace->endpos, mins, maxs, end, MASK_SHOT, self->flags, trace);
				if(trace->fraction < 1.0)
				{
					d_time *= trace->fraction;

					VectorScale(attempt, d_time, move);

					Vec3AddAssign(move, r->origin);

					self->velocity[0] += self->acceleration[0] * d_time;
					self->velocity[1] += self->acceleration[1] * d_time;
					self->velocity[2] += self->acceleration[2] * d_time;
				}
				else
				{
					Vec3AddAssign(attempt, r->origin);

					self->velocity[0] += self->acceleration[0] * d_time;
					self->velocity[1] += self->acceleration[1] * d_time;
					self->velocity[2] += self->acceleration[2] * d_time;
				}
			}
			else//sit on surface
			{//splash sound
				if(do_effect)
				{
					fxi.S_StartSound(r->origin, -1, CHAN_AUTO,
							fxi.S_RegisterSound(va("player/waterrun%c.wav", irand('1', '2'))), 1, ATTN_STATIC, 0);
				}
				VectorCopy(surface_top, r->origin);
				VectorClear(self->velocity);
				VectorClear(self->acceleration);
				self->d_alpha = -0.2;
				self->Update = FXDebris_Vanish;
				self->updateTime = fxi.cl->time + 0.1;
			}
			return false;//no need to update trace counter if not sending collision
		}
		else if(trace->contents & CONTENTS_SLIME&& !(self->SpawnInfo&SIF_INMUCK))
		{
			if(self->flags&CEF_FLAG6)
			{
				self->flags &= ~CEF_FLAG6;
				if(do_effect)
				{
					FizzleEffect(self, surface_top, trace->plane.normal);
				}
			}

			self->SpawnInfo |= SIF_INMUCK;//in muck, sink really really slowly
			//spawn ripples, splash
			if(do_effect)
			{
				FXDoWaterEntrySplash(NULL, FX_WATER_ENTRYSPLASH, 0, surface_top, 64, trace->plane.normal);
			}

			if(flrand(-0.75, 0) < hit_angle)
			{//splash sound
				if(do_effect)
				{
					fxi.S_StartSound(r->origin, -1, CHAN_AUTO,
						fxi.S_RegisterSound(va("player/waterrun%c.wav", irand('1', '2'))), 1, ATTN_STATIC, 0);
				}
				QPostMessage(self, MSG_COLLISION, "g", trace);	// this will be processed next
				return true;
			}

			//bubbles and blurp sound
			if(do_effect)
			{
				FXBubble(NULL, FX_BUBBLE, 0, surface_top);
				fxi.S_StartSound(r->origin, -1, CHAN_AUTO,
						fxi.S_RegisterSound("objects/submerge.wav"), 1, ATTN_STATIC, 0);
			}
			VectorCopy(surface_top, r->origin);
			self->d_alpha = -0.01;
			self->Update = FXDebris_Vanish;
			self->updateTime = fxi.cl->time + 0.1;

			return false;//no need to update trace counter if not sending collision
		}
		else if(trace->contents & CONTENTS_LAVA && !(self->SpawnInfo&SIF_INLAVA))
		{
			self->flags &= ~CEF_FLAG6;
			self->SpawnInfo |= SIF_INLAVA;//in lava now, continue to burn
			//smoke puffs and sizzle here
			if(do_effect)
			{
				FizzleEffect(self, surface_top, trace->plane.normal);
			}
			VectorCopy(surface_top, r->origin);
			self->d_scale = -0.2;
			self->Update = FXDebris_Vanish;
			self->updateTime = fxi.cl->time + 0.1;
			return false;
		}

		QPostMessage(self, MSG_COLLISION, "g", trace);	// this will be processed next
														// works just like a recursive call
		return true;
	}
	else
	{
		Vec3AddAssign(attempt, r->origin);

		self->velocity[0] += self->acceleration[0] * d_time;
		self->velocity[1] += self->acceleration[1] * d_time;
		self->velocity[2] += self->acceleration[2] * d_time;
	}

	return false;
}

// -----------------------------------------------------------------
// Takes desired count and scales it by the desired frame rates
// relationship with actual framerate
// Second parameter is the impact of this effect on the renderer (0.0 - 1.0)
// eg Huge transparent alpha mapped sprites should have a high refdepend.
// Due to the limits of cyrix processors, we have no way of accurately
// determining the time spent in the ref dll, so this feature is not
// implemented yet.

int GetScaledCount(int count, float refdepend)
{
	float	work;

	// if we are doing a time demo, we don't want scaleability
	if (cl_timedemo->value)
		work = count;
	else
		work = count * fxi.cls->framemodifier;

	if (r_detail->value == DETAIL_NORMAL)
		work = work * 0.75;
	else
	if (r_detail->value == DETAIL_LOW)
		work = work * 0.5;

	if(work < 1.0)
	{
		work = 1.0;
	}


	return(Q_ftol(work));
}

void AdvanceParticle(client_particle_t *p, int ms)
{
	GetPositionOverTime(p->origin, p->velocity, p->acceleration, ms * 0.001, p->origin);
	GetVelocityOverTime(p->velocity, p->acceleration, ms * 0.001, p->velocity);
}

// -----------------------------------------------------------------
// We don`t have access to "sv_gravity" so we keep a local client fx
// copy. It is set to same default as on the server.
		
float GetGravity()
{
	return(-clfx_gravity->value);
}

// -----------------------------------------------------------------

// Tells if we have not rendered this reference point for a while.
qboolean RefPointsValid(centity_t *owner)
{
	if (owner->referenceInfo==NULL || 
			owner->current.renderfx & RF_IGNORE_REFS ||			// This one is necessary in case we're a chicken.
			owner->referenceInfo->lastUpdate - fxi.cl->time > REF_MINCULLTIME)
		return false;
	else
		return true;
}



// FIXME Obsolete
qboolean ReferencesInitialized(centity_t *owner)
{
	if(!owner->referenceInfo)
	{	// this probably shouldn't happen, if it does, let me know, JKH
//		assert(0);
		return false;
	}

	return true;
}

// FIXME Obsolete
void EnableRefPoints(LERPedReferences_t *refInfo, int mask)
{
//	assert(refInfo);
}

// FIXME Obsolete
void DisableRefPoints(LERPedReferences_t *refInfo, int mask)
{
//	assert(refInfo);
}

// add a blood splat or a scorchmark to the circular list - removing an entity thats next in the list if there is one
void InsertInCircularList(client_entity_t *self)
{
    client_entity_t **root;
	client_entity_t **prev;
	client_entity_t *current;

	root = &clientEnts;

	// if we have an entry already - delete it
	if(CircularList[CurrentCirclePointer])
	{
		// search for this client entities entry in the client entity list
		for(prev = root, current = *root; current; current = current->next)
		{
			if(current == CircularList[CurrentCirclePointer])
			{
				RemoveEffectFromList(prev,NULL);
				break;
			}
			prev = &(*prev)->next; 
		}
	}
	// add in new one
	CircularList[CurrentCirclePointer] = self;
	CurrentCirclePointer++;
	// delimit the pointer
	if (CurrentCirclePointer >= total_circle_entries)
		CurrentCirclePointer = 0;
}


// -----------------------------------------------------------------
// end