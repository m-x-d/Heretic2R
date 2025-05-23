//
// snd_dma.c -- Main control for any streaming sound output device.
//
// Copyright 1998 Raven Software
//

#include "snd_dma.h"

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
	NOT_IMPLEMENTED
}

void S_StopAllSounds_Sounding(void) // H2
{
	NOT_IMPLEMENTED
}

void S_Update(vec3_t origin, vec3_t forward, vec3_t right, vec3_t up)
{
	NOT_IMPLEMENTED
}