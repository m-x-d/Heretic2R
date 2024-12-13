//
// Debug.h -- Development aids
//
// Copyright 2024 mxd
//

#pragma once

#include "q_Typedef.h"

#define NOT_IMPLEMENTED		assert(!("Not implemented!"));

char* pv(const vec3_t v); // vtos() from g_utils.c, basically...
char* psv(const short* v);

void DBG_IDEPrint(const char* fmt, ...);
void DBG_HudPrint(int slot, const char* label, const char* fmt, ...);
void DBG_DrawMessages(void);