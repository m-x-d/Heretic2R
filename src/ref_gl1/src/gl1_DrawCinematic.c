//
// gl1_DrawCinematic.c
//
// Copyright 1998 Raven Software
//

#include "gl1_DrawCinematic.h"
#include "gl1_Image.h"
#include "gl1_Model.h"

static image_t* cin_frame;

void Draw_InitCinematic(const int width, const int height)
{
	cin_frame = R_GetFreeImage();

	cin_frame->registration_sequence = registration_sequence;
	cin_frame->width = width;
	cin_frame->height = height;
	cin_frame->type = it_pic;
	cin_frame->palette = NULL;
	cin_frame->texnum = TEXNUM_IMAGES + (cin_frame - gltextures);
	cin_frame->has_alpha = false;
}

void Draw_CloseCinematic(void) // H2
{
	cin_frame->palette = NULL; // R_FreeImageNoHash() frees image->palette. Explicitly set to NULL here to avoid that.
	R_FreeImageNoHash(cin_frame);
}

void Draw_Cinematic(const int cols, const int rows, const byte* data, const paletteRGB_t* palette, const float alpha)
{
	NOT_IMPLEMENTED
}