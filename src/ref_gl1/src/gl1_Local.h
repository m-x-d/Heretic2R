//
// gl1_Local.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include <glad-GL1.3/glad.h> // Must be included before SDL.
#include "ref.h"

#define REF_VERSION			GAME_NAME" OpenGL 1.3 Refresher v1.0" // H2_1.07: "GL 2.1"; Q2: "GL 0.01".

#define MAX_TEXTURE_UNITS	2

#pragma region ========================== CVARS  ==========================

#pragma endregion

typedef enum //mxd. Changed in H2
{
	it_skin = 1,
	it_sprite = 2,
	it_wall = 4,
	it_pic = 5,
	it_sky = 6
} imagetype_t;

typedef struct image_s //mxd. Changed in H2. Original size: 104 bytes
{
	struct image_s* next;
	char name[MAX_QPATH];				// Game path, including extension.
	imagetype_t type;
	int width;
	int height;
	int registration_sequence;			// 0 = free
	struct msurface_s* texturechain;	// For sort-by-texture world drawing.
	struct msurface_s* multitexturechain;
	int texnum;							// gl texture binding.
	byte has_alpha;
	byte num_frames;
	struct paletteRGB_s* palette;		// .M8 palette.
} image_t;

#pragma region ========================== GL config stuff  ==========================

typedef struct
{
	const char* renderer_string;
	const char* vendor_string;
	const char* version_string;
	const char* extensions_string;
	float max_anisotropy; // YQ2
} glconfig_t;

typedef struct
{
	float inverse_intensity;
	qboolean fullscreen;

	int prev_mode;
	int lightmap_textures;
	int currenttextures[MAX_TEXTURE_UNITS];
	int currenttmu;
} glstate_t;

extern glconfig_t gl_config;
extern glstate_t gl_state;

#pragma endregion

#pragma region ========================== IMPORTED FUNCTIONS ==========================

extern refimport_t ri;

#pragma endregion