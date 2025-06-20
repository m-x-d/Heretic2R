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

// 64K is > 1 second at 16-bit, 22050 Hz.
#define WAV_BUFFERS				64
#define WAV_MASK				0x3f
#define WAV_BUFFER_SIZE			0x0400
#define SECONDARY_BUFFER_SIZE	0x10000

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
static qboolean primary_format_set;

// Starts at 0 for disabled.
static int snd_buffer_count = 0;
static int sample16;
static int snd_sent;
static int snd_completed;
static DWORD locksize;

// DirectSound variables.
static HANDLE hData;
static HPSTR lpData;

static HGLOBAL hWaveHdr;
static LPWAVEHDR lpWaveHdr;

static HWAVEOUT hWaveOut;

static DWORD gSndBufSize;

static MMTIME mmstarttime;

static LPDIRECTSOUND pDS;

static LPDIRECTSOUNDBUFFER pDSBuf;
static LPDIRECTSOUNDBUFFER pDSPBuf;

static HINSTANCE hInstDS;

// Q2 counterpart.
static const char* DSoundError(const int error)
{
	switch (error)
	{
		case DSERR_BUFFERLOST:
			return "DSERR_BUFFERLOST";

		case DSERR_INVALIDCALL:
			return "DSERR_INVALIDCALLS";

		case DSERR_INVALIDPARAM:
			return "DSERR_INVALIDPARAM";

		case DSERR_PRIOLEVELNEEDED:
			return "DSERR_PRIOLEVELNEEDED";

		default:
			return "unknown";
	}
}

static qboolean DS_CreateBuffers(void)
{
	si.Com_DPrintf("--------------------------------------\n"); // H2

	WAVEFORMATEX format = { 0 };
	format.wFormatTag = WAVE_FORMAT_PCM;
	format.nChannels = (ushort)dma.channels;
	format.wBitsPerSample = (ushort)dma.samplebits;
	format.nSamplesPerSec = dma.speed;
	format.nBlockAlign = format.nChannels * format.wBitsPerSample / 8;
	format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;

	si.Com_DPrintf("Creating DS buffers\n"); // Q2: Com_Printf.

	si.Com_DPrintf("...setting EXCLUSIVE coop level: ");
	if (pDS->lpVtbl->SetCooperativeLevel(pDS, si.cl_hwnd, DSSCL_EXCLUSIVE) != DS_OK)
	{
		si.Com_DPrintf("failed\n"); // Q2: Com_Printf.
		FreeSound();
		si.Com_DPrintf("--------------------------------------\n"); // H2

		return false;
	}

	si.Com_DPrintf("ok\n");

	// Get access to the primary buffer, if possible, so we can set the sound hardware format.
	DSBUFFERDESC dsbuf = { 0 };
	dsbuf.dwSize = sizeof(DSBUFFERDESC);
	dsbuf.dwFlags = DSBCAPS_PRIMARYBUFFER;
	dsbuf.dwBufferBytes = 0;
	dsbuf.lpwfxFormat = NULL;

	DSBCAPS dsbcaps = { 0 };
	dsbcaps.dwSize = sizeof(dsbcaps);
	primary_format_set = false;

	si.Com_DPrintf("...creating primary buffer: ");
	if (pDS->lpVtbl->CreateSoundBuffer(pDS, &dsbuf, &pDSPBuf, NULL) == DS_OK)
	{
		const WAVEFORMATEX pformat = format;

		si.Com_DPrintf("ok\n");
		if (pDSPBuf->lpVtbl->SetFormat(pDSPBuf, &pformat) != DS_OK)
		{
			if (snd_firsttime)
				si.Com_DPrintf("...setting primary sound format: failed\n");
		}
		else
		{
			if (snd_firsttime)
				si.Com_DPrintf("...setting primary sound format: ok\n");

			primary_format_set = true;
		}
	}
	else
	{
		si.Com_DPrintf("failed\n"); // Q2: Com_Printf.
	}

	if (!primary_format_set || !(int)s_primary->value)
	{
		// Create the secondary buffer we'll actually work with.
		memset(&dsbuf, 0, sizeof(dsbuf));
		dsbuf.dwSize = sizeof(DSBUFFERDESC);
		dsbuf.dwFlags = (DSBCAPS_CTRLFREQUENCY | DSBCAPS_LOCSOFTWARE);
		dsbuf.dwBufferBytes = SECONDARY_BUFFER_SIZE;
		dsbuf.lpwfxFormat = &format;

		memset(&dsbcaps, 0, sizeof(dsbcaps));
		dsbcaps.dwSize = sizeof(dsbcaps);

		si.Com_DPrintf("...creating secondary buffer: ");
		if (pDS->lpVtbl->CreateSoundBuffer(pDS, &dsbuf, &pDSBuf, NULL) != DS_OK)
		{
			si.Com_DPrintf("failed\n"); // Q2: Com_Printf.
			FreeSound();
			si.Com_DPrintf("--------------------------------------\n"); // H2

			return false;
		}

		si.Com_DPrintf("ok\n");

		dma.channels = format.nChannels;
		dma.samplebits = format.wBitsPerSample;
		dma.speed = (int)format.nSamplesPerSec;

		if (pDSBuf->lpVtbl->GetCaps(pDSBuf, &dsbcaps) != DS_OK)
		{
			si.Com_DPrintf("*** GetCaps failed ***\n"); // Q2: Com_Printf.
			FreeSound();
			si.Com_DPrintf("--------------------------------------\n"); // H2

			return false;
		}

		si.Com_DPrintf("...using secondary sound buffer\n"); // Q2: Com_Printf.
	}
	else
	{
		si.Com_DPrintf("...using primary buffer\n"); // Q2: Com_Printf.

		si.Com_DPrintf("...setting WRITEPRIMARY coop level: ");
		if (pDS->lpVtbl->SetCooperativeLevel(pDS, si.cl_hwnd, DSSCL_WRITEPRIMARY) != DS_OK)
		{
			si.Com_DPrintf("failed\n"); // Q2: Com_Printf.
			FreeSound();
			si.Com_DPrintf("--------------------------------------\n"); // H2

			return false;
		}

		si.Com_DPrintf("ok\n");

		if (pDSPBuf->lpVtbl->GetCaps(pDSPBuf, &dsbcaps) != DS_OK)
		{
			si.Com_DPrintf("*** GetCaps failed ***\n"); // Q2: Com_Printf.
			si.Com_DPrintf("--------------------------------------\n"); // H2

			return false;
		}

		pDSBuf = pDSPBuf;
	}

	// Make sure mixer is active.
	pDSBuf->lpVtbl->Play(pDSBuf, 0, 0, DSBPLAY_LOOPING);

	if (snd_firsttime)
		si.Com_Printf("   %d channel(s)\n   %d bits/sample\n   %d bytes/sec\n", dma.channels, dma.samplebits, dma.speed);

	gSndBufSize = dsbcaps.dwBufferBytes;

	// We don't want anyone to access the buffer directly w/o locking it first.
	lpData = NULL;

	DWORD dwWrite;
	pDSBuf->lpVtbl->Stop(pDSBuf);
	pDSBuf->lpVtbl->GetCurrentPosition(pDSBuf, &mmstarttime.u.sample, &dwWrite);
	pDSBuf->lpVtbl->Play(pDSBuf, 0, 0, DSBPLAY_LOOPING);

	dma.samples = (int)(gSndBufSize / (dma.samplebits / 8));
	dma.samplepos = 0;
	dma.submission_chunk = 1;
	dma.buffer = (byte*)lpData;

	sample16 = dma.samplebits / 8 - 1;

	si.Com_DPrintf("--------------------------------------\n"); // H2

	return true;
}

static void DS_DestroyBuffers(void)
{
	si.Com_DPrintf("--------------------------------------\n"); // H2
	si.Com_DPrintf("Destroying DS buffers\n");

	if (pDS != NULL)
	{
		si.Com_DPrintf("...setting NORMAL coop level\n");
		pDS->lpVtbl->SetCooperativeLevel(pDS, si.cl_hwnd, DSSCL_NORMAL);
	}

	if (pDSBuf != NULL)
	{
		si.Com_DPrintf("...stopping and releasing sound buffer\n");
		pDSBuf->lpVtbl->Stop(pDSBuf);
		pDSBuf->lpVtbl->Release(pDSBuf);
	}

	// Only release primary buffer if it's not also the mixing buffer we just released.
	if (pDSPBuf != NULL && pDSBuf != pDSPBuf)
	{
		si.Com_DPrintf("...releasing primary buffer\n");
		pDSPBuf->lpVtbl->Release(pDSPBuf);
	}

	pDSBuf = NULL;
	pDSPBuf = NULL;

	dma.buffer = NULL;

	si.Com_DPrintf("--------------------------------------\n"); // H2
}

// Q2 counterpart.
static void FreeSound(void)
{
	si.Com_DPrintf("Shutting down sound system\n");

	if (pDS != NULL)
		DS_DestroyBuffers();

	if (hWaveOut != NULL)
	{
		si.Com_DPrintf("...resetting waveOut\n");
		waveOutReset(hWaveOut);

		if (lpWaveHdr != NULL)
		{
			si.Com_DPrintf("...unpreparing headers\n");

			for (int i = 0; i < WAV_BUFFERS; i++)
				waveOutUnprepareHeader(hWaveOut, lpWaveHdr + i, sizeof(WAVEHDR));
		}

		si.Com_DPrintf("...closing waveOut\n");
		waveOutClose(hWaveOut);

		if (hWaveHdr != NULL)
		{
			si.Com_DPrintf("...freeing WAV header\n");
			GlobalUnlock(hWaveHdr);
			GlobalFree(hWaveHdr);
		}

		if (hData != NULL)
		{
			si.Com_DPrintf("...freeing WAV buffer\n");
			GlobalUnlock(hData);
			GlobalFree(hData);
		}
	}

	if (pDS != NULL)
	{
		si.Com_DPrintf("...releasing DS object\n");
		pDS->lpVtbl->Release(pDS);
	}

	if (hInstDS != NULL)
	{
		si.Com_DPrintf("...freeing DSOUND.DLL\n");
		FreeLibrary(hInstDS);
		hInstDS = NULL;
	}

	pDS = NULL;
	pDSBuf = NULL;
	pDSPBuf = NULL;
	hWaveOut = NULL;
	hData = NULL;
	hWaveHdr = NULL;
	lpData = NULL;
	lpWaveHdr = NULL;

	dsound_init = false;
	wav_init = false;
}

// DirectSound support.
static sndinitstat SNDDMA_InitDirect(void)
{
	static HRESULT (WINAPI *pDirectSoundCreate)(GUID FAR* lpGUID, LPDIRECTSOUND FAR* lplpDS, IUnknown FAR* pUnkOuter) = NULL; //mxd. Made local static.

	dma.channels = 2;
	dma.samplebits = 16;
	dma.speed = (s_khz->value == 44.0f ? 44100 : 22050); // H2: no 11025 speed.

	si.Com_Printf("Initializing DirectSound\n");

	if (hInstDS == NULL)
	{
		si.Com_DPrintf("...loading dsound.dll: ");
		hInstDS = LoadLibrary("dsound.dll");

		if (hInstDS == NULL)
		{
			si.Com_Printf("failed\n");
			return SIS_FAILURE;
		}

		si.Com_DPrintf("ok\n");
		pDirectSoundCreate = (void*)GetProcAddress(hInstDS, "DirectSoundCreate");

		if (pDirectSoundCreate == NULL)
		{
			si.Com_Printf("*** couldn't get DS proc addr ***\n");
			return SIS_FAILURE;
		}
	}

	si.Com_DPrintf("...creating DS object: ");

	HRESULT hresult;
	while ((hresult = pDirectSoundCreate(NULL, &pDS, NULL)) != DS_OK)
	{
		if (hresult != DSERR_ALLOCATED)
		{
			si.Com_Printf("failed\n");
			return SIS_FAILURE;
		}

		if (MessageBox(NULL, "The sound hardware is in use by another app.\n\nSelect Retry to try to start sound again or Cancel to run "GAME_NAME" with no sound.",
			"Sound not available", MB_RETRYCANCEL | MB_SETFOREGROUND | MB_ICONEXCLAMATION) != IDRETRY) // H2: Quake -> Heretic 2.
		{
			si.Com_Printf("failed, hardware already in use\n");
			return SIS_NOTAVAIL;
		}
	}

	si.Com_DPrintf("ok\n");

	DSCAPS dscaps = { .dwSize = sizeof(dscaps) };
	if (pDS->lpVtbl->GetCaps(pDS, &dscaps) != DS_OK)
		si.Com_Printf("*** couldn't get DS caps ***\n");

	if (dscaps.dwFlags & DSCAPS_EMULDRIVER)
	{
		si.Com_DPrintf("...no DSound driver found\n");
		FreeSound();

		return SIS_FAILURE;
	}

	if (!DS_CreateBuffers())
		return SIS_FAILURE;

	dsound_init = true;

	si.Com_DPrintf("...completed successfully\n");

	return SIS_SUCCESS;
}

// Crappy windows multimedia base.
static qboolean SNDDMA_InitWav(void)
{
	si.Com_Printf("Initializing wave sound\n");

	snd_sent = 0;
	snd_completed = 0;

	dma.channels = 2;
	dma.samplebits = 16;
	dma.speed = (s_khz->value == 44.0f ? 44100 : 22050); // H2: no 11025 speed.

	WAVEFORMATEX format;
	format.wFormatTag = WAVE_FORMAT_PCM;
	format.nChannels = (short)dma.channels;
	format.wBitsPerSample = (short)dma.samplebits;
	format.nSamplesPerSec = dma.speed;
	format.nBlockAlign = format.nChannels * format.wBitsPerSample / 8;
	format.cbSize = 0;
	format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;

	// Open a waveform device for output using window callback.
	si.Com_DPrintf("...opening waveform device: ");

	MMRESULT hr;
	while ((hr = waveOutOpen(&hWaveOut, WAVE_MAPPER, &format, 0, 0, CALLBACK_NULL)) != MMSYSERR_NOERROR)
	{
		if (hr != MMSYSERR_ALLOCATED)
		{
			si.Com_Printf("failed\n");
			return false;
		}

		if (MessageBox(NULL, "The sound hardware is in use by another app.\n\nSelect Retry to try to start sound again or Cancel to run "GAME_NAME" with no sound.",
			"Sound not available", MB_RETRYCANCEL | MB_SETFOREGROUND | MB_ICONEXCLAMATION) != IDRETRY) // H2: Quake 2 -> Heretic 2.
		{
			si.Com_Printf("failed, hardware already in use\n");
			return false;
		}
	}

	si.Com_DPrintf("ok\n");

	// Allocate and lock memory for the waveform data. The memory for waveform data must be globally allocated with GMEM_MOVEABLE and GMEM_SHARE flags.
	si.Com_DPrintf("...allocating waveform buffer: ");

	gSndBufSize = WAV_BUFFERS * WAV_BUFFER_SIZE;
	hData = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, gSndBufSize);

	if (hData == NULL)
	{
		si.Com_Printf(" failed\n");
		FreeSound();

		return false;
	}

	si.Com_DPrintf("ok\n");

	si.Com_DPrintf("...locking waveform buffer: ");
	lpData = GlobalLock(hData);

	if (lpData == NULL)
	{
		si.Com_Printf(" failed\n");
		FreeSound();

		return false;
	}

	memset(lpData, 0, gSndBufSize);
	si.Com_DPrintf("ok\n");

	// Allocate and lock memory for the header. This memory must also be globally allocated with GMEM_MOVEABLE and GMEM_SHARE flags.
	si.Com_DPrintf("...allocating waveform header: ");
	hWaveHdr = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, (DWORD)sizeof(WAVEHDR) * WAV_BUFFERS);

	if (hWaveHdr == NULL)
	{
		si.Com_Printf("failed\n");
		FreeSound();

		return false;
	}

	si.Com_DPrintf("ok\n");

	si.Com_DPrintf("...locking waveform header: ");
	lpWaveHdr = (LPWAVEHDR)GlobalLock(hWaveHdr);

	if (lpWaveHdr == NULL)
	{
		si.Com_Printf("failed\n");
		FreeSound();

		return false;
	}

	memset(lpWaveHdr, 0, sizeof(WAVEHDR) * WAV_BUFFERS);
	si.Com_DPrintf("ok\n");

	// After allocation, set up and prepare headers.
	si.Com_DPrintf("...preparing headers: ");

	for (int i = 0; i < WAV_BUFFERS; i++)
	{
		lpWaveHdr[i].dwBufferLength = WAV_BUFFER_SIZE;
		lpWaveHdr[i].lpData = lpData + i * WAV_BUFFER_SIZE;

		if (waveOutPrepareHeader(hWaveOut, lpWaveHdr + i, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
		{
			si.Com_Printf("failed\n");
			FreeSound();

			return false;
		}
	}

	si.Com_DPrintf("ok\n");

	dma.samples = (int)(gSndBufSize / (dma.samplebits / 8));
	dma.samplepos = 0;
	dma.submission_chunk = 512;
	dma.buffer = (byte*)lpData;
	sample16 = (dma.samplebits / 8) - 1;

	wav_init = true;

	return true;
}

qboolean SNDDMA_Init(void) //mxd. Returns int in original logic.
{
	memset(&dma, 0, sizeof(dma));

	const cvar_t* s_wavonly = si.Cvar_Get("s_wavonly", "0", 0);

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
				si.Com_Printf("dsound init succeeded\n");
		}
		else
		{
			snd_isdirect = false;
			si.Com_Printf("*** dsound init failed ***\n");
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
				si.Com_Printf("Wave sound init succeeded\n");
		}
		else
		{
			si.Com_Printf("Wave sound init failed\n");
		}
	}

	snd_firsttime = false;
	snd_buffer_count = 1;

	return (dsound_init || wav_init); //H2: missing '*** No sound device initialized ***' logic.
}

void SNDDMA_Shutdown(void)
{
	FreeSound();
}

// Q2 counterpart.
// Returns the current sample position (in mono samples read) inside the recirculating dma buffer,
// so the mixing code will know how many sample are required to fill it up.
int SNDDMA_GetDMAPos(void)
{
	int s = 0; //mxd. Initialize to avoid compiler warning.
	
	if (dsound_init)
	{
		MMTIME mmtime;
		DWORD dwWrite;
		pDSBuf->lpVtbl->GetCurrentPosition(pDSBuf, &mmtime.u.sample, &dwWrite);

		s = (int)(mmtime.u.sample - mmstarttime.u.sample);
	}
	else if (wav_init)
	{
		s = snd_sent * WAV_BUFFER_SIZE;
	}

	return (s >> sample16) & (dma.samples - 1);
}

// Makes sure dma.buffer is valid.
void SNDDMA_BeginPainting(void)
{
	if (pDSBuf == NULL)
		return;

	// If the buffer was lost or stopped, restore it and/or restart it.
	DWORD dwStatus;
	if (pDSBuf->lpVtbl->GetStatus(pDSBuf, &dwStatus) != DS_OK)
		si.Com_Printf("Couldn't get sound buffer status\n");

	if (dwStatus & DSBSTATUS_BUFFERLOST)
		pDSBuf->lpVtbl->Restore(pDSBuf);

	if (!(dwStatus & DSBSTATUS_PLAYING))
		pDSBuf->lpVtbl->Play(pDSBuf, 0, 0, DSBPLAY_LOOPING);

	// Lock the dsound buffer.
	int retries = 0;
	dma.buffer = NULL;

	HRESULT hresult;
	DWORD* pbuf;
	DWORD* pbuf2;
	DWORD dwSize2;
	while ((hresult = pDSBuf->lpVtbl->Lock(pDSBuf, 0, gSndBufSize, (LPVOID*)&pbuf, &locksize, (LPVOID*)&pbuf2, &dwSize2, 0)) != DS_OK)
	{
		if (hresult != DSERR_BUFFERLOST)
		{
			si.Com_Printf("SNDDMA_BeginPainting: Lock failed with error '%s'\n", DSoundError(hresult)); //mxd. Print correct function name.
			S_Shutdown();

			return;
		}

		pDSBuf->lpVtbl->Restore(pDSBuf);

		if (++retries > 2)
			return;
	}

	dma.buffer = (byte*)pbuf;
}

// Send sound to device if buffer isn't really the dma buffer. Also unlocks the dsound buffer.
void SNDDMA_Submit(void)
{
	if (dma.buffer == NULL)
		return;

	// Unlock the dsound buffer.
	if (pDSBuf != NULL)
		pDSBuf->lpVtbl->Unlock(pDSBuf, dma.buffer, locksize, NULL, 0);

	if (!wav_init)
		return;

	// Find which sound blocks have completed.
	while (true)
	{
		if (snd_completed == snd_sent)
		{
			si.Com_DPrintf("Sound overrun\n");
			break;
		}

		if (!(lpWaveHdr[snd_completed & WAV_MASK].dwFlags & WHDR_DONE))
			break;

		snd_completed++; // This buffer has been played.
	}

	// Submit a few new sound blocks.
	while ((snd_sent - snd_completed) >> sample16 < 8 && paintedtime / 256 <= snd_sent)
	{
		snd_sent++;

		// Now the data block can be sent to the output device.
		// The waveOutWrite function returns immediately and waveform data is sent to the output device in the background.
		if (waveOutWrite(hWaveOut, &lpWaveHdr[snd_completed & WAV_MASK], sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
		{
			si.Com_Printf("Failed to write block to device\n");
			FreeSound();

			return;
		}
	}
}

// Q2 counterpart.
// Called when the main window gains or loses focus.
// The window have been destroyed and recreated between a deactivate and an activate.
void S_Activate(const qboolean active)
{
	if (pDS != NULL && si.cl_hwnd != NULL && snd_isdirect)
	{
		if (active)
			DS_CreateBuffers();
		else
			DS_DestroyBuffers();
	}
}