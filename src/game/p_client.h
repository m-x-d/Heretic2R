//
// p_client.h
//
// Copyright 2025 mxd
//

#pragma once

#include "q_shared.h"
#include "q_Typedef.h"

//NOTE: The precious, delicate player bbox coords!
extern const vec3_t player_mins;
extern const vec3_t player_maxs;

extern qboolean ClientConnect(edict_t* ent, char* userinfo);
extern void ClientThink(edict_t* ent, usercmd_t* cmd);
extern void ClientUserinfoChanged(edict_t* ent, char* userinfo);
extern void ClientDisconnect(edict_t* ent);
extern void ClientBegin(edict_t* ent);
extern void ClientUpdateModelAttributes(edict_t* ent); //mxd

extern int player_pain(edict_t* self, edict_t* other, float kick, int damage);
extern int player_die(edict_t* self, edict_t* inflictor, edict_t* attacker, int damage, vec3_t point);
extern void player_dismember(edict_t* self, edict_t* other, int damage, int HitLocation);
extern void ResetPlayerBaseNodes(edict_t* ent);
extern void player_repair_skin(edict_t* self);

extern void SP_info_player_start(edict_t* self);
extern void SP_info_player_deathmatch(edict_t* self);
extern void SP_info_player_coop(edict_t* self);
extern void SP_info_player_intermission(edict_t* self);