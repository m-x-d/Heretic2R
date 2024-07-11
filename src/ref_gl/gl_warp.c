//
// gl_warp.c -- warped surfaces rendering
//
// Copyright 1998 Raven Software
//

#include "gl_local.h"
#include "Vector.h"

extern model_t* loadmodel;
msurface_t* warpface;

#define SUBDIVIDE_SIZE	64

// Speed up sin calculations - Ed
//mxd. Pre-calculated sine wave in [-8.0 .. 8.0] range. Values are the same as in Q2 //TODO: replace with sinf()?
float r_turbsin[] =
{
	0.0f, 0.19633f, 0.392541f, 0.588517f, 0.784137f, 0.979285f, 1.17384f, 1.3677f, 1.56072f, 1.75281f, 1.94384f, 2.1337f, 2.32228f, 2.50945f, 2.69512f, 2.87916f,
	3.06147f, 3.24193f, 3.42044f, 3.59689f, 3.77117f, 3.94319f, 4.11282f, 4.27998f, 4.44456f, 4.60647f, 4.76559f, 4.92185f, 5.07515f, 5.22538f, 5.37247f, 5.51632f,
	5.65685f, 5.79398f, 5.92761f, 6.05767f, 6.18408f, 6.30677f, 6.42566f, 6.54068f, 6.65176f, 6.75883f, 6.86183f, 6.9607f, 7.05537f, 7.14579f, 7.23191f, 7.31368f,
	7.39104f, 7.46394f, 7.53235f, 7.59623f, 7.65552f, 7.71021f, 7.76025f, 7.80562f, 7.84628f, 7.88222f, 7.91341f, 7.93984f, 7.96148f, 7.97832f, 7.99036f, 7.99759f,
	8.0f, 7.99759f, 7.99036f, 7.97832f, 7.96148f, 7.93984f, 7.91341f, 7.88222f, 7.84628f, 7.80562f, 7.76025f, 7.71021f, 7.65552f, 7.59623f, 7.53235f, 7.46394f,
	7.39104f, 7.31368f, 7.23191f, 7.14579f, 7.05537f, 6.9607f, 6.86183f, 6.75883f, 6.65176f, 6.54068f, 6.42566f, 6.30677f, 6.18408f, 6.05767f, 5.92761f, 5.79398f,
	5.65685f, 5.51632f, 5.37247f, 5.22538f, 5.07515f, 4.92185f, 4.76559f, 4.60647f, 4.44456f, 4.27998f, 4.11282f, 3.94319f, 3.77117f, 3.59689f, 3.42044f, 3.24193f,
	3.06147f, 2.87916f, 2.69512f, 2.50945f, 2.32228f, 2.1337f, 1.94384f, 1.75281f, 1.56072f, 1.3677f, 1.17384f, 0.979285f, 0.784137f, 0.588517f, 0.392541f, 0.19633f,
	0.0f, -0.19633f, -0.392541f, -0.588517f, -0.784137f, -0.979285f, -1.17384f, -1.3677f, -1.56072f, -1.75281f, -1.94384f, -2.1337f, -2.32228f, -2.50945f, -2.69512f, -2.87916f,
	-3.06147f, -3.24193f, -3.42044f, -3.59689f, -3.77117f, -3.94319f, -4.11282f, -4.27998f, -4.44456f, -4.60647f, -4.76559f, -4.92185f, -5.07515f, -5.22538f, -5.37247f, -5.51632f,
	-5.65685f, -5.79398f, -5.92761f, -6.05767f, -6.18408f, -6.30677f, -6.42566f, -6.54068f, -6.65176f, -6.75883f, -6.86183f, -6.9607f, -7.05537f, -7.14579f, -7.23191f, -7.31368f,
	-7.39104f, -7.46394f, -7.53235f, -7.59623f, -7.65552f, -7.71021f, -7.76025f, -7.80562f, -7.84628f, -7.88222f, -7.91341f, -7.93984f, -7.96148f, -7.97832f, -7.99036f, -7.99759f,
	-8.0f, -7.99759f, -7.99036f, -7.97832f, -7.96148f, -7.93984f, -7.91341f, -7.88222f, -7.84628f, -7.80562f, -7.76025f, -7.71021f, -7.65552f, -7.59623f, -7.53235f, -7.46394f,
	-7.39104f, -7.31368f, -7.23191f, -7.14579f, -7.05537f, -6.9607f, -6.86183f, -6.75883f, -6.65176f, -6.54068f, -6.42566f, -6.30677f, -6.18408f, -6.05767f, -5.92761f, -5.79398f,
	-5.65685f, -5.51632f, -5.37247f, -5.22538f, -5.07515f, -4.92185f, -4.76559f, -4.60647f, -4.44456f, -4.27998f, -4.11282f, -3.94319f, -3.77117f, -3.59689f, -3.42044f, -3.24193f,
	-3.06147f, -2.87916f, -2.69512f, -2.50945f, -2.32228f, -2.1337f, -1.94384f, -1.75281f, -1.56072f, -1.3677f, -1.17384f, -0.979285f, -0.784137f, -0.588517f, -0.392541f, -0.19633f,
};

#define TURBSCALE	(256.0f / ANGLE_360) //mxd. Replaced (2 * M_PI) with ANGLE_360

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

			float s = os + r_turbsin[Q_ftol((ot * 0.125f + r_newrefdef.time) * TURBSCALE) & 255];
			s += scroll;
			s /= 64.0f;

			float t = ot + r_turbsin[Q_ftol((os * 0.125f + r_newrefdef.time) * TURBSCALE) & 255];
			t /= 64.0f;

			qglTexCoord2f(s, t);

			if (undulate) // H2: new undulate logic
			{
				vec3_t pos;
				VectorCopy(v, pos);

				pos[2] += r_turbsin[Q_ftol(((v[0] * 2.3f + v[1]) * 0.015f + r_newrefdef.time * 3.0f) * TURBSCALE) & 255] * 0.25f +
						  r_turbsin[Q_ftol(((v[1] * 2.3f + v[0]) * 0.015f + r_newrefdef.time * 6.0f) * TURBSCALE) & 255] * 0.125f;

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