//
// sv_effects.c
//
// Copyright 1998 Raven Software
//

#include "sv_effects.h"
#include "EffectFlags.h"
#include "FX.h"
#include "Vector.h"

qboolean send_fx_framenum;

ResourceManager_t sv_FXBufMngr;
ResourceManager_t EffectsBufferMngr;

int num_persistant_effects;
PerEffectsBuffer_t persistant_effects[MAX_PERSISTANT_EFFECTS];

int clfx_buffer_offset;
static byte clfx_buffer[4096];

#define MAX_EFFECT_BUFFERS	256 //mxd

typedef struct EffectsBufferInfo_s //mxd
{
	EffectsBuffer_t buffer;
	vec3_t position;
	qboolean is_broadcast;
} EffectsBufferInfo_t;

int effects_buffer_index;
static EffectsBufferInfo_t effects_buffers[MAX_EFFECT_BUFFERS];

void SV_CreateEffect(entity_state_t* ent, const int fx_type, int flags, const vec3_t origin, const char* format, ...)
{
	EffectsBuffer_t* clfx;
	int ent_num;
	int length;
	sizebuf_t sb;

	flags &= 0xff; // Strip Client Effects-specific flags?

	const qboolean broadcast = (flags & (CEF_BROADCAST | CEF_MULTICAST));

	if (broadcast)
	{
		ent_num = (ent != NULL ? ent->number : 0);
		ent = NULL; // ???
	}
	else
	{
		ent_num = -1;
	}

	if (ent != NULL && !broadcast)
	{
		clfx = &ent->clientEffects;

		if (clfx->buf == NULL)
		{
			clfx->buf = (byte*)ResMngr_AllocateResource(&sv_FXBufMngr, ENTITY_FX_BUF_SIZE);
			clfx->bufSize = ENTITY_FX_BUF_SIZE;
			clfx->numEffects = 0;
		}

		length = clfx->bufSize - clfx->freeBlock;
	}
	else
	{
		if (effects_buffer_index == MAX_EFFECT_BUFFERS) //mxd. Added sanity check.
		{
			Com_DPrintf("Warning: unable to create frame effect\n");
			return;
		}

		clfx = &effects_buffers[effects_buffer_index].buffer;
		clfx->buf = clfx_buffer;
		clfx->freeBlock = clfx_buffer_offset;

		length = (int)sizeof(clfx_buffer) - clfx_buffer_offset;
	}

	SZ_Init(&sb, &clfx->buf[clfx->freeBlock], length);

	if (flags != 0)
	{
		MSG_WriteShort(&sb, fx_type | EFFECT_FLAGS);

		if (broadcast && ent_num > 255)
			flags |= CEF_ENTNUM16;

		MSG_WriteByte(&sb, flags);

		if (broadcast && ent_num > -1)
		{
			if (ent_num > 255)
				MSG_WriteShort(&sb, ent_num);
			else
				MSG_WriteByte(&sb, ent_num);
		}
	}
	else
	{
		MSG_WriteShort(&sb, fx_type);
	}

	effects_buffers[effects_buffer_index].is_broadcast = broadcast;

	if (!(flags & CEF_OWNERS_ORIGIN))
	{
		assert(origin != NULL); //mxd
		MSG_WritePos(&sb, origin);
	}

	if (format != NULL)
	{
		va_list argptr;

		va_start(argptr, format);
		ParseEffectToSizeBuf(&sb, format, argptr);
		va_end(argptr);
	}

	if (ent != NULL)
	{
		clfx->freeBlock += sb.cursize;
		clfx->numEffects++; // Lasts single frame, cleared in SV_PrepWorldFrame() --mxd.

		assert(clfx->numEffects < FX_BUF_MAX_EFFECTS); //mxd
	}
	else
	{
		clfx->numEffects = 1;
		clfx->bufSize = sb.cursize;

		if (!(flags & CEF_BROADCAST))
			VectorCopy(origin, effects_buffers[effects_buffer_index].position);

		clfx_buffer_offset += sb.cursize;
		effects_buffer_index++;
	}
}

void SV_RemoveEffects(entity_state_t* ent, const int fx_type)
{
	SV_CreateEffect(ent, FX_REMOVE_EFFECTS, CEF_OWNERS_ORIGIN | CEF_BROADCAST, NULL, "s", fx_type);
}

//mxd. Parsed by ParseEffects() in ClientEffects/Main.c
void SV_CreateEffectEvent(const byte event_id, entity_state_t* ent, const int fx_type, int flags, const vec3_t origin, const char* format, ...)
{
	EffectsBuffer_t* clfx;
	int ent_num;
	int length;
	sizebuf_t sb;

	const qboolean broadcast = (flags & (CEF_BROADCAST | CEF_MULTICAST));

	if (broadcast)
		ent_num = (ent != NULL ? ent->number : 0);
	else
		ent_num = -1;

	if (ent != NULL && !broadcast)
	{
		clfx = &ent->clientEffects;

		if (clfx->buf == NULL)
		{
			clfx->buf = (byte*)ResMngr_AllocateResource(&sv_FXBufMngr, ENTITY_FX_BUF_SIZE);
			clfx->bufSize = ENTITY_FX_BUF_SIZE;
			clfx->numEffects = 0;
		}

		length = clfx->bufSize - clfx->freeBlock;
	}
	else
	{
		if (effects_buffer_index == MAX_EFFECT_BUFFERS) //mxd. Added sanity check.
		{
			Com_DPrintf("Warning: unable to create frame effect event\n");
			return;
		}

		clfx = &effects_buffers[effects_buffer_index].buffer;
		clfx->buf = clfx_buffer;
		clfx->freeBlock = clfx_buffer_offset;

		length = (int)sizeof(clfx_buffer) - clfx_buffer_offset;
	}

	SZ_Init(&sb, &clfx->buf[clfx->freeBlock], length);

	if (flags != 0)
	{
		MSG_WriteShort(&sb, fx_type | EFFECT_PRED_INFO | EFFECT_FLAGS);
		MSG_WriteByte(&sb, event_id);

		if (broadcast && ent_num > 255)
			flags |= CEF_ENTNUM16;

		MSG_WriteByte(&sb, flags);

		if (broadcast && ent_num > -1)
		{
			if (ent_num > 255)
				MSG_WriteShort(&sb, ent_num);
			else
				MSG_WriteByte(&sb, ent_num);
		}
	}
	else
	{
		MSG_WriteShort(&sb, fx_type);
	}

	effects_buffers[effects_buffer_index].is_broadcast = broadcast;

	if (!(flags & CEF_OWNERS_ORIGIN))
	{
		assert(origin != NULL); //mxd
		MSG_WritePos(&sb, origin);
	}

	if (format != NULL)
	{
		va_list argptr;

		va_start(argptr, format);
		ParseEffectToSizeBuf(&sb, format, argptr);
		va_end(argptr);
	}

	if (ent != NULL)
	{
		clfx->freeBlock += sb.cursize;
		clfx->numEffects++;

		assert(clfx->numEffects < FX_BUF_MAX_EFFECTS); //mxd
	}
	else
	{
		clfx->numEffects = 1;
		clfx->bufSize = sb.cursize;

		if (!(flags & CEF_BROADCAST))
			VectorCopy(origin, effects_buffers[effects_buffer_index].position);

		clfx_buffer_offset += sb.cursize;
		effects_buffer_index++;
	}
}

void SV_RemoveEffectsEvent(const byte event_id, entity_state_t* ent, const int fx_type)
{
	SV_CreateEffectEvent(event_id, ent, FX_REMOVE_EFFECTS, CEF_OWNERS_ORIGIN | CEF_BROADCAST, NULL, "s", fx_type);
}

int SV_CreatePersistantEffect(const entity_state_t* ent, const int fx_type, int flags, const vec3_t origin, const char* format, ...)
{
	sizebuf_t sb;
	
	if (num_persistant_effects == MAX_PERSISTANT_EFFECTS)
	{
		Com_DPrintf("Warning : Unable to create persistant effect\n");
		return -1;
	}

	// Find a free fx slot.
	int fx_index = -1;

	//mxd. For FX_PLAYER_PERSISTANT specifically, first try to locate existing effect, which may've been restored when loading the game (to avoid creating duplicate effects when creating it unconditionally in SpawnInitialPlayerEffects()).
	if (fx_type == FX_PLAYER_PERSISTANT)
	{
		PerEffectsBuffer_t* pfx = &persistant_effects[0];
		for (int i = 0; i < MAX_PERSISTANT_EFFECTS; i++, pfx++)
		{
			if (pfx->fx_num == fx_type && pfx->numEffects == 0)
			{
				fx_index = i;
				break;
			}
		}
	}

	if (fx_index == -1)
	{
		PerEffectsBuffer_t* pfx = &persistant_effects[0];
		for (int i = 0; i < MAX_PERSISTANT_EFFECTS; i++, pfx++)
		{
			if (pfx->numEffects == 0)
			{
				fx_index = i;
				break;
			}
		}
	}

	if (fx_index == -1)
		return -1;

	// Init effect.
	PerEffectsBuffer_t* pfx = &persistant_effects[fx_index];

	pfx->freeBlock = 0;
	pfx->bufSize = ENTITY_FX_BUF_SIZE;
	pfx->numEffects = 1;
	pfx->fx_num = fx_type;
	pfx->demo_send_mask = -1;
	pfx->send_mask = 0;
	SZ_Init(&sb, pfx->buf, sizeof(pfx->buf));

	// Transmit effect.
	MSG_WriteShort(&sb, fx_type | EFFECT_FLAGS);

	const int ent_num = (ent != NULL ? ent->number : 0);
	if ((flags & (CEF_BROADCAST | CEF_MULTICAST)) && ent_num > 255)
		flags |= CEF_ENTNUM16;

	MSG_WriteByte(&sb, flags);

	if ((flags & CEF_BROADCAST) && ent_num >= 0)
	{
		if (ent_num > 255)
			MSG_WriteShort(&sb, ent_num);
		else
			MSG_WriteByte(&sb, ent_num);
	}

	if (!(flags & CEF_OWNERS_ORIGIN))
		MSG_WritePos(&sb, origin);

	if (format != NULL)
	{
		va_list argptr;
		va_start(argptr, format);
		ParseEffectToSizeBuf(&sb, format, argptr);
		va_end(argptr);
	}

	pfx->bufSize = sb.cursize;
	num_persistant_effects++;

	return fx_index + 1; // 1-based, because 0 means "no effect" --mxd.
}

qboolean SV_RemovePersistantEffect(int fx_index, const int call_from)
{
	static const char* fx_types[] =
	{
		"REMOVE SHIELD",
		"REMOVE TELEPORT PAD",
		"REMOVE METEORS",
		"REMOVE LEADER",
		"REMOVE LEADER ON DEATH",
		"REMOVE LEADER ON DISCONNECT",
		"REMOVE SHRINE BALLS",
		"REMOVE FIRE",
		"REMOVE WATER",
		"REMOVE FISH BUBBLES",
		"REMOVE ENTITY REMOVE",
		"REMOVE LIGHT",
		"REMOVE SMOKE",
		"REMOVE PRIESTESS",
		"REMOVE PLAYER DIE",
		"REMOVE PORTAL",
	};

	fx_index -= 1; // Convert effect index form 1-based to 0-based...

	if (fx_index < 0 || fx_index >= MAX_PERSISTANT_EFFECTS) //mxd. Added upper bound check.
	{
		Com_DPrintf("WARNING: Trying to remove a persistent effect of %i\n", fx_index);
		return false;
	}

	PerEffectsBuffer_t* fx = &persistant_effects[fx_index];
	if (fx->numEffects > 0)
	{
		memset(fx->buf, 0, sizeof(fx->buf));
		fx->bufSize = 0;
		fx->demo_send_mask = 0;
		fx->freeBlock = 0;
		fx->numEffects = 0;
		fx->send_mask = call_from;
		num_persistant_effects--;

		return true;
	}

	Com_DPrintf("WARNING: Persistent effect not found! Call from %s, last deleted by %i, last effect number %i\n", fx_types[call_from], fx->send_mask, fx->fx_num);

	return false;
}

void SV_RemoveEdictFromPersistantEffectsArray(const edict_t* ed)
{
	// Remove edict from send_mask of all persistant_effects...
	const int send_mask = EDICT_MASK(ed);
	for (int i = 0; i < MAX_PERSISTANT_EFFECTS; i++)
		persistant_effects[i].send_mask &= ~send_mask;
}

void SV_RemoveDemoEdictFromPersistantEffectsArray(const client_t* cl)
{
	// Remove edict from demo_send_mask of all persistant_effects...
	const int send_mask = EDICT_MASK(cl->edict);
	for (int i = 0; i < MAX_PERSISTANT_EFFECTS; i++)
		persistant_effects[i].demo_send_mask &= ~send_mask;
}

void SV_ClearPersistantEffects(void)
{
	PerEffectsBuffer_t* fx = &persistant_effects[0];
	for (int i = 0; i < MAX_PERSISTANT_EFFECTS; i++, fx++)
	{
		fx->send_mask = 0;
		fx->demo_send_mask = -1;
	}
}

void SV_ClearPersistantEffectBuffersArray(void)
{
	SV_PrepWorldFrame();
	num_persistant_effects = 0;
	memset(persistant_effects, 0, sizeof(persistant_effects));
}

void SV_SendClientEffects(client_t* cl)
{
#define FOV_SCALER	(0.01047197543084621f) //TODO: what's this value?
#define CAM_SCALER	(0.0000958738f) //TODO: what's this value?

	static byte fx_send_buffer[2000];
	static byte fx_demo_send_buffer[2000];

	int buffer_indices[2][MAX_EFFECT_BUFFERS];

	qboolean send_effects = false;
	qboolean send_demo_effects = false;

	int send_count[2] = { 0, 0 };
	int send_sizes[2] = { 0, 0 };

	int sent_effects_count = 0;
	int send_buffer_offset = 0;

	int sent_demo_effects_count = 0;
	int send_demo_buffer_offset = 0;

	if (effects_buffer_index > 0)
	{
		const float fov = cosf(cl->frames[sv.framenum & UPDATE_MASK].ps.fov * FOV_SCALER);

		vec3_t cam_vieworg;
		for (int i = 0; i < 3; i++)
			cam_vieworg[i] = (float)cl->lastcmd.camera_vieworigin[i] * 0.125f;

		vec3_t cam_viewangles;
		cam_viewangles[0] = -(float)cl->lastcmd.camera_viewangles[0] * CAM_SCALER;
		cam_viewangles[1] = (float)cl->lastcmd.camera_viewangles[1] * CAM_SCALER;
		cam_viewangles[2] = (float)cl->lastcmd.camera_viewangles[2] * CAM_SCALER;

		vec3_t direction;
		DirFromAngles(cam_viewangles, direction);

		vec3_t delta;
		for (int i = 0; i < 3; i++)
			delta[i] = cam_vieworg[i] - direction[i] * 200.0f;

		for (int i = 0; i < effects_buffer_index; i++)
		{
			int send_index = 1;

			if (!effects_buffers[i].is_broadcast)
			{
				vec3_t fx_delta;
				VectorSubtract(effects_buffers[i].position, delta, fx_delta);
				const float dist = VectorNormalize(fx_delta);

				if (dist <= r_farclipdist->value && 
					(dist < 1000.0f || fov <= DotProduct(direction, fx_delta)) && 
					PF_inPVS(cam_vieworg, effects_buffers[i].position))
				{
					send_index = 0;
				}
			}

			buffer_indices[send_index][send_count[send_index]] = i;
			send_sizes[send_index] += effects_buffers[i].buffer.bufSize;
			send_count[send_index]++;
		}
	}

	const int max_send_size = cl->netchan.message.maxsize - 200;
	if (max_send_size < (int)sv_pers_fx_send_cut_off->value)
	{
		Com_Printf("WARNING: sv_pers_fx_send_cut_off exceeds %i and has been capped", max_send_size);
		Cvar_SetValue("sv_pers_fx_send_cut_off", (float)max_send_size);
	}

	const int send_mask = EDICT_MASK(cl->edict);
	PerEffectsBuffer_t* pfx = &persistant_effects[0];

	const int send_size = cl->netchan.message.cursize + send_sizes[1];
	for (int i = 0; i < MAX_PERSISTANT_EFFECTS; i++, pfx++)
	{
		if (pfx->numEffects == 0)
			continue;

		if ((send_mask & pfx->send_mask) == 0)
		{
			if ((int)sv_pers_fx_send_cut_off->value <= send_size + send_buffer_offset + send_demo_buffer_offset)
				break;

			memcpy(&fx_send_buffer[send_buffer_offset], pfx->buf, pfx->bufSize);

			pfx->send_mask |= send_mask;
			send_buffer_offset += pfx->bufSize;
			sent_effects_count++;

			send_effects = true;
		}

		if ((send_mask & pfx->demo_send_mask) == 0)
		{
			if ((int)sv_pers_fx_send_cut_off->value <= send_size + send_buffer_offset + send_demo_buffer_offset)
				break;

			memcpy(&fx_demo_send_buffer[send_demo_buffer_offset], pfx->buf, pfx->bufSize);

			pfx->demo_send_mask |= send_mask;
			send_demo_buffer_offset += pfx->bufSize;
			sent_demo_effects_count++;

			send_demo_effects = true;
		}
	}

	sizebuf_t* msg = &cl->netchan.message;

	if (send_demo_effects)
	{
		MSG_WriteByte(msg, svc_demo_client_effect);
		MSG_WriteShort(msg, send_demo_buffer_offset + 1);
		MSG_WriteByte(msg, sent_demo_effects_count);
		SZ_Write(msg, fx_demo_send_buffer, send_demo_buffer_offset);
	}

	if (send_effects)
	{
		if (send_fx_framenum || cl->lastframe < 1)
		{
			send_fx_framenum = false;
			MSG_WriteByte(msg, svc_framenum);
			MSG_WriteLong(msg, sv.framenum);
		}

		MSG_WriteByte(msg, svc_client_effect);
		MSG_WriteByte(msg, sent_effects_count + send_count[1]);
		SZ_Write(msg, fx_send_buffer, send_buffer_offset);
	}
	else if (send_count[1] > 0)
	{
		if (send_fx_framenum || cl->lastframe < 1)
		{
			send_fx_framenum = false;
			MSG_WriteByte(msg, svc_framenum);
			MSG_WriteLong(msg, sv.framenum);
		}

		MSG_WriteByte(msg, svc_client_effect);
		MSG_WriteByte(msg, send_count[1]);
	}

	if (send_count[1] > 0)
	{
		for (int i = 0; i < send_count[1]; i++)
		{
			const int fx_index = buffer_indices[1][i];

			const EffectsBuffer_t* buffer = &effects_buffers[fx_index].buffer;
			SZ_Write(msg, &buffer->buf[buffer->freeBlock], buffer->bufSize);
		}
	}

	if (send_count[0] > 0)
	{
		MSG_WriteByte(&cl->datagram, svc_client_effect);
		MSG_WriteByte(&cl->datagram, send_count[0]);

		for (int i = 0; i < send_count[0]; i++)
		{
			const int fx_index = buffer_indices[0][i];

			const EffectsBuffer_t* buffer = &effects_buffers[fx_index].buffer;
			SZ_Write(&cl->datagram, &buffer->buf[buffer->freeBlock], buffer->bufSize);
		}
	}
}