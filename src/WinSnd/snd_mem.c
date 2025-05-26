//
// snd_mem.c
//
// Copyright 1998 Raven Software
//

#include "snd_mem.h"
#include "snd_dma.h"
#include "qcommon.h"

static byte* data_p;
static byte* iff_end;
static byte* last_chunk;
static byte* iff_data;
static int iff_chunk_len;

static void ResampleSfx(sfx_t* sfx, int inrate, int inwidth, byte* data)
{
	NOT_IMPLEMENTED
}

// Q2 counterpart.
static short GetLittleShort(void)
{
	const short val = (short)(data_p[0] + (data_p[1] << 8));
	data_p += 2;

	return val;
}

// Q2 counterpart.
static int GetLittleLong(void)
{
	const int val = data_p[0] + (data_p[1] << 8) + (data_p[2] << 16) + (data_p[3] << 24);
	data_p += 4;

	return val;
}

// Q2 counterpart.
static void FindNextChunk(const char* name)
{
	while (true)
	{
		data_p = last_chunk;

		if (data_p >= iff_end)
		{
			// Didn't find the chunk.
			data_p = NULL;
			return;
		}

		data_p += 4;
		iff_chunk_len = GetLittleLong();

		if (iff_chunk_len < 0)
		{
			data_p = NULL;
			return;
		}

		data_p -= 8;
		last_chunk = data_p + 8 + ((iff_chunk_len + 1) & ~1);

		if (strncmp((char*)data_p, name, 4) == 0)
			return;
	}
}

// Q2 counterpart.
static void FindChunk(const char* name)
{
	last_chunk = iff_data;
	FindNextChunk(name);
}

// Q2 counterpart.
static wavinfo_t GetWavinfo(const char* name, byte* wav, const int wavlength) //TODO: either return wavinfo_t*, or pass wavinfo_t* as arg?
{
	wavinfo_t info;

	memset(&info, 0, sizeof(info));

	if (wav == NULL)
		return info;

	iff_data = wav;
	iff_end = wav + wavlength;

	// Find "RIFF" chunk.
	FindChunk("RIFF");

	if (data_p == NULL || strncmp((char*)(&data_p[8]), "WAVE", 4) != 0)
	{
		Com_Printf("Missing RIFF/WAVE chunks\n");
		return info;
	}

	// Get "fmt " chunk.
	iff_data = &data_p[12];
	FindChunk("fmt ");

	if (data_p == NULL)
	{
		Com_Printf("Missing fmt chunk\n");
		return info;
	}

	data_p += 8;
	const int format = GetLittleShort();

	if (format != 1)
	{
		Com_Printf("Microsoft PCM format only\n");
		return info;
	}

	info.channels = GetLittleShort();
	info.rate = GetLittleLong();

	data_p += 6;
	info.width = GetLittleShort() / 8;

	// Get cue chunk.
	FindChunk("cue ");

	if (data_p != NULL)
	{
		data_p += 32;
		info.loopstart = GetLittleLong();

		// If the next chunk is a LIST chunk, look for a cue length marker.
		FindNextChunk("LIST");

		if (data_p != NULL && strncmp((char*)&data_p[28], "mark", 4) == 0)
		{
			// This is not a proper parse, but it works with cooledit...
			data_p += 24;
			info.samples = info.loopstart + GetLittleLong(); // Samples in loop.
		}
	}
	else
	{
		info.loopstart = -1;
	}

	// Find data chunk.
	FindChunk("data");

	if (data_p == NULL)
	{
		Com_Printf("Missing data chunk\n");
		return info;
	}

	data_p += 4;
	const int samples = GetLittleLong() / info.width;

	if (info.samples > 0)
	{
		if (samples < info.samples)
			Com_Error(ERR_DROP, "Sound %s has a bad loop length", name);
	}
	else
	{
		info.samples = samples;
	}

	info.dataofs = data_p - wav;

	return info;
}

sfxcache_t* S_LoadSound(sfx_t* s)
{
	char name_buffer[MAX_QPATH];
	
	// See if still in memory.
	if (s->cache != NULL)
		return s->cache;

	// Load it in. //H2: no s->truename check.
	if (s->name[0] == '#')
		strcpy_s(name_buffer, sizeof(name_buffer), & s->name[1]); //mxd. strcpy -> strcpy_s.
	else if (s->name[0] == '*') //H2: check done in different place.
		return NULL;
	else
		Com_sprintf(name_buffer, sizeof(name_buffer), "%s/%s", s_sounddir->string, s->name); //H2: use s_sounddir cvar.

	byte* data;
	const int size = FS_LoadFile(name_buffer, (void**)&data);

	if (data == NULL)
	{
		Com_DPrintf("S_LoadSound: couldn't load '%s'\n", name_buffer); //H2: function name added to message.
		return NULL;
	}

	const wavinfo_t info = GetWavinfo(s->name, data, size);

	if (info.channels != 1)
	{
		Com_Printf("S_LoadSound: skipping '%s' (expected mono sample)\n", s->name); //mxd. More specific message.
		FS_FreeFile(data);

		return NULL;
	}

	const float stepscale = (float)info.rate / (float)dma.speed;
	int len = (int)((float)info.samples / stepscale);
	len *= info.width * info.channels;

	s->cache = Z_Malloc(len + (int)sizeof(sfxcache_t));

	if (s->cache == NULL)
	{
		FS_FreeFile(data);
		return NULL;
	}

	s->cache->length = info.samples;
	s->cache->loopstart = info.loopstart;
	s->cache->speed = info.rate;
	s->cache->width = info.width;
	s->cache->stereo = info.channels;

	ResampleSfx(s, s->cache->speed, s->cache->width, data + info.dataofs);
	FS_FreeFile(data);

	return s->cache;
}