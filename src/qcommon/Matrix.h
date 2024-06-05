#include "H2Common.h"
#include "q_Typedef.h"

H2COMMON_API void CreateRollMatrix(matrix3_t, vec_t);
H2COMMON_API void CreateYawMatrix(matrix3_t, vec_t);
H2COMMON_API void CreatePitchMatrix(matrix3_t, vec_t);

H2COMMON_API void R_ConcatTransforms (float in1[3][4], float in2[3][4], float out[3][4]);

H2COMMON_API void Matrix3MultByMatrix3(matrix3_t A, matrix3_t B, matrix3_t C);
H2COMMON_API void Matrix3MultByVec3(matrix3_t A, vec3_t B, vec3_t C);
H2COMMON_API void Matrix3FromAngles(vec3_t angles, matrix3_t rotationMatrix);
H2COMMON_API void IMatrix3FromAngles(vec3_t angles, matrix3_t rotationMatrix);
H2COMMON_API void Matrixs3FromDirAndUp(vec3_t direction, vec3_t up, matrix3_t toLocal, matrix3_t fromLocal);
H2COMMON_API double Matricies3FromDirAndUp(vec3_t direction, vec3_t up, matrix3_t toWorld, matrix3_t partialToLocal);
H2COMMON_API void RotatePointAboutLocalOrigin(matrix3_t rotation, vec3_t origin, vec3_t point);
H2COMMON_API void TransformPoint(matrix3_t rotation, vec3_t origin, vec3_t newOrigin, vec3_t point);

#define HACK_PITCH_FOR_REFS	// eliminates a problem with ref points when the player is bent backwards;
							// placed in the header to avoid inconsistencies between projects