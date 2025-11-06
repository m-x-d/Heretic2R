//
// Math.c
//
// Copyright 1998 Raven Software
//

#include "q_shared.h"
#include "Vector.h"

// Q2 counterpart. Expects a positive value.
H2COMMON_API int Q_log2(int val)
{
	int answer = 0;

	assert(val > -1); //mxd
	
	while (val >>= 1)
		answer++;

	return answer;
}

// Q2 counterpart
H2COMMON_API void ClearBounds(vec3_t mins, vec3_t maxs)
{
	mins[0] = mins[1] = mins[2] = 99999.0f;
	maxs[0] = maxs[1] = maxs[2] = -99999.0f;
}

// Q2 counterpart
H2COMMON_API void AddPointToBounds(const vec3_t v, vec3_t mins, vec3_t maxs)
{
	for (int i = 0; i < 3; i++)
	{
		const float val = v[i];

		if (val < mins[i])
			mins[i] = val;

		if (val > maxs[i])
			maxs[i] = val;
	}
}

// Q2 counterpart
H2COMMON_API float anglemod(const float a)
{
	return (360.0f / 65536) * (float)((int)(a * (65536 / 360.0f)) & 65535);
}

// Q2 counterpart, #if 0 version
H2COMMON_API float anglemod_old(const float a)
{
	if (a >= 0.0f)
		return a - 360.0f * (int)(a / 360.0f);

	return a + 360.0f * (int)(-a / 360.0f) + 360.0f; //mxd. Q2 logic: 360 * (1 + (int)(-a / 360));
}

H2COMMON_API float LerpFloat(const float f1, const float f2, const float frac) //mxd
{
	return f1 + frac * (f2 - f1);
}

//mxd. Inverse of Q2 logic!
H2COMMON_API float LerpAngle(const float a1, float a2, const float frac)
{
	if (a2 - a1 > 180.0f)
		a2 -= 360.0f;

	if (a2 - a1 < -180.0f)
		a2 += 360.0f;

	return LerpFloat(a1, a2, frac);
}

//mxd. Angles are expected to be in degrees.
H2COMMON_API void LerpAngles(const vec3_t angle_a, const float frac, const vec3_t angle_b, vec3_t out)
{
	assert(out != vec3_origin);

	out[0] = LerpAngle(angle_a[0], angle_b[0], frac);
	out[1] = LerpAngle(angle_a[1], angle_b[1], frac);
	out[2] = LerpAngle(angle_a[2], angle_b[2], frac);
}

H2COMMON_API float Clamp(const float src, const float val_min, const float val_max)
{
	return min(max(src, val_min), val_max);
}

H2COMMON_API int ClampI(const int src, const int val_min, const int val_max)
{
	return min(max(src, val_min), val_max);
}

H2COMMON_API float Approach(const float curr, const float dest, const float rate)
{
	const float diff = dest - curr;

	if (fabsf(diff) <= rate)
		return curr;

	if (diff < 0.0f)
		return curr - rate;

	if (diff > 0.0f)
		return curr + rate;

	return curr;
}

// Q2 counterpart //TODO: not included in H2 toolkit headers. Included where?
// This is the slow, general version
H2COMMON_API int BoxOnPlaneSide2(const vec3_t emins, const vec3_t emaxs, const struct cplane_s* plane)
{
	vec3_t corners[2];

	for (int i = 0; i < 3; i++)
	{
		if (plane->normal[i] < 0)
		{
			corners[0][i] = emins[i];
			corners[1][i] = emaxs[i];
		}
		else
		{
			corners[1][i] = emins[i];
			corners[0][i] = emaxs[i];
		}
	}

	const float dist1 = DotProduct(plane->normal, corners[0]) - plane->dist;
	const float dist2 = DotProduct(plane->normal, corners[1]) - plane->dist;
	int sides = 0;

	if (dist1 >= 0)
		sides = 1;

	if (dist2 < 0)
		sides |= 2;

	return sides;
}

//mxd
H2COMMON_API int Q_sign(const int val)
{
	return val < 0 ? -1 : 1;
}

//mxd
H2COMMON_API float Q_signf(const float val)
{
	return val < 0.0f ? -1.0f : 1.0f;
}