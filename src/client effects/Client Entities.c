//
// Client Entities.c
//
// Copyright 1998 Raven Software
//

#include "Client Entities.h"
#include "Client Effects.h"
#include "ce_DLight.h"
#include "q_Sprite.h"
#include "g_playstats.h"
#include "Reference.h"
#include "ResourceManager.h"
#include "Particle.h"
#include "Skeletons.h"
#include "Vector.h"
#include "Utilities.h"

client_entity_t* clientEnts = NULL;

static ResourceManager_t entity_manager;
static ResourceManager_t fm_node_info_manager;
static vec3_t view_dir;
static float view_fov;

void InitEntityMngr(void)
{
#define ENTITY_BLOCK_SIZE 196

	ResMngr_Con(&entity_manager, sizeof(client_entity_t), ENTITY_BLOCK_SIZE);
}

void ReleaseEntityMngr(void)
{
	ResMngr_Des(&entity_manager);
}

void InitFMNodeInfoMngr(void)
{
#define FMNODEINFO_BLOCK_SIZE 16

	ResMngr_Con(&fm_node_info_manager, sizeof(fmnodeinfo_t) * MAX_FM_MESH_NODES, FMNODEINFO_BLOCK_SIZE);
}

void ReleaseFMNodeInfoMngr(void)
{
	ResMngr_Des(&fm_node_info_manager);
}

client_entity_t* ClientEntity_new(const int type, const int flags, vec3_t origin, vec3_t direction, const int next_think_time)
{
	client_entity_t* new_ent = ResMngr_AllocateResource(&entity_manager, sizeof(*new_ent));

	memset(new_ent, 0, sizeof(*new_ent));

	SLList_DefaultCon(&new_ent->msgQ.msgs);

	VectorCopy(origin, new_ent->r.origin);
	VectorCopy(origin, new_ent->origin);

	if (direction != NULL)
	{
		VectorCopy(direction, new_ent->direction);

		if (new_ent->direction[2] == 0.0f)
			VectorSet(new_ent->up, 0.0f, 0.0f, 1.0f); // Vertical wall.
		else if (new_ent->direction[0] == 0.0f && new_ent->direction[1] == 0.0f)
			VectorSet(new_ent->up, 1.0f, 0.0f, 0.0f); // Ceiling or floor.
		else
			PerpendicularVector(new_ent->up, new_ent->direction);
	}
	else
	{
		VectorSet(new_ent->direction, 1.0f, 0.0f, 0.0f);
		VectorSet(new_ent->up, 0.0f, 0.0f, 1.0f);
	}

	//TODO: currently either AnglesFromDirAndUp or PerpendicularVector isn't working properly. This will need to be fixed at some point.
	AnglesFromDirI(new_ent->direction, new_ent->r.angles);

	new_ent->r.scale = 1.0f;
	new_ent->r.color.c = 0xffffffff;
	new_ent->alpha = 1.0f;
	new_ent->radius = 1.0f;
	new_ent->effectID = type;

	// Added this because we need to assume every client effect is culled before we do AddEffectsToView
	// for the first time to make sure the ViewStatusChanged fires.
	new_ent->flags = flags | CEF_CULLED;

	new_ent->updateTime = next_think_time;
	new_ent->nextThinkTime = fxi.cl->time + next_think_time;
	new_ent->startTime = fxi.cl->time;
	new_ent->Update = RemoveSelfAI;
	new_ent->r.rootJoint = NULL_ROOT_JOINT;
	new_ent->r.swapFrame = -1;

	return new_ent;
}

static void ClientEntity_delete(client_entity_t* to_delete, const centity_t* owner)
{
	if (to_delete->p_root != NULL)
		RemoveParticleList(&to_delete->p_root);

	if (to_delete->dlight != NULL)
		CE_DLight_delete(to_delete->dlight);

	if (to_delete->r.fmnodeinfo != NULL)
		ResMngr_DeallocateResource(&fm_node_info_manager, to_delete->r.fmnodeinfo, sizeof(fmnodeinfo_t) * MAX_FM_MESH_NODES);

	if (owner != NULL && to_delete->refMask != 0)
		DisableRefPoints(owner->referenceInfo, to_delete->refMask);

	if (to_delete->r.spriteType == SPRITE_VARIABLE && to_delete->r.verts_p != NULL)
		free(to_delete->r.verts_p);

	ClearMessageQueue(to_delete);
	SLList_Des(&to_delete->msgQ.msgs);
	ResMngr_DeallocateResource(&entity_manager, to_delete, sizeof(*to_delete));
}

fmnodeinfo_t* FMNodeInfo_new(void)
{
	return ResMngr_AllocateResource(&fm_node_info_manager, sizeof(fmnodeinfo_t) * MAX_FM_MESH_NODES);
}

void AddEffectToList(client_entity_t** root, client_entity_t* fx)
{
	assert(root);

	fx->next = *root;
	*root = fx;
}

void RemoveEffectFromList(client_entity_t** root, const centity_t* owner)
{
	assert(root);
	assert(*root);

	client_entity_t* to_free = *root;
	*root = to_free->next;

	ClientEntity_delete(to_free, owner);
}

void RemoveEffectList(client_entity_t** root)
{
	assert(root);
	assert(*root);

	while (*root != NULL)
		RemoveEffectFromList(root, NULL);
}

void RemoveOwnedEffectList(centity_t* owner)
{
	client_entity_t** root = &owner->effects;

	assert(root);
	assert(*root);

	while (*root != NULL)
		RemoveEffectFromList(root, owner);
}

// fx = type of effect to remove
//    = 0, remove all effects

void RemoveEffectTypeList(client_entity_t **root, FX_Type_t fx, centity_t *owner)
{
	client_entity_t **prev;
	client_entity_t *current;

	assert(root);
//	assert(*root);		// FIXME:  This shouldn't fire, but it does.  The result is more or less harmless.

	for(prev = root, current = *root; current; current = current->next)
	{
		if(!fx || (current->effectID == fx))
		{
			RemoveEffectFromList(prev, owner);
			continue;
		}
		prev = &(*prev)->next; 
	}
}

void PrepAddEffectsToView()
{
	refdef_t *refDef;

	refDef = &fxi.cl->refdef;

	if(refDef->fov_x > refDef->fov_y)
	{
		view_fov = cos(refDef->fov_x*0.5*ANGLE_TO_RAD*1.2);
	}
	else
	{
		view_fov = cos(refDef->fov_y*0.5*ANGLE_TO_RAD*1.2);
	}

	AngleVectors (refDef->viewangles, view_dir, NULL, NULL);
}

int AddEffectsToView(client_entity_t **root, centity_t *owner)
{
	client_entity_t *current;
	vec3_t dir;
	int	numFX = 0;

	cl_camera_under_surface = Cvar_Get( "cl_camera_under_surface", "0", 0 );

	assert(root);
	assert(*root);

	for(current = *root; current; current = current->next)
	{
		float dot;
		float dist;

		current->flags |= CEF_CULLED;
		// if we have a light, particles, a model, a potential routine that will change our think function, and we aren't disappeared, 
		// determing if we should be culled or not.
		if(current->dlight || current->p_root || current->r.model
			|| (!(current->flags & (CEF_DISAPPEARED ))) || (current->flags & CEF_VIEWSTATUSCHANGED))
		{
			// do this here since we do have an owner, and most probably we are tied to its origin, and we need that to do the proper culling routines
			if(owner && current->AddToView)
			{
				current->AddToView(current, owner);
			}

			VectorSubtract(current->r.origin, fxi.cl->refdef.vieworg, dir);

			current->r.depth = dist = VectorNormalize(dir);

			if(dist > r_farclipdist->value)
			{
				continue;
			}

			if(r_detail->value == DETAIL_LOW && dist < r_nearclipdist->value && !current->p_root && !current->dlight)//not a particle thing and not a dlight thing
			{
				continue;
			}
			dot = DotProduct(dir, view_dir);
		}
		else
		{
			continue;	// has nothing to add to view
		}

		if(current->dlight)
		{
			CE_DLight_t *ce_dlight = current->dlight;

			if (fxi.cls->r_numdlights < MAX_DLIGHTS)
			{
				if(ce_dlight->intensity > 0.0)
				{
					if((dot + (ce_dlight->intensity*ce_dlight->intensity)/(dist*300.0f)) > view_fov) // 300.0 was determined by trial and error with intensities of 200 and 400
					{
						dlight_t *dl;

						dl = &fxi.cls->r_dlights[fxi.cls->r_numdlights++];

						VectorCopy (current->r.origin, dl->origin);

						dl->intensity = ce_dlight->intensity;

						dl->color.r = ce_dlight->color.r;
						dl->color.g = ce_dlight->color.g;
						dl->color.b = ce_dlight->color.b;
					}
				}
			}
		}

		// if no part of our radius is in the field of view or we aren't within the current PVS, cull us.
		if(((dot + (current->radius/dist)) < view_fov) || !(fxi.InCameraPVS(current->r.origin)) ||
			// if we have an owner, and its server culled, and we want to check against it then do so
			(owner && (owner->flags & CF_SERVER_CULLED) && (current->flags & CEF_CHECK_OWNER)))
		{
			continue;
		}

		// we do this here cos we don't have an owner - only do the update if we haven't already been culled.
		if(!owner && current->AddToView)
		{
			current->AddToView(current, owner);
		}

		++numFX;
		current->flags &= ~CEF_CULLED;

		if(current->p_root)	// add any particles
		{
			numrenderedparticles += AddParticlesToView(current);
		}

		if(!(current->flags & (CEF_NO_DRAW | CEF_DISAPPEARED)))
		{
			if(current->alpha < 0)
			{	// wacky all colors at minimum, but drawn at max instead for addative transparent sprites
				current->alpha = 0.0F;
			}

			current->r.color.a = Q_ftol(current->alpha * 255.0);

			if(current->r.color.a && (current->r.scale > 0.0))
			{
				if(!AddEntityToView(&current->r))
				{
					current->flags |= CEF_DROPPED;
				}
			}
			else
			{
				current->flags |= CEF_DISAPPEARED;
			}
		}
	}
	return numFX;
}

void AddEffect(centity_t *owner, client_entity_t *fx)
{
	if(owner)
	{
		AddEffectToList(&owner->effects, fx);
		if(owner->referenceInfo && fx->refMask)
		{
			EnableRefPoints(owner->referenceInfo, fx->refMask);
		}
	}
	else
	{
		AddEffectToList(&clientEnts, fx);
	}

	// copy up the scale on a model so it can be culled properly
	fx->r.cl_scale = fx->r.scale;
}

#define NUM_TRACES 100		// I really, really hope we don't ever see more than this

int UpdateEffects(client_entity_t **root, centity_t *owner)
{
	extern	int ParticleUpdateTime;

	client_entity_t **prev;
	client_entity_t *current;
	entity_t		*r;
	float			d_time = fxi.cls->frametime;
	float			d_time2 = d_time * d_time * 0.5;
	static trace_t	traces[NUM_TRACES];
	static trace_t	traces2[NUM_TRACES];
	static qboolean	useOtherTraces = false;
	trace_t			*trace;
	int				curTrace = 0;
	int				numFX = 0;
	int				curTime = fxi.cl->time;

	assert(root);
	assert(*root);

	// If the world is frozen then add the particles, just don't update the world time.
	// Always update the particle timer
	if (!fx_FreezeWorld)
	{
		ParticleUpdateTime = fxi.cl->time;
	}

	if(!useOtherTraces)
	{
		trace = traces;
	}
	else
	{
		trace = traces2;
	}

	useOtherTraces = !useOtherTraces;

	for(prev = root, current = *root; current; current = current->next)
	{
		numFX++;

		if(current->msgHandler)
		{
			ProcessMessages(current);
		}

		if(current->Update)
		{
			if(current->nextThinkTime <= curTime)
			{
				// Only think when not culled and not think culled
				if(!((current->flags & CEF_VIEWSTATUSCHANGED) && (current->flags & CEF_CULLED)))
				{
					if(!current->Update(current, owner))
					{
						RemoveEffectFromList(prev, owner);
						// current = current->next is still valid in the for loop.
						// a deallocated resource is guaranteed not to be changed until it is
						// reallocated, when the mananger is not shared between threads
						continue;
					}
				}
				assert(current->updateTime > 16);
				current->nextThinkTime = curTime + current->updateTime;
			}
		}

		r = &current->r;

		if(!(current->flags & (CEF_NO_DRAW|CEF_DISAPPEARED)))
		{
			float d_size;

			d_size = d_time * current->d_scale;

			current->radius *= (1 + d_size/r->scale);

			r->scale += d_size;

			// Apply scale to spritelines as appropriate.
			if (current->r.spriteType == SPRITE_LINE)
			{
				// Either copy the scale to scale2, or use the d_scale2.
				if (current->flags & CEF_USE_SCALE2)
				{	// Use the second scale
					d_size = d_time * current->d_scale2;
					r->scale2 += d_size;
				}
				else
				{	// Otherwise the second scale is copied from the first.
					r->scale2 = r->scale;
				}
			}

			current->alpha += d_time * current->d_alpha;
			
			if (current->d_alpha > 0 && current->alpha >= 1.0)
			{
				current->alpha = 0.99;
				if (current->flags & CEF_PULSE_ALPHA)
				{	// If these effects are increasing alpha, reverse and decrease with the PULSE_ALPHA flag.
					current->d_alpha = -current->d_alpha;
				}
				else
				{
					current->d_alpha = 0.0;
				}
			}
		}

		if(current->dlight)
		{
			current->dlight->intensity += (d_time * current->dlight->d_intensity);
		}
		if(current->p_root)
		{
			numprocessedparticles += UpdateParticles(current);
		}

		if(!(current->flags & CEF_NOMOVE))
		{
			if(!owner || (current->flags&CEF_DONT_LINK))
			{
				if(current->flags & CEF_CLIP_TO_WORLD)
				{
					if(curTrace < NUM_TRACES - 1) // leave one at the end to continue checking collisions
					{
						if(Physics_MoveEnt(current, d_time, d_time2, trace))
						{	// collided with something
							++trace;
							++curTrace;
						}
					}
					else
					{
						Com_DPrintf("Max Client Collisions exceeded by %d\n", curTrace - (NUM_TRACES - 1));
						++curTrace;
					}
				}
				else
				{
					if (current->r.spriteType != SPRITE_LINE)
					{	// Update origin velocity based on acceleration.
						r->origin[0] += current->velocity[0] * d_time + current->acceleration[0] * d_time2;
						r->origin[1] += current->velocity[1] * d_time + current->acceleration[1] * d_time2;
						r->origin[2] += current->velocity[2] * d_time + current->acceleration[2] * d_time2;

						current->velocity[0] += current->acceleration[0] * d_time;
						current->velocity[1] += current->acceleration[1] * d_time;
						current->velocity[2] += current->acceleration[2] * d_time;
					}
					else 
					{	// Update the startpos and endpos velocity, then worry about the entity origin.
						vec3_t	dpos, dvel, d2vel;

						VectorScale(current->velocity, d_time, dpos);		// velocity*dt
						VectorScale(current->acceleration, d_time, dvel);	// acceleration*dt
						VectorScale(current->acceleration, d_time2, d2vel);	// acceleration*dt^2

						VectorAdd(r->startpos, dpos, r->startpos);
						VectorAdd(r->startpos, d2vel, r->startpos);					// Calculate change in startpos
						VectorAdd(current->velocity, dvel, current->velocity);	// Calculate change in velocity

						// First, if we don't have auto origin flagged, we want to apply the velocity & such to the origin.
						if (!(current->flags & CEF_AUTO_ORIGIN))
						{
							VectorAdd(r->origin, dpos, r->origin);
							VectorAdd(r->origin, d2vel, r->origin);		// Calculate change in origin.  Sync with startpos.
						}
						//	else wait until the endpos is calculated, then update the origin.

						// Now, check to see if the endpos should use the same information, or maintain its own velocity.
						if (!(current->flags & CEF_USE_VELOCITY2))
						{
							VectorAdd(r->endpos, dpos, r->endpos);
							VectorAdd(r->endpos, d2vel, r->endpos);					// Calculate change in endpos
						}
						else
						{	// Figure out totally seperate changes
							VectorScale(current->velocity2, d_time, dpos);		// velocity2*dt
							VectorScale(current->acceleration2, d_time, dvel);	// acceleration2*dt
							VectorScale(current->acceleration2, d_time2, d2vel);	// acceleration2*dt^2

							VectorAdd(r->endpos, dpos, r->endpos);
							VectorAdd(r->endpos, d2vel, r->endpos);					// Calculate change in endpos
							VectorAdd(current->velocity2, dvel, current->velocity2);	// Calculate change in velocity2
						}

						// Now, if the AUTOORIGIN flag was set, then we haven't updated the origin yet.  Do it now.
						if (current->flags & CEF_AUTO_ORIGIN)
						{
							VectorAdd(r->startpos, r->endpos, r->origin);		// Get a midpoint by
							VectorScale(r->origin, 0.5, r->origin);				// averaging out the startpos and endpos.
						}
					}
				}
			}
		}

		prev = &(*prev)->next; 
	}

	return numFX;
}

qboolean AddEntityToView(entity_t *ent)
{
	if (!ent->model || !*ent->model)
	{
		Com_DPrintf ("AddEntityToView: NULL Model\n");
	}
	if((ent->flags & RF_TRANS_ADD) && (ent->flags & RF_ALPHA_TEXTURE))
	{
		Com_DPrintf ("AddEntityToView: Cannot have additive alpha mapped image. UNSUPPORTED !!\n");
	}

	if((ent->flags & RF_TRANS_ANY) || (ent->color.a != 255))
	{
		if(fxi.cls->r_num_alpha_entities < MAX_ALPHA_ENTITIES)
		{
			fxi.cls->r_alpha_entities[fxi.cls->r_num_alpha_entities++] = ent;
		}
		else
		{
			return false;	
		}
	}
	else
	{
		if(fxi.cls->r_numentities < MAX_ENTITIES)
		{
			fxi.cls->r_entities[fxi.cls->r_numentities++] = ent;
		}
		else
		{
			return false;	
		}
	}

	return true;
}
// end
