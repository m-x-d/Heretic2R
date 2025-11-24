//
// Vector.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include <math.h>

#include "H2Common.h"
#include "Angles.h"
#include "q_Typedef.h"

#ifdef __cplusplus //mxd. Needed, so the Script system code could build...
extern "C"
{
#endif

#define FLOAT_ZERO_EPSILON 0.0005f

//mxd. Empty vector "constructor" macro.
#define VEC3_ZERO				{ 0.0f, 0.0f, 0.0f }

//mxd. Vector "constructor" macro.
#define VEC3_SET(x, y, z)		{ (x), (y), (z) }

//mxd. Vector copy "constructor" macro.
#define VEC3_INIT(v)			{ (v)[0], (v)[1], (v)[2] }

//mxd. Vector copy add "constructor" macro.
#define VEC3_INITA(v, x, y, z)	{ (v)[0] + (x), (v)[1] + (y), (v)[2] + (z) }

//mxd. Vector copy scale "constructor" macro.
#define VEC3_INITS(v, s)	{ (v)[0] * (s), (v)[1] * (s), (v)[2] * (s) }

H2COMMON_API extern void RotatePointAroundVector(vec3_t dst, const vec3_t dir, const vec3_t point, float degrees);
H2COMMON_API extern void ProjectPointOnPlane(vec3_t dst, const vec3_t p, const vec3_t normal);
H2COMMON_API extern void PerpendicularVector(vec3_t dst, const vec3_t src);
H2COMMON_API extern void AngleVectors(const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up);
H2COMMON_API extern void RealAngleVectors(const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up);

H2COMMON_API extern void DirFromAngles(const vec3_t angles, vec3_t direction);
H2COMMON_API extern void DirAndUpFromAngles(const vec3_t angles, vec3_t direction, vec3_t up);
H2COMMON_API extern void AnglesFromDir(const vec3_t direction, vec3_t angles);
H2COMMON_API extern void AnglesFromDirI(const vec3_t direction, vec3_t angles);
H2COMMON_API extern void vectoangles(const vec3_t in, vec3_t out);
H2COMMON_API extern void AnglesFromDirAndUp(vec3_t direction, vec3_t up, vec3_t angles);
H2COMMON_API extern qboolean VectorCompare(const vec3_t v1, const vec3_t v2); //mxd. Return type: int -> qboolean.
H2COMMON_API extern float VectorNormalize(vec3_t v); // Returns vector length.
H2COMMON_API extern float Vec3Normalize(vec3_t v); //mxd. Just calls VectorNormalize() //TODO: remove?
H2COMMON_API extern float VectorNormalize2(const vec3_t v, vec3_t out);
H2COMMON_API extern void VectorClamp(vec3_t v, float max_length); //mxd
H2COMMON_API extern void VectorMA(const vec3_t veca, float scale, const vec3_t vecb, vec3_t out);
H2COMMON_API extern void VectorAverage(const vec3_t veca, const vec3_t vecb, vec3_t out);
H2COMMON_API extern void VectorLerp(const vec3_t veca, float frac, const vec3_t vecb, vec3_t out); //mxd
H2COMMON_API extern void VectorGetOffsetOrigin(const vec3_t offset, const vec3_t origin, float angle_deg, vec3_t out);
H2COMMON_API extern float VectorSeparation(const vec3_t v1, const vec3_t v2);
H2COMMON_API extern void VectorRandomCopy(const vec3_t in, vec3_t out, float random_amount);
H2COMMON_API extern void CrossProduct(const vec3_t v1, const vec3_t v2, vec3_t cross);
H2COMMON_API extern float VectorLength(const vec3_t v);
H2COMMON_API extern float VectorLengthSquared(const vec3_t v);
H2COMMON_API extern void VectorRandomAdd(const vec3_t origin, const vec3_t random_amount, vec3_t out);
H2COMMON_API extern void VectorSubtract(const vec3_t veca, const vec3_t vecb, vec3_t out);
H2COMMON_API extern void VectorAdd(const vec3_t veca, const vec3_t vecb, vec3_t out);
H2COMMON_API extern float vhlen(const vec3_t v1, const vec3_t v2);
H2COMMON_API extern void Create_rand_relect_vect(vec3_t in, vec3_t out);
H2COMMON_API extern qboolean Vec3IsZeroEpsilon(const vec3_t v);

H2COMMON_API extern void VectorAbs(const vec3_t in, vec3_t out);
H2COMMON_API extern void VectorRound(vec3_t v);
H2COMMON_API extern float DotProduct(const vec3_t v1, const vec3_t v2);
H2COMMON_API extern void VectorDec(vec3_t v);
H2COMMON_API extern void VectorInc(vec3_t v);
H2COMMON_API extern void VectorClear(vec3_t v);
H2COMMON_API extern void VectorSet(vec3_t v, float x, float y, float z);
H2COMMON_API extern void VectorRandomSet(vec3_t v, float rand_val); //mxd
H2COMMON_API extern void VectorCopy(const vec3_t in, vec3_t out);
H2COMMON_API extern void VectorInverse(vec3_t v);
H2COMMON_API extern void VectorNegate(const vec3_t in, vec3_t out);
H2COMMON_API extern void VectorScale(const vec3_t in, float scale, vec3_t out);
H2COMMON_API extern void VectorRadiansToDegrees(const vec3_t in, vec3_t out);
H2COMMON_API extern void VectorDegreesToRadians(const vec3_t in, vec3_t out);
H2COMMON_API extern void VectorScaleByVector(const vec3_t in, const vec3_t scale, vec3_t out);
H2COMMON_API extern float VectorSeparationSquared(const vec3_t va, const vec3_t vb);

H2COMMON_API extern void Vec3SubtractAssign(const vec3_t value, vec3_t subFrom);
H2COMMON_API extern void Vec3AddAssign(const vec3_t value, vec3_t addTo);
H2COMMON_API extern void Vec3MultAssign(const vec3_t value, vec3_t multBy);
H2COMMON_API extern void Vec3ScaleAssign(float value, vec3_t scaleBy);
H2COMMON_API extern qboolean FloatIsZeroEpsilon(float f);
H2COMMON_API extern qboolean FloatIsZero(float f, float epsilon);
H2COMMON_API extern qboolean Vec3EqualsEpsilon(const vec3_t v1, const vec3_t v2);
H2COMMON_API extern qboolean Vec3IsZero(const vec3_t vec);
H2COMMON_API extern qboolean Vec3NotZero(const vec3_t vec);

#ifdef __cplusplus
}
#endif