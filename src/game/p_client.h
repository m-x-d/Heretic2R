//
// p_client.h
//
// Copyright 2025 mxd
//

#pragma once

#include "g_HitLocation.h"
#include "q_shared.h"
#include "q_Typedef.h"

//NOTE: The precious, delicate player bbox coords!
extern const vec3_t player_mins;
extern const vec3_t player_maxs;

extern qboolean ClientConnect(edict_t* ent, char* userinfo);
extern void ClientRespawn(edict_t* self);
extern void ClientThink(edict_t* ent, usercmd_t* ucmd);
extern void ClientUserinfoChanged(edict_t* ent, char* userinfo);
extern void ClientDisconnect(edict_t* ent);
extern void ClientBegin(edict_t* ent);
extern void ClientBeginServerFrame(edict_t* ent);

extern void PlayerPain(edict_t* self, edict_t* other, float kick, int damage);
extern void PlayerDie(edict_t* self, edict_t* inflictor, edict_t* attacker, int damage, const vec3_t point);
extern void PlayerDismember(edict_t* self, edict_t* other, int damage, HitLocation_t hl);
extern void PlayerDecapitate(edict_t* self, edict_t* other); //mxd
extern void ResetPlayerBaseNodes(edict_t* ent);
extern void PlayerRepairSkin(edict_t* self);
extern void SelectSpawnPoint(const edict_t* ent, vec3_t origin, vec3_t angles); //mxd
extern void SpawnInitialPlayerEffects(edict_t* ent); //mxd
extern void InitBodyQue(void);
extern void SaveClientData(void);

extern void SP_info_player_start(edict_t* self);
extern void SP_info_player_deathmatch(edict_t* self);
extern void SP_info_player_coop(edict_t* self);
extern void SP_info_player_intermission(edict_t* self);

//mxd. Required by save system...
extern void BleederThink(edict_t* self);
extern void PlayerBodyDie(edict_t* self, edict_t* inflictor, edict_t* attacker, int damage, const vec3_t point);

// Forward declarations local to p_client.c //TODO: move to separate header?
static void PutClientInServer(edict_t* ent);