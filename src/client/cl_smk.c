//
// cl_smk.c -- libsmacker interface (https://github.com/JonnyH/libsmacker)
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include <libsmacker/smacker.h>

static int cinematic_frame;

static smk smk_obj;
static const byte* smk_video_frame;
static const byte* smk_palette;
static float smk_fps; //mxd. Original logic uses cin_rate cvar instead of this.

// Current .smk video properties.
static int smk_vid_width;
static int smk_vid_height;

// Current .smk audio properties.
static int smk_snd_channels;
static int smk_snd_width;
static int smk_snd_rate;

// Auxiliary sound buffer...
static byte* smk_audio_buffer;
static float snd_rate_scaler;

static qboolean SMK_Open(const char* name)
{
	cinematic_frame = 0; // Reset frame counter.

	smk_obj = smk_open_file(name, SMK_MODE_DISK);
	if (smk_obj == NULL)
		return false;

	smk_enable_all(smk_obj, SMK_VIDEO_TRACK | SMK_AUDIO_TRACK_0);

	ulong frame_count;
	double usf; // Microseconds per frame.
	smk_info_all(smk_obj, NULL, &frame_count, &usf);
	smk_fps = floorf(1000000.0f / (float)usf);

	smk_info_video(smk_obj, &smk_vid_width, &smk_vid_height, NULL);

	byte s_channels[7];
	byte s_bitdepth[7];
	ulong s_rate[7];
	smk_info_audio(smk_obj, NULL, s_channels, s_bitdepth, s_rate);

	smk_snd_channels = s_channels[0];
	smk_snd_width = s_bitdepth[0] / 8; // s_bitdepth: 8 or 16.
	smk_snd_rate = (int)s_rate[0];

	//mxd. Setup sound backend rate scaler...
	const cvar_t* s_khz = Cvar_Get("s_khz", "44", CVAR_ARCHIVE);
	const int snd_rate = (s_khz->value == 44.0f ? 44100 : 22050);
	snd_rate_scaler = (float)smk_snd_rate / (float)snd_rate;

	smk_first(smk_obj);
	smk_palette = smk_get_palette(smk_obj);

	if ((smk_vid_width & 7) != 0 || (smk_vid_height & 7) != 0)
	{
		Com_Printf("...Smacker file must but a multiple of 8 high and wide\n");
		SMK_Shutdown();

		return false;
	}

	re.DrawInitCinematic(smk_vid_width, smk_vid_height);

	return true;
}

void SMK_Shutdown(void)
{
	if (smk_obj != NULL)
	{
		re.DrawCloseCinematic();

		smk_close(smk_obj);
		smk_obj = NULL;

		smk_video_frame = NULL;
		smk_palette = NULL;

		if (smk_audio_buffer != NULL)
		{
			free(smk_audio_buffer);
			smk_audio_buffer = NULL;
		}
	}
}

static qboolean SCR_DoCinematicFrame(void) // Called when it's time to render next cinematic frame (e.g. at 15 fps).
{
	static int smk_audio_buffer_offset;
	static int smk_audio_buffer_size;

	// Grab video frame.
	smk_video_frame = smk_get_video(smk_obj);

	// Grab audio frame.
	const int smk_audio_frame_size = (int)smk_get_audio_size(smk_obj, 0);

	//mxd. The first smk frame contains way more audio data than s_rawsamples[] can hold, so use an auxiliary buffer...
	if (cinematic_frame == 0)
	{
		smk_audio_buffer = malloc(smk_audio_frame_size);
		smk_audio_buffer_size = smk_audio_frame_size;
		smk_audio_buffer_offset = 0;
	}
	else
	{
		assert(smk_audio_buffer_size >= smk_audio_buffer_offset + smk_audio_frame_size);
	}

	// Store audio frame in auxiliary buffer...
	memcpy(smk_audio_buffer + smk_audio_buffer_offset, smk_get_audio(smk_obj, 0), smk_audio_frame_size);
	smk_audio_buffer_offset += smk_audio_frame_size;

	// Upload audio chunk RawSamples() can handle...
	const int upload_frame_size = min((int)((MAX_RAW_SAMPLES - 2048) * snd_rate_scaler), smk_audio_frame_size);
	const int num_samples = upload_frame_size / (smk_snd_width * smk_snd_channels);
	se.RawSamples(num_samples, smk_snd_rate, smk_snd_width, smk_snd_channels, smk_audio_buffer, Cvar_VariableValue("s_volume"));

	// Remove uploaded audio chunk...
	memmove_s(smk_audio_buffer, smk_audio_buffer_size, smk_audio_buffer + upload_frame_size, smk_audio_buffer_size - upload_frame_size);
	smk_audio_buffer_offset -= upload_frame_size;

	assert(smk_audio_buffer_offset >= 0);

	// Go to next frame.
	cinematic_frame++;

	return (smk_next(smk_obj) == SMK_MORE);
}

void SCR_PlayCinematic(const char* name)
{
	char smk_filepath[MAX_OSPATH];

	SCR_BeginLoadingPlaque();

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

	cl.cinematictime = (int)((float)cls.realtime - 2000.0f / smk_fps);

	SCR_DoCinematicFrame();
	SCR_DrawCinematic();

	Cvar_SetValue("paused", 0);
	cls.state = ca_connected;

	In_FlushQueue(); // YQ2
	cls.key_dest = key_game;
}

void SCR_DrawCinematic(void) // Called every rendered frame.
{
	if (cl.cinematictime > 0)
		re.DrawCinematic(smk_video_frame, (const paletteRGB_t*)smk_palette);
}

void SCR_RunCinematic(void)
{
	if (cl.cinematictime < 1)
		return;

	// Pause if menu or console is up.
	if (cls.key_dest != key_game)
	{
		cl.cinematictime = (int)((float)cls.realtime - (float)(cinematic_frame * 1000) / smk_fps); // Q2: / 14
		return;
	}

	const int frame = (int)((float)(cls.realtime - cl.cinematictime) * smk_fps / 1000.0f);

	if (frame <= cinematic_frame) //mxd. Original code waits using SmackWait function.
		return;

	if (frame > cinematic_frame + 1)
	{
		Com_Printf("Dropped frame: %i > %i\n", frame, cinematic_frame + 1);
		cl.cinematictime = (int)((float)cls.realtime - (float)(cinematic_frame * 1000) / smk_fps); // Q2: / 14
	}

	if (!SCR_DoCinematicFrame())
		SCR_FinishCinematic();
	else
		SCR_EndLoadingPlaque();
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