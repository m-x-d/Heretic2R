//
// gl_draw_cinematic.c
//
// Copyright 1998 Raven Software
//

#include "gl_local.h"

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

// New in H2
void Draw_CloseCinematic(void)
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

void Draw_Cinematic(int cols, int rows, byte* data, paletteRGB_t* palette, float alpha)
{
	NOT_IMPLEMENTED
}