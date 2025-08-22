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

#ifdef __cplusplus //mxd. Needed, so code in game/ds.cpp could build...
extern "C"
{
#endif

#define FLOAT_ZERO_EPSILON 0.0005f

H2COMMON_API void RotatePointAroundVector(vec3_t dst, const vec3_t dir, const vec3_t point, float degrees);
H2COMMON_API void ProjectPointOnPlane(vec3_t dst, const vec3_t p, const vec3_t normal);
H2COMMON_API void PerpendicularVector(vec3_t dst, const vec3_t src);
H2COMMON_API void AngleVectors(const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up);
H2COMMON_API void RealAngleVectors(const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up);

H2COMMON_API void DirFromAngles(const vec3_t angles, vec3_t direction);
H2COMMON_API void DirAndUpFromAngles(const vec3_t angles, vec3_t direction, vec3_t up);
H2COMMON_API void AnglesFromDir(const vec3_t direction, vec3_t angles);
H2COMMON_API void AnglesFromDirI(const vec3_t direction, vec3_t angles);
H2COMMON_API void vectoangles(const vec3_t in, vec3_t out);
H2COMMON_API void AnglesFromDirAndUp(vec3_t direction, vec3_t up, vec3_t angles);
H2COMMON_API qboolean VectorCompare(const vec3_t v1, const vec3_t v2); //mxd. Return type: int -> qboolean
H2COMMON_API float VectorNormalize(vec3_t v); // Returns vector length
H2COMMON_API float Vec3Normalize(vec3_t v);	//mxd. Just calls VectorNormalize() //TODO: remove?
H2COMMON_API float VectorNormalize2(const vec3_t v, vec3_t out);
H2COMMON_API void VectorMA(const vec3_t veca, float scale, const vec3_t vecb, vec3_t out);
H2COMMON_API void VectorAverage(const vec3_t veca, const vec3_t vecb, vec3_t out);
H2COMMON_API void VectorGetOffsetOrigin(const vec3_t offset, const vec3_t origin, float angle_deg, vec3_t out);
H2COMMON_API float VectorSeparation(const vec3_t v1, const vec3_t v2);
H2COMMON_API void VectorRandomCopy(const vec3_t in, vec3_t out, float random_amount);
H2COMMON_API void CrossProduct(const vec3_t v1, const vec3_t v2, vec3_t cross);
H2COMMON_API float VectorLength(const vec3_t v);
H2COMMON_API float VectorLengthSquared(const vec3_t v);
H2COMMON_API void VectorRandomAdd(const vec3_t origin, const vec3_t random_amount, vec3_t out);
H2COMMON_API void VectorSubtract(const vec3_t veca, const vec3_t vecb, vec3_t out);
H2COMMON_API void VectorAdd(const vec3_t veca, const vec3_t vecb, vec3_t out);
H2COMMON_API float vhlen(const vec3_t v1, const vec3_t v2);
H2COMMON_API void Create_rand_relect_vect(vec3_t in, vec3_t out);
H2COMMON_API qboolean Vec3IsZeroEpsilon(const vec3_t v);

H2COMMON_API void VectorAbs(const vec3_t in, vec3_t out);
H2COMMON_API void VectorRound(vec3_t v);
H2COMMON_API float DotProduct(const vec3_t v1, const vec3_t v2);
H2COMMON_API void VectorDec(vec3_t v);
H2COMMON_API void VectorInc(vec3_t v);
H2COMMON_API void VectorClear (vec3_t v);
H2COMMON_API void VectorSet(vec3_t v, float x, float y, float z);
H2COMMON_API void VectorRandomSet(vec3_t v, float rand_val); //mxd
H2COMMON_API void VectorCopy(const vec3_t in, vec3_t out);
H2COMMON_API void VectorInverse(vec3_t v);
H2COMMON_API void VectorNegate(const vec3_t in, vec3_t out);
H2COMMON_API void VectorScale(const vec3_t in, float scale, vec3_t out);
H2COMMON_API void VectorRadiansToDegrees(const vec3_t in, vec3_t out);
H2COMMON_API void VectorDegreesToRadians(const vec3_t in, vec3_t out);
H2COMMON_API void VectorScaleByVector(const vec3_t in, const vec3_t scale, vec3_t out);
H2COMMON_API float VectorSeparationSquared(const vec3_t va, const vec3_t vb);

H2COMMON_API void Vec3SubtractAssign(const vec3_t value, vec3_t subFrom);
H2COMMON_API void Vec3AddAssign(const vec3_t value, vec3_t addTo);
H2COMMON_API void Vec3MultAssign(const vec3_t value, vec3_t multBy);
H2COMMON_API void Vec3ScaleAssign(float value, vec3_t scaleBy);
H2COMMON_API qboolean FloatIsZeroEpsilon(float f);
H2COMMON_API qboolean FloatIsZero(float f, float epsilon);
H2COMMON_API qboolean Vec3EqualsEpsilon(const vec3_t v1, const vec3_t v2);
H2COMMON_API qboolean Vec3IsZero(const vec3_t vec);
H2COMMON_API qboolean Vec3NotZero(const vec3_t vec);

#ifdef __cplusplus
}
#endif