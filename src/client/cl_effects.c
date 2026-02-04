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

static void UpdateAutoaimCrosshairAnimation(vec3_t origin, const qboolean do_autoaim) //mxd
{
#define CROSSHAIR_ANIMATION_LENGTH	150 //mxd. In milliseconds.

	static vec3_t old_origin;
	static vec3_t lerp_origin;
	static int old_entity_num;
	static int anim_end_time;
	static int old_servercount;

	// Reset when map changed or game loaded...
	if (cl.servercount != old_servercount)
	{
		old_entity_num = 0;
		anim_end_time = -1;
		VectorCopy(origin, old_origin);

		old_servercount = cl.servercount;
	}

	const int cur_entity_num = (do_autoaim ? cl.frame.playerstate.AutotargetEntityNum : 0);

	// Autotargeting started, stopped or jumped to different target.
	if (old_entity_num != cur_entity_num)
	{
		// Already animating? Need to update old_origin...
		if (cl.time < anim_end_time)
			VectorCopy(lerp_origin, old_origin);

		anim_end_time = cl.time + CROSSHAIR_ANIMATION_LENGTH;
		old_entity_num = cur_entity_num;
	}

	// Animate transition.
	if (cl.time < anim_end_time)
	{
		const float lerp = 1.0f - ((float)(anim_end_time - cl.time) / CROSSHAIR_ANIMATION_LENGTH);
		VectorLerp(old_origin, sinf(ANGLE_90 * lerp), origin, lerp_origin);
		VectorCopy(lerp_origin, origin);
	}
	else // Store current origin.
	{
		VectorCopy(origin, old_origin);
	}
}

static qboolean Get_Crosshair(vec3_t origin, byte* type)
{
#define CROSSHAIR_FORWARD_OFFSET	256.0f //mxd

	enum WeaponItemIndex_e //mxd. Weapon gitem_t indices in itemlist[].
	{
		II_WEAPON_SWORDSTAFF = 1,
		II_WEAPON_FLYINGFIST,
		II_WEAPON_HELLSTAFF,
		II_WEAPON_MAGICMISSILE,
		II_WEAPON_REDRAINBOW,
		II_WEAPON_FIREWALL,
		II_WEAPON_PHOENIXBOW,
		II_WEAPON_SPHEREOFANNIHILATION,
		II_WEAPON_MACEBALLS
	};

	if (cl.time < 1001 || !(int)crosshair->value || PlayerEntPtr == NULL || (int)cl_cinematicfreeze->value || cl.frame.playerstate.remote_id != REMOTE_ID_NONE) //mxd. Also hide crosshair when looking through remote camera.
		return false;

	vec3_t angles;
	for (int i = 0; i < 3; i++)
		angles[i] = cl.viewangles[i] + (float)cl.frame.playerstate.pmove.delta_angles[i] * SHORT_TO_ANGLE;

	vec3_t forward;
	vec3_t right;
	AngleVectors(angles, forward, right, NULL);

	vec3_t start = VEC3_INITA(PlayerEntPtr->origin, 0.0f, 0.0f, cl.playerinfo.viewheight);
	const uint weapon = cl.frame.playerstate.weapon;

	switch (weapon)
	{
		case II_WEAPON_FLYINGFIST:
			start[2] += 20.0f;
			VectorMA(start, 12.0f, right, start);
			break;

		case II_WEAPON_HELLSTAFF:
			start[2] += 14.0f;
			VectorMA(start, 4.0f, right, start);
			break;

		case II_WEAPON_MAGICMISSILE:
			start[2] += 18.0f;
			for (int i = 0; i < 3; i++)
				start[i] -= right[i] * 4.0f;
			break;

		case II_WEAPON_REDRAINBOW:
		case II_WEAPON_PHOENIXBOW:
			start[2] += 22.0f;
			VectorMA(start, 4.0f, right, start);
			break;
		
		case II_WEAPON_MACEBALLS:
			start[2] += 21.0f;
			for (int i = 0; i < 3; i++)
				start[i] -= right[i] * 2.0f;
			break;

		default: // II_WEAPON_SWORDSTAFF, II_WEAPON_FIREWALL, II_WEAPON_SPHEREOFANNIHILATION
			start[2] += 18.0f;
			break;
	}

	vec3_t end;
	const qboolean do_autoaim = (in_do_autoaim
		&& weapon != II_WEAPON_SWORDSTAFF && weapon != II_WEAPON_MAGICMISSILE
		&& weapon != II_WEAPON_FIREWALL && weapon != II_WEAPON_MACEBALLS); //mxd. +II_WEAPON_SWORDSTAFF.

	// If we have AutotargetEntityNum, we've already traced to autotarget entity position and can hit it --mxd.
	if (do_autoaim && cl.frame.playerstate.AutotargetEntityNum > 0)
	{
		const centity_t* target_ent = &cl_entities[cl.frame.playerstate.AutotargetEntityNum];

		//mxd. Replicate GetAimVelocity() logic.
		if (cls.serverProtocol == H2R_PROTOCOL_VERSION)
		{
			VectorAverage(target_ent->current.mins, target_ent->current.maxs, end); // Get center of model.
			end[2] += target_ent->current.maxs[2] / 2.0f;
		}
		else
		{
			// Reconstruct ent's bbox (replicate CL_ClipMoveToEntities() logic)...
			const float zd = 8.0f * (float)((target_ent->current.solid >> 5) & 31);
			const float zu = 8.0f * (float)((target_ent->current.solid >> 10) & 63) - 32;

			// Aim at ent's vertical center.
			VectorSet(end, 0.0f, 0.0f, -zd * 0.5f + zu);
		}

		// Make it absolute.
		Vec3AddAssign(target_ent->origin, end);
	}
	else
	{
		//mxd. Setup end pos from view_pos. This way projectiles fired by player will always hit end position, regardless of distance to it (has to be setup the same way in projectile firing logic).
		//mxd. view_pos has to be above player's head, otherwise crosshair will be obscured by it...
		const vec3_t view_pos = VEC3_SET(PlayerEntPtr->origin[0], PlayerEntPtr->origin[1], start[2]);
		VectorMA(view_pos, CROSSHAIR_FORWARD_OFFSET, forward, end);
	}

	trace_t trace;
	CL_Trace(start, vec3_origin, vec3_origin, end, MASK_SHOT | CONTENTS_ILLUSIONARY | CONTENTS_CAMERABLOCK, CTF_CLIP_TO_ALL | CTF_IGNORE_PLAYER, &trace);

	//mxd. Do free aim <-> autoaim crosshair animation.
	UpdateAutoaimCrosshairAnimation(trace.endpos, do_autoaim);

	// Store results.
	VectorCopy(trace.endpos, origin);
	*type = (byte)crosshair->value - 1;

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

	assert(client_prediction_effects.numEffects < FX_BUF_MAX_EFFECTS); //mxd

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

	GetfxAPI = (GetfxAPI_t)GetProcAddress(clfx_library, "GetfxAPI");
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