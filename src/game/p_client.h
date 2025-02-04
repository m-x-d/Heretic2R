//
// p_client.h
//
// Copyright 2025 mxd
//

#pragma once

#include "g_local.h"

qboolean ClientConnect(edict_t* ent, char* userinfo);
void ClientThink(edict_t* ent, usercmd_t* cmd);
void ClientUserinfoChanged(edict_t* ent, char* userinfo);
void ClientDisconnect(edict_t* ent);
void ClientBegin(edict_t* ent);

void SP_info_player_start(edict_t* self);
void SP_info_player_deathmatch(edict_t* self);
void SP_info_player_coop(edict_t* self);
void SP_info_player_intermission(edict_t* self);