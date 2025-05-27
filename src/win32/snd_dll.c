//
// snd_dll.c -- Sound library initialization
//
// Copyright 1998 Raven Software
//

#include <windows.h>
#include "qcommon.h"
#include "client.h"
#include "sound.h"
#include "snd_dll.h" //mxd
#include "clfx_dll.h"

cvar_t* snd_dll;
HINSTANCE sound_library;

// Sound library function pointers.
void (*S_Init)(void);
void (*S_Shutdown)(void);
void (*S_StartSound)(const vec3_t origin, int entnum, int entchannel, struct sfx_s* sfx, float fvol, int attenuation, float timeofs); //TODO: float attenuation in Q2. Which is correct?..
void (*S_StartLocalSound)(const char* sound);
void (*S_StopAllSounds)(void);
void (*S_StopAllSounds_Sounding)(void);
void (*S_Update)(const vec3_t origin, const vec3_t forward, const vec3_t right, const vec3_t up);
void (*S_Activate)(qboolean active);
void (*S_BeginRegistration)(void);
struct sfx_s* (*S_RegisterSound)(const char* name);
void (*S_EndRegistration)(void);
struct sfx_s* (*S_FindName)(const char* name, qboolean create);

#ifdef __A3D_GEOM
void (*S_A3D_ExportRenderGeom)(refexport_t* re);
#endif

void (*SNDEAX_SetEnvironment)(int env_index);
void (*SNDEAX_SetEnvironmentLevel)(int level);

static qboolean A3D_CheckAvailability(void)
{
	return false; //TODO: remove?
}

static qboolean EAX_CheckAvailability(void)
{
	return false; //TODO: remove?
}

void SndDll_FreeLibrary(void)
{
	if (sound_library != NULL && !FreeLibrary(sound_library))
		Sys_Error("Sound Lib FreeLibrary failed");

	sound_library = NULL;
}

void SndDll_Init(void)
{
	if (sound_library != NULL)
	{
		S_Shutdown();
		SndDll_FreeLibrary();
	}
 
	if (snd_dll == NULL)
		snd_dll = Cvar_Get("snd_dll", "", 0);

	if (Q_stricmp(snd_dll->string, "a3dsnd") == 0)
	{
		Com_ColourPrintf(P_OBIT, "Attempting A3D 2.0 support\n");
		if (!A3D_CheckAvailability())
		{
			Com_ColourPrintf(P_OBIT, "A3D NOT supported\n");
			Cvar_Set("snd_dll", "winsnd");
		}
	}
	else if (Q_stricmp(snd_dll->string, "eaxsnd") == 0)
	{
		Com_ColourPrintf(P_OBIT, "Attempting EAX 1.0 support\n");
		if (!EAX_CheckAvailability())
		{
			Com_ColourPrintf(P_OBIT, "EAX NOT supported\n");
			Cvar_Set("snd_dll", "winsnd");
		}
	}
	else if (Q_stricmp(snd_dll->string, "") == 0)
	{
		if (A3D_CheckAvailability())
		{
			Com_ColourPrintf(P_OBIT, "Found A3D 2.0 support\n");
			Cvar_Set("snd_dll", "a3dsnd");
		}
		else if (EAX_CheckAvailability())
		{
			Com_ColourPrintf(P_OBIT, "Found EAX support\n");
			Cvar_Set("snd_dll", "eaxsnd");
		}
		else
		{
			Cvar_Set("snd_dll", "winsnd");
		}
	}

	if (Q_stricmp(snd_dll->string, "") == 0) //mxd. For whatever reason this check is inside of 'if (sound_library == NULL)' block in original version.
		Sys_Error("Couldn\'t load default sound dll!");
	else if (Q_stricmp(snd_dll->string, "winsnd") == 0)
		Com_ColourPrintf(P_OBIT, "Setting Default Sound support\n");

	Com_ColourPrintf(P_HEADER, "------- Loading %s -------\n", snd_dll->string);

	sound_library = LoadLibrary(snd_dll->string);
	if (sound_library == NULL)
	{
		Cvar_Set("snd_dll", "winsnd");

		sound_library = LoadLibrary(snd_dll->string);
		if (sound_library == NULL)
			Sys_Error("LoadLibrary(\"%s\") failed\n", snd_dll->string);
	}

	// Bind sound library function pointers.
	S_Init = (void*)GetProcAddress(sound_library, "S_Init");
	if (S_Init == NULL)
		Sys_Error("GetProcAddress failed on %s", snd_dll->string);

	S_Shutdown = (void*)GetProcAddress(sound_library, "S_Shutdown");
	if (S_Shutdown == NULL)
		Sys_Error("GetProcAddress failed on %s", snd_dll->string);

	S_StartSound = (void*)GetProcAddress(sound_library, "S_StartSound");
	if (S_StartSound == NULL)
		Sys_Error("GetProcAddress failed on %s", snd_dll->string);

	S_StartLocalSound = (void*)GetProcAddress(sound_library, "S_StartLocalSound");
	if (S_StartLocalSound == NULL)
		Sys_Error("GetProcAddress failed on %s", snd_dll->string);

	S_StopAllSounds = (void*)GetProcAddress(sound_library, "S_StopAllSounds");
	if (S_StopAllSounds == NULL)
		Sys_Error("GetProcAddress failed on %s", snd_dll->string);

	// H2:
	S_StopAllSounds_Sounding = (void*)GetProcAddress(sound_library, "S_StopAllSounds_Sounding");
	if (S_StopAllSounds_Sounding == NULL)
		Sys_Error("GetProcAddress failed on %s", snd_dll->string);

	S_Update = (void*)GetProcAddress(sound_library, "S_Update");
	if (S_Update == NULL)
		Sys_Error("GetProcAddress failed on %s", snd_dll->string);

	S_Activate = (void*)GetProcAddress(sound_library, "S_Activate");
	if (S_Activate == NULL)
		Sys_Error("GetProcAddress failed on %s", snd_dll->string);

	S_BeginRegistration = (void*)GetProcAddress(sound_library, "S_BeginRegistration");
	if (S_BeginRegistration == NULL)
		Sys_Error("GetProcAddress failed on %s", snd_dll->string);

	S_RegisterSound = (void*)GetProcAddress(sound_library, "S_RegisterSound");
	if (S_RegisterSound == NULL)
		Sys_Error("GetProcAddress failed on %s", snd_dll->string);

	S_EndRegistration = (void*)GetProcAddress(sound_library, "S_EndRegistration");
	if (S_EndRegistration == NULL)
		Sys_Error("GetProcAddress failed on %s", snd_dll->string);

	S_FindName = (void*)GetProcAddress(sound_library, "S_FindName");
	if (S_FindName == NULL)
		Sys_Error("GetProcAddress failed on %s", snd_dll->string);

	// H2:
#ifdef __A3D_GEOM
	S_A3D_ExportRenderGeom = GetProcAddress(sound_library, "S_A3D_ExportRenderGeom");
	if (S_A3D_ExportRenderGeom != NULL)
		S_A3D_ExportRenderGeom(&re);
	else if (re.A3D_RenderGeometry != NULL)
		re.A3D_RenderGeometry = NULL;
#endif

	SNDEAX_SetEnvironment = (void*)GetProcAddress(sound_library, "SNDEAX_SetEnvironment"); // H2
	SNDEAX_SetEnvironmentLevel = (void*)GetProcAddress(sound_library, "SNDEAX_SetEnvironmentLevel"); // H2 //TODO: unused?

	fxi.S_StartSound = S_StartSound;
	fxi.S_RegisterSound = S_RegisterSound;

	if (GetProcAddress(clfx_library, "GetfxAPI"))
		CLFX_Init();

	Com_ColourPrintf(P_HEADER, "------------------------------------\n");
}