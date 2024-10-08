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

void CL_InitClientEffects(const char* dll_name);
void CL_UnloadClientEffects(void);