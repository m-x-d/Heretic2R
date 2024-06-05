//
// DllMain.c
//
// Copyright 1998 Raven Software
//

#include <windows.h>
#include "..\qcommon\ResourceManager.h"

ResourceManager_t res_mgr;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == 1)
		ResMngr_Con(&res_mgr, 8, 256);
	else
		ResMngr_Des(&res_mgr);

	return TRUE;
}