//
// clfx_dll.c -- Client Effects library interface.
//
// Copyright 1998 Raven Software
//

#include "clfx_dll.h"
#include "dll_io.h"

client_fx_import_t fxi;
client_fx_export_t fxe;
GetfxAPI_t GetfxAPI;
HINSTANCE clfx_library;
qboolean fxapi_initialized;

void CLFX_Init(void)
{
	fxi.FindSurface = re.FindSurface;
	fxi.GetReferencedID = re.GetReferencedID;

	fxe = GetfxAPI(fxi);
}

void CLFX_LoadDll(void)
{
	HMODULE dll_handle;
	DWORD checksum;

	const cvar_t* fx_dll = Cvar_Get("cl_fx_dll", "Client Effects", 0);
	Sys_LoadGameDll(fx_dll->string, &dll_handle, &checksum);

	GetfxAPI = (GetfxAPI_t)GetProcAddress(dll_handle, "GetfxAPI");
	if (GetfxAPI == NULL)
		Com_Error(ERR_FATAL, "GetProcAddress failed on Client Effects DLL");

	fxe = GetfxAPI(fxi);

	strcpy_s(client_string, sizeof(client_string), fxe.client_string);
	fxe.Clear = NULL;
	fxe.RegisterSounds = NULL;

	Sys_UnloadGameDll(fx_dll->string, &dll_handle);
}