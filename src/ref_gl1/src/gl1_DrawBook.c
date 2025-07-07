//
// gl1_DrawBook.c
//
// Copyright 1998 Raven Software
//

#include "gl1_DrawBook.h"
#include "gl1_Draw.h"
#include "gl1_Model.h"
#include "qfiles.h"
#include "vid.h"
#include "gl1_Local.h"

void Draw_BigFont(const int x, const int y, const char* text, const float alpha)
{
	NOT_IMPLEMENTED
}

int BF_Strlen(const char* text)
{
	NOT_IMPLEMENTED
	return 0;
}

//TODO: w and h args are ignored.
void Draw_BookPic(const int w, const int h, const char* name, const float scale)
{
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

	bookframe_t* bframe = &book->bframes[0];
	for (int i = 0; i < book->bheader.num_segments; i++, bframe++)
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