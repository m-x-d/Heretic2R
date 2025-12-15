//
// cl_smk.c -- libsmacker interface (https://github.com/JonnyH/libsmacker)
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include <libsmacker/smacker.h>

typedef struct SMKPlaybackInfo_s
{
	smk smk_obj;

	int frame;
	int total_frames;

	const byte* video_frame;
	const byte* palette;
	float fps; //mxd. Original logic uses cin_rate cvar instead of this.

	// Current .smk video properties.
	int vid_width;
	int vid_height;

	// Current .smk audio properties.
	int snd_channels;
	int snd_width;
	int snd_rate;

	// Auxiliary sound buffer...
	byte* audio_buffer;
	int audio_buffer_size;
	int audio_buffer_end;
} SMKPlaybackInfo_t;

static SMKPlaybackInfo_t spi;

static qboolean SMK_Open(const char* name)
{
	spi.frame = 0; // Reset frame counter.

	spi.smk_obj = smk_open_file(name, SMK_MODE_DISK);
	if (spi.smk_obj == NULL)
		return false;

	smk_enable_all(spi.smk_obj, SMK_VIDEO_TRACK | SMK_AUDIO_TRACK_0);

	ulong frame_count;
	double usf; // Microseconds per frame.
	smk_info_all(spi.smk_obj, NULL, &frame_count, &usf);

	spi.total_frames = (int)frame_count;
	spi.fps = floorf(1000000.0f / (float)usf);

	smk_info_video(spi.smk_obj, &spi.vid_width, &spi.vid_height, NULL);

	byte s_channels[7];
	byte s_bitdepth[7];
	ulong s_rate[7];
	smk_info_audio(spi.smk_obj, NULL, s_channels, s_bitdepth, s_rate);

	spi.snd_channels = s_channels[0];
	spi.snd_width = s_bitdepth[0] / 8; // s_bitdepth: 8 or 16.
	spi.snd_rate = (int)s_rate[0];

	smk_first(spi.smk_obj);
	spi.palette = smk_get_palette(spi.smk_obj);

	if ((spi.vid_width & 7) != 0 || (spi.vid_height & 7) != 0)
	{
		Com_Printf("...Smacker file must but a multiple of 8 high and wide\n");
		SMK_Shutdown();

		return false;
	}

	re.DrawInitCinematic(spi.vid_width, spi.vid_height);

	return true;
}

void SMK_Shutdown(void)
{
	if (spi.smk_obj != NULL)
	{
		re.DrawCloseCinematic();

		smk_close(spi.smk_obj);
		spi.smk_obj = NULL;

		spi.video_frame = NULL;
		spi.palette = NULL;

		if (spi.audio_buffer != NULL)
		{
			free(spi.audio_buffer);
			spi.audio_buffer = NULL;
		}
	}
}

static void SCR_DoCinematicFrame(void) // Called when it's time to render next cinematic frame (e.g. at 15 fps).
{
	// Grab video frame.
	spi.video_frame = smk_get_video(spi.smk_obj);

	// Grab audio frame (way more involved than you may expect)...
	const int smk_audio_frame_size = (int)smk_get_audio_size(spi.smk_obj, 0);

	//mxd. The first smk frame contains 16 frames of audio data. This will overflow s_rawsamples[] if used as is, resulting in desynched audio, so use an auxiliary buffer...
	//mxd. Interestingly, official RAD Video Tools (and SmackW32.dll bundled with vanilla H2) start audio playback from 2-nd frame & end 1 frame too early.
	if (spi.frame == 0)
	{
		spi.audio_buffer = malloc(smk_audio_frame_size);
		spi.audio_buffer_size = smk_audio_frame_size;
		spi.audio_buffer_end = 0;
	}

	// Calculate audio frame size.
	int frame_size;

	//mxd. On frame 0, smk_audio_frame_size is 16 times bigger than needed. Zero smk_audio_frame_size means we have to use remaining buffered audio frames.
	if (spi.frame == 0 || smk_audio_frame_size == 0)
		frame_size = (int)((float)(spi.snd_rate * spi.snd_width) / spi.fps); // Calculate actual frame size.
	else
		frame_size = smk_audio_frame_size; // Use provided frame size.

	// Store audio frame in auxiliary buffer...
	memcpy(spi.audio_buffer + spi.audio_buffer_end, smk_get_audio(spi.smk_obj, 0), smk_audio_frame_size);
	spi.audio_buffer_end += smk_audio_frame_size;

	// Upload audio chunk RawSamples() can handle...
	const int num_samples = frame_size / (spi.snd_width * spi.snd_channels);
	se.RawSamples(num_samples, spi.snd_rate, spi.snd_width, spi.snd_channels, spi.audio_buffer, Cvar_VariableValue("s_volume"));

	// Remove uploaded audio chunk...
	memmove_s(spi.audio_buffer, spi.audio_buffer_size, spi.audio_buffer + frame_size, spi.audio_buffer_size - frame_size);
	spi.audio_buffer_end -= frame_size;

	assert(spi.audio_buffer_end >= 0);

	// Go to next frame.
	smk_next(spi.smk_obj);
	spi.frame++;
}

void SCR_PlayCinematic(const char* name)
{
	char smk_filepath[MAX_OSPATH];

	//mxd. SCR_BeginLoadingPlaque() in original logic.
	se.StopAllSounds_Sounding();
	se.MusicStop();

	//mxd. Skip 'SmackW32.dll' loading logic.

	sprintf_s(smk_filepath, sizeof(smk_filepath), "video/%s", name); //mxd. sprintf -> sprintf_s
	const char* path = FS_GetPath(smk_filepath);
	if (path == NULL)
	{
		Com_Printf("...Unable to find file '%s'\n", smk_filepath);
		SCR_FinishCinematic();

		return;
	}

	sprintf_s(smk_filepath, sizeof(smk_filepath), "%s/video/%s", path, name); //mxd. sprintf -> sprintf_s
	Com_Printf("Opening cinematic file: '%s'...\n", smk_filepath);

	if (!SMK_Open(smk_filepath))
	{
		Com_Printf("...Unable to open file\n");
		SCR_FinishCinematic();

		return;
	}

	cl.cinematictime = (int)((float)cls.realtime - 2000.0f / spi.fps);
	cl.cinematictime = max(1, cl.cinematictime); //mxd. Ensure positive value (can get negative value here, because unlike original logic, cls.realtime starts from 0).

	SCR_DoCinematicFrame(); // Advance cinematic_frame to match with cl.cinematictime in SCR_RunCinematic()...

	Cvar_SetValue("paused", 0);
	cls.state = ca_connected;

	//mxd. Originally called in SCR_RunCinematic().
	SCR_EndLoadingPlaque();

	In_FlushQueue(); // YQ2
	cls.key_dest = key_game;
}

void SCR_DrawCinematic(void) // Called every rendered frame.
{
	if (cl.cinematictime > 0)
		re.DrawCinematic(spi.video_frame, (const paletteRGB_t*)spi.palette);
}

void SCR_RunCinematic(void) // Called every rendered frame.
{
	if (cl.cinematictime < 1)
		return;

	// Pause if menu or console is up.
	if (cls.key_dest != key_game)
	{
		cl.cinematictime = (int)((float)cls.realtime - (float)(spi.frame * 1000) / spi.fps); // Q2: / 14
		return;
	}

	//mxd. Do before SCR_DoCinematicFrame() call to allow the last smk frame to render.
	if (spi.frame >= spi.total_frames)
	{
		SCR_FinishCinematic();
		return;
	}

	const int frame = (int)((float)(cls.realtime - cl.cinematictime) * spi.fps / 1000.0f);

	if (frame <= spi.frame) //mxd. Original code waits using SmackWait function.
		return;

	if (frame > spi.frame + 1)
	{
		Com_Printf("Dropped frame: %i > %i\n", frame, spi.frame + 1);
		cl.cinematictime = (int)((float)cls.realtime - (float)(spi.frame * 1000) / spi.fps); // Q2: / 14
	}

	SCR_DoCinematicFrame();
}

void SCR_StopCinematic(void)
{
	cl.cinematictime = 0; // Done
	SMK_Shutdown(); // H2
}

// Called when either the cinematic completes, or it is aborted.
void SCR_FinishCinematic(void)
{
	// Tell the server to advance to the next map / cinematic.
	MSG_WriteByte(&cls.netchan.message, clc_stringcmd);
	SZ_Print(&cls.netchan.message, va("nextserver %i\n", cl.servercount));

	SCR_StopCinematic();
}