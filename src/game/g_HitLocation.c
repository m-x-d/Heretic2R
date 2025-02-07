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

HitLocation_t MG_GetHitLocation(edict_t *target, edict_t *inflictor, vec3_t ppoint, vec3_t pdir)
{
	vec3_t			dir, point, point_dir;
	vec3_t			forward, right, up;
	vec3_t			tangles, tcenter;
	float			tradius, hdist;
	float			udot, fdot, rdot;
	int				Vertical, Forward, Lateral;
	HitLocation_t	HitLoc;

//get target forward, right and up
	if(target->client)
	{//ignore player's pitch and roll
		VectorSet(tangles, 0, target->s.angles[YAW], 0);
	}
	else
		VectorCopy(target->s.angles, tangles);

	AngleVectors(tangles, forward, right, up);

//get center of target
	VectorAdd(target->absmin, target->absmax, tcenter);
	Vec3ScaleAssign(0.5, tcenter);

//get radius width of target
	tradius = (fabs(target->maxs[0]) + fabs(target->maxs[1]) + fabs(target->mins[0]) + fabs(target->mins[1]))/4;

//get impact point
	if(ppoint && Vec3NotZero(ppoint))
		VectorCopy(ppoint, point);
	else
		VectorCopy(inflictor->s.origin, point);//this is bad!

//get impact dir
	if(pdir && Vec3NotZero(pdir))
		VectorCopy(pdir, dir);
	else
	{//take the inflictor's last origin to current to get dir
		VectorSubtract(inflictor->s.origin, inflictor->s.old_origin, dir);
		if(Vec3IsZero(dir))
		{//ok, that didn't work, make dir to target center, ignoring z
			VectorSubtract(target->s.origin, inflictor->s.origin, dir);
			dir[2] = 0;
		}
		VectorNormalize(dir);
	}

//put point at controlled distance from center
	hdist = vhlen(point, tcenter);

	VectorMA(point, hdist - tradius, dir, point);
	//now a point on the surface of a cylinder with a radius of tradius
	
	VectorSubtract(point, tcenter, point_dir);
	VectorNormalize(point_dir);

	//Get bottom to top (Vertical) position index
	udot = DotProduct(up, point_dir);
	if(udot>.666)
		Vertical = 4;
	else if(udot>.333)
		Vertical = 3;
	else if(udot>-.333)
		Vertical = 2;
	else if(udot>-.666)
		Vertical = 1;
	else
		Vertical = 0;

	//Get back to front (Forward) position index
	fdot = DotProduct(forward, point_dir);
	if(fdot>.666)
		Forward = 4;
	else if(fdot>.333)
		Forward = 3;
	else if(fdot>-.333)
		Forward = 2;
	else if(fdot>-.666)
		Forward = 1;
	else
		Forward = 0;

	//Get left to right (Lateral) position index
	rdot = DotProduct(right, point_dir);
	if(rdot>.666)
		Lateral = 4;
	else if(rdot>.333)
		Lateral = 3;
	else if(rdot>-.333)
		Lateral = 2;
	else if(rdot>-.666)
		Lateral = 1;
	else
		Lateral = 0;

//FIXME: make one for horizonal bodies (harpies, corpses)
	HitLoc = hit_location_for_vfl_zone[Vertical * 25 + Forward * 5 + Lateral];

	switch(HitLoc)
	{
		case hl_Half_LLL_LRL://left lower leg and right lower leg:
			if(!irand(0,1))
				return hl_LegLowerLeft;
			else
				return hl_LegUpperRight;
			break;
		case hl_Half_ULL_URL://left upper leg and right upper leg:
			if(!irand(0,1))
				return hl_LegUpperLeft;
			else
				return hl_LegLowerRight;
			break;
		case hl_Half_FT_BT://front and back torso:
			if(!irand(0,1))
				return hl_TorsoFront;
			else
				return hl_TorsoBack;
			break;
		case hl_Half_FT_URA://front torso and upper right arm:
			if(!irand(0,1))
				return hl_TorsoFront;
			else
				return hl_ArmUpperRight;
			break;
		case hl_Half_FT_ULA://front torso and upper left arm:
			if(!irand(0,1))
				return hl_TorsoFront;
			else
				return hl_ArmUpperLeft;
			break;
		case hl_Half_FT_LRA://front torso and lower right arm:
			if(!irand(0,1))
				return hl_TorsoFront;
			else
				return hl_ArmLowerRight;
			break;
		case hl_Half_FT_LLA://front torso and lower left arm:
			if(!irand(0,1))
				return hl_TorsoFront;
			else
				return hl_ArmLowerLeft;
			break;
		case hl_Half_BT_URA://back torso and upper right arm:
			if(!irand(0,1))
				return hl_TorsoBack;
			else
				return hl_ArmUpperRight;
			break;
		case hl_Half_BT_ULA://back torso and upper left arm:
			if(!irand(0,1))
				return hl_TorsoBack;
			else
				return hl_ArmUpperLeft;
			break;
		case hl_Half_BT_LRA://back torso and lower right arm:
			if(!irand(0,1))
				return hl_TorsoBack;
			else
				return hl_ArmLowerRight;
			break;
		case hl_Half_BT_LLA://back torso and lower left arm:
			if(!irand(0,1))
				return hl_TorsoBack;
			else
				return hl_ArmLowerLeft;
			break;
		default:
			return HitLoc;
			break;
	}
	//never happens:
	return hl_NoneSpecific;
}

// end