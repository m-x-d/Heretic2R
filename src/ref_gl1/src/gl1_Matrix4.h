//
// gl1_Matrix4.h -- Ported from DirectQII.
//
// Copyright 1998 Raven Software
//

#pragma once

#include "q_Typedef.h"

typedef union matrix4_s
{
	float m4x4[4][4];
	float m16[16];
} matrix4_t;

extern void R_MatrixIdentity(matrix4_t* m);
extern void R_MatrixMultiply(matrix4_t* out, const matrix4_t* m1, const matrix4_t* m2);
extern void R_MatrixTranslate(matrix4_t* m, const vec3_t v);
extern void R_MatrixRotate(matrix4_t* m, const vec3_t angles_rad);

extern void R_VectorTransform(const matrix4_t* m, vec3_t out, const vec3_t in);
extern void R_VectorInverseTransform(const matrix4_t* m, vec3_t out, const vec3_t in);