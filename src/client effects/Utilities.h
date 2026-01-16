//
// Utilities.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "Client Entities.h"

// Update functions.
extern qboolean RemoveSelfAI(client_entity_t* self, centity_t* owner); // Set by default in ClientEntity_new.
extern qboolean KeepSelfAI(client_entity_t* self, centity_t* owner);
extern qboolean AttemptRemoveSelf(client_entity_t* self, centity_t* owner);

// AddToView functions.
extern qboolean LinkedEntityUpdatePlacement(client_entity_t* self, centity_t* owner);
extern qboolean OffsetLinkedEntityUpdatePlacement(client_entity_t* self, centity_t* owner);
extern qboolean ReferenceLinkedEntityUpdatePlacement(client_entity_t* self, centity_t* owner);

// Physics functions.
extern float GetSolidDist(const vec3_t origin, float radius, float max_dist, qboolean check_water); //mxd. +check_water arg.
extern int GetFallTime(vec3_t origin, float velocity, float acceleration, float radius, float, trace_t* trace);
extern void AdvanceParticle(struct client_particle_s* p, int ms);
extern qboolean GetWaterNormal(const vec3_t origin, float radius, float maxdist, vec3_t normal, float* dist); //mxd. Returns int in original logic.
extern qboolean Physics_MoveEnt(client_entity_t* self, float d_time, float d_time2, trace_t* trace, qboolean update_velocity); //mxd. +update_velocity arg.

extern int GetScaledCount(int count, float refdepend);
extern float GetGravity(void);

//mxd. r_entity sprite setup functions.
extern void RE_SetupRollSprite(entity_t* ent, float size, float roll);
extern void RE_SetupFlipSprite(entity_t* ent, float origin_x, float origin_y, float width, float height, qboolean flip_x, qboolean flip_y);

// Misc functions.
extern qboolean ReferencesInitialized(const centity_t* owner);
extern qboolean RefPointsValid(const centity_t* owner);

extern void InsertInCircularList(client_entity_t* self);
extern void ClearCircularList(void); //mxd
extern float GetPickupBobPhase(const vec3_t origin); //mxd
extern qboolean GetTruePlane(vec3_t origin, vec3_t direction, float direction_scale, float offset_scale); //mxd