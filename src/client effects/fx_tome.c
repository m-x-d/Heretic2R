//
// fx_tome.c
//
// Heretic II
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Client Entities.h"
#include "Particle.h"
#include "ResourceManager.h"
#include "FX.h"
#include "Vector.h"
#include "Random.h"
#include "Reference.h"
#include "Utilities.h"
#include "ce_dlight.h"

#define TOME_RADIUS 5.0
#define TOME_SCALE	10.0
#define TOME_ACCEL	(-64.0)
#define TOME_ORBIT_DIST	20.0
#define TORCH_ORBIT_DIST	38.0
#define TOME_ORBIT_SCALE	0.0025
#define TORCH_ORBIT_SCALE	0.0025
#define TOME_SPIN_FACTOR	0.004

#define DURATION_OF_TORCH	450
#define TIME_TO_FADE_TORCH	50
#define TIME_TO_FADE_TOME	30
#define TOME_INCOMING_ORBIT TOME_ORBIT_DIST/TIME_TO_FADE_TOME
#define TORCH_INCOMING_ORBIT TORCH_ORBIT_DIST/TIME_TO_FADE_TORCH
#define AMOUNT_TO_FADE_TORCH 255/TIME_TO_FADE_TORCH

#define NUM_TORCH_MODELS	2
static struct model_s *torch_models[NUM_TORCH_MODELS];
void PreCacheTorch()
{
	torch_models[0] = fxi.RegisterModel("sprites/lens/halo1.sp2");
	torch_models[1] = fxi.RegisterModel("models/Spells/book/tris.fm");
}

// update the position of the Tome of power relative to its owner
qboolean FXROTTomeAddToView(client_entity_t *tome, centity_t *owner)
{
	float difftime;

	VectorSet(tome->r.origin, 
				cos(fxi.cl->time*TOME_ORBIT_SCALE)*TOME_ORBIT_DIST, 
				sin(fxi.cl->time*TOME_ORBIT_SCALE)*TOME_ORBIT_DIST, 
				(15.0 + sin(fxi.cl->time*0.0015)*12.0));
	VectorAdd(owner->origin, tome->r.origin, tome->r.origin);
	VectorCopy(tome->r.origin, tome->origin);

	// Set up the last think time.
	difftime = fxi.cl->time - tome->SpawnData;
	tome->SpawnData = fxi.cl->time;

	// Rotate the book
	tome->r.angles[YAW] += difftime*TOME_SPIN_FACTOR;

	return(true);
}

// update the position of the Tome of power relative to its owner
qboolean FXHomeTomeAddToView(client_entity_t *tome, centity_t *owner)
{
	float	tome_orbit;
	float difftime;

	tome_orbit = tome->SpawnInfo * TOME_INCOMING_ORBIT;

	VectorSet(tome->r.origin, 
				cos(fxi.cl->time*TOME_ORBIT_SCALE)*tome_orbit, 
				sin(fxi.cl->time*TOME_ORBIT_SCALE)*tome_orbit, 
				(15.0 + sin(fxi.cl->time*0.0015)*12.0)*tome->SpawnInfo/TIME_TO_FADE_TOME);
	VectorAdd(owner->origin, tome->r.origin, tome->r.origin);
	VectorCopy(tome->r.origin, tome->origin);

	// Set up the last think time.
	difftime = fxi.cl->time - tome->SpawnData;
	tome->SpawnData = fxi.cl->time;

	// Rotate the book
	tome->r.angles[YAW] += difftime*TOME_SPIN_FACTOR;

	return(true);
}

// update that Tome of power, so that more sparkles zip out of it, and the light casts pulses
qboolean FXTomeThink(client_entity_t *tome, centity_t *owner)
{
	client_particle_t	*spark;
	int					i;

	// are we waiting for the shrine light to vanish ?
	if (tome->SpawnInfo)
	{
		if(!(--tome->SpawnInfo))
			return(false);
	}
	// no, could either be no light, or light still active
	else
	{
		tome->dlight->intensity = 150.0 + (cos(fxi.cl->time * 0.01) * 20.0);
		if (!(owner->current.effects & EF_POWERUP_ENABLED))
		{
		 	tome->AddToView = FXHomeTomeAddToView;
			tome->SpawnInfo = TIME_TO_FADE_TOME;
			tome->d_alpha = -0.18;
		}
	}

// Brian P wanted this removed. I thought it was cool though. Jake
	return(true);
	for(i = 0; i < 4; i++)
	{
		spark = ClientParticle_new(PART_16x16_STAR, tome->color, 2000);

		VectorSet(spark->origin, flrand(-TOME_RADIUS, TOME_RADIUS), flrand(-TOME_RADIUS, TOME_RADIUS), flrand(-TOME_RADIUS, TOME_RADIUS));
		VectorAdd(tome->origin, spark->origin, spark->origin);
		spark->scale = TOME_SCALE;
		VectorSet(spark->velocity, flrand(-20.0, 20.0), flrand(-20.0, 20.0), flrand(-10.0, 10.0));
		spark->acceleration[2] = TOME_ACCEL;
		spark->d_scale = flrand(-20.0, -15.0);
		spark->d_alpha = flrand(-500.0, -400.0);
		spark->duration = 1000;

		AddParticleToList(tome, spark);
	}
	return(true);
}

// original version of the tome of power. Casts a blue light etc
void FXTomeOfPower(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*tome;

	tome = ClientEntity_new(type, flags, origin, NULL, 100);

	// Test model
 	tome->r.model = torch_models + 1;
	tome->r.flags |= RF_FULLBRIGHT|RF_TRANSLUCENT|RF_TRANS_ADD|RF_TRANS_ADD_ALPHA;
	tome->flags |= CEF_ADDITIVE_PARTS | CEF_ABSOLUTE_PARTS;
	tome->r.scale = 0.55;
	tome->color.c = 0xe5ff2020;
	tome->radius = 128;
	tome->Update = FXTomeThink;
	tome->AddToView = FXROTTomeAddToView;
	tome->dlight = CE_DLight_new(tome->color, 150.0F, 00.0F);
	tome->SpawnData = fxi.cl->time;
	tome->SpawnInfo = 0;

	AddEffect(owner, tome);
}


/////////// TORCH STUFF

// update the position of the Tome of power relative to its owner
qboolean FXROTTorchAddToView(client_entity_t *tome, centity_t *owner)
{
	float difftime;

	VectorSet(tome->r.origin, 
				cos(fxi.cl->time*TORCH_ORBIT_SCALE)*TORCH_ORBIT_DIST, 
				sin(fxi.cl->time*TORCH_ORBIT_SCALE)*TORCH_ORBIT_DIST, 
				(25.0 + sin(fxi.cl->time*0.0015)*16.0));
	VectorAdd(owner->origin, tome->r.origin, tome->r.origin);
	VectorCopy(tome->r.origin, tome->origin);

	// Set up the last think time.
	difftime = fxi.cl->time - tome->SpawnData;
	tome->SpawnData = fxi.cl->time;

	// Rotate the book
	tome->r.angles[YAW] += difftime*TOME_SPIN_FACTOR;

	return(true);
}


// update the position of the Tome of power relative to its owner
qboolean FXHomeTorchAddToView(client_entity_t *tome, centity_t *owner)
{
	float	tome_orbit;
	float difftime;

	tome_orbit = tome->SpawnInfo * TORCH_INCOMING_ORBIT;

	VectorSet(tome->r.origin, 
				cos(fxi.cl->time*TORCH_ORBIT_SCALE)*tome_orbit, 
				sin(fxi.cl->time*TORCH_ORBIT_SCALE)*tome_orbit, 
				(25.0 + sin(fxi.cl->time*0.0015)*16.0)*tome->SpawnInfo/TIME_TO_FADE_TORCH);
	VectorAdd(owner->origin, tome->r.origin, tome->r.origin);
	VectorCopy(tome->r.origin, tome->origin);

	// Set up the last think time.
	difftime = fxi.cl->time - tome->SpawnData;
	tome->SpawnData = fxi.cl->time;

	// Rotate the book
	tome->r.angles[YAW] += difftime*TOME_SPIN_FACTOR;

	return(true);
}


// make the light follow us
static FXplayertorch_think(struct client_entity_s *self,centity_t *owner)
{
	// kill us if we are done
	if (owner->current.effects & EF_LIGHT_ENABLED)
		return(true);
	else
	if (!(self->SpawnInfo))
	{
	 	self->AddToView = FXHomeTorchAddToView;
		self->SpawnInfo = TIME_TO_FADE_TORCH;
		self->d_alpha = -0.18;
		return(true);
	}

	if (!(--self->SpawnInfo))
		return(false);

	// decrement the amount of light the torch gives out
	if (self->SpawnInfo < TIME_TO_FADE_TORCH)
		self->dlight->intensity -=	AMOUNT_TO_FADE_TORCH;

	return (true);
}

// light that the player gives off when he has this powerup
void FXplayertorch(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*effect;

	effect = ClientEntity_new(type, flags, origin, NULL, 100);
 	effect->r.model = torch_models;
	effect->r.flags |= RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	effect->r.scale = .35;
	effect->color.c = 0xffffff;
	effect->dlight = CE_DLight_new(effect->color, 250.0F, 0.0F);
	effect->Update = FXplayertorch_think;
	effect->AddToView = FXROTTorchAddToView;
	effect->SpawnData = fxi.cl->time;
	effect->alpha = 0.7;
  	AddEffect(owner, effect);
	effect->d_alpha = 0;

}


// end
