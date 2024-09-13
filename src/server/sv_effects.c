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

void SV_RemoveEdictFromPersistantEffectsArray(const edict_t* ed)
{
	// Remove edict from send_mask of all persistant_effects...
	const int bit = ~(1 << (NUM_FOR_EDICT(ed) - 1));
	for (int i = 0; i < MAX_PERSISTANT_EFFECTS; i++)
		persistant_effects_array[i].send_mask &= bit;
}

void SV_ClearPersistantEffects(void)
{
	NOT_IMPLEMENTED
}