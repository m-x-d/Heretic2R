//
// p_utility.h
//
// Copyright 2024 m-x-d
//

#pragma once

#include "g_Typedef.h"

extern void P_Trace(const struct playerinfo_s* info, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, trace_t* trace);
extern void P_Sound(const playerinfo_t* info, byte event_id, int channel, const char* soundname, float fvol);
extern void P_LocalSound(const playerinfo_t* info, const char* soundname);
extern void P_RemoveEffects(const playerinfo_t* info, byte event_id, int type);

//mxd. GROSS HACKS, because one can't just pass optional args from one function to another in C...
#define P_CreateEffect(info, event_id, owner, type, flags, origin, format, ...) \
	if (info->isclient) \
		info->CL_CreateEffect(event_id, owner, (ushort)type, flags, origin, format, ##__VA_ARGS__); \
	else \
		info->G_CreateEffect(event_id, (owner != NULL ? info->G_GetEntityStatePtr((edict_t *)owner) : NULL), type, flags, origin, format, ##__VA_ARGS__)