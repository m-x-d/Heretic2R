//
// gl_local.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "ref.h"

typedef enum //mxd. Changed in H2
{
	it_undefined,
	it_skin,
	it_sprite,
	it_wall1, //TODO: better name...
	it_wall2, //TODO: better name...
	it_pic, //mxd. Or it_book?
	it_sky,
	it_wall3 //TODO: better name...
} imagetype_t;

typedef struct image_s //mxd. Changed in H2. Original size: 104 bytes
{
	struct image_s* next;
	char name[MAX_QPATH];				// Game path, including extension
	imagetype_t type;
	int width;
	int height;
	int registration_sequence;			// 0 = free
	struct msurface_t* texturechain;	// For sort-by-texture world drawing
	struct msurface_t* multitexturechain;
	int texnum;							// gl texture binding
	byte has_alpha;
	byte num_frames;
	//byte pad[2];
	struct palette_s* palette;			// .M8 palette
} image_t;

#include "gl_model.h" //mxd. MUST be below image_t definition...

// gl_draw.c
void Draw_GetPicSize(int* w, int* h, char* name);
void Draw_Pic(int x, int y, char* name, float alpha);
void Draw_StretchPic(int x, int y, int w, int h, char* name, float alpha, qboolean scale);
void Draw_Char(int x, int y, int c, paletteRGBA_t color);
void Draw_BigFont(int x, int y, char* text, float alpha); // New in H2
void Draw_TileClear(int x, int y, int w, int h, char* pic);
void Draw_Fill(int x, int y, int w, int h, byte r, byte g, byte b);
void Draw_FadeScreen(paletteRGBA_t color);
void Draw_BookPic(int w, int h, char* name, float scale); // New in H2

int BF_Strlen(char* text); // New in H2

#pragma region ========================== IMPORTED FUNCTIONS ==========================

extern refimport_t ri;

#pragma endregion

#pragma region ========================== IMPLEMENTATION-SPECIFIC FUNCTIONS ==========================

void GLimp_EndFrame(void);
void GLimp_AppActivate(qboolean active);

#pragma endregion