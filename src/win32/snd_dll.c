//
// snd_dll.c -- Sound library initialization
//
// Copyright 1998 Raven Software
//

#include <windows.h>
#include "qcommon.h"
#include "client.h"
#include "snd_dll.h" //mxd
#include "clfx_dll.h"
#include "sys_win.h"

snd_export_t se; //mxd

cvar_t* snd_dll;
static HINSTANCE sound_library;

static void InitClientEffects(void)
{
	fxi.S_StartSound = se.StartSound;
	fxi.S_RegisterSound = se.RegisterSound;

	if (GetProcAddress(clfx_library, "GetfxAPI"))
		CLFX_Init();
}

#pragma region ========================== NULL SOUND CALLBACKS ==========================

static void NullSnd_Init(void) { }
static void NullSnd_Shutdown(void) { }
static void NullSnd_StartSound(const vec3_t origin, int entnum, int entchannel, struct sfx_s* sfx, float fvol, int attenuation, float timeofs) { }
static void NullSnd_StartLocalSound(const char* sound) { }
static void NullSnd_StopAllSounds(void) { }
static void NullSnd_StopAllSounds_Sounding(void) { }
static void NullSnd_Update(const vec3_t origin, const vec3_t forward, const vec3_t right, const vec3_t up) { }
static void NullSnd_Activate(qboolean active) { }
static void NullSnd_BeginRegistration(void) { }
static struct sfx_s* NullSnd_RegisterSound(const char* name) { return NULL; }
static void NullSnd_EndRegistration(void) { }
static struct sfx_s* NullSnd_FindName(const char* name, qboolean create) { return NULL; }

//mxd. So we can carry on without sound library...
static void InitNullSound(void)
{
	se.api_version = SND_API_VERSION;
	se.library_name = "Null Sound";

	se.Init = NullSnd_Init;
	se.Shutdown = NullSnd_Shutdown;

	se.StartSound = NullSnd_StartSound;
	se.StartLocalSound = NullSnd_StartLocalSound;

	se.StopAllSounds = NullSnd_StopAllSounds;
	se.StopAllSounds_Sounding = NullSnd_StopAllSounds_Sounding;

	se.Update = NullSnd_Update;
	se.Activate = NullSnd_Activate;

	se.BeginRegistration = NullSnd_BeginRegistration;
	se.RegisterSound = NullSnd_RegisterSound;
	se.EndRegistration = NullSnd_EndRegistration;

	se.FindName = NullSnd_FindName;

	se.SetEaxEnvironment = NULL;

	InitClientEffects();
}

#pragma endregion

void SndDll_FreeLibrary(void)
{
	if (sound_library != NULL)
	{
		if (se.SetEaxEnvironment != NULL) //mxd. Done in CL_Shutdown() in original logic.
		{
			se.SetEaxEnvironment(0);
			se.SetEaxEnvironment = NULL;
		}

		if (!FreeLibrary(sound_library))
			Sys_Error("Sound Lib FreeLibrary failed");
	}

	memset(&se, 0, sizeof(se)); //mxd
	sound_library = NULL;
}

void SndDll_Init(void)
{
	if (sound_library != NULL)
	{
		se.Shutdown();
		SndDll_FreeLibrary();
	}

	if (snd_dll == NULL)
		snd_dll = Cvar_Get("snd_dll", DEFAULT_SOUND_LIBRARY_NAME, CVAR_ARCHIVE); //mxd. Make archiveable.

	Com_ColourPrintf(P_HEADER, "------- Loading %s -------\n", snd_dll->string);

	sound_library = LoadLibrary(snd_dll->string);
	if (sound_library == NULL && Q_stricmp(snd_dll->string, DEFAULT_SOUND_LIBRARY_NAME) != 0)
	{
		Com_ColourPrintf(P_OBIT, "Failed to load '%s' sound library. Trying default sound library...\n", snd_dll->string);
		Cvar_Set("snd_dll", DEFAULT_SOUND_LIBRARY_NAME);

		sound_library = LoadLibrary(snd_dll->string);
	}

	if (sound_library == NULL)
	{
		Com_ColourPrintf(P_RED, "Failed to load '%s' sound library.\n", snd_dll->string);
		InitNullSound();

		return;
	}

	snd_import_t si;

	si.cl_hwnd = cl_hwnd;
	si.cl = &cl;
	si.cls = &cls;
	si.cl_entities = cl_entities;
	si.cl_parse_entities = cl_parse_entities;

	si.Com_Error = Com_Error;
	si.Com_Printf = Com_Printf;
	si.Com_DPrintf = Com_DPrintf;

	si.Cvar_Get = Cvar_Get;
	si.Cvar_Set = Cvar_Set;
	si.Cmd_AddCommand = Cmd_AddCommand;
	si.Cmd_RemoveCommand = Cmd_RemoveCommand;

	si.Cmd_Argc = Cmd_Argc;
	si.Cmd_Argv = Cmd_Argv;

	si.FS_FOpenFile = FS_FOpenFile;
	si.FS_FCloseFile = FS_FCloseFile;
	si.FS_LoadFile = FS_LoadFile;
	si.FS_FreeFile = FS_FreeFile;
	si.FS_Gamedir = FS_Gamedir;

	si.Z_Malloc = Z_Malloc;
	si.Z_Free = Z_Free;

#ifdef _DEBUG
	si.pv = pv;
	si.psv = psv;

	si.DBG_IDEPrint = DBG_IDEPrint;
	si.DBG_HudPrint = DBG_HudPrint;

	si.DBG_AddBox = DBG_AddBox;
	si.DBG_AddBbox = DBG_AddBbox;
	si.DBG_AddEntityBbox = DBG_AddEntityBbox;

	si.DBG_AddLine = DBG_AddLine;
	si.DBG_AddArrow = DBG_AddArrow;
#endif

	const GetSoundAPI_t GetSoundAPI = (void*)GetProcAddress(sound_library, "GetSoundAPI");
	if (GetSoundAPI == NULL)
	{
		Com_ColourPrintf(P_RED, "GetProcAddress failed on %s\n", snd_dll->string);
		SndDll_FreeLibrary();
		InitNullSound();

		return;
	}

	se = GetSoundAPI(si);

	if (se.api_version != SND_API_VERSION)
	{
		Com_ColourPrintf(P_RED, "%s has incompatible api_version\n", snd_dll->string);
		SndDll_FreeLibrary();

		// Retry with default sound library?..
		if (Q_stricmp(snd_dll->string, DEFAULT_SOUND_LIBRARY_NAME) != 0)
		{
			Cvar_Set("snd_dll", DEFAULT_SOUND_LIBRARY_NAME);
			SndDll_Init();
		}
		else
		{
			InitNullSound();
		}

		return;
	}

	InitClientEffects();
	Com_ColourPrintf(P_HEADER, "------------------------------------\n");
}