//
// gl_image.c -- image loading and caching
//
// Copyright 1998 Raven Software
//

#include "gl_local.h"

image_t gltextures[MAX_GLTEXTURES];
int numgltextures;

#define NUM_HASHED_GLTEXTURES	256

static image_t* gltextures_hashed[NUM_HASHED_GLTEXTURES]; // H2
qboolean disablerendering; // H2
static qboolean uploaded_paletted; // H2 //TODO: was used only by qglColorTableEXT logic? Remove?

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

		if (inf < 128.0f)
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

//mxd. Part of GL_LoadPic logic in Q2
image_t* GL_GetFreeImage(void)
{
	int i;
	image_t* image;

	// Find a free image_t
	for (i = 0, image = gltextures; i < numgltextures; i++, image++)
		if (image->registration_sequence == 0)
			break;

	if (i == numgltextures)
	{
		if (numgltextures == MAX_GLTEXTURES)
			Sys_Error("GL_GetFreeImage : No image_t slots free.\n");

		numgltextures++;
	}

	memset(image, 0, sizeof(image_t));

	return image;
}

//mxd. Removed qglSelectTextureSGIS logic
void GL_EnableMultitexture(const qboolean enable)
{
	if (qglActiveTextureARB != NULL)
	{
		GL_SelectTexture(GL_TEXTURE1);
		qglToggle(GL_TEXTURE_2D, enable);
		GL_TexEnv(GL_REPLACE);

		GL_SelectTexture(GL_TEXTURE0);
		GL_TexEnv(GL_REPLACE);
	}
}

//mxd. Removed qglSelectTextureSGIS logic
void GL_SelectTexture(const GLenum texture)
{
	if (qglActiveTextureARB != NULL)
	{
		const int tmu = (texture == GL_TEXTURE1);
		if (tmu != gl_state.currenttmu)
		{
			gl_state.currenttmu = tmu;
			qglActiveTextureARB(texture);
			// Missing: qglClientActiveTextureARB(texture);
		}
	}
}

// Q2 counterpart
void GL_TexEnv(const GLint mode)
{
	static GLint lastmodes[2] = { -1, -1 };

	if (mode != lastmodes[gl_state.currenttmu])
	{
		qglTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, mode); //mxd. Q2/H2: qglTexEnvf 
		lastmodes[gl_state.currenttmu] = mode;
	}
}

// Q2 counterpart
void GL_Bind(int texnum)
{
	if ((int)gl_nobind->value && draw_chars != NULL) // Performance evaluation option
		texnum = draw_chars->texnum;

	if (gl_state.currenttextures[gl_state.currenttmu] != texnum)
	{
		gl_state.currenttextures[gl_state.currenttmu] = texnum;
		qglBindTexture(GL_TEXTURE_2D, texnum);
	}
}

//mxd. Most likely was changed from GL_Bind in H2 to use image->palette in qglColorTableEXT logic (which we skip...)
void GL_BindImage(const image_t* image)
{
	int texnum;

	if ((int)gl_nobind->value && draw_chars != NULL) // Performance evaluation option
		texnum = draw_chars->texnum;
	else
		texnum = image->texnum;

	if (gl_state.currenttextures[gl_state.currenttmu] != texnum)
	{
		//mxd. Skipping qglColorTableEXT logic

		gl_state.currenttextures[gl_state.currenttmu] = texnum;
		qglBindTexture(GL_TEXTURE_2D, texnum);
	}
}

// Q2 counterpart
void GL_MBind(const GLenum target, const int texnum)
{
	GL_SelectTexture(target);
	if (gl_state.currenttextures[target == GL_TEXTURE1] != texnum)
		GL_Bind(texnum);
}

void GL_MBindImage(const GLenum target, const image_t* image)
{
	GL_SelectTexture(target);
	if (gl_state.currenttextures[target == GL_TEXTURE1] != image->texnum)
		GL_BindImage(image);
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

void GL_TextureMode(const char* string)
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

			qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_min); // H2_1.07: GL_TEXTURE_MIN_FILTER -> 0x84fe //mxd. Q2/H2: qglTexParameterf
			qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max); // H2_1.07: GL_TEXTURE_MAG_FILTER -> 0x84fe //mxd. Q2/H2: qglTexParameterf
		}
	}
}

void GL_SetFilter(const image_t* image)
{
	//mxd. Q2/H2: qglTexParameterf
	switch (image->type)
	{
		case it_pic:
			qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // H2_1.07: GL_LINEAR 
			qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // H2_1.07: GL_LINEAR
			break;

		case it_sky:
			qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_max); // H2_1.07: GL_TEXTURE_MIN_FILTER -> 0x84fe
			qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max); // H2_1.07: GL_TEXTURE_MAG_FILTER -> 0x84fe
			break;

		default:
			qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_min);
			qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);
			break;
	}
}

void GL_ImageList_f(void)
{
	int i;
	image_t* image;

	int tex_count = 0;
	int tex_texels = 0;
	int sky_count = 0;
	int sky_texels = 0;
	int skin_count = 0;
	int skin_texels = 0;
	int sprite_count = 0;
	int sprite_texels = 0;
	int pic_count = 0;
	int pic_texels = 0;
	
	char* palstrings[] = { "RGB", "PAL" };

	Com_Printf("---------------------------\n");

	for (i = 0, image = gltextures; i < numgltextures; i++, image++)
	{
		switch (image->type)
		{
			case it_skin:
				Com_Printf("M");
				skin_count++;
				skin_texels += image->width * image->height;
				break;

			case it_sprite:
				Com_Printf("S");
				sprite_count++;
				sprite_texels += image->width * image->height;
				break;

			//mxd. Original code also handles types 3 and 7 here. These aren't used anywhere else in the code.
			case it_wall:
				Com_Printf("W");
				tex_count++;
				tex_texels += (image->width * image->height * 4) / 3;
				break;

			case it_pic:
				Com_Printf("P");
				pic_count++;
				pic_texels += image->width * image->height;
				break;

			case it_sky:
				Com_Printf("K"); //mxd. Was also "P" in original logic.
				sky_count++;
				sky_texels += image->width * image->height;
				break;

			default: //mxd. Added to silence compiler warning
				Com_Printf("U%i", image->type, image->name);
				break;
		}

		Com_Printf(" %3i %3i %s %s\n", image->width, image->height, palstrings[image->palette != NULL], image->name);
	}
	
	Com_Printf("-------------------------------\n");
	Com_Printf("Total skin   : %i (%i texels)\n", skin_count, skin_texels);
	Com_Printf("Total world  : %i (%i texels)\n", tex_count, tex_texels);
	Com_Printf("Total sky    : %i (%i texels)\n", sky_count, sky_texels);
	Com_Printf("Total sprite : %i (%i texels)\n", sprite_count, sprite_texels);
	Com_Printf("Total pic    : %i (%i texels)\n", pic_count, pic_texels);
	Com_Printf("-------------------------------\n");
}

#pragma region ========================== .M8 LOADING ==========================

//mxd. Somewhat similar to Q2's GL_Upload8()
void GL_UploadPaletted(const int level, const byte* data, const paletteRGB_t* palette, const int width, const int height) // H2
{
	paletteRGBA_t trans[256 * 256];

	//mxd. Skipping qglColorTableEXT logic

	const uint size = width * height;

	//mxd. Added sanity check
	if (size > sizeof(trans) / 4)
		Sys_Error("GL_UploadPaletted : Image is too large (%i x %i).\n", width, height);

	for (uint i = 0; i < size; i++)
	{
		const paletteRGB_t* src_p = palette + data[i];
		paletteRGBA_t* dst_p = &trans[i];

		// Copy rgb components
		dst_p->r = src_p->r;
		dst_p->g = src_p->g;
		dst_p->b = src_p->b;
		dst_p->a = 255;
	}

	qglTexImage2D(GL_TEXTURE_2D, level, GL_TEX_SOLID_FORMAT, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, trans);
}

static void GrabPalette(paletteRGB_t* src, paletteRGB_t* dst) // H2
{
	int i;
	paletteRGB_t* src_p;
	paletteRGB_t* dst_p;

	for (i = 0, src_p = src, dst_p = dst; i < PAL_SIZE; i++, src_p++, dst_p++)
	{
		dst_p->r = gammatable[src_p->r];
		dst_p->g = gammatable[src_p->g];
		dst_p->b = gammatable[src_p->b];
	}
}

static int GL_GetMipLevel8(const miptex_t* mt, const imagetype_t type) // H2
{
	int mip = (int)(type == it_skin ? gl_skinmip->value : gl_picmip->value);
	mip = ClampI(mip, 0, MIPLEVELS - 1);
	while (mip > 0 && (mt->width[mip] == 0 || mt->height[mip] == 0)) //mxd. Added mip > 0 sanity check
		mip--;

	return mip;
}

static void GL_Upload8M(miptex_t* mt, const image_t* image) // H2
{
	uploaded_paletted = false;

	int mip = GL_GetMipLevel8(mt, image->type);

	for (int level = 0; mip < MIPLEVELS; mip++, level++)
	{
		if (mt->width[mip] == 0 || mt->height[mip] == 0)
			break;

		GL_UploadPaletted(level, (byte*)mt + mt->offsets[mip], image->palette, (int)mt->width[mip], (int)mt->height[mip]);
	}

	GL_SetFilter(image);
}

// Actually loads .M8 image.
static image_t* GL_LoadWal(const char* name, const imagetype_t type)
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

	paletteRGB_t* palette = malloc(768);
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

#pragma endregion

#pragma region ========================== .M32 LOADING ==========================

static void GL_ApplyGamma32(miptex32_t* mt) // H2
{
	for (int mip = 0; mip < MIPLEVELS - 1; mip++) //TODO: last mip level is skipped. Unintentional?
	{
		const uint mip_size = mt->width[mip] * mt->height[mip];
		if (mip_size == 0)
			return;

		// Adjust RGBA colors at offset...
		paletteRGBA_t* color = (paletteRGBA_t*)((byte*)mt + mt->offsets[mip]);
		for (uint i = 0; i < mip_size; i++, color++)
		{
			color->r = gammatable[color->r];
			color->g = gammatable[color->g];
			color->b = gammatable[color->b];
		}
	}
}

//mxd. Same logic as in GL_GetMipLevel8(), but for miptex32_t...
static int GL_GetMipLevel32(const miptex32_t* mt, const imagetype_t type) // H2
{
	int mip = (int)(type == it_skin ? gl_skinmip->value : gl_picmip->value);
	mip = ClampI(mip, 0, MIPLEVELS - 1);
	while (mip > 0 && (mt->width[mip] == 0 || mt->height[mip] == 0)) //mxd. Added mip > 0 sanity check
		mip--;

	return mip;
}

static void GL_Upload32M(miptex32_t* mt, const image_t* img) // H2
{
	uploaded_paletted = false;

	int mip = GL_GetMipLevel32(mt, img->type);

	for (int level = 0; mip < MIPLEVELS; mip++, level++)
	{
		if (mt->width[mip] == 0 || mt->height[mip] == 0)
			break;

		qglTexImage2D(GL_TEXTURE_2D, level, GL_TEX_ALPHA_FORMAT, (int)mt->width[mip], (int)mt->height[mip], 0, GL_RGBA, GL_UNSIGNED_BYTE, (byte*)mt + mt->offsets[mip]);
	}

	GL_SetFilter(img);
}

//mxd. Loads .M32 image.
static image_t* GL_LoadWal32(const char* name, const imagetype_t type) // H2
{
	miptex32_t* mt;

	ri.FS_LoadFile(name, (void**)&mt);
	if (mt == NULL)
	{
		Com_Printf("GL_LoadWal32 : Can\'t load %s\n", name);
		return NULL;
	}

	if (mt->version != MIP32_VERSION)
	{
		Com_Printf("GL_LoadWal32 : Invalid version for %s\n", name);
		ri.FS_FreeFile(mt); //mxd

		return NULL;
	}

	if (strlen(name) >= MAX_QPATH)
	{
		Com_Printf("GL_LoadWal32 : \"%s\" is too long a string\n", name);
		ri.FS_FreeFile(mt); //mxd

		return NULL;
	}

	GL_ApplyGamma32(mt);

	image_t* image = GL_GetFreeImage();
	strcpy_s(image->name, sizeof(image->name), name);
	image->registration_sequence = registration_sequence;
	image->width = (int)mt->width[0];
	image->height = (int)mt->height[0];
	image->type = type;
	image->palette = NULL;
	image->has_alpha = 1;
	image->texnum = TEXNUM_IMAGES + (image - gltextures);
	image->num_frames = (byte)mt->value;

	GL_BindImage(image);
	GL_Upload32M(mt, image);
	ri.FS_FreeFile(mt);

	return image;
}

#pragma endregion

// Now with name hashing. When no texture found, returns r_notexture instead of NULL
image_t* GL_FindImage(const char* name, const imagetype_t type)
{
	//mxd. Skipping new gl_lostfocus_broken logic
	//if (disablerendering && gl_lostfocus_broken->value)
		//return r_notexture;

	if (name == NULL)
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
	if (strcmp(name + len - 3, ".m8") == 0)
		image = GL_LoadWal(name, type);
	else if (strcmp(name + len - 4, ".m32") == 0)
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

// H2: new 'retval' arg.
struct image_s* R_RegisterSkin(const char* name, qboolean* retval)
{
	image_t* img = GL_FindImage(name, it_skin);
	if (retval != NULL)
		*retval = (img != r_notexture);

	return img;
}

static void GL_FreeImage(image_t* image) // H2
{
	image_t* img;
	image_t** tgt;

	// Delete GL texture
	qglDeleteTextures(1, (GLuint*)&image->texnum);
	if (image->palette)
	{
		free(image->palette);
		image->palette = NULL;
	}

	// Remove from hash
	const uint len = strlen(image->name);
	const byte hash = image->name[len - 7] + image->name[len - 5] * image->name[len - 6];

	for (img = gltextures_hashed[hash], tgt = &gltextures_hashed[hash]; img != image; img = img->next)
		tgt = &img->next;

	*tgt = image->next;
	image->registration_sequence = 0;
}

void GL_FreeImageNoHash(image_t* image)
{
	qglDeleteTextures(1, (GLuint*)&image->texnum);
	if (image->palette != NULL)
	{
		free(image->palette);
		image->palette = NULL;
	}

	image->registration_sequence = 0;
}

void GL_FreeUnusedImages(void)
{
	int i;
	image_t* image;

	// Never free r_notexture or particle texture
	r_notexture->registration_sequence = registration_sequence;
	r_particletexture->registration_sequence = registration_sequence;

	// H2: extra never-to-free textures:
	r_aparticletexture->registration_sequence = registration_sequence;
	r_reflecttexture->registration_sequence = registration_sequence;
	draw_chars->registration_sequence = registration_sequence;
	r_font1->registration_sequence = registration_sequence;
	r_font2->registration_sequence = registration_sequence;

	for (i = 0, image = gltextures; i < numgltextures; i++, image++)
	{
		// Used in this sequence
		if (image->registration_sequence == registration_sequence)
			continue;

		// Free image_t slot
		if (!image->registration_sequence)
			continue;

		// Missing: it_pic check

		// Free it
		GL_FreeImage(image);
	}
}

static void GL_RefreshImage(image_t* image) // H2
{
	qglDeleteTextures(1, (GLuint*)&image->texnum);

	const uint len = strlen(image->name);
	if (strcmp(&image->name[len - 3], ".m8") == 0)
	{
		miptex_t* mt;
		ri.FS_LoadFile(image->name, (void**)&mt);

		GrabPalette(mt->palette, image->palette);
		GL_BindImage(image);
		GL_Upload8M(mt, image);

		ri.FS_FreeFile(mt);
	}
	else if (strcmp(&image->name[len - 4], ".m32") == 0)
	{
		miptex32_t* mt;
		ri.FS_LoadFile(image->name, (void**)&mt);

		GL_ApplyGamma32(mt);
		GL_BindImage(image);
		GL_Upload32M(mt, image);

		ri.FS_FreeFile(mt);
	}
}

void GL_GammaAffect(void)
{
	int i;
	image_t* image;

	GL_FreeUnusedImages();

	for (i = 0, image = gltextures; i < numgltextures; i++, image++)
	{
		if (image->registration_sequence == 0)
			continue;

		if (image->type == it_pic || image->type == it_sky || !(int)menus_active->value)
			GL_RefreshImage(image);
	}
}

void GL_InitImages(void)
{
	registration_sequence = 1;
	gl_state.inverse_intensity = 1.0f;
}

void GL_ShutdownImages(void)
{
	int i;
	image_t* image;

	for (i = 0, image = gltextures; i < numgltextures; i++, image++)
		if (image->registration_sequence != 0)
			GL_FreeImage(image);
}

void GL_DisplayHashTable(void)
{
	int i;
	image_t** gl;

	int total_count = 0;
	int hashed_count = 0;

	for (i = 0, gl = gltextures_hashed; i < NUM_HASHED_GLTEXTURES; i++, gl++)
	{
		const image_t* image = *gl;
		if (image != NULL)
		{
			while (image != NULL)
			{
				image = image->next;
				total_count++;
			}

			hashed_count++;
		}
	}

	Com_Printf("Hash entries: %d, Total images: %d\n", hashed_count, total_count);
}