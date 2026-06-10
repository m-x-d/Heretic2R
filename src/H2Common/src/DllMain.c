//
// DllMain.c
//
// Copyright 1998 Raven Software
//

#include "ResourceManager.h"
#include "SinglyLinkedList.h"

extern ResourceManager_t sllist_nodes_mgr;

#ifdef _WIN32

#include <windows.h>

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	switch (fdwReason)
	{
		case DLL_PROCESS_ATTACH:
			ResMngr_Con(&sllist_nodes_mgr, SLL_NODE_SIZE, SLL_NODE_BLOCK_SIZE);
			break;

		case DLL_PROCESS_DETACH:
			ResMngr_Des(&sllist_nodes_mgr);
			break;

		default:
			break;
	}

	return TRUE;
}

#else

// On Linux the .so is initialized/finalized via ELF constructor/destructor.
__attribute__((constructor)) static void H2Common_Attach(void)
{
	ResMngr_Con(&sllist_nodes_mgr, SLL_NODE_SIZE, SLL_NODE_BLOCK_SIZE);
}

__attribute__((destructor)) static void H2Common_Detach(void)
{
	ResMngr_Des(&sllist_nodes_mgr);
}

#endif