//
// gl1_Lightmap.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "gl1_Local.h"

#define LIGHTMAP_BYTES			4

#define LM_BLOCK_WIDTH			128
#define LM_BLOCK_HEIGHT			128

#define MAX_LIGHTMAPS			128
#define MAX_TALLWALL_LIGHTMAPS	512 // H2

#define GL_LIGHTMAP_FORMAT		GL_RGBA

typedef struct
{
	int internal_format;
	int current_lightmap_texture;

	msurface_t* lightmap_surfaces[MAX_LIGHTMAPS];
	msurface_t* tallwall_lightmap_surfaces[MAX_TALLWALL_LIGHTMAPS]; // H2
	int tallwall_lightmaptexturenum; // H2

	int allocated[LM_BLOCK_WIDTH];

	// The lightmap texture data needs to be kept in main memory so texsubimage can update properly.
	byte lightmap_buffer[LM_BLOCK_WIDTH * LM_BLOCK_HEIGHT * 4];
} gllightmapstate_t;

extern gllightmapstate_t gl_lms;

extern void LM_InitBlock(void);
extern void LM_UploadBlock(qboolean dynamic);
extern qboolean LM_AllocBlock(int w, int h, int* x, int* y);

extern void LM_BuildPolygonFromSurface(msurface_t* fa);
extern void LM_CreateSurfaceLightmap(msurface_t* surf);
extern void LM_BeginBuildingLightmaps(void);
extern void LM_EndBuildingLightmaps(void);