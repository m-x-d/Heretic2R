//
// snd_sdl3.c
//
// Copyright 2025 mxd
//

#include "snd_sdl3.h"
#include "client.h"
#include "snd_main.h"
#include "snd_LowpassFilter.h"
#include <SDL3/SDL.h> //mxd. Needs to be included below engine stuff: includes stdbool.h, which messes up qboolean define...

// Global stream handle.
static SDL_AudioStream* stream;

static int playpos = 0;
static int samplesize = 0;
static int soundtime = 0;
static int snd_scaletable[32][256];

static LpfContext_t lpf_context;

// Updates the volume scale table based on current volume setting.
static void SDL_UpdateScaletable(void) // Q2: S_InitScaletable().
{
	if (s_volume->value > 2.0f) // YQ2: extra sanity checks.
		si.Cvar_Set("s_volume", "2");
	else if (s_volume->value < 0.0f)
		si.Cvar_Set("s_volume", "0");

	s_volume->modified = false;

	for (int i = 0; i < 32; i++)
	{
		const int scale = (int)((float)i * 8.0f * 256.0f * s_volume->value);

		for (int j = 0; j < 256; j++)
			snd_scaletable[i][j] = (char)j * scale;
	}
}

// Runs every frame, handles all necessary sound calculations and fills the playback buffer.
void SDL_Update(void)
{
	NOT_IMPLEMENTED
}

// Callback function for SDL. Writes sound data to SDL when requested.
static void SDL_Callback(byte* sdl_stream, const int length)
{
	int pos = playpos * (sound.samplebits / 8);

	if (pos >= samplesize)
	{
		playpos = 0;
		pos = 0;
	}

	const int to_buffer_end = samplesize - pos;

	int length1;
	int length2;

	if (length > to_buffer_end)
	{
		length1 = to_buffer_end;
		length2 = length - length1;
	}
	else
	{
		length1 = length;
		length2 = 0;
	}

	memcpy(sdl_stream, sound.buffer + pos, length1);

	// Set new position.
	if (length2 <= 0)
	{
		playpos += (length1 / (sound.samplebits / 8));
	}
	else
	{
		memcpy(sdl_stream + length1, sound.buffer, length2);
		playpos = length2 / (sound.samplebits / 8);
	}

	if (playpos >= samplesize)
		playpos = 0;
}

// Wrapper function, ties the old existing callback logic from the SDL 1.2 days and later fiddled into SDL 2 to a SDL 3 compatible callback...
static void SDL_SDL3Callback(void* userdata, SDL_AudioStream* sdl_stream, int additional_amount, int total_amount)
{
	if (additional_amount < 1)
		return;

	byte* data = SDL_stack_alloc(byte, additional_amount);

	if (data != NULL)
	{
		SDL_Callback(data, additional_amount);
		SDL_PutAudioStreamData(sdl_stream, data, additional_amount);
		SDL_stack_free(data);
	}
}

// Initializes the SDL sound backend and sets up SDL.
qboolean SDL_BackendInit(void)
{
	si.Com_Printf("Initializing SDL3 audio backend.\n");

	if (!SDL_WasInit(SDL_INIT_AUDIO) && !SDL_Init(SDL_INIT_AUDIO))
	{
		si.Com_Printf("Couldn't initialize SDL3 audio: %s.\n", SDL_GetError());
		return false;
	}

	const char* drivername = SDL_GetCurrentAudioDriver();
	si.Com_Printf("SDL3 audio driver: '%s'.\n", (drivername == NULL ? "UNKNOWN" : drivername));

	const SDL_AudioSpec spec =
	{
		.format = SDL_AUDIO_S16,
		.channels = 2,
		.freq = (s_khz->value == 44.0f ? 44100 : 22050), // H2: no 11025 speed.
	};

	// Let's try our luck.
	stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, SDL_SDL3Callback, NULL);

	if (stream == NULL)
	{
		si.Com_Printf("SDL_OpenAudioDeviceStream() failed: %s\n", SDL_GetError());
		SDL_QuitSubSystem(SDL_INIT_AUDIO);

		return false;
	}

	const int samples = (spec.freq == 44100 ? 1024 : 512);
	int tmp = samples * spec.channels * 10;

	if (tmp & (tmp - 1))
	{
		// Make it a power of two.
		int val = 1;
		while (val < tmp)
			val <<= 1;

		tmp = val;
	}

	sound.samplebits = (spec.format & 0xff);
	sound.channels = spec.channels;
	sound.samples = tmp;
	sound.submission_chunk = 1;
	sound.speed = spec.freq;

	samplesize = sound.samples * (sound.samplebits / 8);
	sound.buffer = calloc(1, samplesize);

	s_underwater_gain_hf->modified = true;
	LPF_Initialize(&lpf_context, LPF_DEFAULT_GAIN_HF, sound.speed);

	SDL_UpdateScaletable();
	SDL_ResumeAudioDevice(SDL_GetAudioStreamDevice(stream));

	playpos = 0;
	soundtime = 0;

	si.Com_Printf("SDL3 audio backend initialized.\n");

	return true;
}