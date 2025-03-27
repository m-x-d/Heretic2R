//
// g_ai_local.h -- Because I don't want to include g_local.h in module header -- mxd.
//
// Copyright 2025 mxd
//

#pragma once

#include "g_local.h"

//mxd. Local forward declarations for g_ai.c:
static qboolean AI_CheckAttack(edict_t* self);
static void AlertTimedOut(alertent_t* self);