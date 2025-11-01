//
// q_Surface.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "q_shared.h"

#define MAX_POLY_VERTS 6 //TODO: unused.

typedef struct Poly_s
{
	float fraction;
	int numverts;
	int flags;
	float (*verts)[7];
} Poly_t;

typedef struct Surface_s
{
	cplane_t* plane;
	vec3_t normal;
	vec3_t point; // Point of intersection with line segment used to find the surface.
	Poly_t poly;
} Surface_t;