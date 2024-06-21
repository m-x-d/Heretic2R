//
// gl_draw.c
//
// Copyright 1998 Raven Software
//

#include "gl_local.h"

image_t* draw_chars;

void Draw_InitLocal(void)
{
	NOT_IMPLEMENTED
}

void Draw_Char(int x, int y, int c, paletteRGBA_t color)
{
	NOT_IMPLEMENTED
}

// New in H2
void Draw_BigFont(int x, int y, char* text, float alpha)
{
	NOT_IMPLEMENTED
}

// New in H2. BigFont_Strlen?
int BF_Strlen(char* text)
{
	NOT_IMPLEMENTED
	return 0;
}

image_t* Draw_FindPic(char* name)
{
	NOT_IMPLEMENTED
	return NULL;
}

void Draw_GetPicSize(int* w, int* h, char* name)
{
	NOT_IMPLEMENTED
}

void Draw_StretchPic(int x, int y, int w, int h, char* name, float alpha, qboolean scale)
{
	NOT_IMPLEMENTED
}

void Draw_Pic(int x, int y, char* name, float alpha)
{
	NOT_IMPLEMENTED
}

void Draw_TileClear(int x, int y, int w, int h, char* pic)
{
	NOT_IMPLEMENTED
}

void Draw_Fill(int x, int y, int w, int h, byte r, byte g, byte b)
{
	NOT_IMPLEMENTED
}

void Draw_FadeScreen(paletteRGBA_t color)
{
	NOT_IMPLEMENTED
}

void Draw_BookPic(int w, int h, char* name, float scale)
{
	NOT_IMPLEMENTED
}

void Draw_Name(vec3_t origin, char* name, paletteRGBA_t color)
{
	NOT_IMPLEMENTED
}