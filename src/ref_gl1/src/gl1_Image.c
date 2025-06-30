//
// gl1_Image.c
//
// Copyright 1998 Raven Software
//

#include "gl1_Image.h"

image_t gltextures[MAX_GLTEXTURES];
int numgltextures;

static byte gammatable[256];

void InitGammaTable(void) // H2
{
	float contrast = 1.0f - vid_contrast->value;

	if (contrast > 0.5f)
		contrast = powf(contrast + 0.5f, 3.0f);
	else
		contrast = powf(contrast + 0.5f, 0.5f);

	gammatable[0] = 0;

	for (int i = 1; i < 256; i++)
	{
		float inf = 255.0f * powf(((float)i + 0.5f) / 255.5f, vid_gamma->value) + 0.5f;
		float sign;

		if (inf > 128.0f)
		{
			inf = 128.0f - inf;
			sign = -1.0f;
		}
		else
		{
			inf -= 128.0f;
			sign = 1.0f;
		}

		inf = (vid_brightness->value * 160.0f - 80.0f) + (powf(inf / 128.0f, contrast) * sign + 1.0f) * 128.0f;

		gammatable[i] = (byte)ClampI((int)inf, 0, 255);
	}
}

void GL_ImageList_f(void)
{
	NOT_IMPLEMENTED
}

struct image_s* R_RegisterSkin(const char* name, qboolean* retval)
{
	NOT_IMPLEMENTED
	return NULL;
}

static void GL_FreeImage(image_t* image) // H2
{
	NOT_IMPLEMENTED
}

void GL_InitImages(void)
{
	NOT_IMPLEMENTED
}

void GL_ShutdownImages(void)
{
	image_t* image = &gltextures[0];
	for (int i = 0; i < numgltextures; i++, image++)
		if (image->registration_sequence != 0)
			GL_FreeImage(image);
}