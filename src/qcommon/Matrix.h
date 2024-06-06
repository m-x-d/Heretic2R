//
// Matrix.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "H2Common.h"
#include "q_Typedef.h"

H2COMMON_API void CreateRollMatrix(matrix3_t, float);
H2COMMON_API void CreateYawMatrix(matrix3_t, float);
H2COMMON_API void CreatePitchMatrix(matrix3_t, float);

H2COMMON_API void R_ConcatTransforms(const float in1[3][4], const float in2[3][4], float out[3][4]);

H2COMMON_API void Matrix3MultByMatrix3(const matrix3_t a, const matrix3_t b, matrix3_t out);
H2COMMON_API void Matrix3MultByVec3(const matrix3_t a, const vec3_t b, vec3_t out);
H2COMMON_API void Matrix3FromAngles(const vec3_t angles, matrix3_t rotationMatrix);
H2COMMON_API void IMatrix3FromAngles(const vec3_t angles, matrix3_t rotationMatrix);
//H2COMMON_API void Matrixs3FromDirAndUp(const vec3_t direction, const vec3_t up, matrix3_t toLocal, matrix3_t fromLocal); //mxd. Missing function definition
H2COMMON_API double Matricies3FromDirAndUp(const vec3_t direction, const vec3_t up, matrix3_t toWorld, matrix3_t partialToLocal); //TODO: typo in function name. Rename?
H2COMMON_API void RotatePointAboutLocalOrigin(const matrix3_t rotation, const vec3_t origin, vec3_t point);
H2COMMON_API void TransformPoint(const matrix3_t rotation, const vec3_t origin, const vec3_t newOrigin, vec3_t point);

// Eliminates a problem with ref points when the player is bent backwards.
// Placed in the header to avoid inconsistencies between projects.
//#define HACK_PITCH_FOR_REFS	//mxd. Never used in Heretic II Toolkit v1.06 sources. Probably used by ref_gl.dll or H2Common.dll (but we'll never know...)