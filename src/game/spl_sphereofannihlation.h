//
// spl_sphereofannihlation.h
//
// Copyright 2025 mxd
//

#pragma once

#include "q_shared.h"

extern void SpellCastSphereOfAnnihilation(edict_t* caster, const vec3_t start_pos, const vec3_t aim_angles, const vec3_t aim_dir, qboolean* release_flags_ptr);
extern edict_t* SphereReflect(edict_t* self, edict_t* other, vec3_t vel);

// Local forward declarations for spl_sphereofannihlation.c:
static void SphereWatcherTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surface);