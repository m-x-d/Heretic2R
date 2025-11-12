//
// g_misc.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "q_shared.h"

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
extern void SP_misc_flag(edict_t* self);

extern void SP_sound_ambient_silverspring(edict_t* self);
extern void SP_sound_ambient_swampcanyon(edict_t* self);
extern void SP_sound_ambient_andoria(edict_t* self);
extern void SP_sound_ambient_hive(edict_t* self);
extern void SP_sound_ambient_mine(edict_t* self);
extern void SP_sound_ambient_cloudfortress(edict_t* self);

extern void MiscTeleporterStaticsInit(void);

//mxd. Required by save system...
extern void FuncAreaportalUse(edict_t* ent, edict_t* other, edict_t* activator);
extern void FuncObjectTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf);
extern void FuncObjectRelease(edict_t* self);
extern void FuncObjectUse(edict_t* self, edict_t* other, edict_t* activator);
extern void FuncWallUse(edict_t* self, edict_t* other, edict_t* activator);
extern void ItemSpitterUse(edict_t* self, edict_t* owner, edict_t* attacker);
extern void MiscFireSparkerThink(edict_t* self);
extern void MiscFireSparkerRemove(edict_t* self, edict_t* other, edict_t* activator);
extern void MiscFireSparkerUse(edict_t* self, edict_t* other, edict_t* activator);
extern void MiscFlagThink(edict_t* self);
extern void MiscMagicPortalTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf);
extern void MiscMagicPortalUse(edict_t* self, edict_t* other, edict_t* activator);
extern void MiscRemoteCameraThink(edict_t* self);
extern void MiscRemoteCameraUse(edict_t* self, edict_t* other, edict_t* activator);
extern void MiscUpdateSpawnerTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf);
extern void PathCornerTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf);
extern void PointCombatTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf);
extern void SoundAmbientThink(edict_t* self);
extern void SoundAmbientUse(edict_t* self, edict_t* other, edict_t* activator);