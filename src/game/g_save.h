//
// g_save.h
//
// Copyright 2025 mxd
//

#pragma once

#include "g_local.h"

extern void WriteGame(char* filename, qboolean autosave);
extern void ReadGame(char* filename);
extern void WriteLevel(char* filename);
extern void ReadLevel(char* filename);
extern void InitGame(void);