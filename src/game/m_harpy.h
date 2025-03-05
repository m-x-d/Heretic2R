//
// m_harpy.h -- mxd. Added public header.
//
// Copyright 2025 mxd
//

#pragma once

extern edict_t* give_head_to_harpy;
extern edict_t* take_head_from;

extern void SP_monster_harpy(edict_t* self);
extern void HarpyStaticsInit(void);
extern void harpy_take_head(edict_t* self, edict_t* victim, int BodyPart, int frame, int flags);