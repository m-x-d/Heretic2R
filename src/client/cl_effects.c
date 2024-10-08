//
// cl_effects.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "cl_effects.h"
#include "clfx_dll.h"
#include "dll_io.h"
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

static int GetEffect(centity_t* ent, int flags, const char* format, ...)
{
	NOT_IMPLEMENTED
	return 0;
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