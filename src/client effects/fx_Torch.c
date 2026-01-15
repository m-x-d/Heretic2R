//
// fx_Torch.c -- mxd. Split from fx_tome.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Particle.h"
#include "Vector.h"
#include "ce_dlight.h"

#define TORCH_ORBIT_DIST		38.0f
#define TORCH_ORBIT_SCALE		0.0025f
#define TORCH_SPIN_FACTOR		0.004f

#define TIME_TO_FADE_TORCH		50
#define TORCH_INCOMING_ORBIT	(TORCH_ORBIT_DIST / TIME_TO_FADE_TORCH)
#define AMOUNT_TO_FADE_TORCH	(255 / TIME_TO_FADE_TORCH)

static struct model_s* torch_model;

void PreCacheTorch(void)
{
	torch_model = fxi.RegisterModel("sprites/lens/halo1.sp2");
}

// Update the position of the torch relative to its owner.
static qboolean PlayerTorchAddToView(client_entity_t* self, centity_t* owner) //mxd. Named 'FXROTTorchAddToView' in original logic.
{
	const float time = (float)fx_time; //mxd

	VectorSet(self->r.origin,
		cosf(time * TORCH_ORBIT_SCALE) * TORCH_ORBIT_DIST,
		sinf(time * TORCH_ORBIT_SCALE) * TORCH_ORBIT_DIST,
		25.0f + sinf(time * 0.0015f) * 16.0f);

	Vec3AddAssign(owner->origin, self->r.origin);
	VectorCopy(self->r.origin, self->origin);

	// Set up the last think time.
	const float diff_time = time - self->SpawnData;
	self->SpawnData = time;

	// Rotate the torch.
	self->r.angles[YAW] += diff_time * TORCH_SPIN_FACTOR;

	return true;
}

// Update the position of the torch relative to its owner.
static qboolean PlayerTorchFadeInAddToView(client_entity_t* self, centity_t* owner)
{
	const float tome_orbit = (float)self->SpawnInfo * TORCH_INCOMING_ORBIT;
	const float time = (float)fx_time; //mxd

	VectorSet(self->r.origin,
		cosf(time * TORCH_ORBIT_SCALE) * tome_orbit,
		sinf(time * TORCH_ORBIT_SCALE) * tome_orbit,
		(25.0f + sinf(time * 0.0015f) * 16.0f) * (float)self->SpawnInfo / TIME_TO_FADE_TORCH);

	Vec3AddAssign(owner->origin, self->r.origin);
	VectorCopy(self->r.origin, self->origin);

	// Set up the last think time.
	const float diff_time = time - self->SpawnData;
	self->SpawnData = time;

	// Rotate the torch.
	self->r.angles[YAW] += diff_time * TORCH_SPIN_FACTOR;

	return true;
}

// Make the light follow us.
static qboolean PlayerTorchUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXplayertorch_think' in original logic.
{
	// Kill us if we are done.
	if (owner->current.effects & EF_LIGHT_ENABLED)
		return true;

	if (self->SpawnInfo == 0)
	{
		self->SpawnInfo = TIME_TO_FADE_TORCH;
		self->d_alpha = -0.18f;
		self->AddToView = PlayerTorchFadeInAddToView;

		return true;
	}

	if (--self->SpawnInfo == 0)
		return false;

	// Decrement the amount of light the torch gives out.
	if (self->SpawnInfo < TIME_TO_FADE_TORCH)
		self->dlight->intensity -= AMOUNT_TO_FADE_TORCH;

	return true;
}

// Light that the player gives off when he has this powerup.
void FXPlayerTorch(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	client_entity_t* torch = ClientEntity_new(type, flags, origin, NULL, 100);

	torch->r.model = &torch_model;
	torch->r.flags = (RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
	torch->r.scale = 0.35f;
	torch->color = color_white; //mxd. 0xffffff in original version.
	torch->SpawnData = (float)fx_time;
	torch->alpha = 0.7f;
	torch->dlight = CE_DLight_new(torch->color, 250.0f, 0.0f);
	torch->AddToView = PlayerTorchAddToView;
	torch->Update = PlayerTorchUpdate;

	AddEffect(owner, torch);
}