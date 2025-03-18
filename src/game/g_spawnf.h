//
// g_spawnf.h
//
// Copyright 2025 mxd
//

#pragma once

#include "q_Typedef.h"

extern void SpawnEntities(const char* map_name, char* entities, const char* spawn_point, qboolean loadgame);
extern void ConstructEntities(void);
extern void CheckCoopTimeout(qboolean been_here_before);

extern void SP_worldspawn(edict_t* ent);