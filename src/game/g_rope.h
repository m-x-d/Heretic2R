//
// g_rope.h
//
// Copyright 2025 mxd
//

#pragma once

#include "g_Edict.h"

#define TUTORIAL_CHICKEN_CLASSNAME	"NATE" //mxd

extern void SP_obj_rope(edict_t* self);

//mxd. Required by save system...
extern void ObjRopeThink(edict_t* self);
extern void ObjRopeTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf);
extern void TutorialChickenPain(edict_t* self, edict_t* other, float kick, int damage);
extern void TutorialChickenRopeEndThink(edict_t* self);
extern void TutorialChickenThink(edict_t* self);
extern void TutorialChickenUse(edict_t* self, edict_t* other, edict_t* activator);