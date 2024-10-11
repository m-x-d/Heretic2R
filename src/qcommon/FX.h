#include "EffectFlags.h"

// ************************************************************************************************
// FX_XXX
// ------
// Id's for all the client-effects types.
// ************************************************************************************************

typedef enum FX_Types_e
{	// NOTE We currently have 126 client effects, we cannot exceed 32768 - ha !! Like we'll ever reach that !
	FX_REMOVE_EFFECTS=0,			// special fx type for removing client fx
	FX_TEST,						
	FX_EXPLOSION1,					
	FX_EXPLOSION2,					
	FX_SPLASH,						
	FX_GIB_TRAIL,					//5
	FX_BLOOD,
	FX_BLOOD_TRAIL,
	FX_LINKEDBLOOD,
	FX_SPARKS,
	FX_PLAYER_TELEPORT_IN,			//10
	FX_PICKUP_HEALTH,
	FX_PICKUP_WEAPON,
	FX_PICKUP_DEFENSE,
	FX_PICKUP_PUZZLE,
	FX_PICKUP_AMMO,					//15
	FX_WEAPON_FLYINGFIST,
	FX_WEAPON_FLYINGFISTEXPLODE,
	FX_SPELL_BLUERING,				
	FX_SPELL_METEORBARRIER,			// there is a reason for this - we create 4 different persistant effects
	FX_SPELL_METEORBARRIER1,		//20
	FX_SPELL_METEORBARRIER2,
	FX_SPELL_METEORBARRIER3,
	FX_SPELL_METEORBARRIER_TRAVEL,
	FX_SPELL_METEORBARRIEREXPLODE,
	FX_SPELL_LIGHTNINGSHIELD,		//25
	FX_WEAPON_SPHERE,
	FX_WEAPON_SPHEREGLOWBALLS,
	FX_WEAPON_SPHEREEXPLODE, 
	FX_WEAPON_SPHEREPOWER,
	FX_WEAPON_SPHEREPLAYEREXPLODE, 	//30
	FX_WEAPON_MAGICMISSILE,
	FX_WEAPON_MAGICMISSILEEXPLODE,
	FX_WEAPON_BLAST,
	FX_WEAPON_REDRAINMISSILE,
	FX_WEAPON_REDRAIN,				//35
	FX_WEAPON_REDRAINGLOW, 		   
	FX_WEAPON_MACEBALL,
	FX_WEAPON_MACEBALLBOUNCE,
	FX_WEAPON_MACEBALLEXPLODE, 
	FX_WEAPON_PHOENIXMISSILE,		//40
	FX_WEAPON_PHOENIXEXPLODE, 
	FX_SPELL_MORPHMISSILE,
	FX_SPELL_MORPHMISSILE_INITIAL,
	FX_SPELL_MORPHEXPLODE,
	FX_WEAPON_FIREWAVE,				//45
	FX_WEAPON_FIREWAVEWORM,
	FX_WEAPON_FIREBURST,
	FX_WEAPON_RIPPEREXPLODE,
	FX_WATER_ENTRYSPLASH,				                  
	FX_WATER_RIPPLES,				//50	                  
	FX_WATER_WAKE,						                  
	FX_BUBBLER,
	FX_SCORCHMARK,						                  
	FX_DEBRIS,							                  
	FX_FLESH_DEBRIS,				//55			                  
	FX_SHADOW,							                  
	FX_ANIMATE,							  
	FX_FOUNTAIN,						  
	FX_WATERFALLBASE,						  
	FX_DRIPPER,						//60	                  
	FX_MIST,							                  
	FX_PLAGUEMIST,						                  
	FX_PLAGUEMISTEXPLODE,				                  
	FX_SPELLHANDS,						                  
	FX_LENSFLARE,					//65	                  
	FX_STAFF,							                  
	FX_SPOO,							                  
	FX_HALO,							  
	FX_REMOTE_CAMERA,				
	FX_WEAPON_HELLBOLT,				//70
	FX_WEAPON_HELLBOLTEXPLODE,
	FX_WEAPON_HELLSTAFF_POWER,
	FX_WEAPON_HELLSTAFF_POWER_BURN,
	FX_SPELL_CHANGE,
	FX_STAFF_CREATE,				//75
	FX_STAFF_CREATEPOOF,
	FX_STAFF_REMOVE,
	FX_DUST_PUFF,
	FX_FIRE,
	FX_SOUND,						//80
	FX_PICKUP,
	FX_HITPUFF,
	FX_DUST,
	FX_ENVSMOKE,
	FX_SPOO_SPLAT,					//85
	FX_BODYPART,	
	FX_PLAYER_TELEPORT_OUT,
	FX_PLAYER_PERSISTANT,
	FX_PLAYER_TORCH,
	FX_TOME_OF_POWER,				//90
	FX_FIRE_ON_ENTITY,
	FX_FLAREUP,
	FX_SHRINE_PLAYER,
	FX_SHRINE_MANA,
	FX_SHRINE_LUNGS,				//95
	FX_SHRINE_LIGHT,
	FX_SHRINE_REFLECT,
	FX_SHRINE_ARMOR,
	FX_SHRINE_HEALTH,
	FX_SHRINE_STAFF,				//100
	FX_SHRINE_GHOST,
	FX_SHRINE_SPEED,
	FX_SHRINE_POWERUP,
	FX_ROPE,
	FX_FIREHANDS,					//105
	FX_SHRINE_BALL,
	FX_SHRINE_BALL_EXPLODE,
	FX_OGLE_HITPUFF,
	FX_HP_MISSILE,
	FX_I_EFFECTS,					//110
	FX_CHICKEN_EXPLODE,
	FX_FLAMETHROWER,
	FX_TELEPORT_PAD,
	FX_QUAKE,
	FX_LIGHTNING,					//115
	FX_POWER_LIGHTNING,
	FX_BUBBLE,
	FX_TPORTSMOKE,
	FX_WATER_PARTICLES,
	FX_M_EFFECTS,					//120
	FX_HP_STAFF,
	FX_WATER_BUBBLE,
	FX_MAGIC_PORTAL,
	FX_TB_EFFECTS,
	FX_TEST_BBOX,					//125
	FX_THROWWEAPON,	
	FX_SSITHRA_ARROW,	
	FX_PE_SPELL,	
	FX_LIGHTNING_HIT,
	FX_FOOTSTEP,					//130
	FX_FALLSHORT,
	FX_WEAPON_STAFF_STRIKE,
	FX_ARMOR_HIT,
	FX_BARREL_EXPLODE,
	FX_CWATCHER,					//135
	FX_CORPSE_REMOVE,
	FX_SHOW_LEADER,
	FX_TORNADO,
	FX_TORNADO_BALL,
	FX_TORNADO_BALL_EXPLODE,		//140
	FX_FOOT_TRAIL,					
	FX_BLOCK_SPARKS,
	FX_CROSSHAIR,
	
	NUM_FX,

	// not spawnable in the Game DLL,

	FX_PUFF
} FX_Type_t;

// ************************************************************************************************
// FX_ANIM_XXX
// -----------
// Id's for static object animations as client effects
// ************************************************************************************************

enum
{
	FX_ANIM_BANNER=0,
	FX_ANIM_CANDELABRUM,
	FX_ANIM_CHANDELIER2,
	FX_ANIM_FLAME,
	FX_ANIM_FIRE,
	FX_ANIM_BANNERONPOLE,
	FX_ANIM_FLAGONPOLE,
	FX_ANIM_COCOON,
	FX_ANIM_LABPARTSCONTAINER,
	FX_ANIM_LABTRAY,
	FX_ANIM_EYEBALLJAR,
	FX_ANIM_HANGING_OGLE,
	NUM_FX_ANIM
};

// ************************************************************************************************
// MaterialID_t
// ------------
// Id's for all the material types that we care about in the world (e.g. different materials yield
// different debris chunks when an FX_DEBRI is created).
// ************************************************************************************************

typedef enum MaterialID_e
{
	MAT_STONE,			// 0
	MAT_GREYSTONE,		// 1
	MAT_CLOTH,			// 2
	MAT_METAL,			// 3 
	MAT_FLESH,			// 4
	MAT_POTTERY,		// 5
	MAT_GLASS,			// 6
	MAT_LEAF,			// 7
	MAT_WOOD,			// 8
	MAT_BROWNSTONE,		// 9
	MAT_NONE,			// 10
	MAT_INSECT,			// 11
	NUM_MAT
} MaterialID_t;

//These are used in the same field as MAT_ for client effects,
//if MAT_ goes up to 16 or higher, change these!
#define SIF_FLAG_MASK	15
#define SIF_INWATER		16
#define SIF_INLAVA		32
#define SIF_INMUCK		64

// ************************************************************************************************
// SPELLHANDS_XXX
// --------------
// ************************************************************************************************

enum
{
	SPELLHANDS_RED=0,
	SPELLHANDS_BLUE,
	SPELLHANDS_INDIGO,
	NUM_SPELLHANDS
};

#define SPELLHANDS_SHIFT		16
#define SPELLHANDS_MASK_COLOUR	0x00ff0000
#define SPELLHANDS_MASK_REFPTS	0x0000ffff
		
// ************************************************************************************************
// All this crap below is to be pulled out at some point REAL soon.
// ************************************************************************************************

#define SERVER_SENT 1
#define SERVER_DELETED 2

enum
{
	SPLASH_UNKNOWN = 0,
	SPLASH_SPARKS,
	SPLASH_BLUE_WATER,
	SPLASH_BROWN_WATER,
	SPLASH_SLIME,
	SPLASH_LAVA,
	SPLASH_BLOOD,
	NUM_SPLASHES
};

// Player Muzzle Flashes
enum
{
	PF_BLASTER = 0,
	PF_HYPERBLASTER,
	PF_LOGIN,
	PF_LOGOUT,
	PF_RESPAWN,
};

// Mosnter Muzzle Flashes
enum
{
	MF_ACTOR_MACHINEGUN_1
};


// persistant effect debugging
enum
{
	REMOVE_SHIELD = 0,
	REMOVE_TELEPORT_PAD,
	REMOVE_METEOR,
	REMOVE_LEADER,
	REMOVE_LEADER_DIE,
	REMOVE_LEADER_CLIENT,
	REMOVE_SHRINE,
	REMOVE_FIRE,
	REMOVE_WATER,
	REMOVE_FISH,
	REMOVE_ENTITY,
	REMOVE_LIGHT,
	REMOVE_SMOKE,
	REMOVE_PRIESTESS,
	REMOVE_DIE,
	REMOVE_PORTAL,
	MAX_REMOVE

};

