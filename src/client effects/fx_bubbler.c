//
// Heretic II
// Copyright 1998 Raven Software
//
// Created by JJS

#include "Client Effects.h"
#include "Client Entities.h"
#include "ce_DefaultMessageHandler.h"
#include "Particle.h"
#include "ResourceManager.h"
#include "FX.h"
#include "Vector.h"
#include "random.h"
#include "Utilities.h"
#include "Ambient effects.h"
#include "motion.h"

#define	BUBBLE_MAX_DURATION	2.0F
#define BUBBLE_RADIUS			2.0F
#define BUBBLE_NUM_SPLASHES		8
#define BUBBLE_ACCELERATION		100

#define	NUM_BUBBLE_MODELS	1

static struct model_s *bubbler_models[NUM_BUBBLE_MODELS];

void PreCacheBubbler()
{
	bubbler_models[0] = fxi.RegisterModel("sprites/fx/bubble.sp2");
}

// -----------------------------------------------------------------------------------------

qboolean FXBubbleThink(client_entity_t *bubble, centity_t *owner)
{
	paletteRGBA_t		color;

	// Delay the death of this entity by 500 ms.
	bubble->updateTime = 500;
	bubble->Update = RemoveSelfAI;
	bubble->nextThinkTime = fxi.cl->time + 500;

	bubble->d_scale = -2.0;
	bubble->d_alpha = -8.0F;
	bubble->velocity[2] = bubble->acceleration[2] = 0;
	bubble->r.origin[2] += 1.0;
	
	color.c = 0xffffffff;
	DoWaterSplash(bubble, color, BUBBLE_NUM_SPLASHES);

	FXWaterRipples(NULL, FX_WATER_RIPPLES, 0, bubble->r.origin);

	fxi.S_StartSound(bubble->r.origin, -1, CHAN_AUTO,
			fxi.S_RegisterSound(va("ambient/waterdrop%c.wav", irand('1', '3'))), 1, ATTN_STATIC, 0);
	return(true);
}


static qboolean FXBubblerParticleSpawner(client_entity_t *spawner, centity_t *owner)
{
	client_entity_t		*bubble;
	vec3_t origin;

	VectorCopy(spawner->r.origin, origin);

	origin[0] += flrand(0.0, 5.0);
	origin[1] += flrand(0.0, 5.0);
	origin[2] += flrand(0.0, 5.0);

	spawner->updateTime = irand(spawner->SpawnDelay / 2, spawner->SpawnDelay * 2);

	bubble = ClientEntity_new(-1, 0, origin, NULL, spawner->SpawnData);

	bubble->r.model = bubbler_models;
	bubble->r.scale = flrand(0.10, 0.20);
	bubble->r.flags = RF_TRANSLUCENT;

	bubble->radius = flrand(0.5, 1.5);
	VectorCopy(spawner->acceleration, bubble->acceleration);

	bubble->Update = FXBubbleThink;

	AddEffect(NULL, bubble); 
	return(true);
}

void FXBubbler(centity_t *Owner, int Type, int Flags, vec3_t Origin)
{
	client_entity_t		*self;
	char				bubblespermin;

	vec3_t dest;
	float time;
	float dist, dust;

	GetSolidDist(Origin, 1.0, 1000, &dist);

	VectorCopy(Origin, dest);
	dest[2] += dist;

	GetSolidDist(dest, 1.0, -1000, &dust);

	dist += dust;

	time = GetTimeToReachDistance(0, 100, abs(dist));
				
	fxi.GetEffect(Owner, Flags, clientEffectSpawners[FX_BUBBLER].formatString, &bubblespermin );

	Flags |= CEF_NO_DRAW | CEF_NOMOVE | CEF_CULLED | CEF_VIEWSTATUSCHANGED |CEF_CHECK_OWNER;
	self = ClientEntity_new(Type, Flags, Origin, NULL, 1000);

	self->SpawnDelay = (60 * 1000) / bubblespermin;
	self->Update = FXBubblerParticleSpawner;
	
	self->acceleration[2] = BUBBLE_ACCELERATION;
	self->radius = BUBBLE_RADIUS;
	self->SpawnData = time;
	AddEffect(Owner, self); 
}

void FXBubble(centity_t *Owner, int Type, int Flags, vec3_t Origin)
{
	int					time;
	client_entity_t		*bubble;
	float up, down;
	vec3_t	dest;

	GetSolidDist(Origin, 1.0, 1000, &up);

	VectorCopy(Origin, dest);
	dest[2] += up;

	GetSolidDist(dest, 1.0, -1000, &down);
	up += down;

	time = GetTimeToReachDistance(0, BUBBLE_ACCELERATION, abs(up));
	bubble = ClientEntity_new(FX_BUBBLE, Flags, Origin, NULL, time);

	bubble->r.model = bubbler_models;
	bubble->r.scale = flrand(0.025, 0.10);
	bubble->r.flags = RF_TRANSLUCENT;
	bubble->radius = BUBBLE_RADIUS*2;
	bubble->acceleration[2] = BUBBLE_ACCELERATION;

	bubble->Update = FXBubbleThink;

	AddEffect(NULL, bubble); 
}


void MakeBubble(vec3_t loc, client_entity_t *spawner)
{
	client_particle_t	*bubble;
	paletteRGBA_t	color;

	color.c = 0xffffffff;
	bubble = ClientParticle_new(PART_32x32_BUBBLE, color, 1000);

	VectorCopy(loc, bubble->origin);
	bubble->d_alpha = 0;
	bubble->scale = flrand(0.5, 1.0);
	bubble->d_scale = -bubble->scale;
	bubble->velocity[0] = flrand(-10.0F, 10.0F);
	bubble->velocity[1] = flrand(-10.0F, 10.0F);
	bubble->acceleration[2] = 100;

	AddParticleToList(spawner, bubble);
}

qboolean Create_Bubble(client_entity_t *self, centity_t *owner)
{
	vec3_t	loc;

	// give us a random location
	VectorSet(loc, flrand(-15,15), flrand(-15,15), flrand(-15,15));
	VectorAdd(loc,self->r.origin,loc);
	// create a bubble
	MakeBubble(loc, self);
	// random time till next bubble
	self->updateTime = irand(50,500);
	// Never kill this
  	return(true);
}

// create a constant client effect attached to something in water that releases bubbles
void FXRandWaterBubble(centity_t *Owner, int Type, int Flags, vec3_t Origin)
{
	client_entity_t		*self;

	self = ClientEntity_new(Type, Flags | CEF_NO_DRAW | CEF_ABSOLUTE_PARTS | CEF_CHECK_OWNER , Origin, NULL, irand(50,500));
	self->Update = Create_Bubble;
	self->AddToView = LinkedEntityUpdatePlacement;
	self->radius = 20;
	AddEffect(Owner, self); 
}

// end



