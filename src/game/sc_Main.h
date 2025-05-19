//
// sc_Main.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "g_local.h"

#ifndef __cplusplus
	extern void ProcessScripts();
	extern void ShutdownScripts(qboolean complete);
	extern void SaveScripts(FILE* f, qboolean do_globals);
	extern void LoadScripts(FILE* f, qboolean do_globals);
#endif