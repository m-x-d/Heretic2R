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

snd_export_t se; //mxd

//mxd. Compatible snd_xxx.dlls info. For use in menu_sound.c.
sndlib_info_t sndlib_infos[MAX_SNDLIBS];
int num_sndlib_infos = 0;

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
static void NullSnd_MusicPlay(int track, qboolean looping) { }
static void NullSnd_MusicStop(void) { }
static void NullSnd_RawSamples(int samples, uint rate, int width, int num_channels, const byte* data, float volume) { }

//mxd. So we can carry on without sound library...
static void InitNullSound(void)
{
	se.api_version = SND_API_VERSION;
	se.title = "Null Sound";

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

	se.MusicPlay = NullSnd_MusicPlay;
	se.MusicStop = NullSnd_MusicStop;
	se.RawSamples = NullSnd_RawSamples;

	se.SetEaxEnvironment = NULL;

	InitClientEffects();
}

#pragma endregion

static qboolean SND_StoreSndlibInfo(const char* snd_path) //mxd
{
	// Try loading it...
	const HINSTANCE sndlib = LoadLibrary(snd_path); //TODO: replace with YQ2 Sys_LoadLibrary()?
	if (sndlib == NULL)
		return false;

	const GetSoundAPI_t GetSoundAPI = (void*)GetProcAddress(sndlib, "GetSoundAPI");
	if (GetSoundAPI == NULL)
	{
		FreeLibrary(sndlib);
		return false;
	}

	const snd_import_t snd_import = { 0 }; // Assume GetSoundAPI() doesn't use snd_import function pointers...
	const snd_export_t snd_export = GetSoundAPI(snd_import);

	if (snd_export.api_version != SND_API_VERSION || snd_export.title == NULL)
	{
		FreeLibrary(sndlib);
		return false;
	}

	const char* start = strchr(snd_path, '_');
	const char* end = strrchr(snd_path, '.');
	const qboolean is_valid = (start != NULL && end != NULL);

	if (is_valid)
	{
		// Seems valid. Store info...
		sndlib_info_t* info = &sndlib_infos[num_sndlib_infos];

		strcpy_s(info->title, sizeof(info->title), snd_export.title);
		strncpy_s(info->id, sizeof(info->id), snd_path, end - snd_path - 1); // Strip ".dll" part...
	}

	FreeLibrary(sndlib);

	return is_valid;
}

static void SND_InitSndlibInfos(void) //mxd
{
	num_sndlib_infos = 0;

	// Find all compatible snd_xxx.dll libraries.
	char mask[MAX_QPATH];
	Com_sprintf(mask, sizeof(mask), "snd_*.dll");

	const char* snd_path = Sys_FindFirst(mask, 0, 0);

	while (snd_path != NULL && num_sndlib_infos < MAX_SNDLIBS)
	{
		const char* path = strchr(snd_path, '/') + 1; // Skip starting '/'...
		if (SND_StoreSndlibInfo(path != NULL ? path : snd_path))
			num_sndlib_infos++;

		snd_path = Sys_FindNext(0, 0);
	}

	Sys_FindClose();
}

static void SndDll_FreeLibrary(void)
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

static void SndDll_Init(void)
{
	SND_Shutdown();

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
		Com_ColourPrintf(P_ERROR, "Failed to load '%s' sound library.\n", snd_dll->string);
		InitNullSound();

		return;
	}

	snd_import_t si;

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
		Com_ColourPrintf(P_ERROR, "GetProcAddress failed on %s\n", snd_dll->string);
		SndDll_FreeLibrary();
		InitNullSound();

		return;
	}

	se = GetSoundAPI(si);

	if (se.api_version != SND_API_VERSION)
	{
		Com_ColourPrintf(P_ERROR, "%s has incompatible api_version\n", snd_dll->string);
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

void SND_Init(void) //mxd
{
	if (num_sndlib_infos == 0)
		SND_InitSndlibInfos();

	SndDll_Init();
	se.Init(); // Must be called after window is created. Also initializes music backend.
}

void SND_Shutdown(void) //mxd
{
	if (sound_library != NULL)
	{
		se.Shutdown(); // Also shuts down music backend.
		SndDll_FreeLibrary();
	}
}