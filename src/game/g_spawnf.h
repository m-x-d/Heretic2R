//
// g_spawnf.h
//
// Copyright 2025 mxd
//

#pragma once

#include "g_local.h"

extern void SpawnEntities(char* mapname, char* entities, char* spawnpoint, qboolean loadgame);
extern void ConstructEntities(void);
extern void CheckCoopTimeout(qboolean BeenHereBefore);