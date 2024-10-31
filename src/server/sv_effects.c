//
// sv_effects.c
//
// Copyright 1998 Raven Software
//

#include "sv_effects.h"
#include "EffectFlags.h"
#include "Vector.h"

qboolean send_fx_framenum;

ResourceManager_t sv_FXBufMngr;
ResourceManager_t EffectsBufferMngr;

int num_persistant_effects;
PerEffectsBuffer_t persistant_effects_array[MAX_PERSISTANT_EFFECTS];

int effects_buffer_index;
int effects_buffer_offset;

static byte clfx_buffer[4096];
static EffectsBuffer_t effects_buffers_arr[256];
static vec3_t BroadcastEffectPositions_arr[256];
static qboolean broadcast_effect_infos_used[256];

void SV_CreateEffect(entity_state_t* ent, int type, int flags, vec3_t origin, char* format, ...)
{
	NOT_IMPLEMENTED
}

void SV_RemoveEffects(entity_state_t* ent, int type)
{
	NOT_IMPLEMENTED
}

//mxd. Parsed by ParseEffects() in ClientEffects/Main.c
void SV_CreateEffectEvent(const byte EventId, entity_state_t* ent, const int type, int flags, const vec3_t origin, const char* format, ...)
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
		clfx = &effects_buffers_arr[effects_buffer_index];
		clfx->buf = clfx_buffer;
		clfx->freeBlock = effects_buffer_offset;

		length = (int)sizeof(clfx_buffer) - effects_buffer_offset;
	}

	SZ_Init(&sb, &clfx->buf[clfx->freeBlock], length);

	if (flags != 0)
	{
		MSG_WriteShort(&sb, type | EFFECT_PRED_INFO | EFFECT_FLAGS);
		MSG_WriteByte(&sb, EventId);

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
		MSG_WriteShort(&sb, type);
	}

	broadcast_effect_infos_used[effects_buffer_index] = broadcast;

	if (!(flags & CEF_OWNERS_ORIGIN))
		MSG_WritePos(&sb, origin);

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
	}
	else
	{
		clfx->numEffects = 1;
		clfx->bufSize = sb.cursize;

		if (!(flags & CEF_BROADCAST))
			VectorCopy(origin, BroadcastEffectPositions_arr[effects_buffer_index]);

		effects_buffer_offset += sb.cursize;
		effects_buffer_index++;
	}
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
	MSG_WriteShort(&sb, type | EFFECT_FLAGS);

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

void SV_SendClientEffects(client_t* cl)
{
#define FOV_SCALER	(0.01047197543084621f) //TODO: what's this value?
#define CAM_SCALER	(0.0000958738f) //TODO: what's this value?

	static byte fx_send_buffer[2000];
	static byte fx_demo_send_buffer[2000];

	int buffer_indices[2][256];

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

			if (!broadcast_effect_infos_used[i])
			{
				vec3_t fx_delta;
				VectorSubtract(BroadcastEffectPositions_arr[i], delta, fx_delta);
				const float dist = VectorNormalize(fx_delta);

				if (dist <= r_farclipdist->value && 
					(dist < 1000.0f || fov <= DotProduct(direction, fx_delta)) && 
					PF_inPVS(cam_vieworg, BroadcastEffectPositions_arr[i]))
				{
					send_index = 0;
				}
			}

			buffer_indices[send_index][send_count[send_index]] = i;
			send_sizes[send_index] += effects_buffers_arr[i].bufSize;
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
	PerEffectsBuffer_t* pfx_buf = &persistant_effects_array[0];

	const int send_size = cl->netchan.message.cursize + send_sizes[1]; // send_size2
	for (int i = 0; i < MAX_PERSISTANT_EFFECTS; i++, pfx_buf++)
	{
		if (pfx_buf->numEffects == 0)
			continue;

		if ((send_mask & pfx_buf->send_mask) == 0)
		{
			if ((int)sv_pers_fx_send_cut_off->value <= send_size + send_buffer_offset + send_demo_buffer_offset)
				break;

			memcpy(&fx_send_buffer[send_buffer_offset], pfx_buf->buf, pfx_buf->bufSize);

			pfx_buf->send_mask |= send_mask;
			send_buffer_offset += pfx_buf->bufSize;
			sent_effects_count++;

			send_effects = true;
		}

		if ((send_mask & pfx_buf->demo_send_mask) == 0)
		{
			if ((int)sv_pers_fx_send_cut_off->value <= send_size + send_buffer_offset + send_demo_buffer_offset)
				break;

			memcpy(&fx_demo_send_buffer[send_demo_buffer_offset], pfx_buf->buf, pfx_buf->bufSize);

			pfx_buf->demo_send_mask |= send_mask;
			send_demo_buffer_offset += pfx_buf->bufSize;
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
			const int buf_offset = effects_buffers_arr[fx_index].freeBlock;
			SZ_Write(msg, &effects_buffers_arr[fx_index].buf[buf_offset], effects_buffers_arr[fx_index].bufSize);
		}
	}

	if (send_count[0] > 0)
	{
		MSG_WriteByte(&cl->datagram, svc_client_effect);
		MSG_WriteByte(&cl->datagram, send_count[0]);

		for (int i = 0; i < send_count[0]; i++)
		{
			const int fx_index = buffer_indices[0][i];
			const int buf_offset = effects_buffers_arr[fx_index].freeBlock;
			SZ_Write(&cl->datagram, &effects_buffers_arr[fx_index].buf[buf_offset], effects_buffers_arr[fx_index].bufSize);
		}
	}
}