//
// snd_mem.c
//
// Copyright 1998 Raven Software
//

#include "snd_mem.h"
#include "snd_dma.h"
#include "qcommon.h"

static void ResampleSfx(sfx_t* sfx, int inrate, int inwidth, byte* data)
{
	NOT_IMPLEMENTED
}

static wavinfo_t GetWavinfo(char* name, byte* wav, int wavlength) //TODO: either return wavinfo_t*, or pass wavinfo_t* as arg?
{
	wavinfo_t info = { 0 };

	NOT_IMPLEMENTED
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