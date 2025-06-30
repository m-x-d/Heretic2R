//
// gl1_Draw.c
//
// Copyright 1998 Raven Software
//

#include "gl1_Draw.h"

//mxd. Each font contains 224 char definitions.
glxy_t* font1; // H2
glxy_t* font2; // H2

void ShutdownFonts(void) // H2
{
	if (font1 != NULL)
	{
		ri.FS_FreeFile(font1);
		font1 = NULL;
	}

	if (font2 != NULL)
	{
		ri.FS_FreeFile(font2);
		font2 = NULL;
	}
}

void Draw_InitLocal(void)
{
	NOT_IMPLEMENTED
}

void Draw_Char(const int x, const int y, int c, const paletteRGBA_t color)
{
	NOT_IMPLEMENTED
}

image_t* Draw_FindPic(const char* name)
{
	NOT_IMPLEMENTED
	return NULL;
}

void Draw_GetPicSize(int* w, int* h, const char* name)
{
	NOT_IMPLEMENTED
}

void Draw_StretchPic(int x, int y, int w, int h, const char* name, const float alpha, const qboolean scale)
{
	NOT_IMPLEMENTED
}

void Draw_Pic(const int x, const int y, const char* name, const float alpha)
{
	NOT_IMPLEMENTED
}

void Draw_TileClear(const int x, const int y, const int w, const int h, const char* pic)
{
	NOT_IMPLEMENTED
}

void Draw_Fill(const int x, const int y, const int w, const int h, const byte r, const byte g, const byte b)
{
	NOT_IMPLEMENTED
}

void Draw_FadeScreen(const paletteRGBA_t color)
{
	NOT_IMPLEMENTED
}

void Draw_Name(const vec3_t origin, const char* name, const paletteRGBA_t color)
{
	NOT_IMPLEMENTED
}