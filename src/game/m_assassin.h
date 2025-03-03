//
// m_assassin.h -- mxd. Added public header.
//
// Copyright 2025 mxd
//

#pragma once

extern void SP_monster_assassin(edict_t* self);

extern edict_t* AssassinArrowReflect(edict_t* self, edict_t* other, vec3_t vel);
extern void AssassinStaticsInit(void);
extern void assassinPrepareTeleportDest(edict_t* self, vec3_t spot, qboolean instant);