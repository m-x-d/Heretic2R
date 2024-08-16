#ifndef PLAYER_H
#define PLAYER_H

#ifdef PLAYER_DLL

#define PLAYER_API __declspec(dllexport)

#else

#define PLAYER_API __declspec(dllimport)

#endif	// PLAYER_DLL

#include "p_types.h"


// **************
// Movement rates
// **************

#define IN_MOVE_CREEP_MIN	16
#define IN_MOVE_CREEP		32
#define IN_MOVE_WALK_MIN	48
#define IN_MOVE_WALK		64
#define IN_MOVE_RUN_MIN		80
#define IN_MOVE_RUN			96

#define IN_MOVE_THRESHOLD	IN_MOVE_CREEP_MIN

#define BUTTON_WALK			0

enum movefwd_e
{
	MOVE_BACK_RUN,
	MOVE_BACK_WALK,
	MOVE_BACK_CREEP,
	MOVE_FWD_NONE,
	MOVE_FWD_CREEP,
	MOVE_FWD_WALK,
	MOVE_FWD_RUN,
	MOVE_FWD_MAX
};

enum moveright_e
{
	MOVE_LEFT_RUN,
	MOVE_LEFT_WALK,
	MOVE_LEFT_CREEP,
	MOVE_RIGHT_NONE,
	MOVE_RIGHT_CREEP,
	MOVE_RIGHT_WALK,
	MOVE_RIGHT_RUN,
	MOVE_RIGHT_MAX
};

enum moveplus_e
{
	MOVE_NORM,
	MOVE_NOFWD,
	MOVE_NOSIDE,
};


#define DEFAULT_PLAYER_LIB "Player"

typedef struct
{
	paceldata_t *PlayerSeqData,*PlayerChickenData;
	int			p_num_items;
	gitem_t		*p_itemlist;
} player_export_t;

void P_Freelib(void);
unsigned int P_Load(char *name);

#ifdef PLAYER_DLL

PLAYER_API void P_Init(void);
PLAYER_API void P_Shutdown(void);

#else

void (*P_Init)(void);
void (*P_Shutdown)(void);

void (*P_PlayerReleaseRope)(playerinfo_t *playerinfo);
void (*P_KnockDownPlayer)(playerinfo_t *playerinfo);
void (*P_PlayFly)(playerinfo_t *playerinfo, float dist);
void (*P_PlaySlap)(playerinfo_t *playerinfo, float dist);
void (*P_PlayScratch)(playerinfo_t *playerinfo, float dist);
void (*P_PlaySigh)(playerinfo_t *playerinfo, float dist);
void (*P_SpawnDustPuff)(playerinfo_t *playerinfo, float dist);
void (*P_PlayerInterruptAction)(playerinfo_t *playerinfo);

qboolean (*P_BranchCheckDismemberAction)(playerinfo_t *playerinfo, int weapon);

void (*P_TurnOffPlayerEffects)(playerinfo_t *playerinfo);
void (*P_AnimUpdateFrame)(playerinfo_t *playerinfo);
void (*P_PlayerFallingDamage)(playerinfo_t *playerinfo);

void (*P_PlayerBasicAnimReset)(playerinfo_t *playerinfo);
void (*P_PlayerAnimReset)(playerinfo_t *playerinfo);
void (*P_PlayerAnimSetLowerSeq)(playerinfo_t *playerinfo, int seq);
void (*P_PlayerAnimSetUpperSeq)(playerinfo_t *playerinfo, int seq);
void (*P_PlayerAnimUpperIdle)(playerinfo_t *playerinfo);
void (*P_PlayerAnimLowerIdle)(playerinfo_t *playerinfo);
void (*P_PlayerAnimUpperUpdate)(playerinfo_t *playerinfo);
void (*P_PlayerAnimLowerUpdate)(playerinfo_t *playerinfo);
void (*P_PlayerAnimSetVault)(playerinfo_t *playerinfo, int seq);
void (*P_PlayerPlayPain)(playerinfo_t *playerinfo, int type);

void (*P_PlayerIntLand)(playerinfo_t *playerinfo_t, float landspeed);

void (*P_PlayerInit)(playerinfo_t *playerinfo, int complete_reset);
void (*P_PlayerClearEffects)(playerinfo_t *playerinfo);
void (*P_PlayerUpdate)(playerinfo_t *playerinfo);
void (*P_PlayerUpdateCmdFlags)(playerinfo_t *playerinfo);
void (*P_PlayerUpdateModelAttributes)(playerinfo_t *playerinfo);

void (*P_Weapon_Ready)(playerinfo_t *playerinfo,gitem_t *Weapon);
void (*P_Weapon_EquipSpell)(playerinfo_t *playerinfo,gitem_t *Weapon);
void (*P_Weapon_EquipSwordStaff)(playerinfo_t *playerinfo,gitem_t *Weapon);
void (*P_Weapon_EquipHellStaff)(playerinfo_t *playerinfo,gitem_t *Weapon);
void (*P_Weapon_EquipBow)(playerinfo_t *playerinfo,gitem_t *Weapon);
void (*P_Weapon_EquipArmor)(playerinfo_t *playerinfo, gitem_t *Weapon);
int (*P_Weapon_CurrentShotsLeft)(playerinfo_t *playerinfo);
int (*P_Defence_CurrentShotsLeft)(playerinfo_t *playerinfo, int intent);

#endif	// PLAYER_DLL

// This is the only function actually exported at the linker level.

typedef	player_export_t	(*GetPlayerAPI_t)(void);

#endif	// PLAYER_H
