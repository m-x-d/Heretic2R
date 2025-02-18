//
// g_HitLocation.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "q_Typedef.h" //mxd

// Enumerated hit locations.
typedef	enum HitLocation_e
{
	hl_Null = -1,
	hl_NoneSpecific = 0,
	hl_Head,
	hl_TorsoFront,
	hl_TorsoBack,
	hl_ArmUpperLeft,
	hl_ArmLowerLeft,
	hl_ArmUpperRight,
	hl_ArmLowerRight,
	hl_LegUpperLeft,
	hl_LegLowerLeft,
	hl_LegUpperRight,
	hl_LegLowerRight,
	hl_BipedPoints,
	hl_WingedPoints,
	hl_extra14,
	hl_extra15,
	hl_MeleeHit, // 16
	hl_Max,

	// 50/50 chance hit locations (mxd. used only by MG_GetHitLocation() logic!).
	hl_Half_LLL_LRL,	// Left lower leg and right lower leg.
	hl_Half_ULL_URL,	// Left upper leg and right upper leg.
	hl_Half_FT_BT,		// Front and back torso.
	hl_Half_FT_URA,		// Front torso and upper right arm.
	hl_Half_FT_ULA,		// Front torso and upper left arm.
	hl_Half_FT_LRA,		// Front torso and lower right arm.
	hl_Half_FT_LLA,		// Front torso and lower left arm.
	hl_Half_BT_URA,		// Back torso and upper right arm.
	hl_Half_BT_ULA,		// Back torso and upper left arm.
	hl_Half_BT_LRA,		// Back torso and lower right arm.
	hl_Half_BT_LLA,		// Back torso and lower left arm.
} HitLocation_t;

HitLocation_t MG_GetHitLocation(const edict_t* target, const edict_t* inflictor, const vec3_t p_point, const vec3_t p_dir);