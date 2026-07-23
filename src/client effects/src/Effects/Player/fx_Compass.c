//
// fx_Compass.c
//
// Copyright 1998 Raven Software
//

#include "ce_Effects.h"
#include "ce_Utilities.h"
#include "Vector.h"

#define COMPASS_ANIM_STEPS	16
#define COMPASS_OFFSET		40.0f
static struct model_s* compass_model;

void PreCacheCompass(void)
{
	compass_model = fxi.RegisterModel("sprites/fx/compass.sp2");
}

static qboolean CompassAddToView(client_entity_t* self, centity_t* owner)
{
	const qboolean show_compass = (int)cl_compass->value;

	if (show_compass && self->SpawnInfo < COMPASS_ANIM_STEPS)
		self->SpawnInfo++;
	else if (!show_compass && self->SpawnInfo > 0)
		self->SpawnInfo--;

	if (self->SpawnInfo == COMPASS_ANIM_STEPS)
	{
		VectorAdd(owner->origin, self->startpos, self->r.origin);
		return true;
	}

	if (self->SpawnInfo > 0)
	{
		const float delta = (float)self->SpawnInfo / (float)(COMPASS_ANIM_STEPS - 1);
		const float scaler = sinf(ANGLE_90 * delta);

		self->r.scale = self->Scale * 0.15f + self->Scale * scaler * 0.85f;
		self->alpha = scaler;

		vec3_t offset;
		VectorScale(self->startpos, 0.75f, offset);
		VectorMA(offset, scaler * 0.25f, self->startpos, offset);
		offset[2] = -4.0f * (1.0f - scaler);
		VectorAdd(owner->origin, offset, self->r.origin);

		return true;
	}

	return false;
}

void FXCompass(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	static const vec3_t compass_offsets[] =
	{
		{ 0.0f, COMPASS_OFFSET, 0.0f },  // North.
		{ COMPASS_OFFSET, 0.0f, 0.0f },  // East.
		{ 0.0f, -COMPASS_OFFSET, 0.0f }, // South.
		{ -COMPASS_OFFSET, 0.0f, 0.0f }, // West.
	};

	assert(owner);

	for (int i = 0; i < 4; i++)
	{
		client_entity_t* dir = ClientEntity_new(type, flags, origin, NULL, 1000);

		dir->r.model = &compass_model;
		dir->r.frame = i;
		dir->r.flags = (RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_NODEPTHTEST);
		dir->r.scale = (i == 0 ? 0.35f : 0.25f); // Make North direction bigger.
		dir->radius = 16.0f;
		dir->Scale = dir->r.scale;
		VectorCopy(compass_offsets[i], dir->startpos);
		dir->Update = KeepSelfAI;
		dir->AddToView = CompassAddToView;

		AddEffect(owner, dir);
	}
}
