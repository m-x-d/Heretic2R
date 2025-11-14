//
// fx_crosshair.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"

static struct model_s* crosshair_model;

void PreCacheCrosshair(void)
{
	crosshair_model = fxi.RegisterModel("sprites/fx/crosshair.sp2");
}

static qboolean DrawCrosshair(struct client_entity_s* crosshair_ent, centity_t* owner)
{
	crosshair_ent->flags |= CEF_CULLED | CEF_DISAPPEARED;

	// Get new destination.
	byte type;
	if (fxi.Get_Crosshair(crosshair_ent->r.origin, &type))
	{
		crosshair_ent->r.frame = (type > 2 ? 0 : type);
		crosshair_ent->r.flags = (RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_NODEPTHTEST);

		float alpha = 0.5f + fabsf(sinf((float)fx_time / 800.0f)) * 0.5f;
		alpha = Clamp(alpha, 0.0f, 1.0f) * 0.5f;

		crosshair_ent->alpha = alpha + 0.25f;
		crosshair_ent->r.scale = alpha;

		crosshair_ent->flags &= ~(CEF_CULLED | CEF_DISAPPEARED | CEF_NO_DRAW);
	}

	return true;
}

void FXCrosshair(centity_t* owner, const int type, int flags, vec3_t origin)
{
	flags |= CEF_NO_DRAW;
	client_entity_t* xh = ClientEntity_new(type, flags, origin, NULL, 0); //mxd. next_think_time:20 in original logic. Update on every renderframe instead.

	xh->r.model = &crosshair_model;
	xh->Update = DrawCrosshair;

	AddEffect(owner, xh);
}