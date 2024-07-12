//
// gl_draw_cinematic.c
//
// Copyright 1998 Raven Software
//

#include "gl_local.h"

#define MAX_CINEMATIC_FRAMES 50

//mxd. Reconstructed data type. Original name unknown...
typedef struct CinematicFrame_s
{
	int width;
	int height;
	image_t* image;
} CinematicFrame_t;

static CinematicFrame_t cinematic_frames[MAX_CINEMATIC_FRAMES];
static int num_cinematic_frames;

static int ChopImage(int w, int h)
{
	NOT_IMPLEMENTED
	return 0;
}

void Draw_InitCinematic(const int w, const int h, char* overlay, char* backdrop)
{
	num_cinematic_frames = ChopImage(w, h);
}

// New in H2
void Draw_CloseCinematic(void)
{
	int i;
	CinematicFrame_t* info;

	for (i = 0, info = cinematic_frames; i < num_cinematic_frames; i++, info++)
	{
		info->image->palette = NULL; //TODO: GL_FreeImageNoHash frees image->palette. Explicitly set to NULL here to avoid that?
		GL_FreeImageNoHash(info->image);
	}

	num_cinematic_frames = 0;
}

void Draw_Cinematic(int cols, int rows, byte* data, paletteRGB_t* palette, float alpha)
{
	NOT_IMPLEMENTED
}