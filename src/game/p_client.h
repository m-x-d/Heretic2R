//
// p_client.h
//
// Copyright 2025 mxd
//

#pragma once

#include "q_shared.h"
#include "q_Typedef.h"

extern qboolean ClientConnect(edict_t* ent, char* userinfo);
extern void ClientThink(edict_t* ent, usercmd_t* cmd);
extern void ClientUserinfoChanged(edict_t* ent, char* userinfo);
extern void ClientDisconnect(edict_t* ent);
extern void ClientBegin(edict_t* ent);

extern void SP_info_player_start(edict_t* self);
extern void SP_info_player_deathmatch(edict_t* self);
extern void SP_info_player_coop(edict_t* self);
extern void SP_info_player_intermission(edict_t* self);