//
// gl1_Matrix4.c -- Ported from DirectQII.
//
// Copyright 1998 Raven Software
//

#include "gl1_Matrix4.h"
#include "Vector.h"
#include "q_shared.h"

static void R_MatrixLoadf(matrix4_t* m, const float _11, const float _12, const float _13, const float _14, const float _21, const float _22, const float _23, const float _24, const float _31, const float _32, const float _33, const float _34, const float _41, const float _42, const float _43, const float _44)
{
	m->m4x4[0][0] = _11; m->m4x4[0][1] = _12; m->m4x4[0][2] = _13; m->m4x4[0][3] = _14;
	m->m4x4[1][0] = _21; m->m4x4[1][1] = _22; m->m4x4[1][2] = _23; m->m4x4[1][3] = _24;
	m->m4x4[2][0] = _31; m->m4x4[2][1] = _32; m->m4x4[2][2] = _33; m->m4x4[2][3] = _34;
	m->m4x4[3][0] = _41; m->m4x4[3][1] = _42; m->m4x4[3][2] = _43; m->m4x4[3][3] = _44;
}

void R_MatrixIdentity(matrix4_t* m)
{
	R_MatrixLoadf(m,
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	);
}

void R_MatrixMultiply(matrix4_t* out, const matrix4_t* m1, const matrix4_t* m2)
{
	R_MatrixLoadf(out,
		(m1->m4x4[0][0] * m2->m4x4[0][0]) + (m1->m4x4[1][0] * m2->m4x4[0][1]) + (m1->m4x4[2][0] * m2->m4x4[0][2]) + (m1->m4x4[3][0] * m2->m4x4[0][3]),
		(m1->m4x4[0][1] * m2->m4x4[0][0]) + (m1->m4x4[1][1] * m2->m4x4[0][1]) + (m1->m4x4[2][1] * m2->m4x4[0][2]) + (m1->m4x4[3][1] * m2->m4x4[0][3]),
		(m1->m4x4[0][2] * m2->m4x4[0][0]) + (m1->m4x4[1][2] * m2->m4x4[0][1]) + (m1->m4x4[2][2] * m2->m4x4[0][2]) + (m1->m4x4[3][2] * m2->m4x4[0][3]),
		(m1->m4x4[0][3] * m2->m4x4[0][0]) + (m1->m4x4[1][3] * m2->m4x4[0][1]) + (m1->m4x4[2][3] * m2->m4x4[0][2]) + (m1->m4x4[3][3] * m2->m4x4[0][3]),
		(m1->m4x4[0][0] * m2->m4x4[1][0]) + (m1->m4x4[1][0] * m2->m4x4[1][1]) + (m1->m4x4[2][0] * m2->m4x4[1][2]) + (m1->m4x4[3][0] * m2->m4x4[1][3]),
		(m1->m4x4[0][1] * m2->m4x4[1][0]) + (m1->m4x4[1][1] * m2->m4x4[1][1]) + (m1->m4x4[2][1] * m2->m4x4[1][2]) + (m1->m4x4[3][1] * m2->m4x4[1][3]),
		(m1->m4x4[0][2] * m2->m4x4[1][0]) + (m1->m4x4[1][2] * m2->m4x4[1][1]) + (m1->m4x4[2][2] * m2->m4x4[1][2]) + (m1->m4x4[3][2] * m2->m4x4[1][3]),
		(m1->m4x4[0][3] * m2->m4x4[1][0]) + (m1->m4x4[1][3] * m2->m4x4[1][1]) + (m1->m4x4[2][3] * m2->m4x4[1][2]) + (m1->m4x4[3][3] * m2->m4x4[1][3]),
		(m1->m4x4[0][0] * m2->m4x4[2][0]) + (m1->m4x4[1][0] * m2->m4x4[2][1]) + (m1->m4x4[2][0] * m2->m4x4[2][2]) + (m1->m4x4[3][0] * m2->m4x4[2][3]),
		(m1->m4x4[0][1] * m2->m4x4[2][0]) + (m1->m4x4[1][1] * m2->m4x4[2][1]) + (m1->m4x4[2][1] * m2->m4x4[2][2]) + (m1->m4x4[3][1] * m2->m4x4[2][3]),
		(m1->m4x4[0][2] * m2->m4x4[2][0]) + (m1->m4x4[1][2] * m2->m4x4[2][1]) + (m1->m4x4[2][2] * m2->m4x4[2][2]) + (m1->m4x4[3][2] * m2->m4x4[2][3]),
		(m1->m4x4[0][3] * m2->m4x4[2][0]) + (m1->m4x4[1][3] * m2->m4x4[2][1]) + (m1->m4x4[2][3] * m2->m4x4[2][2]) + (m1->m4x4[3][3] * m2->m4x4[2][3]),
		(m1->m4x4[0][0] * m2->m4x4[3][0]) + (m1->m4x4[1][0] * m2->m4x4[3][1]) + (m1->m4x4[2][0] * m2->m4x4[3][2]) + (m1->m4x4[3][0] * m2->m4x4[3][3]),
		(m1->m4x4[0][1] * m2->m4x4[3][0]) + (m1->m4x4[1][1] * m2->m4x4[3][1]) + (m1->m4x4[2][1] * m2->m4x4[3][2]) + (m1->m4x4[3][1] * m2->m4x4[3][3]),
		(m1->m4x4[0][2] * m2->m4x4[3][0]) + (m1->m4x4[1][2] * m2->m4x4[3][1]) + (m1->m4x4[2][2] * m2->m4x4[3][2]) + (m1->m4x4[3][2] * m2->m4x4[3][3]),
		(m1->m4x4[0][3] * m2->m4x4[3][0]) + (m1->m4x4[1][3] * m2->m4x4[3][1]) + (m1->m4x4[2][3] * m2->m4x4[3][2]) + (m1->m4x4[3][3] * m2->m4x4[3][3])
	);
}

void R_MatrixTranslate(matrix4_t* m, const vec3_t v)
{
	m->m4x4[3][0] += v[0] * m->m4x4[0][0] + v[1] * m->m4x4[1][0] + v[2] * m->m4x4[2][0];
	m->m4x4[3][1] += v[0] * m->m4x4[0][1] + v[1] * m->m4x4[1][1] + v[2] * m->m4x4[2][1];
	m->m4x4[3][2] += v[0] * m->m4x4[0][2] + v[1] * m->m4x4[1][2] + v[2] * m->m4x4[2][2];
	m->m4x4[3][3] += v[0] * m->m4x4[0][3] + v[1] * m->m4x4[1][3] + v[2] * m->m4x4[2][3];
}

void R_MatrixRotate(matrix4_t* m, const vec3_t angles)
{
	const float sr = sinf(DEG2RAD(angles[ROLL]));
	const float sp = sinf(DEG2RAD(angles[PITCH]));
	const float sy = sinf(DEG2RAD(angles[YAW]));
	const float cr = cosf(DEG2RAD(angles[ROLL]));
	const float cp = cosf(DEG2RAD(angles[PITCH]));
	const float cy = cosf(DEG2RAD(angles[YAW]));

	const matrix4_t m1 =
	{
		(cp * cy),
		(cp * sy),
		-sp,
		0.0f,
		(cr * -sy) + (sr * sp * cy),
		(cr * cy) + (sr * sp * sy),
		(sr * cp),
		0.0f,
		(sr * sy) + (cr * sp * cy),
		(-sr * cy) + (cr * sp * sy),
		(cr * cp),
		0.0f,
		0.0f,
		0.0f,
		0.0f,
		1.0f
	};

	R_MatrixMultiply(m, m, &m1);
}

void R_VectorTransform(const matrix4_t* m, vec3_t out, const vec3_t in)
{
	out[0] = in[0] * m->m4x4[0][0] + in[1] * m->m4x4[1][0] + in[2] * m->m4x4[2][0] + m->m4x4[3][0];
	out[1] = in[1] * m->m4x4[0][1] + in[1] * m->m4x4[1][1] + in[2] * m->m4x4[2][1] + m->m4x4[3][1];
	out[2] = in[2] * m->m4x4[0][2] + in[1] * m->m4x4[1][2] + in[2] * m->m4x4[2][2] + m->m4x4[3][2];
}

void R_VectorInverseTransform(const matrix4_t* m, vec3_t out, const vec3_t in)
{
	// http://content.gpwiki.org/index.php/MathGem:Fast_Matrix_Inversion (note: dead link)
	out[0] = DotProduct(m->m4x4[0], in) - DotProduct(m->m4x4[0], m->m4x4[3]);
	out[1] = DotProduct(m->m4x4[1], in) - DotProduct(m->m4x4[1], m->m4x4[3]);
	out[2] = DotProduct(m->m4x4[2], in) - DotProduct(m->m4x4[2], m->m4x4[3]);
}