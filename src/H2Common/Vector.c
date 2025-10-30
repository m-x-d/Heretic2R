//
// Vector.c
//
// Copyright 1998 Raven Software
//

#include <assert.h>
#include <string.h>

#include "Vector.h"
#include "Matrix.h"
#include "Random.h"

H2COMMON_API const vec3_t vec3_origin =	{  0.0f, 0.0f,  0.0f }; // [0, 0, 0]
H2COMMON_API const vec3_t vec3_left =	{ -1.0f, 0.0f,  0.0f }; // [-1, 0, 0] //mxd. Not exported in original .dll.
H2COMMON_API const vec3_t vec3_right =	{  1.0f, 0.0f,  0.0f }; // [1, 0, 0] //mxd. Not exported in original .dll.
H2COMMON_API const vec3_t vec3_up =		{  0.0f, 0.0f,  1.0f }; // [0, 0, 1]
H2COMMON_API const vec3_t vec3_down =	{  0.0f, 0.0f, -1.0f }; // [0, 0, -1] //mxd. Not exported in original .dll.

//mxd. Function logic does NOT match Q2 counterpart
H2COMMON_API void RotatePointAroundVector(vec3_t dst, const vec3_t dir, const vec3_t point, const float degrees)
{
	vec3_t vf, vr, vup;

	VectorCopy(dir, vf);
	PerpendicularVector(vr, dir);
	CrossProduct(vr, vf, vup);
	VectorNormalize(vup);

	const float x = DotProduct(point, vr);
	const float y = DotProduct(point, vup);
	const float z = DotProduct(point, vf);

	const float a_cos = cosf(degrees * ANGLE_TO_RAD);
	const float a_sin = sinf(degrees * ANGLE_TO_RAD);

	VectorScale(vr,  a_cos * x - a_sin * y, vr);
	VectorScale(vup, a_cos * y + a_sin * x, vup);
	VectorScale(vf, z, vf);

	dst[0] = vf[0] + vr[0] + vup[0];
	dst[1] = vf[1] + vr[1] + vup[1];
	dst[2] = vf[2] + vr[2] + vup[2];
}

// Q2 counterpart
H2COMMON_API void ProjectPointOnPlane(vec3_t dst, const vec3_t p, const vec3_t normal)
{
	const float inv_denom = 1.0f / VectorLengthSquared(normal);
	const float d = DotProduct(normal, p) * inv_denom;

	vec3_t n;
	VectorScale(normal, inv_denom, n);
	VectorScale(n, d, n);
	VectorSubtract(p, n, dst);
}

// Q2 counterpart. Assumes "src" is normalized
H2COMMON_API void PerpendicularVector(vec3_t dst, const vec3_t src)
{
	int	pos, i;
	float minelem = 1.0f;
	
	// Find the smallest magnitude axially aligned vector
	for (pos = 0, i = 0; i < 3; i++)
	{
		if (fabsf(src[i]) < minelem)
		{
			minelem = fabsf(src[i]);
			pos = i;
		}
	}

	vec3_t tempvec;
	VectorClear(tempvec);
	tempvec[pos] = 1.0f;

	// Project the point onto the plane defined by src
	ProjectPointOnPlane(dst, tempvec, src);

	// Normalize the result
	VectorNormalize(dst);
}

// Q2 counterpart
H2COMMON_API void AngleVectors(const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up)
{
	float angle = angles[YAW] * ANGLE_TO_RAD;
	const float sy = sinf(angle);
	const float cy = cosf(angle);

	angle = angles[PITCH] * ANGLE_TO_RAD;
	const float sp = sinf(angle);
	const float cp = cosf(angle);

	angle = angles[ROLL] * ANGLE_TO_RAD;
	const float sr = sinf(angle);
	const float cr = cosf(angle);

	if (forward)
	{
		forward[0] = cp * cy;
		forward[1] = cp * sy;
		forward[2] = -sp;
	}

	if (right)
	{
		right[0] = -1.0f * sr * sp * cy + -1.0f * cr * -sy;
		right[1] = -1.0f * sr * sp * sy + -1.0f * cr * cy;
		right[2] = -1.0f * sr * cp;
	}

	if (up)
	{
		up[0] = cr * sp * cy + -sr * -sy;
		up[1] = cr * sp * sy + -sr * cy;
		up[2] = cr * cp;
	}
}

//mxd. Same as AngleVectors, but without DEG_TO_RAD logic...
H2COMMON_API void RealAngleVectors(const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up)
{
	const float sy = sinf(angles[YAW]);
	const float cy = cosf(angles[YAW]);

	const float sp = sinf(angles[PITCH]);
	const float cp = cosf(angles[PITCH]);

	const float sr = sinf(angles[ROLL]);
	const float cr = cosf(angles[ROLL]);

	if (forward)
	{
		forward[0] = cp * cy;
		forward[1] = cp * sy;
		forward[2] = -sp;
	}

	if (right)
	{
		right[0] = -1.0f * sr * sp * cy + -1.0f * cr * -sy;
		right[1] = -1.0f * sr * sp * sy + -1.0f * cr * cy;
		right[2] = -1.0f * sr * cp;
	}

	if (up)
	{
		up[0] = cr * sp * cy + -sr * -sy;
		up[1] = cr * sp * sy + -sr * cy;
		up[2] = cr * cp;
	}
}

H2COMMON_API void DirFromAngles(const vec3_t angles, vec3_t direction)
{
	const float cy = cosf(angles[YAW]);
	const float sy = sinf(angles[YAW]);

	const float cp = cosf(angles[PITCH]);
	const float sp = sinf(angles[PITCH]);

	VectorSet(direction, cy * cp, sy * cp, sp);
}

H2COMMON_API void DirAndUpFromAngles(const vec3_t angles, vec3_t direction, vec3_t up)
{
	matrix3_t m;

	Matrix3FromAngles(angles, m);
	Matrix3MultByVec3(m, vec3_right, direction);
	Matrix3MultByVec3(m, vec3_up, up);
}

//mxd. Assumes "direction" is normalized
H2COMMON_API void AnglesFromDir(const vec3_t direction, vec3_t angles)
{
	angles[PITCH] = asinf(direction[ROLL]); //mxd. asinf expects value in [-1.0; 1.0] range.
	angles[YAW] = atan2f(direction[YAW], direction[PITCH]);
	angles[ROLL] = 0.0f;

	assert(!isnan(angles[PITCH])); //mxd
}

//mxd. Assumes "direction" is normalized
H2COMMON_API void AnglesFromDirI(const vec3_t direction, vec3_t angles)
{
	angles[PITCH] = asinf(direction[ROLL]); //mxd. asinf expects value in [-1.0; 1.0] range.
	angles[YAW] = atan2f(direction[YAW], direction[PITCH]);
	angles[ROLL] = -angles[YAW];

	assert(!isnan(angles[PITCH])); //mxd
}

//mxd. Function logic does NOT match Q2 counterpart
H2COMMON_API void vectoangles(const vec3_t in, vec3_t out)
{
	if (Vec3IsZero(in))
	{
		VectorClear(out);
		return;
	}

	vec3_t v;
	VectorNormalize2(in, v);
	AnglesFromDir(v, out);

	Vec3ScaleAssign(RAD_TO_ANGLE, out);
}

//mxd. Assumes "direction" is normalized
H2COMMON_API void AnglesFromDirAndUp(vec3_t direction, vec3_t up, vec3_t angles)
{
	vec3_t v_dir, v_up;
	matrix3_t m1, m2, m3;

	angles[0] = asinf(direction[2]); //mxd. asinf expects value in [-1.0; 1.0] range.

	assert(!isnan(angles[0])); //mxd

	memset(m1, 0, sizeof(matrix3_t));
	m1[2][0] = direction[2];
	m1[1][1] = 1.0f;
	m1[0][0] = cosf(angles[0]);
	m1[2][2] = m1[0][0];
	m1[0][2] = -m1[2][0];

	Matrix3MultByVec3(m1, direction, v_dir);

	angles[1] = atan2f(v_dir[1], v_dir[0]);

	memset(m2, 0, sizeof(matrix3_t));
	m2[2][2] = 1.0f;
	m2[0][0] = cosf(angles[1]);
	m2[1][1] = m2[0][0];
	m2[0][1] = sinf(angles[1]);
	m2[1][0] = -sinf(angles[1]);

	Matrix3MultByMatrix3(m1, m2, m3);
	Matrix3MultByVec3(m3, up, v_up);
	v_up[0] = 0.0f;
	VectorNormalize(v_up);

	angles[2] = -(atan2f(v_up[2], v_up[1]) - ANGLE_90);
}

H2COMMON_API qboolean VectorCompare(const vec3_t v1, const vec3_t v2)
{
	return (v1[0] == v2[0] && v1[1] == v2[1] && v1[2] == v2[2]);
}

H2COMMON_API float VectorNormalize(vec3_t v)
{
	if (Vec3IsZeroEpsilon(v))
	{
		VectorClear(v);
		return 0.0f;
	}

	const float length = VectorLength(v);
	VectorScale(v, 1.0f / length, v);

	return length;
}

// No longer used. Can't remove (will break vanilla compatibility)... -- mxd
H2COMMON_API float Vec3Normalize(vec3_t v)
{
	return VectorNormalize(v);
}

H2COMMON_API float VectorNormalize2(const vec3_t v, vec3_t out)
{
	if (Vec3IsZeroEpsilon(v))
	{
		VectorClear(out);
		return 0.0f;
	}

	const float length = VectorLength(v);
	VectorScale(v, 1.0f / length, out);

	return length;
}

H2COMMON_API void VectorClamp(vec3_t v, const float max_length) //mxd
{
	if (Vec3IsZeroEpsilon(v) || FloatIsZeroEpsilon(max_length))
		return;

	const float length = VectorLength(v);

	if (length > max_length)
		Vec3ScaleAssign(max_length / length, v);
}

// out = veca + vecb * scale;
H2COMMON_API void VectorMA(const vec3_t veca, const float scale, const vec3_t vecb, vec3_t out)
{
	assert(out != vec3_origin);

	out[0] = veca[0] + scale * vecb[0];
	out[1] = veca[1] + scale * vecb[1];
	out[2] = veca[2] + scale * vecb[2];
}

// out = (veca + vecb) * 0.5;
H2COMMON_API void VectorAverage(const vec3_t veca, const vec3_t vecb, vec3_t out)
{
	assert(out != vec3_origin);

	out[0] = (veca[0] + vecb[0]) * 0.5f;
	out[1] = (veca[1] + vecb[1]) * 0.5f;
	out[2] = (veca[2] + vecb[2]) * 0.5f;
}

// out = veca + frac * (vecb - veca);
H2COMMON_API void VectorLerp(const vec3_t veca, const float frac, const vec3_t vecb, vec3_t out) //mxd
{
	assert(out != vec3_origin);

	out[0] = veca[0] + frac * (vecb[0] - veca[0]);
	out[1] = veca[1] + frac * (vecb[1] - veca[1]);
	out[2] = veca[2] + frac * (vecb[2] - veca[2]);
}

H2COMMON_API void VectorGetOffsetOrigin(const vec3_t offset, const vec3_t origin, const float angle_deg, vec3_t out)
{
	matrix3_t m_yaw;

	CreateYawMatrix(m_yaw, angle_deg * ANGLE_TO_RAD);
	Matrix3MultByVec3(m_yaw, offset, out);
	VectorAdd(out, origin, out);
}

H2COMMON_API float VectorSeparation(const vec3_t v1, const vec3_t v2)
{
	vec3_t diff;
	VectorSubtract(v1, v2, diff);

	return VectorLength(diff);
}

H2COMMON_API void VectorRandomCopy(const vec3_t in, vec3_t out, const float random_amount)
{
	for (int i = 0; i < 3; i++)
		out[i] = in[i] + flrand(-random_amount, random_amount);
}

// Q2 counterpart
H2COMMON_API void CrossProduct(const vec3_t v1, const vec3_t v2, vec3_t cross)
{
	cross[0] = v1[1] * v2[2] - v1[2] * v2[1];
	cross[1] = v1[2] * v2[0] - v1[0] * v2[2];
	cross[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

H2COMMON_API float VectorLength(const vec3_t v)
{
	return sqrtf(DotProduct(v, v));
}

H2COMMON_API float VectorLengthSquared(const vec3_t v)
{
	return DotProduct(v, v);
}

H2COMMON_API void VectorRandomAdd(const vec3_t origin, const vec3_t random_amount, vec3_t out)
{
	assert(out != vec3_origin);
	
	for (int i = 0; i < 3; i++)
		out[i] = origin[i] + flrand(-random_amount[i], random_amount[i]);
}

H2COMMON_API void VectorSubtract(const vec3_t veca, const vec3_t vecb, vec3_t out)
{
	assert(out != vec3_origin);

	out[0] = veca[0] - vecb[0];
	out[1] = veca[1] - vecb[1];
	out[2] = veca[2] - vecb[2];
}

H2COMMON_API void VectorAdd(const vec3_t veca, const vec3_t vecb, vec3_t out)
{
	assert(out != vec3_origin);

	out[0] = veca[0] + vecb[0];
	out[1] = veca[1] + vecb[1];
	out[2] = veca[2] + vecb[2];
}

// Returns length of XY component.
H2COMMON_API float vhlen(const vec3_t v1, const vec3_t v2)
{
	const float dx = v1[0] - v2[0];
	const float dy = v1[1] - v2[1];

	return sqrtf(dx * dx + dy * dy);
}

//TODO: Typo? "relect" -> "reflect"?
//TODO: Modifies the in vector! Unintentional?
H2COMMON_API void Create_rand_relect_vect(vec3_t in, vec3_t out)
{
	vec3_t angles;

	if (VectorNormalize(in) < FLOAT_ZERO_EPSILON)
	{
		VectorSet(out, 1.0f, 0.0f, 0.0f);
		return;
	}

	VectorInverse(in);
	AnglesFromDir(in, angles);

	angles[0] += flrand(-0.3f, 0.3f);
	angles[1] += flrand(-0.5f, 0.5f);

	DirFromAngles(angles, out);
}

H2COMMON_API qboolean Vec3IsZeroEpsilon(const vec3_t v)
{
	return (FloatIsZeroEpsilon(v[0]) && FloatIsZeroEpsilon(v[1]) && FloatIsZeroEpsilon(v[2]));
}

//mxd. Functions below were originally inlined in Vector.h

H2COMMON_API void VectorAbs(const vec3_t in, vec3_t out)
{
	assert(out != vec3_origin);

	out[0] = fabsf(in[0]);
	out[1] = fabsf(in[1]);
	out[2] = fabsf(in[2]);
}

H2COMMON_API void VectorRound(vec3_t v)
{
	assert(v != vec3_origin);

	v[0] = floorf(v[0] + 0.5f);
	v[1] = floorf(v[1] + 0.5f);
	v[2] = floorf(v[2] + 0.5f);
}

H2COMMON_API float DotProduct(const vec3_t v1, const vec3_t v2)
{
	return (v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2]);
}

H2COMMON_API void VectorDec(vec3_t v)
{
	assert(v != vec3_origin);

	v[0] -= 1.0f;
	v[1] -= 1.0f;
	v[2] -= 1.0f;
}

H2COMMON_API void VectorInc(vec3_t v)
{
	assert(v != vec3_origin);

	v[0] += 1.0f;
	v[1] += 1.0f;
	v[2] += 1.0f;
}

H2COMMON_API void VectorClear(vec3_t v)
{
	assert(v != vec3_origin);

	v[0] = 0.0f;
	v[1] = 0.0f;
	v[2] = 0.0f;
}

H2COMMON_API void VectorSet(vec3_t v, const float x, const float y, const float z)
{
	assert(v != vec3_origin);

	v[0] = x;
	v[1] = y;
	v[2] = z;
}

H2COMMON_API void VectorRandomSet(vec3_t v, const float rand_val) //mxd
{
	assert(v != vec3_origin);

	v[0] = flrand(-rand_val, rand_val);
	v[1] = flrand(-rand_val, rand_val);
	v[2] = flrand(-rand_val, rand_val);
}

H2COMMON_API void VectorCopy(const vec3_t in, vec3_t out)
{
	assert(out != vec3_origin);

	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
}

H2COMMON_API void VectorInverse(vec3_t v)
{
	assert(v != vec3_origin);

	v[0] = -v[0];
	v[1] = -v[1];
	v[2] = -v[2];
}

H2COMMON_API void VectorNegate(const vec3_t in, vec3_t out)
{
	assert(out != vec3_origin);

	out[0] = -in[0];
	out[1] = -in[1];
	out[2] = -in[2];
}

// out = in * scale;
H2COMMON_API void VectorScale(const vec3_t in, const float scale, vec3_t out)
{
	assert(out != vec3_origin);

	out[0] = in[0] * scale;
	out[1] = in[1] * scale;
	out[2] = in[2] * scale;
}

H2COMMON_API void VectorRadiansToDegrees(const vec3_t in, vec3_t out)
{
	assert(out != vec3_origin);

	out[0] = in[0] * RAD_TO_ANGLE;
	out[1] = in[1] * RAD_TO_ANGLE;
	out[2] = in[2] * RAD_TO_ANGLE;
}

H2COMMON_API void VectorDegreesToRadians(const vec3_t in, vec3_t out)
{
	assert(out != vec3_origin);

	out[0] = in[0] * ANGLE_TO_RAD;
	out[1] = in[1] * ANGLE_TO_RAD;
	out[2] = in[2] * ANGLE_TO_RAD;
}

H2COMMON_API void VectorScaleByVector(const vec3_t in, const vec3_t scale, vec3_t out)
{
	assert(out != vec3_origin);

	out[0] = in[0] * scale[0];
	out[1] = in[1] * scale[1];
	out[2] = in[2] * scale[2];
}

H2COMMON_API float VectorSeparationSquared(const vec3_t va, const vec3_t vb)
{
	vec3_t work;

	VectorSubtract(va, vb, work);
	return DotProduct(work, work);
}

H2COMMON_API void Vec3SubtractAssign(const vec3_t value, vec3_t subFrom)
{
	assert(subFrom != vec3_origin);

	subFrom[0] -= value[0];
	subFrom[1] -= value[1];
	subFrom[2] -= value[2];
}

H2COMMON_API void Vec3AddAssign(const vec3_t value, vec3_t addTo)
{
	assert(addTo != vec3_origin);

	addTo[0] += value[0];
	addTo[1] += value[1];
	addTo[2] += value[2];
}

H2COMMON_API void Vec3MultAssign(const vec3_t value, vec3_t multBy)
{
	assert(multBy != vec3_origin);

	multBy[0] *= value[0];
	multBy[1] *= value[1];
	multBy[2] *= value[2];
}

// scaleBy *= value;
H2COMMON_API void Vec3ScaleAssign(const float value, vec3_t scaleBy)
{
	assert(scaleBy != vec3_origin);

	scaleBy[0] *= value;
	scaleBy[1] *= value;
	scaleBy[2] *= value;
}

H2COMMON_API qboolean FloatIsZeroEpsilon(const float f)
{
	return (fabsf(f) < FLOAT_ZERO_EPSILON);
}

H2COMMON_API qboolean FloatIsZero(const float f, const float epsilon)
{
	return (fabsf(f) < epsilon);
}

H2COMMON_API qboolean Vec3EqualsEpsilon(const vec3_t v1, const vec3_t v2)
{
	return (FloatIsZeroEpsilon(v1[0] - v2[0]) && FloatIsZeroEpsilon(v1[1] - v2[1]) && FloatIsZeroEpsilon(v1[2] - v2[2]));
}

H2COMMON_API qboolean Vec3IsZero(const vec3_t vec)
{
	return (vec[0] == 0.0f && vec[1] == 0.0f && vec[2] == 0.0f);
}

H2COMMON_API qboolean Vec3NotZero(const vec3_t vec)
{
	return (vec[0] != 0.0f || vec[1] != 0.0f || vec[2] != 0.0f);
}