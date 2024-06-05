#ifndef VECTOR_H
#define VECTOR_H

#include <math.h>
#include <assert.h>

#include "H2Common.h"
#include "q_Typedef.h"
#include "angles.h"
#include "q_shared.h"

// angle indexes
#define	PITCH				0		// up / down
#define	YAW					1		// left / right
#define	ROLL				2		// fall over

#define FLOAT_ZERO_EPSILON 0.0005f

#ifdef __cplusplus
extern "C"
{
#endif
H2COMMON_API void RotatePointAroundVector( vec3_t dst, const vec3_t dir, const vec3_t point, float degrees );
H2COMMON_API void ProjectPointOnPlane( vec3_t dst, const vec3_t p, const vec3_t normal );
H2COMMON_API void PerpendicularVector( vec3_t dst, const vec3_t src );
H2COMMON_API void AngleVectors (vec3_t angles, vec3_t forward, vec3_t right, vec3_t up);
H2COMMON_API void RealAngleVectors (vec3_t angles, vec3_t forward, vec3_t right, vec3_t up);

H2COMMON_API void DirFromAngles(vec3_t angles, vec3_t direction);
H2COMMON_API void DirAndUpFromAngles(vec3_t angles, vec3_t direction, vec3_t up);
H2COMMON_API void AnglesFromDir(vec3_t direction, vec3_t angles);
H2COMMON_API void AnglesFromDirI(vec3_t direction, vec3_t angles);
H2COMMON_API void vectoangles(vec3_t in, vec3_t out);
H2COMMON_API void AnglesFromDirAndUp(vec3_t direction, vec3_t up, vec3_t angles);
H2COMMON_API int VectorCompare (vec3_t v1, vec3_t v2);
H2COMMON_API vec_t VectorNormalize (vec3_t v);		// returns vector length
H2COMMON_API float Vec3Normalize(vec3_t v1);
H2COMMON_API vec_t VectorNormalize2 (vec3_t v, vec3_t out);
H2COMMON_API void VectorMA (vec3_t veca, float scale, vec3_t vecb, vec3_t vecc);
H2COMMON_API void VectorAverage (vec3_t veca, vec3_t vecb, vec3_t vecc);
H2COMMON_API void VectorGetOffsetOrigin(vec3_t off, vec3_t org, vec_t degree, vec3_t out);
H2COMMON_API vec_t VectorSeparation(vec3_t, vec3_t);
H2COMMON_API void VectorRandomCopy(vec3_t, vec3_t, float);
H2COMMON_API void CrossProduct (vec3_t v1, vec3_t v2, vec3_t cross);
H2COMMON_API vec_t VectorLength (vec3_t v);
H2COMMON_API vec_t VectorLengthSquared (vec3_t v);
H2COMMON_API void VectorRandomAdd (vec3_t veca, vec3_t vecb, vec3_t out);
H2COMMON_API void VectorSubtract (vec3_t veca, vec3_t vecb, vec3_t out);
H2COMMON_API void VectorAdd (vec3_t veca, vec3_t vecb, vec3_t out);
H2COMMON_API float vhlen (vec3_t p1, vec3_t p2);
H2COMMON_API void Create_rand_relect_vect(vec3_t in, vec3_t out);
H2COMMON_API qboolean Vec3IsZeroEpsilon(vec3_t in);

#define INLINE_VEC_FUNCS	// placed in the header to avoid inconsistencies between projects

#ifndef INLINE_VEC_FUNCS

H2COMMON_API void VectorAbs(const vec3_t in, vec3_t out);
H2COMMON_API void VectorRound(vec3_t v);
H2COMMON_API vec_t DotProduct (const vec3_t v1, const vec3_t v2);
H2COMMON_API void VectorDec(vec3_t v);
H2COMMON_API void VectorInc(vec3_t v);
H2COMMON_API void VectorClear (vec3_t in);
H2COMMON_API void VectorSet (vec3_t in, vec_t x, vec_t y, vec_t z);
H2COMMON_API void VectorCopy (vec3_t in, vec3_t out);
H2COMMON_API void VectorInverse (vec3_t v);
H2COMMON_API void VectorNegate (vec3_t in, vec3_t out);
H2COMMON_API void VectorScale (vec3_t in, vec_t scale, vec3_t out);
H2COMMON_API void VectorRadiansToDegrees (vec3_t in, vec3_t out);
H2COMMON_API void VectorDegreesToRadians (vec3_t in, vec3_t out);
H2COMMON_API void VectorScaleByVector(vec3_t, vec3_t, vec3_t);

H2COMMON_API void Vec3SubtractAssign(vec3_t value, vec3_t subFrom);
H2COMMON_API void Vec3AddAssign(vec3_t value, vec3_t addTo);
H2COMMON_API void Vec3MultAssign(vec3_t value, vec3_t multBy);
H2COMMON_API void Vec3ScaleAssign(vec_t value, vec3_t scaleBy);
H2COMMON_API qboolean FloatIsZeroEpsilon(float f);
H2COMMON_API qboolean FloatIsZero(float f, float epsilon);
H2COMMON_API qboolean Vec3EqualsEpsilon(vec3_t v1, vec3_t v2);
H2COMMON_API qboolean Vec3IsZero(vec3_t v1);
H2COMMON_API qboolean Vec3NotZero(vec3_t vec);

#ifdef __cplusplus
}
#endif // __cplusplus

#else // INLINE_VEC_FUNCS

#ifdef __cplusplus
}
#endif // __cplusplus

__inline vec_t DotProduct (const vec3_t v1, const vec3_t v2)
{
	return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}

__inline int VectorCompare (vec3_t v1, vec3_t v2)
{
	if ((v1[0] != v2[0]) || (v1[1] != v2[1]) || (v1[2] != v2[2]))
			return(false);
			
	return(true);
}

__inline void VectorMA (vec3_t veca, float scale, vec3_t vecb, vec3_t vecc)
{
	assert(vecc != vec3_origin);

	vecc[0] = veca[0] + scale*vecb[0];
	vecc[1] = veca[1] + scale*vecb[1];
	vecc[2] = veca[2] + scale*vecb[2];
}

__inline vec_t VectorLength(vec3_t v)
{
	float	length;
	
	length = sqrt(DotProduct(v, v));

	return length;
}

__inline vec_t VectorLengthSquared(vec3_t v)
{
	float	length;
	
	length = DotProduct(v, v);

	return length;
}

__inline void VectorSubtract (vec3_t veca, vec3_t vecb, vec3_t out)
{
	assert(out != vec3_origin);

	out[0] = veca[0]-vecb[0];
	out[1] = veca[1]-vecb[1];
	out[2] = veca[2]-vecb[2];
}

__inline vec_t VectorSeparationSquared(vec3_t va, vec3_t vb)
{
	vec3_t		work;
	vec_t		result;

	VectorSubtract(va, vb, work);
	result = DotProduct(work, work);
	return(result);
}

__inline void VectorAdd (vec3_t veca, vec3_t vecb, vec3_t out)
{
	assert(out != vec3_origin);

	out[0] = veca[0] + vecb[0];
	out[1] = veca[1] + vecb[1];
	out[2] = veca[2] + vecb[2];
}

__inline void VectorAbs(const vec3_t in, vec3_t out)
{
	assert(out != vec3_origin);

	out[0] = (float)Q_fabs(in[0]);
	out[1] = (float)Q_fabs(in[1]);
	out[2] = (float)Q_fabs(in[2]);
}

__inline void VectorRound(vec3_t v)
{
	assert(v != vec3_origin);

	v[0] = (float)floor(v[0] + 0.5);
	v[1] = (float)floor(v[1] + 0.5);
	v[2] = (float)floor(v[2] + 0.5);
}

__inline void VectorInc(vec3_t v)
{
	assert(v != vec3_origin);

	v[0] += 1.0;
	v[1] += 1.0;
	v[2] += 1.0;
}

__inline void VectorDec(vec3_t v)
{
	assert(v != vec3_origin);

	v[0] -= 1.0;
	v[1] -= 1.0;
	v[2] -= 1.0;
}

_inline void VectorClear (vec3_t in)
{
	assert(in != vec3_origin);

	in[0] = 0;
	in[1] = 0;
	in[2] = 0;
}

_inline void VectorSet (vec3_t in, vec_t x, vec_t y, vec_t z)
{
	assert(in != vec3_origin);

	in[0] = x;
	in[1] = y;
	in[2] = z;
}

_inline void VectorCopy(vec3_t in, vec3_t out)
{
	assert(out != vec3_origin);

	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
}

_inline void VectorInverse (vec3_t v)
{
	assert(v != vec3_origin);

	v[0] = -v[0];
	v[1] = -v[1];
	v[2] = -v[2];
}

_inline void VectorNegate (vec3_t in, vec3_t out)
{
	assert(out != vec3_origin);

	out[0] = -in[0];
	out[1] = -in[1];
	out[2] = -in[2];
}

_inline void VectorScale (vec3_t in, vec_t scale, vec3_t out)
{
	assert(out != vec3_origin);

	out[0] = in[0] * scale;
	out[1] = in[1] * scale;
	out[2] = in[2] * scale;
}

_inline void VectorRadiansToDegrees (vec3_t in, vec3_t out)
{
	assert(out != vec3_origin);

	out[0] = in[0] * RAD_TO_ANGLE;
	out[1] = in[1] * RAD_TO_ANGLE;
	out[2] = in[2] * RAD_TO_ANGLE;
}

_inline void VectorDegreesToRadians (vec3_t in, vec3_t out)
{
	assert(out != vec3_origin);

	out[0] = in[0] * ANGLE_TO_RAD;
	out[1] = in[1] * ANGLE_TO_RAD;
	out[2] = in[2] * ANGLE_TO_RAD;
}

_inline void VectorScaleByVector (vec3_t in, vec3_t scale, vec3_t out)
{
	assert(out != vec3_origin);

	out[0] = in[0] * scale[0];
	out[1] = in[1] * scale[1];
	out[2] = in[2] * scale[2];
}

_inline void Vec3SubtractAssign(vec3_t value, vec3_t subFrom)
{
	assert(subFrom != vec3_origin);

	subFrom[0] -= value[0];
	subFrom[1] -= value[1];
	subFrom[2] -= value[2];
}

_inline void Vec3AddAssign(vec3_t value, vec3_t addTo)
{
	assert(addTo != vec3_origin);

	addTo[0] += value[0];
	addTo[1] += value[1];
	addTo[2] += value[2];
}

_inline void Vec3MultAssign(vec3_t value, vec3_t multBy)
{
	assert(multBy != vec3_origin);

	multBy[0] *= value[0];
	multBy[1] *= value[1];
	multBy[2] *= value[2];
}

_inline void Vec3ScaleAssign(vec_t value, vec3_t scaleBy)
{
	assert(scaleBy != vec3_origin);

	scaleBy[0] *= value;
	scaleBy[1] *= value;
	scaleBy[2] *= value;
}

_inline qboolean FloatIsZeroEpsilon(float f)
{
	return (Q_fabs(f) < FLOAT_ZERO_EPSILON);
}

_inline qboolean FloatIsZero(float f, float epsilon)
{
	return (Q_fabs(f) < epsilon);
}

_inline qboolean Vec3EqualsEpsilon(vec3_t v1, vec3_t v2)
{
	if(!FloatIsZeroEpsilon(v1[0] - v2[0]) || !FloatIsZeroEpsilon(v1[1] - v2[1]) || !FloatIsZeroEpsilon(v1[2] - v2[2]))
	{
		return false;
	}
			
	return true;
}

_inline qboolean Vec3IsZero(vec3_t vec)
{
	return !( vec[0] != 0.0 || vec[1] != 0.0 || vec[2] != 0.0 );
}

_inline qboolean Vec3NotZero(vec3_t vec)
{
	return ( vec[0] != 0.0 || vec[1] != 0.0 || vec[2] != 0.0 );
}

#endif

#endif