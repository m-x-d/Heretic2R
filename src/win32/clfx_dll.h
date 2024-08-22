//
// clfx_dll.h -- Client Effects library interface.
//
// Copyright 1998 Raven Software
//

#pragma once

#include <windows.h>
#include "client.h"

extern client_fx_import_t fxi;
extern HINSTANCE clfx_library;

client_fx_export_t (*GetfxAPI)(client_fx_import_t import);

extern void CLFX_Init(void);
extern void CLFX_LoadDll(void);