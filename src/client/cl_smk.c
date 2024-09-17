//
// cl_smk.c
//
// Copyright 1998 Raven Software
//

#include "cl_smk.h"
#include "client.h"
#include "q_shared.h" //mxd. Added only for NOT_IMPLEMENTED macro!
#include "screen.h"

static cvar_t* cin_rate;

static int cinematic_frame;
static int cinematic_total_frames;

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
	NOT_IMPLEMENTED
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
