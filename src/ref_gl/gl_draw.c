//
// gl_draw.c
//
// Copyright 1998 Raven Software
//

#include "gl_local.h"

void Draw_Char(int x, int y, int c, paletteRGBA_t color)
{
	Sys_Error("Not implemented!");
}

// New in H2
void Draw_BigFont(int x, int y, char* text, float alpha)
{
	Sys_Error("Not implemented!");
}

// New in H2. BigFont_Strlen?
int BF_Strlen(char* text)
{
	Sys_Error("Not implemented!");
	return 0;
}

image_t* Draw_FindPic(char* name)
{
	Sys_Error("Not implemented!");
	return NULL;
}

void Draw_GetPicSize(int* w, int* h, char* name)
{
	Sys_Error("Not implemented!");
}

void Draw_StretchPic(int x, int y, int w, int h, char* name, float alpha, qboolean scale)
{
	Sys_Error("Not implemented!");
}

void Draw_Pic(int x, int y, char* name, float alpha)
{
	Sys_Error("Not implemented!");
}

void Draw_TileClear(int x, int y, int w, int h, char* pic)
{
	Sys_Error("Not implemented!");
}

void Draw_Fill(int x, int y, int w, int h, byte r, byte g, byte b)
{
	Sys_Error("Not implemented!");
}

void Draw_FadeScreen(paletteRGBA_t color)
{
	Sys_Error("Not implemented!");
}

void Draw_BookPic(int w, int h, char* name, float scale)
{
	Sys_Error("Not implemented!");
}

void Draw_Name(vec3_t origin, char* name, paletteRGBA_t color)
{
	Sys_Error("Not implemented!");
}