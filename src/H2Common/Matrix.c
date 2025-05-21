//
// Matrix.c
//
// Copyright 1998 Raven Software
//

#include <math.h>
#include <string.h>
#include "Matrix.h"
#include "Vector.h"

H2COMMON_API qboolean HACK_Pitch_Adjust = false;

// Rotation around the x axis
H2COMMON_API void CreateRollMatrix(matrix3_t m, const float roll)
{
	memset(m, 0, sizeof(matrix3_t));

	m[0][0] = 1.0f;

	m[1][1] = cosf(roll);
	m[2][2] = m[1][1];

	m[1][2] = sinf(roll);
	m[2][1] = -sinf(roll);
}

// Rotation around the z axis
H2COMMON_API void CreateYawMatrix(matrix3_t m, const float yaw)
{
	memset(m, 0, sizeof(matrix3_t));

	m[0][0] = cosf(yaw);
	m[1][1] = m[0][0];

	m[0][1] = sinf(yaw);
	m[1][0] = -sinf(yaw);

	m[2][2] = 1.0f;
}

// Rotation around the y axis
H2COMMON_API void CreatePitchMatrix(matrix3_t m, const float pitch)
{
	memset(m, 0, sizeof(matrix3_t));

	m[0][0] = cosf(-pitch);
	m[2][2] = m[0][0];

	m[1][1] = 1.0f;

	m[2][0] = sinf(-pitch);
	m[0][2] = sinf(pitch);
}

// Q2 counterpart (defined in q_shared.c)
H2COMMON_API void R_ConcatTransforms(const float in1[3][4], const float in2[3][4], float out[3][4])
{
	out[0][0] = in1[0][0] * in2[0][0] + in1[0][1] * in2[1][0] +	in1[0][2] * in2[2][0];
	out[0][1] = in1[0][0] * in2[0][1] + in1[0][1] * in2[1][1] +	in1[0][2] * in2[2][1];
	out[0][2] = in1[0][0] * in2[0][2] + in1[0][1] * in2[1][2] +	in1[0][2] * in2[2][2];
	out[0][3] = in1[0][0] * in2[0][3] + in1[0][1] * in2[1][3] +	in1[0][2] * in2[2][3] + in1[0][3];

	out[1][0] = in1[1][0] * in2[0][0] + in1[1][1] * in2[1][0] +	in1[1][2] * in2[2][0];
	out[1][1] = in1[1][0] * in2[0][1] + in1[1][1] * in2[1][1] +	in1[1][2] * in2[2][1];
	out[1][2] = in1[1][0] * in2[0][2] + in1[1][1] * in2[1][2] +	in1[1][2] * in2[2][2];
	out[1][3] = in1[1][0] * in2[0][3] + in1[1][1] * in2[1][3] +	in1[1][2] * in2[2][3] + in1[1][3];

	out[2][0] = in1[2][0] * in2[0][0] + in1[2][1] * in2[1][0] +	in1[2][2] * in2[2][0];
	out[2][1] = in1[2][0] * in2[0][1] + in1[2][1] * in2[1][1] +	in1[2][2] * in2[2][1];
	out[2][2] = in1[2][0] * in2[0][2] + in1[2][1] * in2[1][2] +	in1[2][2] * in2[2][2];
	out[2][3] = in1[2][0] * in2[0][3] + in1[2][1] * in2[1][3] +	in1[2][2] * in2[2][3] + in1[2][3];
}

H2COMMON_API void Matrix3MultByMatrix3(const matrix3_t a, const matrix3_t b, matrix3_t out)
{
	out[0][0] = a[0][0] * b[0][0] + a[1][0] * b[0][1] + a[2][0] * b[0][2];
	out[0][1] = a[0][1] * b[0][0] + a[1][1] * b[0][1] + a[2][1] * b[0][2];
	out[0][2] = a[0][2] * b[0][0] + a[1][2] * b[0][1] + a[2][2] * b[0][2];

	out[1][0] = a[0][0] * b[1][0] + a[1][0] * b[1][1] + a[2][0] * b[1][2];
	out[1][1] = a[0][1] * b[1][0] + a[1][1] * b[1][1] + a[2][1] * b[1][2];
	out[1][2] = a[0][2] * b[1][0] + a[1][2] * b[1][1] + a[2][2] * b[1][2];

	out[2][0] = a[0][0] * b[2][0] + a[1][0] * b[2][1] + a[2][0] * b[2][2];
	out[2][1] = a[0][1] * b[2][0] + a[1][1] * b[2][1] + a[2][1] * b[2][2];
	out[2][2] = a[0][2] * b[2][0] + a[1][2] * b[2][1] + a[2][2] * b[2][2];
}

H2COMMON_API void Matrix3MultByVec3(const matrix3_t a, const vec3_t b, vec3_t out)
{
	out[0] = a[0][0] * b[0] + a[1][0] * b[1] + a[2][0] * b[2];
	out[1] = a[0][1] * b[0] + a[1][1] * b[1] + a[2][1] * b[2];
	out[2] = a[0][2] * b[0] + a[1][2] * b[1] + a[2][2] * b[2];
}

H2COMMON_API void Matrix3FromAngles(const vec3_t angles, matrix3_t rotationMatrix)
{
	matrix3_t m_pitch, m_yaw, m_roll, m_tmp;

	CreatePitchMatrix(m_pitch, angles[PITCH]);
	CreateYawMatrix(m_yaw, angles[YAW]);
	CreateRollMatrix(m_roll, angles[ROLL]);

	Matrix3MultByMatrix3(m_pitch, m_roll, m_tmp);
	Matrix3MultByMatrix3(m_yaw, m_tmp, rotationMatrix);
}

// Creates inverse matrix
H2COMMON_API void IMatrix3FromAngles(const vec3_t angles, matrix3_t rotationMatrix)
{
	matrix3_t m_pitch, m_yaw, m_roll, m_tmp;

	CreatePitchMatrix(m_pitch, -angles[PITCH]);
	CreateYawMatrix(m_yaw, -angles[YAW]);
	CreateRollMatrix(m_roll, -angles[ROLL]);

	Matrix3MultByMatrix3(m_pitch, m_yaw, m_tmp);
	Matrix3MultByMatrix3(m_roll, m_tmp, rotationMatrix);
}

//mxd. Assumes "direction" is normalized. Decompiled logic does NOT match with CMatrix::Matricies3FromDirAndUp from Tools/qMView!
H2COMMON_API double Matricies3FromDirAndUp(const vec3_t direction, const vec3_t up, matrix3_t toWorld, matrix3_t partialToLocal)
{
	vec3_t v_pitch, v_rotated_up;
	matrix3_t m_pitch, m_yaw, m_pitchyaw, m_tmp;

	float pitch = asinf(direction[2]); //mxd. asinf expects value in [-1.0; 1.0] range.

	assert(!isnan(pitch)); //mxd

	if (HACK_Pitch_Adjust && direction[0] < 0.0f)
		pitch = ANGLE_180 - pitch;

	CreatePitchMatrix(m_pitch, -pitch);
	Matrix3MultByVec3(m_pitch, direction, v_pitch);

	float yaw = 0.0f;
	if (direction[0] != 0.0f)
		yaw = atan2f(v_pitch[1], v_pitch[0]);

	CreateYawMatrix(m_yaw, yaw);
	Matrix3MultByMatrix3(m_pitch, m_yaw, m_pitchyaw);

	Matrix3MultByVec3(m_pitchyaw, up, v_rotated_up);
	v_rotated_up[0] = 0.0f;
	VectorNormalize(v_rotated_up);

	memset(m_tmp, 0, sizeof(matrix3_t));
	m_tmp[0][0] = 1.0f;

	double roll = -(atan2((double)v_rotated_up[2], (double)v_rotated_up[1]) - ANGLE_90);

	m_tmp[1][1] = cosf((float)roll);
	m_tmp[2][2] = m_tmp[1][1];

	m_tmp[1][2] = sinf((float)roll);
	m_tmp[2][1] = -sinf((float)roll);

	Matrix3MultByMatrix3(m_tmp, m_pitchyaw, toWorld);

	if (partialToLocal != NULL)
	{
		CreatePitchMatrix(m_pitch, pitch);
		CreateYawMatrix(m_yaw, -yaw);
		Matrix3MultByMatrix3(m_yaw, m_pitch, partialToLocal);
		roll *= -1.0;
	}

	return roll;
}

//mxd. Decompiled logic DOES match with CMatrix::RotatePointAboutLocalOrigin from Tools/qMView (but I replaced math ops with VectorAdd/VectorSubtract anyway...).
H2COMMON_API void RotatePointAboutLocalOrigin(const matrix3_t rotation, const vec3_t origin, vec3_t point)
{
	vec3_t temp;

	VectorSubtract(point, origin, point);
	Matrix3MultByVec3(rotation, point, temp);
	VectorAdd(temp, origin, point);
}

H2COMMON_API void TransformPoint(const matrix3_t rotation, const vec3_t origin, const vec3_t newOrigin, vec3_t point)
{
	vec3_t temp;

	VectorSubtract(point, origin, point);
	Matrix3MultByVec3(rotation, point, temp);
	VectorAdd(temp, newOrigin, point);
}