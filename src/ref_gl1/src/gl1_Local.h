//
// gl1_Local.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "ref.h"

#define REF_VERSION	"H2R_GL1 1.0" // H2_1.07: "GL 2.1"; Q2: "GL 0.01".

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

#pragma region ========================== IMPORTED FUNCTIONS ==========================

extern refimport_t ri;

#pragma endregion