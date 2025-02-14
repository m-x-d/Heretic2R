//
// g_cmds.h
//
// Copyright 2025 mxd
//

#pragma once

#include "g_local.h"

extern qboolean OnSameTeam(const edict_t* ent1, const edict_t* ent2);
extern void ValidateSelectedItem(const edict_t* ent);
extern void Cmd_Score_f(edict_t* ent);
extern void Cmd_Use_f(edict_t* ent, char* name);
extern void Cmd_WeapPrev_f(const edict_t* ent);
extern void ClientCommand(edict_t* ent);