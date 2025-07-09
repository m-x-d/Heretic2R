//
// gl1_Sky.c
//
// Copyright 1998 Raven Software
//

#include "gl1_Sky.h"
#include "gl1_Image.h"
#include "Vector.h"

static float skyrotate;
static vec3_t skyaxis;
static image_t* sky_images[6];

static float skymins[2][6];
static float skymaxs[2][6];
static float sky_min;
static float sky_max;

void R_AddSkySurface(const msurface_t* fa)
{
	NOT_IMPLEMENTED
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

static void R_MakeSkyVec(float s, float t, const int axis)
{
	NOT_IMPLEMENTED
}

void R_DrawSkyBox(void)
{
	static int skytexorder[] = { 0, 2, 1, 3, 4, 5 }; //mxd. Made local static.

	if (skyrotate != 0.0f)
	{
		// Check for no sky at all.
		int side;
		for (side = 0; side < 6; side++)
			if (skymins[0][side] < skymaxs[0][side] && skymins[1][side] < skymaxs[1][side])
				break;

		if (side == 6)
			return; // Nothing visible.
	}

	glPushMatrix();
	glTranslatef(r_origin[0], r_origin[1], r_origin[2]);
	glRotatef(r_newrefdef.time * skyrotate, skyaxis[0], skyaxis[1], skyaxis[2]);

	for (int i = 0; i < 6; i++)
	{
		if (skyrotate != 0.0f)
		{
			// Hack, forces full sky to draw when rotating.
			skymins[0][i] = -1.0f;
			skymins[1][i] = -1.0f;
			skymaxs[0][i] = 1.0f;
			skymaxs[1][i] = 1.0f;
		}

		if (skymins[0][i] < skymaxs[0][i] && skymins[1][i] < skymaxs[1][i])
		{
			R_BindImage(sky_images[skytexorder[i]]); // Q2: GL_Bind()

			glBegin(GL_QUADS);
			R_MakeSkyVec(skymins[0][i], skymins[1][i], i);
			R_MakeSkyVec(skymins[0][i], skymaxs[1][i], i);
			R_MakeSkyVec(skymaxs[0][i], skymaxs[1][i], i);
			R_MakeSkyVec(skymaxs[0][i], skymins[1][i], i);
			glEnd();
		}
	}

	glPopMatrix();
}

void R_SetSky(const char* name, const float rotate, const vec3_t axis)
{
	static char* surf[] = { "rt", "bk", "lf", "ft", "up", "dn" }; // 3dstudio environment map names. //mxd. Made local static.
	char pathname[MAX_QPATH];
	char skyname[MAX_QPATH];

	strncpy_s(skyname, sizeof(skyname), name, sizeof(skyname) - 1); //mxd. strncpy -> strncpy_s
	skyrotate = rotate;
	VectorCopy(axis, skyaxis);

	for (int i = 0; i < 6; i++)
	{
		// H2: missing gl_skymip and qglColorTableEXT logic, 'env/%s%s.pcx' / 'env/%s%s.tga' -> 'pics/skies/%s%s.m8'
		Com_sprintf(pathname, sizeof(pathname), "pics/skies/%s%s.m8", skyname, surf[i]);
		sky_images[i] = R_FindImage(pathname, it_sky);

		if ((int)gl_picmip->value || skyrotate != 0.0f) // H2: gl_skymip -> gl_picmip
		{
			// Take less memory.
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