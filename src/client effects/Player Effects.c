//
// Player Effects.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Reference.h"
#include "Utilities.h"
#include "g_playstats.h"
#include "q_Physics.h"

static qboolean PlayerFirstSeenInit(struct client_entity_s* self, centity_t* owner)
{
	// Is the modelindex valid? E.g. when a player is dead, his modelindex is 0, hence his referenceInfo will be invalid.
	if (owner->current.modelindex != 255)
		return false;

	self->AddToView = NULL;
	self->Update = KeepSelfAI;

	// Spawn the shadow.
	if (r_detail->value >= DETAIL_NORMAL)
		FXPlayerShadow(owner, FX_SHADOW, CEF_OWNERS_ORIGIN, owner->origin);

	if (r_detail->value > DETAIL_NORMAL)
		FXWaterParticles(owner, FX_WATER_PARTICLES, 0, owner->origin);

	FXCrosshair(owner, FX_CROSSHAIR, 0, owner->origin);

	return true;
}

void FXPlayerPersistant(centity_t* owner, const int type, int flags, vec3_t origin)
{
	flags |= CEF_NO_DRAW;

	client_entity_t* self = ClientEntity_new(type, flags, origin, NULL, 17);

	self->Update = NULL;
	self->AddToView = PlayerFirstSeenInit;

	AddEffect(owner, self);

	fxi.Cvar_SetValue("clfx_gravity", GRAVITY); //TODO: already initialized in Init() (in Main.c). Why is it done again here?..
}