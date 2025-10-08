//
// fx_bubbler.c
//
// Copyright 1998 Raven Software
//

#include "Ambient effects.h"
#include "Client Effects.h"
#include "Motion.h"
#include "Particle.h"
#include "Vector.h"
#include "Random.h"
#include "Utilities.h"

#define BUBBLE_RADIUS			2.0f
#define BUBBLE_NUM_SPLASHES		8
#define BUBBLE_ACCELERATION		100.0f

static struct model_s* bubble_model;
static struct sfx_s* bubble_sounds[3]; //mxd

void PreCacheBubbler(void)
{
	bubble_model = fxi.RegisterModel("sprites/fx/bubble.sp2");
}

void PreCacheBubblerSFX(void) //mxd
{
	bubble_sounds[0] = fxi.S_RegisterSound("ambient/waterdrop1.wav");
	bubble_sounds[1] = fxi.S_RegisterSound("ambient/waterdrop2.wav");
	bubble_sounds[2] = fxi.S_RegisterSound("ambient/waterdrop3.wav");
}

static qboolean BubbleThink(client_entity_t* bubble, centity_t* owner)
{
	bubble->d_scale = -2.0f;
	bubble->d_alpha = -8.0f;
	bubble->velocity[2] = 0.0f;
	bubble->acceleration[2] = 0.0f;
	bubble->r.origin[2] += 1.0f;

	// Delay the death of this entity by 500 ms.
	bubble->nextThinkTime = fxi.cl->time + 500;
	bubble->Update = RemoveSelfAI;

	DoWaterSplash(bubble, color_white, BUBBLE_NUM_SPLASHES);
	FXWaterRipples(NULL, FX_WATER_RIPPLES, 0, bubble->r.origin);

	fxi.S_StartSound(bubble->r.origin, -1, CHAN_AUTO, bubble_sounds[irand(0, 2)], 1.0f, ATTN_STATIC, 0.0f);

	return true;
}

static qboolean BubblerParticleSpawner(client_entity_t* spawner, centity_t* owner)
{
	vec3_t origin;
	VectorCopy(spawner->r.origin, origin);

	origin[0] += flrand(0.0f, 5.0f);
	origin[1] += flrand(0.0f, 5.0f);
	origin[2] += flrand(0.0f, 5.0f);

	spawner->updateTime = irand(spawner->SpawnDelay / 2, spawner->SpawnDelay * 2);

	client_entity_t* bubble = ClientEntity_new(-1, 0, origin, NULL, (int)spawner->SpawnData);

	bubble->radius = flrand(0.5f, 1.5f);
	bubble->r.model = &bubble_model;
	bubble->r.scale = flrand(0.1f, 0.2f);
	bubble->r.flags = RF_TRANSLUCENT;
	VectorCopy(spawner->acceleration, bubble->acceleration);
	bubble->Update = BubbleThink;

	AddEffect(NULL, bubble);

	return true;
}

void FXBubbler(centity_t* owner, const int type, int flags, vec3_t origin)
{
	const float up = GetSolidDist(origin, 1.0f, 1000.0f);

	vec3_t dest;
	VectorCopy(origin, dest);
	dest[2] += up;

	const float down = GetSolidDist(dest, 1.0f, -1000.0f);
	const float time = GetTimeToReachDistance(0.0f, 100.0f, fabsf(up + down));

	char bubbles_per_min;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_BUBBLER].formatString, &bubbles_per_min);

	flags |= (CEF_NO_DRAW | CEF_NOMOVE | CEF_CULLED | CEF_VIEWSTATUSCHANGED | CEF_CHECK_OWNER);
	client_entity_t* self = ClientEntity_new(type, flags, origin, NULL, 1000);

	self->radius = BUBBLE_RADIUS;
	self->SpawnDelay = 60 * 1000 / bubbles_per_min;
	self->acceleration[2] = BUBBLE_ACCELERATION;
	self->SpawnData = time;
	self->Update = BubblerParticleSpawner;

	AddEffect(owner, self);
}

void FXBubble(centity_t* owner, int type, const int flags, vec3_t origin)
{
	const float up = GetSolidDist(origin, 1.0f, 1000.0f);

	vec3_t dest;
	VectorCopy(origin, dest);
	dest[2] += up;

	const float down = GetSolidDist(dest, 1.0f, -1000.0f);
	const int time = (int)(GetTimeToReachDistance(0.0f, BUBBLE_ACCELERATION, fabsf(up + down)));

	client_entity_t* bubble = ClientEntity_new(FX_BUBBLE, flags, origin, NULL, time);

	bubble->radius = BUBBLE_RADIUS * 2;
	bubble->r.model = &bubble_model;
	bubble->r.scale = flrand(0.025f, 0.1f);
	bubble->r.flags = RF_TRANSLUCENT;
	bubble->acceleration[2] = BUBBLE_ACCELERATION;
	bubble->Update = BubbleThink;

	AddEffect(NULL, bubble);
}

void MakeBubble(vec3_t loc, client_entity_t* spawner)
{
	client_particle_t* bubble = ClientParticle_new(PART_32x32_BUBBLE, color_white, 1000);

	VectorCopy(loc, bubble->origin);
	bubble->d_alpha = 0;
	bubble->scale = flrand(0.5f, 1.0f);
	bubble->d_scale = -bubble->scale;
	bubble->velocity[0] = flrand(-10.0f, 10.0f);
	bubble->velocity[1] = flrand(-10.0f, 10.0f);
	bubble->acceleration[2] = BUBBLE_ACCELERATION;

	AddParticleToList(spawner, bubble);
}

static qboolean CreateBubble(client_entity_t* self, centity_t* owner)
{
	vec3_t loc;

	// Give us a random location.
	VectorRandomSet(loc, 15.0f); //mxd
	VectorAdd(loc, self->r.origin, loc);

	// Create a bubble.
	MakeBubble(loc, self);

	// Random time till next bubble.
	self->updateTime = irand(50, 500);

	// Never kill this.
	return true;
}

// Create a constant client effect attached to something in water that releases bubbles.
void FXRandWaterBubble(centity_t* owner, const int type, int flags, vec3_t origin)
{
	flags |= CEF_NO_DRAW | CEF_ABSOLUTE_PARTS | CEF_CHECK_OWNER;
	client_entity_t* self = ClientEntity_new(type, flags, origin, NULL, irand(50, 500));

	self->radius = 20.0f;
	self->AddToView = LinkedEntityUpdatePlacement;
	self->Update = CreateBubble;

	AddEffect(owner, self);
}