//
// g_Decals.h -- Named 'decals.h' in original logic --mxd.
//
// Copyright 1998 Raven Software
//

#pragma once

#include "q_Shared.h"

extern qboolean IsDecalApplicable(const edict_t* target, const vec3_t origin, const csurface_t* surface, const cplane_t* plane, vec3_t plane_dir);