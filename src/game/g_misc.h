//
// g_misc.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "q_Typedef.h"

extern void BecomeDebris(edict_t* self);
extern void SprayDebris(const edict_t* self, const vec3_t spot, byte num_chunks, float damage);
extern void ThrowBodyPart(const edict_t* self, const vec3_t* spot, int body_part, float damage, int frame);
extern void ThrowWeapon(const edict_t* self, const vec3_t* spot, int body_part, float damage, int frame);
extern void DefaultObjectDieHandler(edict_t* self, struct G_Message_s* msg);

void BboxYawAndScale(edict_t* self); //TODO: move to g_obj.h