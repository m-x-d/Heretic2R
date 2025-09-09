//
// fx_pickup.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"

static struct model_s* pickup_model;

void PreCachePickup(void)
{
	pickup_model = fxi.RegisterModel("sprites/fx/halo.sp2");
}

void FXPickup(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	client_entity_t* halo = ClientEntity_new(type, flags | CEF_DONT_LINK | CEF_PULSE_ALPHA, origin, NULL, 500);

	halo->radius = 10.0f;
	halo->r.model = &pickup_model;
	halo->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	halo->r.frame = 1;

	halo->r.scale = 0.4f;
	halo->d_scale = 1.0f;

	halo->alpha = 0.8f; //mxd. Was also set to 0.75 above in original version.
	halo->d_alpha = -2.5f; //mxd. 2.5 in original logic. Flipped for fade-out effect.

	halo->r.origin[2] += 8.0f;
	halo->acceleration[2] = 180.0f; //mxd

	AddEffect(NULL, halo);
}