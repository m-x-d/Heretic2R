//
// gl_image.c -- image loading and caching
//
// Copyright 1998 Raven Software
//

#include "gl_local.h"

image_t gltextures[MAX_GLTEXTURES];
int numgltextures;

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

//mxd. Most likely was changed from GL_Bind in H2 to use img->palette in qglColorTableEXT logic (which we skip...)
void GL_BindImage(const image_t* img)
{
	extern image_t* draw_chars;
	const int texnum = ((int)gl_nobind->value && draw_chars ? draw_chars->texnum : img->texnum);

	if (gl_state.currenttextures[gl_state.currenttmu] != texnum)
	{
		//mxd. Skipping qglColorTableEXT logic

		gl_state.currenttextures[gl_state.currenttmu] = texnum;
		qglBindTexture(GL_TEXTURE_2D, texnum);
	}
}

typedef struct
{
	char* name;
	int	minimize;
	int maximize;
} glmode_t;

glmode_t modes[] =
{
	{ "GL_NEAREST", GL_NEAREST, GL_NEAREST },
	{ "GL_LINEAR", GL_LINEAR, GL_LINEAR },
	{ "GL_NEAREST_MIPMAP_NEAREST", GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST },
	{ "GL_LINEAR_MIPMAP_NEAREST", GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR },
	{ "GL_NEAREST_MIPMAP_LINEAR", GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST },
	{ "GL_LINEAR_MIPMAP_LINEAR", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR }
};

#define NUM_GL_MODES ((int)(sizeof(modes) / sizeof(glmode_t))) //mxd. Added int cast

void GL_TextureMode(char* string)
{
	int i;

	for (i = 0; i < NUM_GL_MODES; i++)
		if (!Q_stricmp(modes[i].name, string))
			break;

	if (i == NUM_GL_MODES)
	{
		ri.Con_Printf(PRINT_ALL, "Bad texture filter name\n"); // H2: text change
		return;
	}

	gl_filter_min = modes[i].minimize;
	gl_filter_max = modes[i].maximize;

	// Change all the existing mipmap texture objects
	image_t* glt;
	for (i = 0, glt = gltextures; i < numgltextures; i++, glt++)
	{
		if (glt->type != it_pic && glt->type != it_sky)
		{
			GL_BindImage(glt); // Q2: GL_Bind(glt->texnum)

			//mxd. Decompiled code passes 0x84fe instead of GL_TEXTURE_MIN_FILTER and GL_TEXTURE_MAG_FILTER, which doesn't seem to be a known GL parameter...
			//mxd. but Loki Linux release uses GL_TEXTURE_MIN_FILTER and GL_TEXTURE_MAG_FILTER, just like Q2
			qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_min); //mxd. Q2/H2: qglTexParameterf
			qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max); //mxd. Q2/H2: qglTexParameterf
		}
	}
}

void GL_ImageList_f(void)
{
	NOT_IMPLEMENTED
}

image_t* GL_FindImage(char* name, imagetype_t type)
{
	NOT_IMPLEMENTED
	return NULL;
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