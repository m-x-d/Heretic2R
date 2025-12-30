//
// fx_lensflare.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Vector.h"
#include "g_playstats.h"

#define NUM_LENS_MODELS 7
static struct model_s* flare_models[NUM_LENS_MODELS];

void PreCacheFlare(void)
{
	flare_models[0] = fxi.RegisterModel("sprites/lens/blind1.sp2");

	for (int i = 1; i < NUM_LENS_MODELS; i++)
		flare_models[i] = fxi.RegisterModel(va("sprites/lens/flare%i.sp2", i));
}

//mxd. Added to reduce code duplication. //TODO: the logic is kinda broken, at least for env_sun1 entity...
static qboolean LensFlareUpdateOrigin(struct client_entity_s* self)
{
	// Determine visibility.
	trace_t tr;
	const int brushmask = (CONTENTS_SOLID | CONTENTS_MONSTER | CONTENTS_DEADMONSTER);
	fxi.Trace(fxi.cl->refdef.vieworg, vec3_origin, vec3_origin, self->direction, brushmask, CEF_CLIP_TO_WORLD, &tr);

	if (tr.fraction < 1.0f && !(tr.surface->flags & SURF_SKY))
	{
		self->flags |= CEF_NO_DRAW;
		return true;
	}

	vec3_t view_fwd;
	AngleVectors(fxi.cl->refdef.viewangles, view_fwd, NULL, NULL);

	vec3_t halo_dir;
	VectorSubtract(self->direction, fxi.cl->refdef.vieworg, halo_dir);
	VectorNormalize(halo_dir);

	const float view_dot = DotProduct(view_fwd, halo_dir);

	if (view_dot < 0.75f)
	{
		self->flags |= CEF_NO_DRAW;
		return true;
	}

	if (self->flags & CEF_NO_DRAW)
		self->flags &= ~CEF_NO_DRAW;

	if (self->up[1] != 0.0f) // Halo sprite.
	{
		self->alpha = self->up[2] - ((1.0f - view_dot) * 2.0f);
		Clamp(self->alpha, 0.1f, 1.0f);

		VectorMA(fxi.cl->refdef.vieworg, 16.0f, halo_dir, self->r.origin);

		return true;
	}

	// Lens flare sprite.
	vec3_t view_dir;
	VectorSubtract(self->direction, fxi.cl->refdef.vieworg, view_dir);
	VectorNormalize(view_dir);

	vec3_t view_offset;
	VectorMA(fxi.cl->refdef.vieworg, 250.0f, view_dir, view_offset);

	VectorSubtract(view_offset, fxi.cl->refdef.vieworg, view_dir);
	const float view_dist = VectorNormalize(view_dir);

	vec3_t fwd_offset;
	VectorScale(view_fwd, view_dist, fwd_offset);

	vec3_t light_offset;
	VectorAdd(view_offset, fwd_offset, light_offset);

	const float near_clip = 1.01f;
	VectorScale(view_dir, near_clip, view_dir);

	vec3_t center;
	VectorAdd(view_offset, view_dir, center);

	vec3_t light_dir;
	VectorSubtract(light_offset, view_offset, light_dir);
	VectorNormalize(light_dir);

	const float light_dot = DotProduct(view_dir, light_dir);

	vec3_t tmp;
	VectorScale(light_dir, near_clip, tmp);
	VectorScale(tmp, 1.0f / light_dot, tmp);

	vec3_t light_pos;
	VectorAdd(tmp, view_offset, light_pos);

	vec3_t axis;
	VectorSubtract(light_pos, center, axis);

	vec3_t dx;
	VectorCopy(axis, dx);
	VectorNormalize(dx);

	vec3_t dy;
	CrossProduct(dx, view_dir, dy);

	vec3_t t_axis;
	VectorScale(axis, self->up[0] * 1000.0f, t_axis);
	VectorAdd(center, t_axis, self->r.origin);

	return true;
}

// FIXME: These need to interpolate their movement so as to not do snap position changes.
static qboolean LensFlareUpdate(struct client_entity_s* self, centity_t* owner) //mxd. Named 'FXFlareThink' in original logic.
{
	if (self->LifeTime > 0 && self->LifeTime < fx_time)
		return false;

	return LensFlareUpdateOrigin(self); //mxd
}

static qboolean LensFlareAttachedUpdate(struct client_entity_s* self, centity_t* owner) //mxd. Named 'FXFlareThinkAttached' in original logic.
{
	if (self->LifeTime > 0 && self->LifeTime < fx_time)
		return false;

	const centity_t* fake_owner = (centity_t*)self->extra;
	if (fake_owner->current.effects & EF_DISABLE_EXTRA_FX)
		return false;

	// Interpolate. Why am I only getting 2 frames of interpolation?
	const float oldtime = (float)self->lastThinkTime / 100.0f;
	const float newtime = (float)fx_time / 100.0f;

	if ((int)oldtime < (int)newtime)
	{
		VectorCopy(self->endpos2, self->startpos2);
		VectorCopy(fake_owner->current.origin, self->endpos2); // Where I need to be.
		self->lastThinkTime = fx_time;
	}

	const float lerp = newtime - (int)newtime;

	vec3_t diff;
	VectorSubtract(self->endpos2, self->startpos2, diff); // Diff between last updated spot and where to be.
	VectorMA(self->startpos2, lerp, diff, self->direction);

	return LensFlareUpdateOrigin(self); //mxd
}

void FXLensFlare(centity_t* owner, int type, const int flags, vec3_t origin)
{
	static const int sprite_indices[] = { 1, 2, 4, 3, 6, 3 }; //mxd //TODO: index 5 is unused.
	static const float flare1_pos[] =  { 1.0f, 0.7f,  0.3f, 0.1f,  0.0f, -0.2f, -0.5f };
	static const float flare2_pos[] =  { 1.0f, 0.8f,  0.6f, 0.2f,  0.0f, -0.4f, -0.9f }; //mxd. Split into 2 arrays, added 7-th value.
	static const float flare_scale[] = { 2.0f, 1.75f, 1.5f, 1.25f, 1.5f,  1.75f, 2.0f }; //mxd. Removed 8-th value.

	float alpha;
	paletteRGBA_t tint;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_LENSFLARE].formatString, &tint.r, &tint.g, &tint.b, &alpha);

	// No lens flares in low detail.
	if (R_DETAIL < DETAIL_NORMAL) //mxd. '<= DETAIL_NORMAL' in original logic.
		return;

	for (int i = 0; i < NUM_LENS_MODELS; i++)
	{
		client_entity_t* flare = ClientEntity_new(FX_LENSFLARE, 0, origin, NULL, 0); //mxd. next_think_time:17 in original logic.

		COLOUR_COPY(tint, flare->r.color); //mxd. Use macro.
		flare->alpha = alpha;

		if (i == NUM_LENS_MODELS - 1)
		{
			flare->r.model = &flare_models[0]; // Halo sprite.
			flare->up[1] = 1.0f;
			flare->up[2] = alpha;

			if (flags & CEF_FLAG8)
				COLOUR_SET(flare->r.color, 255, 255, 255);
		}
		else
		{
			const int sprite_index = sprite_indices[i]; // Lens flare sprite.
			flare->r.model = &flare_models[sprite_index];
		}

		if (flags & CEF_FLAG8)
			flare->LifeTime = fx_time + 4000;

		flare->r.flags = (RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_NODEPTHTEST);
		flare->Scale = flare_scale[i];
		VectorCopy(origin, flare->direction);

		if (flags & CEF_FLAG7)
			Vec3ScaleAssign(4.0f, flare->direction);

		flare->up[0] = ((flags & CEF_FLAG6) ? flare2_pos[i] : flare1_pos[i]);

		if (owner != NULL)
		{
			AddEffect(owner, flare);

			flare->extra = owner;
			flare->Update = LensFlareAttachedUpdate;

			LensFlareAttachedUpdate(flare, owner);

			VectorCopy(flare->direction, flare->startpos2);
			VectorCopy(flare->direction, flare->endpos2);

			flare->lastThinkTime = fx_time;
		}
		else
		{
			AddEffect(NULL, flare);

			flare->Update = LensFlareUpdate;
		}
	}
}