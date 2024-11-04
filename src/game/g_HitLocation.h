#ifndef G_HITLOCATION_H
#define	G_HITLOCATION_H
// Enumerated hit locations

typedef	enum HitLocation_e
{
	hl_Null				= -1,
	hl_NoneSpecific		= 0,
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
	hl_MeleeHit,//16
	hl_Max,
	//50/50 HITLOCS
	hl_Half_LLL_LRL,//left lower leg and right lower leg
	hl_Half_ULL_URL,//left upper leg and right upper leg
	hl_Half_FT_BT,//front and back torso
	hl_Half_FT_URA,//front torso and upper right arm
	hl_Half_FT_ULA,//front torso and upper left arm
	hl_Half_FT_LRA,//front torso and lower right arm
	hl_Half_FT_LLA,//front torso and lower left arm
	hl_Half_BT_URA,//back torso and upper right arm
	hl_Half_BT_ULA,//back torso and upper left arm
	hl_Half_BT_LRA,//back torso and lower right arm
	hl_Half_BT_LLA,//back torso and lower left arm
} HitLocation_t;

typedef enum BloodType_e
{
	BLOODTYPE_NONE,
	BLOODTYPE_RED,
	BLOODTYPE_GREEN,
	BLOODTYPE_STONE_CHIPS,
	BLOODTYPE_WOOD_CHIPS,
	NUM_BLOOTYPES
} BloodType_t;

typedef struct HlDef_t
{
	vec3_t	*LocTable;
	int		NumLocs;
} HlDef_t;

HitLocation_t T_GetHitLocation(struct edict_s *target, struct edict_s *inflictor, vec3_t point);
HitLocation_t MG_GetHitLocation(edict_t *target, edict_t *inflictor, vec3_t ppoint, vec3_t pdir);

#define TOTAL_ZONES		125//5 upper, 5 forward, 5 lateral

#endif
