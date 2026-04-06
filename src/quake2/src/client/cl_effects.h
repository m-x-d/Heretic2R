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

extern int CL_CreateEffect(byte event_id, const void* owner, ushort fx_type, int flags, const vec3_t origin, const char* format, ...);
extern void CL_RemoveEffects(byte event_id, const void* owner, int fx_type);

extern void CL_InitClientEffects(const char* dll_name);
extern void CL_UnloadClientEffects(void);