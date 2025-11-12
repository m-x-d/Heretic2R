//
// m_assassin.h -- mxd. Added public header.
//
// Copyright 2025 mxd
//

#pragma once

#include "g_Edict.h"

extern void SP_monster_assassin(edict_t* self);
extern void AssassinStaticsInit(void);

extern edict_t* AssassinDaggerReflect(edict_t* self, edict_t* other, const vec3_t vel);
extern void AssassinPrepareTeleportDestination(edict_t* self, const vec3_t spot, qboolean instant);

//mxd. Required by save system...
extern void AssassinBlocked(edict_t* self, trace_t* trace);
extern void AssassinCheckDefenseThink(edict_t* self, float enemy_dist, qboolean enemy_vis, qboolean enemy_infront);
extern void AssassinCloakFadePreThink(edict_t* self);
extern void AssassinCloakPreThink(edict_t* self);
extern void AssassinDaggerTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surface);
extern void AssassinDeCloakFadePreThink(edict_t* self);
extern void AssassinDismember(edict_t* self, int damage, HitLocation_t hl);