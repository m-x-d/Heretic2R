//
// snd_wav.c
//
// Copyright 1998 Raven Software
//

#include "snd_wav.h"
#include "snd_main.h"
#include "snd_sdl3.h"

// Q2 counterpart.
static wavinfo_t GetWavinfo(const char* name, byte* wav, const int wavlength) //TODO: either return wavinfo_t*, or pass wavinfo_t* as arg?
{
	wavinfo_t info = { 0 };

	NOT_IMPLEMENTED
	return info;
}

static short S_GetVolume(const byte* data, int sound_length, int width)
{
	NOT_IMPLEMENTED
	return 0;
}

static void S_GetStatistics(const byte* data, int sound_length, int width, int channels, double sound_volume, int* begin_length, int* end_length, int* attack_length, int* fade_length)
{
	NOT_IMPLEMENTED
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

	const short sound_volume = S_GetVolume(&data[info.dataofs], info.samples, info.width);
	
	int begin_length = 0;
	int attack_length = 0;
	int fade_length = 0;
	int end_length = 0;
	S_GetStatistics(&data[info.dataofs], info.samples, info.width, info.channels, sound_volume, &begin_length, &end_length, &attack_length, &fade_length);

	if (!SDL_Cache(s, &info, &data[info.dataofs], sound_volume, begin_length, end_length, attack_length, fade_length))
	{
		si.Com_Printf("S_LoadSound: couldn't precache '%s'\n", name_buffer);
		si.FS_FreeFile(data);

		return NULL;
	}

	si.FS_FreeFile(data);
	return s->cache;
}