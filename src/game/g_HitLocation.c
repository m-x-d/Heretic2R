//
// g_HitLocation.c
//
// Copyright 1998 Raven Software
//

#include "g_local.h"
#include "g_HitLocation.h"
#include "Random.h"
#include "Vector.h"

// 5 upper, 5 forward, 5 lateral.
static HitLocation_t hit_location_for_vfl_zone[] =
{
	// Lateral:
	// 0 - 20 (left),	20 - 40 (lmid),		40 - 60(mid),		60 - 80 (rmid),		80 - 100 (right)

	// Vertical: between 0% and 20% of height (lower leg / feet).
	hl_LegLowerLeft,	hl_LegLowerLeft,	hl_Half_LLL_LRL,	hl_LegLowerRight,	hl_LegLowerRight,	// Between 0% and 20% from back to front (back).
	hl_LegLowerLeft,	hl_LegLowerLeft,	hl_Half_LLL_LRL,	hl_LegLowerRight,	hl_LegLowerRight,	// Between 20% and 40% from back to front (middle-back).
	hl_LegLowerLeft,	hl_LegLowerLeft,	hl_Half_LLL_LRL,	hl_LegLowerRight,	hl_LegLowerRight,	// Between 40% and 60% from back to front (middle).
	hl_LegLowerLeft,	hl_LegLowerLeft,	hl_Half_LLL_LRL,	hl_LegLowerRight,	hl_LegLowerRight,	// Between 60% and 80% from back to front (middle-forward).
	hl_LegLowerLeft,	hl_LegLowerLeft,	hl_Half_LLL_LRL,	hl_LegLowerRight,	hl_LegLowerRight,	// Between 80% and 100% from back to front (forward).

	// Vertical: between 20% and 40% of height (upper leg / pelvis).
	hl_LegUpperLeft,	hl_LegUpperLeft,	hl_Half_ULL_URL,	hl_LegUpperRight,	hl_LegUpperRight,	// Between 0% and 20% from back to front (back).
	hl_LegUpperLeft,	hl_LegUpperLeft,	hl_Half_ULL_URL,	hl_LegUpperRight,	hl_LegUpperRight,	// Between 20% and 40% from back to front (middle-back).
	hl_LegUpperLeft,	hl_LegUpperLeft,	hl_Half_FT_BT,		hl_LegUpperRight,	hl_LegUpperRight,	// Between 40% and 60% from back to front (middle).
	hl_LegUpperLeft,	hl_LegUpperLeft,	hl_Half_ULL_URL,	hl_LegUpperRight,	hl_LegUpperRight,	// Between 60% and 80% from back to front (middle-forward).
	hl_LegUpperLeft,	hl_LegUpperLeft,	hl_Half_ULL_URL,	hl_LegUpperRight,	hl_LegUpperRight,	// Between 80% and 100% from back to front (forward).

	// Vertical: between 40% and 60% of height (lower torso / arms).
	hl_Half_BT_LLA,		hl_TorsoBack,		hl_TorsoBack,		hl_TorsoBack,		hl_Half_BT_LRA,		// Between 0% and 20% from back to front (back).
	hl_ArmLowerLeft,	hl_TorsoBack,		hl_TorsoBack,		hl_TorsoBack,		hl_ArmLowerRight,	// Between 20% and 40% from back to front (middle-back).
	hl_ArmLowerLeft,	hl_Half_FT_BT,		hl_Half_FT_BT,		hl_Half_FT_BT,		hl_ArmLowerRight,	// Between 40% and 60% from back to front (middle).
	hl_ArmLowerLeft,	hl_TorsoFront,		hl_TorsoFront,		hl_TorsoFront,		hl_ArmLowerRight,	// Between 60% and 80% from back to front (middle-forward).
	hl_Half_FT_LLA,		hl_TorsoFront,		hl_TorsoFront,		hl_TorsoFront,		hl_Half_FT_LRA,		// Between 80% and 100% from back to front (forward).

	// Vertical: between 60% and 80% of height (upper torso / arms).
	hl_Half_BT_ULA,		hl_TorsoBack,		hl_TorsoBack,		hl_TorsoBack,		hl_Half_BT_URA,		// Between 0% and 20% from back to front (back).
	hl_ArmUpperLeft,	hl_TorsoBack,		hl_TorsoBack,		hl_TorsoBack,		hl_ArmUpperRight,	// Between 20% and 40% from back to front (middle-back).
	hl_ArmUpperLeft,	hl_Half_FT_BT,		hl_Half_FT_BT,		hl_Half_FT_BT,		hl_ArmUpperRight,	// Between 40% and 60% from back to front (middle).
	hl_ArmUpperLeft,	hl_TorsoFront,		hl_TorsoFront,		hl_TorsoFront,		hl_ArmUpperRight,	// Between 60% and 80% from back to front (middle-forward).
	hl_Half_FT_ULA,		hl_TorsoFront,		hl_TorsoFront,		hl_TorsoFront,		hl_Half_FT_URA,		// Between 80% and 100% from back to front (forward).

	// Vertical: between 80% and 100% of height (head).
	hl_Half_BT_ULA,		hl_TorsoBack,		hl_TorsoBack,		hl_TorsoBack,		hl_Half_BT_URA,		// Between 0% and 20% from back to front (Back)
	hl_ArmUpperLeft,	hl_Head,			hl_Head,			hl_Head,			hl_ArmUpperRight,	// Between 20% and 40% from back to front (BackMid)
	hl_ArmUpperLeft,	hl_Head,			hl_Head,			hl_Head,			hl_ArmUpperRight,	// Between 40% and 60% from back to front (Middle)
	hl_ArmUpperLeft,	hl_Head,			hl_Head,			hl_Head,			hl_ArmUpperRight,	// Between 60% and 80% from back to front (Fwd Middle)
	hl_Half_FT_ULA,		hl_TorsoFront,		hl_TorsoFront,		hl_TorsoFront,		hl_Half_FT_URA,		// Between 80% and 100% from back to front (Forward)
};

HitLocation_t MG_GetHitLocation(const edict_t* target, const edict_t* inflictor, const vec3_t p_point, const vec3_t p_dir)
{
	// Get target forward, right and up.
	vec3_t tgt_angles;
	if (target->client != NULL)
		VectorSet(tgt_angles, 0.0f, target->s.angles[YAW], 0.0f); // Ignore player's pitch and roll.
	else
		VectorCopy(target->s.angles, tgt_angles);

	vec3_t v_fwd;
	vec3_t v_right;
	vec3_t v_up;
	AngleVectors(tgt_angles, v_fwd, v_right, v_up);

	// Get center of target.
	vec3_t tgt_center;
	VectorAdd(target->absmin, target->absmax, tgt_center);
	Vec3ScaleAssign(0.5f, tgt_center);

	// Get radius width of target.
	const float tgt_radius = (fabsf(target->maxs[0]) + fabsf(target->maxs[1]) + fabsf(target->mins[0]) + fabsf(target->mins[1])) / 4.0f;

	// Get impact point.
	vec3_t point;
	if (p_point != NULL && Vec3NotZero(p_point))
		VectorCopy(p_point, point);
	else
		VectorCopy(inflictor->s.origin, point); // This is bad!

	// Get impact dir.
	vec3_t dir;
	if (p_dir != NULL && Vec3NotZero(p_dir))
	{
		VectorCopy(p_dir, dir);
	}
	else
	{
		// Take the inflictor's last origin to current to get dir.
		VectorSubtract(inflictor->s.origin, inflictor->s.old_origin, dir);

		if (Vec3IsZero(dir))
		{
			// OK, that didn't work, make dir to target center, ignoring z.
			VectorSubtract(target->s.origin, inflictor->s.origin, dir);
			dir[2] = 0.0f;
		}

		VectorNormalize(dir);
	}

	// Put point at controlled distance from center.
	const float hor_dist = vhlen(point, tgt_center);

	// Now a point on the surface of a cylinder with a radius of tgt_radius.
	VectorMA(point, hor_dist - tgt_radius, dir, point);

	vec3_t point_dir;
	VectorSubtract(point, tgt_center, point_dir);
	VectorNormalize(point_dir);

	// Get bottom to top (Vertical) position index.
	const float up_dot = DotProduct(v_up, point_dir);

	int vertical;
	if (up_dot > 0.666f)
		vertical = 4;
	else if (up_dot > 0.333f)
		vertical = 3;
	else if (up_dot > -0.333f)
		vertical = 2;
	else if (up_dot > -0.666f)
		vertical = 1;
	else
		vertical = 0;

	//Get back to front (Forward) position index
	const float fwd_dot = DotProduct(v_fwd, point_dir);

	int forward;
	if (fwd_dot > 0.666f)
		forward = 4;
	else if (fwd_dot > 0.333f)
		forward = 3;
	else if (fwd_dot > -0.333f)
		forward = 2;
	else if (fwd_dot > -0.666f)
		forward = 1;
	else
		forward = 0;

	// Get left to right (Lateral) position index.
	const float right_dot = DotProduct(v_right, point_dir);

	int lateral;
	if (right_dot > 0.666f)
		lateral = 4;
	else if (right_dot > 0.333f)
		lateral = 3;
	else if (right_dot > -0.333f)
		lateral = 2;
	else if (right_dot > -0.666f)
		lateral = 1;
	else
		lateral = 0;

	//FIXME: make one for horizontal bodies (harpies, corpses).
	const HitLocation_t hit_loc = hit_location_for_vfl_zone[vertical * 25 + forward * 5 + lateral];

	switch (hit_loc)
	{
		case hl_Half_LLL_LRL:	return (irand(0, 1) ? hl_LegUpperRight : hl_LegLowerLeft);	// Left lower leg and right lower leg.
		case hl_Half_ULL_URL:	return (irand(0, 1) ? hl_LegLowerRight : hl_LegUpperLeft);	// Left upper leg and right upper leg.

		case hl_Half_FT_BT:		return (irand(0, 1) ? hl_TorsoBack     : hl_TorsoFront);	// Front and back torso.
		case hl_Half_FT_URA:	return (irand(0, 1) ? hl_ArmUpperRight : hl_TorsoFront);	// Front torso and upper right arm.
		case hl_Half_FT_ULA:	return (irand(0, 1) ? hl_ArmUpperLeft  : hl_TorsoFront);	// Front torso and upper left arm.
		case hl_Half_FT_LRA:	return (irand(0, 1) ? hl_ArmLowerRight : hl_TorsoFront);	// Front torso and lower right arm.
		case hl_Half_FT_LLA:	return (irand(0, 1) ? hl_ArmLowerLeft  : hl_TorsoFront);	// Front torso and lower left arm.

		case hl_Half_BT_URA:	return (irand(0, 1) ? hl_ArmUpperRight : hl_TorsoBack);		// Back torso and upper right arm.
		case hl_Half_BT_ULA:	return (irand(0, 1) ? hl_ArmUpperLeft  : hl_TorsoBack);		// Back torso and upper left arm.
		case hl_Half_BT_LRA:	return (irand(0, 1) ? hl_ArmLowerRight : hl_TorsoBack);		// Back torso and lower right arm.
		case hl_Half_BT_LLA:	return (irand(0, 1) ? hl_ArmLowerLeft  : hl_TorsoBack);		// Back torso and lower left arm.

		default: return hit_loc;
	}
}