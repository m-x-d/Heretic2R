//
// fx_halo.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Vector.h"
#include "g_playstats.h"

#define HALO_MAX_CAMERA_DISTANCE	1024.0f //mxd
#define HALO_MAX_ALPHA				1.0f //mxd
#define HALO_MIN_ALPHA				0.01f //mxd

static struct model_s* halo_models[2];

void PreCacheHalos(void)
{
	halo_models[0] = fxi.RegisterModel("sprites/lens/halo1.sp2");
	halo_models[1] = fxi.RegisterModel("sprites/lens/halo2.sp2");
}

static void HaloUpdateVisibility(client_entity_t* self, const float cam_dist, const qboolean is_visible) //mxd
{
	const float dist_scaler = cam_dist / HALO_MAX_CAMERA_DISTANCE;
	self->r.scale = 0.75f + dist_scaler * 0.75f;

	if (is_visible)
	{
		const float max_alpha = 0.25f + (1.0f - dist_scaler * HALO_MAX_ALPHA) * 0.75f;
		self->alpha = min(max_alpha, self->alpha * 1.35f);
		self->flags &= ~CEF_NO_DRAW;
	}
	else if (!(self->flags & CEF_NO_DRAW))
	{
		self->alpha = max(HALO_MIN_ALPHA, self->alpha * 0.9f);

		if (self->alpha == HALO_MIN_ALPHA)
			self->flags |= CEF_NO_DRAW;
	}
}

static qboolean HaloUpdate(client_entity_t* self, centity_t* owner)
{
	vec3_t dir;
	VectorSubtract(self->r.origin, fxi.cl->refdef.vieworg, dir);
	const float cam_dist = VectorNormalize(dir);

	if (cam_dist > HALO_MAX_CAMERA_DISTANCE) // Too far from camera.
	{
		HaloUpdateVisibility(self, cam_dist, false); //mxd
	}
	else
	{
		// Determine visibility.
		trace_t trace;
		fxi.Trace(self->r.origin, vec3_origin, vec3_origin, fxi.cl->refdef.vieworg, (CONTENTS_SOLID | CONTENTS_MONSTER | CONTENTS_DEADMONSTER), CEF_CLIP_TO_ALL, &trace);

		HaloUpdateVisibility(self, cam_dist, trace.fraction == 1.0f); //mxd
	}

	return true;
}

void FXHalo(centity_t* owner, int type, int flags, vec3_t origin)
{
	// No halo's for normal or low details. They are really expensive in traces.
	if (R_DETAIL <= DETAIL_NORMAL)
		return;

	flags |= (CEF_NO_DRAW | CEF_VIEWSTATUSCHANGED);
	client_entity_t* halo = ClientEntity_new(FX_HALO, flags, origin, NULL, 0); //mxd. next_think_time:100 in original logic.

	// Decide which halo image to use.
	const int sprite_index = ((flags & CEF_FLAG6) ? 1 : 0);
	halo->r.model = &halo_models[sprite_index];

	halo->r.flags = (RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_NODEPTHTEST);

	// To figure out tint, we only want the top two bits of flags.
	flags &= (CEF_FLAG7 | CEF_FLAG8);

	switch (flags)
	{
		case CEF_FLAG7: // Blue-ish halo.
			COLOUR_SET(halo->r.color, 90, 90, 175); //mxd. Use macro.
			break;

		case CEF_FLAG8: // Yellow halo.
			COLOUR_SET(halo->r.color, 190, 180, 16); //mxd. Use macro.
			break;

		case CEF_FLAG7 | CEF_FLAG8: // White halo. mxd. halo->r.color is already white.
			break;

		default: // Orange-brown-ish halo.
			COLOUR_SET(halo->r.color, 148, 132, 82); //mxd. Use macro.
			break;
	}

	halo->radius = ((flags & CEF_FLAG6) ? 16.0f : 32.0f); //mxd
	halo->alpha = HALO_MIN_ALPHA; //mxd. 0.6 in original logic.
	halo->Update = HaloUpdate;

	AddEffect(owner, halo);
}