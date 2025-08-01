//
// snd_wav.c
//
// Copyright 1998 Raven Software
//

#include "snd_wav.h"
#include "snd_main.h"
#include "snd_sdl3.h"

static byte* data_p;
static byte* iff_end;
static byte* last_chunk;
static byte* iff_data;
static int iff_chunk_len;

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
		data_p += 4;

		if (data_p >= iff_end)
		{
			// Didn't find the chunk.
			data_p = NULL;
			return;
		}

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
	wavinfo_t info = { 0 };

	if (wav == NULL)
		return info;

	iff_data = wav;
	iff_end = wav + wavlength;

	// Find "RIFF" chunk.
	FindChunk("RIFF");

	if (data_p == NULL || strncmp((char*)(&data_p[8]), "WAVE", 4) != 0)
	{
		si.Com_Printf("Missing RIFF/WAVE chunks\n");
		return info;
	}

	// Get "fmt " chunk.
	iff_data = &data_p[12];
	FindChunk("fmt ");

	if (data_p == NULL)
	{
		si.Com_Printf("Missing fmt chunk\n");
		return info;
	}

	data_p += 8;
	const int format = GetLittleShort();

	if (format != 1)
	{
		si.Com_Printf("Microsoft PCM format only\n");
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

		if (data_p != NULL && data_p - wav + 32 <= wavlength && strncmp((char*)&data_p[28], "mark", 4) == 0) //YQ2: extra file length sanity check.
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
		si.Com_Printf("Missing data chunk\n");
		return info;
	}

	data_p += 4;
	const int samples = GetLittleLong() / info.width;

	if (info.samples > 0)
	{
		if (samples < info.samples)
		{
			si.Com_Error(ERR_DROP, "Sound %s has a bad loop length", name);
			return info;
		}
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
		strcpy_s(name_buffer, sizeof(name_buffer), &s->name[1]); //mxd. strcpy -> strcpy_s.
	else if (s->name[0] == '*') //H2: check done in different place.
		return NULL;
	else
		Com_sprintf(name_buffer, sizeof(name_buffer), "%s/%s", s_sounddir->string, s->name); //H2: use s_sounddir cvar.

	byte* data;
	const int size = si.FS_LoadFile(name_buffer, (void**)&data);

	if (data == NULL)
	{
		si.Com_DPrintf("S_LoadSound: couldn't load '%s'\n", name_buffer); //H2: function name added to message.
		return NULL;
	}

	const wavinfo_t info = GetWavinfo(s->name, data, size);

	if (info.channels != 1 && info.channels != 2) // YQ2: also stereo.
	{
		si.Com_Printf("S_LoadSound: skipping '%s' (expected mono or stereo sample)\n", s->name); //mxd. More specific message.
		si.FS_FreeFile(data);

		return NULL;
	}

	if (!SNDSDL3_Cache(s, &info, &data[info.dataofs]))
	{
		si.Com_Printf("S_LoadSound: couldn't precache '%s'\n", name_buffer);
		si.FS_FreeFile(data);

		return NULL;
	}

	si.FS_FreeFile(data);
	return s->cache;
}