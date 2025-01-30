//
// fx_tome.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Particle.h"
#include "Random.h"
#include "Vector.h"
#include "ce_dlight.h"

#define TOME_RADIUS				5.0f
#define TOME_SCALE				10.0f
#define TOME_ACCEL				(-64.0f)
#define TOME_ORBIT_DIST			20.0f
#define TORCH_ORBIT_DIST		38.0f
#define TOME_ORBIT_SCALE		0.0025f
#define TORCH_ORBIT_SCALE		0.0025f
#define TOME_SPIN_FACTOR		0.004f

#define TIME_TO_FADE_TORCH		50
#define TIME_TO_FADE_TOME		30
#define TOME_INCOMING_ORBIT		(TOME_ORBIT_DIST / TIME_TO_FADE_TOME)
#define TORCH_INCOMING_ORBIT	(TORCH_ORBIT_DIST / TIME_TO_FADE_TORCH)
#define AMOUNT_TO_FADE_TORCH	(255 / TIME_TO_FADE_TORCH)

static struct model_s* tome_model;
static struct model_s* torch_model;

void PreCacheTorch(void)
{
	tome_model = fxi.RegisterModel("models/Spells/book/tris.fm");
	torch_model = fxi.RegisterModel("sprites/lens/halo1.sp2");
}

// Update the position of the Tome of power relative to its owner.
static qboolean TomeOfPowerAddToView(client_entity_t* tome, centity_t* owner)
{
	const float time = (float)fxi.cl->time; //mxd

	VectorSet(tome->r.origin,
		cosf(time * TOME_ORBIT_SCALE) * TOME_ORBIT_DIST,
		sinf(time * TOME_ORBIT_SCALE) * TOME_ORBIT_DIST,
		15.0f + sinf(time * 0.0015f) * 12.0f);

	VectorAdd(owner->origin, tome->r.origin, tome->r.origin);
	VectorCopy(tome->r.origin, tome->origin);

	// Setup the last think time.
	const float diff_time = time - tome->SpawnData;
	tome->SpawnData = time;

	// Rotate the book.
	tome->r.angles[YAW] += diff_time * TOME_SPIN_FACTOR;

	return true;
}

// Update the position of the Tome of power relative to its owner.
static qboolean TomeOfPowerFadeInAddToView(client_entity_t* tome, centity_t* owner)
{
	const float tome_orbit = (float)tome->SpawnInfo * TOME_INCOMING_ORBIT;
	const float time = (float)fxi.cl->time; //mxd

	VectorSet(tome->r.origin,
		cosf(time * TOME_ORBIT_SCALE) * tome_orbit,
		sinf(time * TOME_ORBIT_SCALE) * tome_orbit,
		(15.0f + sinf(time * 0.0015f) * 12.0f) * (float)tome->SpawnInfo / TIME_TO_FADE_TOME);

	VectorAdd(owner->origin, tome->r.origin, tome->r.origin);
	VectorCopy(tome->r.origin, tome->origin);

	// Setup the last think time.
	const float diff_time = time - tome->SpawnData;
	tome->SpawnData = time;

	// Rotate the book
	tome->r.angles[YAW] += diff_time * TOME_SPIN_FACTOR;

	return true;
}

// Update the Tome of power, so that more sparkles zip out of it, and the light casts pulses.
static qboolean TomeOfPowerThink(client_entity_t* tome, centity_t* owner)
{
	// Are we waiting for the shrine light to vanish?
	if (tome->SpawnInfo > 0)
	{
		if (--tome->SpawnInfo == 0)
			return false;
	}
	else // No, could either be no light, or light still active.
	{
		tome->dlight->intensity = 150.0f + cosf((float)fxi.cl->time * 0.01f) * 20.0f;

		if (!(owner->current.effects & EF_POWERUP_ENABLED))
		{
			tome->AddToView = TomeOfPowerFadeInAddToView;
			tome->SpawnInfo = TIME_TO_FADE_TOME;
			tome->d_alpha = -0.18f;
		}
	}

	//mxd. Disabled in original version. //TODO: make scale smaller and randomized, randomize color a bit? randomize particles count? Use several particle types?
	for (int i = 0; i < 4; i++)
	{
		client_particle_t* spark = ClientParticle_new(PART_16x16_STAR, tome->color, 2000);

		VectorRandomSet(spark->origin, TOME_RADIUS);
		VectorAdd(tome->origin, spark->origin, spark->origin);
		spark->scale = TOME_SCALE;
		VectorSet(spark->velocity, flrand(-20.0f, 20.0f), flrand(-20.0f, 20.0f), flrand(-10.0f, 10.0f));
		spark->acceleration[2] = TOME_ACCEL;
		spark->d_scale = flrand(-20.0f, -15.0f);
		spark->d_alpha = flrand(-500.0f, -400.0f);
		spark->duration = 1000;

		AddParticleToList(tome, spark);
	}

	return true;
}

// Original version of the tome of power. Casts a blue light etc.
void FXTomeOfPower(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	client_entity_t* tome = ClientEntity_new(type, flags, origin, NULL, 100);

	tome->radius = 128.0f;
	tome->r.model = &tome_model;
	tome->r.flags = RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	tome->flags |= CEF_ADDITIVE_PARTS | CEF_ABSOLUTE_PARTS;
	tome->r.scale = 0.55f;
	tome->color.c = 0xe5ff2020;
	tome->SpawnData = (float)fxi.cl->time;
	tome->dlight = CE_DLight_new(tome->color, 150.0f, 0.0f);
	tome->AddToView = TomeOfPowerAddToView;
	tome->Update = TomeOfPowerThink;

	AddEffect(owner, tome);
}

// Update the position of the torch relative to its owner.
static qboolean PlayerTorchAddToView(client_entity_t* tome, centity_t* owner)
{
	const float time = (float)fxi.cl->time; //mxd

	VectorSet(tome->r.origin,
		cosf(time * TORCH_ORBIT_SCALE) * TORCH_ORBIT_DIST,
		sinf(time * TORCH_ORBIT_SCALE) * TORCH_ORBIT_DIST,
		25.0f + sinf(time * 0.0015f) * 16.0f);

	VectorAdd(owner->origin, tome->r.origin, tome->r.origin);
	VectorCopy(tome->r.origin, tome->origin);

	// Set up the last think time.
	const float diff_time = time - tome->SpawnData;
	tome->SpawnData = time;

	// Rotate the torch.
	tome->r.angles[YAW] += diff_time * TOME_SPIN_FACTOR;

	return true;
}

// Update the position of the torch relative to its owner.
static qboolean PlayerTorchFadeInAddToView(client_entity_t* tome, centity_t* owner)
{
	const float tome_orbit = (float)tome->SpawnInfo * TORCH_INCOMING_ORBIT;
	const float time = (float)fxi.cl->time; //mxd

	VectorSet(tome->r.origin,
		cosf(time * TORCH_ORBIT_SCALE) * tome_orbit,
		sinf(time * TORCH_ORBIT_SCALE) * tome_orbit,
		(25.0f + sinf(time * 0.0015f) * 16.0f) * (float)tome->SpawnInfo / TIME_TO_FADE_TORCH);

	VectorAdd(owner->origin, tome->r.origin, tome->r.origin);
	VectorCopy(tome->r.origin, tome->origin);

	// Set up the last think time.
	const float diff_time = time - tome->SpawnData;
	tome->SpawnData = time;

	// Rotate the torch.
	tome->r.angles[YAW] += diff_time * TOME_SPIN_FACTOR;

	return true;
}

// Make the light follow us.
static qboolean PlayerTorchThink(struct client_entity_s* self, centity_t* owner)
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
	client_entity_t* effect = ClientEntity_new(type, flags, origin, NULL, 100);

	effect->r.model = &torch_model;
	effect->r.flags = RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	effect->r.scale = 0.35f;
	effect->color = color_white; //mxd. 0xffffff in original version.
	effect->SpawnData = (float)fxi.cl->time;
	effect->alpha = 0.7f;
	effect->dlight = CE_DLight_new(effect->color, 250.0f, 0.0f);
	effect->AddToView = PlayerTorchAddToView;
	effect->Update = PlayerTorchThink;

	AddEffect(owner, effect);
}