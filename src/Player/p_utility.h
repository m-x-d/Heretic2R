//
// p_utility.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "g_Typedef.h"

void P_Trace(const struct playerinfo_s* info, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, trace_t* trace);
void P_Sound(const playerinfo_t* info, byte EventId, int channel, const char* soundname, float fvol);
void P_LocalSound(const playerinfo_t* info, const char* soundname);

//mxd. GROSS HACKS, because one can't just pass optional args from one function to another in C...
#define P_CreateEffect(info, EventId, owner, type, flags, origin, format, ...) \
	if (info->isclient) \
		info->CL_CreateEffect(EventId, owner, (ushort)type, flags, origin, format, ##__VA_ARGS__); \
	else \
		info->G_CreateEffect(EventId, (owner != NULL ? info->G_GetEntityStatePtr((edict_t *)owner) : NULL), type, flags, origin, format, ##__VA_ARGS__)