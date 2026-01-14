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

//mxd. Added to reduce code duplication.
static qboolean LensFlareUpdateOrigin(client_entity_t* self, const qboolean check_sky)
{
#define LF_BRUSHMASK		(CONTENTS_SOLID | CONTENTS_MONSTER | CONTENTS_DEADMONSTER)
#define LF_DOT_THRESHHOLD	0.8f
#define LF_DOT_SCALER		(1.0f / (1.0f - LF_DOT_THRESHHOLD))

	static float old_view_dot;

	// Determine visibility.
	qboolean sight_blocked = false;

	if (check_sky)
	{
		trace_t trace;
		fxi.Trace(fxi.cl->refdef.vieworg, vec3_origin, vec3_origin, self->direction, LF_BRUSHMASK, CEF_CLIP_TO_WORLD, &trace);
		sight_blocked = (trace.fraction < 1.0f && !(trace.surface->flags & SURF_SKY));
	}

	vec3_t view_fwd;
	AngleVectors(fxi.cl->refdef.viewangles, view_fwd, NULL, NULL);

	vec3_t halo_dir;
	VectorSubtract(self->direction, fxi.cl->refdef.vieworg, halo_dir);
	VectorNormalize(halo_dir);

	float view_dot = DotProduct(view_fwd, halo_dir) + 0.15f; //mxd. Offset a bit, so effect stays at max alpha even when not looking exactly in self->direction.
	view_dot = min(1.0f, view_dot);

	if (sight_blocked || view_dot < LF_DOT_THRESHHOLD)
	{
		self->alpha *= (self->up[1] == 0.0f ? 0.95f : 0.7f); // Fade-out halo sprite slower.
		view_dot = min(LF_DOT_THRESHHOLD, view_dot);

		if (self->alpha < 0.01f)
		{
			self->alpha = 0.01f;
			self->flags |= CEF_NO_DRAW;

			return true;
		}
	}
	else
	{
		const float alpha_scaler = (view_dot - LF_DOT_THRESHHOLD) * LF_DOT_SCALER;
		const float frac = (old_view_dot > view_dot ? 0.1f : 0.2f); // Fade-out slower.
		self->alpha = LerpFloat(self->alpha, alpha_scaler * self->up[2], frac);
		self->alpha = max(0.01f, self->alpha);
	}

	old_view_dot = view_dot;
	self->flags &= ~CEF_NO_DRAW;

	if (self->up[1] == 0.0f) // Halo sprite.
	{
		VectorMA(fxi.cl->refdef.vieworg, 10.0f + 16.0f * (1.0f - self->alpha / self->up[2]), halo_dir, self->r.origin);
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
	Vec3ScaleAssign(near_clip, view_dir);

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

	vec3_t t_axis;
	VectorScale(axis, self->up[0] * 1000.0f, t_axis);
	VectorAdd(center, t_axis, self->r.origin);

	return true;
}

// FIXME: These need to interpolate their movement so as to not do snap position changes.
static qboolean LensFlareUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXFlareThink' in original logic.
{
	if (self->LifeTime > 0 && self->LifeTime < fx_time)
		return false;

	return LensFlareUpdateOrigin(self, true); //mxd
}

static qboolean LensFlareAttachedUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXFlareThinkAttached' in original logic.
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

	return LensFlareUpdateOrigin(self, false); //mxd
}

void FXLensFlare(centity_t* owner, int type, const int flags, vec3_t origin)
{
	static const int sprite_indices[] = { 1, 2, 4, 3, 6, 3, 0 }; //mxd
	static const float flare1_pos[] =  { 1.0f, 0.7f,  0.3f, 0.1f,  0.0f, -0.2f, -0.5f };
	static const float flare2_pos[] =  { 1.0f, 0.8f,  0.6f, 0.2f,  0.0f, -0.4f, -0.9f }; //mxd. Split into 2 arrays, added 7-th value.
	static const float flare_scale[] = { 1.5f, 1.75f, 1.5f, 1.25f, 1.5f,  1.75f, 1.0f }; //mxd. Removed 8-th value.

	float alpha;
	paletteRGBA_t tint;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_LENSFLARE].formatString, &tint.r, &tint.g, &tint.b, &alpha);

	// No lens flares in low detail.
	if (R_DETAIL < DETAIL_NORMAL) //mxd. '<= DETAIL_NORMAL' in original logic.
		return;

	//mxd. Normalize and rescale sun direction.
	vec3_t direction;
	VectorNormalize2(origin, direction);
	Vec3ScaleAssign(4096.0f, direction);

	for (int i = 0; i < NUM_LENS_MODELS; i++)
	{
		client_entity_t* flare = ClientEntity_new(FX_LENSFLARE, 0, origin, NULL, 0); //mxd. next_think_time:17 in original logic.

		COLOUR_COPY(tint, flare->r.color); //mxd. Use macro.
		flare->r.flags = (RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_NODEPTHTEST);
		flare->r.scale = flare_scale[i]; //mxd. Original logic sets flare->Scale, which is not used for anything.
		flare->radius = 256.0f; //mxd. Make sure it's not culled.
		flare->flags |= CEF_NO_DRAW; //mxd. Start invisible.
		flare->alpha = 0.01f; //mxd. Start in fade-out state.

		const int sprite_index = sprite_indices[i];
		flare->r.model = &flare_models[sprite_index];

		flare->up[0] = ((flags & CEF_FLAG6) ? flare2_pos[i] : flare1_pos[i]);
		flare->up[1] = (float)sprite_index;
		flare->up[2] = alpha; //mxd. Store initial alpha for all lensflare sprites.

		if (sprite_index == 6 && (flags & CEF_FLAG7)) //mxd. Sun sprite.
			COLOUR_SET(flare->r.color, 255, 255, 255);

		if (sprite_index == 0 && (flags & CEF_FLAG8)) // Halo sprite.
			COLOUR_SET(flare->r.color, 255, 255, 255);

		VectorCopy(direction, flare->direction);

		if (owner != NULL)
		{
			AddEffect(owner, flare);

			flare->extra = owner;
			flare->Update = LensFlareAttachedUpdate;

			LensFlareAttachedUpdate(flare, owner);

			VectorCopy(flare->direction, flare->startpos2);
			VectorCopy(flare->direction, flare->endpos2);

			flare->LifeTime = fx_time + 4000;
			flare->lastThinkTime = fx_time;
		}
		else
		{
			AddEffect(NULL, flare);

			flare->Update = LensFlareUpdate;
		}
	}
}