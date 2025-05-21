//
// cl_effects.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "ref.h"
#include "ResourceManager.h"

extern entity_t* PlayerEntPtr;
extern float cam_transparency;
extern ResourceManager_t fx_buffer_manager;
extern qboolean precache_models;

int CL_CreateEffect(byte EventId, const void* owner, ushort type, int flags, const vec3_t origin, const char* format, ...);
void CL_RemoveEffects(byte EventId, const void* owner, int fx);

void CL_InitClientEffects(const char* dll_name);
void CL_UnloadClientEffects(void);