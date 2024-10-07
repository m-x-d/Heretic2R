//
// clfx_dll.h -- Client Effects library interface.
//
// Copyright 1998 Raven Software
//

#pragma once

#include <windows.h>
#include "client.h"

extern client_fx_import_t fxi;
extern GetfxAPI_t GetfxAPI;
extern HINSTANCE clfx_library;
extern qboolean fxapi_initialized;

extern void CLFX_Init(void);
extern void CLFX_LoadDll(void);