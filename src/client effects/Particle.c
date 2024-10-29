//
// Particle.c
//
// Copyright 1998 Raven Software
//
// Heretic II
//

#include "Particle.h"
#include "Client Effects.h"
#include "FX.h"
#include "ResourceManager.h"
#include "Client Entities.h"
#include "Random.h"
#include "Vector.h"
#include "g_playstats.h"

#define	MAX_PARTS_PER_CE	2048

ResourceManager_t ParticleMngr;

int ParticleUpdateTime=0;

void InitParticleMngrMngr()
{
#define PARTICLE_BLOCK_SIZE 256

	ResMngr_Con(&ParticleMngr, sizeof(client_particle_t), PARTICLE_BLOCK_SIZE);
}

void ReleaseParticleMngrMngr()
{
	ResMngr_Des(&ParticleMngr);
}

void AddParticleToList(client_entity_t *ce, client_particle_t *fx)
{
	assert(ce);
	assert(fx);

	fx->next = ce->p_root;
	ce->p_root = fx;

#if	_DEBUG
	// Check to see that a ce isn`t spawning particles
	// while not in the view. This is a HUGE memory leak
	// which is most likely responsible for the chug.

	// INSTRUCTIONS TO FIX
	// Find the client effect that called this spawn function (one level up in call stack)
	// Either (a) tell the person who wrote the bugged code in the first place to correct it
	// or (b) emulate the ViewStatusChanged functionality of the fire or the fountain (this is a 
	// 5 minute fix at most)
	{
		int					i = 0;
		client_particle_t	*p = ce->p_root;

		while(p)
		{
			i++;
			p = p->next;
		}
		if(i > MAX_PARTS_PER_CE)
		{
			Com_DPrintf("Memory leak due to lack of CEF_VIEWSTATUSCHANGED\n");
		}
	}
#endif
}

void RemoveParticleList(client_particle_t **root)
{
	client_particle_t *next;
	client_particle_t *toFree;

	assert(root);

	next = *root;

	while(next)
	{
		toFree = next;

		next = next->next;

		ResMngr_DeallocateResource(&ParticleMngr, toFree, sizeof(*toFree));
	}

	*root = NULL;
}

int AddParticlesToView(client_entity_t *ce)
{
	client_particle_t	*current;
	client_particle_t	**prev;
	float				d_time, d_time2;
	int					d_msec;
	int					alpha, ptype;
	particle_t			*r;
	qboolean			cull_parts;
	int					part_info;
	float				maxdepth2, mindepth2, depth;
	int					numparticles;
	float				yaw, pitch, radius;

	assert(ce->p_root);

	numparticles = 0;
	cull_parts = (r_detail->value == DETAIL_LOW);
	maxdepth2 = r_farclipdist->value * r_farclipdist->value;  
	mindepth2 = r_nearclipdist->value * r_nearclipdist->value;

	for(prev = &ce->p_root, current = ce->p_root; current; current = current->next)
	{
		ptype = current->type & PFL_FLAG_MASK;

		d_msec = ParticleUpdateTime - current->startTime;
		d_time = d_msec * 0.001f;
		alpha = (int)current->color.a + Q_ftol(d_time * current->d_alpha);
		
		if (alpha > 255 && ((current->type & PFL_PULSE_ALPHA) || (ce->flags & CEF_PULSE_ALPHA)))
		{	// PULSE ALPHA means that once alpha is at max, reverse and head back down.
			alpha = 255 - (alpha - 255);	// A weird thing to do, but necessary because the alpha is 
											// based off a dtime from the CREATION of the particle
		}
		//add to additive particle list
		if((ce->flags & CEF_ADDITIVE_PARTS) || (current->type & PFL_ADDITIVE))
		{
			if (fxi.cls->r_anumparticles >= MAX_PARTICLES)
				return(numparticles);
			r = &fxi.cls->r_aparticles[fxi.cls->r_anumparticles];
			part_info = 1;
		}
		else
		{
			if (fxi.cls->r_numparticles >= MAX_PARTICLES)
				return(numparticles);

			r = &fxi.cls->r_particles[fxi.cls->r_numparticles];
			part_info = 2;
		}

		assert(ptype < NUM_PARTICLE_TYPES);

		r->type = current->type;
		r->color.c = current->color.c;

		if(alpha > 255)
		{
			r->color.a = 255;
		}
		else
		{
			r->color.a = alpha;
		}

		r->scale = current->scale + (d_time * current->d_scale);

		d_time2 = d_time * d_time * 0.5;

		if (ce->flags & CEF_ABSOLUTE_PARTS)
		{
			r->origin[0] = current->origin[0] + (current->velocity[0] * d_time) + (current->acceleration[0] * d_time2);
			r->origin[1] = current->origin[1] + (current->velocity[1] * d_time) + (current->acceleration[1] * d_time2);
			r->origin[2] = current->origin[2] + (current->velocity[2] * d_time) + (current->acceleration[2] * d_time2);
		}
		else
		{
			switch(current->type & PFL_MOVE_MASK)
			{
			case PFL_MOVE_SPHERE:
				yaw = current->origin[SPH_YAW] + (current->velocity[SPH_YAW] * d_time) + (current->acceleration[SPH_YAW] * d_time2);
				pitch = current->origin[SPH_PITCH] + (current->velocity[SPH_PITCH] * d_time) + (current->acceleration[SPH_YAW] * d_time2);
				radius = current->origin[SPH_RADIUS] + (current->velocity[SPH_RADIUS] * d_time) + (current->acceleration[SPH_RADIUS] * d_time2);
				r->origin[0] = ce->r.origin[0] + cos(yaw) * cos(pitch) * radius;
				r->origin[1] = ce->r.origin[1] + sin(yaw) * cos(pitch) * radius;
				r->origin[2] = ce->r.origin[2] + sin(pitch) * radius;
				break;
			case PFL_MOVE_CYL_X:
				yaw = current->origin[CYL_YAW] + (current->velocity[CYL_YAW] * d_time) + (current->acceleration[CYL_YAW] * d_time2);
				radius = current->origin[CYL_RADIUS] + (current->velocity[CYL_RADIUS] * d_time) + (current->acceleration[CYL_RADIUS] * d_time2);
				r->origin[0] = ce->r.origin[0] + current->origin[CYL_Z] + (current->velocity[CYL_Z] * d_time) + (current->acceleration[CYL_Z] * d_time2);
				r->origin[1] = ce->r.origin[1] + cos(yaw) * radius;
				r->origin[2] = ce->r.origin[2] + sin(yaw) * radius;
				break;
			case PFL_MOVE_CYL_Y:
				yaw = current->origin[CYL_YAW] + (current->velocity[CYL_YAW] * d_time) + (current->acceleration[CYL_YAW] * d_time2);
				radius = current->origin[CYL_RADIUS] + (current->velocity[CYL_RADIUS] * d_time) + (current->acceleration[CYL_RADIUS] * d_time2);
				r->origin[0] = ce->r.origin[0] + cos(yaw) * radius;
				r->origin[1] = ce->r.origin[1] + current->origin[CYL_Z] + (current->velocity[CYL_Z] * d_time) + (current->acceleration[CYL_Z] * d_time2);
				r->origin[2] = ce->r.origin[2] + sin(yaw) * radius;
				break;
			case PFL_MOVE_CYL_Z:
				yaw = current->origin[CYL_YAW] + (current->velocity[CYL_YAW] * d_time) + (current->acceleration[CYL_YAW] * d_time2);
				radius = current->origin[CYL_RADIUS] + (current->velocity[CYL_RADIUS] * d_time) + (current->acceleration[CYL_RADIUS] * d_time2);
				r->origin[0] = ce->r.origin[0] + cos(yaw) * radius;
				r->origin[1] = ce->r.origin[1] + sin(yaw) * radius;
				r->origin[2] = ce->r.origin[2] + current->origin[CYL_Z] + (current->velocity[CYL_Z] * d_time) + (current->acceleration[CYL_Z] * d_time2);
				break;
			case PFL_MOVE_NORM:
			default:
				r->origin[0] = ce->r.origin[0] + current->origin[0] + (current->velocity[0] * d_time) + (current->acceleration[0] * d_time2);
				r->origin[1] = ce->r.origin[1] + current->origin[1] + (current->velocity[1] * d_time) + (current->acceleration[1] * d_time2);
				r->origin[2] = ce->r.origin[2] + current->origin[2] + (current->velocity[2] * d_time) + (current->acceleration[2] * d_time2);
				break;
			}
		}

		if(cull_parts || (current->type & PFL_NEARCULL))
		{
			depth = VectorSeparationSquared(r->origin, fxi.cl->refdef.vieworg);

			if((depth > maxdepth2) || (depth < mindepth2))
			{
				part_info = 0;
			}
				
		}
		switch(part_info)
		{
		case 0:
			break;
		case 1:
			fxi.cls->r_anumparticles++;
			break;
		case 2:
			fxi.cls->r_numparticles++;
			break;
		default:
			assert(0);
			break;
		}
		numparticles++;
		prev = &(*prev)->next;
	}
	return(numparticles);
}

int UpdateParticles(client_entity_t *ce)
{
	client_particle_t	*current;
	client_particle_t	**prev;
	float				d_time;
	int					d_msec;
	int					alpha, ptype;
	int					numparticles;

	assert(ce->p_root);
	numparticles = 0;

	for(prev = &ce->p_root, current = ce->p_root; current; current = current->next)
	{
		ptype = current->type & PFL_FLAG_MASK;

		d_msec = ParticleUpdateTime - current->startTime;

		if(d_msec > current->duration)
		{
			*prev = current->next;

			ResMngr_DeallocateResource(&ParticleMngr, current, sizeof(*current));
			// current = current->next is still valid in the for loop.
			// a deallocated resource is guaranteed not to be changed until it is
			// reallocated, when the mananger is not shared between threads
			continue;
		}

		d_time = d_msec * 0.001f;

		alpha = (int)current->color.a + Q_ftol(d_time * current->d_alpha);
		
		if (alpha > 255 && ((ce->flags & CEF_PULSE_ALPHA) || (current->type & PFL_PULSE_ALPHA)))
		{	// PULSE ALPHA means that once alpha is at max, reverse and head back down.
			alpha = 255 - (alpha - 255);	// A weird thing to do, but necessary because the alpha is 
											// based off a dtime from the CREATION of the particle
		}
		
		if(alpha <= 0)
		{ 
			*prev = current->next;

			ResMngr_DeallocateResource(&ParticleMngr, current, sizeof(*current));
			// current = current->next is still valid in the for loop.
			// a deallocated resource is guaranteed not to be changed until it is
			// reallocated, when the mananger is not shared between threads
			continue;
		}

		prev = &(*prev)->next;
		numparticles++;
	}
	return(numparticles);
}

// This frees all particles attached to the client entity
void FreeParticles(client_entity_t *ce)
{
	client_particle_t	*current;
	client_particle_t	**prev;
	
	for(prev = &ce->p_root, current = ce->p_root; current; current = current->next)
	{
		*prev = current->next;
		ResMngr_DeallocateResource(&ParticleMngr, current, sizeof(*current));
	}
}

client_particle_t *ClientParticle_new(int type, paletteRGBA_t color, int duration)
{
	client_particle_t	*p;

	p = ResMngr_AllocateResource(&ParticleMngr, sizeof(client_particle_t));
	memset(p, 0, sizeof(client_particle_t));

	p->acceleration[2] = -PARTICLE_GRAVITY;

	p->startTime = ParticleUpdateTime;
	p->duration = duration;

	p->type = type;
	p->scale = 1.0F;
	p->color = color;

	p->d_alpha = -255.0 / (flrand(0.8F, 1.0F) * duration * (1.0F / 1000.0F));
	return(p);
}

// end