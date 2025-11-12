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

	//mxd. Required by save system...
	extern void ScriptUse(edict_t* ent, edict_t* other, edict_t* activator);
#endif