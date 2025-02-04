//
// p_client.h
//
// Copyright 2025 mxd
//

#pragma once

#include "g_local.h"

extern qboolean ClientConnect(edict_t* ent, char* userinfo);
extern void ClientThink(edict_t* ent, usercmd_t* cmd);
extern void ClientUserinfoChanged(edict_t* ent, char* userinfo);
extern void ClientDisconnect(edict_t* ent);
extern void ClientBegin(edict_t* ent);