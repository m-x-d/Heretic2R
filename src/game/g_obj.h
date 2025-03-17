//
// g_obj.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "q_Typedef.h"

extern void BboxYawAndScale(edict_t* self);

extern void DefaultObjectDieHandler(edict_t* self, struct G_Message_s* msg);
extern void ObjectStaticsInit(void);
extern void LeverStaticsInit(void);