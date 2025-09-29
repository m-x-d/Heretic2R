//
// fx_BlueRing.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Vector.h"
#include "Utilities.h"

static struct model_s* ring_model;
static struct sfx_s* ring_sound; //mxd

void PreCacheBluering(void)
{
	ring_model = fxi.RegisterModel("sprites/spells/bluering.sp2");
}

void PreCacheBlueringSFX(void) //mxd
{
	ring_sound = fxi.S_RegisterSound("weapons/Spell Blue Ring.wav");

	// Also precache weapon sound...
	fxi.S_RegisterSound("weapons/reflect.wav");
}

void FXBlueRing(centity_t* owner, const int type, const int flags, vec3_t origin)
{
#define NUM_FLAME_ITEMS		20
#define FLAME_ABSVEL		450
#define FLAME_INIT_SCALE	0.1f
#define FLAME_DELTA_SCALE	6.0f
#define FLAME_DURATION		500
#define FLAME_DELTA_ALPHA	(1000 / FLAME_DURATION)

	float count = (float)(GetScaledCount(NUM_FLAME_ITEMS, 0.95f));
	count = Clamp(count, 8.0f, 20.0f);

	float cur_angle = 0.0f;
	while (cur_angle < ANGLE_360)
	{
		client_entity_t* flame_fx = ClientEntity_new(type, flags & ~CEF_OWNERS_ORIGIN, origin, NULL, FLAME_DURATION);

		flame_fx->r.model = &ring_model;
		flame_fx->r.flags = RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		flame_fx->r.frame = 0;
		flame_fx->r.scale = FLAME_INIT_SCALE;
		flame_fx->d_scale = FLAME_DELTA_SCALE;

		VectorSet(flame_fx->velocity, FLAME_ABSVEL * cosf(cur_angle), FLAME_ABSVEL * sinf(cur_angle), 0);
		flame_fx->radius = 20.0f;
		flame_fx->d_alpha = -FLAME_DELTA_ALPHA;

		AddEffect(NULL, flame_fx);

		cur_angle += ANGLE_360 / count;
	}

	fxi.S_StartSound(origin, -1, CHAN_AUTO, ring_sound, 1.0f, ATTN_NORM, 0.0f);
}