//
// fx_scorchmark.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Vector.h"
#include "Utilities.h"
#include "g_playstats.h"

static struct model_s* scorch_model;

void PreCacheScorch(void)
{
	scorch_model = fxi.RegisterModel("sprites/fx/scorchmark.sp2");
}

// Find exact plane to decal the scorchmark to.
// The origin comes in 8 from the wall. No scorchmark generated if no wall found (this does happen).
static qboolean GetTruePlane(vec3_t origin, vec3_t direction)
{
	// Quite a long trace - but its only done once per scorchmark ever.
	vec3_t end;
	VectorMA(origin, 64.0f, direction, end);

	trace_t trace;
	fxi.Trace(origin, vec3_origin, vec3_origin, end, MASK_DRIP, CEF_CLIP_TO_WORLD, &trace);

	if (trace.fraction != 1.0f)
	{
		// Set the new endpos and plane (should be exact).
		VectorCopy(trace.endpos, origin);
		VectorCopy(trace.plane.normal, direction);

		// Raise the scorchmark slightly off the target wall.
		VectorMA(origin, 0.5f, direction, origin);

		return true;
	}

	return false;
}

//mxd. Added to reduce code duplication.
static void CreateScorchmark(vec3_t origin, vec3_t dir, const int type, const int flags)
{
	// No scorchmarks in low detail mode.
	if ((int)r_detail->value == DETAIL_LOW)
		return;

	if (GetTruePlane(origin, dir))
	{
		client_entity_t* scorchmark = ClientEntity_new(type, flags, origin, dir, 1000);

		scorchmark->radius = 10.0f;
		scorchmark->r.model = &scorch_model;
		scorchmark->r.flags = RF_FIXED | RF_TRANSLUCENT;
		scorchmark->r.scale = 0.6f;
		scorchmark->Update = KeepSelfAI;

		AddEffect(NULL, scorchmark);
		InsertInCircularList(scorchmark);
	}
}

void FXClientScorchmark(vec3_t origin, vec3_t dir)
{
	CreateScorchmark(origin, dir, FX_SCORCHMARK, CEF_NOMOVE); //mxd
}

void FXScorchmark(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	vec3_t dir;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_SCORCHMARK].formatString, dir);

	CreateScorchmark(origin, dir, type, flags | CEF_NOMOVE); //mxd
}