//
// p_morph.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "q_Typedef.h"

extern void MorphPlayerToChicken(edict_t* self);
extern void PerformPlayerMorph(edict_t* self);
extern void CleanUpPlayerMorph(edict_t* self);
extern void ResetPlayerMorph(edict_t* self);