//
// sv_effects.c
//
// Copyright 1998 Raven Software
//

#include "sv_effects.h"

ResourceManager_t sv_FXBufMngr;
ResourceManager_t EffectsBufferMngr;

int per_effects_buffers_size;
PerEffectsBuffer_t persistant_effects_array[MAX_PERSISTANT_EFFECTS];

int effects_buffer_index;
int effects_buffer_offset;

void SV_CreateEffect(entity_state_t* ent, int type, int flags, vec3_t origin, char* format, ...)
{
	NOT_IMPLEMENTED
}

void SV_RemoveEffects(entity_state_t* ent, int type)
{
	NOT_IMPLEMENTED
}

void SV_CreateEffectEvent(byte EventId, entity_state_t* ent, int type, int flags, vec3_t origin, char* format, ...)
{
	NOT_IMPLEMENTED
}

void SV_RemoveEffectsEvent(byte EventId, entity_state_t* ent, int type)
{
	NOT_IMPLEMENTED
}

int SV_CreatePersistantEffect(entity_state_t* ent, int type, int flags, vec3_t origin, char* format, ...)
{
	NOT_IMPLEMENTED
	return 0;
}

qboolean SV_RemovePersistantEffect(int toRemove, int call_from)
{
	NOT_IMPLEMENTED
	return false;
}

void SV_RemoveEdictFromEffectsArray(edict_t* ed)
{
	NOT_IMPLEMENTED
}

void SV_ClearPersistantEffects(void)
{
	NOT_IMPLEMENTED
}