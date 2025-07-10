//
// gl1_Warp.c -- warped surfaces rendering
//
// Copyright 1998 Raven Software
//

#include "gl1_Warp.h"
#include "Hunk.h"
#include "Vector.h"

#define SUBDIVIDE_SIZE	64.0f

#pragma region ========================== POLYGON GENERATION ==========================

void R_EmitWaterPolys(const msurface_t* fa, const qboolean undulate)
{
	NOT_IMPLEMENTED
}

void R_EmitUnderwaterPolys(const msurface_t* fa) // H2
{
	NOT_IMPLEMENTED
}

void R_EmitQuakeFloorPolys(const msurface_t* fa) // H2
{
	NOT_IMPLEMENTED
}

#pragma endregion

// Q2 counterpart
static void R_BoundPoly(const int numverts, const float* verts, vec3_t mins, vec3_t maxs)
{
	ClearBounds(mins, maxs); //mxd. Original code directly sets mins to 9999, maxs to -9999.

	const float* v = verts;
	for (int i = 0; i < numverts; i++)
	{
		for (int j = 0; j < 3; j++, v++)
		{
			mins[j] = min(*v, mins[j]);
			maxs[j] = max(*v, maxs[j]);
		}
	}
}

// Q2 counterpart
static void R_SubdividePolygon(msurface_t* warpface, const int numverts, float* verts) //mxd. Added 'warpface' arg.
{
	vec3_t mins;
	vec3_t maxs;
	vec3_t front[64];
	vec3_t back[64];
	float dist[64];
	vec3_t total;

	if (numverts > 60)
		ri.Sys_Error(ERR_DROP, "numverts = %i", numverts);

	R_BoundPoly(numverts, verts, mins, maxs);

	for (int i = 0; i < 3; i++)
	{
		float m = (mins[i] + maxs[i]) * 0.5f;
		m = SUBDIVIDE_SIZE * floorf(m / SUBDIVIDE_SIZE + 0.5f); //mxd. floor -> floorf

		if (maxs[i] - m < 8.0f || m - mins[i] < 8.0f)
			continue;

		// Cut it.
		float* v = verts + i;
		for (int j = 0; j < numverts; j++, v += 3)
			dist[j] = *v - m;

		// Wrap cases.
		dist[numverts] = dist[0];
		v -= i;
		VectorCopy(verts, v);

		int f = 0;
		int b = 0;
		v = verts;
		for (int j = 0; j < numverts; j++, v += 3)
		{
			if (dist[j] >= 0)
			{
				VectorCopy(v, front[f]);
				f++;
			}

			if (dist[j] <= 0)
			{
				VectorCopy(v, back[b]);
				b++;
			}

			if (dist[j] == 0.0f || dist[j + 1] == 0.0f)
				continue;

			if ((dist[j] > 0) != (dist[j + 1] > 0))
			{
				// Clip point.
				const float frac = dist[j] / (dist[j] - dist[j + 1]);

				for (int k = 0; k < 3; k++)
					front[f][k] = back[b][k] = v[k] + frac * (v[3 + k] - v[k]);

				f++;
				b++;
			}
		}

		R_SubdividePolygon(warpface, f, front[0]);
		R_SubdividePolygon(warpface, b, back[0]);

		return;
	}

	// Add a point in the center to help keep warp valid.
	glpoly_t* poly = Hunk_Alloc((int)sizeof(glpoly_t) + ((numverts - 4) + 2) * VERTEXSIZE * sizeof(float));
	poly->next = warpface->polys;
	warpface->polys = poly;
	poly->numverts = numverts + 2;
	VectorClear(total);

	float total_s = 0.0f;
	float total_t = 0.0f;

	for (int i = 0; i < numverts; i++, verts += 3)
	{
		VectorCopy(verts, poly->verts[i + 1]);
		const float s = DotProduct(verts, warpface->texinfo->vecs[0]);
		const float t = DotProduct(verts, warpface->texinfo->vecs[1]);

		total_s += s;
		total_t += t;
		VectorAdd(total, verts, total);

		poly->verts[i + 1][3] = s;
		poly->verts[i + 1][4] = t;
	}

	VectorScale(total, 1.0f / (float)numverts, poly->verts[0]);
	poly->verts[0][3] = total_s / (float)numverts;
	poly->verts[0][4] = total_t / (float)numverts;

	// Copy first vertex to last.
	memcpy(poly->verts[numverts + 1], poly->verts[1], sizeof(poly->verts[0]));
}

// Breaks a polygon up along axial 64 unit boundaries so that turbulent and sky warps can be done reasonably.
void R_SubdivideSurface(const model_t* mdl, msurface_t* fa)
{
	static vec3_t verts[64]; //mxd. Made static.
	float* vec;

	// Convert edges back to a normal polygon.
	int numverts;
	for (numverts = 0; numverts < fa->numedges; numverts++)
	{
		const int lindex = mdl->surfedges[fa->firstedge + numverts];

		if (lindex > 0)
			vec = mdl->vertexes[mdl->edges[lindex].v[0]].position;
		else
			vec = mdl->vertexes[mdl->edges[-lindex].v[1]].position;

		VectorCopy(vec, verts[numverts]);
	}

	R_SubdividePolygon(fa, numverts, verts[0]);
}