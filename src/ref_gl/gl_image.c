//
// gl_image.c -- image loading and caching
//
// Copyright 1998 Raven Software
//

#include "gl_local.h"

static byte gammatable[256];

int gl_filter_min = GL_NEAREST; // Q2: GL_LINEAR_MIPMAP_NEAREST;
int gl_filter_max = GL_LINEAR;

void InitGammaTable(void)
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

// Q2 counterpart
void GL_TexEnv(GLenum mode)
{
	static GLenum lastmodes[2] = { -1, -1 };

	if (mode != lastmodes[gl_state.currenttmu])
	{
		qglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, (GLfloat)mode);
		lastmodes[gl_state.currenttmu] = mode;
	}
}

void GL_TextureMode(char* string)
{
	NOT_IMPLEMENTED
}

void GL_ImageList_f(void)
{
	NOT_IMPLEMENTED
}

struct image_s* R_RegisterSkin(char* name, qboolean* retval)
{
	NOT_IMPLEMENTED
	return NULL;
}

void GL_InitImages(void)
{
	registration_sequence = 1;
	gl_state.inverse_intensity = 1.0f;
}

void GL_ShutdownImages(void)
{
	NOT_IMPLEMENTED
}