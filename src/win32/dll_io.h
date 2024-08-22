//
// dll_io.h -- Windows-specific dll loading/unloading logic.
//
// Copyright 1998 Raven Software
//

#pragma once

#include <windows.h>

extern void Sys_LoadDll(char* name, HINSTANCE* hinst, DWORD* chkSum);
extern void Sys_UnloadDll(char* name, HINSTANCE* hinst);