//
// DllMain.c
//
// Copyright 1998 Raven Software
//

#include <windows.h>
#include "ResourceManager.h"

ResourceManager_t res_mgr;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	switch (fdwReason)
	{
		case DLL_PROCESS_ATTACH:
			ResMngr_Con(&res_mgr, 8, 256);
			break;

		case DLL_PROCESS_DETACH:
			ResMngr_Des(&res_mgr);
			break;

		default:
			break;
	}

	return TRUE;
}