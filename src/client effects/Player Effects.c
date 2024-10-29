#include "Client Effects.h"
#include "Client Entities.h"
#include "Particle.h"
#include "ResourceManager.h"
#include "FX.h"
#include "Random.h"
#include "Reference.h"
#include "Utilities.h"
#include "g_playstats.h"

void FXCompass(centity_t *owner, int type, int flags, vec3_t origin);

qboolean PlayerFirstSeenInit(struct client_entity_s *self, centity_t *owner)
{
	void FXPlayerShadow(centity_t *owner, int type, int flags, vec3_t origin);

	// Is the modelindex valid? E.g. when a player is dead, his modelindex is 0, hence his
	// referenceInfo will be invailid.

	if(owner->current.modelindex!=255)
		return(false);
	
	// Enable all Corvus' reference points.

	self->refMask |= CORVUS_MASK;	

	EnableRefPoints(owner->referenceInfo, self->refMask);

	self->AddToView = NULL;
	self->Update = KeepSelfAI;

	// Spawn the shadow.

	if(r_detail->value >= DETAIL_NORMAL)
		FXPlayerShadow(owner, FX_SHADOW, CEF_OWNERS_ORIGIN, owner->origin);

	if(r_detail->value > DETAIL_NORMAL)
		FXWaterParticles(owner, FX_WATER_PARTICLES, 0, owner->origin);

	//This is a cheap, silly effect that probably could look better and doesn't use the viewstatuschanged flag... urg...
	//if(compass->value)
	//	FXCompass(owner, FX_WATER_PARTICLES, 0, owner->origin);

	FXCrosshair(owner, FX_CROSSHAIR, 0, owner->origin);

	return(true);
}

void FXPlayerPersistant(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*self;
	vec_t				gravity;

	// I believe that gravity is contained somewhere in the player/client sturcture already -JKH

	gravity = 675.0;

	flags |= CEF_NO_DRAW;

	self = ClientEntity_new(type, flags, origin, NULL, 17);
	
	self->Update = NULL;
	self->AddToView = PlayerFirstSeenInit;

	AddEffect(owner, self);

	fxi.Cvar_SetValue("clfx_gravity", gravity);
}

// end