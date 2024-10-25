//
// sv_game.c
//
// Copyright 1998 Raven Software
//

#include "server.h"
#include "cmodel.h"
#include "dll_io.h"
#include "ResourceManager.h"
#include "screen.h"
#include "sv_effects.h"
#include "Vector.h"

game_export_t* ge;

static HINSTANCE game_library; // Defined in sys_win.c in Q2

// Q2 counterpart
// Sends the contents of the mutlicast buffer to a single client.
static void PF_Unicast(const edict_t* ent, const qboolean reliable)
{
	if (ent == NULL)
		return;

	const int p = NUM_FOR_EDICT(ent);
	if (p < 1 || p > (int)maxclients->value)
		return;

	client_t* client = &svs.clients[p - 1];

	if (reliable)
		SZ_Write(&client->netchan.message, sv.multicast.data, sv.multicast.cursize);
	else
		SZ_Write(&client->datagram, sv.multicast.data, sv.multicast.cursize);

	SZ_Clear(&sv.multicast);
}

// Debug print to server console
static void PF_dprintf(char* fmt, ...)
{
	char msg[1024];
	va_list argptr;

	va_start(argptr, fmt);
	vsprintf_s(msg, sizeof(msg), fmt, argptr); //mxd. vsprintf -> vsprintf_s
	va_end(argptr);

	Com_DPrintf("%s", msg); // Q2: Com_Printf
}

static void PF_cprintf(edict_t* ent, int level, char* fmt, ...)
{
	NOT_IMPLEMENTED
}

static void PF_clprintf(edict_t* ent, edict_t* from, int color, char* fmt, ...)
{
	NOT_IMPLEMENTED
}

static void PF_centerprintf(edict_t* ent, char* fmt, ...)
{
	NOT_IMPLEMENTED
}

static void PF_gamemsg_centerprintf(edict_t* ent, short msg)
{
	NOT_IMPLEMENTED
}

static void PF_levelmsg_centerprintf(const edict_t* ent, const short msg) // H2
{
	const int p = NUM_FOR_EDICT(ent);
	if (p < 1 || p >(int)maxclients->value)
		return;

	MSG_WriteByte(&sv.multicast, svc_levelmsg_centerprint);
	MSG_WriteShort(&sv.multicast, msg);
	PF_Unicast(ent, true);
}

static void PF_captionprintf(edict_t* ent, short msg)
{
	NOT_IMPLEMENTED
}

static void PF_msgvar_centerprintf(edict_t* ent, short msg, int vari)
{
	NOT_IMPLEMENTED
}

static void PF_msgdual_centerprintf(edict_t* ent, short msg1, short msg2)
{
	NOT_IMPLEMENTED
}

static void PF_error(char* fmt, ...)
{
	NOT_IMPLEMENTED
}

// Also sets mins and maxs for inline bmodels.
static void PF_setmodel(edict_t* ent, const char* name)
{
	if (name == NULL)
		Com_Error(ERR_DROP, "PF_setmodel: NULL");

	ent->s.modelindex = (byte)SV_ModelIndex(name);

	// If it is an inline model, get the size information for it.
	if (*name == '*')
	{
		const cmodel_t* mod = CM_InlineModel(name);
		VectorCopy(mod->mins, ent->mins);
		VectorCopy(mod->maxs, ent->maxs);
		// H2: missing SV_LinkEdict(ent);
	}
}

// Q2 counterpart
static void PF_Configstring(const int index, const char* val)
{
	if (index < 0 || index >= MAX_CONFIGSTRINGS)
		Com_Error(ERR_DROP, "configstring: bad index %i\n", index);

	if (val == NULL)
		val = "";

	// Change the string in sv
	if (index >= CS_STATUSBAR && index < CS_MAXCLIENTS)
	{
		//mxd. Statusbar layout string takes several configstring slots (this is by design)...
		const uint len = strlen(val) + 1; // Count trailing zero
		if (len > (CS_MAXCLIENTS - index) * sizeof(sv.configstrings[0]))
			Com_Error(ERR_DROP, "configstring: too big statusbar layout string (%i at index %i)\n", len, index);
		else
			memcpy(sv.configstrings[index], val, len);
	}
	else
	{
		strcpy_s(sv.configstrings[index], sizeof(sv.configstrings[index]), val);
	}

	if (sv.state != ss_loading)
	{
		// Send the update to everyone
		SZ_Clear(&sv.multicast);
		MSG_WriteChar(&sv.multicast, svc_configstring);
		MSG_WriteShort(&sv.multicast, index);
		MSG_WriteString(&sv.multicast, val);

		SV_Multicast(vec3_origin, MULTICAST_ALL_R);
	}
}

static void PF_WriteByte(int c)
{
	NOT_IMPLEMENTED
}

static void PF_WriteShort(int c)
{
	NOT_IMPLEMENTED
}

static void PF_WriteLong(int c)
{
	NOT_IMPLEMENTED
}

static void PF_WriteFloat(float f)
{
	NOT_IMPLEMENTED
}

static void PF_WriteString(char* s)
{
	NOT_IMPLEMENTED
}

static void PF_WritePos(vec3_t pos)
{
	NOT_IMPLEMENTED
}

static void PF_WriteDir(vec3_t dir)
{
	NOT_IMPLEMENTED
}

static void PF_WriteAngle(float f)
{
	NOT_IMPLEMENTED
}

qboolean PF_inPVS(vec3_t p1, vec3_t p2)
{
	NOT_IMPLEMENTED
	return false;
}

static qboolean PF_inPHS(vec3_t p1, vec3_t p2)
{
	NOT_IMPLEMENTED
	return false;
}

static void PF_StartSound(edict_t* entity, int channel, int sound_num, float volume, float attenuation, float timeofs)
{
	NOT_IMPLEMENTED
}

static void PF_SoundEvent(byte EventId, float leveltime, edict_t* ent, int channel, int soundindex, float volume, float attenuation, float timeofs)
{
	NOT_IMPLEMENTED
}

static void ChangeCDtrack(edict_t* ent, int track, int loop)
{
	NOT_IMPLEMENTED
}

static void CleanLevel(void)
{
	NOT_IMPLEMENTED
}

// Called when either the entire server is being killed, or it is changing to a different game directory.
void SV_ShutdownGameProgs(void)
{
	if (ge != NULL)
	{
		// H2: shutdown effect buffers
		SV_ClearPersistantEffectBuffersArray();
		ResMngr_Des(&sv_FXBufMngr);
		ResMngr_Des(&EffectsBufferMngr);

		ge->Shutdown();
		Sys_UnloadGameDll("gamex86", &game_library); // H2
		ge = NULL;
	}
}

// Init the game subsystem for a new map
void SV_InitGameProgs(void)
{
	game_import_t import;
	game_export_t* (*GetGameAPI)(game_import_t* gi);
	
	// Unload anything we have now
	if (ge != NULL)
		SV_ShutdownGameProgs();

	// Load a new game dll
	import.multicast = SV_Multicast;
	import.unicast = PF_Unicast;
	import.bprintf = SV_BroadcastPrintf;
	import.bcaption = SV_BroadcastCaption;
	import.Obituary = SV_BroadcastObituary;
	import.dprintf = PF_dprintf;
	import.cprintf = PF_cprintf;
	import.clprintf = PF_clprintf;
	import.centerprintf = PF_centerprintf;
	import.gamemsg_centerprintf = PF_gamemsg_centerprintf;
	import.levelmsg_centerprintf = PF_levelmsg_centerprintf;
	import.captionprintf = PF_captionprintf;
	import.msgvar_centerprintf = PF_msgvar_centerprintf;
	import.msgdual_centerprintf = PF_msgdual_centerprintf;
	import.error = PF_error;
	import.changeCDtrack = ChangeCDtrack;

	import.linkentity = SV_LinkEdict;
	import.unlinkentity = SV_UnlinkEdict;
	import.BoxEdicts = SV_AreaEdicts;
	import.trace = SV_Trace;
	import.pointcontents = SV_PointContents;
	import.setmodel = PF_setmodel;
	import.inPVS = PF_inPVS;
	import.inPHS = PF_inPHS;
	import.Pmove = Pmove;
	import.FindEntitiesInBounds = SV_FindEntitiesInBounds;
	import.TraceBoundingForm = SV_TraceBoundingForm;
	import.ResizeBoundingForm = SV_ResizeBoundingForm;
	import.GetContentsAtPoint = SV_GetContentsAtPoint;
	import.CheckDistances = SV_CheckDistances;
	import.cleanlevel = CleanLevel;

	import.modelindex = SV_ModelIndex;
	import.modelremove = SV_ModelRemove;
	import.soundindex = SV_SoundIndex;
	import.soundremove = SV_SoundRemove;
	import.imageindex = SV_ImageIndex;

	import.configstring = PF_Configstring;
	import.sound = PF_StartSound;
	import.soundevent = PF_SoundEvent;
	import.positioned_sound = SV_StartSound;

	import.WriteChar = PF_WriteByte; // PF_WriteChar in Q2
	import.WriteByte = PF_WriteByte;
	import.WriteShort = PF_WriteShort;
	import.WriteLong = PF_WriteLong;
	import.WriteFloat = PF_WriteFloat;
	import.WriteString = PF_WriteString;
	import.WritePosition = PF_WritePos;
	import.WriteDir = PF_WriteDir;
	import.WriteAngle = PF_WriteAngle;

	import.CreateEffect = SV_CreateEffect;
	import.RemoveEffects = SV_RemoveEffects;
	import.CreateEffectEvent = SV_CreateEffectEvent;
	import.RemoveEffectsEvent = SV_RemoveEffectsEvent;
	import.CreatePersistantEffect = SV_CreatePersistantEffect;
	import.RemovePersistantEffect = SV_RemovePersistantEffect;

	import.TagMalloc = Z_TagMalloc;
	import.TagFree = Z_Free;
	import.FreeTags = Z_FreeTags;

	import.cvar = Cvar_Get;
	import.cvar_set = Cvar_Set;
	import.cvar_forceset = Cvar_ForceSet;
	import.cvar_variablevalue = Cvar_VariableValue;

	import.argc = Cmd_Argc;
	import.argv = Cmd_Argv;
	import.args = Cmd_Args;
	import.AddCommandString = Cbuf_AddText;

	import.DebugGraph = SCR_DebugGraph; //TODO: check definition mismatch...
	import.SetAreaPortalState = CM_SetAreaPortalState;
	import.AreasConnected = CM_AreasConnected;

	import.FS_LoadFile = FS_LoadFile;
	import.FS_FreeFile = FS_FreeFile;
	import.FS_Userdir = FS_Userdir;
	import.FS_CreatePath = FS_CreatePath;
	import.Sys_LoadGameDll = Sys_LoadGameDll; //TODO: this breaks Windows logic separation.
	import.Sys_UnloadGameDll = Sys_UnloadGameDll; //TODO: this breaks Windows logic separation.
	import.ClearPersistantEffects = SV_ClearPersistantEffects;
	import.Persistant_Effects_Array = persistant_effects_array;

	//TODO: this breaks Windows logic separation.
	DWORD checksum;
	Sys_LoadGameDll("gamex86", &game_library, &checksum);

	GetGameAPI = (void*)GetProcAddress(game_library, "GetGameAPI");
	if (GetGameAPI == NULL)
		Sys_Error("Failed to obtain 'Gamex86' API");

	ge = (*GetGameAPI)(&import);
	if (ge->apiversion != GAME_API_VERSION)
	{
		SV_ShutdownGameProgs();
		Com_Error(ERR_DROP, "Game is version %i, not %i", ge->apiversion, GAME_API_VERSION);
	}

	ResMngr_Con(&sv_FXBufMngr, ENTITY_FX_BUF_SIZE, ENTITY_FX_BUF_BLOCK_SIZE);
	ResMngr_Con(&EffectsBufferMngr, sizeof(EffectsBuffer_t), 12);

	effects_buffer_index = 0;
	effects_buffer_offset = 0;
	is_local_client = false;

	ge->Init();
}