//
// Motion.c
//
// Copyright 1998 Raven Software
//

#include <math.h>
#include <stdlib.h>
#include "Motion.h"

H2COMMON_API float GetTimeToReachDistance(const float vel, const float accel, const float dist)
{
    if (accel == 0.0f)
    {
        if (vel != 0.0f)
            return dist / vel * 1000.0f;

        return 0.0f;
    }

    const float l = sqrtf(accel * dist + accel * dist + vel * vel);

    const float time1 = (l - vel)  * 1000.0f / accel;
    const float time2 = (-vel - l) * 1000.0f / accel;

    return max(time1, time2);
}

H2COMMON_API float GetDistanceOverTime(const float vel, const float accel, const float time)
{
    return vel * time + accel * time * time * 0.5f;
}

H2COMMON_API void GetPositionOverTime(const vec3_t origin, const vec3_t vel, const vec3_t accel, const float time, vec3_t out)
{
    for (int i = 0; i < 3; i++)
        out[i] = origin[i] + GetDistanceOverTime(vel[i], accel[i], time);
}

H2COMMON_API void GetVelocityOverTime(const vec3_t vel, const vec3_t accel, const float time, vec3_t out)
{
	for (int i = 0; i < 3; i++)
        out[i] = time * accel[i] + vel[i];
}