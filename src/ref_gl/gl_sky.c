//
// gl_sky.c -- skybox rendering
//
// Copyright 1998 Raven Software
//

#include "gl_local.h"
#include "Vector.h"

static float skyrotate;
static vec3_t skyaxis;
static image_t* sky_images[6];

static float skymins[2][6];
static float skymaxs[2][6];
static float sky_min;
static float sky_max;

static vec3_t skyclip[6] =
{
	{  1.0f,  1.0f, 0.0f },
	{  1.0f, -1.0f, 0.0f },
	{  0.0f, -1.0f, 1.0f },
	{  0.0f,  1.0f, 1.0f },
	{  1.0f,  0.0f, 1.0f },
	{ -1.0f,  0.0f, 1.0f }
};

// 1 = s, 2 = t, 3 = 2048
static int st_to_vec[6][3] =
{
	{  3, -1,  2 },
	{ -3,  1,  2 },

	{  1,  3,  2 },
	{ -1, -3,  2 },

	{ -2, -1,  3 },	// 0 degrees yaw, look straight up
	{  2, -1, -3 }	// Look straight down
};

// s = [0]/[2], t = [1]/[2]
static int vec_to_st[6][3] =
{
	{ -2,  3,  1 },
	{  2,  3, -1 },

	{  1,  3,  2 },
	{ -1,  3, -2 },

	{ -2, -1,  3 },
	{ -2,  1, -3 }
};

// Q2 counterpart
static void DrawSkyPolygon(const int nump, vec3_t vecs)
{
	int i;
	float* vp;
	vec3_t v;
	vec3_t av;
	float s;
	float t;
	float dv;
	int axis;

	// Decide which face it maps to
	VectorCopy(vec3_origin, v);
	for (i = 0, vp = vecs; i < nump; i++, vp += 3)
		VectorAdd(vp, v, v);

	VectorAbs(v, av);

	if (av[0] > av[1] && av[0] > av[2])
		axis = (v[0] < 0 ? 1 : 0);
	else if (av[1] > av[2] && av[1] > av[0])
		axis = (v[1] < 0 ? 3 : 2);
	else
		axis = (v[2] < 0 ? 5 : 4);

	// Project new texture coords
	for (i = 0; i < nump; i++, vecs += 3)
	{
		int j = vec_to_st[axis][2];
		if (j > 0)
			dv = vecs[j - 1];
		else
			dv = -vecs[-j - 1];

		if (dv < 0.001f)
			continue; // Don't divide by zero

		j = vec_to_st[axis][0];
		if (j < 0)
			s = -vecs[-j - 1] / dv;
		else
			s = vecs[j - 1] / dv;

		j = vec_to_st[axis][1];
		if (j < 0)
			t = -vecs[-j - 1] / dv;
		else
			t = vecs[j - 1] / dv;

		skymins[0][axis] = min(s, skymins[0][axis]);
		skymins[1][axis] = min(t, skymins[1][axis]);

		skymaxs[0][axis] = max(s, skymaxs[0][axis]);
		skymaxs[1][axis] = max(t, skymaxs[1][axis]);
	}
}

#define MAX_CLIP_VERTS	64
#define ON_EPSILON		0.1f // Point on plane side epsilon

// Q2 counterpart
static void ClipSkyPolygon(const int nump, vec3_t vecs, const int stage)
{
	int i;
	float* v;
	float dists[MAX_CLIP_VERTS];
	int sides[MAX_CLIP_VERTS];
	vec3_t newv[2][MAX_CLIP_VERTS];
	int newc[2];

	if (nump > MAX_CLIP_VERTS - 2)
		ri.Sys_Error(ERR_DROP, "ClipSkyPolygon: MAX_CLIP_VERTS");

	if (stage == 6)
	{
		// Fully clipped, so draw it
		DrawSkyPolygon(nump, vecs);
		return;
	}

	qboolean front = false;
	qboolean back = false;
	const float* norm = skyclip[stage];

	for (i = 0, v = vecs; i < nump; i++, v += 3)
	{
		const float d = DotProduct(v, norm);

		if (d > ON_EPSILON)
		{
			front = true;
			sides[i] = SIDE_FRONT;
		}
		else if (d < -ON_EPSILON)
		{
			back = true;
			sides[i] = SIDE_BACK;
		}
		else
		{
			sides[i] = SIDE_ON;
		}

		dists[i] = d;
	}

	if (!front || !back)
	{
		// Not clipped
		ClipSkyPolygon(nump, vecs, stage + 1);
		return;
	}

	// Clip it
	sides[i] = sides[0];
	dists[i] = dists[0];
	VectorCopy(vecs, vecs + i * 3);
	newc[0] = 0;
	newc[1] = 0;

	for (i = 0, v = vecs; i < nump; i++, v += 3)
	{
		switch (sides[i])
		{
			case SIDE_FRONT:
				VectorCopy(v, newv[0][newc[0]]);
				newc[0]++;
				break;

			case SIDE_BACK:
				VectorCopy(v, newv[1][newc[1]]);
				newc[1]++;
				break;

			case SIDE_ON:
				VectorCopy(v, newv[0][newc[0]]);
				newc[0]++;
				VectorCopy(v, newv[1][newc[1]]);
				newc[1]++;
				break;
		}

		if (sides[i] == SIDE_ON || sides[i + 1] == SIDE_ON || sides[i + 1] == sides[i])
			continue;

		const float d = dists[i] / (dists[i] - dists[i + 1]);
		for (int j = 0; j < 3; j++)
		{
			const float e = v[j] + d * (v[j + 3] - v[j]);
			newv[0][newc[0]][j] = e;
			newv[1][newc[1]][j] = e;
		}

		newc[0]++;
		newc[1]++;
	}

	// Continue
	ClipSkyPolygon(newc[0], newv[0][0], stage + 1);
	ClipSkyPolygon(newc[1], newv[1][0], stage + 1);
}

// Q2 counterpart
void R_AddSkySurface(const msurface_t* fa)
{
	vec3_t verts[MAX_CLIP_VERTS];

	// Calculate vertex values for sky box
	for (const glpoly_t* p = fa->polys; p != NULL; p = p->next)
	{
		for (int i = 0; i < p->numverts; i++)
			VectorSubtract(p->verts[i], r_origin, verts[i]);

		ClipSkyPolygon(p->numverts, verts[0], 0);
	}
}

// Q2 counterpart
void R_ClearSkyBox(void)
{
	for (int i = 0; i < 6; i++)
	{
		skymins[0][i] = 9999.0f;
		skymins[1][i] = 9999.0f;
		skymaxs[0][i] = -9999.0f;
		skymaxs[1][i] = -9999.0f;
	}
}

static void MakeSkyVec(float s, float t, const int axis)
{
	vec3_t v;
	vec3_t b;
	float clipdist;

	// H2: new r_farclipdist logic
	if ((int)r_fog->value) //mxd. Removed gl_fog_broken cvar check
		clipdist = r_farclipdist->value;
	else
		clipdist = r_farclipdist->value * 0.5773503f; //TODO: what's with the scaler?

	VectorSet(b, s * clipdist, t * clipdist, clipdist); // Q2: 2300

	for (int i = 0; i < 3; i++)
	{
		const int k = st_to_vec[axis][i];
		if (k < 0)
			v[i] = -b[-k - 1];
		else
			v[i] = b[k - 1];
	}

	// Avoid bilerp seam
	s = (s + 1.0f) * 0.5f;
	t = (t + 1.0f) * 0.5f;

	s = Clamp(s, sky_min, sky_max);
	t = Clamp(t, sky_min, sky_max);

	qglTexCoord2f(s, 1.0f - t);
	qglVertex3fv(v);
}

void R_DrawSkyBox(void)
{
	static int skytexorder[] = { 0, 2, 1, 3, 4, 5 }; //mxd. Made local static
	int i;

	if (skyrotate != 0.0f)
	{
		// Check for no sky at all
		for (i = 0; i < 6; i++)
			if (skymins[0][i] < skymaxs[0][i] && skymins[1][i] < skymaxs[1][i])
				break;

		if (i == 6)
			return; // Nothing visible
	}

	qglPushMatrix();
	qglTranslatef(r_origin[0], r_origin[1], r_origin[2]);
	qglRotatef(r_newrefdef.time * skyrotate, skyaxis[0], skyaxis[1], skyaxis[2]);

	for (i = 0; i < 6; i++)
	{
		if (skyrotate != 0.0f)
		{
			// Hack, forces full sky to draw when rotating
			skymins[0][i] = -1.0f;
			skymins[1][i] = -1.0f;
			skymaxs[0][i] = 1.0f;
			skymaxs[1][i] = 1.0f;
		}

		if (skymins[0][i] < skymaxs[0][i] && skymins[1][i] < skymaxs[1][i])
		{
			GL_BindImage(sky_images[skytexorder[i]]); // Q2: GL_Bind()

			qglBegin(GL_QUADS);
			MakeSkyVec(skymins[0][i], skymins[1][i], i);
			MakeSkyVec(skymins[0][i], skymaxs[1][i], i);
			MakeSkyVec(skymaxs[0][i], skymaxs[1][i], i);
			MakeSkyVec(skymaxs[0][i], skymins[1][i], i);
			qglEnd();
		}
	}

	qglPopMatrix();
}

void R_SetSky(const char* name, const float rotate, const vec3_t axis)
{
	static char* suf[] = { "rt", "bk", "lf", "ft", "up", "dn" }; // 3dstudio environment map names //mxd. Made local static
	char pathname[MAX_QPATH];
	char skyname[MAX_QPATH];

	strncpy_s(skyname, sizeof(skyname), name, sizeof(skyname) - 1); //mxd. strncpy -> strncpy_s
	skyrotate = rotate;
	VectorCopy(axis, skyaxis);

	for (int i = 0; i < 6; i++)
	{
		// H2: missing gl_skymip and qglColorTableEXT logic, 'env/%s%s.pcx' / 'env/%s%s.tga' -> 'pics/skies/%s%s.m8'
		Com_sprintf(pathname, sizeof(pathname), "pics/skies/%s%s.m8", skyname, suf[i]);
		sky_images[i] = GL_FindImage(pathname, it_sky);

		if ((int)gl_picmip->value || skyrotate != 0.0f) //H2: gl_skymip -> gl_picmip
		{
			// Take less memory
			sky_min = 1.0f / 256.0f;
			sky_max = 255.0f / 256.0f;
		}
		else
		{
			sky_min = 1.0f / 512.0f;
			sky_max = 511.0f / 512.0f;
		}
	}
}