//
// sv_effects.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "server.h"
#include "ResourceManager.h"

extern ResourceManager_t sv_FXBufMngr;
extern ResourceManager_t EffectsBufferMngr;

extern int per_effects_buffers_size; //TODO: better name?
extern PerEffectsBuffer_t persistant_effects_array[MAX_PERSISTANT_EFFECTS];

extern int effects_buffer_index;
extern int effects_buffer_offset;

void SV_CreateEffect(entity_state_t* ent, int type, int flags, vec3_t origin, char* format, ...);
void SV_RemoveEffects(entity_state_t* ent, int type);
void SV_CreateEffectEvent(byte EventId, entity_state_t* ent, int type, int flags, vec3_t origin, char* format, ...);
void SV_RemoveEffectsEvent(byte EventId, entity_state_t* ent, int type);
int SV_CreatePersistantEffect(entity_state_t* ent, int type, int flags, vec3_t origin, char* format, ...);
qboolean SV_RemovePersistantEffect(int toRemove, int call_from);
void SV_RemoveEdictFromPersistantEffectsArray(const edict_t* ed);
void SV_UpdatePersistantEffectsDemoMask(client_t* cl);
void SV_ClearPersistantEffects(void);
