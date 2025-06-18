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

void OGG_Stream(void)
{
	//NOT_IMPLEMENTED //TODO: implement when the rest of the sound logic is in.
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
	//NOT_IMPLEMENTED //TODO: implement when the rest of the sound logic is in.
}