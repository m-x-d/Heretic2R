//
// gl_surf.c -- surface-related refresh code
//
// Copyright 1998 Raven Software
//

#include "gl_local.h"

#define LIGHTMAP_BYTES			4

#define BLOCK_WIDTH				128
#define BLOCK_HEIGHT			128

#define MAX_LIGHTMAPS			128
#define MAX_TALLWALL_LIGHTMAPS	512 // New in H2

#define GL_LIGHTMAP_FORMAT		GL_RGBA

typedef struct
{
	int internal_format;
	int	current_lightmap_texture;

	msurface_t* lightmap_surfaces[MAX_LIGHTMAPS];
	msurface_t* tallwall_lightmap_surfaces[MAX_TALLWALL_LIGHTMAPS]; // New in H2
	int tallwall_lightmaptexturenum; // New in H2

	int allocated[BLOCK_WIDTH];

	// The lightmap texture data needs to be kept in main memory so texsubimage can update properly
	byte lightmap_buffer[BLOCK_WIDTH * BLOCK_HEIGHT * 4];
} gllightmapstate_t;

static gllightmapstate_t gl_lms;

extern void R_SetCacheState(msurface_t* surf);
extern void R_BuildLightMap(msurface_t* surf, byte* dest, int stride);

// Q2 counterpart. //TODO: parameter 'm' is never used
void GL_BeginBuildingLightmaps(model_t* m)
{
	static lightstyle_t	lightstyles[MAX_LIGHTSTYLES];
	static uint dummy[BLOCK_WIDTH * BLOCK_HEIGHT]; //mxd. Made it static.
	
	memset(gl_lms.allocated, 0, sizeof(gl_lms.allocated));

	r_framecount = 1; // No dlightcache.

	GL_EnableMultitexture(true);
	GL_SelectTexture(GL_TEXTURE1);

	// Setup the base lightstyles so the lightmaps won't have to be regenerated the first time they're seen.
	for (int i = 0; i < MAX_LIGHTSTYLES; i++)
	{
		lightstyles[i].rgb[0] = 1.0f;
		lightstyles[i].rgb[1] = 1.0f;
		lightstyles[i].rgb[2] = 1.0f;
		lightstyles[i].white = 3.0f;
	}

	r_newrefdef.lightstyles = lightstyles;
	gl_state.lightmap_textures = TEXNUM_LIGHTMAPS; //mxd. Set only here, never changed.
	gl_lms.current_lightmap_texture = 1;

	//mxd. Skip gl_monolightmap logic.
	gl_lms.internal_format = GL_TEX_SOLID_FORMAT;

	// Initialize the dynamic lightmap texture.
	GL_Bind(gl_state.lightmap_textures);

	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //mxd. qglTexParameterf -> qglTexParameteri
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //mxd. qglTexParameterf -> qglTexParameteri
	qglTexImage2D(GL_TEXTURE_2D, 0, gl_lms.internal_format, BLOCK_WIDTH, BLOCK_HEIGHT, 0, GL_LIGHTMAP_FORMAT, GL_UNSIGNED_BYTE, dummy);
}

void GL_EndBuildingLightmaps(void)
{
	NOT_IMPLEMENTED
}

static void LM_InitBlock(void)
{
	NOT_IMPLEMENTED
}

static void LM_UploadBlock(qboolean dynamic)
{
	NOT_IMPLEMENTED
}

static qboolean LM_AllocBlock(int w, int h, int* x, int* y)
{
	NOT_IMPLEMENTED
	return false;
}

void GL_BuildPolygonFromSurface(msurface_t* fa)
{
	NOT_IMPLEMENTED
}

// Q2 counterpart
void GL_CreateSurfaceLightmap(msurface_t* surf)
{
	if (surf->flags & (SURF_DRAWSKY | SURF_DRAWTURB))
		return;

	const int smax = ((int)surf->extents[0] >> 4) + 1;
	const int tmax = ((int)surf->extents[1] >> 4) + 1;

	if (!LM_AllocBlock(smax, tmax, &surf->light_s, &surf->light_t))
	{
		LM_UploadBlock(false);
		LM_InitBlock();

		if (!LM_AllocBlock(smax, tmax, &surf->light_s, &surf->light_t))
			ri.Sys_Error(ERR_FATAL, "Consecutive calls to LM_AllocBlock(%d,%d) failed\n", smax, tmax);
	}

	surf->lightmaptexturenum = gl_lms.current_lightmap_texture;

	byte* base = gl_lms.lightmap_buffer;
	base += (surf->light_t * BLOCK_WIDTH + surf->light_s) * LIGHTMAP_BYTES;

	R_SetCacheState(surf);
	R_BuildLightMap(surf, base, BLOCK_WIDTH * LIGHTMAP_BYTES);
}