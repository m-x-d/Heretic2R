//
// fx_halo.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Vector.h"
#include "g_playstats.h"

static struct model_s* halo_models[2];

void PreCacheHalos(void)
{
	halo_models[0] = fxi.RegisterModel("sprites/lens/halo1.sp2");
	halo_models[1] = fxi.RegisterModel("sprites/lens/halo2.sp2");
}

static qboolean HaloThink(struct client_entity_s* self, centity_t* owner)
{
	// Effect will be deleted if CEF_DISAPPEARED flag is set.
	self->flags &= ~CEF_DISAPPEARED;

	// Default to nodraw.
	self->flags |= CEF_NO_DRAW;

	vec3_t cam_fwd;
	AngleVectors(fxi.cl->refdef.viewangles, cam_fwd, NULL, NULL);

	vec3_t dir;
	VectorSubtract(self->r.origin, fxi.cl->refdef.vieworg, dir);
	const float cam_dist = VectorNormalize(dir);

	if (cam_dist > 1024.0f || DotProduct(cam_fwd, dir) < 0.75f) // Too far from camera or outside of camera fov(?). //TODO: use actual camera fov?
		return true;

	// Determine visibility.
	trace_t trace;
	fxi.Trace(self->r.origin, vec3_origin, vec3_origin, fxi.cl->refdef.vieworg, (CONTENTS_SOLID | CONTENTS_MONSTER | CONTENTS_DEADMONSTER), CEF_CLIP_TO_ALL, &trace);

	if (trace.fraction < 1.0f) // Hit something.
	{
		if (trace.ent == (struct edict_s*)-1) // Hit a brush.
			return true;

		// Hit a model.
		const entity_state_t* es = (entity_state_t*)trace.ent;

		// Not the player entity.
		if (fxi.cl->playernum + 1 != es->number)
			return true;

		// Hit the player entity.
		//TODO: this logic is strange.
		// 1. Since halo is a purely visual effect, we shouldn't care about player entity position at all.
		// 2. Reduce alpha increment steps (to 0.1 or 0.05?), decrement to much lower value (0.05?), don't immediately turn off halo when path between camera and halo is blocked.
		vec3_t ent_pos;
		VectorCopy(es->origin, ent_pos);
		ent_pos[2] += 8.0f;

		vec3_t light_dir; // Direction from halo to camera position.
		VectorSubtract(fxi.cl->refdef.vieworg, self->r.origin, light_dir);
		VectorNormalize(light_dir);

		vec3_t halo_dir; // Direction from halo to player model.
		VectorSubtract(self->r.origin, ent_pos, halo_dir);

		vec3_t player_dir; // Direction from player to camera position.
		VectorSubtract(fxi.cl->refdef.vieworg, ent_pos, player_dir);

		const float player_dist = VectorNormalize(player_dir);
		float dist = VectorNormalize(halo_dir);

		vec3_t res_vec;
		VectorMA(self->r.origin, dist, light_dir, res_vec);
		VectorSubtract(ent_pos, res_vec, player_dir);

		dist = VectorNormalize(player_dir);

		if (dist < 10.0f + player_dist / 100.0f)
		{
			if (self->alpha > 0.25f)
				self->alpha -= 0.25f;

			return true;
		}
	}

	self->flags &= ~CEF_NO_DRAW;

	if (self->alpha < 0.5f)
		self->alpha += 0.25f;

	return true;
}

void FXHalo(centity_t* owner, int type, int flags, vec3_t origin)
{
	// No halo's for normal or low details. They are really expensive in traces.
	if (R_DETAIL <= DETAIL_NORMAL)
		return;

	flags |= CEF_NO_DRAW | CEF_VIEWSTATUSCHANGED;
	client_entity_t* halo = ClientEntity_new(FX_HALO, flags, origin, NULL, 100);

	// Decide which halo image to use.
	const int sprite_index = ((flags & CEF_FLAG6) ? 1 : 0);
	halo->r.model = &halo_models[sprite_index];

	halo->r.flags = RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_NODEPTHTEST;

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

	halo->alpha = 0.6f;
	halo->Update = HaloThink;

	AddEffect(owner, halo);
}