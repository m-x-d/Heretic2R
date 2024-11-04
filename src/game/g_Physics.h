#ifndef G_PHYSICS_H
#define G_PHYSICS_H

#include "g_BoundingForm.h"
#include "q_Physics.h"

#define	FRICTION_STOPSPEED			100
#define FRICTION_SURFACE			6
#define FRICTION_WATER				1

#define MONSTER_STEPHEIGHT			30
#define MONSTER_DROPHEIGHT			100

#define TRACE_FRACTION_FAIL_FOR_WHOLE_MOVE_CHECK 0.8

#define PF_ROTATIONAL_FRICTION		0x00000001	// angular friction
#define PF_RESIZE					0x00000002	// indicates the ents bounding form should be resized
#define PF_FORCEFUL_COLLISIONS		0x00000004	// the ent will knockback of other ents, and get knockback during 
												// collision resolution

#define CH_ISBLOCKED				0x00000001
#define CH_ISBLOCKING				0x00000002
#define CH_BOUNCED					0x00000004

#define CH_STANDARD					(CH_ISBLOCKED|CH_ISBLOCKING)

extern void EntityPhysics(edict_t *ent);
extern void CheckEntityOn(edict_t *ent);
extern void ApplyGravity(edict_t *self, vec3_t move);
extern void HandleCollision(edict_t *self, trace_t *trace, vec3_t move, int forceful, int flags);
extern void HandleForcefulCollision(edict_t *forcer, edict_t *forcee, vec3_t move, int forceful);
extern void MoveEntity_Bounce(edict_t *self, FormMove_t *formMove);
extern void MoveEntity_Slide(edict_t *ent);
extern void ActivateTriggers(edict_t *ent);
extern qboolean ApplySurfaceFriction(edict_t *self);
extern void ApplyRotationalFriction(edict_t *ent);
extern void SetGroundEntFromTrace(edict_t *self, trace_t *trace);
extern void SetBlockingEntFromTrace(edict_t *self, trace_t *trace);
extern void CheckInWater(FormMove_t *formMove);
extern qboolean DiscreteMove_Step(edict_t *self, vec3_t move, FormMove_t *formMove);
extern void CreateMove_Step(edict_t *self, vec3_t move, float dist);

qboolean CheckAnimMove(edict_t *self, vec3_t origin, vec3_t move, float *dist, float attemptDist, int recursionLevel);

#endif