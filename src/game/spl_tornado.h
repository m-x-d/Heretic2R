//
// spl_tornado.h
//
// Copyright 2025 mxd
//

#pragma once

#include "g_Edict.h"

extern void SpellCastDropTornado(edict_t* caster, const vec3_t start_pos);

//mxd. Required by save system...
extern void TornadoCreateThink(edict_t* tornado);
extern void TornadoThink(edict_t* self);