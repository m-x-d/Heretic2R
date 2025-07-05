//
// gl1_Draw.c
//
// Copyright 1998 Raven Software
//

#include "gl1_Draw.h"
#include "gl1_Image.h"

image_t* draw_chars;

//mxd. Each font contains 224 char definitions.
glxy_t* font1; // H2
glxy_t* font2; // H2

static void InitFonts(void) // H2
{
	ri.FS_LoadFile("pics/misc/font1.fnt", (void**)&font1);
	ri.FS_LoadFile("pics/misc/font2.fnt", (void**)&font2);
}

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

static image_t* Draw_FindPicFilter(const char* name) // H2
{
	NOT_IMPLEMENTED
	return NULL;
}

void Draw_InitLocal(void)
{
	r_notexture = NULL;
	r_notexture = R_FindImage("textures/general/notex.m8", it_wall);
	if (r_notexture == NULL)
		ri.Sys_Error(ERR_DROP, "Draw_InitLocal: could not find textures/general/notex.m8"); //mxd. Sys_Error() -> ri.Sys_Error().

	draw_chars = Draw_FindPic("misc/conchars.m32");
	r_particletexture = Draw_FindPicFilter("misc/particle.m32");
	r_aparticletexture = Draw_FindPicFilter("misc/aparticle.m8");
	r_font1 = Draw_FindPic("misc/font1.m32");
	r_font2 = Draw_FindPic("misc/font2.m32");
	r_reflecttexture = Draw_FindPicFilter("misc/reflect.m32");

	InitFonts();
}

void Draw_Char(const int x, const int y, int c, const paletteRGBA_t color)
{
	NOT_IMPLEMENTED
}

image_t* Draw_FindPic(const char* name)
{
	if (name[0] != '/' && name[0] != '\\')
	{
		char fullname[MAX_QPATH];
		Com_sprintf(fullname, sizeof(fullname), "pics/%s", name); // Q2: pics/%s.pcx

		return R_FindImage(fullname, it_pic);
	}

	return R_FindImage(name + 1, it_pic);
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