//
// gl_draw_book.c
//
// Copyright 1998 Raven Software
//

#include "gl_local.h"
#include "vid.h"

//mxd. Not part of original logic
static void Draw_BigFontChar(int x, int y, const glxy_t* char_def)
{
	y -= char_def->baseline;

	const int xl = viddef.width * x / DEF_WIDTH;
	const int xr = viddef.width * (char_def->w + x) / DEF_WIDTH;
	const int yt = viddef.height * y / DEF_HEIGHT;
	const int yb = viddef.height * (char_def->h + y) / DEF_HEIGHT;

	qglBegin(GL_QUADS);

	qglTexCoord2f(char_def->xl, char_def->yt);
	qglVertex2i(xl, yt); //mxd. qglVertex2f -> qglVertex2i

	qglTexCoord2f(char_def->xr, char_def->yt);
	qglVertex2i(xr, yt); //mxd. qglVertex2f -> qglVertex2i

	qglTexCoord2f(char_def->xr, char_def->yb);
	qglVertex2i(xr, yb); //mxd. qglVertex2f -> qglVertex2i

	qglTexCoord2f(char_def->xl, char_def->yb);
	qglVertex2i(xl, yb); //mxd. qglVertex2f -> qglVertex2i

	qglEnd();
}

void Draw_BigFont(const int x, const int y, const char* text, const float alpha)
{
	if (font1 == NULL || font2 == NULL)
		return;

	qglEnable(GL_ALPHA_TEST);
	qglEnable(GL_BLEND);
	qglColor4f(1.0f, 1.0f, 1.0f, alpha);
	qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	qglAlphaFunc(GL_GREATER, 0.05f);
	GL_TexEnv(GL_MODULATE);

	const glxy_t* curfont = font1;
	GL_BindImage(r_font1);

	int ox = x;
	int oy = y;

	while (true)
	{
		const byte c = *text++;

		if (c == 0)
			break;

		switch (c)
		{
			case 1:
				ox = (viddef.width - BF_Strlen(text)) / 2;
				break;

			case 2:
				curfont = font1;
				GL_BindImage(r_font1);
				break;
			case 3:
				curfont = font2;
				GL_BindImage(r_font2);
				break;
			
			case '\t':
				ox += 63;
				break;

			case '\n':
				oy += 18;
				ox = x;
				break;

			case '\r':
				break;

			case 32: // Whitespace char
				ox += 8;
				break;

			default:
				if (c > 32)
				{
					const glxy_t* char_def = &curfont[c - 32];
					if (curfont[c - 32].w == 0) //TODO: is this ever triggered?
						char_def = &curfont[14];

					Draw_BigFontChar(ox, oy, char_def); //mxd. Logic split into helper function
					ox += char_def->w;
				}
				break;
		}
	}

	qglDisable(GL_ALPHA_TEST);
	qglDisable(GL_BLEND);
}

// BigFont_Strlen. Returns width of given text in pixels.
int BF_Strlen(const char* text)
{
	if (font1 == NULL || font2 == NULL)
		return 0;

	int width = 0;
	const glxy_t* curfont = font1;

	while (true)
	{
		const byte c = *text++;

		switch (c)
		{
			case 0:
			case 1:
			case '\t':
			case '\n':
				return width;

			case 2:
				curfont = font1;
				break;

			case 3:
				curfont = font2;
				break;

			case '\r':
				break;

			case 32: // Whitespace char
				width += 8;
				break;

			default:
				if (c > 32) // When printable char
				{
					const glxy_t* char_def = &curfont[c - 32];
					if (char_def->w == 0) //TODO: is this ever triggered?
						char_def = &curfont[14]; // Dot chardef?

					width += char_def->w;
				}
				break;
		}
	}
}

//TODO: w and h args are ignored
void Draw_BookPic(const int w, const int h, const char* name, const float scale)
{
	int i;
	bookframe_t* bframe;
	char frame_name[MAX_QPATH];

	const model_t* mod = R_RegisterModel(name);

	if (mod == NULL)
	{
		ri.Con_Printf(PRINT_ALL, "Draw_BookPic: can't find book '%s'\n", name); //mxd. Com_Printf() -> ri.Con_Printf().
		return;
	}

	book_t* book = mod->extradata;

	const float vid_w = (float)viddef.width;
	const float vid_h = (float)viddef.height;

	const float header_w = (float)book->bheader.total_w;
	const float header_h = (float)book->bheader.total_h;

	const int offset_x = (const int)((header_w - header_w * scale) * 0.5f * (vid_w / header_w));
	const int offset_y = (const int)((header_h - header_h * scale) * 0.5f * (vid_h / header_h));
 
	for (i = 0, bframe = book->bframes; i < book->bheader.num_segments; i++, bframe++)
	{
		//TODO: not needed? Mod_LoadBookModel already loads all frames into mod.skins[]. Could use mod.skins[i] instead of frame_img?
		Com_sprintf(frame_name, sizeof(frame_name), "/book/%s", bframe->name);
		const image_t* frame_img = Draw_FindPic(frame_name);

		const int pic_x = (const int)floorf((float)bframe->x * vid_w / header_w * scale);
		const int pic_y = (const int)floorf((float)bframe->y * vid_h / header_h * scale);
		const int pic_w = (const int)ceilf((float)bframe->w * vid_w / header_w * scale);
		const int pic_h = (const int)ceilf((float)bframe->h * vid_h / header_h * scale);

		Draw_Render(offset_x + pic_x, offset_y + pic_y, pic_w, pic_h, frame_img, gl_bookalpha->value);
	}
}