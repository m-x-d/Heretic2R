//
// p_view.h
//
// Copyright 2025 mxd
//

#pragma once

#include "q_Typedef.h"

extern void ClientEndServerFrame(edict_t* player);
extern void InitPlayerinfo(const edict_t* ent);
extern void SetupPlayerinfo(edict_t* ent);
extern void WritePlayerinfo(edict_t* ent);
extern void Player_UpdateModelAttributes(edict_t* ent); //mxd. Named this way to avoid naming conflict with PlayerUpdateModelAttributes() in Player\p_main.c...