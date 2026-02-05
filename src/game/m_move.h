//
// m_move.h
//
// Copyright 2025 mxd
//

#pragma once

#include "q_Typedef.h"

extern qboolean SV_movestep(edict_t* ent, const vec3_t move, qboolean relink);
extern float M_ChangeYaw(edict_t* ent);
extern qboolean M_walkmove(edict_t* ent, float yaw, float dist);