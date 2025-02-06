//
// g_combat.h
//
// Copyright 2025 mxd
//

#pragma once

#include "g_local.h"

extern void AlertMonsters(edict_t* self, edict_t* enemy, float lifetime, qboolean ignore_shadows);
extern void AI_SetSightClient(void);
extern void FoundTarget(edict_t* self, qboolean setsightent);
extern qboolean FacingIdeal(edict_t* self);

extern void ai_stand(edict_t* self, float dist);
extern void ai_move(edict_t* self, float dist);
extern void ai_walk(edict_t* self, float dist);
extern void ai_turn(edict_t* self, float dist);
extern void ai_run(edict_t* self, float dist); //mxd. Actually defined in mg_ai.c!
extern void ai_charge(edict_t* self, float dist);
extern void ai_eat(edict_t* self, float dist);
extern void ai_flee(edict_t* self, float dist);