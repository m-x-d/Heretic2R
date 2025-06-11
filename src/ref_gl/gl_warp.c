//
// gl_warp.c -- warped surfaces rendering
//
// Copyright 1998 Raven Software
//

#include "gl_local.h"
#include "Hunk.h" //mxd
#include "turbsin.h"
#include "Vector.h"

extern model_t* loadmodel;
msurface_t* warpface;

#define SUBDIVIDE_SIZE	64
#define TURBSCALE		(256.0f / ANGLE_360) //mxd. Replaced (2 * M_PI) with ANGLE_360

//mxd. Helper defines...
#define TURBSIN_V0(v0, v1)	(Q_ftol((((v0) * 2.3f + (v1)) * 0.015f + r_newrefdef.time * 3.0f) * TURBSCALE) & 255)
#define TURBSIN_V1(v0, v1)	(Q_ftol((((v1) * 2.3f + (v0)) * 0.015f + r_newrefdef.time * 6.0f) * TURBSCALE) & 255)

void EmitWaterPolys(const msurface_t* fa, const qboolean undulate)
{
	int i;
	float* v;
	float scroll;

	if (fa->texinfo->flags & SURF_FLOWING)
		scroll = -64.0f * ((r_newrefdef.time * 0.5f) - floorf(r_newrefdef.time * 0.5f)); //mxd. Replaced int cast with floorf.
	else
		scroll = 0.0f;

	for (glpoly_t* p = fa->polys; p != NULL; p = p->next)
	{
		qglBegin(GL_TRIANGLE_FAN);

		for (i = 0, v = p->verts[0]; i < p->numverts; i++, v += VERTEXSIZE)
		{
			const float os = v[3];
			const float ot = v[4];

			float s = os + turbsin[Q_ftol((ot * 0.125f + r_newrefdef.time) * TURBSCALE) & 255];
			s += scroll;
			s /= 64.0f;

			float t = ot + turbsin[Q_ftol((os * 0.125f + r_newrefdef.time) * TURBSCALE) & 255];
			t /= 64.0f;

			qglTexCoord2f(s, t);

			if (undulate) // H2: new undulate logic
			{
				vec3_t pos;
				VectorCopy(v, pos);

				pos[2] += turbsin[TURBSIN_V0(v[0], v[1])] * 0.25f + 
						  turbsin[TURBSIN_V1(v[0], v[1])] * 0.125f;

				qglVertex3fv(pos);
			}
			else
			{
				qglVertex3fv(v);
			}
		}

		qglEnd();
	}
}

//TODO: Warps all bmodel polys when camera is underwater. Seems to be used only when r_fullbright is 1. H2 bug?
void EmitUnderWaterPolys(const msurface_t* fa)
{
	int i;
	float* v;
	
	for (glpoly_t* p = fa->polys; p != NULL; p = p->next)
	{
		qglBegin(GL_TRIANGLE_FAN);

		for (i = 0, v = p->verts[0]; i < p->numverts; i++, v += VERTEXSIZE)
		{
			vec3_t pos;
			VectorCopy(v, pos);

			pos[2] += turbsin[TURBSIN_V0(v[0], v[1])] * 0.5f + 
					  turbsin[TURBSIN_V1(v[0], v[1])] * 0.25f;

			qglTexCoord2f(v[3], v[4]);
			qglVertex3fv(pos);
		}

		qglEnd();
	}
}

//TODO: Warps all bmodel polys when quake_amount > 0. Seems to be used only when r_fullbright is 1. H2 bug?
void EmitQuakeFloorPolys(const msurface_t* fa)
{
	int i;
	float* v;

	for (glpoly_t* p = fa->polys; p != NULL; p = p->next)
	{
		qglBegin(GL_TRIANGLE_FAN);

		for (i = 0, v = p->verts[0]; i < p->numverts; i++, v += VERTEXSIZE)
		{
			vec3_t pos;
			VectorCopy(v, pos);

			pos[2] += turbsin[TURBSIN_V0(v[0], v[1])] * (quake_amount->value * 0.05f) * 0.5f +
					  turbsin[TURBSIN_V1(v[0], v[1])] * (quake_amount->value * 0.05f) * 0.25f;

			qglTexCoord2f(v[3], v[4]);
			qglVertex3fv(pos);

		}

		qglEnd();
	}
}

// Q2 counterpart
static void BoundPoly(const int numverts, const float* verts, vec3_t mins, vec3_t maxs)
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
static void SubdividePolygon(const int numverts, float* verts)
{
	vec3_t mins;
	vec3_t maxs;
	vec3_t front[64];
	vec3_t back[64];
	float dist[64];
	vec3_t total;

	if (numverts > 60)
		ri.Sys_Error(ERR_DROP, "numverts = %i", numverts);

	BoundPoly(numverts, verts, mins, maxs);

	for (int i = 0; i < 3; i++)
	{
		float m = (mins[i] + maxs[i]) * 0.5f;
		m = SUBDIVIDE_SIZE * floorf(m / SUBDIVIDE_SIZE + 0.5f); //mxd. floor -> floorf

		if (maxs[i] - m < 8.0f || m - mins[i] < 8.0f)
			continue;

		// Cut it
		float* v = verts + i;
		for (int j = 0; j < numverts; j++, v += 3)
			dist[j] = *v - m;

		// Wrap cases
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
				// Clip point
				const float frac = dist[j] / (dist[j] - dist[j + 1]);

				for (int k = 0; k < 3; k++)
					front[f][k] = back[b][k] = v[k] + frac * (v[3 + k] - v[k]);

				f++;
				b++;
			}
		}

		SubdividePolygon(f, front[0]);
		SubdividePolygon(b, back[0]);

		return;
	}

	// Add a point in the center to help keep warp valid
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

	// Copy first vertex to last
	memcpy(poly->verts[numverts + 1], poly->verts[1], sizeof(poly->verts[0]));
}

void GL_SubdivideSurface(msurface_t* fa)
{
	static vec3_t verts[64]; //mxd. Made static
	float* vec;

	warpface = fa;

	// Convert edges back to a normal polygon
	int c;
	for (c = 0; c < fa->numedges; c++)
	{
		const int lindex = loadmodel->surfedges[fa->firstedge + c];

		if (lindex > 0)
			vec = loadmodel->vertexes[loadmodel->edges[lindex].v[0]].position;
		else
			vec = loadmodel->vertexes[loadmodel->edges[-lindex].v[1]].position;

		VectorCopy(vec, verts[c]);
	}

	SubdividePolygon(c, verts[0]);
}