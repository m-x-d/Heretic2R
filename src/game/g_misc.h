//
// g_misc.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "q_Typedef.h"

extern void DefaultObjectDieHandler(edict_t* self, struct G_Message_s* msg);

void BboxYawAndScale(edict_t* self); //TODO: move to g_obj.h