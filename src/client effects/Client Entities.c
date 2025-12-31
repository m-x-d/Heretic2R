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

int fx_time; //mxd
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

client_entity_t* ClientEntity_new(const int type, const int flags, const vec3_t origin, const vec3_t direction, const int next_think_time)
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
	new_ent->r.color = color_white; //mxd
	new_ent->r.rootJoint = NULL_ROOT_JOINT;
	new_ent->r.swapFrame = NO_SWAP_FRAME;

	new_ent->alpha = 1.0f;
	new_ent->radius = 1.0f;
	new_ent->effectID = type;

	// Added this because we need to assume every client effect is culled before we do AddEffectsToView
	// for the first time to make sure the ViewStatusChanged fires.
	new_ent->flags = flags | CEF_CULLED;

	new_ent->updateTime = next_think_time;
	new_ent->framelerp_scale = 1.0f / (float)next_think_time; //mxd. Setup frame interpolation.
	new_ent->nextThinkTime = fx_time + next_think_time;
	new_ent->startTime = fx_time;
	new_ent->Update = RemoveSelfAI;

	return new_ent;
}

static void ClientEntity_delete(client_entity_t* to_delete, const centity_t* owner) //TODO: remove 'owner' arg (unused). 
{
	if (to_delete->p_root != NULL)
		RemoveParticleList(&to_delete->p_root);

	if (to_delete->dlight != NULL)
		CE_DLight_delete(to_delete->dlight);

	if (to_delete->r.fmnodeinfo != NULL)
		ResMngr_DeallocateResource(&fm_node_info_manager, to_delete->r.fmnodeinfo, sizeof(fmnodeinfo_t) * MAX_FM_MESH_NODES);

	if (to_delete->r.spriteType == SPRITE_VARIABLE && to_delete->r.verts_p != NULL)
		free(to_delete->r.verts_p);

	CE_ClearMessageQueue(to_delete);
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

// fx = type of effect to remove; 0 - remove all effects.
void RemoveEffectTypeList(client_entity_t** root, const FX_Type_t fx, const centity_t* owner)
{
	client_entity_t** prev;
	client_entity_t* current;

	assert(root);
	assert(*root); // FIXME: This shouldn't fire, but it does. The result is more or less harmless.

	for (prev = root, current = *root; current != NULL; current = current->next)
	{
		if (fx == FX_REMOVE_EFFECTS || current->effectID == fx)
			RemoveEffectFromList(prev, owner);
		else
			prev = &(*prev)->next;
	}
}

void PrepAddEffectsToView(void)
{
	const refdef_t* refdef = &fxi.cl->refdef;
	const float fov = max(refdef->fov_x, refdef->fov_y);

	view_fov = cosf(fov * 0.5f * ANGLE_TO_RAD * 1.2f);
	AngleVectors(refdef->viewangles, view_dir, NULL, NULL);
}

int AddEffectsToView(client_entity_t** root, centity_t* owner)
{
	int num_fx = 0;

	assert(root);
	assert(*root);

	for (client_entity_t* current = *root; current != NULL; current = current->next)
	{
		float dot;
		float dist;
		qboolean add_to_view = true; //mxd

		current->flags |= CEF_CULLED;

		// If we have a light, particles, a model, a potential routine that will change our think function, and we aren't disappeared, 
		// determine if we should be culled or not.
		if (current->dlight != NULL || current->p_root != NULL || current->r.model != NULL || !(current->flags & CEF_DISAPPEARED) || (current->flags & CEF_VIEWSTATUSCHANGED))
		{
			// Do this here since we do have an owner, and most probably we are tied to its origin, and we need that to do the proper culling routines.
			if (owner != NULL && current->AddToView != NULL)
				add_to_view = current->AddToView(current, owner);

			vec3_t dir;
			VectorSubtract(current->r.origin, fxi.cl->refdef.vieworg, dir);
			current->r.depth = VectorNormalize(dir);
			dist = current->r.depth;

			if (dist > r_farclipdist->value)
				continue;

			if (R_DETAIL == DETAIL_LOW && dist < r_nearclipdist->value && current->p_root == NULL && current->dlight == NULL) // Not a particle thing and not a dlight thing.
				continue;

			dot = DotProduct(dir, view_dir);
		}
		else
		{
			continue; // Has nothing to add to view.
		}

		if (current->dlight != NULL)
		{
			const CE_DLight_t* ce_dlight = current->dlight;

			if (fxi.cls->r_numdlights < MAX_DLIGHTS && ce_dlight->intensity > 0.0f &&
				dot + (ce_dlight->intensity * ce_dlight->intensity) / (dist * 300.0f) > view_fov) // 300.0 was determined by trial and error with intensities of 200 and 400.
			{
				dlight_t* dl = &fxi.cls->r_dlights[fxi.cls->r_numdlights++];

				VectorCopy(current->r.origin, dl->origin);
				dl->intensity = ce_dlight->intensity;
				COLOUR_COPY(ce_dlight->color, dl->color); //mxd. Use macro.
			}
		}

		// If no part of our radius is in the field of view, cull us.
		if (dot + (current->radius / dist) < view_fov)
			continue;

		// If we aren't within the current PVS, cull us. //mxd. Skip when RF_NODEPTHTEST is set (fixes lensflares disappearing when clipped by world geometry).
		if (!(current->r.flags & RF_NODEPTHTEST) && !fxi.InCameraPVS(current->r.origin))
			continue;

		// If we have an owner, and its server culled, and we want to check against it, cull us.
		if ((current->flags & CEF_CHECK_OWNER) && owner != NULL && (owner->flags & CF_SERVER_CULLED))
			continue;

		// We do this here because we don't have an owner - only do the update if we haven't already been culled.
		if (owner == NULL && current->AddToView != NULL)
			add_to_view = current->AddToView(current, owner);

		num_fx++;
		current->flags &= ~CEF_CULLED;

		if (current->p_root != NULL) // Add any particles.
			numrenderedparticles += AddParticlesToView(current);

		if (add_to_view && !(current->flags & (CEF_NO_DRAW | CEF_DISAPPEARED))) //mxd. +add_to_view check.
		{
			// Wacky all colors at minimum, but drawn at max instead for additive transparent sprites.
			current->alpha = max(0.0f, current->alpha);
			current->r.color.a = (byte)(current->alpha * 255.0f);

			if (current->r.color.a > 0 && current->r.scale > 0.0f)
			{
				if (current->flags & CEF_FRAME_LERP) //mxd
				{
					const float backlerp = 1.0f - (float)(fx_time - current->framelerp_time) * current->framelerp_scale;
					current->r.backlerp = max(0.0f, backlerp); // Otherwise it will advance towards negative infinity when the game is paused.
				}

				if (!AddEntityToView(&current->r))
					current->flags |= CEF_DROPPED;
			}
			else
			{
				current->flags |= CEF_DISAPPEARED;
			}
		}
	}

	return num_fx;
}

void AddEffect(centity_t* owner, client_entity_t* fx)
{
	AddEffectToList((owner != NULL ? &owner->effects : &clientEnts), fx);

	// Copy up the scale on a model so it can be culled properly.
	fx->r.cl_scale = fx->r.scale;
}

int UpdateEffects(client_entity_t** root, centity_t* owner)
{
#define NUM_TRACES	100 // I really, really hope we don't ever see more than this.

	//mxd. Original logic used single trace_t, which could result in unrelated trace being used by Debris_Collision() (because trace is sent via QPostMessage()).
	static trace_t traces[NUM_TRACES];
	static int trace_index = 0;

	client_entity_t** prev;
	client_entity_t* current;

	const float d_time = fxi.cls->rframetime;
	const float d_time2 = d_time * d_time * 0.5f;
	int cur_trace = 0;
	int num_fx = 0;

	assert(root);
	assert(*root);

	for (prev = root, current = *root; current != NULL; current = current->next)
	{
		num_fx++;

		if (current->msgHandler != NULL)
			CE_ProcessMessages(current);

		if (current->Update != NULL && current->nextThinkTime <= fx_time)
		{
			// Only think when not culled and not think culled.
			const qboolean is_culled = (current->flags & CEF_VIEWSTATUSCHANGED) && (current->flags & CEF_CULLED); //mxd
			if (!is_culled && !current->Update(current, owner))
			{
				RemoveEffectFromList(prev, owner);

				// current = current->next is still valid in the for loop.
				// A deallocated resource is guaranteed not to be changed until it is reallocated, when the manager is not shared between threads.
				continue;
			}

			//mxd. Original logic asserts here when current->updateTime < 17. Removed to allow updating client_ents every renderframe.
			current->nextThinkTime = fx_time + current->updateTime;
		}

		entity_t* r = &current->r;

		if (!(current->flags & (CEF_NO_DRAW | CEF_DISAPPEARED)))
		{
			const float d_size = d_time * current->d_scale;
			current->radius *= (1.0f + d_size / r->scale);

			r->scale += d_size;

			// Apply scale to spritelines as appropriate.
			if (current->r.spriteType == SPRITE_LINE)
			{
				// Either copy the scale to scale2, or use the d_scale2.
				if (current->flags & CEF_USE_SCALE2)
					r->scale2 += d_time * current->d_scale2; // Use the second scale
				else
					r->scale2 = r->scale; // Otherwise the second scale is copied from the first.
			}

			current->alpha += d_time * current->d_alpha;

			if (current->d_alpha > 0.0f && current->alpha >= 1.0f)
			{
				current->alpha = 0.99f;
				current->d_alpha = ((current->flags & CEF_PULSE_ALPHA) ? -current->d_alpha : 0.0f); // If these effects are increasing alpha, reverse and decrease with the PULSE_ALPHA flag.
			}
		}

		if (current->dlight != NULL)
			current->dlight->intensity += (d_time * current->dlight->d_intensity);

		if (current->p_root != NULL)
			numprocessedparticles += UpdateParticles(current);

		if (!(current->flags & CEF_NOMOVE) && (owner == NULL || (current->flags & CEF_DONT_LINK)))
		{
			if (current->flags & CEF_CLIP_TO_WORLD)
			{
				if (cur_trace < NUM_TRACES - 1) // Leave one at the end to continue checking collisions.
				{
					trace_t* trace = &traces[trace_index]; //mxd
					trace_index = (trace_index + 1) % (NUM_TRACES - 1);

					if (Physics_MoveEnt(current, d_time, d_time2, trace, true))
						cur_trace++; // Collided with something.
				}
				else
				{
					Com_DPrintf("Max Client Collisions exceeded by %d\n", cur_trace - (NUM_TRACES - 1));
					cur_trace++;
				}
			}
			else
			{
				if (current->r.spriteType != SPRITE_LINE)
				{
					// Update origin velocity based on acceleration.
					for (int i = 0; i < 3; i++)
					{
						r->origin[i] += current->velocity[i] * d_time + current->acceleration[i] * d_time2;
						current->velocity[i] += current->acceleration[i] * d_time;
					}
				}
				else
				{
					// Update the startpos and endpos velocity, then worry about the entity origin.
					vec3_t dpos;
					vec3_t dvel;
					vec3_t d2vel;

					VectorScale(current->velocity, d_time, dpos);		// velocity * dt
					VectorScale(current->acceleration, d_time, dvel);	// acceleration * dt
					VectorScale(current->acceleration, d_time2, d2vel);	// acceleration * dt ^ 2

					Vec3AddAssign(dpos, r->startpos);
					Vec3AddAssign(d2vel, r->startpos); // Calculate change in startpos.
					Vec3AddAssign(dvel, current->velocity); // Calculate change in velocity.

					// First, if we don't have auto origin flagged, we want to apply the velocity & such to the origin.
					if (!(current->flags & CEF_AUTO_ORIGIN))
					{
						Vec3AddAssign(dpos, r->origin);
						Vec3AddAssign(d2vel, r->origin); // Calculate change in origin. Sync with startpos.
					}
					// Else wait until the endpos is calculated, then update the origin.

					// Now, check to see if the endpos should use the same information, or maintain its own velocity.
					if (current->flags & CEF_USE_VELOCITY2)
					{
						// Figure out totally separate changes.
						VectorScale(current->velocity2, d_time, dpos);			// velocity2 * dt
						VectorScale(current->acceleration2, d_time, dvel);		// acceleration2 * dt
						VectorScale(current->acceleration2, d_time2, d2vel);	// acceleration2 * dt ^ 2

						Vec3AddAssign(dvel, current->velocity2); // Calculate change in velocity2.
					}

					Vec3AddAssign(dpos, r->endpos);
					Vec3AddAssign(d2vel, r->endpos); // Calculate change in endpos.

					// Now, if the AUTOORIGIN flag was set, then we haven't updated the origin yet. Do it now.
					if (current->flags & CEF_AUTO_ORIGIN)
					{
						VectorAdd(r->startpos, r->endpos, r->origin); // Get a midpoint by averaging out the startpos and endpos.
						Vec3ScaleAssign(0.5f, r->origin);
					}
				}
			}
		}

		prev = &(*prev)->next;
	}

	return num_fx;
}

qboolean AddEntityToView(entity_t* ent)
{
	if (ent->model == NULL || *ent->model == NULL)
		Com_DPrintf("AddEntityToView: NULL Model\n");

	if ((ent->flags & RF_TRANS_ADD) && (ent->flags & RF_ALPHA_TEXTURE))
		Com_DPrintf("AddEntityToView: Cannot have additive alpha mapped image. UNSUPPORTED!!\n");

	if ((ent->flags & RF_TRANS_ANY) || ent->color.a != 255)
	{
		if (fxi.cls->r_num_alpha_entities < MAX_ALPHA_ENTITIES)
		{
			fxi.cls->r_alpha_entities[fxi.cls->r_num_alpha_entities++] = ent;
			return true;
		}
	}
	else
	{
		if (fxi.cls->r_numentities < MAX_ENTITIES)
		{
			fxi.cls->r_entities[fxi.cls->r_numentities++] = ent;
			return true;
		}
	}

	return false;
}