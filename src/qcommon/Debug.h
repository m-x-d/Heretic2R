//
// Debug.h -- Development aids
//
// Copyright 2024 mxd
//

#pragma once

#include <assert.h>
#include "Heretic2.h"
#include "q_Typedef.h"

#define NOT_IMPLEMENTED		assert(!("Not implemented!"));

GAME_DECLSPEC char* pv(const vec3_t v); // vtos() from g_utils.c, basically...
GAME_DECLSPEC char* psv(const short* v);

GAME_DECLSPEC void DBG_IDEPrint(const char* fmt, ...);
GAME_DECLSPEC void DBG_HudPrint(int slot, const char* label, const char* fmt, ...);
void DBG_DrawMessages(void);