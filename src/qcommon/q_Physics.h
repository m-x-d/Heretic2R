//
// q_Physics.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "H2Common.h"
#include "q_Typedef.h"
#include "q_shared.h" // For trace_t.

#define ELASTICITY_NONE				0.0f
#define ELASTICITY_SLIDE			1.0001f
#define ELASTICITY_MACEBALL			2.0f

#define GROUND_NORMAL				0.7f
#define GRAVITY						675.0f

#define GRAVITY_STRING				"675.0" // To set cvars with.
#define FRICTION_STRING				"1600.0" // To set cvars with.
#define MAX_VELOCITY_STRING			"2000.0" // To set cvars with.

#define STOP_EPSILON				0.1f

#define PHYSICS_Z_FUDGE				0.5f
#define CHECK_BELOW_DIST			0.5f
#define Z_VEL_NOT_ONGROUND			100.0f

typedef struct FormMove_s
{
	vec3_t mins;
	vec3_t maxs;
	float* start;
	float* end;
	void* passEntity;
	int clipMask;
	trace_t trace;
} FormMove_t;

H2COMMON_API void BounceVelocity(const vec3_t in, const vec3_t normal, vec3_t out, float elasticity);
H2COMMON_API qboolean BoundVelocity(float *vel);