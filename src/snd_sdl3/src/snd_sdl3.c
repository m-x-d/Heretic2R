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
#include "snd_wav.h"
#include "Vector.h"
#include "g_local.h"
#include <SDL3/SDL.h> //mxd. Needs to be included below engine stuff: includes stdbool.h, which messes up qboolean define...

#define SDL_PAINTBUFFER_SIZE 2048
#define SDL_FULLVOLUME	80 // Only begin attenuating sound volumes when outside the FULLVOLUME range.

#define ENT_ATTEN_MASK	(255 - ENT_VOL_MASK) //mxd

// Global stream handle.
static SDL_AudioStream* stream;

static portable_samplepair_t paintbuffer[SDL_PAINTBUFFER_SIZE];
static portable_samplepair_t s_rawsamples[MAX_RAW_SAMPLES];

static int playpos = 0;
static int samplesize = 0;
static int soundtime = 0;
static int snd_scaletable[32][256];
static int snd_vol;

static LpfContext_t lpf_context;

// Transfers a mixed "paint buffer" to the SDL output buffer and places it at the appropriate position.
static void SDL_TransferPaintBuffer(const int endtime)
{
	if ((int)s_testsound->value)
	{
		// Write a fixed sine wave.
		const int count = endtime - paintedtime;

		for (int i = 0; i < count; i++)
		{
			const int val = (int)(sinf((float)(paintedtime + i) * 0.1f) * 20000.0f * 256.0f);
			paintbuffer[i].left = val;
			paintbuffer[i].right = val;
		}
	}

	// Optimized case.
	if (sound.samplebits == 16 && sound.channels == 2)
	{
		const int* snd_p = (int*)paintbuffer;
		int ls_paintedtime = paintedtime;

		while (ls_paintedtime < endtime)
		{
			// Handle recirculating buffer issues.
			const int lpos = ls_paintedtime & ((sound.samples >> 1) - 1);
			short* snd_out = (short*)sound.buffer + (lpos << 1);

			int snd_linear_count = (sound.samples >> 1) - lpos;
			snd_linear_count = min(endtime - ls_paintedtime, snd_linear_count);
			snd_linear_count <<= 1;

			for (int i = 0; i < snd_linear_count; i += 2)
			{
				// Write left and right channels.
				for (int c = 0; c < 2; c++)
				{
					const int val = snd_p[i + c] >> 8;
					snd_out[i + c] = (short)ClampI(val, -0x8000, 0x7fff);
				}
			}

			snd_p += snd_linear_count;
			ls_paintedtime += (snd_linear_count >> 1);
		}

		return;
	}

	// General case.
	const int* p = (int*)paintbuffer;
	int count = (endtime - paintedtime) * sound.channels;
	const int out_mask = sound.samples - 1;
	int out_idx = (paintedtime * sound.channels) & out_mask;
	const int step = 3 - sound.channels;

	if (sound.samplebits == 16)
	{
		short* out = (short*)sound.buffer;

		while (count--)
		{
			const int val = *p >> 8;
			p += step;

			out[out_idx] = (short)ClampI(val, -0x8000, 0x7fff);
			out_idx = (out_idx + 1) & out_mask;
		}
	}
	else if (sound.samplebits == 8)
	{
		byte* out = sound.buffer;

		while (count--)
		{
			int val = *p >> 8;
			p += step;

			val = ClampI(val, -0x8000, 0x7fff);

			// FIXME: val might be negative and right-shifting it is implementation defined
			//        on x86 it does sign extension (=> fills up with 1 bits from the left)
			//        so this /might/ break on other platforms - if it does, look at this code again.
			out[out_idx] = (byte)((val >> 8) + 128);
			out_idx = (out_idx + 1) & out_mask;
		}
	}
}

// Mixes an 8 bit sample into a channel.
static void SDL_PaintChannelFrom8(channel_t * ch, const sfxcache_t * sc, const int count, const int offset)
{
	const int leftvol = min(255, ch->leftvol) & ENT_VOL_MASK; // H2: &ENT_VOL_MASK.
	const int rightvol = min(255, ch->rightvol) & ENT_VOL_MASK; // H2: &ENT_VOL_MASK.

	//ZOID-- >>11 has been changed to >>3, >>11 didn't make much sense as it would always be zero.
	const int* lscale = snd_scaletable[leftvol >> 3];
	const int* rscale = snd_scaletable[rightvol >> 3];
	const byte* sfx = &sc->data[ch->pos];

	portable_samplepair_t* samp = &paintbuffer[offset];
	for (int i = 0; i < count; i++, samp++)
	{
		const int data = sfx[i];
		samp->left += lscale[data];
		samp->right += rscale[data];
	}

	ch->pos += count;
}

// Q2 counterpart.
// Mixes an 16 bit sample into a channel.
static void SDL_PaintChannelFrom16(channel_t* ch, const sfxcache_t* sc, const int count, const int offset)
{
	const int leftvol = ch->leftvol * snd_vol;
	const int rightvol = ch->rightvol * snd_vol;
	const short* sfx = (const short*)sc->data + ch->pos;

	portable_samplepair_t* samp = &paintbuffer[offset];
	for (int i = 0; i < count; i++, samp++)
	{
		samp->left += (sfx[i] * leftvol) >> 8;
		samp->right += (sfx[i] * rightvol) >> 8;
	}

	ch->pos += count;
}

// Mixes all pending sounds into the available output channels.
static void SDL_PaintChannels(const int endtime)
{
	snd_vol = (int)(s_volume->value * 256.0f);

	while (paintedtime < endtime)
	{
		// If paintbuffer is smaller than SDL buffer.
		int end = min(endtime, paintedtime + SDL_PAINTBUFFER_SIZE);

		// Start any playsounds.
		while (true)
		{
			playsound_t* ps = s_pendingplays.next;

			if (ps == NULL || ps == &s_pendingplays) // YQ2: extra NULL check.
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

				if (count > 0)
				{
					if (sc->width == 1)
						SDL_PaintChannelFrom8(ch, sc, count, ltime - paintedtime);
					else
						SDL_PaintChannelFrom16(ch, sc, count, ltime - paintedtime);

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

		if ((int)s_camera_under_surface->value)
			LPF_UpdateSamples(&lpf_context, end - paintedtime, paintbuffer);
		else
			lpf_context.is_history_initialized = false;

		if (s_rawend >= paintedtime)
		{
			// Add from the streaming sound source.
			const int stop = (end < s_rawend) ? end : s_rawend;

			for (int i = paintedtime; i < stop; i++)
			{
				const int s = i & (MAX_RAW_SAMPLES - 1);
				paintbuffer[i - paintedtime].left += s_rawsamples[s].left;
				paintbuffer[i - paintedtime].right += s_rawsamples[s].right;
			}
		}

		// Transfer out according to SDL format.
		SDL_TransferPaintBuffer(end);
		paintedtime = end;
	}
}

// Q2 counterpart.
// Used for spatializing channels and autosounds.
static void SDL_SpatializeOrigin(const vec3_t origin, const float master_vol, const float dist_mult, int* left_vol, int* right_vol)
{
	if (si.cls->state != ca_active)
	{
		*left_vol = 255;
		*right_vol = 255;

		return;
	}

	// Calculate stereo separation and distance attenuation.
	vec3_t source_vec;
	VectorSubtract(origin, listener_origin, source_vec);

	float dist = VectorNormalize(source_vec);
	dist = max(0.0f, dist - SDL_FULLVOLUME); // Close enough to be at full volume.
	dist *= dist_mult; // Different attenuation levels.

	float lscale;
	float rscale;

	if (sound.channels == 1 || dist_mult == 0.0f)
	{
		// No attenuation -> no spatialization.
		rscale = 1.0f;
		lscale = 1.0f;
	}
	else
	{
		const float dot = DotProduct(listener_right, source_vec);

		rscale = 0.5f * (1.0f + dot);
		lscale = 0.5f * (1.0f - dot);
	}

	// Add in distance effect.
	float scale = (1.0f - dist) * rscale;
	*right_vol = (int)(master_vol * scale);
	*right_vol = max(0, *right_vol);

	scale = (1.0f - dist) * lscale;
	*left_vol = (int)(master_vol * scale);
	*left_vol = max(0, *left_vol);
}

void SDL_Spatialize(channel_t* ch)
{
	// Anything coming from the view entity will always be full volume.
	if (ch->entnum == si.cl->playernum + 1)
	{
		ch->leftvol = ch->master_vol;
		ch->rightvol = ch->master_vol;

		return;
	}

	if (ch->flags == CF_LEFT_ONLY) // H2.
	{
		ch->leftvol = 255;
		ch->rightvol = 0;

		return;
	}

	if (ch->flags == CF_RIGHT_ONLY) // H2.
	{
		ch->leftvol = 0;
		ch->rightvol = 255;

		return;
	}

	if (!ch->fixed_origin && ch->entnum >= 0 && ch->entnum < MAX_EDICTS) //mxd. Inline CL_GetEntitySoundOrigin().
		VectorCopy(si.cl_entities[ch->entnum].lerp_origin, ch->origin); // H2: update ch->origin instead of using separate var. //TODO: use YQ2 GetEntitySoundOrigin()?

	SDL_SpatializeOrigin(ch->origin, (float)ch->master_vol, ch->dist_mult, &ch->leftvol, &ch->rightvol);
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
	static int buffers;
	static int oldsamplepos;

	const int fullsamples = sound.samples / sound.channels;

	// It is possible to miscount buffers if it has wrapped twice between calls to S_Update. Oh well.
	if (playpos < oldsamplepos)
	{
		buffers++; // Buffer wrapped.

		if (paintedtime > 0x40000000)
		{
			// Time to chop things off to avoid 32 bit limits.
			buffers = 0;
			paintedtime = fullsamples;

			S_StopAllSounds();
		}
	}

	oldsamplepos = playpos;
	soundtime = buffers * fullsamples + playpos / sound.channels;
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

// Shuts the SDL backend down.
void SDL_BackendShutdown(void)
{
	NOT_IMPLEMENTED
}