//
// p_hud.h
//
// Copyright 2025 mxd
//

#pragma once

#include "q_Typedef.h"

extern char* dm_statusbar;
extern char* single_statusbar;

extern void BeginIntermission(const edict_t* target_changelevel);
extern void Cmd_Score_f(const edict_t* ent);