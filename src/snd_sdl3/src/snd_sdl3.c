//
// snd_sdl3.c
//
// Copyright 2025 mxd
//

#include "snd_sdl3.h"
#include "client.h"
#include "snd_main.h"
#include "snd_LowpassFilter.h"
#include "snd_ogg.h"
#include "Vector.h"
#include "g_local.h"
#include <SDL3/SDL.h> //mxd. Needs to be included below engine stuff: includes stdbool.h, which messes up qboolean define...

// Only begin attenuating sound volumes when outside the FULLVOLUME range.
#define SOUND_FULLVOLUME	80
#define ENT_ATTEN_MASK		(255 - ENT_VOL_MASK) //mxd

// Global stream handle.
static SDL_AudioStream* stream;

static int playpos = 0;
static int samplesize = 0;
static int soundtime = 0;
static int snd_scaletable[32][256];

static LpfContext_t lpf_context;

// Mixes all pending sounds into the available output channels.
static void SDL_PaintChannels(int endtime)
{
	NOT_IMPLEMENTED
}

// Q2 counterpart.
// Used for spatializing channels and autosounds.
static void SDL_SpatializeOrigin(const vec3_t origin, const float master_vol, const float dist_mult, int* left_vol, int* right_vol)
{
	NOT_IMPLEMENTED
}

void SDL_Spatialize(channel_t* ch)
{
	NOT_IMPLEMENTED
}

// Entities with a sound field will generate looped sounds that are automatically started, stopped and merged together as the entities are sent to the client.
static void SDL_AddLoopSounds(void)
{
	int sounds[MAX_EDICTS];
	float attenuations[MAX_EDICTS]; //H2
	float volumes[MAX_EDICTS]; //H2

	if ((int)s_paused->value || si.cls->state != ca_active || !si.cl->sound_prepped)
		return;

	for (int i = 0; i < si.cl->frame.num_entities; i++)
	{
		const int num = (si.cl->frame.parse_entities + i) & (MAX_PARSE_ENTITIES - 1);
		const entity_state_t* ent = &si.cl_parse_entities[num];
		sounds[i] = ent->sound;
		attenuations[i] = snd_attenuations[ent->sound_data & ENT_ATTEN_MASK]; //H2
		volumes[i] = (float)(ent->sound_data & ENT_VOL_MASK); //H2
	}

	for (int i = 0; i < si.cl->frame.num_entities; i++)
	{
		if (sounds[i] == 0)
			continue;

		sfx_t* sfx = si.cl->sound_precache[sounds[i]];

		if (sfx == NULL || sfx->cache == NULL)
			continue; // Bad sound effect.

		int num = (si.cl->frame.parse_entities + i) & (MAX_PARSE_ENTITIES - 1);
		const entity_state_t* ent = &si.cl_parse_entities[num];

		// Find the total contribution of all sounds of this type. //TODO: use YQ2 GetBSPEntitySoundOrigin()?
		vec3_t origin;
		VectorAdd(ent->origin, ent->bmodel_origin, origin); // H2. Original logic does Vec3NotZero(bmodel_origin) check before adding, but who cares... --mxd.

		int left_total;
		int right_total;
		SDL_SpatializeOrigin(origin, volumes[i], attenuations[i], &left_total, &right_total);

		for (int j = i + 1; j < si.cl->frame.num_entities; j++)
		{
			if (sounds[j] != sounds[i])
				continue;

			sounds[j] = 0; // Don't check this again later.

			num = (si.cl->frame.parse_entities + j) & (MAX_PARSE_ENTITIES - 1);
			ent = &si.cl_parse_entities[num];

			int left;
			int right;
			SDL_SpatializeOrigin(ent->origin, volumes[j], attenuations[j], &left, &right);

			left_total += left;
			right_total += right;
		}

		if (left_total == 0 && right_total == 0)
			continue; // Not audible.

		// Allocate a channel.
		channel_t* ch = S_PickChannel(0, 0);

		if (ch == NULL)
			return;

		ch->leftvol = min(255, left_total);
		ch->rightvol = min(255, right_total);
		ch->autosound = true; // Remove next frame.
		ch->sfx = sfx;

		if (sfx->cache->length == 0) // YQ2
		{
			ch->pos = 0;
			ch->end = 0;
		}
		else
		{
			ch->pos = paintedtime % sfx->cache->length;
			ch->end = paintedtime + sfx->cache->length - ch->pos;
		}
	}
}

// Calculates the absolute timecode of current playback.
static void SDL_UpdateSoundtime(void)
{
	NOT_IMPLEMENTED
}

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

// Saves a sound sample into cache. If necessary, endianness conversions are performed.
qboolean SDL_Cache(sfx_t* sfx, const wavinfo_t* info, byte* data)
{
	const float stepscale = (float)info->rate / (float)sound.speed; // This is usually 0.5, 1, or 2.
	const int outcount = (int)((float)info->samples / stepscale);
	const int len = outcount * info->width * info->channels;

	if (len == 0)
	{
		si.Com_Printf("SDL_Cache: invalid sound file '%s' (zero length)\n", sfx->name);
		return false;
	}

	sfx->cache = si.Z_Malloc(len + (int)sizeof(sfxcache_t));

	sfxcache_t* sc = sfx->cache;

	if (sc == NULL)
		return false;

	sc->loopstart = info->loopstart;
	sc->stereo = (info->channels > 1);
	sc->length = outcount;
	sc->speed = sound.speed;
	sc->width = ((int)s_loadas8bit->value ? 1 : info->width);

	if (sc->loopstart != -1)
		sc->loopstart = (int)((float)sc->loopstart / stepscale);

	// Resample / decimate to the current source rate.
	uint samplefrac = 0;
	for (int i = 0; i < outcount; i++)
	{
		const uint srcsample = samplefrac >> 8;
		samplefrac += (int)(stepscale * 256.0f);

		int sample;

		if (info->width == 2)
			sample = LittleShort(((short*)data)[srcsample]);
		else
			sample = (data[srcsample] - 128) << 8;

		if (sc->width == 2)
			((short*)sc->data)[i] = (short)sample;
		else
			((char*)sc->data)[i] = (char)(sample >> 8);
	}

	return true;
}

// Runs every frame, handles all necessary sound calculations and fills the playback buffer.
void SDL_Update(void)
{
	if (s_underwater_gain_hf->modified)
	{
		s_underwater_gain_hf->modified = false;
		LPF_Initialize(&lpf_context, s_underwater_gain_hf->value, sound.speed);
	}

	// Rebuild scale tables if volume is modified.
	if (s_volume->modified)
		SDL_UpdateScaletable();

	// Update spatialization for dynamic sounds.
	channel_t* ch = &channels[0];
	for (int i = 0; i < MAX_CHANNELS; i++, ch++)
	{
		if (ch->sfx == NULL)
			continue;

		if (ch->autosound)
			memset(ch, 0, sizeof(*ch)); // Autosounds are regenerated fresh each frame.
		else
			SDL_Spatialize(ch); // Re-spatialize channel.

		// Clear channel when it can't be heard.
		if (ch->leftvol == 0 && ch->rightvol == 0)
			memset(ch, 0, sizeof(*ch));
	}

	// Add looping sounds.
	SDL_AddLoopSounds();

	// Debugging output.
	if ((int)s_show->value)
	{
		int total_sounds = 0;

		ch = &channels[0];
		for (int i = 0; i < MAX_CHANNELS; i++, ch++)
		{
			if (ch->sfx != NULL && (ch->leftvol > 0 || ch->rightvol > 0))
			{
				si.Com_Printf("%3i %3i %s\n", ch->leftvol, ch->rightvol, ch->sfx->name);
				total_sounds++;
			}
		}

		si.Com_Printf("----(%i)---- painted: %i\n", total_sounds, paintedtime);
	}

	// Stream music.
	OGG_Stream();

	if (sound.buffer == NULL)
		return;

	// Mix the samples.
	SDL_UpdateSoundtime();

	if (soundtime == 0)
		return;

	// Check to make sure that we haven't overshot.
	if (paintedtime < soundtime)
	{
		si.Com_DPrintf("SDL_Update: overflow\n");
		paintedtime = soundtime;
	}

	// Mix ahead of current position.
	uint endtime = soundtime + (int)(s_mixahead->value * (float)sound.speed);

	// Mix to an even submission block size.
	endtime = (endtime + sound.submission_chunk - 1) & ~(sound.submission_chunk - 1);

	const uint samps = sound.samples >> (sound.channels - 1);
	SDL_PaintChannels(min(endtime, soundtime + samps));
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