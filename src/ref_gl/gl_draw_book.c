//
// gl_draw_book.c
//
// Copyright 1998 Raven Software
//

#include "gl_local.h"
#include "vid.h"

void Draw_BigFont(int x, int y, char* text, float alpha)
{
	NOT_IMPLEMENTED
}

// BigFont_Strlen. Returns width of given text in pixels.
int BF_Strlen(const char* text)
{
	if (font1 == NULL || font2 == NULL)
		return 0;

	int width = 0;
	curfont = font1;

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

			case ' ':
				width += 8;
				break;

			default:
				if (c > 32) // When printable char
				{
					glxy_t* char_def = &curfont[c - 32];
					if (char_def->w == 0) //TODO: is this ever triggered?
						char_def = &curfont[14]; // Dot chardef?

					width += char_def->w;
				}
				break;
		}
	}
}

//TODO: w and h args are ignored
void Draw_BookPic(const int w, const int h, char* name, const float scale)
{
	int i;
	bookframe_t* bframe;
	char frame_name[MAX_QPATH];

	const model_t* mod = R_RegisterModel(name);

	if (mod == NULL)
	{
		Com_Printf("Draw_BookPic : Can't find book: %s\n", name);
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