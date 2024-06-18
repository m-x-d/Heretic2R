#ifndef Q_SURFACE_H
#define Q_SURFACE_H

#include "q_shared.h"

#define MAX_POLY_VERTS 6

typedef struct Poly_s
{
	float		fraction;
	int			numverts;
	int			flags;
	float		(*verts)[7];
} Poly_t;

typedef struct Surface_s
{
	cplane_t	*plane;
	vec3_t		normal;
	vec3_t		point;			// point of intersection with line segment used to find the surface
	Poly_t		poly;
} Surface_t;

#endif // Q_SURFACE_H