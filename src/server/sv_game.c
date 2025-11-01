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
#include "tokens.h"
#include "Vector.h"

game_export_t* ge;

static HINSTANCE game_library; // Defined in sys_win.c in Q2

// Q2 counterpart
// Sends the contents of the mutlicast buffer to a single client.
static void PF_Unicast(const edict_t* ent, const qboolean reliable)
{
	if (ent == NULL)
		return;

	const int n = NUM_FOR_EDICT(ent);
	if (n < 1 || n > (int)maxclients->value)
		return;

	client_t* client = &svs.clients[n - 1];

	if (reliable)
		SZ_Write(&client->netchan.message, sv.multicast.data, sv.multicast.cursize);
	else
		SZ_Write(&client->datagram, sv.multicast.data, sv.multicast.cursize);

	SZ_Clear(&sv.multicast);
}

// Debug print to server console
static void PF_dprintf(const char* fmt, ...)
{
	char msg[1024];
	va_list argptr;

	va_start(argptr, fmt);
	vsprintf_s(msg, sizeof(msg), fmt, argptr); //mxd. vsprintf -> vsprintf_s
	va_end(argptr);

	Com_DPrintf("%s", msg); // Q2: Com_Printf
}

// Q2 counterpart
// Print to a single client.
static void PF_cprintf(const edict_t* ent, const int level, const char* fmt, ...)
{
	char msg[1024];
	va_list argptr;
	int n = 1; //mxd. Initialize to avoid compiler warning.

	if (ent != NULL)
	{
		n = NUM_FOR_EDICT(ent);
		if (n < 1 || n > (int)maxclients->value)
			Com_Error(ERR_DROP, "cprintf to a non-client");
	}

	va_start(argptr, fmt);
	vsprintf_s(msg, sizeof(msg), fmt, argptr); //mxd. vsprintf -> vsprintf_s
	va_end(argptr);

	if (ent != NULL)
		SV_ClientPrintf(&svs.clients[n - 1], level, "%s", msg);
	else
		Com_Printf("%s", msg);
}

static void PF_clprintf(const edict_t* ent, const edict_t* from, const int color, const char* fmt, ...) // H2
{
	char msg[1024];
	va_list argptr;
	int n = 1; //mxd. Initialize to avoid compiler warning.

	if (ent != NULL)
	{
		n = NUM_FOR_EDICT(ent);
		if (n < 1 || n > (int)maxclients->value)
			Com_Error(ERR_DROP, "clprintf to a non-client");
	}

	va_start(argptr, fmt);
	vsprintf_s(msg, sizeof(msg), fmt, argptr); //mxd. vsprintf -> vsprintf_s
	va_end(argptr);

	if (ent != NULL)
		SV_ClientColorPrintf(&svs.clients[n - 1], NUM_FOR_EDICT(from) - 1, (byte)color, "%s", msg);
	else
		Com_Printf("%s", msg);
}

// Q2 counterpart
// centerprint to a single client.
static void PF_centerprintf(const edict_t* ent, const char* fmt, ...)
{
	char msg[1024];
	va_list argptr;

	const int n = NUM_FOR_EDICT(ent);
	if (n < 1 || n > (int)maxclients->value)
		return;

	va_start(argptr, fmt);
	vsprintf_s(msg, sizeof(msg), fmt, argptr); //mxd. vsprintf -> vsprintf_s
	va_end(argptr);

	MSG_WriteByte(&sv.multicast, svc_centerprint);
	MSG_WriteString(&sv.multicast, msg);
	PF_Unicast(ent, true);
}

static void PF_gamemsg_centerprintf(const edict_t* ent, const short msg) // H2
{
	const int n = NUM_FOR_EDICT(ent);
	if (n < 1 || n > (int)maxclients->value)
		return;

	MSG_WriteByte(&sv.multicast, svc_gamemsg_centerprint);
	MSG_WriteShort(&sv.multicast, msg);
	PF_Unicast(ent, true);
}

static void PF_levelmsg_centerprintf(const edict_t* ent, const short msg) // H2
{
	const int n = NUM_FOR_EDICT(ent);
	if (n < 1 || n > (int)maxclients->value)
		return;

	MSG_WriteByte(&sv.multicast, svc_levelmsg_centerprint);
	MSG_WriteShort(&sv.multicast, msg);
	PF_Unicast(ent, true);
}

static void PF_captionprintf(const edict_t* ent, const short msg) // H2
{
	const int n = NUM_FOR_EDICT(ent);
	if (n < 1 || n > (int)maxclients->value)
		return;

	MSG_WriteByte(&sv.multicast, svc_captionprint);
	MSG_WriteShort(&sv.multicast, msg);
	PF_Unicast(ent, true);
}

static void PF_msgvar_centerprintf(const edict_t* ent, const short msg, const int vari) // H2
{
	const int n = NUM_FOR_EDICT(ent);
	if (n < 1 || n > (int)maxclients->value)
		return;

	MSG_WriteByte(&sv.multicast, svc_gamemsgvar_centerprint);
	MSG_WriteShort(&sv.multicast, msg);
	MSG_WriteLong(&sv.multicast, vari);
	PF_Unicast(ent, true);
}

static void PF_msgdual_centerprintf(const edict_t* ent, const short msg1, const short msg2) // H2
{
	const int n = NUM_FOR_EDICT(ent);
	if (n < 1 || n > (int)maxclients->value)
		return;

	MSG_WriteByte(&sv.multicast, svc_gamemsgdual_centerprint);
	MSG_WriteShort(&sv.multicast, msg1);
	MSG_WriteShort(&sv.multicast, msg2);
	PF_Unicast(ent, true);
}

// Q2 counterpart
// Abort the server with a game error
H2R_NORETURN static void PF_error(const char* fmt, ...)
{
	char msg[1024];
	va_list argptr;

	va_start(argptr, fmt);
	vsprintf_s(msg, sizeof(msg), fmt, argptr); //mxd. vsprintf -> vsprintf_s
	va_end(argptr);

	Com_Error(ERR_DROP, "Game Error: %s", msg);
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

		memcpy(sv.configstrings[index], val, len);
	}
	else
	{
		strcpy_s(sv.configstrings[index], sizeof(sv.configstrings[0]), val);
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

// Q2 counterpart
static void PF_WriteByte(const int c)
{
	MSG_WriteByte(&sv.multicast, c);
}

// Q2 counterpart
static void PF_WriteShort(const int c)
{
	MSG_WriteShort(&sv.multicast, c);
}

// Q2 counterpart
static void PF_WriteLong(const int c)
{
	MSG_WriteLong(&sv.multicast, c);
}

// Q2 counterpart
static void PF_WriteFloat(const float f)
{
	MSG_WriteFloat(&sv.multicast, f);
}

// Q2 counterpart
static void PF_WriteString(const char* s)
{
	MSG_WriteString(&sv.multicast, s);
}

// Q2 counterpart
static void PF_WritePos(const vec3_t pos)
{
	MSG_WritePos(&sv.multicast, pos);
}

// Q2 counterpart
static void PF_WriteDir(const vec3_t dir)
{
	MSG_WriteDir(&sv.multicast, dir);
}

// Q2 counterpart
static void PF_WriteAngle(const float f)
{
	MSG_WriteAngle(&sv.multicast, f);
}

// Q2 counterpart
// Also checks portalareas so that doors block sight.
qboolean PF_inPVS(const vec3_t p1, const vec3_t p2)
{
	int leafnum = CM_PointLeafnum(p1);
	int cluster = CM_LeafCluster(leafnum);
	const int area1 = CM_LeafArea(leafnum);
	const byte* mask = CM_ClusterPVS(cluster); //mxd. The only difference between this and PF_inPHS().

	leafnum = CM_PointLeafnum(p2);
	cluster = CM_LeafCluster(leafnum);
	const int area2 = CM_LeafArea(leafnum);

	if (mask != NULL && !(mask[cluster >> 3] & (1 << (cluster & 7))))
		return false; // More than one bounce away.

	return CM_AreasConnected(area1, area2); // A door blocks sight?
}

// Q2 counterpart
// Also checks portalareas so that doors block sight.
static qboolean PF_inPHS(const vec3_t p1, const vec3_t p2)
{
	int leafnum = CM_PointLeafnum(p1);
	int cluster = CM_LeafCluster(leafnum);
	const int area1 = CM_LeafArea(leafnum);
	const byte* mask = CM_ClusterPHS(cluster); //mxd. The only difference between this and PF_inPVS().

	leafnum = CM_PointLeafnum(p2);
	cluster = CM_LeafCluster(leafnum);
	const int area2 = CM_LeafArea(leafnum);

	if (mask != NULL && !(mask[cluster >> 3] & (1 << (cluster & 7))))
		return false; // More than one bounce away.
	
	return CM_AreasConnected(area1, area2); // A door blocks sight?
}

static void PF_StartSound(const edict_t* ent, const int channel, const int soundindex, const float volume, const float attenuation, const float timeofs)
{
	if (ent == NULL || soundindex == -1)
		return;

	if (volume > 1.0f)
		SV_StartSound(ent->s.origin, ent, channel, soundindex, 1.0f, attenuation, timeofs);
	else
		SV_StartSound(NULL, ent, channel, soundindex, volume, attenuation, timeofs);
}

static void PF_SoundEvent(const byte event_id, const float leveltime, const edict_t* ent, const int channel, const int soundindex, const float volume, const float attenuation, const float timeofs) // H2
{
	if (ent == NULL || soundindex == -1)
		return;

	if (volume > 1.0f)
		SV_StartEventSound(event_id, leveltime, ent->s.origin, ent, channel, soundindex, 1.0f, attenuation, timeofs);
	else
		SV_StartEventSound(event_id, leveltime, NULL, ent, channel, soundindex, volume, attenuation, timeofs);
}

static void PF_ChangeCDtrack(const edict_t* ent, const int track, const int loop) // H2
{
	if (ent == NULL)
		return;

	const int n = NUM_FOR_EDICT(ent);
	if (n < 1 || n > (int)maxclients->value)
		Com_Error(ERR_DROP, "changeCDtrack to a non-client");

	//mxd. Was done in a separate function in original version.
	client_t* cl = &svs.clients[n - 1];
	sizebuf_t* sb = &cl->netchan.message;

	MSG_WriteByte(sb, svc_changeCDtrack);
	MSG_WriteByte(sb, track);
	MSG_WriteByte(sb, loop);
}

static void CleanLevel(void) // H2
{
	for (int i = 1; i < MAX_SOUNDS; i++)
	{
		char* s = sv.configstrings[CS_SOUNDS + i];

		if (*s != 0 && *s != (char)TOKEN_S_PLAYER && *s != (char)TOKEN_S_MISC && *s != (char)TOKEN_S_WEAPONS && *s != (char)TOKEN_S_ITEMS)
			*s = 0;
	}

	for (int i = 1; i < MAX_MODELS; i++)
		sv.configstrings[CS_MODELS + i][0] = 0;
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

//mxd. game_import_t.DebugGraph -> SCR_DebugGraph adapter. Original logic directly calls SCR_DebugGraph.
static void SV_DebugGraph(const float value, const byte r, const byte g, const byte b)
{
	const paletteRGBA_t color = { .r = r, .g = g, .b = b, .a = 0xff };
	SCR_DebugGraph(value, color.c);
}

// Init the game subsystem for a new map
void SV_InitGameProgs(void)
{
#define FX_BUF_SIZE			16 //mxd. == sizeof(EffectsBuffer_t)
#define FX_BUF_BLOCK_SIZE	12 //mxd

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
	import.changeCDtrack = PF_ChangeCDtrack;

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
	import.cleanlevel = CleanLevel; //TODO: unused

	import.modelindex = SV_ModelIndex;
	import.modelremove = SV_ModelRemove; //TODO: unused
	import.soundindex = SV_SoundIndex;
	import.soundremove = SV_SoundRemove;
	import.imageindex = SV_ImageIndex;

	import.configstring = PF_Configstring;
	import.sound = PF_StartSound;
	import.soundevent = PF_SoundEvent;
	import.positioned_sound = SV_StartSound;

	import.WriteChar = PF_WriteByte; // PF_WriteChar in Q2 //TODO: unused
	import.WriteByte = PF_WriteByte;
	import.WriteShort = PF_WriteShort; //TODO: unused
	import.WriteLong = PF_WriteLong; //TODO: unused
	import.WriteFloat = PF_WriteFloat; //TODO: unused
	import.WriteString = PF_WriteString;
	import.WritePosition = PF_WritePos; //TODO: unused
	import.WriteDir = PF_WriteDir; //TODO: unused
	import.WriteAngle = PF_WriteAngle; //TODO: unused

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

	import.DebugGraph = SV_DebugGraph;
	import.SetAreaPortalState = CM_SetAreaPortalState;
	import.AreasConnected = CM_AreasConnected;

	import.FS_LoadFile = FS_LoadFile;
	import.FS_FreeFile = FS_FreeFile;
	import.FS_Userdir = FS_Userdir;
	import.FS_CreatePath = FS_CreatePath;
	import.Sys_LoadGameDll = Sys_LoadGameDll; //TODO: this breaks Windows logic separation.
	import.Sys_UnloadGameDll = Sys_UnloadGameDll; //TODO: this breaks Windows logic separation.
	import.ClearPersistantEffects = SV_ClearPersistantEffects;
	import.Persistant_Effects_Array = persistant_effects;

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
	ResMngr_Con(&EffectsBufferMngr, FX_BUF_SIZE, FX_BUF_BLOCK_SIZE);

	effects_buffer_index = 0;
	clfx_buffer_offset = 0;
	is_local_client = false;

	ge->Init();
}