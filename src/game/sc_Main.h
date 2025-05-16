//
// sc_Main.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "g_local.h"

#ifndef __cplusplus
	extern void ProcessScripts();
	extern void ShutdownScripts(qboolean Complete);
	extern void SaveScripts(FILE* FH, qboolean DoGlobals);
	extern void LoadScripts(FILE* FH, qboolean DoGlobals);
#endif