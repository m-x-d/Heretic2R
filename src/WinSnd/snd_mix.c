//
// snd_mix.c
//
// Copyright 1998 Raven Software
//

#include "snd_mix.h"
#include "snd_dma.h"
#include "snd_local.h"

static int snd_scaletable[32][256];

// Q2 counterpart.
void S_InitScaletable(void)
{
	s_volume->modified = false;

	for (int i = 0; i < 32; i++)
	{
		const int scale = (int)((float)i * 8.0f * 256.0f * s_volume->value);

		for (int j = 0; j < 256; j++)
			snd_scaletable[i][j] = (char)j * scale;
	}
}