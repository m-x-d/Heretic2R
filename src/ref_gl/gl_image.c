//
// gl_image.c -- image loading and caching
//
// Copyright 1998 Raven Software
//

#include "gl_local.h"

image_t gltextures[MAX_GLTEXTURES];
int numgltextures;

image_t* gltextures_hashed[256]; // New in H2
qboolean disablerendering;

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

image_t* GL_GetFreeImage(void)
{
	NOT_IMPLEMENTED
	return NULL;
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
			//mxd. ...but Loki Linux release uses GL_TEXTURE_MIN_FILTER and GL_TEXTURE_MAG_FILTER, just like Q2
			qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_min); //mxd. Q2/H2: qglTexParameterf
			qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max); //mxd. Q2/H2: qglTexParameterf
		}
	}
}

void GL_ImageList_f(void)
{
	NOT_IMPLEMENTED
}

static void GrabPalette(palette_t* src, palette_t* dst)
{
	NOT_IMPLEMENTED
}

static void GL_Upload8M(miptex_t* mt, image_t* image)
{
	NOT_IMPLEMENTED
}

// Actually loads .M8 image.
static image_t* GL_LoadWal(char* name, const imagetype_t type)
{
	miptex_t* mt;
	ri.FS_LoadFile(name, (void**)&mt);

	if (mt == NULL)
	{
		Com_Printf("GL_LoadWal : Can't load %s\n", name);
		return NULL;
	}

	if (mt->version != MIP_VERSION)
	{
		Com_Printf("GL_LoadWal : Invalid version for %s\n", name);
		ri.FS_FreeFile(mt); //mxd

		return NULL;
	}

	if (strlen(name) >= MAX_QPATH)
	{
		Com_Printf("GL_LoadWal : \"%s\" is too long a string\n", name);
		ri.FS_FreeFile(mt); //mxd

		return NULL;
	}

	palette_t* palette = malloc(768);

	if (palette == NULL)
	{
		Com_Printf("GL_LoadWal : Failed to allocate palette for %s\n", name); //mxd
		ri.FS_FreeFile(mt); //mxd

		return NULL;
	}

	GrabPalette(mt->palette, palette);

	image_t* image = GL_GetFreeImage();
	strcpy_s(image->name, sizeof(image->name), name);
	image->registration_sequence = registration_sequence;
	image->width = (int)mt->width[0];
	image->height = (int)mt->height[0];
	image->type = type;
	image->palette = palette;
	image->has_alpha = 0;
	image->texnum = TEXNUM_IMAGES + (image - gltextures);
	image->num_frames = (byte)mt->value;

	GL_BindImage(image);
	GL_Upload8M(mt, image);
	ri.FS_FreeFile(mt);

	return image;
}

// New in H2. Loads .M32 image.
static image_t* GL_LoadWal32(char* name, imagetype_t type)
{
	NOT_IMPLEMENTED
	return NULL;
}

// Now with name hashing. When no texture found, returns r_notexture instead of NULL
image_t* GL_FindImage(char* name, imagetype_t type)
{
	//mxd. Skipping new gl_lostfocus_broken logic
	//if (disablerendering && gl_lostfocus_broken->value)
		//return r_notexture;

	if (!name)
	{
		Com_Printf("GL_FindImage: Invalid null name\n");
		return r_notexture;
	}

	const uint len = strlen(name);

	if (len < 8)
	{
		Com_Printf("GL_FindImage: Name too short (%s)\n", name);
		return r_notexture;
	}

	// Check for hashed image first.
	const byte hash = name[len - 7] + name[len - 5] * name[len - 6];
	image_t* image = gltextures_hashed[hash];

	if (image != NULL)
	{
		while (strcmp(name, image->name) != 0)
		{
			image = image->next;
			if (image == NULL)
				break;
		}

		if (image != NULL)
		{
			image->registration_sequence = registration_sequence;
			return image;
		}
	}

	// Not hashed. Load image from disk.
	if (!strcmp(name + len - 3, ".m8"))
		image = GL_LoadWal(name, type);
	else if (!strcmp(name + len - 4, ".m32"))
		image = GL_LoadWal32(name, type);
	else
		Com_Printf("GL_FindImage: Extension not recognized in %s\n", name);

	if (image == NULL)
		return r_notexture;

	// Add image to hash.
	image->next = gltextures_hashed[hash];
	gltextures_hashed[hash] = image;

	return image;
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