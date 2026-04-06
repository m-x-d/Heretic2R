//
// g_Physics.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "q_Physics.h"

#define PF_ROTATIONAL_FRICTION	0x00000001	// Angular friction. //TODO: never set?
#define PF_RESIZE				0x00000002	// Indicates the ents bounding form should be resized.
#define PF_FORCEFUL_COLLISIONS	0x00000004	// The ent will knockback of other ents, and get knockback during collision resolution. //TODO: never set?

#define CH_ISBLOCKED			0x00000001
#define CH_ISBLOCKING			0x00000002
#define CH_BOUNCED				0x00000004

#define CH_STANDARD				(CH_ISBLOCKED | CH_ISBLOCKING)

extern void EntityPhysics(edict_t* self);
extern void CheckEntityOn(edict_t* self);
extern void PhysicsCheckWaterTransition(edict_t* self); //mxd
extern void DoImpactDamage(edict_t* self, trace_t* trace); //mxd