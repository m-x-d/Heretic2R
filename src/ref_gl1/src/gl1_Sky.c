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

void R_ClearSkyBox(void)
{
	NOT_IMPLEMENTED
}

void R_DrawSkyBox(void)
{
	NOT_IMPLEMENTED
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