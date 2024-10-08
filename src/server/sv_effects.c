//
// sv_effects.c
//
// Copyright 1998 Raven Software
//

#include "sv_effects.h"
#include "EffectFlags.h"

ResourceManager_t sv_FXBufMngr;
ResourceManager_t EffectsBufferMngr;

int num_persistant_effects;
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

int SV_CreatePersistantEffect(const entity_state_t* ent, const int type, int flags, vec3_t origin, char* format, ...)
{
	sizebuf_t sb;
	
	if (num_persistant_effects == MAX_PERSISTANT_EFFECTS)
	{
		Com_DPrintf("Warning : Unable to create persistant effect\n");
		return -1;
	}

	// Find a free fx slot.
	int fx_index = -1;

	for (int i = 0; i < MAX_PERSISTANT_EFFECTS; i++)
	{
		if (persistant_effects_array[i].numEffects == 0)
		{
			fx_index = i;
			break;
		}
	}

	if (fx_index == -1)
		return -1;

	// Init effect.
	PerEffectsBuffer_t* fx = &persistant_effects_array[fx_index];

	fx->freeBlock = 0;
	fx->bufSize = ENTITY_FX_BUF_SIZE;
	fx->numEffects = 1;
	fx->fx_num = type;
	fx->demo_send_mask = -1;
	fx->send_mask = 0;
	SZ_Init(&sb, fx->buf, sizeof(fx->buf));

	// Transmit effect.
	MSG_WriteShort(&sb, type | 0x8000);

	const int ent_num = (ent != NULL ? ent->number : 0);
	if ((flags & (CEF_BROADCAST | CEF_MULTICAST)) != 0 && ent_num > 255)
		flags |= CEF_ENTNUM16;

	MSG_WriteByte(&sb, flags);

	if ((flags & CEF_BROADCAST) != 0 && ent_num >= 0)
	{
		if (ent_num > 255)
			MSG_WriteShort(&sb, ent_num);
		else
			MSG_WriteByte(&sb, ent_num);
	}

	if ((flags & CEF_OWNERS_ORIGIN) == 0)
		MSG_WritePos(&sb, origin);

	if (format != NULL)
	{
		va_list argptr;
		va_start(argptr, format);
		ParseEffectToSizeBuf(&sb, format, argptr);
		va_end(argptr);
	}

	fx->bufSize = sb.cursize;
	num_persistant_effects++;

	return fx_index + 1; //mxd. 1-based, because fx type 0 is FX_REMOVE_EFFECTS?
}

qboolean SV_RemovePersistantEffect(int toRemove, int call_from)
{
	NOT_IMPLEMENTED
	return false;
}

void SV_RemoveEdictFromPersistantEffectsArray(const edict_t* ed)
{
	// Remove edict from send_mask of all persistant_effects...
	const int bit = ~EDICT_MASK(ed);
	for (int i = 0; i < MAX_PERSISTANT_EFFECTS; i++)
		persistant_effects_array[i].send_mask &= bit;
}

void SV_UpdatePersistantEffectsDemoMask(client_t* cl)
{
	NOT_IMPLEMENTED
}

void SV_ClearPersistantEffects(void)
{
	NOT_IMPLEMENTED
}

void SV_ClearPersistantEffectBuffersArray(void)
{
	SV_PrepWorldFrame();
	num_persistant_effects = 0;
	memset(persistant_effects_array, 0, sizeof(persistant_effects_array));
}