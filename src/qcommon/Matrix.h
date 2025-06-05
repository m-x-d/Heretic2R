//
// Matrix.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "H2Common.h"
#include "q_Typedef.h"

extern H2COMMON_API qboolean HACK_Pitch_Adjust;

H2COMMON_API extern void CreateRollMatrix(matrix3_t m, float roll);
H2COMMON_API extern void CreateYawMatrix(matrix3_t m, float yaw);
H2COMMON_API extern void CreatePitchMatrix(matrix3_t m, float pitch);

H2COMMON_API extern void R_ConcatTransforms(const float in1[3][4], const float in2[3][4], float out[3][4]);

H2COMMON_API extern void Matrix3MultByMatrix3(const matrix3_t a, const matrix3_t b, matrix3_t out);
H2COMMON_API extern void Matrix3MultByVec3(const matrix3_t a, const vec3_t b, vec3_t out);

H2COMMON_API extern void Matrix3FromAngles(const vec3_t angles, matrix3_t rotation);
H2COMMON_API extern void IMatrix3FromAngles(const vec3_t angles, matrix3_t rotation);

H2COMMON_API extern double Matricies3FromDirAndUp(const vec3_t direction, const vec3_t up, matrix3_t to_world, matrix3_t partial_to_local); //mxd. Typo in function name. Can't rename, exported function...

H2COMMON_API extern void RotatePointAboutLocalOrigin(const matrix3_t rotation, const vec3_t origin, vec3_t point);
H2COMMON_API extern void TransformPoint(const matrix3_t rotation, const vec3_t origin, const vec3_t newOrigin, vec3_t point);