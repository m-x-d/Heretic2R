#include "q_shared.h"
#include "g_local.h"
#include "matrix.h"
#include "vector.h"
#include "g_HitLocation.h"
#include "random.h"

HitLocation_t HitLocationForVFLZone [] = 
{//Lateral:
//	0-20(left),		20-40(lmid),	40-60(mid),		60-80(rmid),	80-100(right)
//Vertical: Between 0% and 20% of height (Lower Leg/Feet)
	//Forward: Between 0% and 20% from back to front (Back)
	hl_LegLowerLeft,hl_LegLowerLeft,hl_Half_LLL_LRL,hl_LegLowerRight,hl_LegLowerRight,
	//Forward: Between 20% and 40% from back to front (BackMid)
	hl_LegLowerLeft,hl_LegLowerLeft,hl_Half_LLL_LRL,hl_LegLowerRight,hl_LegLowerRight,
	//Forward: Between 40% and 60% from back to front (Middle)
	hl_LegLowerLeft,hl_LegLowerLeft,hl_Half_LLL_LRL,hl_LegLowerRight,hl_LegLowerRight,
	//Forward: Between 60% and 80% from back to front (Fwd Middle)
	hl_LegLowerLeft,hl_LegLowerLeft,hl_Half_LLL_LRL,hl_LegLowerRight,hl_LegLowerRight,
	//Forward: Between 80% and 100% from back to front (Forward)
	hl_LegLowerLeft,hl_LegLowerLeft,hl_Half_LLL_LRL,hl_LegLowerRight,hl_LegLowerRight,

//Vertical: Between 20% and 40% of height (Upper Leg/Pelvis)
	//Forward: Between 0% and 20% from back to front (Back)
	hl_LegUpperLeft,hl_LegUpperLeft,hl_Half_ULL_URL,hl_LegUpperRight,hl_LegUpperRight,
	//Forward: Between 20% and 40% from back to front (BackMid)
	hl_LegUpperLeft,hl_LegUpperLeft,hl_Half_ULL_URL,hl_LegUpperRight,hl_LegUpperRight,
	//Forward: Between 40% and 60% from back to front (Middle)
	hl_LegUpperLeft,hl_LegUpperLeft,hl_Half_FT_BT,	hl_LegUpperRight,hl_LegUpperRight,
	//Forward: Between 60% and 80% from back to front (Fwd Middle)
	hl_LegUpperLeft,hl_LegUpperLeft,hl_Half_ULL_URL,hl_LegUpperRight,hl_LegUpperRight,
	//Forward: Between 80% and 100% from back to front (Forward)
	hl_LegUpperLeft,hl_LegUpperLeft,hl_Half_ULL_URL,hl_LegUpperRight,hl_LegUpperRight,

//Vertical: Between 40% and 60% of height (Lower Torso/Arm)
	//Forward: Between 0% and 20% from back to front (Back)
	hl_Half_BT_LLA,	hl_TorsoBack,	hl_TorsoBack,	hl_TorsoBack,	hl_Half_BT_LRA,
	//Forward: Between 20% and 40% from back to front (BackMid)
	hl_ArmLowerLeft,hl_TorsoBack,	hl_TorsoBack,	hl_TorsoBack,	hl_ArmLowerRight,
	//Forward: Between 40% and 60% from back to front (Middle)
	hl_ArmLowerLeft,hl_Half_FT_BT,	hl_Half_FT_BT,	hl_Half_FT_BT,	hl_ArmLowerRight,
	//Forward: Between 60% and 80% from back to front (Fwd Middle)
	hl_ArmLowerLeft,hl_TorsoFront,	hl_TorsoFront,	hl_TorsoFront,	hl_ArmLowerRight,
	//Forward: Between 80% and 100% from back to front (Forward)
	hl_Half_FT_LLA,	hl_TorsoFront,	hl_TorsoFront,	hl_TorsoFront,	hl_Half_FT_LRA,

//Vertical: Between 60% and 80% of height (Upper Torso/Arm)
	//Forward: Between 0% and 20% from back to front (Back)
	hl_Half_BT_ULA,	hl_TorsoBack,	hl_TorsoBack,	hl_TorsoBack,	hl_Half_BT_URA,
	//Forward: Between 20% and 40% from back to front (BackMid)
	hl_ArmUpperLeft,hl_TorsoBack,	hl_TorsoBack,	hl_TorsoBack,	hl_ArmUpperRight,
	//Forward: Between 40% and 60% from back to front (Middle)
	hl_ArmUpperLeft,hl_Half_FT_BT,	hl_Half_FT_BT,	hl_Half_FT_BT,	hl_ArmUpperRight,
	//Forward: Between 60% and 80% from back to front (Fwd Middle)
	hl_ArmUpperLeft,hl_TorsoFront,	hl_TorsoFront,	hl_TorsoFront,	hl_ArmUpperRight,
	//Forward: Between 80% and 100% from back to front (Forward)
	hl_Half_FT_ULA,	hl_TorsoFront,	hl_TorsoFront,	hl_TorsoFront,	hl_Half_FT_URA,

//Vertical: Between 80% and 100% of height (Head)
	//Forward: Between 0% and 20% from back to front (Back)
	hl_Half_BT_ULA,	hl_TorsoBack,	hl_TorsoBack,	hl_TorsoBack,	hl_Half_BT_URA,
	//Forward: Between 20% and 40% from back to front (BackMid)
	hl_ArmUpperLeft,hl_Head,		hl_Head,		hl_Head,		hl_ArmUpperRight,
	//Forward: Between 40% and 60% from back to front (Middle)
	hl_ArmUpperLeft,hl_Head,		hl_Head,		hl_Head,		hl_ArmUpperRight,
	//Forward: Between 60% and 80% from back to front (Fwd Middle)
	hl_ArmUpperLeft,hl_Head,		hl_Head,		hl_Head,		hl_ArmUpperRight,
	//Forward: Between 80% and 100% from back to front (Forward)
	hl_Half_FT_ULA,	hl_TorsoFront,	hl_TorsoFront,	hl_TorsoFront,	hl_Half_FT_URA,
//Lateral:
//	0-20(left),		20-40(lmid),	40-60(mid),		60-80(rmid),	80-100(right)
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
	HitLoc = HitLocationForVFLZone[Vertical * 25 + Forward * 5 + Lateral];

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