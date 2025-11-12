//
// p_morph.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "q_Typedef.h"

extern void MorphPlayerToChickenStart(edict_t* self);
extern void MorphPlayerToChickenEnd(edict_t* self);
extern void MorphChickenToPlayerEnd(edict_t* self);
extern void CleanUpPlayerMorph(edict_t* self);

//mxd. Required by save system...
extern void ChickenPlayerThink(edict_t* self);