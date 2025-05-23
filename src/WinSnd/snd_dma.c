//
// snd_dma.c -- Main control for any streaming sound output device.
//
// Copyright 1998 Raven Software
//

#include "snd_dma.h"

// Internal sound data & structures.
channel_t channels[MAX_CHANNELS];

static qboolean sound_started = false; //mxd. int in Q2.

#define MAX_PLAYSOUNDS	128
static playsound_t s_playsounds[MAX_PLAYSOUNDS];
static playsound_t s_freeplays;
playsound_t s_pendingplays;

void S_Init(void)
{
	NOT_IMPLEMENTED
}

// Shutdown sound engine.
void S_Shutdown(void)
{
	NOT_IMPLEMENTED
}

sfx_t* S_FindName(char* name, qboolean create)
{
	NOT_IMPLEMENTED
	return NULL;
}

void S_BeginRegistration(void)
{
	NOT_IMPLEMENTED
}

sfx_t* S_RegisterSound(const char* name)
{
	NOT_IMPLEMENTED
	return NULL;
}

void S_EndRegistration(void)
{
	NOT_IMPLEMENTED
}

void S_StartSound(const vec3_t origin, int entnum, int entchannel, sfx_t* sfx, float fvol, int attenuation, float timeofs)
{
	NOT_IMPLEMENTED
}

void S_StartLocalSound(const char* sound)
{
	NOT_IMPLEMENTED
}

void S_StopAllSounds(void)
{
	if (!sound_started)
		return;

	// Clear all the playsounds.
	memset(s_playsounds, 0, sizeof(s_playsounds));

	s_freeplays.next = &s_freeplays;
	s_freeplays.prev = &s_freeplays;
	s_pendingplays.next = &s_pendingplays;
	s_pendingplays.prev = &s_pendingplays;

	for (int i = 0; i < MAX_PLAYSOUNDS; i++)
	{
		s_playsounds[i].prev = &s_freeplays;
		s_playsounds[i].next = s_freeplays.next;
		s_playsounds[i].prev->next = &s_playsounds[i];
		s_playsounds[i].next->prev = &s_playsounds[i];
	}

	// Clear all the channels.
	memset(channels, 0, sizeof(channels));
	// H2: no S_ClearBuffer() call.
}

void S_StopAllSounds_Sounding(void) // H2
{
	NOT_IMPLEMENTED
}

void S_Update(vec3_t origin, vec3_t forward, vec3_t right, vec3_t up)
{
	NOT_IMPLEMENTED
}