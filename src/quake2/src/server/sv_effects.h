//
// sv_effects.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "server.h"
#include "ResourceManager.h"

extern qboolean send_fx_framenum;

extern ResourceManager_t sv_FXBufMngr;
extern ResourceManager_t EffectsBufferMngr;

extern int num_persistant_effects;
extern PerEffectsBuffer_t persistant_effects[MAX_PERSISTANT_EFFECTS];

extern int effects_buffer_index;
extern int clfx_buffer_offset;

extern void SV_CreateEffect(entity_state_t* ent, int fx_type, int flags, const vec3_t origin, const char* format, ...);
extern void SV_RemoveEffects(entity_state_t* ent, int fx_type);
extern void SV_CreateEffectEvent(byte event_id, entity_state_t* ent, int fx_type, int flags, const vec3_t origin, const char* format, ...);
extern void SV_RemoveEffectsEvent(byte event_id, entity_state_t* ent, int fx_type);
extern int SV_CreatePersistantEffect(const entity_state_t* ent, int fx_type, int flags, const vec3_t origin, const char* format, ...);
extern qboolean SV_RemovePersistantEffect(int fx_index, int call_from);
extern void SV_RemoveEdictFromPersistantEffectsArray(const edict_t* ed);
extern void SV_RemoveDemoEdictFromPersistantEffectsArray(const client_t* cl);
extern void SV_ClearPersistantEffects(void);
extern void SV_ClearPersistantEffectBuffersArray(void);
extern void SV_SendClientEffects(client_t* cl);