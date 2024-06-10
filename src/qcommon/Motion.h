//
// Motion.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "H2Common.h"
#include "q_Typedef.h"

H2COMMON_API float GetTimeToReachDistance(float vel, float accel, float dist);
H2COMMON_API float GetDistanceOverTime(float vel, float accel, float time);
H2COMMON_API void GetPositionOverTime(const vec3_t origin, const vec3_t vel, const vec3_t accel, float time, vec3_t out);
H2COMMON_API void GetVelocityOverTime(const vec3_t vel, const vec3_t accel, float time, vec3_t out);