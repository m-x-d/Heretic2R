//
// snd_win.c
//
// Copyright 1998 Raven Software
//

#include <dsound.h>
#include "snd_win.h"
#include "snd_dma.h"
#include "sys_win.h"
#include "qcommon.h"

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
static HINSTANCE hInstDS;

static void FreeSound(void)
{
	NOT_IMPLEMENTED
}

static qboolean DS_CreateBuffers(void)
{
	NOT_IMPLEMENTED
	return false;
}

static void DS_DestroyBuffers(void)
{
	NOT_IMPLEMENTED
}

// DirectSound support.
static sndinitstat SNDDMA_InitDirect(void)
{
	static HRESULT (WINAPI *pDirectSoundCreate)(GUID FAR* lpGUID, LPDIRECTSOUND FAR* lplpDS, IUnknown FAR* pUnkOuter) = NULL; //mxd. Made local static.

	dma.channels = 2;
	dma.samplebits = 16;
	dma.speed = (s_khz->value == 44.0f ? 44100 : 22050); // H2: no 11025 speed.

	Com_Printf("Initializing DirectSound\n");

	if (hInstDS == NULL)
	{
		Com_DPrintf("...loading dsound.dll: ");
		hInstDS = LoadLibrary("dsound.dll");

		if (hInstDS == NULL)
		{
			Com_Printf("failed\n");
			return SIS_FAILURE;
		}

		Com_DPrintf("ok\n");
		pDirectSoundCreate = (void*)GetProcAddress(hInstDS, "DirectSoundCreate");

		if (pDirectSoundCreate == NULL)
		{
			Com_Printf("*** couldn't get DS proc addr ***\n");
			return SIS_FAILURE;
		}
	}

	Com_DPrintf("...creating DS object: ");

	HRESULT hresult;
	while ((hresult = pDirectSoundCreate(NULL, &pDS, NULL)) != DS_OK)
	{
		if (hresult != DSERR_ALLOCATED)
		{
			Com_Printf("failed\n");
			return SIS_FAILURE;
		}

		if (MessageBox(NULL, "The sound hardware is in use by another app.\n\nSelect Retry to try to start sound again or Cancel to run Heretic 2 with no sound.", "Sound not available", MB_RETRYCANCEL | MB_SETFOREGROUND | MB_ICONEXCLAMATION) != IDRETRY) // H3: Quake -> Heretic 2.
		{
			Com_Printf("failed, hardware already in use\n");
			return SIS_NOTAVAIL;
		}
	}

	Com_DPrintf("ok\n");

	DSCAPS dscaps = { .dwSize = sizeof(dscaps) };
	if (pDS->lpVtbl->GetCaps(pDS, &dscaps) != DS_OK)
		Com_Printf("*** couldn't get DS caps ***\n");

	if (dscaps.dwFlags & DSCAPS_EMULDRIVER)
	{
		Com_DPrintf("...no DSound driver found\n");
		FreeSound();

		return SIS_FAILURE;
	}

	if (!DS_CreateBuffers())
		return SIS_FAILURE;

	dsound_init = true;

	Com_DPrintf("...completed successfully\n");

	return SIS_SUCCESS;
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