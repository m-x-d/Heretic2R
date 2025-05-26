//
// snd_mix.c
//
// Copyright 1998 Raven Software
//

#include "snd_mix.h"
#include "snd_dma.h"
#include "snd_mem.h"
#include "snd_local.h"

#define PAINTBUFFER_SIZE	2048
portable_samplepair_t paintbuffer[PAINTBUFFER_SIZE];
static int snd_scaletable[32][256];
static int snd_vol;

static void S_TransferPaintBuffer(int endtime)
{
	NOT_IMPLEMENTED
}

static void S_PaintChannelFrom8(channel_t* ch, sfxcache_t* sc, int count, int offset)
{
	NOT_IMPLEMENTED
}

static void S_PaintChannelFrom16(channel_t* ch, sfxcache_t* sc, int count, int offset)
{
	NOT_IMPLEMENTED
}

void S_PaintChannels(const int endtime)
{
	snd_vol = (int)(s_volume->value * 256.0f);

	while (paintedtime < endtime)
	{
		// If paintbuffer is smaller than DMA buffer.
		int end = min(endtime, paintedtime + PAINTBUFFER_SIZE);

		// Start any playsounds.
		while (true)
		{
			playsound_t* ps = s_pendingplays.next;

			if (ps == &s_pendingplays)
				break; // No more pending sounds.

			if ((int)ps->begin <= paintedtime)
			{
				S_IssuePlaysound(ps);
				continue;
			}

			if ((int)ps->begin < end)
				end = (int)ps->begin; // Stop here.

			break;
		}

		// Clear the paint buffer.
		memset(paintbuffer, 0, (end - paintedtime) * sizeof(portable_samplepair_t)); // H2: no s_rawend logic.

		// Paint in the channels.
		channel_t* ch = &channels[0];
		for (int i = 0; i < MAX_CHANNELS; i++, ch++)
		{
			int ltime = paintedtime;

			while (ltime < end)
			{
				if (ch->sfx == NULL || (ch->leftvol == 0 && ch->rightvol == 0))
					break;

				// Max painting is to the end of the buffer.
				int count = end - ltime;

				// Might be stopped by running out of data.
				if (ch->end - ltime < count)
					count = ch->end - ltime;

				sfxcache_t* sc = S_LoadSound(ch->sfx);

				if (sc == NULL)
					break;

				if (count > 0 && ch->sfx != NULL)
				{
					if (sc->width == 1)
						S_PaintChannelFrom8(ch, sc, count, ltime - paintedtime);
					else
						S_PaintChannelFrom16(ch, sc, count, ltime - paintedtime);

					ltime += count;
				}

				// If at end of loop, restart.
				if (ltime >= ch->end)
				{
					if (ch->autosound)
					{
						// Autolooping sounds always go back to start.
						ch->pos = 0;
						ch->end = ltime + sc->length;
					}
					else if (sc->loopstart >= 0)
					{
						ch->pos = sc->loopstart;
						ch->end = ltime + sc->length - ch->pos;
					}
					else
					{
						// Channel just stopped.
						ch->sfx = NULL;
					}
				}
			}
		}

		// Transfer out according to DMA format.
		S_TransferPaintBuffer(end);
		paintedtime = end;
	}
}

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