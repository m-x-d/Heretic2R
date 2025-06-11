//
// gl_draw.c
//
// Copyright 1998 Raven Software
//

#include "gl_local.h"
#include "Vector.h"
#include "vid.h"

image_t* draw_chars;

//mxd. Each font contains 224 char definitions
glxy_t* font1;
glxy_t* font2;

qboolean gl_alphatest_broken;

static void InitFonts(void) // H2
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

static image_t* Draw_FindPicFilter(const char* name) // H2
{
	if (name[0] != '/' && name[0] != '\\')
	{
		char fullname[MAX_QPATH];
		Com_sprintf(fullname, sizeof(fullname), "pics/%s", name);

		return GL_FindImage(fullname, it_sky);
	}

	return GL_FindImage(name + 1, it_sky);
}

void Draw_InitLocal(void)
{
	r_notexture = NULL;
	r_notexture = GL_FindImage("textures/general/notex.m8", it_wall);
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
	qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // H2_1.07: GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR 
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

image_t* Draw_FindPic(const char* name)
{
	if (name[0] != '/' && name[0] != '\\')
	{
		char fullname[MAX_QPATH];
		Com_sprintf(fullname, sizeof(fullname), "pics/%s", name); // Q2: pics/%s.pcx

		return GL_FindImage(fullname, it_pic);
	}

	return GL_FindImage(name + 1, it_pic);
}

void Draw_GetPicSize(int* w, int* h, const char* name)
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

void Draw_StretchPic(int x, int y, int w, int h, const char* name, const float alpha, const qboolean scale)
{
	const image_t* image = Draw_FindPicFilter(name);

	if (scale)
	{
		const int xr = x + w;
		const int yb = y + h;

		x = viddef.width * x / DEF_WIDTH;
		y = viddef.height * y / DEF_HEIGHT;
		w = viddef.width * xr / DEF_WIDTH - x;
		h = viddef.height * yb / DEF_HEIGHT - y;
	}

	Draw_Render(x, y, w, h, image, alpha);
}

void Draw_Pic(const int x, const int y, const char* name, const float alpha)
{
	const image_t* pic = Draw_FindPic(name);
	Draw_Render(x, y, pic->width, pic->height, pic, alpha);
}

//mxd. Used in SCR_TileClear frame border drawing logic. //TODO: remove?
void Draw_TileClear(const int x, const int y, const int w, const int h, const char* pic)
{
	const image_t* image = Draw_FindPic(pic);

	if (gl_alphatest_broken && !image->has_alpha)
		qglDisable(GL_ALPHA_TEST);

	GL_BindImage(image);

	//mxd. Divided by 64 in Q2
	const float sl = (float)x / 128.0f;
	const float sr = (float)(x + w) / 128.0f;
	const float tt = (float)y / 128.0f;
	const float tb = (float)(y + h) / 128.0f;

	qglBegin(GL_QUADS);

	qglTexCoord2f(sl, tt);
	qglVertex2i(x, y); //mxd. qglVertex2f -> qglVertex2i

	qglTexCoord2f(sr, tt);
	qglVertex2i(x + w, y); //mxd. qglVertex2f -> qglVertex2i

	qglTexCoord2f(sr, tb);
	qglVertex2i(x + w, y + h); //mxd. qglVertex2f -> qglVertex2i

	qglTexCoord2f(sl, tb);
	qglVertex2i(x, y + h); //mxd. qglVertex2f -> qglVertex2i

	qglEnd();

	if (gl_alphatest_broken && !image->has_alpha)
		qglEnable(GL_ALPHA_TEST);
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

void Draw_FadeScreen(const paletteRGBA_t color)
{
	qglEnable(GL_BLEND);
	qglEnable(GL_ALPHA_TEST);
	qglDisable(GL_TEXTURE_2D);
	qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // H2_1.07: GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR
	qglAlphaFunc(GL_GREATER, 0.05f);
	qglColor4ubv(color.c_array); //mxd. qglColor4ub -> qglColor4ubv

	qglBegin(GL_QUADS);

	const int x = r_newrefdef.x;
	const int y = viddef.height - r_newrefdef.y - r_newrefdef.height;
	const int w = r_newrefdef.width;
	const int h = r_newrefdef.height;

	//mxd. qglVertex2f -> qglVertex2i
	qglVertex2i(x, y); 
	qglVertex2i(x + w, y);
	qglVertex2i(x + w, y + h);
	qglVertex2i(x, y + h);

	qglEnd();

	qglColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	qglEnable(GL_TEXTURE_2D);
	qglDisable(GL_ALPHA_TEST);
	qglDisable(GL_BLEND);
}

void Draw_Name(const vec3_t origin, const char* name, const paletteRGBA_t color)
{
	vec3_t diff;
	vec3_t screen_pos;

	VectorSubtract(origin, r_origin, diff);
	TransformVector(diff, screen_pos);

	if (screen_pos[2] < 0.01f)
		return;

	const int len = (int)strlen(name);
	const float center_x = (float)r_newrefdef.width * 0.5f;
	const float center_y = (float)r_newrefdef.height * 0.5f;
	const float scaler = center_x / screen_pos[2] * 1.28f;

	int x = (int)(center_x + screen_pos[0] * scaler) - len * 4;
	const int y = (int)(center_y - screen_pos[1] * scaler);

	if (x < 0 || y < 0 || x + len * 8 > r_newrefdef.width || y + 8 > r_newrefdef.height)
		return;

	for (int i = 0; i < len; i++)
	{
		Draw_Char(x, y, name[i], color);
		x += 8;
	}
}