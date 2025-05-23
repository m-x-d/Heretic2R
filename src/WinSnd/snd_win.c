//
// snd_win.c
//
// Copyright 1998 Raven Software
//

#include <dsound.h>
#include "snd_dma.h"
#include "snd_win.h"
#include "sys_win.h"

typedef enum
{
	SIS_SUCCESS,
	SIS_FAILURE,
	SIS_NOTAVAIL
} sndinitstat;

static qboolean dsound_init;
static qboolean wav_init;
static qboolean snd_firsttime = true;
static qboolean snd_isdirect;
static qboolean snd_iswave;

// Starts at 0 for disabled.
static int snd_buffer_count = 0;

// DirectSound variables.
static LPDIRECTSOUND pDS;

static qboolean DS_CreateBuffers(void)
{
	NOT_IMPLEMENTED
	return false;
}

static void DS_DestroyBuffers(void)
{
	NOT_IMPLEMENTED
}

static sndinitstat SNDDMA_InitDirect(void)
{
	NOT_IMPLEMENTED
	return 0;
}

static qboolean SNDDMA_InitWav(void)
{
	NOT_IMPLEMENTED
	return 0;
}

qboolean SNDDMA_Init(void) //mxd. Returns int in original logic.
{
	memset(&dma, 0, sizeof(dma));

	const cvar_t* s_wavonly = Cvar_Get("s_wavonly", "0", 0);

	dsound_init = false;
	wav_init = false;

	sndinitstat stat = SIS_FAILURE;	// Assume DirectSound won't initialize.

	// Init DirectSound
	if (!(int)s_wavonly->value && (snd_firsttime || snd_isdirect))
	{
		stat = SNDDMA_InitDirect();

		if (stat == SIS_SUCCESS)
		{
			snd_isdirect = true;

			if (snd_firsttime)
				Com_Printf("dsound init succeeded\n");
		}
		else
		{
			snd_isdirect = false;
			Com_Printf("*** dsound init failed ***\n");
		}
	}

	// If DirectSound didn't succeed in initializing, try to initialize waveOut sound, unless DirectSound failed
	// because the hardware is already allocated (in which case the user has already chosen not to have sound).
	if (!dsound_init && stat != SIS_NOTAVAIL && (snd_firsttime || snd_iswave))
	{
		snd_iswave = SNDDMA_InitWav();

		if (snd_iswave)
		{
			if (snd_firsttime)
				Com_Printf("Wave sound init succeeded\n");
		}
		else
		{
			Com_Printf("Wave sound init failed\n");
		}
	}

	snd_firsttime = false;
	snd_buffer_count = 1;

	return (dsound_init || wav_init); //H2: missing '*** No sound device initialized ***' logic.
}

// Q2 counterpart.
// Called when the main window gains or loses focus.
// The window have been destroyed and recreated between a deactivate and an activate.
void S_Activate(const qboolean active)
{
	if (pDS != NULL && cl_hwnd != NULL && snd_isdirect)
	{
		if (active)
			DS_CreateBuffers();
		else
			DS_DestroyBuffers();
	}
}