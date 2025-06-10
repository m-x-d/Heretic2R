//
// cl_effects.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "cl_effects.h"
#include "clfx_dll.h"
#include "cmodel.h"
#include "dll_io.h"
#include "FX.h"
#include "game.h"
#include "g_items.h"
#include "ResourceManager.h"
#include "Vector.h"
#include "vid_dll.h"

entity_t* PlayerEntPtr;
float cam_transparency;
ResourceManager_t fx_buffer_manager;
qboolean precache_models;

static float effect_event_id_time_array[128];
static EffectsBuffer_t client_prediction_effects;
static int cl_effectpredict;

static qboolean Get_Crosshair(vec3_t origin, byte* type)
{
	vec3_t start;
	vec3_t end;
	vec3_t angles;
	vec3_t forward;
	vec3_t right;
	vec3_t mins;
	vec3_t maxs;
	trace_t trace;

	if (crosshair == NULL || !(int)crosshair->value || cl.time < 1001 || PlayerEntPtr == NULL || (int)cl_cinematicfreeze->value)
		return false;

	VectorClear(mins);
	VectorClear(maxs);

	for (int i = 0; i < 3; i++)
		angles[i] = cl.viewangles[i] + (float)cl.frame.playerstate.pmove.delta_angles[i] * SHORT_TO_ANGLE;

	AngleVectors(angles, forward, right, NULL);

	VectorCopy(PlayerEntPtr->origin, start);
	start[2] += cl.playerinfo.viewheight;

	const uint weapon = cl.frame.playerstate.weapon;

	switch (weapon)
	{
		case ITEM_WEAPON_HELLSTAFF:
			start[2] += 20.0f;
			VectorMA(start, 12.0f, right, start);
			break;

		case ITEM_WEAPON_MAGICMISSILE:
			start[2] += 14.0f;
			VectorMA(start, 4.0f, right, start);
			break;

		case ITEM_WEAPON_REDRAINBOW:
			start[2] += 18.0f;
			for (int i = 0; i < 3; i++)
				start[i] -= right[i] * 4.0f;
			break;

		case ITEM_WEAPON_SPHEREOFANNIHILATION:
		case ITEM_WEAPON_MACEBALLS:
			start[2] += 22.0f;
			VectorMA(start, 4.0f, right, start);
			break;
		
		case 9: //TODO: either we are using incorrect enum for other cases, or this one is for non-existing weapon...
			start[2] += 21.0f;
			for (int i = 0; i < 3; i++)
				start[i] -= right[i] * 2.0f;
			break;

		default:
			start[2] += 18.0f;
			break;
	}

	float fwd_offset = 256.0f;
	if (in_do_autoaim && cl.frame.playerstate.AutotargetEntityNum > 0 && 
		weapon != 9 && weapon != ITEM_WEAPON_REDRAINBOW && weapon != ITEM_WEAPON_PHOENIXBOW)
	{
		const centity_t* target_ent = &cl_entities[cl.frame.playerstate.AutotargetEntityNum];
		VectorSubtract(target_ent->origin, start, forward);
		fwd_offset = VectorNormalize(forward);
	}

	VectorMA(start, fwd_offset, forward, end);

	trace_ignore_player = true;
	CL_Trace(start, mins, maxs, end, MASK_SHOT | CONTENTS_ILLUSIONARY | CONTENTS_CAMERABLOCK, CONTENTS_DETAIL | CONTENTS_TRANSLUCENT, &trace);
	trace_ignore_player = false;

	// Store results.
	VectorCopy(trace.endpos, origin);
	*type = (byte)Q_ftol(crosshair->value - 1);

	return true;
}

static qboolean InCameraPVS(vec3_t pos) // H2
{
	const int leafnum = CM_PointLeafnum(pos);
	const int area = CM_LeafArea(leafnum);

	if (cl.refdef.areabits == NULL)
		return true;

	// Check for door-connected areas.
	if (cl.refdef.areabits[area >> 3] & (1 << (area & 7)))
		return true;

	return false;
}

static struct model_s* RegisterModel(const char* name)
{
	if (!precache_models)
		Sys_Error("Client fx model not precached");

	return re.RegisterModel(name);
}

static int GetEffect(centity_t* ent, const int flags, const char* format, ...)
{
	va_list argptr;
	EffectsBuffer_t* fx_buf;
	sizebuf_t* msg;
	sizebuf_t sb;

	if (format == NULL)
		Com_Error(ERR_DROP, "CL_ReadEffect: null format string");

	const qboolean broadcast = (flags & (CEF_BROADCAST | CEF_MULTICAST));

	if (ent != NULL && !broadcast)
	{
		if (cl_effectpredict)
			fx_buf = &client_prediction_effects;
		else
			fx_buf = &ent->current.clientEffects;

		memset(&sb, 0, sizeof(sb));
		sb.data = fx_buf->buf;
		sb.maxsize = fx_buf->bufSize;
		sb.readcount = fx_buf->freeBlock;
		sb.cursize = sb.maxsize;

		msg = &sb;
	}
	else
	{
		fx_buf = NULL; //mxd
		msg = &net_message;
	}

	int num_params = 0;

	va_start(argptr, format);
	for (const char* s = format; *s != 0; s++, num_params++)
	{
		switch (*s)
		{
			case 'b':
				*va_arg(argptr, byte*) = (byte)MSG_ReadByte(msg);
				break;

			case 's':
				*va_arg(argptr, short*) = (short)MSG_ReadShort(msg);
				break;

			case 'i':
				*va_arg(argptr, int*) = MSG_ReadLong(msg);
				break;

			case 'f':
				*va_arg(argptr, float*) = MSG_ReadFloat(msg);
				break;

			case 'p':
			case 'v':
				MSG_ReadPos(msg, *va_arg(argptr, vec3_t*));
				break;

			case 'd':
				MSG_ReadDir(msg, *va_arg(argptr, vec3_t*));
				break;

			case 'u':
				MSG_ReadDirMag(msg, *va_arg(argptr, vec3_t*));
				break;

			case 'x':
				MSG_ReadYawPitch(msg, *va_arg(argptr, vec3_t*));
				break;

			case 't':
				MSG_ReadShortYawPitch(msg, *va_arg(argptr, vec3_t*));
				break;

			default:
				return 0;
		}
	}
	va_end(argptr);

	if (ent != NULL && !broadcast)
		fx_buf->freeBlock = msg->readcount;

	return num_params;
}

//mxd. Also written by SV_CreateEffectEvent(). Parsed by ParseEffects() in ClientEffects/Main.c
int CL_CreateEffect(const byte event_id, const void* owner, const ushort fx_type, const int flags, const vec3_t origin, const char* format, ...)
{
	sizebuf_t sb;

	if (cl.playerinfo.ishistory || !cl.playerinfo.pers.connected || owner == NULL)
		return 0;

	if (client_prediction_effects.buf == NULL)
	{
		client_prediction_effects.buf = (byte*)ResMngr_AllocateResource(&fx_buffer_manager, ENTITY_FX_BUF_SIZE);
		client_prediction_effects.bufSize = ENTITY_FX_BUF_SIZE;
		client_prediction_effects.numEffects = 0;
	}

	SZ_Init(&sb, &client_prediction_effects.buf[client_prediction_effects.freeBlock], client_prediction_effects.bufSize - client_prediction_effects.freeBlock);

	if (flags != 0)
	{
		MSG_WriteShort(&sb, fx_type | EFFECT_FLAGS);
		MSG_WriteByte(&sb, flags);
	}
	else
	{
		MSG_WriteShort(&sb, fx_type);
	}

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

		effect_event_id_time_array[event_id] = cl.playerinfo.Highestleveltime;
	}

	client_prediction_effects.freeBlock += sb.cursize;
	client_prediction_effects.numEffects++;

	return sb.cursize;
}

void CL_RemoveEffects(const byte event_id, const void* owner, const int fx_type)
{
	if (owner != NULL)
		CL_CreateEffect(event_id, ((const edict_t*)owner)->client, FX_REMOVE_EFFECTS, CEF_OWNERS_ORIGIN, NULL, "s", fx_type);
}

void CL_UnloadClientEffects(void)
{
	fxe.ShutDown();
	ResMngr_Des(&fx_buffer_manager);
	Sys_UnloadGameDll("Client Effects", &clfx_library);
	memset(&fxe, 0, sizeof(fxe));

	fxapi_initialized = false;
}

void CL_InitClientEffects(const char* dll_name)
{
	DWORD checksum;

	if (fxapi_initialized)
		CL_UnloadClientEffects();

	Com_ColourPrintf(P_HEADER, "------ Loading %s ------\n", dll_name);
	Sys_LoadGameDll(dll_name, &clfx_library, &checksum);

	// Init client_fx_import_t 
	fxi.cl_predict = cl_predict;
	fxi.cl = &cl;
	fxi.cls = &cls;
	fxi.server_entities = cl_entities;
	fxi.parse_entities = cl_parse_entities;
	fxi.EffectEventIdTimeArray = effect_event_id_time_array;
	fxi.leveltime = &cl.frame.playerstate.leveltime;
	fxi.Highestleveltime = &cl.playerinfo.Highestleveltime;
	fxi.clientPredEffects = &client_prediction_effects;
	fxi.net_message = &net_message;
	fxi.PlayerEntPtr = &PlayerEntPtr;
	fxi.PlayerAlpha = &cam_transparency;
	fxi.predictinfo = &cl.predictinfo;
	fxi.FXBufMngr = &fx_buffer_manager;
	fxi.cl_effectpredict = &cl_effectpredict;
	fxi.Sys_Error = VID_Error;
	fxi.Com_Error = Com_Error;
	fxi.Con_Printf = VID_Printf;
	fxi.Cvar_Get = Cvar_Get;
	fxi.Cvar_Set = Cvar_Set;
	fxi.Cvar_SetValue = Cvar_SetValue;
	fxi.Cvar_VariableValue = Cvar_VariableValue;
	fxi.Cvar_VariableString = Cvar_VariableString;
	fxi.Activate_Screen_Flash = Activate_Screen_Flash;
	fxi.Activate_Screen_Shake = Activate_Screen_Shake;
	fxi.Get_Crosshair = Get_Crosshair;
	fxi.S_StartSound = se.StartSound;
	fxi.S_RegisterSound = se.RegisterSound;
	fxi.RegisterModel = RegisterModel;
	fxi.GetEffect = GetEffect;
	fxi.TagMalloc = Z_TagMalloc;
	fxi.TagFree = Z_Free;
	fxi.FreeTags = Z_FreeTags;
	fxi.Trace = CL_Trace;
	fxi.InCameraPVS = InCameraPVS;

	GetfxAPI = (void*)GetProcAddress(clfx_library, "GetfxAPI");
	if (GetfxAPI == NULL)
		Com_Error(ERR_FATAL, "GetProcAddress failed on %s", dll_name);

	CLFX_Init();
	fxe.Init();

	if (fxe.api_version != FX_API_VERSION)
	{
		CL_UnloadClientEffects();
		Com_Error(ERR_FATAL, "%s has incompatible api_version", dll_name);
	}

	ResMngr_Con(&fx_buffer_manager, ENTITY_FX_BUF_SIZE, ENTITY_FX_BUF_BLOCK_SIZE);
	Com_ColourPrintf(P_HEADER, "------------------------------------\n");

	fxapi_initialized = true;
}