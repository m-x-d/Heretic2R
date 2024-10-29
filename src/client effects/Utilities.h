#ifndef UTILITIES_H
#define UTILITIES_H

#include "Client Entities.h"

//------------------------------------------------------------------
// Update funcs
//------------------------------------------------------------------

qboolean RemoveSelfAI(client_entity_t *this, centity_t *owner);		// set by default in ClientEntity_new
qboolean KeepSelfAI(client_entity_t *this, centity_t *owner);
qboolean AttemptRemoveSelf(client_entity_t *self, centity_t *owner);

//------------------------------------------------------------------
// AddToView funcs
//------------------------------------------------------------------

qboolean LinkedEntityUpdatePlacement(client_entity_t *current, centity_t *owner);
qboolean OffsetLinkedEntityUpdatePlacement(client_entity_t *current, centity_t *owner);
qboolean ReferenceLinkedEntityUpdatePlacement(struct client_entity_s *self, centity_t *owner);

//------------------------------------------------------------------
// Message Response Helper Funcs
//------------------------------------------------------------------

void BecomeStatic(client_entity_t *self);

//------------------------------------------------------------------
// Physics Funcs
//------------------------------------------------------------------

int GetSolidDist(vec3_t origin, vec_t radius, float maxdist, vec_t *dist);
int GetFallTime(vec3_t origin, vec_t veloc, vec_t accel, vec_t radius, float, trace_t *);
void AdvanceParticle(struct client_particle_s *p, int ms);
int GetWaterNormal(vec3_t, float, float, vec3_t, vec_t *);
qboolean Physics_MoveEnt(client_entity_t *self, float d_time, float d_time2, trace_t *trace);

int GetScaledCount(int count, float refdepend);
float GetGravity();

qboolean ReferencesInitialized(centity_t *owner);
qboolean RefPointsValid(centity_t *owner);


#endif
