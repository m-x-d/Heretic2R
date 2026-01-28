//
// g_debris.h -- mxd. Part of g_misc.c in original logic.
//
// Copyright 1998 Raven Software
//

#pragma once

#include "q_Typedef.h"

extern void BecomeDebris(edict_t* self);
extern void SprayDebris(const edict_t* self, const vec3_t spot, int num_chunks);
extern void ThrowBodyPart(edict_t* self, const vec3_t* spot, int body_part, float damage, int frame);
extern void ThrowWeapon(const edict_t* self, const vec3_t* spot, int body_part, float damage, int frame);