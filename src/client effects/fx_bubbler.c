//
// fx_bubbler.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "fx_WaterSplash.h"
#include "g_playstats.h"
#include "Motion.h"
#include "Particle.h"
#include "Vector.h"
#include "Random.h"
#include "Utilities.h"

#define BUBBLE_RADIUS			2.0f
#define BUBBLE_NUM_SPLASHES		8
#define BUBBLE_ACCELERATION		100.0f
#define BUBBLE_CHECK_DISTANCE	1000.0f //mxd

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

static qboolean BubbleExpireUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXBubbleThink' in original logic.
{
	self->d_scale = -1.0f;
	self->d_alpha = -8.0f;
	self->velocity[2] = 0.0f;
	self->acceleration[2] = 0.0f;
	self->Update = RemoveSelfAI;

	//mxd. Don't spawn splash/ripple effects when expired underwater.
	if (self->SpawnInfo)
	{
		// Delay the death of this entity by 500 ms. (to allow splash sound to play --mxd).
		self->nextThinkTime = fx_time + 500;
		
		DoWaterSplash(self, color_white, BUBBLE_NUM_SPLASHES, false);
		FXWaterRipples(NULL, FX_WATER_RIPPLES, 0, self->r.origin);

		fxi.S_StartSound(self->r.origin, -1, CHAN_AUTO, bubble_sounds[irand(0, 2)], 1.0f, ATTN_STATIC, 0.0f);
	}
	else
	{
		self->nextThinkTime = fx_time;
	}

	return true;
}

static qboolean BubblerUpdate(client_entity_t* spawner, centity_t* owner) //mxd. Named 'FXBubblerParticleSpawner' in original logic.
{
	spawner->updateTime = irand(spawner->SpawnDelay / 2, spawner->SpawnDelay * 2);

	const vec3_t origin = VEC3_INITA(spawner->r.origin, flrand(-5.0f, 5.0f), flrand(-5.0f, 5.0f), 0.0f); //mxd. XYZ: flrand(0.0f, 5.0f) in original logic.
	client_entity_t* bubble = ClientEntity_new(-1, 0, origin, NULL, (int)spawner->SpawnData);

	const int r_flags = (R_DETAIL > DETAIL_HIGH ? RF_LM_COLOR : 0); //mxd
	bubble->radius = flrand(0.5f, 1.5f);
	bubble->r.model = &bubble_model;
	bubble->r.scale = flrand(0.1f, 0.2f);
	bubble->r.flags = (RF_TRANSLUCENT | r_flags);
	bubble->alpha = 0.01f; //mxd. Added subtle fade-in effect.
	bubble->d_alpha = 1.5f; //mxd
	bubble->SpawnInfo = spawner->SpawnInfo; //mxd
	VectorCopy(spawner->acceleration, bubble->acceleration);
	bubble->Update = BubbleExpireUpdate;

	AddEffect(NULL, bubble);

	return true;
}

void FXBubbler(centity_t* owner, const int type, int flags, vec3_t origin)
{
	const float up = GetSolidDist(origin, BUBBLE_RADIUS * 0.5f, BUBBLE_CHECK_DISTANCE, false);

	const vec3_t dest = VEC3_INITA(origin, 0.0f, 0.0f, up);
	const float down = GetSolidDist(dest, BUBBLE_RADIUS * 0.5f, -BUBBLE_CHECK_DISTANCE, true);
	const float time = GetTimeToReachDistance(0.0f, BUBBLE_ACCELERATION, fabsf(up + down));

	byte bubbles_per_min; //mxd. char in original logic (but sent as byte).
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_BUBBLER].formatString, &bubbles_per_min);

	flags |= (CEF_NO_DRAW | CEF_NOMOVE | CEF_CULLED | CEF_VIEWSTATUSCHANGED | CEF_CHECK_OWNER);
	client_entity_t* self = ClientEntity_new(type, flags, origin, NULL, 1000);

	self->radius = BUBBLE_RADIUS;
	self->SpawnDelay = 60 * 1000 / bubbles_per_min;
	self->acceleration[2] = BUBBLE_ACCELERATION;
	self->SpawnData = time;
	self->SpawnInfo = (down < 0.0f); //mxd. down 0 means dest pos is underwater.
	self->Update = BubblerUpdate;

	AddEffect(owner, self);
}

void FXBubble(centity_t* owner, int type, const int flags, vec3_t origin)
{
	const float up = GetSolidDist(origin, 1.0f, BUBBLE_CHECK_DISTANCE, false);

	const vec3_t dest = VEC3_INITA(origin, 0.0f, 0.0f, up);
	const float down = GetSolidDist(dest, 1.0f, -BUBBLE_CHECK_DISTANCE, true);
	const int time = (int)(GetTimeToReachDistance(0.0f, BUBBLE_ACCELERATION, fabsf(up + down)));

	client_entity_t* bubble = ClientEntity_new(FX_BUBBLE, flags, origin, NULL, time);

	const int r_flags = (R_DETAIL > DETAIL_HIGH ? RF_LM_COLOR : 0); //mxd
	bubble->radius = BUBBLE_RADIUS * 2.0f;
	bubble->r.model = &bubble_model;
	bubble->r.scale = flrand(0.025f, 0.1f);
	bubble->r.flags = (RF_TRANSLUCENT | r_flags);
	bubble->alpha = 0.01f; //mxd. Added subtle fade-in effect.
	bubble->d_alpha = 1.5f; //mxd
	bubble->acceleration[2] = BUBBLE_ACCELERATION;
	bubble->SpawnInfo = (down < 0.0f); //mxd. down 0 means dest pos is underwater.
	bubble->Update = BubbleExpireUpdate;

	AddEffect(NULL, bubble);
}

void MakeBubble(const vec3_t origin, client_entity_t* spawner)
{
	const int extra_flags = (R_DETAIL > DETAIL_HIGH ? PFL_LM_COLOR : 0); //mxd
	client_particle_t* bubble = ClientParticle_new(PART_32x32_BUBBLE | extra_flags, color_white, 1000);

	VectorCopy(origin, bubble->origin);
	bubble->d_alpha = 0.0f;
	bubble->scale = flrand(0.5f, 1.0f);
	bubble->d_scale = -bubble->scale;
	bubble->velocity[0] = flrand(-10.0f, 10.0f);
	bubble->velocity[1] = flrand(-10.0f, 10.0f);
	bubble->acceleration[2] = BUBBLE_ACCELERATION;

	AddParticleToList(spawner, bubble);
}

static qboolean RandWaterBubbleUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'Create_Bubble' in original logic.
{
	vec3_t origin;

	// Give us a random location.
	VectorRandomSet(origin, 15.0f); //mxd
	Vec3AddAssign(self->r.origin, origin);

	// Create a bubble.
	MakeBubble(origin, self);

	// Random time till next bubble.
	self->updateTime = irand(50, 500);

	// Never kill this.
	return true;
}

// Create a constant client effect attached to something in water that releases bubbles.
void FXRandWaterBubble(centity_t* owner, const int type, int flags, vec3_t origin)
{
	flags |= (CEF_NO_DRAW | CEF_ABSOLUTE_PARTS | CEF_CHECK_OWNER);
	client_entity_t* bubble = ClientEntity_new(type, flags, origin, NULL, irand(50, 500));

	bubble->radius = 20.0f;
	bubble->AddToView = LinkedEntityUpdatePlacement;
	bubble->Update = RandWaterBubbleUpdate;

	AddEffect(owner, bubble);
}