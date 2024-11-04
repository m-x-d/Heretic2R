#ifndef _M_SPREADERMIST_H_
#define _M_SPREADERMIST_H_

#include "q_typedef.h"
#include "q_shared.h"
#include "g_local.h"
#include "g_volume_effect.h"

void spreader_grenade_die(edict_t *self);
static void spreader_grenade_explode (edict_t *self);
static void spreader_grenade_think(edict_t *self);
static void spreader_grenade_touch (edict_t *ent, edict_t *other, cplane_t *plane, 
							csurface_t *surf);

void spreader_mist_fast(edict_t *self, float x, float y, float z);
void spreader_mist(edict_t *self, float x, float y, float z);
void spreader_toss_grenade(edict_t *self);
void mist_damage_think(edict_t *victim, volume_effect_t *vol);

#endif 
