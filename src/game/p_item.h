//
// p_item.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "p_types.h"

extern void Use_Defence(playerinfo_t *info, gitem_t *defence);
extern void DefenceThink_Powerup(edict_t *caster, char *format, ...);
extern void DefenceThink_RingOfRepulsion(edict_t *caster, char *format, ...);
extern void DefenceThink_MeteorBarrier(edict_t *caster, char *format, ...);
extern void DefenceThink_Teleport(edict_t *caster, char *format, ...);
extern void DefenceThink_Morph(edict_t *caster, char *format, ...);
extern void DefenceThink_Shield(edict_t *caster, char *format, ...);
extern void DefenceThink_Tornado(edict_t *caster, char *format, ...);