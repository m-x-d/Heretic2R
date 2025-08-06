//
// gl1_DrawCinematic.c
//
// Copyright 1998 Raven Software
//

#include "gl1_DrawCinematic.h"
#include "gl1_Draw.h"
#include "gl1_Image.h"
#include "vid.h"

static image_t* cin_frame;
static byte* cin_frame_data;

void Draw_InitCinematic(const int width, const int height) // H2
{
	cin_frame = R_GetFreeImage();

	cin_frame->registration_sequence = registration_sequence;
	cin_frame->width = width;
	cin_frame->height = height;
	cin_frame->type = it_pic;
	cin_frame->palette = malloc(sizeof(paletteRGB_t) * 256);
	cin_frame->has_alpha = false;
	cin_frame->texnum = TEXNUM_IMAGES + (cin_frame - gltextures);

	cin_frame_data = malloc(width * height);
}

void Draw_CloseCinematic(void) // H2
{
	R_FreeImageNoHash(cin_frame); // Will also free palette.
	cin_frame = NULL;

	free(cin_frame_data);
	cin_frame_data = NULL;
}

void Draw_Cinematic(const byte* data, const paletteRGB_t* palette) // H2
{
	// Draw black BG over whole screen.
	Draw_Fill(0, 0, viddef.width, viddef.height, TextPalette[P_BLACK]);

	// Copy frame palette and data.
	memcpy(cin_frame->palette, palette, sizeof(paletteRGB_t) * 256);
	memcpy(cin_frame_data, data, cin_frame->width * cin_frame->height);

	R_BindImage(cin_frame);
	R_UploadPaletted(0, cin_frame_data, cin_frame->palette, cin_frame->width, cin_frame->height);
	R_SetFilter(cin_frame);

	// Scale to screen, preserve aspect ratio...
	const int scaler = min(viddef.width / cin_frame->width, viddef.height / cin_frame->height);
	const int w = cin_frame->width * scaler;
	const int h = cin_frame->height * scaler;
	const int x = (int)(roundf((float)(viddef.width - w) * 0.5f));
	const int y = (int)(roundf((float)(viddef.height - h) * 0.5f));

	Draw_Render(x, y, w, h, cin_frame, 1.0f);
}