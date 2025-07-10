//
// gl1_FindSurface.c
//
// Copyright 1998 Raven Software
//

#include "gl1_FindSurface.h"
#include "q_Surface.h"
#include "Vector.h"

static qboolean PolyContainsPoint(const float* verts, const int numverts, const int vert_size, const vec3_t normal, const vec3_t mid)
{
	vec3_t cur_vert;
	vec3_t next_vert;
	vec3_t cross;

	for (int i = 0; i < numverts; i++)
	{
		const int cur_index = i * vert_size;
		VectorSubtract(&verts[cur_index], mid, cur_vert);

		const int next_index = (i == numverts - 1 ? 0 : i + 1) * vert_size;
		VectorSubtract(&verts[next_index], mid, next_vert);

		CrossProduct(next_vert, cur_vert, cross);

		if (VectorNormalize(cross) == 0.0f)
		{
			VectorNormalize(cur_vert);
			VectorNormalize(next_vert);

			return (fabsf(DotProduct(cur_vert, next_vert) + 1.0f) < 0.0005f);
		}

		if (fabsf(DotProduct(cross, normal) - 1.0f) > 0.06f)
			return false;
	}

	return true;
}

static void NormalForPoly(const glpoly_t* poly, const int index, vec3_t normal)
{
	vec3_t start;
	vec3_t end;

	VectorSubtract(poly->verts[index + 0], poly->verts[index + 1], start);
	VectorSubtract(poly->verts[index + 2], poly->verts[index + 1], end);
	CrossProduct(start, end, normal);
	VectorNormalize(normal);
}

//mxd. Somewhat similar to RecursiveLightPoint().
static int FindSurfaceR(const mnode_t* node, const vec3_t start, const vec3_t end, Surface_t* surface, const int index)
{
#define MAX_HIT_SURFACES	10

	if (node == NULL)
		node = r_worldmodel->nodes;

	// Didn't hit anything.
	if (node->contents != -1)
		return -1;

	// Calculate mid point.
	const cplane_t* plane = node->plane;

	const float front = DotProduct(start, plane->normal) - plane->dist;
	const float back = DotProduct(end, plane->normal) - plane->dist;
	const uint side = front < 0.0f;

	if ((back < 0.0f) == side)
		return FindSurfaceR(node->children[side], start, end, surface, index + 1);

	const float frac = front / (front - back);

	vec3_t mid;
	for (int i = 0; i < 3; i++)
		mid[i] = start[i] + (end[i] - start[i]) * frac;

	// Go down front side.
	const int r = FindSurfaceR(node->children[side], start, mid, surface, index + 1);

	// Hit something.
	if (r >= 0)
		return r;

	// Didn't hit anything.
	if ((back < 0.0f) == side)
		return -1;

	int num_hits = 0;
	int hit_surfaces[MAX_HIT_SURFACES];

	msurface_t* surf = r_worldmodel->surfaces + node->firstsurface;
	for (int i = 0; i < node->numsurfaces; i++, surf++)
	{
		if (surf->flags & (SURF_DRAWTURB | SURF_DRAWSKY))
			continue; // No lightmaps.

		const mtexinfo_t* tex = surf->texinfo;

		const int s = (int)(DotProduct(mid, tex->vecs[0]) + tex->vecs[0][3]);
		const int t = (int)(DotProduct(mid, tex->vecs[1]) + tex->vecs[1][3]);

		if (s >= surf->texturemins[0] && t >= surf->texturemins[1] && s - surf->texturemins[0] <= surf->extents[0] && t - surf->texturemins[1] <= surf->extents[1])
		{
			hit_surfaces[num_hits] = i;
			num_hits++;

			//mxd. Added sanity check.
			if (num_hits == MAX_HIT_SURFACES)
				break;
		}
	}

	for (int i = 0; i < num_hits; i++)
	{
		vec3_t surf_normal;
		const msurface_t* hit_surf = &r_worldmodel->surfaces[node->firstsurface + hit_surfaces[i]];

		for (int j = 0; j < hit_surf->polys->numverts - 2; j++)
		{
			NormalForPoly(hit_surf->polys, j, surf_normal);
			if (Vec3NotZero(surf_normal))
				break;
		}

		if (PolyContainsPoint(hit_surf->polys->verts[0], hit_surf->polys->numverts, VERTEXSIZE, surf_normal, mid))
		{
			VectorCopy(surf_normal, surface->normal);
			VectorCopy(mid, surface->point);

			surface->plane = hit_surf->plane;
			surface->poly.numverts = hit_surf->polys->numverts;
			surface->poly.flags = hit_surf->polys->flags;
			surface->poly.verts = hit_surf->polys->verts;

			return 1;
		}
	}

	if (num_hits != 0)
		return -1;

	// Go down back side.
	return FindSurfaceR(node->children[front >= 0.0f], mid, end, surface, index + 1);
}

//TODO: seems unused. Untested...
int RI_FindSurface(const vec3_t start, const vec3_t end, struct Surface_s* surface)
{
	return FindSurfaceR(NULL, start, end, surface, 0);
}