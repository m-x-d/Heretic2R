//
// p_animactor.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "p_types.h"

extern PLAYER_API void TurnOffPlayerEffects(playerinfo_t *info);
extern PLAYER_API void AnimUpdateFrame(playerinfo_t *info);
extern PLAYER_API void PlayerFallingDamage(playerinfo_t *info);
extern PLAYER_API void PlayerIntLand(playerinfo_t* info, float landspeed); //mxd. Defined in p_ctrl.h in original version.