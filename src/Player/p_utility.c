//
// p_utility.c -- mxd. Utility functions to reduce code repetition...
//
// Copyright 2024 mxd
//

#include "Player.h"
#include "p_utility.h"
#include "EffectFlags.h"

//mxd. 'contantmask', 'flags' and 'passent' args are always the same in Player.dll, so...
void P_Trace(const playerinfo_t* info, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, trace_t* trace)
{
	if (info->isclient)
	{
		//H2_BUGFIX: mxd. CEF_CLIP_TO_WORLD in original logic.
		// Fixes broken animations when failing to un-crouch while below a func_ brush or rotating in place while standing on a func_ brush.
		info->CL_Trace(start, mins, maxs, end, MASK_PLAYERSOLID, CTF_CLIP_TO_ALL, trace);
	}
	else
	{
		info->G_Trace(start, mins, maxs, end, info->self, MASK_PLAYERSOLID, trace);
	}
}

//mxd. 'origin', 'leveltime', 'entity', 'attenuation' and 'timeofs' args are always the same in Player.dll, so...
void P_Sound(const playerinfo_t* info, const byte event_id, const int channel, const char* soundname, const float fvol)
{
	if (info->isclient)
		info->CL_Sound(event_id, info->origin, channel, soundname, fvol, ATTN_NORM, 0);
	else
		info->G_Sound(event_id, info->leveltime, info->self, channel, info->G_SoundIndex(soundname), fvol, ATTN_NORM, 0);
}

//mxd. 'entity' arg is always the same in Player.dll, so...
void P_LocalSound(const playerinfo_t* info, const char* soundname)
{
	if (!info->isclient)
		info->G_L_Sound(info->self, info->G_SoundIndex(soundname));
}

//mxd. 'owner' arg is always the same in Player.dll, so...
void P_RemoveEffects(const playerinfo_t* info, const byte event_id, const int type)
{
	if (info->isclient)
		info->CL_RemoveEffects(event_id, info->self, type);
	else
		info->G_RemoveEffects(event_id, info->G_GetEntityStatePtr(info->self), type);
}

float ClampAngleDeg(float angle) //mxd. Was CL_NormaliseAngle() in p_actions in original version.
{
	// Returns the remainder.
	angle = fmodf(angle, 360.0f);

	// Makes the angle signed.
	if (angle > 180.0f)
		return angle - 360.0f;

	if (angle < -180.0f)
		return angle + 360.0f;

	return angle;
}