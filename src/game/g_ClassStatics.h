#ifndef G_CLASSSTATICS_H
#define G_CLASSSTATICS_H

#include "g_Message.h"
#include "g_local.h"

// ************************************************************************************************
// ClassID_t
// ************************************************************************************************

/*==========================================================================

  IMPORTANT!!!

  If you add or remove classID's here, you MUST adjust the tables in
  m_stats.c accordingly, as well as the StaticsInit and Precache stuff.
  Search for "NUM_CLASSIDS" if you're not certain what's effected by this...

  ==========================================================================*/
typedef enum ClassID_e
{
	CID_NONE = 0,		//changed this so that CID_NONE is default
	CID_RAT,			//1
	CID_GORGON,			//2
	CID_PLAGUEELF,		//3
	CID_GKROKON,		//4
	CID_FISH,			//5
	CID_OBJECT,			//6
	CID_LIGHT,			//7
	CID_TRIGGER,		//8
	CID_HARPY,			//9
	CID_SPREADER,		//10
	CID_ELFLORD,		//11
	CID_BBRUSH,			//12
	CID_FUNC_ROTATE,	//13
	CID_FUNC_DOOR,		//14
	CID_CHICKEN,		//15
	CID_SSITHRA,		//16
	CID_SPELL,			//17
	CID_MSSITHRA,		//18
	CID_OGLE,			//19
	CID_SERAPH_OVERLORD,//20
	CID_SERAPH_GUARD,	//21
	CID_ASSASSIN,		//22
	CID_TELEPORTER,		//23
	CID_HIGHPRIESTESS,	//24
	CID_TCHECKRIK,		//25
	CID_BUTTON,			//26
	CID_BEE,			//27
	CID_CORVUS,			//28
	CID_MORK,			//29
	CID_TBEAST,			//30
	CID_IMP,			//31
	CID_LEVER,			//32
	CID_FLAMETHROWER,	//33
//Monsters ^

//Cinematics/other...
	CID_MOTHER,			//34
	CID_SSITHRA_VICTIM,	//35
	CID_SSITHRA_SCOUT,	//36
	CID_DRANOR,			//37
	CID_TRIG_DAMAGE,	//38
	CID_TRIG_PUSH,		//39
	CID_C_ELFLORD,		//40
	CID_C_SIERNAN1,		//41
	CID_C_SIERNAN2,		//42
	CID_C_HIGHPRIESTESS,//43
	CID_C_HIGHPRIESTESS2,//44
	CID_C_TOME,			//45
	CID_C_MORCALAVIN,	//46
	CID_CORVUS2,		//47
	CID_CORVUS3,		//48
	CID_CORVUS4,		//49
	CID_CORVUS5,		//50
	CID_CORVUS6,		//51
	CID_CORVUS7,		//52
	CID_CORVUS8,		//53
	CID_CORVUS9,		//54
	NUM_CLASSIDS		//55
} ClassID_t;

#define NUM_ATTACKRANGES NUM_CLASSIDS *4
// ************************************************************************************************
// ClassResourceInfo_s
// ************************************************************************************************

typedef struct ClassResourceInfo_s
{
	int		numAnims;
	animmove_t **animations;
	int		modelIndex;
	int		numSounds;
	int		*sounds;
	int		(*NextAnimForStep)(edict_t *self, float dir, int step);
	float	(*DistMovedForAnim)(int animID);
	float	(*SpeedForAnim)(int animID);
	int		(*NextAnimForPivot)(edict_t *self, float facingDelta);
	float	(*AnglePivotedForAnim)(int animID);						// passing in -1 will return the smallest angle pivoted
} ClassResourceInfo_t;

#define MINIMUM_PIVOT_ANIM -1

// ************************************************************************************************
// ClassMoveInfo_s
// ************************************************************************************************

typedef struct ClassMoveInfo_s
{
	float	stepHeight;		// max dist something step up or down in a frame
	float	dropHeight;		// max dist something will drop off a ledge
	void	(*SetForStep)(edict_t *self, float stepDir, int step);
	void	(*SetForPivot)(edict_t *self, float facingDelta);
	void	(*SetForStop)(edict_t *self);
} ClassMoveInfo_t;

// ************************************************************************************************
// ClassMoveInfo_s
// ************************************************************************************************

typedef struct ClassActionInfo_s
{
	void	(*SetForStrike)(edict_t *self, int weaponID);
	void	(*SetForShoot)(edict_t *self, int weaponID);
} ClassActionInfo_t;

// ************************************************************************************************
// G_ClassStatics_t
// ************************************************************************************************

typedef struct G_ClassStatics_s
{
	G_MsgReceiver_t		msgReceivers[NUM_MESSAGES];
	void				(*update)(edict_t *self);
	ClassResourceInfo_t *resInfo;
	ClassMoveInfo_t		*moveInfo;
	ClassActionInfo_t	*actionInfo;
	void				(*SetForDamage)(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, int hitLocation, int flags);
	void				(*SetForDeath)(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, int hitLocation, int flags);
	void				(*SetForKnockback)(edict_t *self, vec3_t kvel, int flags);
	void				(*EndKnockback)(edict_t *self);
	void				(*DieAI)(edict_t *self);
} G_ClassStatics_t;

extern G_ClassStatics_t	classStatics[NUM_CLASSIDS];
extern int				Cid_init[NUM_CLASSIDS];
extern void				(*classStaticsInits[NUM_CLASSIDS])();

#endif