//
// gl1_Draw.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "gl1_Local.h"

// H2. Font character definition struct.
typedef struct glxy_s
{
	float xl;
	float yt;
	float xr;
	float yb;
	int w;
	int h;
	int baseline;
} glxy_t;

extern glxy_t* font1; // H2
extern glxy_t* font2; // H2

extern image_t* draw_chars;

extern void ShutdownFonts(void);

extern void Draw_InitLocal(void);

extern image_t* Draw_FindPic(const char* name);
extern void Draw_GetPicSize(int* w, int* h, const char* name);

extern void Draw_Pic(int x, int y, const char* name, float alpha);
extern void Draw_StretchPic(int x, int y, int w, int h, const char* name, float alpha, qboolean scale);
extern void Draw_TileClear(int x, int y, int w, int h, const char* pic);
extern void Draw_Fill(int x, int y, int w, int h, byte r, byte g, byte b);
extern void Draw_FadeScreen(paletteRGBA_t color);
extern void Draw_Name(const vec3_t origin, const char* name, paletteRGBA_t color);
extern void Draw_Char(int x, int y, int c, paletteRGBA_t color);
extern void Draw_Render(int x, int y, int w, int h, const image_t* image, float alpha);