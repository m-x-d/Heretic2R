//
// fx_scorchmark.c
//
// Heretic II
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Client Entities.h"
#include "Particle.h"
#include "ResourceManager.h"
#include "FX.h"
#include "Vector.h"
#include "Utilities.h"
#include "g_playstats.h"

#define	NUM_SCORCH_MODELS	1

static struct model_s *scorch_models[NUM_SCORCH_MODELS];

void PreCacheScorch()
{
	scorch_models[0] = fxi.RegisterModel("sprites/fx/scorchmark.sp2");
}


static qboolean EndLessLoop(struct client_entity_s *self, centity_t *owner)
{
	return(true);
}


// --------------------------------------------------------------
// Find exact plane to decal the scorchmark to

// The origin comes in 8 from the wall
// No scorchmark generated if no wall found (this does happen)

static qboolean GetTruePlane(vec3_t origin, vec3_t direction)
{
	trace_t		trace;
	vec3_t		end;
	vec3_t		mins, maxs;

	VectorClear(mins);
	VectorClear(maxs);
	// Quite a long trace - but its only done once per scorchmark ever
	VectorMA(origin, 64.0, direction, end);

	fxi.Trace(origin, mins, maxs, end, MASK_DRIP, CEF_CLIP_TO_WORLD, &trace);
	if(trace.fraction != 1.0)
	{
		// Set the new endpos and plane (should be exact)
		VectorCopy(trace.endpos, origin);
		VectorCopy(trace.plane.normal, direction);
		// Raise the scorchmark slightly off the target wall
		VectorMA(origin, 0.5, direction, origin);
		return(true);
	}
	return(false);
}

void FXClientScorchmark(vec3_t origin, vec3_t dir)
{
	client_entity_t	*scorchmark;

	// no scorchmarks in low detail mode
	if (r_detail->value == DETAIL_LOW)
		return;
	
	if(GetTruePlane(origin, dir))
	{
		scorchmark = ClientEntity_new(FX_SCORCHMARK, CEF_NOMOVE, origin, dir, 1000);

		scorchmark->r.model = scorch_models;
		scorchmark->r.flags |= RF_FIXED | RF_TRANSLUCENT;

		scorchmark->radius = 10.0;
		scorchmark->r.scale = 0.6;

		scorchmark->Update = EndLessLoop;
		
		AddEffect(NULL, scorchmark);
		InsertInCircularList(scorchmark);
	}
}

void FXScorchmark(centity_t *owner, int type, int flags, vec3_t origin)
{
	vec3_t			dir;
	client_entity_t	*scorchmark;

	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_SCORCHMARK].formatString, dir);

	// no scorchmarks in low detail mode
	if (r_detail->value == DETAIL_LOW)
		return;

	if(GetTruePlane(origin, dir))
	{
		scorchmark = ClientEntity_new(type, flags | CEF_NOMOVE, origin, dir, 1000);

		scorchmark->r.model = scorch_models;
		scorchmark->r.flags |= RF_FIXED | RF_TRANSLUCENT;

		scorchmark->radius = 10.0;
		scorchmark->r.scale = 0.6;

		scorchmark->Update = EndLessLoop;
		
		AddEffect(NULL, scorchmark);
		InsertInCircularList(scorchmark);
	}
}

// end