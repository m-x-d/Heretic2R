//
// cl_smk.c
//
// Copyright 1998 Raven Software
//

#include "cl_smk.h"
#include "client.h"
#include "q_shared.h" //mxd. Added only for NOT_IMPLEMENTED macro!
#include "screen.h"
#include "snd_dll.h"
#include "sound.h"

static cvar_t* cin_rate;

static int cinematic_frame;
static int cinematic_total_frames;

static int SMK_Open(const char* name)
{
	NOT_IMPLEMENTED
	return 0;
}

static void SCR_DoCinematicFrame(void)
{
	NOT_IMPLEMENTED
}

static int SCR_CinematicWait(void)
{
	NOT_IMPLEMENTED
	return 0;
}

void SCR_PlayCinematic(const char* name)
{
	char smk_filepath[MAX_OSPATH];

	SCR_BeginLoadingPlaque();

	cin_rate = Cvar_Get("cin_rate", "15.0", 0); // H2

	//CDAudio_Stop(); //mxd. Skip CDAudio logic. //mxd. Already done in SCR_BeginLoadingPlaque()!
	if (sound_library != NULL)
		S_Shutdown();

	//mxd. Skip 'SmackW32.dll' loading logic.

	sprintf_s(smk_filepath, sizeof(smk_filepath), "video/%s", name); //mxd. sprintf -> sprintf_s
	const char* path = FS_GetPath(smk_filepath);
	if (path == NULL)
	{
		Com_Printf("...Unable to find file\n");
		SCR_FinishCinematic();

		return;
	}

	sprintf_s(smk_filepath, sizeof(smk_filepath), "%s/video/%s", path, name); //mxd. sprintf -> sprintf_s
	Com_Printf("Opening cinematic file : %s.....\n", smk_filepath);

	cinematic_frame = 0;
	cinematic_total_frames = SMK_Open(smk_filepath);
	if (cinematic_total_frames == 0)
	{
		Com_Printf("...Unable to open file\n");
		SCR_FinishCinematic();

		return;
	}

	cl.cinematictime = (int)((float)(cls.realtime - 2000) / cin_rate->value);

	SCR_DoCinematicFrame();
	SCR_DrawCinematic();

	Cvar_SetValue("paused", 0);
	cls.state = ca_connected;

	Key_ClearStates();
	cls.key_dest = key_game;
}

qboolean SCR_DrawCinematic(void)
{
	NOT_IMPLEMENTED
	return false;
}

void SCR_RunCinematic(void)
{
	if (cl.cinematictime < 1)
		return;

	// Pause if menu or console is up.
	if (cls.key_dest != key_game)
	{
		cl.cinematictime = cls.realtime - cinematic_frame * 1000 / (int)cin_rate->value; // Q2: / 14
		return;
	}

	if (SCR_CinematicWait())
		return;

	const int frame = (int)((float)(cls.realtime - cl.cinematictime) * cin_rate->value / 1000.0f);

	if (frame > cinematic_frame + 1)
	{
		Com_Printf("Dropped frame: %i > %i\n", frame, cinematic_frame + 1);
		cl.cinematictime = cls.realtime - cinematic_frame * 1000 / (int)cin_rate->value; // Q2: / 14
	}

	SCR_DoCinematicFrame();

	if (cinematic_frame >= cinematic_total_frames - 1)
		SCR_FinishCinematic();
	else
		SCR_EndLoadingPlaque();
}

void SCR_FinishCinematic(void)
{
	NOT_IMPLEMENTED
}

void SMK_Stop(void) //TODO: rename to SMK_StopCinematic?
{
	NOT_IMPLEMENTED
}
