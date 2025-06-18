//
// snd_ogg.c
//
// Copyright 2025 mxd
//

#include "snd_ogg.h"
#include "client.h"
#include "snd_local.h"

typedef enum
{
	OGG_PLAY,
	OGG_PAUSE,
	OGG_STOP
} ogg_status_t;

static cvar_t* ogg_volume; // Music volume.

static int ogg_curfile; // Index of currently played file.
static int ogg_numsamples; // Number of samples read from the current file.
static ogg_status_t ogg_status; // Status indicator.
static qboolean ogg_started;

#define MAX_OGG_TRACKS	128
static char* ogg_tracks[MAX_OGG_TRACKS];
static int ogg_maxfileindex;

void OGG_Stream(void)
{
	//NOT_IMPLEMENTED //TODO: implement when the rest of the sound logic is in.
}

// Stop playing the current file.
void OGG_Stop(void)
{
	NOT_IMPLEMENTED
}

// Initialize the Ogg Vorbis subsystem.
void OGG_Init(void)
{
	// Cvars.
	ogg_volume = si.Cvar_Get("m_volume", "0.5", CVAR_ARCHIVE);

	// Global variables.
	ogg_curfile = -1;
	ogg_numsamples = 0;
	ogg_status = OGG_STOP;

	ogg_started = true;
}

// Shutdown the Ogg Vorbis subsystem.
void OGG_Shutdown(void)
{
	if (!ogg_started)
		return;

	// Music must be stopped.
	OGG_Stop();

	// Free file list.
	for (int i = 0; i < MAX_OGG_TRACKS; i++)
	{
		if (ogg_tracks[i] != NULL)
		{
			free(ogg_tracks[i]);
			ogg_tracks[i] = NULL;
		}
	}

	ogg_maxfileindex = 0;
	ogg_started = false;
}