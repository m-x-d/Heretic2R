#include "q_shared.h"
#include "g_local.h"
#include "matrix.h"
#include "vector.h"
#include "g_HitLocation.h"
#include "random.h"

// Returns hit location (as defined in h file) assuming upright bipedal humanoid creature.
// This will return incorrect results if called on a non humanoid (eg beetle) or a non
// upright creature (eg a tumbling creature)
#define LT_BIPED	0
#define LT_WINGED	1
#define LT_MAX		2

// Percentage offsets for a bipedal creature

static vec3_t LocationsBiped[hl_BipedPoints] =
{
	 0.00F,  0.00F,  0.24F,		// head
	 0.16F,  0.00F,  0.10F,		// hl_TorsoFront
	-0.16F,	 0.00F,  0.10F,		// hl_TorsoBack    
	 0.00F,	 0.35F,	 0.08F,		// hl_ArmUpperLeft
	 0.00F,	 0.35F,	-0.04F,		// hl_ArmLowerLeft 
	 0.00F,	-0.35F,	 0.08F,		// hl_ArmUpperRight
	 0.00F,	-0.35F,	-0.04F,		// hl_ArmLowerRight
	 0.00F,	 0.20F,	-0.10F,		// hl_LegUpperLeft 
  	 0.00F,	 0.20F,	-0.20F,		// hl_LegLowerLeft 
	 0.00F,	-0.20F,	-0.10F,		// hl_LegUpperRight
	 0.00F,	-0.20F,	-0.20F,		// hl_LegLowerRight
};

// Percentage offsets for a winged creature

static vec3_t LocationsWinged[hl_WingedPoints] =
{//	for+	left+	up+
	 0.60F,  0.00F,  0.10F,		// head (horns and neckspikes)
	 0.00F,	 0.00F,	 0.20F,		// hl_backspikes
	 0.00F,	 0.20F,	-0.20F,		// hl_leftupperleg
	 0.00F,	 0.20F,	-0.40F,		// hl_leftlowerleg
	 0.10F,	 0.20F,	-0.60F,		// hl_lefthand
  	 0.00F,	-0.20F,	-0.20F,		// hl_rightupperleg
	 0.00F,	-0.20F,	-0.40F,		// hl_rightlowerleg
	 0.10F,	-0.20F,	-0.60F,		// hl_righthand
	-0.30F,	 0.00F,	 0.00F,		// hl_tailspikes
  	 0.25F,	-1.00F,	 0.00F,		// hl_rwing
	 0.25F,	 1.00F,	 0.00F,		// hl_lwing
	-0.50F,	 0.00F,	-0.60F,		// hl_stinger
};

// Basic hit location types

static HlDef_t HitLocTypes[LT_MAX] =
{
	{ LocationsBiped, hl_BipedPoints },
	{ LocationsWinged, hl_WingedPoints }
};

void GetWorkLocations(vec3_t *work, vec3_t scale, vec3_t origin, vec_t angle, int LocType)
{
	int			i;
	vec3_t		*loc;
	vec3_t		temp;
	matrix3_t	mat;

	// Create a yaw matrix dependent on targets yaw
	CreateYawMatrix(mat, angle); 
	
	// Grab base of hit location table
	loc = HitLocTypes[LocType].LocTable;

	// Rotate each point by targets yaw and scale by targets size
	for(i = 0; i < HitLocTypes[LocType].NumLocs; i++, loc++, work++)
	{
		VectorScaleByVector(*loc, scale, temp);
		Matrix3MultByVec3(mat, temp, *work);
		VectorAdd(*work, origin, *work);
	}
}

// Find the nearest point to point in the created table

HitLocation_t GetNearestPoint(vec3_t *work, vec3_t point, int LocType)
{
	int		i;
	vec_t	dist;
	vec_t	mindist = 1024.0F;
	int		minidx = -1;

	for(i = 0; i < HitLocTypes[LocType].NumLocs; i++, work++)
	{
		dist = VectorSeparation(*work, point);
		if(dist < mindist)
		{
			mindist = dist;
			minidx = i;
		}
	}
	return(minidx);
}

HitLocation_t MG_GetHitLocation(edict_t *target, edict_t *inflictor, vec3_t ppoint, vec3_t pdir);
HitLocation_t T_GetHitLocation(edict_t *target, edict_t *inflictor, vec3_t ppoint)
{
	HitLocation_t	result;
	int				LocType;
	vec3_t			center;
	vec3_t			WorkLocations[hl_Max];
	vec3_t			point;

	// Work out which type of location table to use
/*	if(target->movetype == PHYSICSTYPE_FLY)
	{
		LocType = LT_WINGED;
	}
	else
	{*/
		return MG_GetHitLocation(target, inflictor, ppoint, vec3_origin);
//		LocType = LT_BIPED;
//	}

	// If incoming point is zero, use inflictors origin
	if(Vec3IsZero(ppoint))
		VectorCopy(inflictor->s.origin, point);
	else
		VectorCopy(ppoint, point);

	// Use absmin and absmax to get center of object
	VectorAdd(target->absmin, target->absmax, center);
	Vec3ScaleAssign(0.5, center);

	// Create a table of rotated offsets 
	GetWorkLocations(WorkLocations, target->size, center, target->s.angles[YAW], LocType);

	// Find nearest point in table of rotated offsets to collision point
	result = GetNearestPoint(WorkLocations, point, LocType);
	return(result);
}

HitLocation_t HitLocationForVFLZone [TOTAL_ZONES] = 
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