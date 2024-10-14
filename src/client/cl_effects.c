//
// cl_effects.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "cl_effects.h"
#include "clfx_dll.h"
#include "dll_io.h"
#include "EffectFlags.h"
#include "FX.h"
#include "ResourceManager.h"
#include "sound.h"
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
	NOT_IMPLEMENTED
	return false;
}

static qboolean InCameraPVS(vec3_t pos)
{
	NOT_IMPLEMENTED
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

	if (ent != NULL && !(flags & (FX_EXPLOSION1 | FX_LINKEDBLOOD)))
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

	if (ent != NULL && !(flags & (FX_EXPLOSION1 | FX_LINKEDBLOOD)))
		fx_buf->freeBlock = msg->readcount;

	return num_params;
}

int CL_CreateEffect(byte EventId, void* owner, ushort type, int flags, vec3_t position, char* format, ...)
{
	NOT_IMPLEMENTED
	return 0;
}

void CL_RemoveEffects(byte EventId, void* owner, int fx)
{
	NOT_IMPLEMENTED
}

void CL_UnloadClientEffects(void)
{
	NOT_IMPLEMENTED
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
	fxi.S_StartSound = S_StartSound;
	fxi.S_RegisterSound = S_RegisterSound;
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