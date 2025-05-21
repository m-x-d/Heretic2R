//
// gl_draw_cinematic.c
//
// Copyright 1998 Raven Software
//

#include "gl_local.h"
#include "vid.h"

#define MAX_CINEMATIC_TILES 50

//mxd. Reconstructed data type. Original name unknown...
typedef struct
{
	int offset_x;
	int offset_y;
	image_t* image;
} CinematicTile_t;

static CinematicTile_t cinematic_tiles[MAX_CINEMATIC_TILES];
static int num_cinematic_tiles;

static paletteRGB_t cinematic_palette[256];

static int GetCoords(int size, int* offsets)
{
	int num_tiles = 1;
	int target_size = 256;
	int total_size = 0;

	if (size == 0)
		return num_tiles;

	int* cur_offset = offsets;
	*cur_offset = 0;
	cur_offset++;

	do
	{
		if (size < target_size)
		{
			target_size >>= 1;
		}
		else
		{
			total_size += target_size;
			size -= target_size;
			*cur_offset = total_size;
			
			cur_offset++;
			num_tiles++;
		}
	} while (size != 0);

	return num_tiles;
}

static int ChopImage(const int width, const int height)
{
	int offsets_x[64];
	int offsets_y[64];
	
	const int tiles_x = GetCoords(width, offsets_x) - 1;
	const int tiles_y = GetCoords(height, offsets_y) - 1;

	if (tiles_y < 1)
		return 0;

	CinematicTile_t* tile = cinematic_tiles;

	for (int y = 0; y < tiles_y; y++)
	{
		for (int x = 0; x < tiles_x; x++, tile++)
		{
			image_t* image = GL_GetFreeImage();

			image->registration_sequence = registration_sequence;
			image->width = offsets_x[x + 1] - offsets_x[x];
			image->height = offsets_y[y + 1] - offsets_y[y];
			image->type = it_pic;
			image->palette = NULL;
			image->texnum = TEXNUM_IMAGES + (image - gltextures);
			image->has_alpha = 0;

			tile->offset_x = offsets_x[x];
			tile->offset_y = offsets_y[y];
			tile->image = image;
		}
	}

	return tiles_x * tiles_y;
}

void Draw_InitCinematic(const int w, const int h, char* overlay, char* backdrop)
{
	num_cinematic_tiles = ChopImage(w, h);
}

void Draw_CloseCinematic(void) // H2
{
	int i;
	CinematicTile_t* tile;

	for (i = 0, tile = cinematic_tiles; i < num_cinematic_tiles; i++, tile++)
	{
		tile->image->palette = NULL; //TODO: GL_FreeImageNoHash frees image->palette. Explicitly set to NULL here to avoid that?
		GL_FreeImageNoHash(tile->image);
	}

	num_cinematic_tiles = 0;
}

static void CopyChunkToImage(const byte* src, const int w, const int h, const int cols, const int rows, const image_t* img, byte* dst)
{
	const byte* src_p = src + w + cols * h;

	for (int ch = 0; ch < img->height; ch++, src_p += ((cols - img->width) >> 2) * 4)
	{
		if (img->width >= 0 && (img->width & 0xFFFFFFF8) != 0)
		{
			memcpy(dst, src_p, img->width);
			src_p += img->width;
			dst += img->width;
		}
	}
}

void Draw_Cinematic(const int cols, const int rows, const byte* data, const paletteRGB_t* palette, const float alpha)
{
	int i;
	CinematicTile_t* tile;
	static byte frame_data[256 * 256]; //mxd. Made static

	Draw_Fill(0, 0, viddef.width, viddef.height, 0, 0, 0);

	const int alpha255 = Q_ftol(alpha * 255.0f);
	for (i = 0; i < 256; i++, palette++)
	{
		cinematic_palette[i].r = (byte)((palette->r * alpha255) >> 8);
		cinematic_palette[i].g = (byte)((palette->g * alpha255) >> 8);
		cinematic_palette[i].b = (byte)((palette->b * alpha255) >> 8);
	}

	const float col_width = (float)viddef.width / (float)cols;
	const float col_height = ((float)viddef.height - (float)rows * col_width) / (col_width * 2.0f);

	for (i = 0, tile = cinematic_tiles; i < num_cinematic_tiles; i++, tile++)
	{
		image_t* img = tile->image;
		img->palette = cinematic_palette;

		CopyChunkToImage(data, tile->offset_x, tile->offset_y, cols, rows, img, frame_data);

		GL_BindImage(img);
		GL_UploadPaletted(0, frame_data, img->palette, img->width, img->height);
		GL_SetFilter(img);

		const int x = Q_ftol(floorf((float)tile->offset_x * col_width));
		const int y = Q_ftol(floorf(((float)tile->offset_y + col_height) * col_width));
		const int w = Q_ftol(ceilf((float)img->width * col_width));
		const int h = Q_ftol(ceilf((float)img->height * col_width));

		Draw_Render(x, y, w, h, img, 1.0f);
	}
}