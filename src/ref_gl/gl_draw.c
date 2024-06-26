//
// gl_draw.c
//
// Copyright 1998 Raven Software
//

#include "gl_local.h"
#include "vid.h"

image_t* draw_chars;

//mxd. Each font contains 224 char definitions
glxy_t* font1;
glxy_t* font2;

qboolean gl_alphatest_broken;

// New in H2
void InitFonts(void)
{
	ri.FS_LoadFile("pics/misc/font1.fnt", (void**)&font1);
	ri.FS_LoadFile("pics/misc/font2.fnt", (void**)&font2);
}

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

// Draws one 8*8 graphics character with 0 being transparent.
// It can be clipped to the top of the screen to allow the console to be smoothly scrolled off.
void Draw_Char(const int x, const int y, int c, const paletteRGBA_t color)
{
	#define CELL_SIZE	0.0625f	// 16 chars per row/column (0.0625 == 1 / 16)
	#define CHAR_SIZE	8		// Each char is 8x8 pixels

	c &= 255;

	// Skip when whitespace char or totally off-screen
	if ((c & 127) == 32 || y <= -8)
		return;

	const float frow = (float)(c >> 4) * CELL_SIZE;
	const float fcol = (float)(c & 15) * CELL_SIZE;

	GL_BindImage(draw_chars);
 
	qglEnable(GL_ALPHA_TEST);
	qglEnable(GL_BLEND);
	qglColor4ub(color.r, color.g, color.b, color.a);
	qglBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);
	qglAlphaFunc(GL_GREATER, 0.05f);

	GL_TexEnv(GL_MODULATE);
	qglBegin(GL_QUADS);

	qglTexCoord2f(fcol, frow);
	qglVertex2i(x, y); //mxd. qglVertex2f -> qglVertex2i

	qglTexCoord2f(fcol + CELL_SIZE, frow);
	qglVertex2i(x + CHAR_SIZE, y); //mxd. qglVertex2f -> qglVertex2i

	qglTexCoord2f(fcol + CELL_SIZE, frow + CELL_SIZE);
	qglVertex2i(x + CHAR_SIZE, y + CHAR_SIZE); //mxd. qglVertex2f -> qglVertex2i

	qglTexCoord2f(fcol, frow + CELL_SIZE);
	qglVertex2i(x, y + CHAR_SIZE); //mxd. qglVertex2f -> qglVertex2i

	qglEnd();
	GL_TexEnv(GL_REPLACE);

	qglDisable(GL_ALPHA_TEST);
	qglDisable(GL_BLEND);
}

image_t* Draw_FindPic(char* name)
{
	if (name[0] != '/' && name[0] != '\\')
	{
		char fullname[MAX_QPATH];
		Com_sprintf(fullname, sizeof(fullname), "pics/%s", name); // Q2: pics/%s.pcx

		return GL_FindImage(fullname, it_pic);
	}

	return GL_FindImage(name + 1, it_pic);
}

// New in H2
image_t* Draw_FindPicFilter(char* name)
{
	if (name[0] != '/' && name[0] != '\\')
	{
		char fullname[MAX_QPATH];
		Com_sprintf(fullname, sizeof(fullname), "pics/%s", name);

		return GL_FindImage(fullname, it_sky);
	}

	return GL_FindImage(name + 1, it_sky);
}

void Draw_GetPicSize(int* w, int* h, char* name)
{
	const image_t* image = GL_FindImage(name, it_pic);

	if (image != r_notexture)
	{
		*w = image->width;
		*h = image->height;
	}
	else
	{
		*w = 0; // Q2: -1
		*h = 0; // Q2: -1
	}
}

void Draw_Render(const int x, const int y, const int w, const int h, const image_t* image, const float alpha)
{
	if (gl_alphatest_broken && !image->has_alpha)
		qglDisable(GL_ALPHA_TEST);

	GL_BindImage(image);

	qglColor4f(1.0f, 1.0f, 1.0f, alpha);
	qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	qglAlphaFunc(GL_GREATER, 0.05f);
	qglEnable(GL_BLEND);

	GL_TexEnv(GL_MODULATE);
	qglBegin(GL_QUADS);

	qglTexCoord2f(0.0f, 0.0f);
	qglVertex2i(x, y); //mxd. qglVertex2f -> qglVertex2i

	qglTexCoord2f(1.0f, 0.0f);
	qglVertex2i(x + w, y); //mxd. qglVertex2f -> qglVertex2i

	qglTexCoord2f(1.0f, 1.0f);
	qglVertex2i(x + w, y + h); //mxd. qglVertex2f -> qglVertex2i

	qglTexCoord2f(0.0f, 1.0f);
	qglVertex2i(x, y + h); //mxd. qglVertex2f -> qglVertex2i

	qglEnd();
	GL_TexEnv(GL_REPLACE);

	if (gl_alphatest_broken && !image->has_alpha)
		qglEnable(GL_ALPHA_TEST);
}

void Draw_StretchPic(int x, int y, int w, int h, char* name, const float alpha, const qboolean scale)
{
	const image_t* image = Draw_FindPicFilter(name);

	if (scale)
	{
		const int end_x = x + w;
		const int end_y = y + h;

		x = viddef.width * x / DEF_WIDTH;
		y = viddef.height * y / DEF_HEIGHT;
		w = viddef.width * end_x / DEF_WIDTH - x;
		h = viddef.height * end_y / DEF_HEIGHT - y;
	}

	Draw_Render(x, y, w, h, image, alpha);
}

void Draw_Pic(int x, int y, char* name, float alpha)
{
	NOT_IMPLEMENTED
}

void Draw_TileClear(int x, int y, int w, int h, char* pic)
{
	NOT_IMPLEMENTED
}

void Draw_Fill(const int x, const int y, const int w, const int h, const byte r, const byte g, const byte b)
{
	qglDisable(GL_TEXTURE_2D);

	//mxd. qglColor4f -> qglColor3ub; H2: qglColor4f((float)r / 256.0f, (float)g / 256.0f,(float)b / 256.0f, 1.0f); Q2: color components divided by 255.0
	qglColor3ub(r, g, b); 

	qglBegin(GL_QUADS);

	//mxd. qglVertex2f -> qglVertex2i
	qglVertex2i(x, y);
	qglVertex2i(x + w, y);
	qglVertex2i(x + w, y + h);
	qglVertex2i(x, y + h);

	qglEnd();

	qglColor3f(1.0f, 1.0f, 1.0f); // mxd. qglColor4f -> qglColor3f
	qglEnable(GL_TEXTURE_2D);
}

void Draw_FadeScreen(paletteRGBA_t color)
{
	NOT_IMPLEMENTED
}

void Draw_Name(vec3_t origin, char* name, paletteRGBA_t color)
{
	NOT_IMPLEMENTED
}