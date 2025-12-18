//
// fx_HitPuffs.c -- originally part of Generic Character Effects.c.
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Vector.h"
#include "Random.h"

static struct model_s* puff_model;
static struct model_s* halo_model;
static struct model_s* rock_models[4];

void PreCacheOgleHitPuff(void) //mxd. Named 'PrecacheOgleHitPuff' in original logic.
{
	puff_model = fxi.RegisterModel("sprites/fx/steam_add.sp2");

	rock_models[0] = fxi.RegisterModel("models/debris/stone/schunk1/tris.fm");
	rock_models[1] = fxi.RegisterModel("models/debris/stone/schunk2/tris.fm");
	rock_models[2] = fxi.RegisterModel("models/debris/stone/schunk3/tris.fm");
	rock_models[3] = fxi.RegisterModel("models/debris/stone/schunk4/tris.fm");
}

void PreCacheHitPuff(void) //mxd
{
	puff_model = fxi.RegisterModel("sprites/fx/steam_add.sp2");
	halo_model = fxi.RegisterModel("sprites/fx/halo.sp2");
}

static qboolean PebbleUpdate(struct client_entity_s* self, centity_t* owner)
{
	const int cur_time = fx_time;
	const float d_time = (float)(cur_time - self->lastThinkTime) / 1000.0f;

	self->acceleration[2] -= 75.0f;
	self->r.angles[0] += ANGLE_360 * d_time;
	self->r.angles[1] += ANGLE_360 * d_time;

	self->lastThinkTime = cur_time;

	return cur_time <= self->LifeTime;
}

// Slight variation on the normal puff.
void FXOgleHitPuff(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	vec3_t dir;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_OGLE_HITPUFF].formatString, dir); // Normalized direction vector.

	const float speed = VectorNormalize(dir);
	const int count = (speed > 1.0f ? irand(10, 15) : irand(1, 4));

	for (int i = 0; i < count; i++)
	{
		// Puff!
		client_entity_t* puff = ClientEntity_new(type, flags, origin, NULL, 500);

		puff->r.model = &puff_model;
		puff->r.flags |= (RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);

		vec3_t work;
		VectorRandomCopy(dir, work, 0.5f);

		if (speed > 1.0f)
			VectorScale(work, speed, puff->velocity);
		else if (flags & CEF_FLAG6)
			VectorScale(work, flrand(8.0f, 12.0f), puff->velocity);
		else
			VectorScale(work, 4.0f, puff->velocity);

		puff->acceleration[2] = flrand(10.0f, 50.0f);
		puff->alpha = 0.35f;
		puff->r.scale = (speed > 1.0f ? flrand(0.3f, 0.75f) : 0.1f);
		puff->d_scale = 2.0f;
		puff->d_alpha = -2.0f;
		puff->color = color_white; //mxd

		AddEffect(NULL, puff); // Add the effect as independent world effect.
	}

	for (int i = 0; i < count; i++)
	{
		// Rock!
		client_entity_t* rock = ClientEntity_new(type, flags, origin, NULL, 50);

		rock->r.model = &rock_models[irand(0, 3)]; //TODO: each rock model has 2 skins. Randomly pick 2-nd one?

		vec3_t work;
		VectorRandomCopy(dir, work, 0.5f);
		VectorScale(work, (speed > 1.0f ? speed : flrand(8.0f, 16.0f)), rock->velocity);

		if (flags & CEF_FLAG6 || speed > 1.0f)
			VectorSet(rock->acceleration, flrand(-75.0f, 75.0f), flrand(-75.0f, 75.0f), flrand(125.0f, 250.0f));
		else
			rock->acceleration[2] = flrand(-50.0f, 50.0f);

		rock->Update = PebbleUpdate;

		if (speed > 1.0f)
			rock->r.scale = flrand(0.8f, 1.5f) * speed / 100;
		else
			rock->r.scale = flrand(0.1f, 0.25f);

		rock->d_scale = 0.0f;
		rock->d_alpha = 0.0f;
		rock->color = color_white; //mxd
		rock->LifeTime = fx_time + 5000;

		AddEffect(NULL, rock); // Add the effect as independent world effect.
	}
}

void FXHitPuff(centity_t* owner, const int type, const int flags, vec3_t origin) //mxd. Named 'FXGenericHitPuff' in original logic.
{
	byte count;
	vec3_t dir;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_HITPUFF].formatString, dir, &count); // Normalized direction vector.
	count = min(40, count);

	for (int i = 0; i < count; i++)
	{
		client_entity_t* fx = ClientEntity_new(type, flags, origin, NULL, 500);

		fx->r.model = &puff_model;
		fx->r.flags |= (RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);

		vec3_t work;
		VectorRandomCopy(dir, work, 0.5f);
		VectorScale(work, 50.0f, fx->velocity);
		fx->acceleration[2] = -100.0f;
		fx->alpha = 0.5f;
		fx->r.scale = 0.1f;
		fx->d_scale = 1.0f;
		fx->d_alpha = -1.0f;
		fx->color = color_white; //mxd

		AddEffect(NULL, fx); // Add the effect as independent world effect.
	}

	if (flags & CEF_FLAG6)
	{
		// High-intensity impact point.
		client_entity_t* impact = ClientEntity_new(type, flags | CEF_DONT_LINK, origin, NULL, 250);

		impact->r.model = &halo_model;
		impact->r.flags = (RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
		impact->r.scale = 0.4f;
		impact->d_scale = -1.6f;
		impact->d_alpha = -3.0f; // Alpha goes up to 1, then down to zero.
		impact->color.c = 0xc0ffffff;
		impact->radius = 10.0f;
		impact->alpha = 0.8f;
		impact->r.origin[2] += 8.0f;

		AddEffect(NULL, impact); // Add the effect as independent world effect.
	}
}