//
// snd_main.c -- Main control for SDL3 streaming sound output device.
//
// Copyright 2025 mxd
//

#include "snd_main.h"

#define SNDLIB_DECLSPEC __declspec(dllexport)

snd_import_t si;

// Activate or deactivate sound backend.
static void S_Activate(qboolean active)
{
	NOT_IMPLEMENTED
}

static void S_Init(void)
{
	NOT_IMPLEMENTED
}

static void S_Shutdown(void)
{
	NOT_IMPLEMENTED
}

// Q2 counterpart.
static sfx_t* S_FindName(const char* name, const qboolean create)
{
	NOT_IMPLEMENTED
	return NULL;
}

// Q2 counterpart.
static void S_BeginRegistration(void)
{
	NOT_IMPLEMENTED
}

// Q2 counterpart.
static sfx_t* S_RegisterSound(const char* name)
{
	NOT_IMPLEMENTED
	return NULL;
}

// Q2 counterpart.
static void S_EndRegistration(void)
{
	NOT_IMPLEMENTED
}

// Q2 counterpart.
// Validates the params and queues the sound up.
// If origin is NULL, the sound will be dynamically sourced from the entity.
// Entchannel 0 will never override a playing sound.
static void S_StartSound(const vec3_t origin, const int ent_num, const int ent_channel, sfx_t* sfx, const float volume, const int attenuation, const float time_offset)
{
	NOT_IMPLEMENTED
}

// Q2 counterpart.
static void S_StartLocalSound(const char* name)
{
	NOT_IMPLEMENTED
}

static void S_StopAllSounds(void)
{
	NOT_IMPLEMENTED
}

static void S_StopAllSounds_Sounding(void) // H2
{
	NOT_IMPLEMENTED
}

// Called once each time through the main loop.
static void S_Update(const vec3_t origin, const vec3_t forward, const vec3_t right, const vec3_t up)
{
	NOT_IMPLEMENTED
}

SNDLIB_DECLSPEC snd_export_t GetSoundAPI(const snd_import_t snd_import)
{
	snd_export_t snd_export;

	si = snd_import;

	snd_export.api_version = SND_API_VERSION;
	snd_export.library_name = "SDL3";

	snd_export.Init = S_Init;
	snd_export.Shutdown = S_Shutdown;

	snd_export.StartSound = S_StartSound;
	snd_export.StartLocalSound = S_StartLocalSound;

	snd_export.StopAllSounds = S_StopAllSounds;
	snd_export.StopAllSounds_Sounding = S_StopAllSounds_Sounding;

	snd_export.Update = S_Update;
	snd_export.Activate = S_Activate;

	snd_export.BeginRegistration = S_BeginRegistration;
	snd_export.RegisterSound = S_RegisterSound;
	snd_export.EndRegistration = S_EndRegistration;

	snd_export.FindName = S_FindName;

	snd_export.SetEaxEnvironment = NULL;

	return snd_export;
}