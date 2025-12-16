//
// snd_ogg.c
//
// Copyright 2025 mxd
//

#include "snd_ogg.h"
#include "snd_local.h"
#include "snd_main.h"

#define STB_VORBIS_NO_PUSHDATA_API
#include <stb/stb_vorbis.h>

typedef enum
{
	OGG_PLAY,
	OGG_STOP
} ogg_status_t;

static cvar_t* ogg_volume; // Music volume.

static int ogg_curtrack;
static qboolean ogg_loop;

static ogg_status_t ogg_status;
static stb_vorbis* ogg_file;
static qboolean ogg_started;

#define OGG_FADEIN_DURATION	1500 //mxd. In ms.
static int ogg_fadein_time; //mxd.

void OGG_Stream(void)
{
#define BUFFER_SIZE	4096

	static short buffer[BUFFER_SIZE];

	if (!ogg_started || ogg_status != OGG_PLAY)
		return;

	//mxd. Setup fade-in effect.
	float fadein_volume = 1.0f;
	if (si.cl->time < ogg_fadein_time)
		fadein_volume -= (float)(ogg_fadein_time - si.cl->time) / (float)OGG_FADEIN_DURATION; // [0.0 .. 1.0]

	// Read that number samples into the buffer, that were played since the last call to this function.
	// This keeps the buffer at all times at an "optimal" fill level.
	while (paintedtime + MAX_RAW_SAMPLES - 2048 > s_rawend)
	{
		const int read_samples = stb_vorbis_get_samples_short_interleaved(ogg_file, ogg_file->channels, buffer, BUFFER_SIZE);

		if (read_samples > 0)
		{
			S_RawSamples(read_samples, ogg_file->sample_rate, sizeof(buffer[0]), ogg_file->channels, (const byte*)buffer, ogg_volume->value * fadein_volume);
		}
		else // Track ended. Time to restart?
		{
			OGG_Stop();

			if (ogg_loop)
				OGG_PlayTrack(ogg_curtrack, 0, true);

			break; //mxd
		}
	}
}

void OGG_PlayTrack(const int track, const uint track_pos, const qboolean looping)
{
	if (!ogg_started)
		return;

	// Track 0 means "stop music".
	if (track == 0)
	{
		OGG_Stop();
		return;
	}

	// Check running music.
	if (ogg_status == OGG_PLAY)
	{
		if (ogg_curtrack == track)
			return;

		OGG_Stop();
	}

	// Open ogg vorbis file.
	char track_path[MAX_OSPATH];
	sprintf_s(track_path, sizeof(track_path), "%s/music/Track%02i.ogg", si.FS_Gamedir(), track);

	int vorbis_error = VORBIS__no_error;
	ogg_file = stb_vorbis_open_filename(track_path, &vorbis_error, NULL);

	if (vorbis_error != VORBIS__no_error)
	{
		si.Com_Printf("OGG_PlayTrack: '%s' is not a valid Ogg Vorbis file (error %i).\n", track_path, vorbis_error);
		return;
	}

	if (track_pos > 0)
	{
		stb_vorbis_seek_frame(ogg_file, track_pos);
		ogg_fadein_time = si.cl->time + OGG_FADEIN_DURATION;
	}

	// Play file.
	ogg_curtrack = track;
	ogg_loop = looping;
	ogg_status = OGG_PLAY;
}

// Stop playing the current file.
void OGG_Stop(void)
{
	if (ogg_status != OGG_STOP)
	{
		stb_vorbis_close(ogg_file);
		ogg_status = OGG_STOP;
		ogg_fadein_time = 0;
	}
}

void OGG_GetCurrentTrackInfo(int* track, uint* track_pos, qboolean* looping) //mxd
{
	if (ogg_started && ogg_status == OGG_PLAY)
	{
		*track = ogg_curtrack;
		*track_pos = ogg_file->current_loc;
		*looping = ogg_loop;
	}
	else
	{
		*track = 0;
		*track_pos = 0;
		*looping = false;
	}
}

// Initialize the Ogg Vorbis subsystem.
void OGG_Init(void)
{
	// Cvars.
	ogg_volume = si.Cvar_Get("m_volume", "0.5", CVAR_ARCHIVE);

	// Global variables.
	ogg_curtrack = 0; // Track 0 means "stop music".
	ogg_status = OGG_STOP;
	ogg_started = true;
}

// Shutdown the Ogg Vorbis subsystem.
void OGG_Shutdown(void)
{
	if (ogg_started)
	{
		OGG_Stop();
		ogg_started = false;
	}
}