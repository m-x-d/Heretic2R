//
// g_ClassStatics.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "g_Message.h"
#include "g_local.h"

// If you add or remove classID's here, you MUST adjust the tables in m_stats.c accordingly, as well as the StaticsInit and Precache stuff.
// Search for "NUM_CLASSIDS" if you're not certain what's effected by this...
typedef enum ClassID_e
{
	CID_NONE,

	// Monsters.
	CID_RAT,
	CID_GORGON,
	CID_PLAGUEELF,
	CID_GKROKON,
	CID_FISH,
	CID_OBJECT,
	CID_LIGHT,
	CID_TRIGGER,
	CID_HARPY,
	CID_SPREADER,
	CID_ELFLORD,
	CID_BBRUSH,
	CID_FUNC_ROTATE,
	CID_FUNC_DOOR,
	CID_CHICKEN,
	CID_SSITHRA,
	CID_SPELL,
	CID_MSSITHRA,
	CID_OGLE,
	CID_SERAPH_OVERLORD,
	CID_SERAPH_GUARD,
	CID_ASSASSIN,
	CID_TELEPORTER,
	CID_HIGHPRIESTESS,
	CID_TCHECKRIK,
	CID_BUTTON,
	CID_BEE,
	CID_CORVUS,
	CID_MORK,
	CID_TBEAST,
	CID_IMP,
	CID_LEVER,
	CID_FLAMETHROWER,

	// Cinematics / other...
	CID_MOTHER,
	CID_SSITHRA_VICTIM,
	CID_SSITHRA_SCOUT,
	CID_DRANOR,
	CID_TRIG_DAMAGE,
	CID_TRIG_PUSH,
	CID_C_ELFLORD,
	CID_C_SIERNAN1,
	CID_C_SIERNAN2,
	CID_C_HIGHPRIESTESS,
	CID_C_HIGHPRIESTESS2,
	CID_C_TOME,
	CID_C_MORCALAVIN,
	CID_CORVUS2,
	CID_CORVUS3,
	CID_CORVUS4,
	CID_CORVUS5,
	CID_CORVUS6,
	CID_CORVUS7,
	CID_CORVUS8,
	CID_CORVUS9,

	NUM_CLASSIDS
} ClassID_t;

#define NUM_ATTACK_RANGES	(NUM_CLASSIDS * 4)

typedef struct ClassResourceInfo_s
{
	int numAnims;
	const animmove_t** animations;
	int modelIndex;
	int numSounds;
	int* sounds;

	int (*NextAnimForStep)(edict_t* self, float dir, int step);
	float (*DistMovedForAnim)(int animID);
	float (*SpeedForAnim)(int animID);
	int (*NextAnimForPivot)(edict_t* self, float facingDelta);
	float (*AnglePivotedForAnim)(int animID); // Passing in -1 will return the smallest angle pivoted.
} ClassResourceInfo_t;

#define MINIMUM_PIVOT_ANIM	(-1)

typedef struct ClassMoveInfo_s
{
	float stepHeight; // Maximum distance something can step up or down in a frame.
	float dropHeight; // Maximum distance something can drop off a ledge.

	void (*SetForStep)(edict_t* self, float stepDir, int step);
	void (*SetForPivot)(edict_t* self, float facingDelta);
	void (*SetForStop)(edict_t* self);
} ClassMoveInfo_t;

typedef struct ClassActionInfo_s
{
	void (*SetForStrike)(edict_t* self, int weaponID);
	void (*SetForShoot)(edict_t* self, int weaponID);
} ClassActionInfo_t;

typedef struct G_ClassStatics_s
{
	G_MsgReceiver_t msgReceivers[NUM_MESSAGES];
	void (*update)(edict_t* self);

	ClassResourceInfo_t* resInfo;
	ClassMoveInfo_t* moveInfo;
	ClassActionInfo_t* actionInfo;

	void (*SetForDamage)(edict_t* self, edict_t* inflictor, edict_t* attacker, int damage, int hitLocation, int flags);
	void (*SetForDeath)(edict_t* self, edict_t* inflictor, edict_t* attacker, int damage, int hitLocation, int flags);
	void (*SetForKnockback)(edict_t* self, vec3_t kvel, int flags);
	void (*EndKnockback)(edict_t* self);
	void (*DieAI)(edict_t* self);
} G_ClassStatics_t;

extern G_ClassStatics_t classStatics[NUM_CLASSIDS];
extern qboolean classStaticsInitialized[NUM_CLASSIDS]; //mxd. int Cid_init[] in original version.
extern void (*classStaticsInits[NUM_CLASSIDS])(void);