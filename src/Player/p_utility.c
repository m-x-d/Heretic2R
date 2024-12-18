//
// p_utility.c -- mxd. Utility functions to reduce code repetition...
//
// Copyright 1998 Raven Software
//

#include "Player.h"
#include "p_utility.h"
#include "EffectFlags.h"

//mxd. 'contantmask', 'flags' and 'passent' args are always the same in Player.dll, so...
void P_Trace(const playerinfo_t* info, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, trace_t* trace)
{
	if (info->isclient)
		info->CL_Trace(start, mins, maxs, end, MASK_PLAYERSOLID, CEF_CLIP_TO_WORLD, trace);
	else
		info->G_Trace(start, mins, maxs, end, info->self, MASK_PLAYERSOLID, trace);
}

//mxd. 'origin', 'leveltime', 'entity', 'attenuation' and 'timeofs' args are always the same in Player.dll, so...
void P_Sound(const playerinfo_t* info, const byte EventId, const int channel, const char* soundname, const float fvol)
{
	if (info->isclient)
		info->CL_Sound(EventId, info->origin, channel, soundname, fvol, ATTN_NORM, 0);
	else
		info->G_Sound(EventId, info->leveltime, info->self, channel, info->G_SoundIndex(soundname), fvol, ATTN_NORM, 0);
}