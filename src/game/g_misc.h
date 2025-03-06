//
// g_misc.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "q_Typedef.h"

extern void SP_func_areaportal(edict_t* ent);
extern void SP_path_corner(edict_t* self);
extern void SP_point_combat(edict_t* self);
extern void SP_info_null(edict_t* self);
extern void SP_info_notnull(edict_t* self);
extern void SP_func_wall(edict_t* self);
extern void SP_func_object(edict_t* self);
extern void SP_item_spitter(edict_t* self);
extern void SP_misc_update_spawner(edict_t* ent);
extern void SP_misc_teleporter(edict_t* ent);
extern void SP_misc_teleporter_dest(edict_t* ent);
extern void SP_misc_magic_portal(edict_t* self);
extern void SP_misc_remote_camera(edict_t* self);
extern void SP_misc_fire_sparker(edict_t* self);

extern void SP_sound_ambient_silverspring(edict_t* self);
extern void SP_sound_ambient_swampcanyon(edict_t* self);
extern void SP_sound_ambient_andoria(edict_t* self);
extern void SP_sound_ambient_hive(edict_t* self);
extern void SP_sound_ambient_mine(edict_t* self);
extern void SP_sound_ambient_cloudfortress(edict_t* self);

extern void TeleporterStaticsInit(void);

extern void DefaultObjectDieHandler(edict_t* self, struct G_Message_s* msg);

void BboxYawAndScale(edict_t* self); //TODO: move to g_obj.h