//
// m_imp.h -- mxd. Added public header.
//
// Copyright 2025 mxd
//

#pragma once

void SP_monster_imp(edict_t* self);
edict_t* ImpFireballReflect(edict_t* self, edict_t* other, vec3_t vel);
void ImpStaticsInit(void);