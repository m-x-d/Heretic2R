//
// gl_draw.c
//
// Copyright 1998 Raven Software
//

#include "gl_local.h"

image_t* draw_chars;

glxy_t* font1; // New in H2
glxy_t* font2; // New in H2

// New in H2
void InitFonts(void)
{
	ri.FS_LoadFile("pics/misc/font1.fnt", (void**)&font1);
	ri.FS_LoadFile("pics/misc/font2.fnt", (void**)&font2);
}

// New in H2
void ShutdownFonts(void)
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

image_t* Draw_FindPicFilter(char* name);

void Draw_InitLocal(void)
{
	r_notexture = NULL;
	r_notexture = GL_FindImage("textures/general/notex.m8", it_wall2);
	if (r_notexture == NULL)
		Sys_Error("Draw_InitLocal: could not find textures/general/notex.m8");

	draw_chars = Draw_FindPic("misc/conchars.m32");
	r_particletexture = Draw_FindPicFilter("misc/particle.m32");
	r_aparticletexture = Draw_FindPicFilter("misc/aparticle.m8");
	r_font1 = Draw_FindPic("misc/font1.m32");
	r_font2 = Draw_FindPic("misc/font2.m32");
	r_reflecttexture = Draw_FindPicFilter("misc/reflect.m32");

	InitFonts();
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

// New in H2
image_t* Draw_FindPicFilter(char* name)
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