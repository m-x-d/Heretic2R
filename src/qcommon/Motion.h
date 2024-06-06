#include "H2Common.h"
#include "q_typedef.h"

H2COMMON_API float GetTimeToReachDistance(float, float, float);
H2COMMON_API float GetDistanceOverTime(float, float, float);
H2COMMON_API void GetPositionOverTime(vec3_t, vec3_t, vec3_t, float, vec3_t);
H2COMMON_API void GetVelocityOverTime(vec3_t velocity, vec3_t accel, float time, vec3_t output);

// end