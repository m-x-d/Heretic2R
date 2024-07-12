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

static int GetCoords(int size, int* image_sizes)
{
	NOT_IMPLEMENTED
	return 0;
}

static int ChopImage(const int width, const int height)
{
	int img_widths[64];
	int img_heights[64];
	
	const int tiles_w = GetCoords(width, img_widths) - 1;
	const int tiles_h = GetCoords(height, img_heights) - 1;

	if (tiles_h < 1)
		return 0;

	CinematicFrame_t* frame = cinematic_frames;

	for (int h = 0; h < tiles_h; h++)
	{
		for (int w = 0; w < tiles_w; w++, frame++)
		{
			image_t* image = GL_GetFreeImage();

			image->registration_sequence = registration_sequence;
			image->width = img_widths[w + 1] - img_widths[w];
			image->height = img_heights[h + 1] - img_heights[h];
			image->type = it_pic;
			image->palette = NULL;
			image->texnum = TEXNUM_IMAGES + (image - gltextures);
			image->has_alpha = 0;

			frame->width = img_widths[w];
			frame->height = img_heights[h];
			frame->image = image;
		}
	}

	return tiles_w * tiles_h;
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