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

static qboolean FXDrawCrosshair(struct client_entity_s *cross_hair, centity_t *owner)
{
	float			alpha;
	byte			type;

	cross_hair->flags |= CEF_CULLED | CEF_DISAPPEARED;

	// Get new destination
	if (fxi.Get_Crosshair(cross_hair->r.origin, &type))
	{
		if (type > 2)
			cross_hair->r.frame = 0;
		else
			cross_hair->r.frame = type;

		cross_hair->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_NODEPTHTEST;
		
		alpha = 0.5 + (Q_fabs(sin(fxi.cl->time / 800.0)) * 0.5);

		if (alpha > 1.0f)
			alpha = 1.0f;
		
		if (alpha < 0.0f)
			alpha = 0.0f;

		cross_hair->alpha = 0.25 + alpha * 0.5;
		cross_hair->r.scale = alpha * 0.5;

		cross_hair->flags &= ~(CEF_CULLED | CEF_DISAPPEARED | CEF_NO_DRAW);
	}
	return(true);
}

/*
==============
UpdateCrosshair

==============
*/
void FXCrosshair(centity_t *owner, int type, int flags, vec3_t origin)
{
#define CROSSHAIR_THINKTIME	20

	client_entity_t		*xh;

	xh = ClientEntity_new(type, flags | CEF_NO_DRAW, origin, NULL, CROSSHAIR_THINKTIME);
	
	xh->r.model = &crosshair_model;
	xh->Update = FXDrawCrosshair;

	AddEffect(owner, xh);
}

// end