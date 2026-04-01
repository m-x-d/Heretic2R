//
// q_Physics.c
//
// Copyright 1998 Raven Software
//

#include "q_Physics.h"
#include "Vector.h"

H2COMMON_API void BounceVelocity(const vec3_t in, const vec3_t normal, vec3_t out, const float elasticity)
{
	const float len = DotProduct(in, normal) * elasticity;
	VectorMA(in, -len, normal, out);
}

//mxd. Commonly used vector reflection logic (https://3dkingdoms.com/weekly/weekly.php?a=2).
//mxd. BounceVelocity() used in original logic occasionally resulted in rapid velocity growth when used in Debris_Collision().
H2COMMON_API void ReflectVelocity(const vec3_t in, const vec3_t normal, vec3_t out, const float elasticity)
{
	const float dot = DotProduct(in, normal);

	for (int i = 0; i < 3; i++)
		out[i] = (elasticity * 0.5f) * (-2.0f * dot * normal[i] + in[i]); //mxd. Scale elasticity by 0.5 to compensate for -2.0 scaler...
}

H2COMMON_API qboolean BoundVelocity(vec3_t vel)
{
	int zero_axes = 0;

	for (int i = 0; i < 3; i++)
	{
		if (vel[i] > -0.1f && vel[i] < 0.1f)
		{
			vel[i] = 0.0f;
			zero_axes += 1;
		}
		else
		{
			vel[i] = Clamp(vel[i], -2000.0f, 2000.0f);
		}
	}

	return zero_axes != 3;
}