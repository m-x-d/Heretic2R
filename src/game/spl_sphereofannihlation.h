//
// spl_sphereofannihlation.h
//
// Copyright 2025 mxd
//

#pragma once

#include "q_Typedef.h"

extern void SpellCastSphereOfAnnihilation(edict_t* Caster, vec3_t StartPos, vec3_t AimAngles, vec3_t AimDir, float Value, qboolean* ReleaseFlagsPtr);
extern edict_t* SphereReflect(edict_t* self, edict_t* other, vec3_t vel);