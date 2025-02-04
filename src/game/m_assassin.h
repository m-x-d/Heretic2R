//
// m_assassin.h -- mxd. Added public header.
//
// Copyright 2025 mxd
//

#pragma once

void SP_monster_assassin(edict_t* self);

edict_t* AssassinArrowReflect(edict_t* self, edict_t* other, vec3_t vel);
void AssassinStaticsInit(void);