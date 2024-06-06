#ifndef Q_PHYSICS_H
#define Q_PHYSICS_H

#include "H2Common.h"
#include "q_Typedef.h"
#include "q_Shared.h"	// for trace_t

#define ELASTICITY_NONE				0.0f
#define ELASTICITY_SLIDE			1.0001f
#define ELASTICITY_ENTITY_BOUNCE	1.1f
#define ELASTICITY_BOUNCE			1.5f
#define ELASTICITY_REFLECT			2.0f
#define ELASTICITY_MACEBALL			2.0f

#define GROUND_NORMAL				0.7f

#define GRAVITY						675.0f
#define GRAVITY_STRING				"675.0"		// to set cvars with

#define	FRICTION					1600.0f
#define FRICTION_STRING				"1600.0"		// to set cvars with

#define	STOP_EPSILON				0.1

#define MAX_VELOCITY				2000
#define MAX_VELOCITY_STRING			"2000"

#define	PHYSICS_Z_FUDGE				0.5
#define	CHECK_BELOW_DIST			0.5
#define Z_VEL_NOT_ONGROUND			100

typedef struct FormMove_s
{
	vec3_t					mins, maxs;
	float					*start;
	float					*end;
	void					*passEntity;
	int						clipMask;
	trace_t					trace;
	int						waterLevel;
	int						waterType;
	float					stepHeight;
	float					dropHeight;
	int						processFlags;		// filled in prior to passing in the FormMove
												// to a high level physics call
	int						resultFlags;		// will eventually be filled in by phsyics
} FormMove_t;

//************************************************************************
//
// Physics Process Flags
//
//************************************************************************

#define PPF_INFO_GRAB				0x00000001	// indicates the funtion is being only for informational purposes
												// The entities position will not be modified and it will not be linked
												// This currently only works with DiscreteMove_Step

//************************************************************************
//
// Physics Result Flags
//
//************************************************************************

#define PRF_COLLISION				0x00000001	// indicates there was a collision
#define PRF_EXPANSION_BLOCKED		0x00000002	// indicates an expansion was blocked

H2COMMON_API void BounceVelocity(vec3_t in, vec3_t normal, vec3_t out, float elasticity);
H2COMMON_API qboolean BoundVelocity(float *vel);

#endif // Q_PHYSICS_H