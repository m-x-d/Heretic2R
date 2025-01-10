//
// Utilities.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "Client Entities.h"

// Update functions.
qboolean RemoveSelfAI(client_entity_t* this, centity_t* owner); // Set by default in ClientEntity_new.
qboolean KeepSelfAI(client_entity_t* this, centity_t* owner);
qboolean AttemptRemoveSelf(client_entity_t* self, centity_t* owner);

// AddToView functions.
qboolean LinkedEntityUpdatePlacement(client_entity_t* current, const centity_t* owner);
qboolean OffsetLinkedEntityUpdatePlacement(client_entity_t* current, const centity_t* owner);
qboolean ReferenceLinkedEntityUpdatePlacement(struct client_entity_s* self, const centity_t* owner);

// Message response helper functions.
void BecomeStatic(client_entity_t* self);

// Physics functions.
int GetSolidDist(vec3_t origin, float radius, float maxdist, float* dist);
int GetFallTime(vec3_t origin, float velocity, float acceleration, float radius, float, trace_t* trace);
void AdvanceParticle(struct client_particle_s* p, int ms);
int GetWaterNormal(vec3_t origin, float radius, float maxdist, vec3_t normal, float* dist);
qboolean Physics_MoveEnt(client_entity_t* self, float d_time, float d_time2, trace_t* trace);

int GetScaledCount(int count, float refdepend);
float GetGravity(void);

qboolean ReferencesInitialized(const centity_t* owner);
qboolean RefPointsValid(const centity_t* owner);