//
// p_anim_data.c
//
// Copyright 1998 Raven Software
//

#include "Player.h"
#include "p_actions.h"
#include "p_anim_branch.h"
#include "p_anim_data.h"
#include "p_anims.h"
#include "p_chicken.h"
#include "p_main.h"
#include "m_player.h"

//NOTE: This value is applied generically to all player move functions that travel along 
//		the ground to defeat the friction addition in the new system.
#define PHYS_SCALER	1.4f //mxd. Named 'UNJH_VALUE' in original version.

#pragma region ========================== Animation sequence names ==========================

char* SeqNames[ASEQ_MAX] = //TODO: unused.
{
	"NONE",
	"WSWORD_STD1",
	"WSWORD_STD2",
	"WSWORD_STEP2",
	"WSWORD_STEP",
	"WSWORD_BACK",
	"WSWORD_DOWNSTAB",
	"WSWORD_STABHOLD",
	"WSWORD_PULLOUT",
	"WSWORD_BLOCK_L",
	"WSWORD_BLOCK2_L",
	"WSWORD_BLOCKATK_L",
	"WSWORD_BLOCK_R",
	"WSWORD_BLOCK2_R",
	"WSWORD_BLOCKATK_R",
	"WSWORD_BLOCKED_L",
	"WSWORD_BLOCKED_R",
	"WFIREBALL",
	"WARRAY",
	"WSPHERE_GO",
	"WSPHERE_HOLD",
	"WSPHERE_FIRE1",
	"WSPHERE_FIRE2",
	"WSPHERE_FIRE3",
	"WSPHERE_FIRE4",
	"WFIREWALL",
	"WRIPPER",
	"WBIGBALL",
	"WBLAST",
	"WRRBOW_GO",
	"WRRBOW_DRAW",
	"WRRBOW_HOLD",
	"WRRBOW_FIRE",
	"WRRBOW_READY",
	"WRRBOW_END",
	"WPHBOW_GO",
	"WPHBOW_DRAW",
	"WPHBOW_HOLD",
	"WPHBOW_FIRE",
	"WPHBOW_READY",
	"WPHBOW_END",
	"WHELL_GO",
	"WHELL_FIRE1",
	"WHELL_FIRE2",
	"WHELL_END",
	"SPELL_DEF",
	"HAND2HAND",
	"HAND2SWD",
	"HAND2HELL",
	"HAND2BOW",
	"SWD2HAND",
	"SWD2HELL",
	"SWD2BOW",
	"HELL2HAND",
	"HELL2SWD",
	"HELL2BOW",
	"BOW2HAND",
	"BOW2SWD",
	"BOW2HELL",
	"BOW2BOW",
	"PUSHBUTTON_GO",
	"PUSHLEVERLEFT",
	"PUSHLEVERRIGHT",

	"LOWER_BASE",	// LOWER FRAMES.
	"STAND",
	"PIVOTL_GO",
	"PIVOTL",
	"PIVOTL_END",
	"PIVOTR_GO",
	"PIVOTR",
	"PIVOTR_END",
	"TURN180",
	"RUNF_GO",
	"RUNF",
	"RUNF_END",
	"WALKF_GO",
	"WALKF",
	"WALKF_END",
	"CREEPF",
	"CREEPF_END",
	"WALKB",
	"CREEPB",
	"CREEPB_END",
	"CROUCH_GO",
	"CROUCH",
	"CROUCH_END",
	"CROUCH_PIVOTL",
	"CROUCH_PIVOTR",
	"STRAFEL",
	"STRAFEL_END",
	"STRAFER",
	"STRAFER_END",
	"JUMPSTD_GO",
	"JUMPFWD_SGO",
	"JUMPFWD_WGO",
	"JUMPFWD_RGO",
	"JUMPFWD",
	"JUMPUP",
	"JUMPUP_LOOP", //BUGFIX: no comma after value in original version.
	"JUMPFLIPL",
	"JUMPFLIPR",
	"JUMPSPRINGBGO",
	"JUMPSPRINGB",
	"JUMPFLIPB",
	"ROLLDIVEF_W",
	"ROLLDIVEF_R",
	"ROLL_FROM_FFLIP",
	"POLEVAULT1_W",
	"POLEVAULT1_R",
	"POLEVAULT2",
	"LANDLIGHT",
	"LANDHEAVY",
	"FALLWALK_GO",
	"FALLRUN_GO",
	"FALL",
	"FALLARMSUP",
	"VAULT_LOW",
	"VAULT_HIGH",
	"PULLUP_WALL",
	"SSWIM_IDLE",
	"SSWIMF_GO",
	"SSWIMF",
	"SSWIMF_END",
	"SSWIMB_GO",
	"SSWIMB",
	"SSWIMB_END",
	"SSWIML_GO",
	"SSWIMR_GO",
	"SSWIML",
	"SSWIMR",
	"SSWIML_END",
	"SSWIMR_END",
	"WSWORD_SPIN",
	"WSWORD_SPIN2",
	"WSWORD_SPINBLOCKED",
	"WSWORD_SPINBLOCKED2",
	"WSWORD_LOWERDOWNSTAB",
	"WSWORD_LOWERPULLOUT",
	"PULLUP_HALFWALL",
	"TUMBLEON1",
	"TUMBLEON2",
	"LSTAIR4",
	"LSTAIR8",
	"LSTAIR12",
	"LSTAIR16",
	"RSTAIR4",
	"RSTAIR8",
	"RSTAIR12",
	"RSTAIR16",
	"IDLE_READY_GO",
	"IDLE_READY",
	"IDLE_READY_END",
	"IDLE_LOOKL",
	"IDLE_LOOKR",
	"PAIN_A",
	"PAIN_B",
	"IDLE_FLY1",
	"IDLE_FLY2",
	"FALLL",
	"FALLR",
	"FALLL_END",
	"FALLR_END",
	"DEATH_A",
	"USWIMF_GO",
	"USWIMF",
	"USWIMF_END",
	"DIVE",
	"USWIMB_GO",
	"USWIMB",
	"USWIMB_END",
	"USWIML_GO",
	"USWIMR_GO",
	"USWIML",
	"USWIMR",
	"USWIML_END",
	"USWIMR_END",
	"SLIDE_FORWARD",
	"SLIDE_BACKWARD",
	"SSWIM_RESURFACE",
	"ROLL_L",
	"ROLL_R",
	"USWIM_IDLE",
	"ROLL_B",
	"CLIMB_ON",
	"CLIMB_UP_START_R",
	"CLIMB_UP_START_L",
	"CLIMB_UP_R",
	"CLIMB_UP_L",
	"CLIMB_DOWN_START_R",
	"CLIMB_DOWN_START_L",
	"CLIMB_DOWN_R",
	"CLIMB_DOWN_L",
	"CLIMB_OFF",
	"CLIMB_HOLD_R",
	"CLIMB_HOLD_L",
	"CLIMB_SETTLE_R",
	"CLIMB_SETTLE_L",
	"KNOCKDOWN",
	"KNOCKDOWN_GETUP",
	"KNOCKDOWN_EVADE",
	"SHRINE",
	"TAKEPUZZLEPIECE",
	"TAKEPUZZLEUNDERWATER",
	"DROWN",
	"FORWARD_FLIP_L_GO",
	"FORWARD_FLIP_R_GO",
	"FORWARD_FLIP_L",
	"FORWARD_FLIP_R",
	"CSTRAFE_LEFT",
	"CSTRAFE_RIGHT",
	"WSTRAFE_LEFT",
	"WSTRAFE_RIGHT",
	"RSTRAFE_LEFT",
	"RSTRAFE_RIGHT",
	"JUMPBACK_SGO",
	"JUMPBACK_WGO",
	"JUMPBACK_RGO",
	"JUMPBACK",
	"JUMPFLIPBACK",
	"JUMPLEFT_SGO",
	"JUMPLEFT_WGO",
	"JUMPLEFT_RGO",
	"JUMPLEFT",
	"JUMPFLIPLEFT",
	"JUMPRIGHT_SGO",
	"JUMPRIGHT_WGO",
	"JUMPRIGHT_RGO",
	"JUMPRIGHT",
	"JUMPFLIPRIGHT",
	"DROWN_IDLE",
	"DASH_LEFT_GO",
	"DASH_LEFT",
	"DASH_RIGHT_GO",
	"DASH_RIGHT",
	"CSTRAFEB_LEFT",
	"CSTRAFEB_RIGHT",
	"WSTRAFEB_LEFT",
	"WSTRAFEB_RIGHT",
	"OVERHANG",
	"DEATH_B",
	"DEATH_FLYFWD",
	"DEATH_FLYBACK",
	"DEATH_CHOKE",
	"IDLE_LOOKBACK",
	"IDLE_SCRATCH_ASS",
	"IDLE_WIPE_BROW",
	"CROUCH_WALK_F",
	"CROUCH_WALK_B",
	"CROUCH_WALK_L",
	"CROUCH_WALK_R",
	"SSWIM_FAST_GO",
	"SSWIM_FAST",
};

#pragma endregion

#pragma region ========================== Player animation tables ==========================

int PlayerAnimWeaponSwitchSeq[WEAPON_READY_MAX][WEAPON_READY_MAX] =
{
	// NONE			// HANDS		// STAFFSTUB	// SWORD		// HELLSTAFF	// BOW
	{ ASEQ_NONE,	ASEQ_HAND2HAND,	ASEQ_NONE,		ASEQ_HAND2SWD,	ASEQ_HAND2HELL,	ASEQ_HAND2BOW },
	{ ASEQ_NONE,	ASEQ_HAND2HAND,	ASEQ_NONE,		ASEQ_HAND2SWD,	ASEQ_HAND2HELL,	ASEQ_HAND2BOW },
	{ ASEQ_NONE,	ASEQ_SWD2HAND,	ASEQ_NONE,		ASEQ_NONE,		ASEQ_SWD2HELL,	ASEQ_SWD2BOW },
	{ ASEQ_NONE,	ASEQ_SWD2HAND,	ASEQ_NONE,		ASEQ_NONE,		ASEQ_SWD2HELL,	ASEQ_SWD2BOW },
	{ ASEQ_NONE,	ASEQ_HELL2HAND,	ASEQ_NONE,		ASEQ_HELL2SWD,	ASEQ_NONE,		ASEQ_HELL2BOW },
	{ ASEQ_NONE,	ASEQ_BOW2HAND,	ASEQ_NONE,		ASEQ_BOW2SWD,	ASEQ_BOW2HELL,	ASEQ_BOW2BOW },
};

seqctrl_t SeqCtrl[ASEQ_MAX] =
{
	{ ACMD_NONE,		ASEQ_NONE,					ASEQ_NONE,				BranchLwrStanding },		// ASEQ_NONE

	// UPPER SEQENCES
	// ACMD,			CONTSEQ,					CEASESEQ,				BRANCHFUNC
	{ ACMDU_ATTACK,		ASEQ_WSWORD_STEP2,			ASEQ_WSWORD_STD2,		NULL },						// ASEQ_WSWORD_STD1,
	{ ACMDU_ATTACK,		ASEQ_WSWORD_STEP,			ASEQ_NONE,				BranchUprReady },			// ASEQ_WSWORD_STD2,
	{ ACMDU_ATTACK,		ASEQ_WSWORD_STD1,			ASEQ_NONE,				BranchUprReady },			// ASEQ_WSWORD_STEP2,
	{ ACMDU_ATTACK,		ASEQ_WSWORD_STD1,			ASEQ_NONE,				BranchUprReady },			// ASEQ_WSWORD_STEP,
	{ ACMD_NONE,		ASEQ_NONE,					ASEQ_NONE,				BranchUprReady },			// ASEQ_WSWORD_BACK,
	{ ACMD_NONE,		ASEQ_WSWORD_STABHOLD,		ASEQ_WSWORD_STABHOLD,	NULL },						// ASEQ_WSWORD_DOWNSTAB,
	{ ACMDU_ATTACK,		ASEQ_WSWORD_STABHOLD,		ASEQ_WSWORD_PULLOUT,	NULL },						// ASEQ_WSWORD_STABHOLD,
	{ ACMD_NONE,		ASEQ_NONE,					ASEQ_NONE,				BranchUprReady },			// ASEQ_WSWORD_PULLOUT,
	{ ACMDU_ATTACK,		ASEQ_WSWORD_BLOCKATK_L,		ASEQ_WSWORD_BLOCK2_L,	NULL },						// ASEQ_WSWORD_BLOCK_L,
	{ ACMD_NONE,		ASEQ_NONE,					ASEQ_NONE,				BranchUprReady },			// ASEQ_WSWORD_BLOCK2_L,
	{ ACMD_NONE,		ASEQ_NONE,					ASEQ_NONE,				BranchUprReady },			// ASEQ_WSWORD_BLOCKATK_L,
	{ ACMDU_ATTACK,		ASEQ_WSWORD_BLOCKATK_R,		ASEQ_WSWORD_BLOCK2_R,	NULL },						// ASEQ_WSWORD_BLOCK_R,
	{ ACMD_NONE,		ASEQ_NONE,					ASEQ_NONE,				BranchUprReady },			// ASEQ_WSWORD_BLOCK2_R,
	{ ACMD_NONE,		ASEQ_NONE,					ASEQ_NONE,				BranchUprReady },			// ASEQ_WSWORD_BLOCKATK_R,
	{ ACMD_NONE,		ASEQ_NONE,					ASEQ_NONE,				BranchUprReady },			// ASEQ_WSWORD_BLOCKED_L,
	{ ACMD_NONE,		ASEQ_NONE,					ASEQ_NONE,				BranchUprReady },			// ASEQ_WSWORD_BLOCKED_R,
	{ ACMD_NONE,		ASEQ_NONE,					ASEQ_NONE,				BranchUprReady },			// ASEQ_WFIREBALL,
	{ ACMD_NONE,		ASEQ_NONE,					ASEQ_NONE,				BranchCheckMana },			// ASEQ_WARRAY,
	{ ACMDU_ATTACK,		ASEQ_WSPHERE_HOLD,			ASEQ_WSPHERE_FIRE4,		NULL },						// ASEQ_WSPHERE_GO,
	{ ACMDU_ATTACK,		ASEQ_WSPHERE_HOLD,			ASEQ_WSPHERE_FIRE4,		NULL },						// ASEQ_WSPHERE_HOLD,
	{ ACMD_NONE,		ASEQ_NONE,					ASEQ_NONE,				BranchCheckMana },			// ASEQ_WSPHERE_FIRE1,
	{ ACMD_NONE,		ASEQ_NONE,					ASEQ_NONE,				BranchCheckMana },			// ASEQ_WSPHERE_FIRE2,
	{ ACMD_NONE,		ASEQ_NONE,					ASEQ_NONE,				BranchCheckMana },			// ASEQ_WSPHERE_FIRE3,
	{ ACMD_NONE,		ASEQ_NONE,					ASEQ_NONE,				BranchCheckMana },			// ASEQ_WSPHERE_FIRE4,
	{ ACMD_NONE,		ASEQ_NONE,					ASEQ_NONE,				BranchCheckMana },			// ASEQ_WFIREWALL,
	{ ACMD_NONE,		ASEQ_NONE,					ASEQ_NONE,				BranchCheckMana },			// ASEQ_WRIPPER,
	{ ACMD_NONE,		ASEQ_NONE,					ASEQ_NONE,				BranchCheckMana },			// ASEQ_WBIGBALL,
	{ ACMD_NONE,		ASEQ_NONE,					ASEQ_NONE,				BranchCheckMana },			// ASEQ_WBLAST,

	{ ACMDU_ATTACK,		ASEQ_WRRBOW_DRAW,			ASEQ_WRRBOW_DRAW,		NULL },						// ASEQ_WRRBOW_GO,
	{ ACMDU_ATTACK,		ASEQ_WRRBOW_HOLD,			ASEQ_WRRBOW_FIRE,		NULL },						// ASEQ_WRRBOW_DRAW,
	{ ACMDU_ATTACK,		ASEQ_WRRBOW_HOLD,			ASEQ_WRRBOW_FIRE,		NULL },						// ASEQ_WRRBOW_HOLD,
	{ ACMDU_ATTACK,		ASEQ_WRRBOW_DRAW,			ASEQ_WRRBOW_READY,		BranchCheckBowAmmo },		// ASEQ_WRRBOW_FIRE,
	{ ACMDU_ATTACK,		ASEQ_WRRBOW_DRAW,			ASEQ_WRRBOW_END,		NULL },						// ASEQ_WRRBOW_READY,
	{ ACMDU_ATTACK,		ASEQ_WRRBOW_GO,				ASEQ_NONE,				BranchUprReady },			// ASEQ_WRRBOW_END,

	{ ACMDU_ATTACK,		ASEQ_WPHBOW_DRAW,			ASEQ_WPHBOW_DRAW,		NULL },						// ASEQ_WPHBOW_GO,
	{ ACMDU_ATTACK,		ASEQ_WPHBOW_HOLD,			ASEQ_WPHBOW_FIRE,		NULL },						// ASEQ_WPHBOW_DRAW,
	{ ACMDU_ATTACK,		ASEQ_WPHBOW_HOLD,			ASEQ_WPHBOW_FIRE,		NULL },						// ASEQ_WPHBOW_HOLD,
	{ ACMDU_ATTACK,		ASEQ_WPHBOW_DRAW,			ASEQ_WPHBOW_READY,		BranchCheckBowAmmo },		// ASEQ_WPHBOW_FIRE,
	{ ACMDU_ATTACK,		ASEQ_WPHBOW_DRAW,			ASEQ_WPHBOW_END,		NULL },						// ASEQ_WPHBOW_READY,
	{ ACMDU_ATTACK,		ASEQ_WPHBOW_GO,				ASEQ_NONE,				BranchUprReady },			// ASEQ_WPHBOW_END,

	{ ACMDU_ATTACK,		ASEQ_WHELL_FIRE1,			ASEQ_WHELL_END,			NULL },						// ASEQ_WHELL_GO,
	{ ACMDU_ATTACK,		ASEQ_WHELL_FIRE2,			ASEQ_WHELL_END,			BranchCheckHellAmmo },		// ASEQ_WHELL_FIRE1,
	{ ACMDU_ATTACK,		ASEQ_WHELL_FIRE1,			ASEQ_WHELL_END,			BranchCheckHellAmmo },		// ASEQ_WHELL_FIRE2,
	{ ACMDU_ATTACK,		ASEQ_WHELL_GO,				ASEQ_NONE,				BranchUprReady },			// ASEQ_WHELL_END,
	{ ACMD_NONE,		ASEQ_NONE,					ASEQ_NONE,				BranchUprReady },			// ASEQ_SPELL_DEF
	{ ACMD_NONE,		ASEQ_NONE,					ASEQ_NONE,				BranchUprReady },			// ASEQ_HAND2HAND
	{ ACMD_NONE,		ASEQ_NONE,					ASEQ_NONE,				BranchUprReady },			// ASEQ_HAND2SWD,
	{ ACMD_NONE,		ASEQ_NONE,					ASEQ_NONE,				BranchUprReady },			// ASEQ_HAND2HELL,
	{ ACMD_NONE,		ASEQ_NONE,					ASEQ_NONE,				BranchUprReady },			// ASEQ_HAND2BOW,
	{ ACMD_NONE,		ASEQ_NONE,					ASEQ_NONE,				BranchUprReady },			// ASEQ_SWD2HAND,
	{ ACMD_NONE,		ASEQ_NONE,					ASEQ_NONE,				BranchUprReady },			// ASEQ_SWD2HELL,
	{ ACMD_NONE,		ASEQ_NONE,					ASEQ_NONE,				BranchUprReady },			// ASEQ_SWD2BOW,
	{ ACMD_NONE,		ASEQ_NONE,					ASEQ_NONE,				BranchUprReady },			// ASEQ_HELL2HAND,
	{ ACMD_NONE,		ASEQ_NONE,					ASEQ_NONE,				BranchUprReady },			// ASEQ_HELL2SWD,
	{ ACMD_NONE,		ASEQ_NONE,					ASEQ_NONE,				BranchUprReady },			// ASEQ_HELL2BOW,
	{ ACMD_NONE,		ASEQ_NONE,					ASEQ_NONE,				BranchUprReady },			// ASEQ_BOW2HAND,
	{ ACMD_NONE,		ASEQ_NONE,					ASEQ_NONE,				BranchUprReady },			// ASEQ_BOW2SWD,
	{ ACMD_NONE,		ASEQ_NONE,					ASEQ_NONE,				BranchUprReady },			// ASEQ_BOW2HELL,
	{ ACMD_NONE,		ASEQ_NONE,					ASEQ_NONE,				BranchUprReady },			// ASEQ_BOW2BOW,
	{ ACMD_NONE,		ASEQ_NONE,					ASEQ_STAND,				NULL },						// ASEQ_PUSHBUTTON_GO,
	{ ACMD_NONE,		ASEQ_NONE,					ASEQ_STAND,				NULL },						// ASEQ_PUSHBLEVERLEFT,
	{ ACMD_NONE,		ASEQ_NONE,					ASEQ_STAND,				NULL },						// ASEQ_PUSHBLEVERRIGHT,

	// LOWER SEQUENCES
	{ ACMD_NONE,		ASEQ_NONE,					ASEQ_NONE,				NULL },						// ASEQ_LOWER_BASE

	// ACMD,			CONTSEQ,					CEASESEQ,				BRANCHFUNC
	{ ACMD_NONE,		ASEQ_STAND,					ASEQ_STAND,				BranchLwrStanding },		// ASEQ_STAND
	{ ACMDL_ROTATE_L,	ASEQ_PIVOTL,				ASEQ_PIVOTL,			BranchLwrStanding },		// ASEQ_PIVOTL_GO
	{ ACMDL_ROTATE_L,	ASEQ_STAND,					ASEQ_STAND,				BranchLwrStanding },		// ASEQ_PIVOTL
	{ ACMDL_ROTATE_L,	ASEQ_PIVOTL,				ASEQ_STAND,				BranchLwrStanding },		// ASEQ_PIVOTL_END
	{ ACMDL_ROTATE_R,	ASEQ_PIVOTR,				ASEQ_PIVOTR,			BranchLwrStanding },		// ASEQ_PIVOTR_GO
	{ ACMDL_ROTATE_R,	ASEQ_STAND,					ASEQ_STAND,				BranchLwrStanding },		// ASEQ_PIVOTR
	{ ACMDL_ROTATE_R,	ASEQ_PIVOTR,				ASEQ_STAND,				BranchLwrStanding },		// ASEQ_PIVOTR_END
	{ ACMDL_QUICKTURN,	ASEQ_TURN180,				ASEQ_STAND,				BranchLwrStanding },		// ASEQ_TURN180
	{ ACMDL_RUN_F,		ASEQ_RUNF,					ASEQ_STAND,				BranchLwrRunning },			// ASEQ_RUNF_GO
	{ ACMDL_RUN_F,		ASEQ_RUNF,					ASEQ_STAND,				BranchLwrRunning },			// ASEQ_RUNF
	{ ACMDL_RUN_F,		ASEQ_RUNF_GO,				ASEQ_STAND,				BranchLwrStanding },		// ASEQ_RUNF_END
	{ ACMDL_WALK_F,		ASEQ_WALKF,					ASEQ_STAND,				BranchLwrWalking },			// ASEQ_WALKF_GO
	{ ACMDL_WALK_F,		ASEQ_WALKF,					ASEQ_STAND,				BranchLwrWalking },			// ASEQ_WALKF
	{ ACMDL_WALK_F,		ASEQ_WALKF_GO,				ASEQ_STAND,				BranchLwrStanding },		// ASEQ_WALKF_END //obsolete
	{ ACMDL_CREEP_F,	ASEQ_CREEPF,				ASEQ_CREEPF_END,		BranchLwrShortstep },		// ASEQ_CREEPF
	{ ACMDL_CREEP_F,	ASEQ_CREEPF,				ASEQ_STAND,				BranchLwrShortstep },		// ASEQ_CREEPF_END
	{ ACMDL_BACK,		ASEQ_WALKB,					ASEQ_STAND,				BranchLwrWalking },			// ASEQ_WALKB
	{ ACMDL_CREEP_B,	ASEQ_CREEPB,				ASEQ_CREEPB_END,		BranchLwrShortstep  },		// ASEQ_CREEPB
	{ ACMDL_CREEP_B,	ASEQ_CREEPB,				ASEQ_STAND,				BranchLwrShortstep  },		// ASEQ_CREEPB_END
	{ ACMDL_CROUCH,		ASEQ_CROUCH,				ASEQ_CROUCH_END,		BranchLwrCrouching },		// ASEQ_CROUCH_GO
	{ ACMDL_CROUCH,		ASEQ_CROUCH,				ASEQ_CROUCH_END,		BranchLwrCrouching },		// ASEQ_CROUCH
	{ ACMD_NONE,		ASEQ_STAND,					ASEQ_STAND,				BranchLwrStanding },		// ASEQ_CROUCH_END
	{ ACMDL_CROUCH,		ASEQ_CROUCH_PIVOTL,			ASEQ_CROUCH_END,		BranchLwrCrouching },		// ASEQ_CROUCH_PIVOTL
	{ ACMDL_CROUCH,		ASEQ_CROUCH_PIVOTR,			ASEQ_CROUCH_END,		BranchLwrCrouching },		// ASEQ_CROUCH_PIVOTR
	{ ACMDL_STRAFE_L,	ASEQ_STRAFEL,				ASEQ_STRAFEL_END,		BranchLwrStanding },		// ASEQ_STRAFEL
	{ ACMD_NONE,		ASEQ_STAND,					ASEQ_STAND,				BranchLwrStanding },		// ASEQ_STRAFEL_END
	{ ACMDL_STRAFE_R,	ASEQ_STRAFER,				ASEQ_STRAFER_END,		BranchLwrStanding },		// ASEQ_STRAFER1
	{ ACMD_NONE,		ASEQ_STAND,					ASEQ_STAND,				BranchLwrStanding },		// ASEQ_STRAFER_END
	{ ACMD_NONE,		ASEQ_JUMPUP,				ASEQ_JUMPUP,			BranchLwrJumping },			// ASEQ_JUMPSTD_GO
	{ ACMD_NONE,		ASEQ_JUMPFWD,				ASEQ_JUMPFWD,			NULL },						// ASEQ_JUMPFWD_SGO
	{ ACMD_NONE,		ASEQ_JUMPFWD,				ASEQ_JUMPFWD,			NULL },						// ASEQ_JUMPFWD_WGO
	{ ACMD_NONE,		ASEQ_JUMPFWD,				ASEQ_JUMPFWD,			NULL },						// ASEQ_JUMPFWD_RGO
	{ ACMD_NONE,		ASEQ_FALL,					ASEQ_FALL,				NULL },						// ASEQ_JUMPFWD
	{ ACMD_NONE,		ASEQ_JUMPUP_LOOP,			ASEQ_JUMPUP_LOOP,		NULL },						// ASEQ_JUMPUP
	{ ACMD_NONE,		ASEQ_JUMPUP_LOOP,			ASEQ_JUMPUP_LOOP,		NULL },						// ASEQ_JUMPUP_LOOP
	{ ACMD_NONE,		ASEQ_FALL,					ASEQ_FALL,				NULL },						// ASEQ_JUMPFLIPL
	{ ACMD_NONE,		ASEQ_FALL,					ASEQ_FALL,				NULL },						// ASEQ_JUMPFLIPR
	{ ACMD_NONE,		ASEQ_JUMPSPRINGB,			ASEQ_JUMPSPRINGB,		NULL },						// ASEQ_JUMPSPRINGBGO
	{ ACMD_NONE,		ASEQ_LANDLIGHT,				ASEQ_LANDLIGHT,			BranchLwrBackspring },		// ASEQ_JUMPSPRINGB
	{ ACMD_NONE,		ASEQ_FALL,					ASEQ_FALL,				NULL },						// ASEQ_JUMPFLIPB
	{ ACMDL_CROUCH,		ASEQ_CROUCH,				ASEQ_CROUCH_END,		BranchLwrCrouching },		// ASEQ_ROLLDIVEF_W
	{ ACMDL_CROUCH,		ASEQ_CROUCH,				ASEQ_CROUCH_END,		BranchLwrCrouching },		// ASEQ_ROLLDIVEF_R
	{ ACMDL_CROUCH,		ASEQ_CROUCH,				ASEQ_CROUCH_END,		BranchLwrCrouching },		// ASEQ_ROLL_FROM_FFLIP
	{ ACMD_NONE,		ASEQ_POLEVAULT2,			ASEQ_POLEVAULT2,		NULL, },					// ASEQ_POLEVAULT1_W
	{ ACMD_NONE,		ASEQ_POLEVAULT2,			ASEQ_POLEVAULT2,		NULL, },					// ASEQ_POLEVAULT1_R
	{ ACMD_NONE,		ASEQ_FALL,					ASEQ_FALL,				NULL, },					// ASEQ_POLEVAULT2
	{ ACMD_NONE,		ASEQ_STAND,					ASEQ_STAND,				BranchLwrStanding },		// ASEQ_LANDLIGHT
	{ ACMD_NONE,		ASEQ_STAND,					ASEQ_STAND,				BranchLwrStanding },		// ASEQ_LANDHEAVY
	{ ACMD_NONE,		ASEQ_FALL,					ASEQ_FALL,				NULL },						// ASEQ_FALLWALK_GO
	{ ACMD_NONE,		ASEQ_FALL,					ASEQ_FALL,				NULL },						// ASEQ_FALLRUN_GO
	{ ACMD_NONE,		ASEQ_FALL,					ASEQ_FALL,				NULL },						// ASEQ_FALL
	{ ACMD_NONE,		ASEQ_FALLARMSUP,			ASEQ_FALLARMSUP,		NULL },						// ASEQ_FALLARMSUP
	{ ACMD_NONE,		ASEQ_STAND,					ASEQ_STAND,				BranchLwrStandingRun },		// ASEQ_VAULT_LOW
	{ ACMD_NONE,		ASEQ_NONE,					ASEQ_NONE,				BranchLwrStandingRun },		// ASEQ_VAULT_HIGH
	{ ACMD_NONE,		ASEQ_STAND,					ASEQ_STAND,				BranchLwrStandingRun },		// ASEQ_PULLUP_WALL
	{ ACMD_NONE,		ASEQ_STAND,					ASEQ_STAND,				BranchLwrSurfaceSwim },		// ASEQ_SSWIM_IDLE
	{ ACMDL_FWD,		ASEQ_SSWIMF,				ASEQ_SSWIMF_END,		NULL },						// ASEQ_SSWIMF_GO
	{ ACMDL_FWD,		ASEQ_SSWIMF,				ASEQ_SSWIMF_END,		BranchLwrSurfaceSwim },		// ASEQ_SSWIMF
	{ ACMDL_WALK_F,		ASEQ_SSWIMF_GO,				ASEQ_SSWIM_IDLE,		BranchLwrSurfaceSwim },		// ASEQ_SSWIMF_END
	{ ACMDL_BACK,		ASEQ_SSWIMB,				ASEQ_SSWIMB_END,		NULL },						// ASEQ_SSWIMB_GO
	{ ACMDL_BACK,		ASEQ_SSWIMB,				ASEQ_SSWIMB_END,		BranchLwrSurfaceSwim },		// ASEQ_SSWIMB
	{ ACMDL_WALK_B,		ASEQ_SSWIMB_GO,				ASEQ_SSWIM_IDLE,		BranchLwrSurfaceSwim },		// ASEQ_SSWIMB_END
	{ ACMDL_STRAFE_L,	ASEQ_SSWIML,				ASEQ_SSWIML_END,		BranchLwrSurfaceSwim },		// ASEQ_SSWIML_GO
	{ ACMDL_STRAFE_R,	ASEQ_SSWIMR,				ASEQ_SSWIMR_END,		BranchLwrSurfaceSwim },		// ASEQ_SSWIMR_GO
	{ ACMDL_STRAFE_L,	ASEQ_SSWIML,				ASEQ_SSWIML_END,		BranchLwrSurfaceSwim },		// ASEQ_SSWIML
	{ ACMDL_STRAFE_R,	ASEQ_SSWIMR,				ASEQ_SSWIMR_END,		BranchLwrSurfaceSwim },		// ASEQ_SSWIMR
	{ ACMD_NONE,		ASEQ_SSWIM_IDLE,			ASEQ_SSWIM_IDLE,		BranchLwrSurfaceSwim },		// ASEQ_SSWIML_END
	{ ACMD_NONE,		ASEQ_SSWIM_IDLE,			ASEQ_SSWIM_IDLE,		BranchLwrSurfaceSwim },		// ASEQ_SSWIMR_END
	{ ACMDL_RUN_F,		ASEQ_RUNF,					ASEQ_STAND,				BranchLwrRunning },			// ASEQ_WSWORD_SPIN
	{ ACMDL_RUN_F,		ASEQ_RUNF,					ASEQ_STAND,				BranchLwrRunning },			// ASEQ_WSWORD_SPIN2
	{ ACMD_NONE,		ASEQ_STAND,					ASEQ_STAND,				BranchLwrStanding },		// ASEQ_WSWORD_SPINBLOCKED
	{ ACMD_NONE,		ASEQ_STAND,					ASEQ_STAND,				BranchLwrStanding },		// ASEQ_WSWORD_SPINBLOCKED2
	{ ACMD_NONE,		ASEQ_WSWORD_LOWERPULLOUT,	ASEQ_WSWORD_LOWERPULLOUT,	NULL},					// ASEQ_WSWORD_LOWERDOWNSTAB
	{ ACMD_NONE,		ASEQ_STAND,					ASEQ_STAND,				BranchLwrStanding },		// ASEQ_WSWORD_LOWERPULLOUT
	{ ACMD_NONE,		ASEQ_STAND,					ASEQ_STAND,				BranchLwrStandingRun},		// ASEQ_PULLUP_HALFWALL
	{ ACMD_NONE,		ASEQ_TUMBLEON2,				ASEQ_TUMBLEON2,			NULL },						// ASEQ_TUMBLEON1
	{ ACMD_NONE,		ASEQ_CROUCH,				ASEQ_CROUCH,			BranchLwrCrouching },		// ASEQ_TUMBLEON2
	{ ACMD_NONE,		ASEQ_LSTAIR4,				ASEQ_LSTAIR4,			BranchLwrStanding },		// ASEQ_LSTAIR4
	{ ACMD_NONE,		ASEQ_LSTAIR8,				ASEQ_LSTAIR8,			BranchLwrStanding },		// ASEQ_LSTAIR8
	{ ACMD_NONE,		ASEQ_LSTAIR12,				ASEQ_LSTAIR12,			BranchLwrStanding },		// ASEQ_LSTAIR12
	{ ACMD_NONE,		ASEQ_LSTAIR16,				ASEQ_LSTAIR16,			BranchLwrStanding },		// ASEQ_LSTAIR16
	{ ACMD_NONE,		ASEQ_RSTAIR4,				ASEQ_RSTAIR4,			BranchLwrStanding },		// ASEQ_RSTAIR4
	{ ACMD_NONE,		ASEQ_RSTAIR8,				ASEQ_RSTAIR8,			BranchLwrStanding },		// ASEQ_RSTAIR8
	{ ACMD_NONE,		ASEQ_RSTAIR12,				ASEQ_RSTAIR12,			BranchLwrStanding },		// ASEQ_RSTAIR12
	{ ACMD_NONE,		ASEQ_RSTAIR16,				ASEQ_RSTAIR16,			BranchLwrStanding },		// ASEQ_RSTAIR16
	{ ACMD_NONE,		ASEQ_IDLE_READY,			ASEQ_IDLE_READY,		BranchLwrStanding },		// ASEQ_IDLE_READY_GO
	{ ACMD_NONE,		ASEQ_IDLE_READY,			ASEQ_IDLE_READY,		BranchLwrStanding },		// ASEQ_IDLE_READY
	{ ACMD_NONE,		ASEQ_STAND,					ASEQ_STAND,				BranchLwrStanding },		// ASEQ_IDLE_READY_END
	{ ACMD_NONE,		ASEQ_IDLE_READY,			ASEQ_IDLE_READY,		BranchLwrStanding },		// ASEQ_IDLE_LOOKL
	{ ACMD_NONE,		ASEQ_IDLE_READY,			ASEQ_IDLE_READY,		BranchLwrStanding },		// ASEQ_IDLE_LOOKR
	{ ACMD_NONE,		ASEQ_STAND,					ASEQ_STAND,				BranchLwrStanding },		// ASEQ_PAIN_A
	{ ACMD_NONE,		ASEQ_STAND,					ASEQ_STAND,				BranchLwrStanding },		// ASEQ_PAIN_B
	{ ACMD_NONE,		ASEQ_STAND,					ASEQ_STAND,				BranchLwrStanding },		// ASEQ_IDLE_FLY1
	{ ACMD_NONE,		ASEQ_STAND,					ASEQ_STAND,				BranchLwrStanding },		// ASEQ_IDLE_FLY2
	{ ACMD_NONE,		ASEQ_FALLL,					ASEQ_FALLL_END,			BranchLwrStanding },		// ASEQ_FALLL
	{ ACMD_NONE,		ASEQ_FALLR,					ASEQ_FALLR_END,			BranchLwrStanding },		// ASEQ_FALLR
	{ ACMD_NONE,		ASEQ_FALLL_END,				ASEQ_FALLL_END,			BranchLwrStanding },		// ASEQ_FALLL_END
	{ ACMD_NONE,		ASEQ_FALLR_END,				ASEQ_FALLR_END,			BranchLwrStanding },		// ASEQ_FALLR_END
	{ ACMD_NONE,		ASEQ_DEATH_A,				ASEQ_DEATH_A,			NULL },						// ASEQ_DEATH_A
	{ ACMDL_FWD,		ASEQ_USWIMF,				ASEQ_USWIMF_END,		BranchLwrUnderwaterSwim },	// ASEQ_USWIMF_GO
	{ ACMDL_FWD,		ASEQ_USWIMF,				ASEQ_USWIMF_END,		BranchLwrUnderwaterSwim },	// ASEQ_USWIMF
	{ ACMDL_FWD,		ASEQ_USWIMF_GO,				ASEQ_USWIM_IDLE,		BranchLwrUnderwaterSwim },	// ASEQ_USWIMF_END
	{ ACMD_NONE,		ASEQ_USWIMF,				ASEQ_USWIMF_END,		BranchLwrUnderwaterSwim },	// ASEQ_DIVE
	{ ACMDL_BACK,		ASEQ_USWIMB,				ASEQ_USWIMB_END,		BranchLwrUnderwaterSwim },	// ASEQ_USWIMB_GO
	{ ACMDL_BACK,		ASEQ_USWIMB,				ASEQ_USWIMB_END,		BranchLwrUnderwaterSwim },	// ASEQ_USWIMB
	{ ACMDL_WALK_B,		ASEQ_USWIMB_GO,				ASEQ_USWIM_IDLE,		BranchLwrUnderwaterSwim },	// ASEQ_USWIMB_END
	{ ACMDL_STRAFE_L,	ASEQ_USWIML,				ASEQ_USWIML_END,		BranchLwrUnderwaterSwim },	// ASEQ_USWIML_GO
	{ ACMDL_STRAFE_R,	ASEQ_USWIMR,				ASEQ_USWIMR_END,		BranchLwrUnderwaterSwim },	// ASEQ_USWIMR_GO
	{ ACMDL_STRAFE_L,	ASEQ_USWIML,				ASEQ_USWIML_END,		BranchLwrUnderwaterSwim },	// ASEQ_USWIML
	{ ACMDL_STRAFE_R,	ASEQ_USWIMR,				ASEQ_USWIMR_END,		BranchLwrUnderwaterSwim },	// ASEQ_USWIMR
	{ ACMD_NONE,		ASEQ_USWIM_IDLE,			ASEQ_USWIM_IDLE,		BranchLwrUnderwaterSwim },	// ASEQ_USWIML_END
	{ ACMD_NONE,		ASEQ_USWIM_IDLE,			ASEQ_USWIM_IDLE,		BranchLwrUnderwaterSwim },	// ASEQ_USWIMR_END
	{ ACMD_NONE,		ASEQ_SLIDE_FORWARD,			ASEQ_STAND,				BranchLwrStanding },		// ASEQ_SLIDE_FORWARD
	{ ACMD_NONE,		ASEQ_SLIDE_BACKWARD,		ASEQ_STAND,				BranchLwrStanding },		// ASEQ_SLIDE_BACKWARD
	{ ACMD_NONE,		ASEQ_SSWIM_RESURFACE,		ASEQ_SSWIM_IDLE,		BranchLwrSurfaceSwim },		// ASEQ_RESURFACE
	{ ACMDL_STRAFE_L,	ASEQ_ROLL_L,				ASEQ_CROUCH,			BranchLwrCrouching },		// ASEQ_ROLL_L
	{ ACMDL_STRAFE_R,	ASEQ_ROLL_R,				ASEQ_CROUCH,			BranchLwrCrouching },		// ASEQ_ROLL_R
	{ ACMD_NONE,		ASEQ_STAND,					ASEQ_STAND,				BranchLwrUnderwaterSwim },	// ASEQ_USWIM_IDLE
	{ ACMDL_BACK,		ASEQ_ROLL_B,				ASEQ_CROUCH,			BranchLwrCrouching },		// ASEQ_ROLL_B
	{ ACMD_NONE,		ASEQ_STAND,					ASEQ_STAND,				BranchLwrClimbing },		// ASEQ_CLIMB_ON
	{ ACMD_NONE,		ASEQ_STAND,					ASEQ_STAND,				BranchLwrClimbing },		// ASEQ_CLIMB_UP_START_R
	{ ACMD_NONE,		ASEQ_STAND,					ASEQ_STAND,				BranchLwrClimbing },		// ASEQ_CLIMB_UP_START_L
	{ ACMD_NONE,		ASEQ_STAND,					ASEQ_STAND,				BranchLwrClimbing },		// ASEQ_CLIMB_UP_R
	{ ACMD_NONE,		ASEQ_STAND,					ASEQ_STAND,				BranchLwrClimbing },		// ASEQ_CLIMB_UP_L
	{ ACMD_NONE,		ASEQ_STAND,					ASEQ_STAND,				BranchLwrClimbing },		// ASEQ_CLIMB_DOWN_START_R
	{ ACMD_NONE,		ASEQ_STAND,					ASEQ_STAND,				BranchLwrClimbing },		// ASEQ_CLIMB_DOWN_START_L
	{ ACMD_NONE,		ASEQ_STAND,					ASEQ_STAND,				BranchLwrClimbing },		// ASEQ_CLIMB_DOWN_R
	{ ACMD_NONE,		ASEQ_STAND,					ASEQ_STAND,				BranchLwrClimbing },		// ASEQ_CLIMB_DOWN_L
	{ ACMD_NONE,		ASEQ_STAND,					ASEQ_STAND,				BranchLwrStanding },		// ASEQ_CLIMB_OFF
	{ ACMD_NONE,		ASEQ_STAND,					ASEQ_STAND,				BranchLwrClimbing },		// ASEQ_CLIMB_HOLD_R
	{ ACMD_NONE,		ASEQ_STAND,					ASEQ_STAND,				BranchLwrClimbing },		// ASEQ_CLIMB_HOLD_L
	{ ACMD_NONE,		ASEQ_STAND,					ASEQ_STAND,				BranchLwrClimbing },		// ASEQ_CLIMB_SETTLE_R
	{ ACMD_NONE,		ASEQ_STAND,					ASEQ_STAND,				BranchLwrClimbing },		// ASEQ_CLIMB_SETTLE_L
	{ ACMD_NONE,		ASEQ_STAND,					ASEQ_STAND,				BranchLwrKnockDown },		// ASEQ_KNOCK_DOWN
	{ ACMD_NONE,		ASEQ_STAND,					ASEQ_STAND,				BranchLwrStanding },		// ASEQ_KNOCK_DOWN_GETUP
	{ ACMD_NONE,		ASEQ_STAND,					ASEQ_STAND,				BranchLwrStanding },		// ASEQ_KNOCK_DOWN_EVADE
	{ ACMD_NONE,		ASEQ_SHRINE,				ASEQ_STAND,				BranchLwrStanding },		// ASEQ_SHRINE
	{ ACMD_NONE,		ASEQ_NONE,					ASEQ_STAND,				BranchLwrStanding },		// ASEQ_TAKEPUZZLEPIECE
	{ ACMD_NONE,		ASEQ_NONE,					ASEQ_STAND,				BranchLwrSurfaceSwim },		// ASEQ_TAKEPUZZLEUNDERWATER
	{ ACMD_NONE,		ASEQ_DROWN,					ASEQ_DROWN,				BranchLwrStanding },		// ASEQ_DROWN
	{ ACMD_NONE,		ASEQ_FORWARD_FLIP_R,		ASEQ_FORWARD_FLIP_R,	NULL },						// ASEQ_FORWARD_FLIP_L_GO
	{ ACMD_NONE,		ASEQ_FORWARD_FLIP_L,		ASEQ_FORWARD_FLIP_L,	NULL },						// ASEQ_FORWARD_FLIP_R_GO
	{ ACMD_NONE,		ASEQ_FALL,					ASEQ_FALL,				NULL },						// ASEQ_FORWARD_FLIP_L
	{ ACMD_NONE,		ASEQ_FALL,					ASEQ_FALL,				NULL },						// ASEQ_FORWARD_FLIP_R
	{ ACMD_NONE,		ASEQ_CSTRAFE_LEFT,			ASEQ_STAND,				BranchLwrShortstep },		// ASEQ_CSTRAFE_LEFT
	{ ACMD_NONE,		ASEQ_CSTRAFE_RIGHT,			ASEQ_STAND,				BranchLwrShortstep },		// ASEQ_CSTRAFE_RIGHT
	{ ACMD_NONE,		ASEQ_WSTRAFE_LEFT,			ASEQ_STAND,				BranchLwrWalking },			// ASEQ_WSTRAFE_LEFT
	{ ACMD_NONE,		ASEQ_WSTRAFE_RIGHT,			ASEQ_STAND,				BranchLwrWalking },			// ASEQ_WSTRAFE_RIGHT
	{ ACMD_NONE,		ASEQ_RSTRAFE_LEFT,			ASEQ_STAND,				BranchLwrRunning },			// ASEQ_RSTRAFE_LEFT
	{ ACMD_NONE,		ASEQ_RSTRAFE_RIGHT, 		ASEQ_STAND,				BranchLwrRunning },			// ASEQ_RSTRAFE_RIGHT
	{ ACMD_NONE,		ASEQ_JUMPBACK,				ASEQ_JUMPBACK,			NULL },						// ASEQ_JUMPBACK_SGO
	{ ACMD_NONE,		ASEQ_JUMPBACK,				ASEQ_JUMPBACK,			NULL },						// ASEQ_JUMPBACK_WGO
	{ ACMD_NONE,		ASEQ_JUMPBACK,				ASEQ_JUMPBACK,			NULL },						// ASEQ_JUMPBACK_RGO
	{ ACMD_NONE,		ASEQ_FALL,					ASEQ_FALL,				NULL },						// ASEQ_JUMPBACK
	{ ACMD_NONE,		ASEQ_FALL,					ASEQ_FALL,				NULL },						// ASEQ_JUMPBACKFLIP
	{ ACMD_NONE,		ASEQ_JUMPLEFT,				ASEQ_JUMPLEFT,			NULL },						// ASEQ_JUMPLEFT_SGO
	{ ACMD_NONE,		ASEQ_JUMPLEFT,				ASEQ_JUMPLEFT,			NULL },						// ASEQ_JUMPLEFT_WGO
	{ ACMD_NONE,		ASEQ_JUMPLEFT,				ASEQ_JUMPLEFT,			NULL },						// ASEQ_JUMPLEFT_RGO
	{ ACMD_NONE,		ASEQ_FALL,					ASEQ_FALL,				NULL },						// ASEQ_JUMPLEFT
	{ ACMD_NONE,		ASEQ_FALL,					ASEQ_FALL,				NULL },						// ASEQ_JUMPLEFTFLIP
	{ ACMD_NONE,		ASEQ_JUMPRIGHT,				ASEQ_JUMPRIGHT,			NULL },						// ASEQ_JUMPRIGHT_SGO
	{ ACMD_NONE,		ASEQ_JUMPRIGHT,				ASEQ_JUMPRIGHT,			NULL },						// ASEQ_JUMPRIGHT_WGO
	{ ACMD_NONE,		ASEQ_JUMPRIGHT,				ASEQ_JUMPRIGHT,			NULL },						// ASEQ_JUMPRIGHT_RGO
	{ ACMD_NONE,		ASEQ_FALL,					ASEQ_FALL,				NULL },						// ASEQ_JUMPRIGHT
	{ ACMD_NONE,		ASEQ_FALL,					ASEQ_FALL,				NULL },						// ASEQ_JUMPRIGHTFLIP
	{ ACMD_NONE,		ASEQ_DROWN_IDLE,			ASEQ_DROWN_IDLE,		NULL },						// ASEQ_DROWN_IDLE
	{ ACMDL_STRAFE_L,	ASEQ_DASH_LEFT,				ASEQ_STAND,				NULL },						// ASEQ_DASH_LEFT_GO
	{ ACMDL_STRAFE_L,	ASEQ_DASH_LEFT,				ASEQ_STAND,				BranchLwrStanding },		// ASEQ_DASH_LEFT
	{ ACMDL_STRAFE_R,	ASEQ_DASH_RIGHT,			ASEQ_STAND,				NULL },						// ASEQ_DASH_RIGHT_GO
	{ ACMDL_STRAFE_R,	ASEQ_DASH_RIGHT,			ASEQ_STAND,				BranchLwrStanding },		// ASEQ_DASH_RIGHT
	{ ACMDL_BACK,		ASEQ_CSTRAFEB_LEFT,			ASEQ_STAND,				BranchLwrShortstep },		// ASEQ_CSTRAFEB_LEFT
	{ ACMDL_BACK,		ASEQ_CSTRAFEB_RIGHT,		ASEQ_STAND,				BranchLwrShortstep },		// ASEQ_CSTRAFEB_RIGHT
	{ ACMDL_BACK,		ASEQ_WSTRAFEB_LEFT, 		ASEQ_STAND,				BranchLwrWalking },			// ASEQ_WSTRAFEB_LEFT
	{ ACMDL_BACK,		ASEQ_WSTRAFEB_RIGHT,		ASEQ_STAND,				BranchLwrWalking },			// ASEQ_WSTRAFEB_RIGHT
	{ ACMD_NONE,		ASEQ_STAND,					ASEQ_STAND,				BranchLwrStandingRun },		// ASEQ_OVERHANG
	{ ACMD_NONE,		ASEQ_DEATH_B,				ASEQ_DEATH_B,			NULL },						// ASEQ_DEATH_B
	{ ACMD_NONE,		ASEQ_DEATH_FLYFWD,			ASEQ_DEATH_FLYFWD,		NULL },						// ASEQ_DEATH_FLYFWD
	{ ACMD_NONE,		ASEQ_DEATH_FLYBACK,			ASEQ_DEATH_FLYBACK,		NULL },						// ASEQ_DEATH_FLYBACK
	{ ACMD_NONE,		ASEQ_DEATH_CHOKE,			ASEQ_DEATH_CHOKE,		NULL },						// ASEQ_DEATH_CHOKE
	{ ACMD_NONE,		ASEQ_STAND,					ASEQ_STAND,				BranchLwrStanding },		// ASEQ_IDLE_LOOKBACK
	{ ACMD_NONE,		ASEQ_STAND,					ASEQ_STAND,				BranchLwrStanding },		// ASEQ_IDLE_SCRATCH_ASS
	{ ACMD_NONE,		ASEQ_STAND,					ASEQ_STAND,				BranchLwrStanding },		// ASEQ_IDLE_WIPE_BROW
	{ ACMDL_CREEP_F,	ASEQ_CROUCH_WALK_F,			ASEQ_CROUCH,			BranchLwrCrouching },		// ASEQ_CROUCH_WALK_F
	{ ACMDL_CREEP_B,	ASEQ_CROUCH_WALK_B,			ASEQ_CROUCH,			BranchLwrCrouching },		// ASEQ_CROUCH_WALK_B
	{ ACMDL_STRAFE_L,	ASEQ_CROUCH_WALK_L,			ASEQ_CROUCH,			BranchLwrCrouching },		// ASEQ_CROUCH_WALK_L
	{ ACMDL_STRAFE_R,	ASEQ_CROUCH_WALK_R,			ASEQ_CROUCH,			BranchLwrCrouching },		// ASEQ_CROUCH_WALK_R

	{ ACMDL_RUN_F,		ASEQ_SSWIM_FAST,			ASEQ_SSWIM_IDLE,		BranchLwrSurfaceSwim },		// ASEQ_SSWIM_FAST_GO
	{ ACMDL_RUN_F,		ASEQ_SSWIM_FAST,			ASEQ_SSWIM_IDLE,		BranchLwrSurfaceSwim },		// ASEQ_SSWIM_FAST
};

PLAYER_API paceldata_t PlayerSeqData[ASEQ_MAX] =
{
	// move									fly	lck	playerflags
	{ &player_move_nothing,					0,	0,	PLAYER_FLAG_IDLE | PLAYER_FLAG_FALLBREAK },							// ASEQ_NONE
	{ &player_move_staffatkstand1,			0,	0,	PLAYER_FLAG_NONE },													// ASEQ_WSWORD_STD1,
	{ &player_move_staffatkstand2,			0,	0,	PLAYER_FLAG_NONE },													// ASEQ_WSWORD_STD2,
	{ &player_move_staffatkstep2,			0,	0,	PLAYER_FLAG_NONE },													// ASEQ_WSWORD_STEP2,
	{ &player_move_staffatkstep,			0,	0,	PLAYER_FLAG_NONE },													// ASEQ_WSWORD_STEP,
	{ &player_move_staffatkback,			0,	0,	PLAYER_FLAG_NONE },													// ASEQ_WSWORD_BACK,
	{ &player_move_staffdownstab,			0,	0,	PLAYER_FLAG_NONE },													// ASEQ_WSWORD_DOWNSTAB,
	{ &player_move_staffstabhold,			0,	0,	PLAYER_FLAG_NONE },													// ASEQ_WSWORD_STABHOLD,
	{ &player_move_staffpullout,			0,	0,	PLAYER_FLAG_NONE },													// ASEQ_WSWORD_PULLOUT,
	{ &player_move_staffblockleft,			0,	0,	PLAYER_FLAG_NONE },													// ASEQ_WSWORD_BLOCK_L,
	{ &player_move_staffblockleft2,			0,	0,	PLAYER_FLAG_NONE },													// ASEQ_WSWORD_BLOCK2_L,
	{ &player_move_staffblockleftatk,		0,	0,	PLAYER_FLAG_NONE },													// ASEQ_WSWORD_BLOCKATK_L
	{ &player_move_staffblockright,			0,	0,	PLAYER_FLAG_NONE },													// ASEQ_WSWORD_BLOCK_R,
	{ &player_move_staffblockright2,		0,	0,	PLAYER_FLAG_NONE },													// ASEQ_WSWORD_BLOCKATK1_R
	{ &player_move_staffblockrightatk,		0,	0,	PLAYER_FLAG_NONE },													// ASEQ_WSWORD_BLOCKATK2_R
	{ &player_move_staffblockedleft,		0,	0,	PLAYER_FLAG_NONE },													// ASEQ_WSWORD_BLOCKED_L,
	{ &player_move_staffblockedright,		0,	0,	PLAYER_FLAG_NONE },													// ASEQ_WSWORD_BLOCKED_R,
	{ &player_move_spellfireball,			0,	0,	PLAYER_FLAG_NONE },													// ASEQ_WFIREBALL,
	{ &player_move_spellarray,				0,	0,	PLAYER_FLAG_NONE },													// ASEQ_WARRAY,
	{ &player_move_spellspherestart,		0,	0,	PLAYER_FLAG_LEAVELOWER | PLAYER_FLAG_NONE },						// ASEQ_WSPHERE_GO,
	{ &player_move_spellspherecharge,		0,	0,	PLAYER_FLAG_LEAVELOWER | PLAYER_FLAG_NONE },						// ASEQ_WSPHERE_HOLD,
	{ &player_move_spellspherefire1,		0,	0,	PLAYER_FLAG_LEAVELOWER | PLAYER_FLAG_NONE },						// ASEQ_WSPHERE_FIRE1,
	{ &player_move_spellspherefire2,		0,	0,	PLAYER_FLAG_LEAVELOWER | PLAYER_FLAG_NONE },						// ASEQ_WSPHERE_FIRE2,
	{ &player_move_spellspherefire3,		0,	0,	PLAYER_FLAG_LEAVELOWER | PLAYER_FLAG_NONE },						// ASEQ_WSPHERE_FIRE3,
	{ &player_move_spellspherefire4,		0,	0,	PLAYER_FLAG_LEAVELOWER | PLAYER_FLAG_NONE },						// ASEQ_WSPHERE_FIRE4,
	{ &player_move_spellfirewall,			0,	0,	PLAYER_FLAG_NONE },													// ASEQ_WFIREWALL,
	{ &player_move_spellripper,				0,	0,	PLAYER_FLAG_NONE },													// ASEQ_WRIPPER,
	{ &player_move_spellbigball,			0,	0,	PLAYER_FLAG_NONE },													// ASEQ_WBIGBALL,
	{ &player_move_spellblast,				0,	0,	PLAYER_FLAG_NONE },													// ASEQ_WBLAST,
	{ &player_move_bowready,				0,	0,	PLAYER_FLAG_LEAVELOWER | PLAYER_FLAG_BOWDRAWN },					// ASEQ_WRRBOW_GO,
	{ &player_move_rrbowdrawarrow,			0,	0,	PLAYER_FLAG_LEAVELOWER | PLAYER_FLAG_BOWDRAWN },					// ASEQ_WRRBOW_DRAW,
	{ &player_move_bowholdarrow,			0,	0,	PLAYER_FLAG_LEAVELOWER | PLAYER_FLAG_BOWDRAWN },					// ASEQ_WRRBOW_HOLD,
	{ &player_move_rrbowfire,				0,	0,	PLAYER_FLAG_LEAVELOWER | PLAYER_FLAG_BOWDRAWN },					// ASEQ_WRRBOW_FIRE,
	{ &player_move_bowholdready,			0,	0,	PLAYER_FLAG_LEAVELOWER | PLAYER_FLAG_BOWDRAWN },					// ASEQ_WRRBOW_READY,
	{ &player_move_bowunready,				0,	0,	PLAYER_FLAG_LEAVELOWER | PLAYER_FLAG_NONE },						// ASEQ_WRRBOW_END,
	{ &player_move_bowready,				0,	0,	PLAYER_FLAG_LEAVELOWER | PLAYER_FLAG_BOWDRAWN },					// ASEQ_WPHBOW_GO,
	{ &player_move_phbowdrawarrow,			0,	0,	PLAYER_FLAG_LEAVELOWER | PLAYER_FLAG_BOWDRAWN },					// ASEQ_WPHBOW_DRAW,
	{ &player_move_bowholdarrow,			0,	0,	PLAYER_FLAG_LEAVELOWER | PLAYER_FLAG_BOWDRAWN },					// ASEQ_WPHBOW_HOLD,
	{ &player_move_phbowfire,				0,	0,	PLAYER_FLAG_LEAVELOWER | PLAYER_FLAG_BOWDRAWN },					// ASEQ_WPHBOW_FIRE,
	{ &player_move_bowholdready,			0,	0,	PLAYER_FLAG_LEAVELOWER | PLAYER_FLAG_BOWDRAWN },					// ASEQ_WPHBOW_READY,
	{ &player_move_bowunready,				0,	0,	PLAYER_FLAG_LEAVELOWER | PLAYER_FLAG_NONE },						// ASEQ_WPHBOW_END,
	{ &player_move_hellready,				0,	0,	PLAYER_FLAG_NONE },													// ASEQ_WHELL_GO,
	{ &player_move_hellfire1,				0,	0,	PLAYER_FLAG_NONE },													// ASEQ_WHELL_FIRE1,
	{ &player_move_hellfire2,				0,	0,	PLAYER_FLAG_NONE },													// ASEQ_WHELL_FIRE2,
	{ &player_move_hellunready,				0,	0,	PLAYER_FLAG_NONE },													// ASEQ_WHELL_END,
	{ &player_move_spelldefensive,			0,	0,	PLAYER_FLAG_NONE },													// ASEQ_SPELL_DEF
	{ &player_move_spellchange,				0,	0,	PLAYER_FLAG_NONE },													// ASEQ_HAND2HAND
	{ &player_move_drawsword,				0,	0,	PLAYER_FLAG_NONE },													// ASEQ_HAND2SWD,
	{ &player_move_drawhell,				0,	0,	PLAYER_FLAG_NONE },													// ASEQ_HAND2HELL,
	{ &player_move_drawbow,					0,	0,	PLAYER_FLAG_NONE },													// ASEQ_HAND2BOW,
	{ &player_move_stowsword,				0,	0,	PLAYER_FLAG_NONE },													// ASEQ_SWD2HAND,
	{ &player_move_sword2hell,				0,	0,	PLAYER_FLAG_NONE },													// ASEQ_SWD2HELL,
	{ &player_move_sword2bow,				0,	0,	PLAYER_FLAG_NONE },													// ASEQ_SWD2BOW,
	{ &player_move_stowhell,				0,	0,	PLAYER_FLAG_NONE },													// ASEQ_HELL2HAND,
	{ &player_move_hell2sword,				0,	0,	PLAYER_FLAG_NONE },													// ASEQ_HELL2SWD,
	{ &player_move_hell2bow,				0,	0,	PLAYER_FLAG_NONE },													// ASEQ_HELL2BOW,
	{ &player_move_stowbow,					0,	0,	PLAYER_FLAG_NONE },													// ASEQ_BOW2HAND,
	{ &player_move_bow2sword,				0,	0,	PLAYER_FLAG_NONE },													// ASEQ_BOW2SWD,
	{ &player_move_bow2hell,				0,	0,	PLAYER_FLAG_NONE },													// ASEQ_BOW2HELL,
	{ &player_move_bow2bow,					0,	0,	PLAYER_FLAG_NONE },													// ASEQ_BOW2BOW,
	{ &player_move_pushbuttongo,			0,	0,	PLAYER_FLAG_NONE },													// ASEQ_PUSHBUTTONGO,
	{ &player_move_pushleverleft,			0,	0,	PLAYER_FLAG_NONE },													// ASEQ_PUSHLEVERLEFT,
	{ &player_move_pushleverright,			0,	0,	PLAYER_FLAG_NONE },													// ASEQ_PUSHLEVERRIGHT,
	{ NULL,									0,	0,	PLAYER_FLAG_NONE },													// ASEQ_LOWER_BASE
	{ &player_move_stand,					0,	0,	PLAYER_FLAG_STAND | PLAYER_FLAG_IDLE | PLAYER_FLAG_FALLBREAK }, 	// ASEQ_STAND
	{ &player_move_pivotleftgo,				0,	0,	PLAYER_FLAG_STAND | PLAYER_FLAG_FALLBREAK },						// ASEQ_PIVOTL_GO
	{ &player_move_pivotleft,				0,	0,	PLAYER_FLAG_STAND | PLAYER_FLAG_FALLBREAK },						// ASEQ_PIVOTL
	{ &player_move_pivotleftend,			0,	0,	PLAYER_FLAG_STAND | PLAYER_FLAG_FALLBREAK },						// ASEQ_PIVOTL_END
	{ &player_move_pivotrightgo,			0,	0,	PLAYER_FLAG_STAND | PLAYER_FLAG_FALLBREAK },						// ASEQ_PIVOTR_GO
	{ &player_move_pivotright,				0,	0,	PLAYER_FLAG_STAND | PLAYER_FLAG_FALLBREAK },						// ASEQ_PIVOTR
	{ &player_move_pivotrightend,			0,	0,	PLAYER_FLAG_STAND | PLAYER_FLAG_FALLBREAK },						// ASEQ_PIVOTR_END
	{ &player_move_turn180,					0,	0,	PLAYER_FLAG_TURNLOCK | PLAYER_FLAG_STAND | PLAYER_FLAG_TURN180 },	// ASEQ_TURN180
	{ &player_move_runstart,				0,	0,	PLAYER_FLAG_FALLBREAK },											// ASEQ_RUNF_GO
	{ &player_move_run,						0,	0,	PLAYER_FLAG_FALLBREAK },											// ASEQ_RUNF
	{ &player_move_runstop,					0,	0,	PLAYER_FLAG_FALLBREAK },											// ASEQ_RUNF_END
	{ &player_move_walkstart,				0,	0,	PLAYER_FLAG_FALLBREAK },											// ASEQ_WALKF_GO
	{ &player_move_walk,					0,	0,	PLAYER_FLAG_FALLBREAK },											// ASEQ_WALKF
	{ &player_move_walkstop,				0,	0,	PLAYER_FLAG_FALLBREAK },											// ASEQ_WALKF_END
	{ &player_move_creepforward,			0,	0,	PLAYER_FLAG_FALLBREAK },											// ASEQ_CREEPF
	{ &player_move_creepforward_end,		0,	0,	PLAYER_FLAG_FALLBREAK },											// ASEQ_CREEPF_END
	{ &player_move_walkback,				0,	0,	PLAYER_FLAG_FALLBREAK },											// ASEQ_WALKB
	{ &player_move_creepback,				0,	0,	PLAYER_FLAG_FALLBREAK },											// ASEQ_CREEPB
	{ &player_move_creepback_end,			0,	0,	PLAYER_FLAG_FALLBREAK },											// ASEQ_CREEPB_END
	{ &player_move_crouchdown,				0,	0,	PLAYER_FLAG_FALLBREAK | PLAYER_FLAG_STAND | PLAYER_FLAG_RESIZED },	// ASEQ_CROUCH_GO
	{ &player_move_crouch,					0,	0,	PLAYER_FLAG_FALLBREAK | PLAYER_FLAG_STAND | PLAYER_FLAG_RESIZED },	// ASEQ_CROUCH
	{ &player_move_crouchup,				0,	0,	PLAYER_FLAG_FALLBREAK | PLAYER_FLAG_STAND | PLAYER_FLAG_RESIZED },	// ASEQ_CROUCH_END
	{ &player_move_crouchpivotleft,			0,	0,	PLAYER_FLAG_FALLBREAK | PLAYER_FLAG_STAND | PLAYER_FLAG_RESIZED },	// ASEQ_CROUCH_PIVOTL
	{ &player_move_crouchpivotright,		0,	0,	PLAYER_FLAG_FALLBREAK | PLAYER_FLAG_STAND | PLAYER_FLAG_RESIZED },	// ASEQ_CROUCH_PIVOTR
	{ &player_move_strafeleft,				0,	0,	PLAYER_FLAG_FALLBREAK },											// ASEQ_STRAFEL
	{ &player_move_strafeleft_end,			0,	0,	PLAYER_FLAG_FALLBREAK },											// ASEQ_STRAFEL_END
	{ &player_move_straferight,				0,	0,	PLAYER_FLAG_FALLBREAK },											// ASEQ_STRAFER,
	{ &player_move_straferight_end,			0,	0,	PLAYER_FLAG_FALLBREAK },											// ASEQ_STRAFER_END,
	{ &player_move_standjumpstart,			0,	0,	PLAYER_FLAG_TURNLOCK },												// ASEQ_JUMPSTD_GO
	{ &player_move_standjumpfwdstart,		0,	0,	PLAYER_FLAG_FALLING | PLAYER_FLAG_TURNLOCK },						// ASEQ_JUMPFWD_SGO
	{ &player_move_walkjumpfwdstart,		0,	0,	PLAYER_FLAG_FALLING | PLAYER_FLAG_TURNLOCK },						// ASEQ_JUMPFWD_WGO
	{ &player_move_runjumpfwdstart,			0,	0,	PLAYER_FLAG_FALLING | PLAYER_FLAG_TURNLOCK },						// ASEQ_JUMPFWD_RGO
	{ &player_move_jumpfwd,					0,	0,	PLAYER_FLAG_FALLING },												// ASEQ_JUMPFWD
	{ &player_move_jumpup,					0,	0,	PLAYER_FLAG_FALLING },												// ASEQ_JUMPUP
	{ &player_move_jumpuploop,				0,	0,	PLAYER_FLAG_FALLING },												// ASEQ_JUMPUP_LOOP
	{ &player_move_jumpflipleft,			0,	0,	PLAYER_FLAG_FALLING },												// ASEQ_JUMPFLIPL
	{ &player_move_jumpflipright,			0,	0,	PLAYER_FLAG_FALLING },												// ASEQ_JUMPFLIPR
	{ &player_move_jumpfliphandspringgo,	0,	0,	PLAYER_FLAG_FALLBREAK },  											// ASEQ_JUMPSPRINGBGO
	{ &player_move_jumpfliphandspring,		0,	0,	PLAYER_FLAG_FALLBREAK },  											// ASEQ_JUMPSPRINGB
	{ &player_move_jumpflipback,			0,	0,	PLAYER_FLAG_FALLBREAK },											// ASEQ_JUMPFLIPB
	{ &player_move_rolldivefwdwalk,			0,	0,	PLAYER_FLAG_FALLBREAK | PLAYER_FLAG_RESIZED },						// ASEQ_ROLLDIVEF_W
	{ &player_move_rolldivefwdrun,			0,	0,	PLAYER_FLAG_FALLBREAK | PLAYER_FLAG_RESIZED },						// ASEQ_ROLLDIVEF_R
	{ &player_move_rollfromfflip,			0,	0,	PLAYER_FLAG_FALLBREAK | PLAYER_FLAG_RESIZED },						// ASEQ_ROLL_FROM_FFLIP
	{ &player_move_polevault1walk,			0,	0,	PLAYER_FLAG_FALLBREAK },											// ASEQ_POLEVAULT1_W
	{ &player_move_polevault1run,			0,	0,	PLAYER_FLAG_FALLBREAK },											// ASEQ_POLEVAULT1_R
	{ &player_move_polevault2,				0,	0,	PLAYER_FLAG_FALLING | PLAYER_FLAG_TURNLOCK },						// ASEQ_POLEVAULT2
	{ &player_move_land1,					0,	0,	PLAYER_FLAG_NONE },													// ASEQ_LANDLIGHT
	{ &player_move_land3,					0,	0,	PLAYER_FLAG_NONE },													// ASEQ_LANDHEAVY
	{ &player_move_fallwalkstart,			0,	0,	PLAYER_FLAG_FALLING },												// ASEQ_FALLWALK_GO
	{ &player_move_fallwalkloop,			0,	0,	PLAYER_FLAG_FALLING },												// ASEQ_FALLRUN_GO
	{ &player_move_fall,					0,	0,	PLAYER_FLAG_FALLING },												// ASEQ_FALL
	{ &player_move_fallarmsup,				0,	0,	PLAYER_FLAG_FALLING },												// ASEQ_FALLARMSUP
	{ &player_move_vaultwall,				1,	1,	PLAYER_FLAG_TURNLOCK },												// ASEQ_VAULT_LOW
	{ &player_move_vaulthigh,				1,	1,	PLAYER_FLAG_TURNLOCK },												// ASEQ_VAULT_HIGH
	{ &player_move_pullupwall,				1,	1,	PLAYER_FLAG_TURNLOCK },												// ASEQ_PULLUP_WALL
	{ &player_move_sswimidle,				1,	0,	PLAYER_FLAG_SURFSWIM },												// ASEQ_SSWIM_IDLE
	{ &player_move_sswimfwdgo,				1,	0,	PLAYER_FLAG_SURFSWIM },												// ASEQ_SSWIMF_GO
	{ &player_move_sswimfwd,				1,	0,	PLAYER_FLAG_SURFSWIM },												// ASEQ_SSWIMF
	{ &player_move_sswimfwdstop,			1,	0,	PLAYER_FLAG_SURFSWIM },												// ASEQ_SSWIMF_END
	{ &player_move_sswimbackgo,				1,	0,	PLAYER_FLAG_SURFSWIM },												// ASEQ_SSWIMB_GO
	{ &player_move_sswimback,				1,	0,	PLAYER_FLAG_SURFSWIM },												// ASEQ_SSWIMB
	{ &player_move_sswimbackstop,			1,	0,	PLAYER_FLAG_SURFSWIM },												// ASEQ_SSWIMB_END
	{ &player_move_sswim_left_go,			1,	0,	PLAYER_FLAG_SURFSWIM },												// ASEQ_SSWIML_GO
	{ &player_move_sswim_right_go,			1,	0,	PLAYER_FLAG_SURFSWIM },												// ASEQ_SSWIMR_GO
	{ &player_move_sswim_left,				1,	0,	PLAYER_FLAG_SURFSWIM },												// ASEQ_SSWIML
	{ &player_move_sswim_right,				1,	0,	PLAYER_FLAG_SURFSWIM },												// ASEQ_SSWIMR
	{ &player_move_sswim_left_stop,			1,	0,	PLAYER_FLAG_SURFSWIM },												// ASEQ_SSWIML_END
	{ &player_move_sswim_right_stop,		1,	0,	PLAYER_FLAG_SURFSWIM },												// ASEQ_SSWIMR_END
	{ &player_move_staffatkspin,			0,	0,	PLAYER_FLAG_FALLBREAK },											// ASEQ_WSWORD_SPIN
	{ &player_move_staffatkspin2,			0,	0,	PLAYER_FLAG_FALLBREAK },											// ASEQ_WSWORD_SPIN2
	{ &player_move_staffspinblockedright,	0,	0,	PLAYER_FLAG_FALLBREAK },											// ASEQ_WSWORD_SPINBLOCKED
	{ &player_move_staffspinblockedleft,	0,	0,	PLAYER_FLAG_FALLBREAK },											// ASEQ_WSWORD_SPINBLOCKED2
	{ &player_move_stafflowerdownstab,		0,	0,	PLAYER_FLAG_FALLBREAK },											// ASEQ_WSWORD_LOWERDOWNSTAB
	{ &player_move_stafflowerpullout,		0,	0,	PLAYER_FLAG_FALLBREAK },											// ASEQ_WSWORD_LOWERPULLOUT
	{ &player_move_pulluphalfwall,			1,	1,	PLAYER_FLAG_TURNLOCK },												// ASEQ_PULLUP_HALFWALL
	{ &player_move_tumbleon1,				0,	0,	PLAYER_FLAG_TURNLOCK },												// ASEQ_TUMBLEON1
	{ &player_move_tumbleon2,				1,	1,	PLAYER_FLAG_FALLBREAK },											// ASEQ_TUMBLEON2
	{ &player_move_lstair4,					0,	0,	PLAYER_FLAG_STAND | PLAYER_FLAG_IDLE | PLAYER_FLAG_FALLBREAK },		// ASEQ_LSTAIR4
	{ &player_move_lstair8,					0,	0,	PLAYER_FLAG_STAND | PLAYER_FLAG_IDLE | PLAYER_FLAG_FALLBREAK },		// ASEQ_LSTAIR8
	{ &player_move_lstair12,				0,	0,	PLAYER_FLAG_STAND | PLAYER_FLAG_IDLE | PLAYER_FLAG_FALLBREAK },		// ASEQ_LSTAIR12
	{ &player_move_lstair16,				0,	0,	PLAYER_FLAG_STAND | PLAYER_FLAG_IDLE | PLAYER_FLAG_FALLBREAK },		// ASEQ_LSTAIR16
	{ &player_move_rstair4,					0,	0,	PLAYER_FLAG_STAND | PLAYER_FLAG_IDLE | PLAYER_FLAG_FALLBREAK },		// ASEQ_RSTAIR4
	{ &player_move_rstair8,					0,	0,	PLAYER_FLAG_STAND | PLAYER_FLAG_IDLE | PLAYER_FLAG_FALLBREAK },		// ASEQ_RSTAIR8
	{ &player_move_rstair12,				0,	0,	PLAYER_FLAG_STAND | PLAYER_FLAG_IDLE | PLAYER_FLAG_FALLBREAK },		// ASEQ_RSTAIR12
	{ &player_move_rstair16,				0,	0,	PLAYER_FLAG_STAND | PLAYER_FLAG_IDLE | PLAYER_FLAG_FALLBREAK },		// ASEQ_RSTAIR16
	{ &player_move_standreadystart,			0,	0,	PLAYER_FLAG_STAND | PLAYER_FLAG_IDLE | PLAYER_FLAG_FALLBREAK },		// ASEQ_IDLE_READY_GO
	{ &player_move_standready,				0,	0,	PLAYER_FLAG_STAND | PLAYER_FLAG_IDLE | PLAYER_FLAG_FALLBREAK },		// ASEQ_IDLE_READY
	{ &player_move_standreadyend,			0,	0,	PLAYER_FLAG_STAND | PLAYER_FLAG_IDLE | PLAYER_FLAG_FALLBREAK },		// ASEQ_IDLE_READY_END
	{ &player_move_standlookleft,			0,	0,	PLAYER_FLAG_STAND | PLAYER_FLAG_IDLE | PLAYER_FLAG_FALLBREAK },		// ASEQ_IDLE_LOOKL
	{ &player_move_standlookright,			0,	0,	PLAYER_FLAG_STAND | PLAYER_FLAG_IDLE | PLAYER_FLAG_FALLBREAK },		// ASEQ_IDLE_LOOKR
	{ &player_move_paina,					0,	0,	PLAYER_FLAG_IDLE | PLAYER_FLAG_FALLBREAK },							// ASEQ_PAIN_A
	{ &player_move_painb,					0,	0,	PLAYER_FLAG_NONE },													// ASEQ_PAIN_B
	{ &player_move_pest1,					0,	0,	PLAYER_FLAG_STAND | PLAYER_FLAG_IDLE | PLAYER_FLAG_FALLBREAK },		// ASEQ_IDLE_FLY1
	{ &player_move_pest2,					0,	0,	PLAYER_FLAG_STAND | PLAYER_FLAG_IDLE | PLAYER_FLAG_FALLBREAK },		// ASEQ_IDLE_FLY2
	{ &player_move_fallleft,				0,	0,	PLAYER_FLAG_FALLING },												// ASEQ_FALLL
	{ &player_move_fallright,				0,	0,	PLAYER_FLAG_FALLING },												// ASEQ_FALLR
	{ &player_move_fallleftend,				0,	0,	PLAYER_FLAG_FALLING },												// ASEQ_FALLL_END
	{ &player_move_fallrightend,			0,	0,	PLAYER_FLAG_FALLING },												// ASEQ_FALLR_END
	{ &player_move_death1,					1,	1,	PLAYER_FLAG_TURNLOCK | PLAYER_FLAG_FALLBREAK },						// ASEQ_DEATH_A
	{ &player_move_uswimfwd_go,				1,	0,	PLAYER_FLAG_UNDERWATER },											// ASEQ_USWIMF_GO
	{ &player_move_uswimfwd,				1,	0,	PLAYER_FLAG_UNDERWATER },											// ASEQ_USWIMF
	{ &player_move_uswimfwd_end,			1,	0,	PLAYER_FLAG_UNDERWATER },											// ASEQ_USWIMF_END
	{ &player_move_dive1,					1,	0,	PLAYER_FLAG_UNDERWATER },											// ASEQ_DIVE
	{ &player_move_uswimbackgo,				1,	0,	PLAYER_FLAG_UNDERWATER },											// ASEQ_USWIMB_GO
	{ &player_move_uswimback,				1,	0,	PLAYER_FLAG_UNDERWATER },											// ASEQ_USWIMB
	{ &player_move_uswimbackstop,			1,	0,	PLAYER_FLAG_UNDERWATER },											// ASEQ_USWIMB_END
	{ &player_move_uswim_left_go,			1,	0,	PLAYER_FLAG_UNDERWATER },											// ASEQ_USWIML_GO
	{ &player_move_uswim_right_go,			1,	0,	PLAYER_FLAG_UNDERWATER },											// ASEQ_USWIMR_GO
	{ &player_move_uswim_left,				1,	0,	PLAYER_FLAG_UNDERWATER },											// ASEQ_USWIML
	{ &player_move_uswim_right,				1,	0,	PLAYER_FLAG_UNDERWATER },											// ASEQ_USWIMR
	{ &player_move_uswim_left_stop,			1,	0,	PLAYER_FLAG_UNDERWATER },											// ASEQ_USWIML_END
	{ &player_move_uswim_right_stop,		1,	0,	PLAYER_FLAG_UNDERWATER },											// ASEQ_USWIMR_END
	{ &player_move_slide_forward,			0,	0,	PLAYER_FLAG_FALLING | PLAYER_FLAG_TURNLOCK },						// ASEQ_SLIDE_FORWARD
	{ &player_move_slide_backward,			0,	0,	PLAYER_FLAG_FALLING | PLAYER_FLAG_TURNLOCK },						// ASEQ_SLIDE_BACKWARD
	{ &player_move_resurface,				1,	0,	PLAYER_FLAG_SURFSWIM },												// ASEQ_SSWIM_RESURFACE
	{ &player_move_roll_l,					0,	0,	PLAYER_FLAG_FALLBREAK | PLAYER_FLAG_RESIZED },						// ASEQ_ROLL_L
	{ &player_move_roll_r,					0,	0,	PLAYER_FLAG_FALLBREAK | PLAYER_FLAG_RESIZED },						// ASEQ_ROLL_R
	{ &player_move_idle_under,				1,	0,	PLAYER_FLAG_UNDERWATER },											// ASEQ_USWIM_IDLE
	{ &player_move_roll_b,					0,	0,	PLAYER_FLAG_FALLBREAK | PLAYER_FLAG_RESIZED },						// ASEQ_ROLL_B
	{ &player_move_climb_on,				1,	1,	PLAYER_FLAG_ONROPE },												// ASEQ_CLIMB_ON
	{ &player_move_climb_up_start_r,		1,	1,	PLAYER_FLAG_ONROPE },												// ASEQ_CLIMB_UP_START_R
	{ &player_move_climb_up_start_l,		1,	1,	PLAYER_FLAG_ONROPE },												// ASEQ_CLIMB_UP_START_L
	{ &player_move_climb_up_r,				1,	1,	PLAYER_FLAG_ONROPE },												// ASEQ_CLIMB_UP_R
	{ &player_move_climb_up_l,				1,	1,	PLAYER_FLAG_ONROPE },												// ASEQ_CLIMB_UP_L
	{ &player_move_climb_down_start_r,		1,	1,	PLAYER_FLAG_ONROPE },												// ASEQ_CLIMB_DOWN_START_R
	{ &player_move_climb_down_start_l,		1,	1,	PLAYER_FLAG_ONROPE },												// ASEQ_CLIMB_DOWN_START_L
	{ &player_move_climb_down_r,			1,	1,	PLAYER_FLAG_ONROPE },												// ASEQ_CLIMB_DOWN_R
	{ &player_move_climb_down_l,			1,	1,	PLAYER_FLAG_ONROPE },												// ASEQ_CLIMB_DOWN_L
	{ &player_move_climb_off,				0,	0,	PLAYER_FLAG_FALLING },												// ASEQ_CLIMB_OFF
	{ &player_move_climb_hold_r,			1,	1,	PLAYER_FLAG_ONROPE },												// ASEQ_CLIMB_HOLD_R
	{ &player_move_climb_hold_l,			1,	1,	PLAYER_FLAG_ONROPE },												// ASEQ_CLIMB_HOLD_L
	{ &player_move_climb_settle_r,			1,	1,	PLAYER_FLAG_ONROPE },												// ASEQ_CLIMB_SETTLE_R
	{ &player_move_climb_settle_l,			1,	1,	PLAYER_FLAG_ONROPE },												// ASEQ_CLIMB_SETTLE_L
	{ &player_move_knockdown,				0,	0,	PLAYER_FLAG_TURNLOCK | PLAYER_FLAG_RESIZED },						// ASEQ_KNOCKDOWN
	{ &player_move_knockdown_getup,			0,	0,	PLAYER_FLAG_RESIZED },												// ASEQ_KNOCKDOWN_GETUP
	{ &player_move_knockdown_evade,			0,	0,	PLAYER_FLAG_RESIZED },												// ASEQ_KNOCKDOWN_EVADE
	{ &player_move_shrine,					0,	1,	PLAYER_FLAG_TURNLOCK },												// ASEQ_SHRINE
	{ &player_move_takepuzzlepiece,			0,	0,	PLAYER_FLAG_TURNLOCK },												// ASEQ_TAKEPUZZLEPIECE
	{ &player_move_takepuzzleunderwater,	1,	0,	PLAYER_FLAG_SURFSWIM },												// ASEQ_TAKEPUZZLEUNDERWATER //TODO: unused?
	{ &player_move_drown,					1,	1,	PLAYER_FLAG_SURFSWIM | PLAYER_FLAG_TURNLOCK },						// ASEQ_DROWN
	{ &player_move_forward_flip_l_go,		0,	0,	PLAYER_FLAG_FALLING },												// ASEQ_FORWARD_FLIP_L_GO
	{ &player_move_forward_flip_r_go,		0,	0,	PLAYER_FLAG_FALLING },												// ASEQ_FORWARD_FLIP_R_GO
	{ &player_move_forward_flip_l,			0,	0,	PLAYER_FLAG_FALLING | PLAYER_FLAG_RESIZED },						// ASEQ_FORWARD_FLIP_L
	{ &player_move_forward_flip_r,			0,	0,	PLAYER_FLAG_FALLING | PLAYER_FLAG_RESIZED },						// ASEQ_FORWARD_FLIP_R
	{ &player_move_creep_strafe_left,		0,	0,	PLAYER_FLAG_FALLBREAK },											// ASEQ_CSTRAFE_LEFT
	{ &player_move_creep_strafe_right,		0,	0,	PLAYER_FLAG_FALLBREAK },											// ASEQ_CSTRAFE_RIGHT
	{ &player_move_walk_strafe_left,		0,	0,	PLAYER_FLAG_FALLBREAK },											// ASEQ_WSTRAFE_LEFT
	{ &player_move_walk_strafe_right,		0,	0,	PLAYER_FLAG_FALLBREAK },											// ASEQ_WSTRAFE_RIGHT
	{ &player_move_run_strafe_left,			0,	0,	PLAYER_FLAG_FALLBREAK },											// ASEQ_RSTRAFE_LEFT
	{ &player_move_run_strafe_right,		0,	0,	PLAYER_FLAG_FALLBREAK },											// ASEQ_RSTRAFE_RIGHT
	{ &player_move_standjumpbackstart,		0,	0,	PLAYER_FLAG_FALLING | PLAYER_FLAG_TURNLOCK },						// ASEQ_JUMPBACK_SGO
	{ &player_move_walkjumpbackstart,		0,	0,	PLAYER_FLAG_FALLING | PLAYER_FLAG_TURNLOCK },						// ASEQ_JUMPBACK_WGO
	{ &player_move_runjumpbackstart,		0,	0,	PLAYER_FLAG_FALLING | PLAYER_FLAG_TURNLOCK },						// ASEQ_JUMPBACK_RGO
	{ &player_move_jumpback,				0,	0,	PLAYER_FLAG_FALLING },												// ASEQ_JUMPBACK
	{ &player_move_jumpbackflip,			0,	0,	PLAYER_FLAG_FALLING },												// ASEQ_JUMPBACKFLIP
	{ &player_move_standjumpleftstart,		0,	0,	PLAYER_FLAG_FALLING | PLAYER_FLAG_TURNLOCK },						// ASEQ_JUMPLEFT_SGO
	{ &player_move_walkjumpleftstart,		0,	0,	PLAYER_FLAG_FALLING | PLAYER_FLAG_TURNLOCK },						// ASEQ_JUMPLEFT_WGO
	{ &player_move_runjumpleftstart,		0,	0,	PLAYER_FLAG_FALLING | PLAYER_FLAG_TURNLOCK },						// ASEQ_JUMPLEFT_RGO
	{ &player_move_jumpleft,				0,	0,	PLAYER_FLAG_FALLING },												// ASEQ_JUMPLEFT
	{ &player_move_jumpleftflip,			0,	0,	PLAYER_FLAG_FALLING },												// ASEQ_JUMPLEFTFLIP
	{ &player_move_standjumprightstart,		0,	0,	PLAYER_FLAG_FALLING | PLAYER_FLAG_TURNLOCK },						// ASEQ_JUMPRIGHT_SGO
	{ &player_move_walkjumprightstart,		0,	0,	PLAYER_FLAG_FALLING | PLAYER_FLAG_TURNLOCK },						// ASEQ_JUMPRIGHT_WGO
	{ &player_move_runjumprightstart,		0,	0,	PLAYER_FLAG_FALLING | PLAYER_FLAG_TURNLOCK },						// ASEQ_JUMPRIGHT_RGO
	{ &player_move_jumpright,				0,	0,	PLAYER_FLAG_FALLING },												// ASEQ_JUMPRIGHT
	{ &player_move_jumprightflip,			0,	0,	PLAYER_FLAG_FALLING },												// ASEQ_JUMPRIGHTFLIP
	{ &player_move_drown_idle,				1,	1,	PLAYER_FLAG_TURNLOCK },												// ASEQ_DROWN_IDLE
	{ &player_move_dash_left_go,			0,	0,	PLAYER_FLAG_FALLBREAK },											// ASEQ_DASH_LEFT_GO
	{ &player_move_dash_left,				0,	0,	PLAYER_FLAG_FALLBREAK },											// ASEQ_DASH_LEFT
	{ &player_move_dash_right_go,			0,	0,	PLAYER_FLAG_FALLBREAK },											// ASEQ_DASH_RIGHT_GO
	{ &player_move_dash_right,				0,	0,	PLAYER_FLAG_FALLBREAK },											// ASEQ_DASH_RIGHT
	{ &player_move_creepb_strafe_left,		0,	0,	PLAYER_FLAG_FALLBREAK },											// ASEQ_CSTRAFEB_LEFT
	{ &player_move_creepb_strafe_right,		0,	0,	PLAYER_FLAG_FALLBREAK },											// ASEQ_CSTRAFEB_RIGHT
	{ &player_move_walkb_strafe_left,		0,	0,	PLAYER_FLAG_FALLBREAK },											// ASEQ_WSTRAFEB_LEFT
	{ &player_move_walkb_strafe_right,		0,	0,	PLAYER_FLAG_FALLBREAK },											// ASEQ_WSTRAFEB_RIGHT
	{ &player_move_overhang,				1,	1,	PLAYER_FLAG_TURNLOCK },												// ASEQ_OVERHANG
	{ &player_move_death_b,					1,	1,	PLAYER_FLAG_TURNLOCK | PLAYER_FLAG_FALLBREAK },						// ASEQ_DEATH_B
	{ &player_move_death_fly_forward,		1,	1,	PLAYER_FLAG_TURNLOCK | PLAYER_FLAG_FALLBREAK },						// ASEQ_DEATH_FLYFWD
	{ &player_move_death_fly_back,			1,	1,	PLAYER_FLAG_TURNLOCK | PLAYER_FLAG_FALLBREAK },						// ASEQ_DEATH_FLYBACK
	{ &player_move_death_choke,				1,	1,	PLAYER_FLAG_TURNLOCK | PLAYER_FLAG_FALLBREAK },						// ASEQ_DEATH_CHOKE
	{ &player_move_idle_lookback,			0,	0,	PLAYER_FLAG_FALLBREAK | PLAYER_FLAG_IDLE | PLAYER_FLAG_STAND },		// ASEQ_IDLE_LOOKBACK
	{ &player_move_idle_scratch_ass,		0,	0,	PLAYER_FLAG_FALLBREAK | PLAYER_FLAG_IDLE | PLAYER_FLAG_STAND },		// ASEQ_IDLE_SCRATCH_ASS
	{ &player_move_idle_wipe_brow,			0,	0,	PLAYER_FLAG_FALLBREAK | PLAYER_FLAG_IDLE | PLAYER_FLAG_STAND },		// ASEQ_IDLE_WIPE_BROW
	{ &player_move_crouch_creep_forward,	0,	0,	PLAYER_FLAG_FALLBREAK | PLAYER_FLAG_RESIZED },						// ASEQ_CROUCH_WALK_F
	{ &player_move_crouch_creep_back,		0,	0,	PLAYER_FLAG_FALLBREAK | PLAYER_FLAG_RESIZED },						// ASEQ_CROUCH_WALK_B
	{ &player_move_crouch_creep_left,		0,	0,	PLAYER_FLAG_FALLBREAK | PLAYER_FLAG_RESIZED },						// ASEQ_CROUCH_WALK_L
	{ &player_move_crouch_creep_right,		0,	0,	PLAYER_FLAG_FALLBREAK | PLAYER_FLAG_RESIZED },						// ASEQ_CROUCH_WALK_R

	{ &player_move_swim_fast_go,			1,	0,	PLAYER_FLAG_SURFSWIM },												// ASEQ_SSWIM_FAST_GO
	{ &player_move_swim_fast,				1,	0,	PLAYER_FLAG_SURFSWIM },												// ASEQ_SSWIM_FAST
};

seq_data2_t PlayerSeqData2[ASEQ_MAX] =
{
	{ false,	14,		ASEQ_NONE,	ASEQ_NONE },		// ASEQ_NONE,

	// UPPER SEQEUNCES
	// NOSPLIT	VIEWHT	COLLIDE		WATERSEQ
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_WSWORD_STD1,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_WSWORD_STD2,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_WSWORD_STEP2,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_WSWORD_STEP,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_WSWORD_BACK,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_WSWORD_DOWNSTAB,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_WSWORD_STABHOLD,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_WSWORD_PULLOUT,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_WSWORD_BLOCK_L,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_WSWORD_BLOCK2_L,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_WSWORD_BLOCKATK_L,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_WSWORD_BLOCK_R,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_WSWORD_BLOCK2_R,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_WSWORD_BLOCKATK_R,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_WSWORD_BLOCKED_L,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_WSWORD_BLOCKED_R,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_WFIREBALL,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_WARRAY,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_WSPHERE_GO,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_WSPHERE_HOLD
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_WSPHERE_FIRE1
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_WSPHERE_FIRE2
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_WSPHERE_FIRE3
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_WSPHERE_FIRE4
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_WFIREWALL,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_WRIPPER,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_WBIGBALL,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_WBLAST,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_WRRBOW_GO,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_WRRBOW_DRAW,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_WRRBOW_HOLD,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_WRRBOW_FIRE,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_WRRBOW_READY,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_WRRBOW_END,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_WPHBOW_GO,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_WPHBOW_DRAW,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_WPHBOW_HOLD,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_WPHBOW_FIRE,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_WPHBOW_READY,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_WPHBOW_END,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_WHELL_GO,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_WHELL_FIRE1,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_WHELL_FIRE2,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_WHELL_END,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_SPELL_DEF,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_HAND2HAND,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_HAND2SWD,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_HAND2HELL,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_HAND2BOW,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_SWD2HAND,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_SWD2HELL,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_SWD2BOW,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_HELL2HAND,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_HELL2SWD,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_HELL2BOW,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_BOW2HAND,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_BOW2SWD,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_BOW2HELL,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_BOW2BOW,
	{ true,		14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_PUSHBUTTON_GO,
	{ true,		14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_PUSHLEVERLEFT,
	{ true,		14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_PUSHLEVERRIGHT,

	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_LOWER_BASE,

	// LOWER SEQUENCES
	// NOSPLIT	VIEWHT	COLLIDE		WATERSEQ
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_STAND,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_PIVOTL_GO,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_PIVOTL,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_PIVOTL_END,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_PIVOTR_GO,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_PIVOTR,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_PIVOTR_END,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_TURN180,
	{ false,	14,		ASEQ_STAND,	ASEQ_SSWIM_IDLE },	// ASEQ_RUNF_GO,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_RUNF,
	{ false,	14,		ASEQ_STAND,	ASEQ_SSWIM_IDLE },	// ASEQ_RUNF_END,
	{ false,	14,		ASEQ_STAND,	ASEQ_SSWIM_IDLE },	// ASEQ_WALKF_GO,
	{ false,	14,		ASEQ_STAND,	ASEQ_SSWIM_IDLE },	// ASEQ_WALKF,
	{ false,	14,		ASEQ_STAND,	ASEQ_SSWIM_IDLE },	// ASEQ_WALKF_END,
	{ false,	14,		ASEQ_STAND,	ASEQ_SSWIM_IDLE },	// ASEQ_CREEPF,
	{ false,	14,		ASEQ_STAND,	ASEQ_SSWIM_IDLE },	// ASEQ_CREEPF_END,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_WALKB,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_CREEPB,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_CREEPB_END,
	{ false,	-11,	ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_CROUCH_GO,
	{ false,	-11,	ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_CROUCH,
	{ false,	-11,	ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_CROUCH_END,
	{ false,	-11,	ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_CROUCHPIVOTL,
	{ false,	-11,	ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_CROUCHPIVOTR,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_STRAFEL,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_STRAFEL_END
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_STRAFER,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_STRAFER_END
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_JUMPSTD_GO,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_JUMPFWD_SGO,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_JUMPFWD_WGO,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_JUMPFWD_RGO,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_JUMPFWD,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_JUMPUP,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_JUMPUP_LOOP,
	{ true,		14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_JUMPFLIPL,
	{ true,		14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_JUMPFLIPR,
	{ true,		14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_JUMPSPRINGBGO,
	{ true,		14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_JUMPSPRINGB,
	{ true,		14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_JUMPFLIPB,
	{ true,		-11,	ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_ROLLDIVEF_W,
	{ true,		-11,	ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_ROLLDIVEF_R,
	{ true,		-11,	ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_ROLL_FROM_FFLIP,
	{ true,		14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_POLEVAULT1_W,
	{ true,		14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_POLEVAULT1_R,
	{ true,		14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_POLEVAULT2,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_LANDLIGHT,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_LANDHEAVY,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_FALLWALK_GO,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_FALLRUN_GO,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_FALL,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_FALLARMSUP,
	{ false,	14,		ASEQ_NONE,	ASEQ_NONE },		// ASEQ_VAULT_LOW,
	{ true,		14,		ASEQ_NONE,	ASEQ_NONE },		// ASEQ_VAULT_HIGH,
	{ true,		14,		ASEQ_NONE,	ASEQ_NONE },		// ASEQ_PULLUP_WALL,
	{ false,	14,		ASEQ_NONE,	ASEQ_STAND },		// ASEQ_SSWIM_IDLE,
	{ false,	14,		ASEQ_NONE,	ASEQ_STAND },		// ASEQ_SSWIMF_GO,
	{ false,	14,		ASEQ_NONE,	ASEQ_STAND },		// ASEQ_SSWIMF
	{ false,	14,		ASEQ_NONE,	ASEQ_STAND },		// ASEQ_SSWIMF_END
	{ false,	14,		ASEQ_NONE,	ASEQ_STAND },		// ASEQ_SSWIMB_GO,
	{ false,	14,		ASEQ_NONE,	ASEQ_STAND },		// ASEQ_SSWIMB,
	{ false,	14,		ASEQ_NONE,	ASEQ_STAND },		// ASEQ_SSWIMB_END,
	{ false,	14,		ASEQ_NONE,	ASEQ_STAND },		// ASEQ_SSWIML_GO,
	{ false,	14,		ASEQ_NONE,	ASEQ_STAND },		// ASEQ_SSWIMR_GO
	{ false,	14,		ASEQ_NONE,	ASEQ_STAND },		// ASEQ_SSWIML
	{ false,	14,		ASEQ_NONE,	ASEQ_STAND },		// ASEQ_SSWIMR
	{ false,	14,		ASEQ_NONE,	ASEQ_STAND },		// ASEQ_SSWIML_END
	{ false,	14,		ASEQ_NONE,	ASEQ_STAND },		// ASEQ_SSWIMR_END
	{ true,		14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_WSWORD_SPIN,
	{ true,		14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_WSWORD_SPIN2,
	{ true,		14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_WSWORD_SPINBLOCKED,
	{ true,		14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_WSWORD_SPINBLOCKED2,	
	{ true,		14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_WSWORD_LOWERDOWNSTAB,	
	{ true,		14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_WSWORD_LOWERPULLOU,	
	{ true,		14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_PULLUP_HALFWALL,
	{ true,		-11,	ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_TUMBLEON1,
	{ true,		-11,	ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_TUMBLEON2,
	{ false,	10,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_LSTAIR4,
	{ false,	6,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_LSTAIR8,
	{ false,	2,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_LSTAIR12,
	{ false,	-2,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_LSTAIR16,
	{ false,	10,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_RSTAIR4,
	{ false,	6,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_RSTAIR8,
	{ false,	2,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_RSTAIR12,
	{ false,	-2,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_RSTAIR16,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_IDLE_READY_GO,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_IDLE_READY,
	{ true,		14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_IDLE_READY_END,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_IDLE_LOOKL,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_IDLE_LOOKR,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_PAIN_A,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_PAIN_B,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_IDLE_FLY1,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_IDLE_FLY2,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_FALLL,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_FALLR,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_FALLL_END,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_FALLR_END,
	{ true,		14,		ASEQ_NONE,	ASEQ_DEATH_A },		// ASEQ_DEATH_A,
	{ false,	14,		ASEQ_NONE,	ASEQ_STAND },		// ASEQ_USWIMF_GO,
	{ false,	14,		ASEQ_NONE,	ASEQ_STAND },		// ASEQ_USWIMF,
	{ false,	14,		ASEQ_NONE,	ASEQ_STAND },		// ASEQ_USWIMF_END,
	{ true,		14,		ASEQ_NONE,	ASEQ_STAND },		// ASEQ_DIVE,
	{ false,	14,		ASEQ_NONE,	ASEQ_STAND },		// ASEQ_USWIMB_GO,
	{ false,	14,		ASEQ_NONE,	ASEQ_STAND },		// ASEQ_USWIMB,
	{ false,	14,		ASEQ_NONE,	ASEQ_STAND },		// ASEQ_USWIMB_END,
	{ false,	14,		ASEQ_NONE,	ASEQ_STAND },		// ASEQ_USWIML_GO,
	{ false,	14,		ASEQ_NONE,	ASEQ_STAND },		// ASEQ_USWIMR_GO
	{ false,	14,		ASEQ_NONE,	ASEQ_STAND },		// ASEQ_USWIML
	{ false,	14,		ASEQ_NONE,	ASEQ_STAND },		// ASEQ_USWIMR
	{ false,	14,		ASEQ_NONE,	ASEQ_STAND },		// ASEQ_USWIML_END
	{ false,	14,		ASEQ_NONE,	ASEQ_STAND },		// ASEQ_USWIMR_END
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_SLIDE_FORWARD
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_SLIDE_BACKWARD
	{ true,		14,		ASEQ_NONE,	ASEQ_STAND },		// ASEQ_SSWIM_RESURFACE
	{ true,		-11,	ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_ROLL_L
	{ true,		-11,	ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_ROLL_R
	{ false,	14,		ASEQ_NONE,	ASEQ_STAND },		// ASEQ_USWIM_IDLE
	{ true,		-11,	ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_ROLL_B
	{ true,		14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_CLIMB_ON
	{ true,		14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_CLIMB_UP_START_R
	{ true,		14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_CLIMB_UP_START_L
	{ true,		14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_CLIMB_UP_R
	{ true,		14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_CLIMB_UP_L
	{ true,		14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_CLIMB_DOWN_START_R
	{ true,		14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_CLIMB_DOWN_START_L
	{ true,		14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_CLIMB_DOWN_R
	{ true,		14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_CLIMB_DOWN_L
	{ true,		14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_CLIMB_OFF
	{ true,		14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_CLIMB_HOLD_R
	{ true,		14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_CLIMB_HOLD_L
	{ true,		14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_CLIMB_SETTLE_R
	{ true,		14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_CLIMB_SETTLE_L
	{ true,		14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_KNOCKDOWN
	{ true,		14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_KNOCKDOWN_GETUP
	{ true,		14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_KNOCKDOWN_EVADE
	{ true,		14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_SHRINE
	{ true,		14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_TAKEPUZZLEPIECE,
	{ true,		14,		ASEQ_NONE,	ASEQ_STAND },		// ASEQ_TAKEPUZZLEPIECEUNDERWATER
	{ true,		14,		ASEQ_NONE,	ASEQ_DROWN },		// ASEQ_DROWN
	{ true,		14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_FORWARD_FLIP_L_GO
	{ true,		14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_FORWARD_FLIP_R_GO
	{ true,		14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_FORWARD_FLIP_L
	{ true,		14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_FORWARD_FLIP_R
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_CSTRAFE_LEFT
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_CSTRAFE_RIGHT
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_WSTRAFE_LEFT
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_WSTRAFE_RIGHT
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_RSTRAFE_LEFT
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_RSTRAFE_RIGHT
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_JUMPBACK_SGO,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_JUMPBACK_WGO,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_JUMPBACK_RGO,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_JUMPBACK,
	{ true,		14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_JUMPBACKFLIP,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_JUMPLEFT_SGO,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_JUMPLEFT_WGO,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_JUMPLEFT_RGO,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_JUMPLEFT,
	{ true,		14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_JUMPLEFTFLIP,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_JUMPRIGHT_SGO,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_JUMPRIGHT_WGO,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_JUMPRIGHT_RGO,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_JUMPRIGHT,
	{ true,		14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_JUMPRIGHTFLIP,
	{ true,		14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_DROWN_IDLE,
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_DASH_LEFT_GO
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_DASH_LEFT
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_DASH_RIGHT_GO
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_DASH_RIGHT
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_CSTRAFEB_LEFT
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_CSTRAFEB_RIGHT
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_WSTRAFEB_LEFT
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_WSTRAFEB_RIGHT
	{ true,		14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_OVERHANG
	{ true,		14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_DEATH_B
	{ true,		14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_DEATH_FLYFWD
	{ true,		14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_DEATH_FLYBACK
	{ true,		14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_DEATH_CHOKE
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_IDLE_LOOKBACK
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_IDLE_SCRATCH_ASS
	{ false,	14,		ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_IDLE_WIPE_BROW

	{ false,	-11,	ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_CROUCH_WALK_F
	{ false,	-11,	ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_CROUCH_WALK_B
	{ false,	-11,	ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_CROUCH_WALK_L
	{ false,	-11,	ASEQ_NONE,	ASEQ_SSWIM_IDLE },	// ASEQ_CROUCH_WALK_R

	{ false,	14,		ASEQ_NONE,	ASEQ_STAND },		// ASEQ_SSWIM_FAST_GO
	{ false,	14,		ASEQ_NONE,	ASEQ_STAND },		// ASEQ_SSWIM_FAST
};

#pragma endregion

#pragma region ========================== Chicken animation tables ==========================

// Chicken tables. Pretty much duplicated the above tables - only with Chicken animations instead.
// Most are null entries so we match the ones above, but they do nothing, since your a chicken. All you can do is cluck and peck. And die.
seqctrl_t ChickenCtrl[ASEQ_MAX] =
{
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_NONE

	// UPPER SEQENCES - of which there are none for a chicken
	// ACMD,			CONTSEQ, 			CEASESEQ, 		BRANCHFUNC
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_WSWORD_STD1,
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_WSWORD_STD2,
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_WSWORD_STEP2,
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_WSWORD_STEP,
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_WSWORD_BACK,
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_WSWORD_DOWNSTAB,
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_WSWORD_STABHOLD,
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_WSWORD_PULLOUT,
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_WSWORD_BLOCK_L,
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_WSWORD_BLOCK2_L,
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_WSWORD_BLOCKATK_L,
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_WSWORD_BLOCK_R,
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_WSWORD_BLOCK2_R,
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_WSWORD_BLOCKATK_R,
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_WSWORD_BLOCKED_L,
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_WSWORD_BLOCKED_R,
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_WFIREBALL,
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_WARRAY,
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_WSPHERE_GO,
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_WSPHERE_HOLD
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_WSPHERE_FIRE1
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_WSPHERE_FIRE2
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_WSPHERE_FIRE3
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_WSPHERE_FIRE4
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_WFIREWALL,
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_WRIPPER,
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_WBIGBALL,
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_WBLAST,
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_WRRBOW_GO,
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_WRRBOW_DRAW,
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_WRRBOW_HOLD,
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_WRRBOW_FIRE,
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_WRRBOW_READY,
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_WRRBOW_END,
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_WPHBOW_GO,
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_WPHBOW_DRAW,
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_WPHBOW_HOLD,
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_WPHBOW_FIRE,
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_WPHBOW_READY,
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_WPHBOW_END,
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_WHELL_GO,
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_WHELL_FIRE1,
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_WHELL_FIRE2,
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_WHELL_END,
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_SPELL_DEF
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_HAND2HAND
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_HAND2SWD,
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_HAND2HELL,
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_HAND2BOW,
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_SWD2HAND,
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_SWD2HELL,
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_SWD2BOW,
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_HELL2HAND,
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_HELL2SWD,
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_HELL2BOW,
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_BOW2HAND,
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_BOW2SWD,
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_BOW2HELL,
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_BOW2BOW,
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_PUSHBUTTON_GO,
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_PUSHLEVERLEFT,
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_PUSHLEVERRIGHT,

	// LOWER SEQUENCES
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		NULL },						// ASEQ_LOWER_BASE

	// ACMD,			CONTSEQ,			CEASESEQ,		BRANCHFUNC
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		ChickenBranchLwrStanding },	// ASEQ_STAND
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_PIVOTL_GO
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_PIVOTL
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_PIVOTL_END
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_PIVOTR_GO
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_PIVOTR
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_PIVOTR_END
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,	 	NULL },						// ASEQ_TURN180
	{ ACMDL_FWD,		ASEQ_RUNF_GO,		ASEQ_NONE,		ChickenBranchLwrStanding },	// ASEQ_RUNF_GO
	{ ACMDU_ATTACK,		ASEQ_RUNF,			ASEQ_NONE,		ChickenBranchLwrStanding },	// ASEQ_RUNF
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_RUNF_END
	{ ACMD_NONE,		ASEQ_WALKF_GO,		ASEQ_STAND,		ChickenBranchLwrStanding },	// ASEQ_WALKF_GO
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_WALKF
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_WALKF_END
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_CREEPF
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_CREEPF_END
	{ ACMD_NONE,		ASEQ_WALKB,			ASEQ_STAND,		ChickenBranchLwrStanding },	// ASEQ_WALKB
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_CREEPB
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_CREEPB_END
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_CROUCH_GO
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_CROUCH
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_CROUCH_END
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_CROUCH_PIVOTL
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_CROUCH_PIVOTR
	{ ACMD_NONE,		ASEQ_STRAFEL,		ASEQ_STAND,		ChickenBranchLwrStanding},	// ASEQ_STRAFEL
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_STRAFEL_END
	{ ACMD_NONE,		ASEQ_STRAFER,		ASEQ_STAND,		ChickenBranchLwrStanding},	// ASEQ_STRAFER
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_STRAFER_END
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_JUMPSTD_GO
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_JUMPFWD_SGO,
	{ ACMD_NONE,		ASEQ_FALL,			ASEQ_STAND,		NULL },						// ASEQ_JUMPFWD_WGO,
	{ ACMD_NONE,		ASEQ_FALL,			ASEQ_STAND,		NULL },						// ASEQ_JUMPFWD_RGO,
	{ ACMD_NONE,		ASEQ_JUMPFWD,		ASEQ_STAND,		NULL },						// ASEQ_JUMPFWD
	{ ACMD_NONE,		ASEQ_JUMPFWD,  		ASEQ_STAND,		NULL },						// ASEQ_JUMPUP
	{ ACMD_NONE,		ASEQ_JUMPUP_LOOP,	ASEQ_STAND,		NULL },						// ASEQ_JUMPUP_LOOP
	{ ACMD_NONE,		ASEQ_FALL,			ASEQ_STAND,		NULL },						// ASEQ_JUMPFLIPL
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_JUMPFLIPR
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_JUMPSPRINGBGO
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_JUMPSPRINGB
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_JUMPFLIPB
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_ROLLDIVEF_W
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_ROLLDIVEF_R
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_ROLL_FROM_FFLIP
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL, },					// ASEQ_POLEVAULT1_W
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL, },					// ASEQ_POLEVAULT1_R
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL, },					// ASEQ_POLEVAULT2
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_LANDLIGHT
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_LANDHEAVY
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_FALLWALK_GO
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_FALLRUN_GO
	{ ACMD_NONE,		ASEQ_FALL,			ASEQ_STAND,		ChickenBranchLwrStanding },	// ASEQ_FALL
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_FALLARMSUP
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_VAULT_LOW
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_VAULT_HIGH
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_PULLUP_WALL
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_SSWIM_IDLE
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_SSWIMF_GO
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_SSWIMF
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_SSWIMF_END
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_SSWIMB_GO
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_SSWIMB
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_SSWIMB_END
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_SSWIML_GO
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND, 	NULL },						// ASEQ_SSWIMR_GO
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND, 	NULL },						// ASEQ_SSWIML
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND, 	NULL },						// ASEQ_SSWIMR
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_SSWIML_END
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_SSWIMR_END
	{ ACMD_NONE,		ASEQ_WSWORD_SPIN,	ASEQ_STAND,		ChickenBranchLwrStanding },	// ASEQ_WSWORD_SPIN
	{ ACMD_NONE,		ASEQ_WSWORD_SPIN2,	ASEQ_STAND,		ChickenBranchLwrStanding },	// ASEQ_WSWORD_SPIN2
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_WSWORD_SPINBLOCKED
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_WSWORD_SPINBLOCKED2
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_WSWORD_LOWERSTABDOWN
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_PULLUP_HALFWALL
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_TUMBLEON1
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_TUMBLEON2
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_LSTAIR4
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_LSTAIR8
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_LSTAIR12
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_LSTAIR16
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_RSTAIR4
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_RSTAIR8
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_RSTAIR12
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_RSTAIR16
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_IDLE_READY_GO
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_IDLE_READY
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_IDLE_READY_END
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		ChickenBranchLwrStanding },	// ASEQ_IDLE_LOOKL
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		ChickenBranchLwrStanding },	// ASEQ_IDLE_LOOKR
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_PAIN_A
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_PAIN_B
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_IDLE_FLY1
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_IDLE_FLY2
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_FALLL
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_FALLR
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_FALLL_END
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_FALLR_END
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_DEATH_A
	{ ACMDL_FWD,		ASEQ_USWIMF,		ASEQ_USWIMF,	ChickenBranchLwrStanding },	// ASEQ_USWIMF_GO
	{ ACMDL_FWD,		ASEQ_USWIMF,		ASEQ_USWIMF,	ChickenBranchLwrStanding },	// ASEQ_USWIMF
	{ ACMDL_FWD,		ASEQ_USWIMF,		ASEQ_USWIMF,	ChickenBranchLwrStanding },	// ASEQ_USWIMF_END
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_DIVE
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_USWIMB_GO
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_USWIMB
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_USWIMB_END
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_USWIML_GO
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND, 	NULL },						// ASEQ_USWIMR_GO
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND, 	NULL },						// ASEQ_USWIML
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND, 	NULL },						// ASEQ_USWIMR
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_USWIML_END
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_USWIMR_END
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_SLIDE_FORWARD
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_SLIDE_BACKWARD
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_RESURFACE
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_ROLL_L
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_ROLL_R
	{ ACMD_NONE,		ASEQ_NONE,			ASEQ_NONE,		ChickenBranchLwrStanding },	// ASEQ_USWIM_IDLE
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_ROLL_B
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_CLIMB_ON
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_CLIMB_UP_START_R
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_CLIMB_UP_START_L
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_CLIMB_UP_R
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_CLIMB_UP_L
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_CLIMB_DOWN_START_R
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_CLIMB_DOWN_START_L
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_CLIMB_DOWN_R
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_CLIMB_DOWN_L
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_CLIMB_OFF
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_CLIMB_HOLD_R
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_CLIMB_HOLD_L
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_CLIMB_SETTLE_R
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_CLIMB_SETTLE_L
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_KNOCK_DOWN
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_KNOCK_DOWN_GETUP
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_KNOCK_DOWN_EVADE
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_SHRINE
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_TAKEPUZZLEPIECE
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_TAKEPUZZLEUNDERWATER
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_DROWN
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_FORWARD_FLIP_L_GO
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_FORWARD_FLIP_R_GO
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_FORWARD_FLIP_L
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_FORWARD_FLIP_R
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_CSTRAFE_LEFT
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_CSTRAFE_RIGHT
	{ ACMDL_RUN_B,		ASEQ_WSTRAFE_LEFT,	ASEQ_STAND,		NULL },						// ASEQ_WSTRAFE_LEFT
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_WSTRAFE_RIGHT
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_RSTRAFE_LEFT
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_RSTRAFE_RIGHT
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_JUMPBACK_SGO,
	{ ACMD_NONE,		ASEQ_FALL,			ASEQ_STAND,		NULL },						// ASEQ_JUMPBACK_WGO,
	{ ACMD_NONE,		ASEQ_FALL,			ASEQ_STAND,		NULL },						// ASEQ_JUMPBACK_RGO,
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_JUMPBACK
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_JUMPBACKFLIP
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_JUMPLEFT_SGO,
	{ ACMD_NONE,		ASEQ_FALL,			ASEQ_STAND,		NULL },						// ASEQ_JUMPLEFT_WGO,
	{ ACMD_NONE,		ASEQ_FALL,			ASEQ_STAND,		NULL },						// ASEQ_JUMPLEFT_RGO,
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_JUMPLEFT
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_JUMPLEFTFLIP
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_JUMPRIGHT_SGO,
	{ ACMD_NONE,		ASEQ_FALL,			ASEQ_STAND,		NULL },						// ASEQ_JUMPRIGHT_WGO,
	{ ACMD_NONE,		ASEQ_FALL,			ASEQ_STAND,		NULL },						// ASEQ_JUMPRIGHT_RGO,
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_JUMPRIGHT
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_JUMPRIGHTFLIP
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_DROWN_IDLE
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_DASH_LEFT_GO
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_DASH_LEFT
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_DASH_RIGHT_GO
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_DASH_RIGHT
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_CSTRAFEB_LEFT
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_CSTRAFEB_RIGHT
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_WSTRAFEB_LEFT
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_WSTRAFEB_RIGHT
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_OVERHANG
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_DEATH_B
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_DEATH_FLYFWD
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_DEATH_FLYBACK
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_DEATH_CHOKE
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_IDLE_LOOKBACK
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_IDLE_SCRATCH_ASS
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_IDLE_WIPE_BROW

	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_CROUCH_WALK_F
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_CROUCH_WALK_B
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_CROUCH_WALK_L
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_CROUCH_WALK_R

	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_SSWIM_FAST_GO
	{ ACMD_NONE,		ASEQ_STAND,			ASEQ_STAND,		NULL },						// ASEQ_SSWIM_FAST
};

PLAYER_API paceldata_t PlayerChickenData[ASEQ_MAX] =
{
	// move						fly lck	playerflags
	{ &chickenp_move_stand, 	0,	0,	PLAYER_FLAG_IDLE | PLAYER_FLAG_FALLBREAK },						// ASEQ_NONE
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_WSWORD_STD1,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_WSWORD_STD2,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_WSWORD_STEP2,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_WSWORD_STEP,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_WSWORD_BACK,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_WSWORD_DOWNSTAB,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_WSWORD_STABHOLD,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_WSWORD_PULLOUT,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_WSWORD_BLOCK_L,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_WSWORD_BLOCK2_L,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_WSWORD_BLOCKATK_L,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_WSWORD_BLOCK_R,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_WSWORD_BLOCK2_R,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_WSWORD_BLOCKATK_R,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_WSWORD_BLOCKED_L,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_WSWORD_BLOCKED_R,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_WFIREBALL,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_WARRAY,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_WSPHERE_GO,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_WSPHERE_HOLD,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_WSPHERE_FIRE1,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_WSPHERE_FIRE2,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_WSPHERE_FIRE3,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_WSPHERE_FIRE4,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_WFIREWALL,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_RIPPER,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_WBIGBALL,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_WBLAST,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_WRRBOW_GO,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_WRRBOW_DRAW,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_WRRBOW_HOLD,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_WRRBOW_FIRE,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_WRRBOW_READY,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_WRRBOW_END,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_WPHBOW_GO,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_WPHBOW_DRAW,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_WPHBOW_HOLD,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_WPHBOW_FIRE,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_WPHBOW_READY,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_WPHBOW_END,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_WHELL_GO,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_WHELL_FIRE1,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_WHELL_FIRE2,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_WHELL_END,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_SPELL_DEF
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_HAND2HAND
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_HAND2SWD,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_HAND2HELL,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_HAND2BOW,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_SWD2HAND,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_SWD2HELL,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_SWD2BOW
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_HELL2HAND
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_HELL2SWD
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_HELL2BOW
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_BOW2HAND
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_BOW2SWD
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_BOW2HELL
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_BOW2BOW
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_PUSHBUTTONGO
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_PUSHLEVERLEFT
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_PUSHLEVERRIGHT
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_LOWER_BASE
	{ &chickenp_move_stand,  	0,	0,	PLAYER_FLAG_STAND | PLAYER_FLAG_IDLE | PLAYER_FLAG_FALLBREAK },	// ASEQ_STAND
	{ &chickenp_move_stand,		0,	0,	PLAYER_FLAG_STAND | PLAYER_FLAG_FALLBREAK },					// ASEQ_PIVOTL_GO
	{ &chickenp_move_stand,		0,	0,	PLAYER_FLAG_STAND | PLAYER_FLAG_FALLBREAK },					// ASEQ_PIVOTL
	{ &chickenp_move_stand,		0,	0,	PLAYER_FLAG_STAND | PLAYER_FLAG_FALLBREAK },					// ASEQ_PIVOTL_END
	{ &chickenp_move_stand,		0,	0,	PLAYER_FLAG_STAND | PLAYER_FLAG_FALLBREAK },					// ASEQ_PIVOTR_GO
	{ &chickenp_move_stand,		0,	0,	PLAYER_FLAG_STAND | PLAYER_FLAG_FALLBREAK },					// ASEQ_PIVOTR
	{ &chickenp_move_stand,		0,	0,	PLAYER_FLAG_STAND | PLAYER_FLAG_FALLBREAK },					// ASEQ_PIVOTR_END
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_TURN180
	{ &chickenp_move_run,	  	0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_RUNF_GO,
	{ &chickenp_move_runattack,	0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_RUNF,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_RUNF_END,
	{ &chickenp_move_walk,	  	0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_WALKF_GO
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_WALKF
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_WALKF_END
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_CREEPF
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_CREEPF_END
	{ &chickenp_move_back,	  	0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_WALKB
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_CREEPB
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_CREEPB_END
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_CROUCH_GO
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_CROUCH
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_CROUCH_END
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_CROUCH_PIVOTL
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_CROUCH_PIVOTR
	{ &chickenp_move_strafel,	0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_STRAFEL
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_STRAFEL_END
	{ &chickenp_move_strafer,	0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_STRAFER
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_STRAFER_END
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_TURNLOCK },											// ASEQ_JUMPSTD_GO
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLING | PLAYER_FLAG_TURNLOCK},					// ASEQ_JUMPFWD_SGO
	{ &chickenp_move_wjump,		0,	0,	PLAYER_FLAG_FALLING | PLAYER_FLAG_TURNLOCK},					// ASEQ_JUMPFWD_WGO
	{ &chickenp_move_rjump,		0,	0,	PLAYER_FLAG_FALLING | PLAYER_FLAG_TURNLOCK},					// ASEQ_JUMPFWD_RGO
	{ &chickenp_move_jump_flap,	0,	0,	PLAYER_FLAG_FALLING },											// ASEQ_JUMPFWD
	{ &chickenp_move_jump,		0,	0,	PLAYER_FLAG_FALLING },											// ASEQ_JUMPUP
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLING },											// ASEQ_JUMPUP_LOOP
	{ &chickenp_move_rjumpb,	0,	0,	PLAYER_FLAG_FALLING },											// ASEQ_JUMPFLIPL
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLING },											// ASEQ_JUMPFLIPR
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_JUMPSRINGGO
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_JUMPSRINGB
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_JUMPFLIPB
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_ROLLDIVEF_W
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_ROLLDIVEF_R
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_ROLL_FROM_FFLIP
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_POLEVAULT1_W
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_POLEVAULT1_R
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLING },											// ASEQ_POLEVAULT2
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_LANDLIGHT
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_LANDHEAVY
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLING | PLAYER_FLAG_TURNLOCK},					// ASEQ_FALLWALK_GO
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLING },											// ASEQ_FALLRUN_GO
	{ &chickenp_move_jump_loop,	0,	0,	PLAYER_FLAG_FALLING },											// ASEQ_FALL
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLING },											// ASEQ_FALLARMSUP
	{ &chickenp_move_dummy,		1,	1,	PLAYER_FLAG_TURNLOCK },											// ASEQ_VAULT_LOW
	{ &chickenp_move_dummy,		1,	1,	PLAYER_FLAG_TURNLOCK },											// ASEQ_VAULT_HIGH
	{ &chickenp_move_dummy,		1,	1,	PLAYER_FLAG_TURNLOCK },											// ASEQ_PULLUP_WALL
	{ &chickenp_move_dummy,		1,	0,	PLAYER_FLAG_SURFSWIM },											// ASEQ_SSWIM_IDLE
	{ &chickenp_move_dummy,		1,	0,	PLAYER_FLAG_SURFSWIM },											// ASEQ_SSWIMF_GO
	{ &chickenp_move_dummy,		1,	0,	PLAYER_FLAG_SURFSWIM },											// ASEQ_SSWIMF
	{ &chickenp_move_dummy,		1,	0,	PLAYER_FLAG_SURFSWIM },											// ASEQ_SSWIMF_END
	{ &chickenp_move_dummy,		1,	0,	PLAYER_FLAG_SURFSWIM },											// ASEQ_SSWIMB_GO
	{ &chickenp_move_dummy,		1,	0,	PLAYER_FLAG_SURFSWIM },											// ASEQ_SSWIMB
	{ &chickenp_move_dummy,		1,	0,	PLAYER_FLAG_SURFSWIM },											// ASEQ_SSWIMB_END
	{ &chickenp_move_dummy,		1,	0,	PLAYER_FLAG_SURFSWIM },											// ASEQ_SSWIML_GO
	{ &chickenp_move_dummy,		1,	0,	PLAYER_FLAG_SURFSWIM },											// ASEQ_SSWIMR_GO
	{ &chickenp_move_dummy,		1,	0,	PLAYER_FLAG_SURFSWIM },											// ASEQ_SSWIML
	{ &chickenp_move_dummy,		1,	0,	PLAYER_FLAG_SURFSWIM },											// ASEQ_SSWIMR
	{ &chickenp_move_dummy,		1,	0,	PLAYER_FLAG_SURFSWIM },											// ASEQ_SSWIML_END
	{ &chickenp_move_dummy,		1,	0,	PLAYER_FLAG_SURFSWIM },											// ASEQ_SSWIMR_END
	{ &chickenp_move_attack,	0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_WSWORD_SPIN
	{ &chickenp_move_attack,	0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_WSWORD_SPIN2
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_WSWORD_SPINBLOCKED
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_WSWORD_SPINBLOCKED2
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_WSWORD_LOWERDOWNSTAB
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_WSWORD_LOWERPULLOUT
	{ &chickenp_move_dummy,		1,	1,	PLAYER_FLAG_TURNLOCK },											// ASEQ_PULLUP_HALFWALL
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_TURNLOCK },											// ASEQ_TUMBLEON1
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_TUMBLEON2
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_STAND | PLAYER_FLAG_IDLE | PLAYER_FLAG_FALLBREAK },	// ASEQ_LSTAIR4
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_STAND | PLAYER_FLAG_IDLE | PLAYER_FLAG_FALLBREAK },	// ASEQ_LSTAIR8
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_STAND | PLAYER_FLAG_IDLE | PLAYER_FLAG_FALLBREAK },	// ASEQ_LSTAIR12
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_STAND | PLAYER_FLAG_IDLE | PLAYER_FLAG_FALLBREAK },	// ASEQ_LSTAIR16
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_STAND | PLAYER_FLAG_IDLE | PLAYER_FLAG_FALLBREAK },	// ASEQ_RSTAIR4
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_STAND | PLAYER_FLAG_IDLE | PLAYER_FLAG_FALLBREAK },	// ASEQ_RSTAIR8
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_STAND | PLAYER_FLAG_IDLE | PLAYER_FLAG_FALLBREAK },	// ASEQ_RSTAIR12
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_STAND | PLAYER_FLAG_IDLE | PLAYER_FLAG_FALLBREAK },	// ASEQ_RSTAIR16
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_IDLE | PLAYER_FLAG_FALLBREAK },						// ASEQ_IDLE_READY_GO
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_IDLE | PLAYER_FLAG_FALLBREAK },						// ASEQ_IDLE_READY
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_IDLE | PLAYER_FLAG_FALLBREAK },						// ASEQ_IDLE_READY_END
	{ &chickenp_move_stand2,	0,	0,	PLAYER_FLAG_IDLE | PLAYER_FLAG_FALLBREAK },						// ASEQ_IDLE_LOOKL
	{ &chickenp_move_stand1, 	0,	0,	PLAYER_FLAG_IDLE | PLAYER_FLAG_FALLBREAK },						// ASEQ_IDLE_LOOKR
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_IDLE | PLAYER_FLAG_FALLBREAK },						// ASEQ_PAIN_A
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_NONE },												// ASEQ_PAIN_B
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_STAND | PLAYER_FLAG_IDLE | PLAYER_FLAG_FALLBREAK },	// ASEQ_IDLE_FLY1
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_STAND | PLAYER_FLAG_IDLE | PLAYER_FLAG_FALLBREAK },	// ASEQ_IDLE_FLY2
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_TURNLOCK },											// ASEQ_FALLL
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_TURNLOCK },											// ASEQ_FALLR
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_TURNLOCK },											// ASEQ_FALLL_END
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_TURNLOCK },											// ASEQ_FALLR_END
	{ &chickenp_move_dummy,		1,	1,	PLAYER_FLAG_TURNLOCK },											// ASEQ_DEATH_A
	{ &chickenp_move_swim,		1,	0,	PLAYER_FLAG_SURFSWIM },											// ASEQ_USWIMF_GO
	{ &chickenp_move_swim,		1,	0,	PLAYER_FLAG_SURFSWIM },											// ASEQ_USWIMF
	{ &chickenp_move_swim,		1,	0,	PLAYER_FLAG_SURFSWIM },											// ASEQ_USWIMF_END
	{ &chickenp_move_dummy,		1,	0,	PLAYER_FLAG_SURFSWIM },											// ASEQ_DIVE
	{ &chickenp_move_dummy,		1,	0,	PLAYER_FLAG_SURFSWIM },											// ASEQ_USWIMB_GO
	{ &chickenp_move_dummy,		1,	0,	PLAYER_FLAG_SURFSWIM },											// ASEQ_USWIMB
	{ &chickenp_move_dummy,		1,	0,	PLAYER_FLAG_SURFSWIM },											// ASEQ_USWIMB_END
	{ &chickenp_move_dummy,		1,	0,	PLAYER_FLAG_SURFSWIM },											// ASEQ_USWIML_GO
	{ &chickenp_move_dummy,		1,	0,	PLAYER_FLAG_SURFSWIM },											// ASEQ_USWIMR_GO
	{ &chickenp_move_dummy,		1,	0,	PLAYER_FLAG_SURFSWIM },											// ASEQ_USWIML
	{ &chickenp_move_dummy,		1,	0,	PLAYER_FLAG_SURFSWIM },											// ASEQ_USWIMR
	{ &chickenp_move_dummy,		1,	0,	PLAYER_FLAG_SURFSWIM },											// ASEQ_USWIML_END
	{ &chickenp_move_dummy,		1,	0,	PLAYER_FLAG_SURFSWIM },											// ASEQ_USWIMR_END
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLING | PLAYER_FLAG_TURNLOCK},					// ASEQ_SLIDE_FORWARD
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLING | PLAYER_FLAG_TURNLOCK},					// ASEQ_SLIDE_BACKWARD
	{ &chickenp_move_dummy,		1,	0,	PLAYER_FLAG_SURFSWIM },											// ASEQ_SSWIM_RESURFACE
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_ROLL_L
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_ROLL_R
	{ &chickenp_move_swim_idle,	1,	0,	PLAYER_FLAG_SURFSWIM },											// ASEQ_USWIM_IDLE
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_ROLL_B
	{ &chickenp_move_dummy,		1,	1,	PLAYER_FLAG_ONROPE },											// ASEQ_CLIMB_ON
	{ &chickenp_move_dummy,		1,	1,	PLAYER_FLAG_ONROPE },											// ASEQ_CLIMB_UP_START_R
	{ &chickenp_move_dummy,		1,	1,	PLAYER_FLAG_ONROPE },											// ASEQ_CLIMB_UP_START_L
	{ &chickenp_move_dummy,		1,	1,	PLAYER_FLAG_ONROPE },											// ASEQ_CLIMB_UP_R
	{ &chickenp_move_dummy,		1,	1,	PLAYER_FLAG_ONROPE },											// ASEQ_CLIMB_UP_L
	{ &chickenp_move_dummy,		1,	1,	PLAYER_FLAG_ONROPE },											// ASEQ_CLIMB_DOWN_START_R
	{ &chickenp_move_dummy,		1,	1,	PLAYER_FLAG_ONROPE },											// ASEQ_CLIMB_DOWN_START_L
	{ &chickenp_move_dummy,		1,	1,	PLAYER_FLAG_ONROPE },											// ASEQ_CLIMB_DOWN_R
	{ &chickenp_move_dummy,		1,	1,	PLAYER_FLAG_ONROPE },											// ASEQ_CLIMB_DOWN_L
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLING },											// ASEQ_CLIMB_OFF
	{ &chickenp_move_dummy,		1,	1,	PLAYER_FLAG_ONROPE },											// ASEQ_CLIMB_HOLD_R
	{ &chickenp_move_dummy,		1,	1,	PLAYER_FLAG_ONROPE },											// ASEQ_CLIMB_HOLD_L
	{ &chickenp_move_dummy,		1,	1,	PLAYER_FLAG_ONROPE },											// ASEQ_CLIMB_SETTLE_R
	{ &chickenp_move_dummy,		1,	1,	PLAYER_FLAG_ONROPE },											// ASEQ_CLIMB_SETTLE_L
	{ &chickenp_move_dummy,		0,	1,	PLAYER_FLAG_TURNLOCK },											// ASEQ_KNOCKDOWN
	{ &chickenp_move_dummy,		0,	1,	PLAYER_FLAG_TURNLOCK },											// ASEQ_KNOCKDOWN_GETUP
	{ &chickenp_move_dummy,		0,	1,	PLAYER_FLAG_TURNLOCK },											// ASEQ_KNOCKDOWN_EVADE
	{ &chickenp_move_dummy,		0,	1,	PLAYER_FLAG_TURNLOCK },											// ASEQ_SHRINE
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_TURNLOCK },											// ASEQ_TAKEPUZZLEPIECE
	{ &chickenp_move_dummy,		1,	0,	PLAYER_FLAG_SURFSWIM },											// ASEQ_TAKEPUZZLEUNDERWATER
	{ &chickenp_move_dummy,		1,	1,	PLAYER_FLAG_SURFSWIM | PLAYER_FLAG_TURNLOCK},					// ASEQ_DROWN
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLING },											// ASEQ_FORWARD_FLIP_L_GO
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLING },											// ASEQ_FORWARD_FLIP_R_GO
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLING },											// ASEQ_FORWARD_FLIP_L
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLING },											// ASEQ_FORWARD_FLIP_R
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_CSTRAFE_LEFT	
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_CSTRAFE_RIGHT
	//FIXME: Check this!!!
	{ &chickenp_move_runb,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_WSTRAFE_LEFT	
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_WSTRAFE_RIGHT
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_RSTRAFE_LEFT
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_RSTRAFE_RIGHT
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_JUMPBACK_SGO,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_JUMPBACK_WGO,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_JUMPBACK_RGO,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_JUMPBACK
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_JUMPFLIPBACK
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_JUMPLEFT_SGO,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_JUMPLEFT_WGO,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_JUMPLEFT_RGO,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_JUMPLEFT
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_JUMPFLIPLEFT
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_JUMPRIGHT_SGO,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_JUMPRIGHT_WGO,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_JUMPRIGHT_RGO,
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_JUMPRIGHT
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_JUMPFLIPRIGHT
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_DROWN_IDLE
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_DASH_LEFT_GO
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_DASH_LEFT
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_DASH_RIGHT_GO
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_DASH_RIGHT
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_CSTRAFEB_LEFT
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_CSTRAFEB_RIGHT
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_WSTRAFEB_LEFT
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_WSTRAFEB_RIGHT
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_OVERHANG
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_DEATH_B
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_DEATH_FLYFWD
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_DEATH_FLYBACK
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_DEATH_CHOKE
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_IDLE_LOOKBACK
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_IDLE_SCRATCH_ASS
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_IDLE_WIPE_BROW

	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_CROUCH_WALK_F
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_CROUCH_WALK_B
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_CROUCH_WALK_L
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_CROUCH_WALK_R

	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_SSWIM_FAST_GO
	{ &chickenp_move_dummy,		0,	0,	PLAYER_FLAG_FALLBREAK },										// ASEQ_SSWIM_FAST
};

#pragma endregion

#pragma region ========================== PLAYER SPELL ATTACK ANIMATIONS ==========================

// Throwing a fireball.
static panimframe_t player_frames_spellfireball[] =
{
	FRAME_throw2,  NULL, 0, 0, 0, PlayerActionHandFXStart, HANDFX_FIREBALL,	NULL,
	FRAME_throw4,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_throw5,  NULL, 0, 0, 0, PlayerActionSpellFireball, 0, NULL,
	FRAME_throw6,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_throw7,  NULL, 0, 0, 0, NULL, 0, NULL, // Trail end.
	FRAME_throw8,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_throw10, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_spellfireball = { 7, player_frames_spellfireball, PlayerAnimUpperUpdate };

// Casting the blast.
static panimframe_t player_frames_spellblast[] =
{
	FRAME_phbuton1, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_phbuton3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_phbuton5, NULL, 0, 0, 0, PlayerActionSpellBlast, 0, NULL,
	FRAME_phbuton6, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_phbuton7, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_phbuton8, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_phbuton9, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_spellblast = { 7, player_frames_spellblast, PlayerAnimUpperUpdate };

// Throwing a group of missiles.
static panimframe_t player_frames_spellarray[] =
{
	FRAME_throw2,  NULL, 0, 0, 0, PlayerActionHandFXStart, HANDFX_MISSILE, NULL,
	FRAME_throw3,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_throw4,  NULL, 0, 0, 0, PlayerActionSpellArray, -1, NULL,
	FRAME_throw5,  NULL, 0, 0, 0, PlayerActionSpellArray,  0, NULL,
	FRAME_throw6,  NULL, 0, 0, 0, PlayerActionSpellArray,  1, NULL,
	FRAME_throw7,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_throw8,  NULL, 0, 0, 0, NULL, 0, NULL, // Trail end
	FRAME_throw9,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_throw10, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_throw11, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_spellarray = { 10, player_frames_spellarray, PlayerAnimUpperUpdate };

#pragma endregion

#pragma region ========================== PLAYER SPHERE OF ANNIHILIATION ANIMATIONS ==========================

// Start casting sphere-of-annihilation (part1).
static panimframe_t player_frames_spellspherestart[] =
{
	FRAME_conjure1, NULL, 0, 0, 0, PlayerActionSpellSphereCreate, 0, NULL,
	FRAME_conjure3, NULL, 0, 0, 0, PlayerActionSpellSphereCharge, 1, NULL,
	FRAME_conjure5, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_conjure7, NULL, 0, 0, 0, PlayerActionSpellSphereCharge, 2, NULL,
	FRAME_conjure8, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_conjure9, NULL, 0, 0, 0, PlayerActionSpellSphereCharge, 3, NULL,
	FRAME_conjure10,NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_spellspherestart = { 7, player_frames_spellspherestart, PlayerAnimUpperUpdate };

// Build up power in sphere-of-annihilation.
static panimframe_t player_frames_spellspherecharge[] =
{
	FRAME_conjure11, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_conjure11, NULL, 0, 0, 0, PlayerActionSpellSphereCharge, 4, NULL,
};
panimmove_t player_move_spellspherecharge = { 2, player_frames_spellspherecharge, PlayerAnimUpperUpdate };

// Release sphere-of-annihilation.
static panimframe_t player_frames_spellspherefire1[] =
{
	FRAME_conjure7,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_conjure13, NULL, 0, 0, 0, PlayerActionSpellSphereRelease, 0, NULL,
	FRAME_conjure14, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_conjure15, NULL, 0, 0, 0, PlayerActionSphereTrailEnd, 0, NULL,
};
panimmove_t player_move_spellspherefire1 = { 4, player_frames_spellspherefire1, PlayerAnimUpperUpdate };

static panimframe_t player_frames_spellspherefire2[] =
{
	FRAME_conjure8,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_conjure13, NULL, 0, 0, 0, PlayerActionSpellSphereRelease, 0, NULL,
	FRAME_conjure14, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_conjure15, NULL, 0, 0, 0, PlayerActionSphereTrailEnd, 0, NULL,
};
panimmove_t player_move_spellspherefire2 = { 4, player_frames_spellspherefire2, PlayerAnimUpperUpdate };

static panimframe_t player_frames_spellspherefire3[] =
{
	FRAME_conjure11, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_conjure13, NULL, 0, 0, 0, PlayerActionSpellSphereRelease, 0, NULL,
	FRAME_conjure14, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_conjure15, NULL, 0, 0, 0, PlayerActionSphereTrailEnd, 0, NULL,
};
panimmove_t player_move_spellspherefire3 = { 4, player_frames_spellspherefire3, PlayerAnimUpperUpdate };

static panimframe_t player_frames_spellspherefire4[] =
{
	FRAME_conjure13, NULL, 0, 0, 0, PlayerActionSpellSphereRelease, 0, NULL,
	FRAME_conjure14, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_conjure15, NULL, 0, 0, 0, PlayerActionSphereTrailEnd, 0, NULL,
};
panimmove_t player_move_spellspherefire4 = { 3, player_frames_spellspherefire4, PlayerAnimUpperUpdate };

#pragma endregion

#pragma region ========================== PLAYER SPELL OF LOBBY BALL ANIMATIONS ==========================

// Throw Ripper, the unpowered Iron Doom.
static panimframe_t player_frames_spellripper[] =
{
	FRAME_phbuton1, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_phbuton3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_phbuton5, NULL, 0, 0, 0, PlayerActionSpellBigBall, 0, NULL,
	FRAME_phbuton6, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_phbuton7, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_phbuton8, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_phbuton9, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_spellripper = { 7, player_frames_spellripper, PlayerAnimUpperUpdate };

// Throw Big lobby ball
static panimframe_t player_frames_spellbigball[] =
{
	FRAME_ballspell1, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_ballspell2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_ballspell3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_ballspell4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_ballspell5, NULL, 0, 0, 0, PlayerActionSpellBigBall, 0, NULL,
	FRAME_ballspell6, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_ballspell7, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_ballspell8, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_breath1,    NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_breath14,   NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_spellbigball = { 10, player_frames_spellbigball, PlayerAnimUpperUpdate };

#pragma endregion

#pragma region ========================== PLAYER SPELL OF FIREWALL ANIMATIONS ==========================

// Cast firewall.
static panimframe_t player_frames_spellfirewall[] =
{
	FRAME_conjure15, NULL, 0, 0, 0, PlayerActionHandFXStart, HANDFX_FIREWALL, NULL,
	FRAME_conjure13, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_conjure11, NULL, 0, 0, 0, PlayerActionSpellFirewall, 0, NULL,
	FRAME_conjure9,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_conjure8,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_conjure7,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_conjure6,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_conjure5,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_conjure4,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_conjure3,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_conjure2,  NULL, 0, 0, 0, NULL, 0, NULL, // Trail end
	FRAME_conjure1,  NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_spellfirewall = { 12, player_frames_spellfirewall, PlayerAnimUpperUpdate };

#pragma endregion

#pragma region ========================== PLAYER BOW AND ARROW ANIMATIONS ==========================

// Bringing bow up and preparing to draw arrow.
static panimframe_t player_frames_bowready[] =
{
	FRAME_redybow3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_redybow5, NULL, 0, 0, 0, NULL, 0, NULL,
};

panimmove_t player_move_bowready = { 2, player_frames_bowready, PlayerAnimUpperUpdate };

// Drawing red rain arrow back.
static panimframe_t player_frames_rrbowdrawarrow[] =
{
	FRAME_draw1, NULL, 0, 0, 0, PlayerActionBowReadySound, 0, NULL,
	FRAME_draw3, NULL, 0, 0, 0, PlayerActionRedRainBowTrailStart, 0, NULL,
	FRAME_draw5, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_rrbowdrawarrow = { 3, player_frames_rrbowdrawarrow, PlayerAnimUpperUpdate };

// Drawing phoenix arrow back.
static panimframe_t player_frames_phbowdrawarrow[] =
{
	FRAME_draw1, NULL, 0, 0, 0, PlayerActionBowReadySound, 0, NULL,
	FRAME_draw3, NULL, 0, 0, 0, PlayerActionPhoenixBowTrailStart, 0, NULL,
	FRAME_draw5, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_phbowdrawarrow = { 3, player_frames_phbowdrawarrow, PlayerAnimUpperUpdate };

// Drawing phoenix arrow back.
static panimframe_t player_frames_bowholdarrow[] =
{
	FRAME_draw5, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_bowholdarrow = { 1, player_frames_bowholdarrow, PlayerAnimUpperUpdate };

// Releasing red rain arrow and preparing to draw another.
static panimframe_t player_frames_rrbowfire[] =
{
	FRAME_shoot1,   NULL, 0, 0, 0, PlayerActionRedRainBowAttack, 0, NULL,
	FRAME_shoot2,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_shoot3,   NULL, 0, 0, 0, PlayerActionBowTrailEnd, 0, NULL,
	FRAME_shoot4,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_shoot5,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_redybow5, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_redybow5, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_redybow5, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_rrbowfire = { 8, player_frames_rrbowfire, PlayerAnimUpperUpdate };

// Releasing phoenix arrow and preparing to draw another.
static panimframe_t player_frames_phbowfire[] =
{
	FRAME_shoot1,   NULL, 0, 0, 0, PlayerActionPhoenixBowAttack, 0, NULL,
	FRAME_shoot2,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_shoot3,   NULL, 0, 0, 0, PlayerActionBowTrailEnd, 0, NULL,
	FRAME_shoot4,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_shoot5,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_redybow5, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_redybow5, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_redybow5, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_phbowfire = { 8, player_frames_phbowfire, PlayerAnimUpperUpdate };

static panimframe_t player_frames_bowholdready[] =
{
	FRAME_redybow5, NULL, 0, 0, 0, NULL, 0, PlayerActionCheckBowRefire,
	FRAME_redybow5, NULL, 0, 0, 0, NULL, 0, PlayerActionCheckBowRefire,
	FRAME_redybow5, NULL, 0, 0, 0, NULL, 0, PlayerActionCheckBowRefire,
	FRAME_redybow5, NULL, 0, 0, 0, NULL, 0, PlayerActionCheckBowRefire,
	FRAME_redybow5, NULL, 0, 0, 0, NULL, 0, PlayerActionCheckBowRefire,
	FRAME_redybow5, NULL, 0, 0, 0, NULL, 0, PlayerActionCheckBowRefire,
	FRAME_redybow5, NULL, 0, 0, 0, NULL, 0, PlayerActionCheckBowRefire,
	FRAME_redybow5, NULL, 0, 0, 0, NULL, 0, PlayerActionCheckBowRefire,
	FRAME_redybow5, NULL, 0, 0, 0, NULL, 0, PlayerActionCheckBowRefire,
	FRAME_redybow5, NULL, 0, 0, 0, NULL, 0, PlayerActionCheckBowRefire,
};
panimmove_t player_move_bowholdready = { 10, player_frames_bowholdready, PlayerAnimUpperUpdate };

// Taking bow from ready position, back down to hip.
static panimframe_t player_frames_bowunready[] =
{
	FRAME_redybow4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_redybow2, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_bowunready = { 2, player_frames_bowunready, PlayerAnimUpperUpdate };

#pragma endregion

#pragma region ========================== PLAYER HELLSTAFF ANIMATIONS ==========================

// Bringing hellstaff up.
static panimframe_t player_frames_hellready[] =
{
	FRAME_helstf1, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_helstf2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_helstf3, NULL, 0, 0, 0, PlayerActionHellstaffAttack, 0, NULL,
};
panimmove_t player_move_hellready = { 3, player_frames_hellready, PlayerAnimUpperUpdate };

static panimframe_t player_frames_hellfire1[] =
{
	FRAME_helstf4, NULL, 0, 0, 0, PlayerActionHellstaffAttack, 0, NULL,
	FRAME_helstf5, NULL, 0, 0, 0, PlayerActionHellstaffAttack, 0, NULL,
};
panimmove_t player_move_hellfire1 = { 2, player_frames_hellfire1, PlayerAnimUpperUpdate };

static panimframe_t player_frames_hellfire2[] =
{
	FRAME_helstf6, NULL, 0, 0, 0, PlayerActionHellstaffAttack, 0, NULL,
	FRAME_helstf7, NULL, 0, 0, 0, PlayerActionHellstaffAttack, 0, NULL,
};
panimmove_t player_move_hellfire2 = { 2, player_frames_hellfire2, PlayerAnimUpperUpdate };

static panimframe_t player_frames_hellunready[] =
{
	FRAME_helstf2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_helstf1, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_hellunready = { 2, player_frames_hellunready, PlayerAnimUpperUpdate };

#pragma endregion

#pragma region ========================== PLAYER DEFENSIVE SPELL ANIMATION ==========================

// Casting the current defensive spell.
static panimframe_t player_frames_spelldefensive[] =
{
	FRAME_conjure1, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_conjure2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_conjure3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_conjure4, NULL, 0, 0, 0, PlayerActionSpellDefensive, 0, NULL,
	FRAME_conjure3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_conjure2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_conjure1, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_spelldefensive = { 7, player_frames_spelldefensive, PlayerAnimUpperUpdate };

#pragma endregion

#pragma region ========================== PLAYER SPELL CHANGE ANIMATION ==========================

// Reaching into his pouch and grabbing a new spell.
static panimframe_t player_frames_spellchange[] =
{	// Go to next spell
	FRAME_chngsplA1, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_chngsplA2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_chngsplA3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_chngsplA4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_chngsplA5, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_chngsplA6, NULL, 0, 0, 0, PlayerActionSpellChange, 0, NULL,
	FRAME_chngsplA7, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_chngsplA8, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_chngsplA9, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_spellchange = { 9, player_frames_spellchange, PlayerAnimUpperUpdate };

#pragma endregion

#pragma region ========================== PLAYER DRAW WEAPON ANIMATIONS ==========================

// Making the sword staff appear.
static panimframe_t player_frames_drawsword[] =
{
	FRAME_getstaff1, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getstaff2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getstaff3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getstaff4, NULL, 0, 0, 0, PlayerActionWeaponChange, WEAPON_READY_STAFFSTUB, NULL,
	FRAME_getstaff5, NULL, 0, 0, 0, PlayerActionStartStaffGlow, WEAPON_READY_SWORDSTAFF, NULL,
	FRAME_getstaff6, NULL, 0, 0, 0, PlayerActionWeaponChange, 0, NULL,
	FRAME_getstaff7, NULL, 0, 0, 0,	NULL, 0, NULL,
	FRAME_getstaff8, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_drawsword = { 8, player_frames_drawsword, PlayerAnimUpperUpdate };

// Making the hellstaff appear.
static panimframe_t player_frames_drawhell[] =
{
	FRAME_getstaff1, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getstaff2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getstaff3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getstaff4, NULL, 0, 0, 0, PlayerActionWeaponChange, WEAPON_READY_STAFFSTUB, NULL,
	FRAME_getstaff5, NULL, 0, 0, 0, PlayerActionStartStaffGlow, WEAPON_READY_HELLSTAFF, NULL,
	FRAME_getstaff6, NULL, 0, 0, 0, PlayerActionWeaponChange, 0, NULL,
	FRAME_getstaff7, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getstaff8, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_drawhell = { 8, player_frames_drawhell, PlayerAnimUpperUpdate };

// Taking the bow off his back and bringing it to his hip.
static panimframe_t player_frames_drawbow[] =
{
	FRAME_getbow1, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getbow2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getbow3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getbow4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getbow5, NULL, 0, 0, 0, PlayerActionWeaponChange, 0, NULL,
	FRAME_getbow6, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getbow7, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getbow8, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getbow9, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_drawbow = { 9, player_frames_drawbow, PlayerAnimUpperUpdate };

#pragma endregion

#pragma region ========================== PLAYER CHANGE WEAPON ANIMATIONS ==========================

// Making staff disappear.
static panimframe_t player_frames_stowsword[] =
{
	FRAME_unstff1, NULL, 0, 0, 0, PlayerActionEndStaffGlow, WEAPON_READY_SWORDSTAFF, NULL,
	FRAME_unstff2, NULL, 0, 0, 0, PlayerActionWeaponChange, WEAPON_READY_STAFFSTUB, NULL,
	FRAME_unstff3, NULL, 0, 0, 0, PlayerActionWeaponChange, 0, NULL,
	FRAME_unstff4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_unstff5, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_unstff6, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_stowsword = { 6, player_frames_stowsword, PlayerAnimUpperUpdate };

// Change from staff to hellstaff.
static panimframe_t player_frames_sword2hell[] =
{
	FRAME_getstaff8, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getstaff7, NULL, 0, 0, 0, PlayerActionEndStaffGlow, WEAPON_READY_SWORDSTAFF, NULL,
	FRAME_getstaff6, NULL, 0, 0, 0, PlayerActionWeaponChange, WEAPON_READY_STAFFSTUB, NULL,
	FRAME_getstaff5, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getstaff4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getstaff5, NULL, 0, 0, 0, PlayerActionStartStaffGlow, WEAPON_READY_HELLSTAFF, NULL,
	FRAME_getstaff6, NULL, 0, 0, 0, PlayerActionWeaponChange, 0, NULL,
	FRAME_getstaff7, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getstaff8, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_sword2hell = { 8, player_frames_sword2hell, PlayerAnimUpperUpdate };

// Change from staff to bow.
static panimframe_t player_frames_sword2bow[] =
{
	FRAME_unstff1, NULL, 0, 0, 0, PlayerActionEndStaffGlow, WEAPON_READY_SWORDSTAFF, NULL,
	FRAME_unstff2, NULL, 0, 0, 0, PlayerActionWeaponChange, WEAPON_READY_STAFFSTUB, NULL,
	FRAME_unstff3, NULL, 0, 0, 0, PlayerActionWeaponChange, WEAPON_READY_HANDS, NULL,
	FRAME_unstff4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_unstff5, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_unstff6, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getbow1, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getbow2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getbow3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getbow4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getbow5, NULL, 0, 0, 0, PlayerActionWeaponChange, 0, NULL,
	FRAME_getbow6, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getbow7, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getbow8, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getbow9, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_sword2bow = { 15, player_frames_sword2bow, PlayerAnimUpperUpdate };

// Put away hell staff.
static panimframe_t player_frames_stowhell[] =
{
	FRAME_unstff1, NULL, 0, 0, 0, PlayerActionEndStaffGlow, WEAPON_READY_HELLSTAFF, NULL,
	FRAME_unstff2, NULL, 0, 0, 0, PlayerActionWeaponChange, WEAPON_READY_STAFFSTUB, NULL,
	FRAME_unstff3, NULL, 0, 0, 0, PlayerActionWeaponChange, 0, NULL,
	FRAME_unstff4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_unstff5, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_unstff6, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_stowhell = { 6, player_frames_stowhell, PlayerAnimUpperUpdate };

// Change from hell staff to sword staff.
static panimframe_t player_frames_hell2sword[] =
{
	FRAME_getstaff8, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getstaff7, NULL, 0, 0, 0, PlayerActionEndStaffGlow, WEAPON_READY_HELLSTAFF, NULL,
	FRAME_getstaff6, NULL, 0, 0, 0, PlayerActionWeaponChange, WEAPON_READY_STAFFSTUB, NULL,
	FRAME_getstaff5, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getstaff4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getstaff5, NULL, 0, 0, 0, PlayerActionStartStaffGlow, WEAPON_READY_SWORDSTAFF, NULL,
	FRAME_getstaff6, NULL, 0, 0, 0, PlayerActionWeaponChange, 0, NULL,
	FRAME_getstaff7, NULL, 0, 0, 0,	NULL, 0, NULL,
	FRAME_getstaff8, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_hell2sword = { 9, player_frames_hell2sword, PlayerAnimUpperUpdate };

// Change from hell staff to bow.
static panimframe_t player_frames_hell2bow[] =
{
	FRAME_unstff1, NULL, 0, 0, 0, PlayerActionEndStaffGlow, WEAPON_READY_HELLSTAFF, NULL,
	FRAME_unstff2, NULL, 0, 0, 0, PlayerActionWeaponChange, WEAPON_READY_STAFFSTUB, NULL,
	FRAME_unstff3, NULL, 0, 0, 0, PlayerActionWeaponChange, WEAPON_READY_HANDS, NULL,
	FRAME_unstff4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_unstff5, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_unstff6, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getbow1, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getbow2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getbow3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getbow4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getbow5, NULL, 0, 0, 0, PlayerActionWeaponChange, 0, NULL,
	FRAME_getbow6, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getbow7, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getbow8, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getbow9, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_hell2bow = { 15, player_frames_hell2bow, PlayerAnimUpperUpdate };

// Put away bow.
static panimframe_t player_frames_stowbow[] =
{
	FRAME_getbow9, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getbow8, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getbow7, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getbow6, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getbow5, NULL, 0, 0, 0, PlayerActionWeaponChange, 0, NULL,
	FRAME_getbow4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getbow3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getbow2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getbow1, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_stowbow = { 9, player_frames_stowbow, PlayerAnimUpperUpdate };

// Change from bow to sword staff.
static panimframe_t player_frames_bow2sword[] =
{
	FRAME_getbow9,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getbow8,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getbow7,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getbow6,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getbow5,   NULL, 0, 0, 0, PlayerActionWeaponChange, WEAPON_READY_HANDS, NULL,
	FRAME_getbow4,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getbow3,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getbow2,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getbow1,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getstaff1, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getstaff2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getstaff3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getstaff4, NULL, 0, 0, 0, PlayerActionWeaponChange, WEAPON_READY_STAFFSTUB, NULL,
	FRAME_getstaff5, NULL, 0, 0, 0, PlayerActionStartStaffGlow, WEAPON_READY_SWORDSTAFF, NULL,
	FRAME_getstaff6, NULL, 0, 0, 0, PlayerActionWeaponChange, 0, NULL,
	FRAME_getstaff7, NULL, 0, 0, 0,	NULL, 0, NULL,
	FRAME_getstaff8, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_bow2sword = { 17, player_frames_bow2sword, PlayerAnimUpperUpdate };

// Change from bow to hell staff.
static panimframe_t player_frames_bow2hell[] =
{
	FRAME_getbow9,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getbow8,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getbow7,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getbow6,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getbow5,   NULL, 0, 0, 0, PlayerActionWeaponChange, WEAPON_READY_HANDS, NULL,
	FRAME_getbow4,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getbow3,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getbow2,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getbow1,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getstaff1, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getstaff2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getstaff3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getstaff4, NULL, 0, 0, 0, PlayerActionWeaponChange, WEAPON_READY_STAFFSTUB, NULL,
	FRAME_getstaff5, NULL, 0, 0, 0, PlayerActionStartStaffGlow, WEAPON_READY_HELLSTAFF, NULL,
	FRAME_getstaff6, NULL, 0, 0, 0, PlayerActionWeaponChange, 0, NULL,
	FRAME_getstaff7, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_getstaff8, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_bow2hell = { 17, player_frames_bow2hell, PlayerAnimUpperUpdate };

// Change arrows on the bow.
static panimframe_t player_frames_bow2bow[] =
{
	FRAME_swcharo1, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_swcharo2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_swcharo3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_swcharo4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_swcharo5, NULL, 0, 0, 0, PlayerActionArrowChange, 0, NULL,
	FRAME_swcharo6, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_swcharo7, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_bow2bow = { 7, player_frames_bow2bow, PlayerAnimUpperUpdate };

#pragma endregion

#pragma region ========================== PLAYER STAFF ATTACK ANIMATIONS ==========================

// Spin attack while running.
static panimframe_t player_frames_staffatkspin[] =
{
	FRAME_newspin1, PlayerMoveFunc, 200 * PHYS_SCALER, 0, 0, PlayerActionStaffTrailStart, TRAIL_SPIN1, NULL,
	FRAME_newspin2, PlayerMoveFunc, 175 * PHYS_SCALER, 0, 0, NULL, 0, NULL,
	FRAME_newspin3, PlayerMoveFunc, 175 * PHYS_SCALER, 0, 0, NULL, 0, NULL,
	FRAME_newspin4, PlayerMoveFunc, 175 * PHYS_SCALER, 0, 0, NULL, 0, NULL,
	FRAME_newspin5, PlayerMoveFunc, 175 * PHYS_SCALER, 0, 0, PlayerActionSwordAttack, 9, NULL,
	FRAME_newspin6, PlayerMoveFunc, 175 * PHYS_SCALER, 0, 0, PlayerActionSwordAttack, 10, NULL,
	FRAME_newspin7, PlayerMoveFunc, 175 * PHYS_SCALER, 0, 0, PlayerActionSwordAttack, 11, NULL,
	FRAME_newspin8, PlayerMoveFunc, 200 * PHYS_SCALER, 0, 0, NULL, 0, NULL, // End trail here
};
panimmove_t player_move_staffatkspin = { 8, player_frames_staffatkspin, PlayerAnimLowerUpdate };

// Also spin attack while running.
static panimframe_t player_frames_staffatkspin2[] =
{
	FRAME_spining1, PlayerMoveFunc, 200 * PHYS_SCALER, 0, 0, PlayerActionStaffTrailStart, TRAIL_SPIN2, NULL,
	FRAME_spining2, PlayerMoveFunc, 175 * PHYS_SCALER, 0, 0, NULL, 0, NULL,
	FRAME_spining3, PlayerMoveFunc, 175 * PHYS_SCALER, 0, 0, NULL, 0, NULL,
	FRAME_spining4, PlayerMoveFunc, 175 * PHYS_SCALER, 0, 0, PlayerActionSwordAttack, 13, NULL,
	FRAME_spining5, PlayerMoveFunc, 175 * PHYS_SCALER, 0, 0, PlayerActionSwordAttack, 14, NULL,
	FRAME_spining6, PlayerMoveFunc, 175 * PHYS_SCALER, 0, 0, PlayerActionSwordAttack, 15, NULL,
	FRAME_spining7, PlayerMoveFunc, 200 * PHYS_SCALER, 0, 0, NULL, 0, NULL, // End trail here
};
panimmove_t player_move_staffatkspin2 = { 7, player_frames_staffatkspin2, PlayerAnimLowerUpdate };

// Blocked spin left.
static panimframe_t player_frames_staffspinblockedright[] =
{
	FRAME_swipeA4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_swipeA3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_swipeA3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_swipeA3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_swipeA2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_swipeA1, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_staffspinblockedright = { 6, player_frames_staffspinblockedright, PlayerAnimLowerUpdate };

// Blocked swipe right
static panimframe_t player_frames_staffspinblockedleft[] =
{
	FRAME_swipeB2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_swipeB3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_swipeB3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_swipeB3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_swipeB2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_swipeB1, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_staffspinblockedleft = { 6, player_frames_staffspinblockedleft, PlayerAnimLowerUpdate };

// Upper frame staff moves, these can be split.

// Standing attack with one swipe in front from right to left.
panimframe_t player_frames_staffatkstand1[] =
{
	FRAME_swipeA1, NULL, 0, 0, 0, PlayerActionStaffTrailStart, TRAIL_STAND, NULL,
	FRAME_swipeA3, NULL, 0, 0, 0, PlayerActionSwordAttack, 1, NULL,
	FRAME_swipeA5, NULL, 0, 0, 0, PlayerActionSwordAttack, 2, NULL,
	FRAME_swipeA6, NULL, 0, 0, 0, PlayerActionSwordAttack, 3, NULL,
	FRAME_swipeA7, NULL, 0, 0, 0, PlayerActionSwordAttack, 4, NULL,
	FRAME_swipeA8, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_swipeA9, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_staffatkstand1 = { 7, player_frames_staffatkstand1, PlayerAnimUpperUpdate };

// Transition of staff sword from attack to standing.
static panimframe_t player_frames_staffatkstand2[] =
{
	FRAME_swipeA10, NULL, 0, 0, 0, NULL, 0, NULL, // End trail
	FRAME_swipeA11, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_staffatkstand2 = { 2, player_frames_staffatkstand2, PlayerAnimUpperUpdate };

// Standing attack with one swipe from left to right.
static panimframe_t player_frames_staffatkstep2[] =
{
	FRAME_swipeB2,  PlayerMoveUpperFunc,  30 * PHYS_SCALER, 0, 0, PlayerActionStaffTrailStart, TRAIL_STEP, NULL,
	FRAME_swipeB4,  PlayerMoveUpperFunc,  91 * PHYS_SCALER, 0, 0, PlayerActionSwordAttack, 5, NULL,
	FRAME_swipeB5,  PlayerMoveUpperFunc,  37 * PHYS_SCALER, 0, 0, PlayerActionSwordAttack, 6, NULL,
	FRAME_swipeB6,  PlayerMoveUpperFunc,  27 * PHYS_SCALER, 0, 0, PlayerActionSwordAttack, 7, NULL,
	FRAME_swipeB7,  PlayerMoveUpperFunc,  13 * PHYS_SCALER, 0, 0, PlayerActionSwordAttack, 8, NULL,
	FRAME_swipeB9,  PlayerMoveUpperFunc, -58 * PHYS_SCALER, 0, 0, NULL, 0, NULL,
	FRAME_swipeB10, PlayerMoveUpperFunc, -51 * PHYS_SCALER, 0, 0, NULL, 0, NULL, // End trail
	FRAME_swipeB11, PlayerMoveUpperFunc, -51 * PHYS_SCALER, 0, 0, NULL, 0, NULL,
	FRAME_swipeB12, PlayerMoveUpperFunc, -38 * PHYS_SCALER, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_staffatkstep2 = { 9, player_frames_staffatkstep2, PlayerAnimUpperUpdate };

// Standing attack with one swipe from left to right.
static panimframe_t player_frames_staffatkstep[] =
{
	FRAME_swipeB1,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_swipeB2,  PlayerMoveUpperFunc,  30 * PHYS_SCALER, 0, 0, PlayerActionStaffTrailStart, TRAIL_STEP, NULL,
	FRAME_swipeB4,  PlayerMoveUpperFunc,  91 * PHYS_SCALER, 0, 0, PlayerActionSwordAttack, 5, NULL,
	FRAME_swipeB5,  PlayerMoveUpperFunc,  37 * PHYS_SCALER, 0, 0, PlayerActionSwordAttack, 6, NULL,
	FRAME_swipeB6,  PlayerMoveUpperFunc,  27 * PHYS_SCALER, 0, 0, PlayerActionSwordAttack, 7, NULL,
	FRAME_swipeB7,  PlayerMoveUpperFunc,  13 * PHYS_SCALER, 0, 0, PlayerActionSwordAttack, 8, NULL,
	FRAME_swipeB9,  PlayerMoveUpperFunc, -58 * PHYS_SCALER, 0, 0, NULL, 0, NULL,
	FRAME_swipeB10, PlayerMoveUpperFunc, -51 * PHYS_SCALER, 0, 0, NULL, 0, NULL, // End trail
	FRAME_swipeB11, PlayerMoveUpperFunc, -51 * PHYS_SCALER, 0, 0, NULL, 0, NULL,
	FRAME_swipeB12, PlayerMoveUpperFunc, -38 * PHYS_SCALER, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_staffatkstep = { 10, player_frames_staffatkstep, PlayerAnimUpperUpdate };

static panimframe_t player_frames_staffatkback[] =
{
	FRAME_roundbck1, NULL, 0, 0, 0, PlayerActionStaffTrailStart, TRAIL_BACK, NULL,
	FRAME_roundbck2, NULL, 0, 0, 0, PlayerActionSwordAttack, 17, NULL,
	FRAME_roundbck3, NULL, 0, 0, 0, PlayerActionSwordAttack, 18, NULL,
	FRAME_roundbck4, NULL, 0, 0, 0, PlayerActionSwordAttack, 19, NULL,
	FRAME_roundbck5, NULL, 0, 0, 0, NULL, 0, NULL, // End trail
	FRAME_roundbck6, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_roundbck7, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_roundbck8, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_staffatkback = { 8, player_frames_staffatkback, PlayerAnimUpperUpdate };

// Stab downward.
static panimframe_t player_frames_staffdownstab[] =
{
	FRAME_spikedwn1, NULL, 0, 0, 0, PlayerActionStaffTrailStart, TRAIL_STAB, NULL,
	FRAME_spikedwn2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_spikedwn3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_spikedwn4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_spikedwn5, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_spikedwn6, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_spikedwn7, NULL, 0, 0, 0, PlayerActionSwordAttack, 21, NULL,
	FRAME_spikedwn8, NULL, 0, 0, 0, PlayerActionSwordAttack, 22, NULL,
};
panimmove_t player_move_staffdownstab = { 8, player_frames_staffdownstab, PlayerAnimUpperUpdate };

static panimframe_t player_frames_staffstabhold[] =
{
	FRAME_spikedwn8, NULL, 0, 0, 0, PlayerActionSwordAttack, 22, NULL,
};
panimmove_t player_move_staffstabhold = { 1, player_frames_staffstabhold, PlayerAnimUpperUpdate };

static panimframe_t player_frames_staffpullout[] =
{
	FRAME_pullout1,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_pullout2,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_pullout3,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_pullout4,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_pullout5,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_pullout6,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_pullout7,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_pullout8,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_pullout9,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_pullout10, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_pullout11, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_staffpullout = { 11, player_frames_staffpullout, PlayerAnimUpperUpdate };

// Stab downward
static panimframe_t player_frames_stafflowerdownstab[] =
{
	FRAME_spikedwn1, NULL, 0, 0, 0, PlayerActionStaffTrailStart, TRAIL_STAB, NULL,
	FRAME_spikedwn2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_spikedwn3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_spikedwn4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_spikedwn5, PlayerMoveFunc, 25 * PHYS_SCALER, 0, 0, NULL, 0, NULL,
	FRAME_spikedwn6, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_spikedwn7, NULL, 0, 0, 0, PlayerActionSwordAttack, 21, NULL,
	FRAME_spikedwn8, NULL, 0, 0, 0, PlayerActionSwordAttack, 22, NULL,
};
panimmove_t player_move_stafflowerdownstab = { 8, player_frames_stafflowerdownstab, PlayerAnimLowerUpdate };

// Pull staff out after stab.
static panimframe_t player_frames_stafflowerpullout[] =
{
	FRAME_pullout1,  NULL, 0, 0, 0, PlayerActionSwordAttack, 22, NULL,
	FRAME_pullout2,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_pullout3,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_pullout4,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_pullout5,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_pullout6,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_pullout7,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_pullout8,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_pullout9,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_pullout10, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_pullout11, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_stafflowerpullout = { 11, player_frames_stafflowerpullout, PlayerAnimLowerUpdate };

// Do block to player's left.
static panimframe_t player_frames_staffblockleft[] =
{
	FRAME_swipeA4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_swipeA2, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_staffblockleft = { 2, player_frames_staffblockleft, PlayerAnimUpperUpdate };

// Do block to player's left.
static panimframe_t player_frames_staffblockleft2[] =
{
	FRAME_swipeA1, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_staffblockleft2 = { 1, player_frames_staffblockleft2, PlayerAnimUpperUpdate };

// Block on left and attack.
static panimframe_t player_frames_staffblockleftatk[] =
{
	FRAME_swipeB2,  PlayerMoveUpperFunc,  30 * PHYS_SCALER, 0, 0, PlayerActionStaffTrailStart, TRAIL_COUNTERLEFT, NULL,
	FRAME_swipeB4,  PlayerMoveUpperFunc,  91 * PHYS_SCALER, 0, 0, PlayerActionSwordAttack, 5, NULL,
	FRAME_swipeB5,  PlayerMoveUpperFunc,  37 * PHYS_SCALER, 0, 0, PlayerActionSwordAttack, 6, NULL,
	FRAME_swipeB6,  PlayerMoveUpperFunc,  27 * PHYS_SCALER, 0, 0, PlayerActionSwordAttack, 7, NULL,
	FRAME_swipeB7,  PlayerMoveUpperFunc,  13 * PHYS_SCALER, 0, 0, PlayerActionSwordAttack, 8, NULL,
	FRAME_swipeB9,  PlayerMoveUpperFunc, -58 * PHYS_SCALER, 0, 0, NULL, 0, NULL,
	FRAME_swipeB10, PlayerMoveUpperFunc, -51 * PHYS_SCALER, 0, 0, NULL, 0, NULL, // End trail
	FRAME_swipeB11, PlayerMoveUpperFunc, -51 * PHYS_SCALER, 0, 0, NULL, 0, NULL,
	FRAME_swipeB12, PlayerMoveUpperFunc, -38 * PHYS_SCALER, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_staffblockleftatk = { 9, player_frames_staffblockleftatk, PlayerAnimUpperUpdate };

// Do block to player's right.
static panimframe_t player_frames_staffblockright[] =
{
	FRAME_swipeB5, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_swipeB2, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_staffblockright = { 2, player_frames_staffblockright, PlayerAnimUpperUpdate };

// Do block to player's right.
static panimframe_t player_frames_staffblockright2[] =
{
	FRAME_swipeB1, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_staffblockright2 = { 1, player_frames_staffblockright2, PlayerAnimUpperUpdate };

// Block on right and attack
static panimframe_t player_frames_staffblockrightatk[] =
{
	FRAME_swipeA2, PlayerMoveUpperFunc,  30 * PHYS_SCALER, 0, 0, PlayerActionStaffTrailStart, TRAIL_COUNTERRIGHT, NULL,
	FRAME_swipeA3, PlayerMoveUpperFunc,  91 * PHYS_SCALER, 0, 0, PlayerActionSwordAttack, 1, NULL,
	FRAME_swipeA5, PlayerMoveUpperFunc,  37 * PHYS_SCALER, 0, 0, PlayerActionSwordAttack, 2, NULL,
	FRAME_swipeA6, PlayerMoveUpperFunc,  27 * PHYS_SCALER, 0, 0, PlayerActionSwordAttack, 3, NULL,
	FRAME_swipeA7, PlayerMoveUpperFunc,  13 * PHYS_SCALER, 0, 0, PlayerActionSwordAttack, 4, NULL,
	FRAME_swipeA8, PlayerMoveUpperFunc, -58 * PHYS_SCALER, 0, 0, NULL, 0, NULL,
	FRAME_swipeA9, PlayerMoveUpperFunc, -51 * PHYS_SCALER, 0, 0, NULL, 0, NULL, // End trail
};
panimmove_t player_move_staffblockrightatk = { 7, player_frames_staffblockrightatk, PlayerAnimUpperUpdate };

// Blocked swipe left.
static panimframe_t player_frames_staffblockedleft[] =
{
	FRAME_swipeA4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_swipeA3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_swipeA3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_swipeA3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_swipeA2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_swipeA1, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_staffblockedleft = { 6, player_frames_staffblockedleft, PlayerAnimUpperUpdate };

// Blocked swipe right.
static panimframe_t player_frames_staffblockedright[] =
{
	FRAME_swipeB2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_swipeB3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_swipeB3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_swipeB3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_swipeB2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_swipeB1, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_staffblockedright = { 6, player_frames_staffblockedright, PlayerAnimUpperUpdate };

#pragma endregion

#pragma region ========================== PLAYER CREEP ANIMATIONS ==========================

static panimframe_t player_frames_creepforward[] =
{
	FRAME_creep1,  PlayerMoveFunc, 89 * PHYS_SCALER, 0, 0, NULL, 0, PlayerActionCheckCreep,
	FRAME_creep2,  PlayerMoveFunc, 58 * PHYS_SCALER, 0, 0, NULL, 0, PlayerActionCheckCreep,
	FRAME_creep3,  PlayerMoveFunc, 63 * PHYS_SCALER, 0, 0, NULL, 0, PlayerActionCheckCreep,
	FRAME_creep4,  PlayerMoveFunc, 66 * PHYS_SCALER, 0, 0, NULL, 0, PlayerActionCheckCreep,
	FRAME_creep5,  PlayerMoveFunc, 69 * PHYS_SCALER, 0, 0, NULL, 0, PlayerActionCheckCreep,
	FRAME_creep6,  PlayerMoveFunc, 75 * PHYS_SCALER, 0, 0, NULL, 0, PlayerActionCheckCreep,
	FRAME_creep7,  PlayerMoveFunc, 90 * PHYS_SCALER, 0, 0, NULL, 0, PlayerActionCheckCreep,
	FRAME_creep8,  PlayerMoveFunc, 67 * PHYS_SCALER, 0, 0, NULL, 0, PlayerActionCheckCreep,
	FRAME_creep9,  PlayerMoveFunc, 60 * PHYS_SCALER, 0, 0, NULL, 0, PlayerActionCheckCreep,
	FRAME_creep10, PlayerMoveFunc, 62 * PHYS_SCALER, 0, 0, NULL, 0, PlayerActionCheckCreep,
	FRAME_creep11, PlayerMoveFunc, 70 * PHYS_SCALER, 0, 0, NULL, 0, PlayerActionCheckCreep,
	FRAME_creep12, PlayerMoveFunc, 66 * PHYS_SCALER, 0, 0, NULL, 0, PlayerActionCheckCreep,
};
panimmove_t player_move_creepforward = { 12, player_frames_creepforward, PlayerAnimLowerUpdate };

static panimframe_t player_frames_creepforward_end[] =
{
	FRAME_creep9, PlayerMoveFunc, 20 * PHYS_SCALER, 0, 0, NULL, 0, PlayerActionCheckCreep,
};
panimmove_t player_move_creepforward_end = { 1, player_frames_creepforward_end, PlayerAnimLowerUpdate };

// Creep back-left.
static panimframe_t player_frames_creepb_strafe_left[] =
{
	FRAME_Lcrepbck1,  PlayerMoveFunc, -63 * PHYS_SCALER, -63 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckCreepBackUnStrafe,
	FRAME_Lcrepbck2,  PlayerMoveFunc, -58 * PHYS_SCALER, -58 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckCreepBackUnStrafe,
	FRAME_Lcrepbck3,  PlayerMoveFunc, -89 * PHYS_SCALER, -89 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckCreepBackUnStrafe,
	FRAME_Lcrepbck4,  PlayerMoveFunc, -66 * PHYS_SCALER, -66 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckCreepBackUnStrafe,
	FRAME_Lcrepbck5,  PlayerMoveFunc, -70 * PHYS_SCALER, -70 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckCreepBackUnStrafe,
	FRAME_Lcrepbck6,  PlayerMoveFunc, -62 * PHYS_SCALER, -62 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckCreepBackUnStrafe,
	FRAME_Lcrepbck7,  PlayerMoveFunc, -62 * PHYS_SCALER, -62 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckCreepBackUnStrafe,
	FRAME_Lcrepbck8,  PlayerMoveFunc, -62 * PHYS_SCALER, -62 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckCreepBackUnStrafe,
	FRAME_Lcrepbck9,  PlayerMoveFunc, -62 * PHYS_SCALER, -62 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckCreepBackUnStrafe,
	FRAME_Lcrepbck10, PlayerMoveFunc, -62 * PHYS_SCALER, -62 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckCreepBackUnStrafe,
	FRAME_Lcrepbck11, PlayerMoveFunc, -62 * PHYS_SCALER, -62 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckCreepBackUnStrafe,
	FRAME_Lcrepbck12, PlayerMoveFunc, -62 * PHYS_SCALER, -62 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckCreepBackUnStrafe,
};
panimmove_t player_move_creepb_strafe_left = { 12, player_frames_creepb_strafe_left, PlayerAnimLowerUpdate };

// Creep back-right.
static panimframe_t player_frames_creepb_strafe_right[] =
{
	FRAME_Rcrepbck1,  PlayerMoveFunc, -63 * PHYS_SCALER, 63 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckCreepBackUnStrafe,
	FRAME_Rcrepbck2,  PlayerMoveFunc, -58 * PHYS_SCALER, 58 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckCreepBackUnStrafe,
	FRAME_Rcrepbck3,  PlayerMoveFunc, -89 * PHYS_SCALER, 89 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckCreepBackUnStrafe,
	FRAME_Rcrepbck4,  PlayerMoveFunc, -66 * PHYS_SCALER, 66 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckCreepBackUnStrafe,
	FRAME_Rcrepbck5,  PlayerMoveFunc, -70 * PHYS_SCALER, 70 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckCreepBackUnStrafe,
	FRAME_Rcrepbck6,  PlayerMoveFunc, -62 * PHYS_SCALER, 62 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckCreepBackUnStrafe,
	FRAME_Rcrepbck7,  PlayerMoveFunc, -62 * PHYS_SCALER, 62 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckCreepBackUnStrafe,
	FRAME_Rcrepbck8,  PlayerMoveFunc, -62 * PHYS_SCALER, 62 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckCreepBackUnStrafe,
	FRAME_Rcrepbck9,  PlayerMoveFunc, -62 * PHYS_SCALER, 62 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckCreepBackUnStrafe,
	FRAME_Rcrepbck10, PlayerMoveFunc, -62 * PHYS_SCALER, 62 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckCreepBackUnStrafe,
	FRAME_Rcrepbck11, PlayerMoveFunc, -62 * PHYS_SCALER, 62 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckCreepBackUnStrafe,
	FRAME_Rcrepbck12, PlayerMoveFunc, -62 * PHYS_SCALER, 62 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckCreepBackUnStrafe,
};
panimmove_t player_move_creepb_strafe_right = { 12, player_frames_creepb_strafe_right, PlayerAnimLowerUpdate };

// Creep left.
static panimframe_t player_frames_creep_strafe_left[] =
{
	FRAME_creepL1,  PlayerMoveFunc, 89 * PHYS_SCALER, -89 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckCreepUnStrafe,
	FRAME_creepL2,  PlayerMoveFunc, 58 * PHYS_SCALER, -58 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckCreepUnStrafe,
	FRAME_creepL3,  PlayerMoveFunc, 63 * PHYS_SCALER, -63 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckCreepUnStrafe,
	FRAME_creepL4,  PlayerMoveFunc, 66 * PHYS_SCALER, -66 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckCreepUnStrafe,
	FRAME_creepL5,  PlayerMoveFunc, 69 * PHYS_SCALER, -69 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckCreepUnStrafe,
	FRAME_creepL6,  PlayerMoveFunc, 75 * PHYS_SCALER, -75 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckCreepUnStrafe,
	FRAME_creepL7,  PlayerMoveFunc, 90 * PHYS_SCALER, -90 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckCreepUnStrafe,
	FRAME_creepL8,  PlayerMoveFunc, 67 * PHYS_SCALER, -67 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckCreepUnStrafe,
	FRAME_creepL9,  PlayerMoveFunc, 60 * PHYS_SCALER, -60 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckCreepUnStrafe,
	FRAME_creepL10, PlayerMoveFunc, 62 * PHYS_SCALER, -62 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckCreepUnStrafe,
	FRAME_creepL11, PlayerMoveFunc, 70 * PHYS_SCALER, -70 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckCreepUnStrafe,
	FRAME_creepL12, PlayerMoveFunc, 66 * PHYS_SCALER, -66 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckCreepUnStrafe,
};
panimmove_t player_move_creep_strafe_left = { 12, player_frames_creep_strafe_left, PlayerAnimLowerUpdate };

// Creep right.
static panimframe_t player_frames_creep_strafe_right[] =
{
	FRAME_creepR1,  PlayerMoveFunc, 89 * PHYS_SCALER, 89 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckCreepUnStrafe,
	FRAME_creepR2,  PlayerMoveFunc, 58 * PHYS_SCALER, 58 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckCreepUnStrafe,
	FRAME_creepR3,  PlayerMoveFunc, 63 * PHYS_SCALER, 63 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckCreepUnStrafe,
	FRAME_creepR4,  PlayerMoveFunc, 66 * PHYS_SCALER, 66 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckCreepUnStrafe,
	FRAME_creepR5,  PlayerMoveFunc, 69 * PHYS_SCALER, 69 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckCreepUnStrafe,
	FRAME_creepR6,  PlayerMoveFunc, 75 * PHYS_SCALER, 75 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckCreepUnStrafe,
	FRAME_creepR7,  PlayerMoveFunc, 90 * PHYS_SCALER, 90 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckCreepUnStrafe,
	FRAME_creepR8,  PlayerMoveFunc, 67 * PHYS_SCALER, 67 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckCreepUnStrafe,
	FRAME_creepR9,  PlayerMoveFunc, 60 * PHYS_SCALER, 60 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckCreepUnStrafe,
	FRAME_creepR10, PlayerMoveFunc, 62 * PHYS_SCALER, 62 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckCreepUnStrafe,
	FRAME_creepR11, PlayerMoveFunc, 70 * PHYS_SCALER, 70 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckCreepUnStrafe,
	FRAME_creepR12, PlayerMoveFunc, 66 * PHYS_SCALER, 66 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckCreepUnStrafe,
};
panimmove_t player_move_creep_strafe_right = { 12, player_frames_creep_strafe_right, PlayerAnimLowerUpdate };

#pragma endregion

#pragma region ========================== PLAYER CROUCH-CREEP ANIMATIONS ==========================

// Crouch-creep forward.
static panimframe_t player_frames_crouch_creep_forward[] =
{
	FRAME_crhpvt1, PlayerMoveFunc, 70, 0, 0, NULL, 0, NULL,
	FRAME_crhpvt2, PlayerMoveFunc, 90, 0, 0, NULL, 0, NULL,
	FRAME_crhpvt3, PlayerMoveFunc, 90, 0, 0, NULL, 0, NULL,
	FRAME_crhpvt4, PlayerMoveFunc, 70, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_crouch_creep_forward = { 4, player_frames_crouch_creep_forward, PlayerAnimLowerUpdate };

// Crouch-creep back.
static panimframe_t player_frames_crouch_creep_back[] =
{
	FRAME_crhpvt4, PlayerMoveFunc, -70, 0, 0, NULL, 0, NULL,
	FRAME_crhpvt3, PlayerMoveFunc, -90, 0, 0, NULL, 0, NULL,
	FRAME_crhpvt2, PlayerMoveFunc, -90, 0, 0, NULL, 0, NULL,
	FRAME_crhpvt1, PlayerMoveFunc, -70, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_crouch_creep_back = { 4, player_frames_crouch_creep_back, PlayerAnimLowerUpdate };

// Crouch-creep left.
static panimframe_t player_frames_crouch_creep_left[] =
{
	FRAME_crhpvt1, PlayerMoveFunc, 0, -70, 0, NULL, 0, NULL,
	FRAME_crhpvt2, PlayerMoveFunc, 0, -90, 0, NULL, 0, NULL,
	FRAME_crhpvt3, PlayerMoveFunc, 0, -90, 0, NULL, 0, NULL,
	FRAME_crhpvt4, PlayerMoveFunc, 0, -70, 0, NULL, 0, NULL,
};
panimmove_t player_move_crouch_creep_left = { 4, player_frames_crouch_creep_left, PlayerAnimLowerUpdate };

static panimframe_t player_frames_crouch_creep_right[] =
{
	FRAME_crhpvt4, PlayerMoveFunc, 0, 70, 0, NULL, 0, NULL,
	FRAME_crhpvt3, PlayerMoveFunc, 0, 90, 0, NULL, 0, NULL,
	FRAME_crhpvt2, PlayerMoveFunc, 0, 90, 0, NULL, 0, NULL,
	FRAME_crhpvt1, PlayerMoveFunc, 0, 70, 0, NULL, 0, NULL,
};
panimmove_t player_move_crouch_creep_right = { 4, player_frames_crouch_creep_right, PlayerAnimLowerUpdate };

#pragma endregion

#pragma region ========================== PLAYER JOG ANIMATIONS ==========================

// Start jogging.
static panimframe_t player_frames_walkstart[] =
{
	FRAME_gorun2, PlayerMoveFunc, 80 * PHYS_SCALER, 0, 0, NULL, 0, PlayerActionCheckWalk,
};
panimmove_t player_move_walkstart = { 1, player_frames_walkstart, PlayerAnimLowerUpdate };

// First half of jog cycle.
static panimframe_t player_frames_walk[] =
{
	FRAME_jog1, PlayerMoveFunc, 160 * PHYS_SCALER, 0, 0, PlayerActionFootstep, STEP_WALK, PlayerActionCheckWalk,
	FRAME_jog2, PlayerMoveFunc, 170 * PHYS_SCALER, 0, 0, NULL, 0, PlayerActionCheckWalk,
	FRAME_jog3, PlayerMoveFunc, 160 * PHYS_SCALER, 0, 0, NULL, 0, PlayerActionCheckWalk,
	FRAME_jog4, PlayerMoveFunc, 155 * PHYS_SCALER, 0, 0, NULL, 0, PlayerActionCheckWalk,
	FRAME_jog5, PlayerMoveFunc, 160 * PHYS_SCALER, 0, 0, PlayerActionFootstep, STEP_WALK2, PlayerActionCheckWalk,
	FRAME_jog6, PlayerMoveFunc, 170 * PHYS_SCALER, 0, 0, NULL, 0, PlayerActionCheckWalk,
	FRAME_jog7, PlayerMoveFunc, 160 * PHYS_SCALER, 0, 0, NULL, 0, PlayerActionCheckWalk,
	FRAME_jog8, PlayerMoveFunc, 155 * PHYS_SCALER, 0, 0, NULL, 0, PlayerActionCheckWalk,
};
panimmove_t player_move_walk = { 8, player_frames_walk, PlayerAnimLowerUpdate };

// Used if stopping at first half of jog cycle.
static panimframe_t player_frames_walkstop[] =
{
	FRAME_jog1, PlayerMoveFunc, 80 * PHYS_SCALER, 0, 0, PlayerActionFootstep, STEP_WALK, PlayerActionCheckWalk,
};
panimmove_t player_move_walkstop = { 1, player_frames_walkstop, PlayerAnimLowerUpdate };

#pragma endregion

#pragma region ========================== PLAYER RUN ANIMATIONS ==========================

// Start the run cycle.
static panimframe_t player_frames_runstart[] =
{
	FRAME_gorun1, PlayerMoveFunc,  60 * PHYS_SCALER, 0, 0, NULL, 0, PlayerActionCheckRun,
	FRAME_gorun2, PlayerMoveFunc, 120 * PHYS_SCALER, 0, 0, NULL, 0, PlayerActionCheckRun,
};
panimmove_t player_move_runstart = { 2, player_frames_runstart, PlayerAnimLowerUpdate };

// The run cycle.
static panimframe_t player_frames_run[] =
{
	FRAME_run1, PlayerMoveFunc, 340 * PHYS_SCALER, 0, 0, PlayerActionFootstep, STEP_RUN, PlayerActionCheckRun,
	FRAME_run2, PlayerMoveFunc, 340 * PHYS_SCALER, 0, 0, NULL, 0, PlayerActionCheckRun,
	FRAME_run3, PlayerMoveFunc, 340 * PHYS_SCALER, 0, 0, NULL, 0, PlayerActionCheckRun,
	FRAME_run4, PlayerMoveFunc, 340 * PHYS_SCALER, 0, 0, PlayerActionFootstep, STEP_RUN2, PlayerActionCheckRun,
	FRAME_run5, PlayerMoveFunc, 340 * PHYS_SCALER, 0, 0, NULL, 0, PlayerActionCheckRun,
	FRAME_run6, PlayerMoveFunc, 340 * PHYS_SCALER, 0, 0, NULL, 0, PlayerActionCheckRun,
};
panimmove_t player_move_run = { 6, player_frames_run, PlayerAnimLowerUpdate };

// End the run cycle.
static panimframe_t player_frames_runstop[] =
{
	FRAME_jog1, PlayerMoveFunc, 120 * PHYS_SCALER, 0, 0, PlayerActionFootstep, STEP_RUN, PlayerActionCheckRun,
};
panimmove_t player_move_runstop = { 1, player_frames_runstop, PlayerAnimLowerUpdate };

#pragma endregion

#pragma region ========================== PLAYER MOVE BACKWARDS ANIMATIONS ==========================

// Creep backwards loop.
static panimframe_t player_frames_creepback[] =
{
	FRAME_crepbak1,  PlayerMoveFunc, -63 * PHYS_SCALER, 0, 0, NULL, 0, PlayerActionCheckCreepBack,
	FRAME_crepbak2,  PlayerMoveFunc, -58 * PHYS_SCALER, 0, 0, NULL, 0, PlayerActionCheckCreepBack,
	FRAME_crepbak3,  PlayerMoveFunc, -89 * PHYS_SCALER, 0, 0, NULL, 0, PlayerActionCheckCreepBack,
	FRAME_crepbak4,  PlayerMoveFunc, -66 * PHYS_SCALER, 0, 0, NULL, 0, PlayerActionCheckCreepBack,
	FRAME_crepbak5,  PlayerMoveFunc, -70 * PHYS_SCALER, 0, 0, NULL, 0, PlayerActionCheckCreepBack,
	FRAME_crepbak6,  PlayerMoveFunc, -62 * PHYS_SCALER, 0, 0, NULL, 0, PlayerActionCheckCreepBack,
	FRAME_crepbak7,  PlayerMoveFunc, -62 * PHYS_SCALER, 0, 0, NULL, 0, PlayerActionCheckCreepBack,
	FRAME_crepbak8,  PlayerMoveFunc, -62 * PHYS_SCALER, 0, 0, NULL, 0, PlayerActionCheckCreepBack,
	FRAME_crepbak9,  PlayerMoveFunc, -62 * PHYS_SCALER, 0, 0, NULL, 0, PlayerActionCheckCreepBack,
	FRAME_crepbak10, PlayerMoveFunc, -62 * PHYS_SCALER, 0, 0, NULL, 0, PlayerActionCheckCreepBack,
	FRAME_crepbak11, PlayerMoveFunc, -62 * PHYS_SCALER, 0, 0, NULL, 0, PlayerActionCheckCreepBack,
	FRAME_crepbak12, PlayerMoveFunc, -62 * PHYS_SCALER, 0, 0, NULL, 0, PlayerActionCheckCreepBack,
};
panimmove_t player_move_creepback = { 12, player_frames_creepback, PlayerAnimLowerUpdate };

// Creep backwards end.
static panimframe_t player_frames_creepback_end[] =
{
	FRAME_crepbak12, PlayerMoveFunc, -30 * PHYS_SCALER, 0, 0, NULL, 0, PlayerActionCheckCreepBack,
};
panimmove_t player_move_creepback_end = { 1, player_frames_creepback_end, PlayerAnimLowerUpdate };

// Jogging backwards loop.
static panimframe_t player_frames_walkback[] =
{
	FRAME_jogback1,	PlayerMoveFunc,	-150 * PHYS_SCALER, 0, 0, PlayerActionFootstep, STEP_WALK, PlayerActionCheckWalkBack,
	FRAME_jogback2, PlayerMoveFunc, -175 * PHYS_SCALER, 0, 0, NULL, 0, PlayerActionCheckWalkBack,
	FRAME_jogback3, PlayerMoveFunc, -150 * PHYS_SCALER, 0, 0, NULL, 2, PlayerActionCheckWalkBack,
	FRAME_jogback4,	PlayerMoveFunc,	-150 * PHYS_SCALER, 0, 0, PlayerActionFootstep, STEP_WALK2, PlayerActionCheckWalkBack,
	FRAME_jogback5, PlayerMoveFunc, -175 * PHYS_SCALER, 0, 0, NULL, 0, PlayerActionCheckWalkBack,
	FRAME_jogback6,	PlayerMoveFunc,	-150 * PHYS_SCALER, 0, 0, NULL, 2, PlayerActionCheckWalkBack,
};
panimmove_t player_move_walkback = { 6, player_frames_walkback, PlayerAnimLowerUpdate };

#pragma endregion

#pragma region ========================== PLAYER STRAFE ANIMATIONS ==========================

// Move sideways left loop.
static panimframe_t player_frames_strafeleft[] =
{
	FRAME_Lstep1, PlayerMoveFunc, 0, -125 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckStrafe,
	FRAME_Lstep2, PlayerMoveFunc, 0, -150 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckStrafe,
	FRAME_Lstep3, PlayerMoveFunc, 0, -175 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckStrafe,
	FRAME_Lstep4, PlayerMoveFunc, 0, -150 * PHYS_SCALER, 0, PlayerActionFootstep, STEP_WALK,  PlayerActionCheckStrafe,
	FRAME_Lstep5, PlayerMoveFunc, 0, -125 * PHYS_SCALER, 0, PlayerActionFootstep, STEP_WALK2, PlayerActionCheckStrafe,
};
panimmove_t player_move_strafeleft = { 5, player_frames_strafeleft, PlayerAnimLowerUpdate };

// Move sideways left end.
static panimframe_t player_frames_strafeleft_end[] =
{
	FRAME_Lstep5, PlayerMoveFunc, 0, -100 * PHYS_SCALER, 0, PlayerActionFootstep, STEP_WALK2, NULL,
};
panimmove_t player_move_strafeleft_end = { 1, player_frames_strafeleft_end, PlayerAnimLowerUpdate };

// Move sideways right loop.
static panimframe_t player_frames_straferight[] =
{
	FRAME_Rstep1, PlayerMoveFunc, 0, 125 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckStrafe,
	FRAME_Rstep2, PlayerMoveFunc, 0, 150 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckStrafe,
	FRAME_Rstep3, PlayerMoveFunc, 0, 175 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckStrafe,
	FRAME_Rstep4, PlayerMoveFunc, 0, 150 * PHYS_SCALER, 0, PlayerActionFootstep, STEP_WALK,  PlayerActionCheckStrafe,
	FRAME_Rstep5, PlayerMoveFunc, 0, 125 * PHYS_SCALER, 0, PlayerActionFootstep, STEP_WALK2, PlayerActionCheckStrafe,
};
panimmove_t player_move_straferight = { 5, player_frames_straferight, PlayerAnimLowerUpdate };

// Move sideways right end.
static panimframe_t player_frames_straferight_end[] =
{
	FRAME_Rstep5, PlayerMoveFunc, 0, 100 * PHYS_SCALER, 0, PlayerActionFootstep, STEP_WALK2, NULL,
};
panimmove_t player_move_straferight_end = { 1, player_frames_straferight_end, PlayerAnimLowerUpdate };

#pragma endregion

#pragma region ========================== PLAYER JUMP ANIMATIONS ==========================

// Crouching down to jump.
static panimframe_t player_frames_standjumpstart[] =
{
	FRAME_jump1, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_jump3, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_standjumpstart = { 2, player_frames_standjumpstart, PlayerAnimLowerUpdate };

// First frame to jump forward from standing position.
static panimframe_t player_frames_standjumpfwdstart[] =
{
	FRAME_jump5, PlayerJumpMoveForce, 120 * PHYS_SCALER, 0, 0, PlayerActionJump, 120, NULL,
};
panimmove_t player_move_standjumpfwdstart = { 1, player_frames_standjumpfwdstart, PlayerAnimLowerUpdate };

// First frame to jump forward from jogging.
static panimframe_t player_frames_walkjumpfwdstart[] =
{
	FRAME_jump5, PlayerJumpMoveForce, 200, 0, 0, PlayerActionJump, 250, NULL,
};
panimmove_t player_move_walkjumpfwdstart = { 1, player_frames_walkjumpfwdstart, PlayerAnimLowerUpdate };

// First frame to jump froward from running.
static panimframe_t player_frames_runjumpfwdstart[] =
{
	FRAME_jump5, PlayerJumpMoveForce, 250, 0, 0, PlayerActionJump, 300, NULL,
};
panimmove_t player_move_runjumpfwdstart = { 1, player_frames_runjumpfwdstart, PlayerAnimLowerUpdate };

// Jumping forward (used by jogging and running jumps).
static panimframe_t player_frames_jumpfwd[] =
{
	FRAME_jump6,  PlayerJumpNudge, 64, 0, 0, PlayerActionCheckGrab, 0, NULL,
	FRAME_jump7,  PlayerJumpNudge, 64, 0, 0, PlayerActionCheckGrab, 0, NULL,
	FRAME_jump8,  PlayerJumpNudge, 64, 0, 0, PlayerActionCheckGrab, 0, PlayerActionCheckDoubleJump,
	FRAME_jump9,  PlayerJumpNudge, 64, 0, 0, PlayerActionCheckGrab, 0, PlayerMoveAdd,
	FRAME_jump10, PlayerJumpNudge, 64, 0, 0, PlayerActionCheckGrab, 0, PlayerMoveAdd,
	FRAME_jump11, PlayerJumpNudge, 64, 0, 0, PlayerActionCheckGrab, 0, PlayerMoveAdd,
	FRAME_jump12, PlayerJumpNudge, 64, 0, 0, PlayerActionCheckGrab, 0, PlayerMoveAdd,
	FRAME_jump13, PlayerJumpNudge, 64, 0, 0, PlayerActionCheckGrab, 0, PlayerMoveAdd,
	FRAME_jump14, PlayerJumpNudge, 64, 0, 0, PlayerActionCheckGrab, 0, PlayerMoveAdd,
	FRAME_jump15, PlayerJumpNudge, 64, 0, 0, PlayerActionCheckGrab, 0, PlayerMoveAdd,
	FRAME_jump16, PlayerJumpNudge, 64, 0, 0, PlayerActionCheckGrab, 0, PlayerMoveAdd,
	FRAME_jump17, PlayerJumpNudge, 64, 0, 0, PlayerActionCheckGrab, 0, PlayerMoveAdd,
	FRAME_jump18, PlayerJumpNudge, 64, 0, 0, PlayerActionCheckGrab, 0, PlayerMoveAdd,
	FRAME_jump19, PlayerJumpNudge, 64, 0, 0, PlayerActionCheckGrab, 0, PlayerMoveAdd,
};
panimmove_t player_move_jumpfwd = { 14, player_frames_jumpfwd, PlayerAnimLowerUpdate };

// Jumping straight up.
static panimframe_t player_frames_jumpup[] =
{
	FRAME_grab5, NULL, 0, 0, 0, PlayerActionJump, 350, PlayerMoveAdd,
};
panimmove_t player_move_jumpup = { 1, player_frames_jumpup, PlayerAnimLowerUpdate };

// Moves player forward a little and starts forward jump again.
static panimframe_t player_frames_jumpuploop[] =
{
	FRAME_grab5, PlayerMoveALittle, 10, 0, 0, PlayerActionCheckGrab, 0, PlayerMoveAdd,
};
panimmove_t player_move_jumpuploop = { 1, player_frames_jumpuploop, NULL };

// First frame to jump backwards from standing position.
static panimframe_t player_frames_standjumpbackstart[] =
{
	FRAME_jumpback2, PlayerJumpMoveForce, -150, 0, 0, PlayerActionJump, 250, NULL,
};
panimmove_t player_move_standjumpbackstart = { 1, player_frames_standjumpbackstart, PlayerAnimLowerUpdate };

// First frame to jump backwards from jogging.
static panimframe_t player_frames_walkjumpbackstart[] =
{
	FRAME_jumpback2, PlayerJumpMoveForce, -200, 0, 0, PlayerActionJump, 300, NULL,
};
panimmove_t player_move_walkjumpbackstart = { 1, player_frames_walkjumpbackstart, PlayerAnimLowerUpdate };

// First frame to jump backwards from running.
static panimframe_t player_frames_runjumpbackstart[] =
{
	FRAME_jumpback2, PlayerJumpMoveForce, -300, 0, 0, PlayerActionJumpBack, 350, NULL,
};
panimmove_t player_move_runjumpbackstart = { 1, player_frames_runjumpbackstart, PlayerAnimLowerUpdate };

// Jumping backwards (used by jogging and running jumps).
panimframe_t player_frames_jumpback[] =
{
	FRAME_jumpback3,  PlayerJumpNudge, -64, 0, 0, NULL, 0, NULL,
	FRAME_jumpback4,  PlayerJumpNudge, -64, 0, 0, NULL, 0, NULL,
	FRAME_jumpback5,  PlayerJumpNudge, -64, 0, 0, NULL, 0, PlayerActionCheckDoubleJump,
	FRAME_jumpback6,  PlayerJumpNudge, -64, 0, 0, NULL, 0, PlayerMoveAdd,
	FRAME_jumpback8,  PlayerJumpNudge, -64, 0, 0, NULL, 0, PlayerMoveAdd,
	FRAME_jumpback10, PlayerJumpNudge, -64, 0, 0, NULL, 0, PlayerMoveAdd,
	FRAME_jumpback12, PlayerJumpNudge, -64, 0, 0, NULL, 0, PlayerMoveAdd,
	FRAME_jumpback13, PlayerJumpNudge, -64, 0, 0, NULL, 0, PlayerMoveAdd,
	FRAME_jumpback14, PlayerJumpNudge, -64, 0, 0, PlayerActionCheckGrab, 0, PlayerMoveAdd,
	FRAME_jumpback16, PlayerJumpNudge, -64, 0, 0, PlayerActionCheckGrab, 0, PlayerMoveAdd,
	FRAME_jumpback18, PlayerJumpNudge, -64, 0, 0, PlayerActionCheckGrab, 0, PlayerMoveAdd,
	FRAME_jumpback20, PlayerJumpNudge, -64, 0, 0, PlayerActionCheckGrab, 0, PlayerMoveAdd,
	FRAME_jumpback22, PlayerJumpNudge, -64, 0, 0, PlayerActionCheckGrab, 0, PlayerMoveAdd,
	FRAME_jumpback24, PlayerJumpNudge, -64, 0, 0, PlayerActionCheckGrab, 0, PlayerMoveAdd,
};
panimmove_t player_move_jumpback = { 14, player_frames_jumpback, PlayerAnimLowerUpdate };

// Backwards-flip jump.
static panimframe_t player_frames_jumpflipback[] =
{
	FRAME_bkflip17, PlayerJumpNudge, -64, 0, 0, PlayerActionFlip, 100, NULL,
	FRAME_bkflip18, PlayerJumpNudge, -64, 0, 0, NULL, 0, NULL,
	FRAME_bkflip19, PlayerJumpNudge, -64, 0, 0, NULL, 0, NULL,
	FRAME_bkflip20, PlayerJumpNudge, -64, 0, 0, NULL, 0, NULL,
	FRAME_bkflip21, PlayerJumpNudge, -64, 0, 0, NULL, 0, NULL,
	FRAME_bkflip22, PlayerJumpNudge, -64, 0, 0, PlayerActionCheckGrab, 0, NULL,
	FRAME_bkflip23, PlayerJumpNudge, -64, 0, 0, PlayerActionCheckGrab, 0, NULL,
	FRAME_bkflip24, PlayerJumpNudge, -64, 0, 0, PlayerActionCheckGrab, 0, NULL,
	FRAME_bkflip25, PlayerJumpNudge, -64, 0, 0, PlayerActionCheckGrab, 0, NULL,
};
panimmove_t player_move_jumpbackflip = { 9, player_frames_jumpflipback, PlayerAnimLowerUpdate };

// First frame to jump left from standing position.
static panimframe_t player_frames_standjumpleftstart[] =
{
	FRAME_jumpleft5, PlayerJumpMoveForce, 0, -150, 0, PlayerActionJump, 100, NULL,
};
panimmove_t player_move_standjumpleftstart = { 1, player_frames_standjumpleftstart, PlayerAnimLowerUpdate };

// First frame to jump left from jogging.
static panimframe_t player_frames_walkjumpleftstart[] =
{
	FRAME_jumpleft5, PlayerJumpMoveForce, 0, -250, 0, PlayerActionJump, 100, NULL,
};
panimmove_t player_move_walkjumpleftstart = { 1, player_frames_walkjumpleftstart, PlayerAnimLowerUpdate };

// First frame to jump left from running.
static panimframe_t player_frames_runjumpleftstart[] =
{
	FRAME_jumpleft5, PlayerJumpMoveForce, 0,-300, 0, PlayerActionJump, 100, NULL,
};
panimmove_t player_move_runjumpleftstart = { 1, player_frames_runjumpleftstart, PlayerAnimLowerUpdate };

// Jumping left (used by jogging and running jumps).
static panimframe_t player_frames_jumpleft[] =
{
	FRAME_jumpleft6,  PlayerJumpNudge, 0, -64, 0, NULL, 0, NULL,
	FRAME_jumpleft7,  PlayerJumpNudge, 0, -64, 0, NULL, 0, NULL,
	FRAME_jumpleft8,  PlayerJumpNudge, 0, -64, 0, NULL, 0, PlayerActionCheckDoubleJump,
	FRAME_jumpleft9,  PlayerJumpNudge, 0, -64, 0, NULL, 0, PlayerMoveAdd,
	FRAME_jumpleft10, PlayerJumpNudge, 0, -64, 0, NULL, 0, PlayerMoveAdd,
	FRAME_jumpleft11, PlayerJumpNudge, 0, -64, 0, NULL, 0, PlayerMoveAdd,
	FRAME_jumpleft12, PlayerJumpNudge, 0, -64, 0, NULL, 0, PlayerMoveAdd,
	FRAME_jumpleft13, PlayerJumpNudge, 0, -64, 0, NULL, 0, PlayerMoveAdd,
	FRAME_jumpleft14, PlayerJumpNudge, 0, -64, 0, PlayerActionCheckGrab, 0, PlayerMoveAdd,
	FRAME_jumpleft16, PlayerJumpNudge, 0, -64, 0, PlayerActionCheckGrab, 0, PlayerMoveAdd,
	FRAME_jumpleft18, PlayerJumpNudge, 0, -64, 0, PlayerActionCheckGrab, 0, PlayerMoveAdd,
	FRAME_jumpleft20, PlayerJumpNudge, 0, -64, 0, PlayerActionCheckGrab, 0, PlayerMoveAdd,
	FRAME_jumpleft22, PlayerJumpNudge, 0, -64, 0, PlayerActionCheckGrab, 0, PlayerMoveAdd,
	FRAME_jumpleft24, PlayerJumpNudge, 0, -64, 0, PlayerActionCheckGrab, 0, PlayerMoveAdd,
};
panimmove_t player_move_jumpleft = { 14, player_frames_jumpleft, PlayerAnimLowerUpdate };

// Left-flip jump.
static panimframe_t player_frames_jumpleftflip[] =
{
	FRAME_Lflip2,  PlayerJumpNudge, 0, -64, 0, PlayerActionFlip, 100, NULL,
	FRAME_Lflip3,  PlayerJumpNudge, 0, -64, 0, NULL, 0, NULL,
	FRAME_Lflip4,  PlayerJumpNudge, 0, -64, 0, NULL, 0, NULL,
	FRAME_Lflip5,  PlayerJumpNudge, 0, -64, 0, NULL, 0, NULL,
	FRAME_Lflip6,  PlayerJumpNudge, 0, -64, 0, NULL, 0, NULL,
	FRAME_Lflip7,  PlayerJumpNudge, 0, -64, 0, NULL, 0, NULL,
	FRAME_Lflip8,  PlayerJumpNudge, 0, -64, 0, PlayerActionCheckGrab, 0, NULL,
	FRAME_Lflip9,  PlayerJumpNudge, 0, -64, 0, PlayerActionCheckGrab, 0, NULL,
	FRAME_Lflip10, PlayerJumpNudge, 0, -64, 0, PlayerActionCheckGrab, 0, NULL,
	FRAME_Lflip11, PlayerJumpNudge, 0, -64, 0, PlayerActionCheckGrab, 0, NULL,
	FRAME_Lflip12, PlayerJumpNudge, 0, -64, 0, PlayerActionCheckGrab, 0, NULL,
	FRAME_Lflip13, PlayerJumpNudge, 0, -64, 0, PlayerActionCheckGrab, 0, NULL,
	FRAME_Lflip14, PlayerJumpNudge, 0, -64, 0, PlayerActionCheckGrab, 0, NULL,
};
panimmove_t player_move_jumpleftflip = { 13, player_frames_jumpleftflip, PlayerAnimLowerUpdate };

// First frame to jump right from standing position.
static panimframe_t player_frames_standjumprightstart[] =
{
	FRAME_jumprite5, PlayerJumpMoveForce, 0, 150, 0, PlayerActionJump, 200, NULL,
};
panimmove_t player_move_standjumprightstart = { 1, player_frames_standjumprightstart, PlayerAnimLowerUpdate };

// First frame to jump right from jogging.
static panimframe_t player_frames_walkjumprightstart[] =
{
	FRAME_jumprite5, PlayerJumpMoveForce, 0, 250, 0, PlayerActionJump, 250, NULL,
};
panimmove_t player_move_walkjumprightstart = { 1, player_frames_walkjumprightstart, PlayerAnimLowerUpdate };

// First frame to jump right from running.
static panimframe_t player_frames_runjumprightstart[] =
{
	FRAME_jumprite5, PlayerJumpMoveForce, 0, 300, 0, PlayerActionJump, 300, NULL,
};
panimmove_t player_move_runjumprightstart = { 1, player_frames_runjumprightstart, PlayerAnimLowerUpdate };

// Jumping right (used by jogging and running jumps).
static panimframe_t player_frames_jumpright[] =
{
	FRAME_jumprite6,  PlayerJumpNudge,  0, 64, 0, NULL, 0, NULL,
	FRAME_jumprite7,  PlayerJumpNudge,  0, 64, 0, NULL, 0, NULL,
	FRAME_jumprite8,  PlayerJumpNudge,  0, 64, 0, NULL, 0, PlayerActionCheckDoubleJump,
	FRAME_jumprite9,  PlayerJumpNudge,  0, 64, 0, NULL, 0, PlayerMoveAdd,
	FRAME_jumprite10, PlayerJumpNudge,  0, 64, 0, NULL, 0, PlayerMoveAdd,
	FRAME_jumprite11, PlayerJumpNudge,  0, 64, 0, NULL, 0, PlayerMoveAdd,
	FRAME_jumprite12, PlayerJumpNudge,  0, 64, 0, NULL, 0, PlayerMoveAdd,
	FRAME_jumprite13, PlayerJumpNudge,  0, 64, 0, NULL, 0, PlayerMoveAdd,
	FRAME_jumprite14, PlayerJumpNudge,  0, 64, 0, PlayerActionCheckGrab, 0, PlayerMoveAdd,
	FRAME_jumprite16, PlayerJumpNudge,  0, 64, 0, PlayerActionCheckGrab, 0, PlayerMoveAdd,
	FRAME_jumprite18, PlayerJumpNudge,  0, 64, 0, PlayerActionCheckGrab, 0, PlayerMoveAdd,
	FRAME_jumprite20, PlayerJumpNudge,  0, 64, 0, PlayerActionCheckGrab, 0, PlayerMoveAdd,
	FRAME_jumprite22, PlayerJumpNudge,  0, 64, 0, PlayerActionCheckGrab, 0, PlayerMoveAdd,
	FRAME_jumprite24, PlayerJumpNudge,  0, 64, 0, PlayerActionCheckGrab, 0, PlayerMoveAdd,
};
panimmove_t player_move_jumpright = { 14, player_frames_jumpright, PlayerAnimLowerUpdate };

// Right-flip jump.
static panimframe_t player_frames_jumprightflip[] =
{
		FRAME_Rflip2,  PlayerJumpNudge, 0, 64, 0, PlayerActionFlip, 100, NULL,
		FRAME_Rflip3,  PlayerJumpNudge, 0, 64, 0, NULL, 0, NULL,
		FRAME_Rflip4,  PlayerJumpNudge, 0, 64, 0, NULL, 0, NULL,
		FRAME_Rflip5,  PlayerJumpNudge, 0, 64, 0, NULL, 0, NULL,
		FRAME_Rflip6,  PlayerJumpNudge, 0, 64, 0, NULL, 0, NULL,
		FRAME_Rflip7,  PlayerJumpNudge, 0, 64, 0, NULL, 0, NULL,
		FRAME_Rflip8,  PlayerJumpNudge, 0, 64, 0, PlayerActionCheckGrab, 0, NULL,
		FRAME_Rflip9,  PlayerJumpNudge, 0, 64, 0, PlayerActionCheckGrab, 0, NULL,
		FRAME_Rflip10, PlayerJumpNudge, 0, 64, 0, PlayerActionCheckGrab, 0, NULL,
		FRAME_Rflip11, PlayerJumpNudge, 0, 64, 0, PlayerActionCheckGrab, 0, NULL,
		FRAME_Rflip12, PlayerJumpNudge, 0, 64, 0, PlayerActionCheckGrab, 0, NULL,
		FRAME_Rflip13, PlayerJumpNudge, 0, 64, 0, PlayerActionCheckGrab, 0, NULL,
		FRAME_Rflip14, PlayerJumpNudge, 0, 64, 0, PlayerActionCheckGrab, 0, NULL,
};
panimmove_t player_move_jumprightflip = { 13, player_frames_jumprightflip, PlayerAnimLowerUpdate };

#pragma endregion

#pragma region ========================== PLAYER FALLING ANIMATIONS ==========================

// Falling with arms up.
static panimframe_t player_frames_fall[] =
{
	FRAME_falling1,  NULL, 0, 0, 0, PlayerActionCheckFallingGrab, 0, NULL,
	FRAME_falling2,  NULL, 0, 0, 0, PlayerActionCheckFallingGrab, 0, NULL,
	FRAME_falling3,  NULL, 0, 0, 0, PlayerActionCheckFallingGrab, 0, NULL,
	FRAME_falling4,  NULL, 0, 0, 0, PlayerActionCheckFallingGrab, 0, NULL,
	FRAME_falling5,  NULL, 0, 0, 0, PlayerActionCheckFallingGrab, 0, NULL,
	FRAME_falling6,  NULL, 0, 0, 0, PlayerActionCheckFallingGrab, 0, NULL,
	FRAME_falling7,  NULL, 0, 0, 0, PlayerActionCheckFallingGrab, 0, NULL,
	FRAME_falling8,  NULL, 0, 0, 0, PlayerActionCheckFallingGrab, 0, NULL,
	FRAME_falling9,  NULL, 0, 0, 0, PlayerActionCheckFallingGrab, 0, NULL,
	FRAME_falling10, NULL, 0, 0, 0, PlayerActionCheckFallingGrab, 0, NULL,
	FRAME_falling11, NULL, 0, 0, 0, PlayerActionCheckFallingGrab, 0, NULL,
	FRAME_falling12, NULL, 0, 0, 0, PlayerActionCheckFallingGrab, 0, NULL,
	FRAME_falling13, NULL, 0, 0, 0, PlayerActionCheckFallingGrab, 0, NULL,
	FRAME_falling14, NULL, 0, 0, 0, PlayerActionCheckFallingGrab, 0, NULL,
	FRAME_falling15, NULL, 0, 0, 0, PlayerActionCheckFallingGrab, 0, NULL,
};
panimmove_t player_move_fall = { 15, player_frames_fall, PlayerAnimLowerUpdate };

// Falling with arms above head.
panimframe_t player_frames_fallarmsup[] =
{
	FRAME_grab5, NULL, 0, 0, 0, PlayerActionCheckFallingGrab, 0, NULL,
};
panimmove_t player_move_fallarmsup = { 1, player_frames_fallarmsup, PlayerAnimLowerUpdate };

// Transition from walking to falling.
static panimframe_t player_frames_fallwalkstart[] =
{
	FRAME_drop1, NULL, 0, 0, 0, PlayerActionCheckFallingGrab, 0, NULL,
	FRAME_drop2, NULL, 0, 0, 0, PlayerActionCheckFallingGrab, 0, NULL,
	FRAME_drop3, NULL, 0, 0, 0, PlayerActionCheckFallingGrab, 0, NULL,
	FRAME_drop4, NULL, 0, 0, 0, PlayerActionCheckFallingGrab, 0, NULL,
};
panimmove_t player_move_fallwalkstart = { 4, player_frames_fallwalkstart, PlayerAnimLowerUpdate };

// One frame used while falling with arms down.
static panimframe_t player_frames_fallwalkloop[] =
{
	FRAME_drop4, NULL, 0, 0, 0, PlayerActionCheckFallingGrab, 0, NULL,
};
panimmove_t player_move_fallwalkloop = { 1, player_frames_fallwalkloop, NULL };

#pragma endregion

#pragma region ========================== PLAYER LANDING ANIMATIONS ==========================

// Transition from standing with legs together to idle stance.
static panimframe_t player_frames_land1[] =
{
	FRAME_recover4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_recover5, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_recover6, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_recover7, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_land1 = { 4, player_frames_land1, PlayerAnimLowerUpdate };

// Recovering from a fall to going into idle stance.
static panimframe_t player_frames_land3[] =
{
	FRAME_recover1, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_recover2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_recover3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_recover4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_recover5, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_recover6, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_recover7, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_land3 = { 7, player_frames_land3, PlayerAnimLowerUpdate };

#pragma endregion

#pragma region ========================== PLAYER CROUCHING ANIMATIONS ==========================

// One frame, preparing to jump and roll forward.
static panimframe_t player_frames_crouchdown[] =
{
	FRAME_rollA1, NULL, 0, 0, 0, NULL, 0, PlayerActionSetCrouchHeight,
};
panimmove_t player_move_crouchdown = { 1, player_frames_crouchdown, PlayerAnimLowerUpdate };

// One frame, in a kneeling position.
static panimframe_t player_frames_crouch[] =
{
	FRAME_rollA15, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_crouch = { 1, player_frames_crouch, PlayerAnimLowerUpdate };

// One frame, coming up out of crouch.
static panimframe_t player_frames_crouchup[] =
{
	FRAME_rollA1, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_crouchup = { 1, player_frames_crouchup, PlayerAnimLowerUpdate };

#pragma endregion

#pragma region ========================== PLAYER ROLL DIVE ANIMATIONS ==========================

// From jog, dive forward into a roll.
static panimframe_t player_frames_rolldivefwdwalk[] =
{
	FRAME_rollA1,  PlayerMoveFunc, 170 * PHYS_SCALER, 0, 0, NULL, 0, PlayerActionSetCrouchHeight,
	FRAME_rollA3,  PlayerMoveFunc, 170 * PHYS_SCALER, 0, 0, NULL, 0, NULL,
	FRAME_rollA5,  PlayerMoveFunc, 170 * PHYS_SCALER, 0, 0, PlayerActionFootstep, STEP_ROLL, NULL,
	FRAME_rollA7,  PlayerMoveFunc, 170 * PHYS_SCALER, 0, 0, NULL, 0, NULL,
	FRAME_rollA8,  PlayerMoveFunc, 170 * PHYS_SCALER, 0, 0, SpawnDustPuff, 0, NULL,
	FRAME_rollA10, PlayerMoveFunc, 170 * PHYS_SCALER, 0, 0, NULL, 0, NULL,
	FRAME_rollA11, PlayerMoveFunc, 170 * PHYS_SCALER, 0, 0, NULL, 0, NULL,
	FRAME_rollA13, PlayerMoveFunc, 150 * PHYS_SCALER, 0, 0, NULL, 0, NULL,
	FRAME_rollA14, PlayerMoveFunc, 115 * PHYS_SCALER, 0, 0, NULL, 0, NULL,
	FRAME_rollA15, PlayerMoveFunc,  50 * PHYS_SCALER, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_rolldivefwdwalk = { 10, player_frames_rolldivefwdwalk, PlayerAnimLowerUpdate };

// From run, dive forward into a roll.
static panimframe_t player_frames_rolldivefwdrun[] =
{
	FRAME_rollA1,  PlayerMoveFunc, 340 * PHYS_SCALER, 0, 0, NULL, 0, PlayerActionSetCrouchHeight,
	FRAME_rollA3,  PlayerMoveFunc, 340 * PHYS_SCALER, 0, 0, NULL, 0, NULL,
	FRAME_rollA5,  PlayerMoveFunc, 320 * PHYS_SCALER, 0, 0, PlayerActionFootstep, STEP_ROLL, NULL,
	FRAME_rollA7,  PlayerMoveFunc, 280 * PHYS_SCALER, 0, 0, NULL, 0, NULL,
	FRAME_rollA8,  PlayerMoveFunc, 260 * PHYS_SCALER, 0, 0, SpawnDustPuff, 0, NULL,
	FRAME_rollA10, PlayerMoveFunc, 220 * PHYS_SCALER, 0, 0, NULL, 0, NULL,
	FRAME_rollA11, PlayerMoveFunc, 200 * PHYS_SCALER, 0, 0, NULL, 0, NULL,
	FRAME_rollA13, PlayerMoveFunc, 160 * PHYS_SCALER, 0, 0, NULL, 0, NULL,
	FRAME_rollA14, PlayerMoveFunc, 120 * PHYS_SCALER, 0, 0, NULL, 0, NULL,
	FRAME_rollA15, PlayerMoveFunc,  60 * PHYS_SCALER, 0, 0, NULL, 0, NULL,

};
panimmove_t player_move_rolldivefwdrun = { 10, player_frames_rolldivefwdrun, PlayerAnimLowerUpdate };

#pragma endregion

#pragma region ========================== PLAYER POLE VAULT ANIMATIONS ==========================

// Transition from jog to pole vault.
static panimframe_t player_frames_polevault1walk[] =
{
	FRAME_vault1, PlayerMoveFunc, 180, 0, 0, NULL, 0, NULL,
	FRAME_vault2, PlayerMoveFunc, 200, 0, 0, PlayerActionVaultSound, 0, NULL,
};
panimmove_t player_move_polevault1walk = { 2, player_frames_polevault1walk, PlayerAnimLowerUpdate };

// Transition from run to pole vault.
static panimframe_t player_frames_polevault1run[] =
{
	FRAME_vault1, PlayerMoveFunc, 300, 0, 0, NULL, 0, NULL,
	FRAME_vault2, PlayerMoveFunc, 300, 0, 0, PlayerActionVaultSound, 0, NULL,
};
panimmove_t player_move_polevault1run = { 2, player_frames_polevault1run, PlayerAnimLowerUpdate };

// Pole vault animation.
static panimframe_t player_frames_polevault2[] =
{
	FRAME_vault3,  PlayerMoveForce, 300, 0, 0, PlayerActionJump, 250, NULL,
	FRAME_vault4,  PlayerJumpNudge, 64,  0, 0, NULL, 0, NULL,
	FRAME_vault5,  PlayerJumpNudge, 64,  0, 0, NULL, 0, PlayerActionCheckVaultKick,
	FRAME_vault6,  PlayerJumpNudge, 64,  0, 0, NULL, 0, PlayerActionCheckVaultKick,
	FRAME_vault7,  PlayerJumpNudge, 64,  0, 0, NULL, 0, PlayerActionCheckVaultKick,
	FRAME_vault8,  PlayerJumpNudge, 64,  0, 0, NULL, 0, PlayerActionCheckVaultKick,
	FRAME_vault9,  PlayerJumpNudge, 64,  0, 0, PlayerActionCheckGrab, 0, NULL,
	FRAME_vault10, PlayerJumpNudge, 64,  0, 0, PlayerActionCheckGrab, 0, NULL,
	FRAME_vault11, PlayerJumpNudge, 64,  0, 0, PlayerActionCheckGrab, 0, NULL,
	FRAME_vault12, PlayerJumpNudge, 64,  0, 0, PlayerActionCheckGrab, 0, NULL,
	FRAME_vault13, PlayerJumpNudge, 64,  0, 0, PlayerActionCheckGrab, 0, NULL,
	FRAME_vault14, PlayerJumpNudge, 64,  0, 0, PlayerActionCheckGrab, 0, NULL,
	FRAME_vault15, PlayerJumpNudge, 64,  0, 0, PlayerActionCheckGrab, 0, NULL,
	FRAME_vault16, PlayerJumpNudge, 64,  0, 0, PlayerActionCheckGrab, 0, NULL,
	FRAME_vault17, PlayerJumpNudge, 64,  0, 0, PlayerActionCheckGrab, 0, NULL,
};
panimmove_t player_move_polevault2 = { 15, player_frames_polevault2, PlayerAnimLowerUpdate };

#pragma endregion

#pragma region ========================== PLAYER SWIM ANIMATIONS ==========================

// Idle swim animation.
static panimframe_t player_frames_sswimidle[] =
{
	FRAME_idlswm1,  PlayerMoveFunc, 0, 0, 0, PlayerActionSwimIdleSound, 0, PlayerAnimLowerIdle,
	FRAME_idlswm2,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_idlswm3,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_idlswm4,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_idlswm5,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_idlswm6,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_idlswm7,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_idlswm8,  NULL, 0, 0, 0, PlayerActionSwim, 0, PlayerAnimLowerIdle,
	FRAME_idlswm9,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_idlswm10, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_idlswm11, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_idlswm12, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_idlswm13, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_idlswm14, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_idlswm15, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
};
panimmove_t player_move_sswimidle = { 15, player_frames_sswimidle, NULL };

// Transition from swim idle to swim forward.
static panimframe_t player_frames_sswimfwdgo[] =
{
	FRAME_t4swim1, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_t4swim2, PlayerSwimMoveFunc, 90, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_sswimfwdgo = { 2, player_frames_sswimfwdgo, PlayerAnimLowerUpdate };

// Forward swim animation.
static panimframe_t player_frames_sswimfwd[] =
{
	FRAME_4swim1, PlayerSwimMoveFunc, 120, 0, 0, PlayerActionSwimSound, SOUND_SWIM_FORWARD, NULL,
	FRAME_4swim2, PlayerSwimMoveFunc, 150, 0, 0, NULL, 0, NULL,
	FRAME_4swim3, PlayerSwimMoveFunc, 160, 0, 0, NULL, 0, NULL,
	FRAME_4swim4, PlayerSwimMoveFunc, 180, 0, 0, NULL, 0, NULL,
	FRAME_4swim5, PlayerSwimMoveFunc, 190, 0, 0, PlayerActionSwim, 0, NULL,
	FRAME_4swim6, PlayerSwimMoveFunc, 200, 0, 0, NULL, 0, NULL,
	FRAME_4swim7, PlayerSwimMoveFunc, 190, 0, 0, NULL, 0, NULL,
	FRAME_4swim8, PlayerSwimMoveFunc, 180, 0, 0, NULL, 0, NULL,
	FRAME_4swim9, PlayerSwimMoveFunc, 160, 0, 0, NULL, 0, NULL,
	FRAME_4swim10,PlayerSwimMoveFunc, 140, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_sswimfwd = { 10, player_frames_sswimfwd, PlayerAnimLowerUpdate };

// Transition from swim forward to swim idle.
static panimframe_t player_frames_sswimfwdstop[] =
{
	FRAME_t4swim2, PlayerSwimMoveFunc, 40, 0, 0, NULL, 0, NULL,
	FRAME_t4swim1, PlayerSwimMoveFunc, 0,  0, 0, NULL, 0, NULL,
};
panimmove_t player_move_sswimfwdstop = { 2, player_frames_sswimfwdstop, PlayerAnimLowerUpdate };

// Transition from swim idle to swim backward.
static panimframe_t player_frames_sswimbackgo[] =
{
	FRAME_tbswim1, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_tbswim2, PlayerSwimMoveFunc, -25, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_sswimbackgo = { 2, player_frames_sswimbackgo, PlayerAnimLowerUpdate };

// Backwards swim animation.
static panimframe_t player_frames_sswimback[] =
{
	FRAME_bkswim1, PlayerSwimMoveFunc, -70,  0, 0, PlayerActionSwimSound, SOUND_SWIM_BACK, NULL,
	FRAME_bkswim2, PlayerSwimMoveFunc, -100, 0, 0, NULL, 0, NULL,
	FRAME_bkswim3, PlayerSwimMoveFunc, -130, 0, 0, NULL, 0, NULL,
	FRAME_bkswim4, PlayerSwimMoveFunc, -150, 0, 0, PlayerActionSwim, 0, NULL,
	FRAME_bkswim5, PlayerSwimMoveFunc, -130, 0, 0, NULL, 0, NULL,
	FRAME_bkswim6, PlayerSwimMoveFunc, -100, 0, 0, NULL, 0, NULL,
	FRAME_bkswim7, PlayerSwimMoveFunc, -70,  0, 0, NULL, 0, NULL,
};
panimmove_t player_move_sswimback = { 7, player_frames_sswimback, PlayerAnimLowerUpdate };

// Transition from swim backwards to swim idle.
static panimframe_t player_frames_sswimbackstop[] =
{
	FRAME_tbswim2, PlayerSwimMoveFunc, -30, 0, 0, NULL, 0, NULL,
	FRAME_tbswim1, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_sswimbackstop = { 2, player_frames_sswimbackstop, PlayerAnimLowerUpdate };

// Transition from swim idle to swim left.
static panimframe_t player_frames_sswim_left_go[] =
{
	FRAME_tswimL1, PlayerMoveFunc, 0, -25,  0, NULL, 0, NULL,
	FRAME_tswimL2, PlayerMoveFunc, 0, -50,  0, NULL, 0, NULL,
	FRAME_tswimL3, PlayerMoveFunc, 0, -75,  0, PlayerActionSwimSound, SOUND_SWIM_SIDE, NULL,
	FRAME_tswimL4, PlayerMoveFunc, 0, -100, 0, NULL, 0, NULL,
};
panimmove_t	player_move_sswim_left_go = { 4, player_frames_sswim_left_go, PlayerAnimLowerUpdate };

// Transition from swim idle to swim right.
static panimframe_t player_frames_sswim_right_go[] =
{
	FRAME_tswimR1, PlayerMoveFunc, 0, 25,  0, NULL, 0, NULL,
	FRAME_tswimR2, PlayerMoveFunc, 0, 50,  0, NULL, 0, NULL,
	FRAME_tswimR3, PlayerMoveFunc, 0, 75,  0, PlayerActionSwimSound, SOUND_SWIM_SIDE, NULL,
	FRAME_tswimR4, PlayerMoveFunc, 0, 100, 0, NULL, 0, NULL,
};
panimmove_t	player_move_sswim_right_go = { 4, player_frames_sswim_right_go, PlayerAnimLowerUpdate };

// Swim left animation.
static panimframe_t player_frames_sswim_left[] =
{
	FRAME_swimL1, PlayerMoveFunc, 0, -250, 0, NULL, 0, NULL,
	FRAME_swimL2, PlayerMoveFunc, 0, -225, 0, NULL, 0, NULL,
	FRAME_swimL3, PlayerMoveFunc, 0, -200, 0, PlayerActionSwimSound, SOUND_SWIM_SIDE, NULL,
	FRAME_swimL4, PlayerMoveFunc, 0, -150, 0, NULL, 0, NULL,
	FRAME_swimL5, PlayerMoveFunc, 0, -100, 0, NULL, 0, NULL,
};
panimmove_t	player_move_sswim_left = { 5, player_frames_sswim_left, PlayerAnimLowerUpdate };

// Swim right animation.
static panimframe_t player_frames_sswim_right[] =
{
	FRAME_swimR1, PlayerMoveFunc, 0, 250, 0, NULL, 0, NULL,
	FRAME_swimR2, PlayerMoveFunc, 0, 225, 0, NULL, 0, NULL,
	FRAME_swimR3, PlayerMoveFunc, 0, 200, 0, PlayerActionSwimSound, SOUND_SWIM_SIDE, NULL,
	FRAME_swimR4, PlayerMoveFunc, 0, 150, 0, NULL, 0, NULL,
	FRAME_swimR5, PlayerMoveFunc, 0, 100, 0, NULL, 0, NULL,
};
panimmove_t	player_move_sswim_right = { 5, player_frames_sswim_right, PlayerAnimLowerUpdate };

// Transition from swim left to swim idle.
static panimframe_t player_frames_sswim_left_stop[] =
{
	FRAME_tswimL4, PlayerMoveFunc, 0, -100, 0, NULL, 0, NULL,
	FRAME_tswimL3, PlayerMoveFunc, 0, -75, 0,  NULL, 0, NULL,
	FRAME_tswimL2, PlayerMoveFunc, 0, -50, 0,  NULL, 0, NULL,
	FRAME_tswimL1, PlayerMoveFunc, 0, -25, 0,  NULL, 0, NULL,
};
panimmove_t	player_move_sswim_left_stop = { 4, player_frames_sswim_left_stop, PlayerAnimLowerUpdate };

// Transition from swim right to swim idle.
static panimframe_t player_frames_sswim_right_stop[] =
{
	FRAME_tswimR4, PlayerMoveFunc, 0, 100, 0, NULL, 0, NULL,
	FRAME_tswimR3, PlayerMoveFunc, 0, 75,  0, NULL, 0, NULL,
	FRAME_tswimR2, PlayerMoveFunc, 0, 50,  0, NULL, 0, NULL,
	FRAME_tswimR1, PlayerMoveFunc, 0, 25,  0, NULL, 0, NULL,
};
panimmove_t	player_move_sswim_right_stop = { 4, player_frames_sswim_right_stop, PlayerAnimLowerUpdate };

// Fast swim start.
static panimframe_t player_frames_swim_fast_go[] =
{
	FRAME_gofast1, PlayerSwimMoveFunc, 64,  0, 0, NULL, 0, NULL,
	FRAME_gofast2, PlayerSwimMoveFunc, 120, 0, 0, NULL, 0, NULL,
	FRAME_gofast3, PlayerSwimMoveFunc, 180, 0, 0, NULL, 0, NULL,
	FRAME_gofast4, PlayerSwimMoveFunc, 240, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_swim_fast_go = { 4, player_frames_swim_fast_go, PlayerAnimLowerUpdate };

// Fast swim loop.
static panimframe_t player_frames_swim_fast[] =
{
	FRAME_swmfast1, PlayerSwimMoveFunc, 240, 0, 0, NULL, 0, NULL,
	FRAME_swmfast2, PlayerSwimMoveFunc, 240, 0, 0, PlayerActionSwimSound, SOUND_SWIM_SIDE, NULL,
	FRAME_swmfast3, PlayerSwimMoveFunc, 240, 0, 0, NULL, 0, NULL,
	FRAME_swmfast4, PlayerSwimMoveFunc, 240, 0, 0, NULL, 0, NULL,
	FRAME_swmfast5, PlayerSwimMoveFunc, 240, 0, 0, PlayerActionSwimSound, SOUND_SWIM_SIDE, NULL,
	FRAME_swmfast6, PlayerSwimMoveFunc, 240, 0, 0, NULL, 0, NULL,
	FRAME_swmfast7, PlayerSwimMoveFunc, 240, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_swim_fast = { 7, player_frames_swim_fast, PlayerAnimLowerUpdate };

#pragma endregion

#pragma region ========================== PLAYER UNDERWATER SWIM ANIMATIONS ==========================

// Transition from swim idle to swim forward.
static panimframe_t player_frames_uswimfwd_go[] =
{
	FRAME_resurf1, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t	player_move_uswimfwd_go = { 1, player_frames_uswimfwd_go, PlayerAnimLowerUpdate };

// Underwater forward swim animation.
static panimframe_t player_frames_uswimfwd[] =
{
	FRAME_undrswmA1, PlayerSwimMoveFunc, 400, 0, 0, PlayerActionSwimSound, SOUND_SWIM_UNDER, NULL,
	FRAME_undrswmA2, PlayerSwimMoveFunc, 350, 0, 0, NULL, 0, NULL,
	FRAME_undrswmA3, PlayerSwimMoveFunc, 300, 0, 0, NULL, 0, NULL,
	FRAME_undrswmA4, PlayerSwimMoveFunc, 250, 0, 0, NULL, 0, NULL,
	FRAME_undrswmA5, PlayerSwimMoveFunc, 650, 0, 0, NULL, 0, NULL, // Stroke!
	FRAME_undrswmA6, PlayerSwimMoveFunc, 600, 0, 0, NULL, 0, NULL,
	FRAME_undrswmA7, PlayerSwimMoveFunc, 550, 0, 0, NULL, 0, NULL,
	FRAME_undrswmA8, PlayerSwimMoveFunc, 500, 0, 0, NULL, 0, NULL,
	FRAME_undrswmA9, PlayerSwimMoveFunc, 450, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_uswimfwd = { 9, player_frames_uswimfwd, PlayerAnimLowerUpdate };

// Transition from swim forward to swim idle.
static panimframe_t player_frames_uswimfwd_end[] =
{
	FRAME_resurf1, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t	player_move_uswimfwd_end = { 1, player_frames_uswimfwd_end, PlayerAnimLowerUpdate };

// Transition from swim idle to swim backwards.
static panimframe_t player_frames_uswimbackgo[] =
{
	FRAME_tnsubckswm1, PlayerSwimMoveFunc, -75,  0, 0, NULL, 0, NULL,
	FRAME_tnsubckswm2, PlayerSwimMoveFunc, -125, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_uswimbackgo = { 2, player_frames_uswimbackgo, PlayerAnimLowerUpdate };

// Underwater backwards swim animation.
static panimframe_t player_frames_uswimback[] =
{
	FRAME_subckswm1, PlayerSwimMoveFunc, -170, 0, 0, PlayerActionSwimSound, SOUND_SWIM_UNDER, NULL,
	FRAME_subckswm2, PlayerSwimMoveFunc, -200, 0, 0, NULL, 0, NULL,
	FRAME_subckswm3, PlayerSwimMoveFunc, -230, 0, 0, NULL, 0, NULL,
	FRAME_subckswm4, PlayerSwimMoveFunc, -250, 0, 0, PlayerActionSwim, 0, NULL,
	FRAME_subckswm5, PlayerSwimMoveFunc, -230, 0, 0, NULL, 0, NULL,
	FRAME_subckswm6, PlayerSwimMoveFunc, -200, 0, 0, NULL, 0, NULL,
	FRAME_subckswm7, PlayerSwimMoveFunc, -170, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_uswimback = { 7, player_frames_uswimback, PlayerAnimLowerUpdate };

// Transition from swim backwards to swim idle.
static panimframe_t player_frames_uswimbackstop[] =
{
	FRAME_tnsubckswm1, PlayerSwimMoveFunc, -15, 0, 0, NULL, 0, NULL,
	FRAME_tnsubckswm1, PlayerSwimMoveFunc, -30, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_uswimbackstop = { 1, player_frames_uswimbackstop, PlayerAnimLowerUpdate };

// Transition from swim idle to swim left.
static panimframe_t player_frames_uswim_left_go[] =
{
	FRAME_trnswmL1, PlayerMoveFunc, 0, -50,  0, NULL, 0, NULL,
	FRAME_trnswmL2, PlayerMoveFunc, 0, -100, 0, PlayerActionSwimSound, SOUND_SWIM_UNDER, NULL,
};
panimmove_t	player_move_uswim_left_go = { 2, player_frames_uswim_left_go, PlayerAnimLowerUpdate };

// Transition from swim idle to swim right.
static panimframe_t player_frames_uswim_right_go[] =
{
	FRAME_trnswmR1, PlayerMoveFunc, 0, 50,  0, NULL, 0, NULL,
	FRAME_trnswmR2, PlayerMoveFunc, 0, 100, 0, PlayerActionSwimSound, SOUND_SWIM_UNDER, NULL,
};
panimmove_t	player_move_uswim_right_go = { 2, player_frames_uswim_right_go, PlayerAnimLowerUpdate };

// Underwater swim left animation.
static panimframe_t player_frames_uswim_left[] =
{
	FRAME_swmL1, PlayerMoveFunc, 0, -200, 0, NULL, 0, NULL,
	FRAME_swmL2, PlayerMoveFunc, 0, -220, 0, NULL, 0, NULL,
	FRAME_swmL3, PlayerMoveFunc, 0, -240, 0, NULL, 0, NULL,
	FRAME_swmL4, PlayerMoveFunc, 0, -250, 0, NULL, 0, NULL,
	FRAME_swmL5, PlayerMoveFunc, 0, -240, 0, PlayerActionSwimSound, SOUND_SWIM_UNDER, NULL,
	FRAME_swmL6, PlayerMoveFunc, 0, -220, 0, NULL, 0, NULL,
	FRAME_swmL7, PlayerMoveFunc, 0, -200, 0, NULL, 0, NULL,
	FRAME_swmL8, PlayerMoveFunc, 0, -150, 0, NULL, 0, NULL,
	FRAME_swmL9, PlayerMoveFunc, 0, -100, 0, NULL, 0, NULL,
};
panimmove_t	player_move_uswim_left = { 9, player_frames_uswim_left, PlayerAnimLowerUpdate };

// Underwater swim right animation.
static panimframe_t player_frames_uswim_right[] =
{
	FRAME_swmR1, PlayerMoveFunc, 0, 200, 0, NULL, 0, NULL,
	FRAME_swmR2, PlayerMoveFunc, 0, 220, 0, NULL, 0, NULL,
	FRAME_swmR3, PlayerMoveFunc, 0, 240, 0, NULL, 0, NULL,
	FRAME_swmR4, PlayerMoveFunc, 0, 250, 0, NULL, 0, NULL,
	FRAME_swmR5, PlayerMoveFunc, 0, 240, 0, PlayerActionSwimSound, SOUND_SWIM_UNDER, NULL,
	FRAME_swmR6, PlayerMoveFunc, 0, 220, 0, NULL, 0, NULL,
	FRAME_swmR7, PlayerMoveFunc, 0, 200, 0, NULL, 0, NULL,
	FRAME_swmR8, PlayerMoveFunc, 0, 150, 0, NULL, 0, NULL,
	FRAME_swmR9, PlayerMoveFunc, 0, 100, 0, NULL, 0, NULL,
};
panimmove_t	player_move_uswim_right = { 9, player_frames_uswim_right, PlayerAnimLowerUpdate };

// Transition from swim left to swim idle.
static panimframe_t player_frames_uswim_left_stop[] =
{
	FRAME_trnswmL2, PlayerMoveFunc, 0, -100, 0, PlayerActionSwimSound, SOUND_SWIM_UNDER, NULL,
	FRAME_trnswmL1, PlayerMoveFunc, 0, -50,  0, NULL, 0, NULL,
};
panimmove_t	player_move_uswim_left_stop = { 2, player_frames_uswim_left_stop, PlayerAnimLowerUpdate };

// Transition from swim right to swim idle.
static panimframe_t player_frames_uswim_right_stop[] =
{
	FRAME_trnswmR2, PlayerMoveFunc, 0, 100, 0, PlayerActionSwimSound, SOUND_SWIM_UNDER, NULL,
	FRAME_trnswmR1, PlayerMoveFunc, 0, 50,  0, NULL, 0, NULL,
};
panimmove_t	player_move_uswim_right_stop = { 2, player_frames_uswim_right_stop, PlayerAnimLowerUpdate };

// Underwater idle swim animation.
static panimframe_t player_frames_idle_under[] =
{
	FRAME_idlundr1,  PlayerMoveFunc, 0, 0, 0, PlayerActionSwimIdleSound, 0, PlayerAnimLowerIdle,
	FRAME_idlundr2,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_idlundr3,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_idlundr4,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_idlundr5,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_idlundr6,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_idlundr7,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_idlundr8,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_idlundr9,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_idlundr10, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_idlundr11, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_idlundr12, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_idlundr13, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_idlundr14, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_idlundr15, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_idlundr16, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_idlundr17, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_idlundr18, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_idlundr19, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_idlundr20, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
};
panimmove_t	player_move_idle_under = { 20, player_frames_idle_under, NULL };

// Underwater dive animation.
static panimframe_t player_frames_dive1[] =
{
	FRAME_subdive1, PlayerMoveFunc, 400, 0, -100, NULL, 0, NULL,
	FRAME_subdive2, PlayerMoveFunc, 350, 0, -50,  NULL, 0, NULL,
	FRAME_subdive3, PlayerMoveFunc, 300, 0, -50,  NULL, 0, NULL,
	FRAME_subdive4, PlayerMoveFunc, 250, 0, -25,  NULL, 0, NULL,
	FRAME_subdive5, PlayerMoveFunc, 200, 0, -5,   NULL, 0, NULL,
	FRAME_subdive6, PlayerMoveFunc, 150, 0, 0,    NULL, 0, NULL,
	FRAME_subdive7, PlayerMoveFunc, 125, 0, 0,    NULL, 0, NULL,
	FRAME_subdive8, PlayerMoveFunc, 100, 0, 0,    NULL, 0, NULL,
	FRAME_subdive9, PlayerMoveFunc, 80,  0, 0,    NULL, 0, NULL,
};
panimmove_t player_move_dive1 = { 9, player_frames_dive1, PlayerAnimLowerUpdate };

// Resurface animation.
static panimframe_t player_frames_resurface[] =
{
	FRAME_resurf1, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_resurf2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_resurf3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_resurf4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_resurf5, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t	player_move_resurface = { 5, player_frames_resurface, PlayerAnimLowerUpdate };

#pragma endregion

#pragma region ========================== PLAYER PULL UP / VAULT ANIMATIONs ==========================

// Transition, pulling himself up wall to standing.
static panimframe_t player_frames_pullupwall[] =
{
	FRAME_pullup1,  PlayerPullupHeight,  40, 0, 0, PlayerActionClimbStartSound, 0, NULL,
	FRAME_pullup2,  PlayerPullupHeight,  40, 0, 0, NULL, 0, NULL,
	FRAME_pullup3,  PlayerPullupHeight,  38, 0, 0, PlayerActionClimbWallSound, 0, NULL,
	FRAME_pullup4,  PlayerPullupHeight,  32, 0, 0, NULL, 0, NULL,
	FRAME_pullup5,  PlayerPullupHeight,  24, 0, 0, NULL, 0, NULL,
	FRAME_pullup6,  PlayerPullupHeight,  13, 0, 0, NULL, 0, NULL,
	FRAME_pullup7,  PlayerPullupHeight,  0,  0, 0, NULL, 0, NULL,
	FRAME_pullup8,  PlayerPullupHeight, -10, 0, 0, NULL, 0, NULL,
	FRAME_pullup9,  PlayerPullupHeight, -20, 0, 0, NULL, 0, NULL,
	FRAME_pullup10, PlayerPullupHeight, -38, 0, 0, NULL, 0, NULL,
	FRAME_pullup11, PlayerPullupHeight, -42, 1, 0, NULL, 0, NULL,
	FRAME_pullup12, PlayerPullupHeight, -44, 2, 0, PlayerActionClimbFinishSound, 0, NULL,
};
panimmove_t player_move_pullupwall = { 12, player_frames_pullupwall, PlayerAnimLowerUpdate };

// Pulling himself up a short wall.
static panimframe_t player_frames_pulluphalfwall[] =
{
	FRAME_Lhop1, PlayerPullupHeight, -32, 0, 1, NULL, 0, NULL,
	FRAME_Lhop2, PlayerPullupHeight, -40, 0, 1, NULL, 0, NULL,
	FRAME_Lhop3, PlayerPullupHeight, -44, 4, 1, PlayerActionClimbFinishSound, 0, NULL,
};
panimmove_t player_move_pulluphalfwall = { 3, player_frames_pulluphalfwall, PlayerAnimLowerUpdate };

// Pulling himself up a wall.
static panimframe_t player_frames_vaultwall[] =
{
	FRAME_pullup5,  PlayerPullupHeight,  24, 0, 0, PlayerActionClimbStartSound, 0, NULL,
	FRAME_pullup6,  PlayerPullupHeight,  13, 0, 0, NULL, 0, NULL,
	FRAME_pullup7,  PlayerPullupHeight,  8,  0, 0, NULL, 0, NULL,
	FRAME_pullup8,  PlayerPullupHeight, -8,  0, 0, NULL, 0, NULL,
	FRAME_pullup9,  PlayerPullupHeight, -32, 0, 0, NULL, 0, NULL,
	FRAME_pullup10, PlayerPullupHeight, -32, 0, 0, NULL, 0, NULL,
	FRAME_pullup11, PlayerPullupHeight, -36, 0, 0, NULL, 0, NULL,
	FRAME_pullup12, PlayerPullupHeight, -42, 2, 0, PlayerActionClimbFinishSound, 0, NULL,
};
panimmove_t player_move_vaultwall = { 10, player_frames_vaultwall, PlayerAnimLowerUpdate };

// Pulling himself up a wall. //TODO: player_frames_vaultwall duplicate. Remove?
static panimframe_t player_frames_vaulthigh[] =
{
	FRAME_pullup5,  PlayerPullupHeight,  24, 0, 0, PlayerActionClimbStartSound, 0, NULL,
	FRAME_pullup6,  PlayerPullupHeight,  13, 0, 0, NULL, 0, NULL,
	FRAME_pullup7,  PlayerPullupHeight,  8,  0, 0, NULL, 0, NULL,
	FRAME_pullup8,  PlayerPullupHeight, -8,  0, 0, NULL, 0, NULL,
	FRAME_pullup9,  PlayerPullupHeight, -32, 0, 0, NULL, 0, NULL,
	FRAME_pullup10, PlayerPullupHeight, -32, 0, 0, NULL, 0, NULL,
	FRAME_pullup11, PlayerPullupHeight, -36, 0, 0, NULL, 0, NULL,
	FRAME_pullup12, PlayerPullupHeight, -42, 2, 0, PlayerActionClimbFinishSound, 0, NULL,
};
panimmove_t player_move_vaulthigh = { 10, player_frames_vaulthigh, PlayerAnimLowerUpdate };

// Climb up from overhang.
static panimframe_t player_frames_overhang[] =
{
	FRAME_ovrhang3,  PlayerPullupHeight, 46,  0, 0, PlayerActionClimbStartSound, 0, NULL,
	FRAME_ovrhang4,  PlayerPullupHeight, 46,  0, 0, NULL, 0, NULL,
	FRAME_ovrhang5,  PlayerPullupHeight, 46,  0, 0, NULL, 0, NULL,
	FRAME_ovrhang6,  PlayerPullupHeight, 46,  0, 0, NULL, 0, NULL,
	FRAME_ovrhang7,  PlayerPullupHeight, 46,  0, 0, NULL, 0, NULL,
	FRAME_ovrhang8,  PlayerPullupHeight, 46,  0, 0, NULL, 0, NULL,
	FRAME_ovrhang9,  PlayerPullupHeight, 46,  0, 0, NULL, 0, NULL,
	FRAME_ovrhang10, PlayerPullupHeight, 32,  0, 0, NULL, 0, NULL, // Pullup begins.
	FRAME_ovrhang11, PlayerPullupHeight, 16,  0, 0, NULL, 0, NULL,
	FRAME_ovrhang12, PlayerPullupHeight,  8,  0, 0, NULL, 0, NULL,
	FRAME_ovrhang13, PlayerPullupHeight,  0,  0, 0, NULL, 0, NULL,
	FRAME_ovrhang14, PlayerPullupHeight,-16,  0, 0, NULL, 0, NULL,
	FRAME_ovrhang15, PlayerPullupHeight,-32,  0, 0, NULL, 0, NULL,
	FRAME_ovrhang16, PlayerPullupHeight,-40,  0, 0, NULL, 0, NULL,
	FRAME_ovrhang17, PlayerPullupHeight,-48,  2, 0, PlayerActionClimbFinishSound, 0, NULL,
};
panimmove_t player_move_overhang = { 15, player_frames_overhang, PlayerAnimLowerUpdate };

#pragma endregion

#pragma region ========================== PLAYER HANDSPRING ANIMATIONS ==========================

// Start of backwards handspring.
static panimframe_t player_frames_jumpfliphandspringgo[] =
{
	FRAME_bkflip4, PlayerMoveFunc, -60 * PHYS_SCALER, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_jumpfliphandspringgo = { 1, player_frames_jumpfliphandspringgo, PlayerAnimLowerUpdate };

// Cycle of handspring which can be run over and over.
static panimframe_t player_frames_jumpfliphandspring[] =
{
	FRAME_bkflip6,  PlayerMoveFunc, -380 * PHYS_SCALER, 0, 0, NULL, 0, NULL,
	FRAME_bkflip7,  PlayerMoveFunc, -340 * PHYS_SCALER, 0, 0, NULL, 0, NULL,
	FRAME_bkflip8,  PlayerMoveFunc, -300 * PHYS_SCALER, 0, 0, NULL, 0, NULL,
	FRAME_bkflip9,  PlayerMoveFunc, -220 * PHYS_SCALER, 0, 0, SpawnDustPuff, 0, NULL,
	FRAME_bkflip10, PlayerMoveFunc, -220 * PHYS_SCALER, 0, 0, NULL, 0, NULL,
	FRAME_bkflip11, PlayerMoveFunc, -380 * PHYS_SCALER, 0, 0, NULL, 0, NULL,
	FRAME_bkflip12, PlayerMoveFunc, -300 * PHYS_SCALER, 0, 0, NULL, 0, NULL,
	FRAME_bkflip13, PlayerMoveFunc, -280 * PHYS_SCALER, 0, 0, NULL, 0, NULL,
	FRAME_bkflip14, PlayerMoveFunc, -160 * PHYS_SCALER, 0, 0, SpawnDustPuff, 0, NULL,
};
panimmove_t player_move_jumpfliphandspring = { 9, player_frames_jumpfliphandspring, PlayerAnimLowerUpdate };

#pragma endregion

#pragma region ========================== PLAYER BACKFLIP ANIMATION ==========================

// Backwards flip.
static panimframe_t player_frames_flipback[] =
{
	FRAME_bkflip15, PlayerMoveForce, -125 * PHYS_SCALER, 0, 0, PlayerActionJumpBack, 250, NULL,
	FRAME_bkflip16, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_bkflip17, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_bkflip18, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_bkflip19, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_bkflip20, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_bkflip21, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_bkflip22, NULL, 0, 0, 0, PlayerActionCheckGrab, 0, NULL,
	FRAME_bkflip23, NULL, 0, 0, 0, PlayerActionCheckGrab, 0, NULL,
	FRAME_bkflip24, NULL, 0, 0, 0, PlayerActionCheckGrab, 0, NULL,
	FRAME_bkflip25, NULL, 0, 0, 0, SpawnDustPuff, 0, NULL,
};
panimmove_t player_move_jumpflipback = { 11, player_frames_flipback, PlayerAnimLowerUpdate };

#pragma endregion

#pragma region ========================== PLAYER PIVOT ANIMATIONS ==========================

// Start of pivot left.
static panimframe_t player_frames_pivotleftgo[] =
{
	FRAME_Lpivot1, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_Lpivot2, NULL, 0, 0, 0, PlayerActionFootstep, STEP_CREEP, NULL,
};
panimmove_t player_move_pivotleftgo = { 2, player_frames_pivotleftgo, PlayerAnimLowerUpdate };

// Cycle of pivot left.
static panimframe_t player_frames_pivotleft[] =
{
	FRAME_Lpivot3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_Lpivot4, NULL, 0, 0, 0, PlayerActionFootstep, STEP_CREEP2, NULL,
};
panimmove_t player_move_pivotleft = { 2, player_frames_pivotleft, PlayerAnimLowerUpdate };

// Transition from pivot to standing.
static panimframe_t player_frames_pivotleftend[] =
{
	FRAME_Lpivot4, NULL, 0, 0, 0, PlayerActionFootstep, STEP_CREEP, NULL,
};
panimmove_t player_move_pivotleftend = { 1, player_frames_pivotleftend, PlayerAnimLowerUpdate };

// Start of pivot right.
static panimframe_t player_frames_pivotrightgo[] =
{
	FRAME_Lpivot4, NULL, 0, 0, 0, PlayerActionFootstep, STEP_CREEP, NULL,
	FRAME_Lpivot3, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_pivotrightgo = { 2, player_frames_pivotrightgo, PlayerAnimLowerUpdate };

// Cycle of pivot right.
static panimframe_t player_frames_pivotright[] =
{
	FRAME_Lpivot2, NULL, 0, 0, 0, PlayerActionFootstep, STEP_CREEP2, NULL,
	FRAME_Lpivot1, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_pivotright = { 2, player_frames_pivotright, PlayerAnimLowerUpdate };

// Transition from pivot to standing.
static panimframe_t player_frames_pivotrightend[] =
{
	FRAME_Lpivot1, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_pivotrightend = { 1, player_frames_pivotrightend, PlayerAnimLowerUpdate };

#pragma endregion

#pragma region ========================== PLAYER NOTHING ANIMATION ==========================

//mxd. Fall-back animation used by ASEQ_NONE. Frame pick is strange...
static panimframe_t player_frames_nothing[] =
{
	FRAME_jog5, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
};
panimmove_t player_move_nothing = { 1, player_frames_nothing, PlayerAnimLowerUpdate };

#pragma endregion

#pragma region ========================== PLAYER PUZZLE PIECE / BUTTON / LEVER ANIMATIONS ==========================

// Taking a puzzle piece.
static panimframe_t player_frames_takepuzzlepiece[] =
{
	FRAME_phbuton1, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_phbuton2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_phbuton3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_phbuton4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_phbuton5, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_phbuton6, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_phbuton7, NULL, 0, 0, 0, PlayerActionTakePuzzle, 0, NULL,
	FRAME_phbuton8, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_phbuton9, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_takepuzzlepiece = { 9, player_frames_takepuzzlepiece, PlayerAnimLowerUpdate };

// Taking a puzzle piece underwater. //TODO: unused!
static panimframe_t player_frames_takepuzzleunderwater[] =
{
	FRAME_swmgrab1,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_swmgrab2,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_swmgrab3,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_swmgrab4,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_swmgrab5,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_swmgrab6,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_swmgrab7,  NULL, 0, 0, 0, PlayerActionTakePuzzle, 0, NULL,
	FRAME_swmgrab8,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_swmgrab9,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_swmgrab10, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_swmgrab11, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_takepuzzleunderwater = { 11, player_frames_takepuzzleunderwater, PlayerAnimLowerUpdate };

// Push button animation.
static panimframe_t player_frames_pushbuttongo[] =
{
	FRAME_phbuton1, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_phbuton2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_phbuton3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_phbuton4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_phbuton5, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_phbuton6, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_phbuton7, NULL, 0, 0, 0, PlayerActionPushButton, 0, NULL,
	FRAME_phbuton8, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_phbuton9, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_pushbuttongo = { 9, player_frames_pushbuttongo, PlayerAnimLowerUpdate };

// Push lever left animation.
static panimframe_t player_frames_pushleverleft[] =
{
	FRAME_LswichL2R1, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_LswichL2R2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_LswichL2R3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_LswichL2R4, NULL, 0, 0, 0, PlayerActionPushLever, 0, NULL,
	FRAME_LswichL2R5, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_LswichL2R6, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_LswichL2R7, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_pushleverleft = { 7, player_frames_pushleverleft, PlayerAnimLowerUpdate };

// Push lever right animation.
static panimframe_t player_frames_pushleverright[] =
{
	FRAME_RswichR2L1, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_RswichR2L2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_RswichR2L3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_RswichR2L4, NULL, 0, 0, 0, PlayerActionPushLever, 0, NULL,
	FRAME_RswichR2L5, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_RswichR2L6, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_RswichR2L7, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_pushleverright = { 7, player_frames_pushleverright, PlayerAnimLowerUpdate };

#pragma endregion

#pragma region ========================== PLAYER TUMBLE ANIMATIONS ==========================

//TODO: unused
static panimframe_t player_frames_tumbleon1[] =
{
	FRAME_rollA1, PlayerPullupHeight, -17,  0, 0, NULL, 0, NULL,
	FRAME_rollA2, PlayerPullupHeight, -27,  0, 0, NULL, 0, NULL,
	FRAME_rollA3, PlayerPullupHeight, -37, -1, 0, NULL, 0, NULL,
};
panimmove_t player_move_tumbleon1 = { 3, player_frames_tumbleon1, PlayerAnimLowerUpdate };

// Player roll animation.
static panimframe_t player_frames_tumbleon2[] =
{
	FRAME_rollA1,  PlayerPullupHeight, -10, 0, 0,  NULL, 0, PlayerActionSetCrouchHeight,
	FRAME_rollA2,  PlayerPullupHeight, -20, 0, 0,  NULL, 0, NULL,
	FRAME_rollA3,  PlayerPullupHeight, -44, 4, 0,  NULL, 0, NULL,
	FRAME_rollA5,  PlayerMoveALittle,   25, 0, 25, NULL, 0, NULL,
	FRAME_rollA7,  PlayerMoveALittle,   10, 0, 0,  NULL, 0, NULL,
	FRAME_rollA8,  PlayerMoveALittle,   0,  0, 0,  NULL, 0, NULL,
	FRAME_rollA10, PlayerMoveALittle,   0,  0, 0,  NULL, 0, NULL,
	FRAME_rollA11, PlayerMoveALittle,   0,  0, 0,  NULL, 0, NULL,
	FRAME_rollA13, PlayerMoveALittle,   0,  0, 0,  NULL, 0, NULL,
	FRAME_rollA14, PlayerMoveALittle,   0,  0, 0,  NULL, 0, NULL,
	FRAME_rollA15, PlayerMoveALittle,   0,  0, 0,  NULL, 0, NULL,
};
panimmove_t player_move_tumbleon2 = { 11, player_frames_tumbleon2, PlayerAnimLowerUpdate };

#pragma endregion

#pragma region ========================== PLAYER CROUCH PIVOT ANIMATIONS ==========================

// Pivot to left while crouched.
static panimframe_t player_frames_crouchpivotleft[] =
{
	FRAME_crhpvt1, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_crhpvt2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_crhpvt3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_crhpvt4, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_crouchpivotleft = { 4, player_frames_crouchpivotleft, PlayerAnimLowerUpdate };

// Pivot to right while crouched.
static panimframe_t player_frames_crouchpivotright[] =
{
	FRAME_crhpvt4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_crhpvt3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_crhpvt2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_crhpvt1, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_crouchpivotright = { 4, player_frames_crouchpivotright, PlayerAnimLowerUpdate };

#pragma endregion

#pragma region ========================== PLAYER FLIP ANIMATIONS ==========================

// Flip to left side.
static panimframe_t player_frames_jumpflipleft[] =
{
	FRAME_Lflip1,  PlayerMoveForce, 0, -250, 0, PlayerActionJump, 300, NULL,
	FRAME_Lflip2,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_Lflip3,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_Lflip4,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_Lflip5,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_Lflip6,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_Lflip7,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_Lflip8,  NULL, 0, 0, 0, PlayerActionCheckGrab, 0, NULL,
	FRAME_Lflip9,  NULL, 0, 0, 0, PlayerActionCheckGrab, 0, NULL,
	FRAME_Lflip10, NULL, 0, 0, 0, PlayerActionCheckGrab, 0, NULL,
	FRAME_Lflip11, NULL, 0, 0, 0, PlayerActionCheckGrab, 0, NULL,
	FRAME_Lflip12, NULL, 0, 0, 0, PlayerActionCheckGrab, 0, NULL,
	FRAME_Lflip13, NULL, 0, 0, 0, PlayerActionCheckGrab, 0, NULL,
	FRAME_Lflip14, NULL, 0, 0, 0, PlayerActionCheckGrab, 0, NULL,
};
panimmove_t player_move_jumpflipleft = { 14, player_frames_jumpflipleft, PlayerAnimLowerUpdate };

// Flip to right side.
static panimframe_t player_frames_jumpflipright[] =
{
	FRAME_Rflip1,  PlayerMoveForce, 0, 250, 0, PlayerActionJump, 300, NULL,
	FRAME_Rflip2,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_Rflip3,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_Rflip4,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_Rflip5,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_Rflip6,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_Rflip7,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_Rflip8,  NULL, 0, 0, 0, PlayerActionCheckGrab, 0, NULL,
	FRAME_Rflip9,  NULL, 0, 0, 0, PlayerActionCheckGrab, 0, NULL,
	FRAME_Rflip10, NULL, 0, 0, 0, PlayerActionCheckGrab, 0, NULL,
	FRAME_Rflip11, NULL, 0, 0, 0, PlayerActionCheckGrab, 0, NULL,
	FRAME_Rflip12, NULL, 0, 0, 0, PlayerActionCheckGrab, 0, NULL,
	FRAME_Rflip13, NULL, 0, 0, 0, PlayerActionCheckGrab, 0, NULL,
	FRAME_Rflip14, NULL, 0, 0, 0, PlayerActionCheckGrab, 0, NULL,
};
panimmove_t player_move_jumpflipright = { 14, player_frames_jumpflipright, PlayerAnimLowerUpdate };

// Roll from forward flip.
static panimframe_t player_frames_rollfromfflip[] =
{
	FRAME_rollA11, PlayerMoveFunc, 240 * PHYS_SCALER, 0, 0, NULL, 0, NULL,
	FRAME_rollA12, PlayerMoveFunc, 200 * PHYS_SCALER, 0, 0, NULL, 0, NULL,
	FRAME_rollA13, PlayerMoveFunc, 160 * PHYS_SCALER, 0, 0, NULL, 0, NULL,
	FRAME_rollA14, PlayerMoveFunc, 120 * PHYS_SCALER, 0, 0, NULL, 0, NULL,
	FRAME_rollA15, PlayerMoveFunc, 60 * PHYS_SCALER,  0, 0, NULL, 0, NULL,
};
panimmove_t player_move_rollfromfflip = { 5, player_frames_rollfromfflip, PlayerAnimLowerUpdate };

// Forward-left flip start.
static panimframe_t player_frames_forward_flip_l_go[] =
{
	FRAME_ritflip2, NULL, 0, 0, 0, PlayerActionFlip, 100, NULL, //TODO: should use FRAME_lftflip2?
};
panimmove_t	player_move_forward_flip_l_go = { 1, player_frames_forward_flip_l_go, PlayerAnimLowerUpdate };

// Forward-right flip start.
static panimframe_t player_frames_forward_flip_r_go[] =
{
	FRAME_ritflip2, NULL, 0, 0, 0, PlayerActionFlip, 100, NULL,
};
panimmove_t	player_move_forward_flip_r_go = { 1, player_frames_forward_flip_r_go, PlayerAnimLowerUpdate };

// Forward-left flip.
static panimframe_t player_frames_forward_flip_l[] =
{
	FRAME_lftflip4, PlayerJumpNudge, 64, 0, 0, NULL, 0, PlayerActionSetCrouchHeight,
	FRAME_lftflip5, PlayerJumpNudge, 64, 0, 0, NULL, 0, NULL,
	FRAME_lftflip6, PlayerJumpNudge, 64, 0, 0, NULL, 0, NULL,
	FRAME_lftflip7, PlayerJumpNudge, 64, 0, 0, NULL, 0, PlayerActionCheckUncrouchToFinishSeq,
	FRAME_lftflip8, PlayerJumpNudge, 64, 0, 0, PlayerActionCheckGrab, 0, NULL,
	FRAME_lftflip9, PlayerJumpNudge, 64, 0, 0, PlayerActionCheckGrab, 0, NULL,
};
panimmove_t	player_move_forward_flip_l = { 6, player_frames_forward_flip_l, PlayerAnimLowerUpdate };

// Forward-right flip.
static panimframe_t player_frames_forward_flip_r[] =
{
	FRAME_ritflip4, PlayerJumpNudge, 64, 0, 0, NULL, 0, PlayerActionSetCrouchHeight,
	FRAME_ritflip5, PlayerJumpNudge, 64, 0, 0, NULL, 0, NULL,
	FRAME_ritflip6, PlayerJumpNudge, 64, 0, 0, NULL, 0, NULL,
	FRAME_ritflip7, PlayerJumpNudge, 64, 0, 0, NULL, 0, PlayerActionCheckUncrouchToFinishSeq,
	FRAME_ritflip8, PlayerJumpNudge, 64, 0, 0, PlayerActionCheckGrab, 0, NULL,
	FRAME_ritflip9, PlayerJumpNudge, 64, 0, 0, PlayerActionCheckGrab, 0, NULL,
};
panimmove_t	player_move_forward_flip_r = { 6, player_frames_forward_flip_r, PlayerAnimLowerUpdate };

#pragma endregion

#pragma region ========================== PLAYER STAIR/SLOPE STANDING ANIMATIONS ==========================

// Left
static panimframe_t player_frames_lstair4[] = {	FRAME_Lstair1, NULL, 0, 0, 0, NULL, 0, NULL };
panimmove_t player_move_lstair4 = { 1, player_frames_lstair4, PlayerAnimLowerUpdate };

static panimframe_t player_frames_lstair8[] = {	FRAME_Lstair2, NULL, 0, 0, 0, NULL, 0, NULL };
panimmove_t player_move_lstair8 = { 1, player_frames_lstair8, PlayerAnimLowerUpdate };

static panimframe_t player_frames_lstair12[] = { FRAME_Lstair3, NULL, 0, 0, 0, NULL, 0, NULL };
panimmove_t player_move_lstair12 = { 1, player_frames_lstair12, PlayerAnimLowerUpdate };

static panimframe_t player_frames_lstair16[] = { FRAME_Lstair4, NULL, 0, 0, 0, NULL, 0, NULL };
panimmove_t player_move_lstair16 = { 1, player_frames_lstair16, PlayerAnimLowerUpdate };

// Right.
static panimframe_t player_frames_rstair4[] = { FRAME_Rstair1, NULL, 0, 0, 0, NULL, 0, NULL };
panimmove_t player_move_rstair4 = { 1, player_frames_rstair4, PlayerAnimLowerUpdate };

static panimframe_t player_frames_rstair8[] = { FRAME_Rstair2, NULL, 0, 0, 0, NULL, 0, NULL };
panimmove_t player_move_rstair8 = { 1, player_frames_rstair8, PlayerAnimLowerUpdate };

static panimframe_t player_frames_rstair12[] = { FRAME_Rstair3, NULL, 0, 0, 0, NULL, 0, NULL };
panimmove_t player_move_rstair12 = { 1, player_frames_rstair12, PlayerAnimLowerUpdate };

static panimframe_t player_frames_rstair16[] = { FRAME_Rstair4, NULL, 0, 0, 0, NULL, 0, NULL };
panimmove_t player_move_rstair16 = { 1, player_frames_rstair16, PlayerAnimLowerUpdate };

#pragma endregion

#pragma region ========================== PLAYER IDLE ANIMATIONS ==========================

// Standing and breathing.
static panimframe_t player_frames_stand[] =
{
	FRAME_breath1,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_breath2,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_breath3,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_breath4,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_breath5,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_breath6,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_breath7,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_breath8,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_breath9,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_breath10, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_breath11, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_breath12, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_breath13, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_breath14, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_breath15, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_breath16, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_breath17, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_breath18, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_breath19, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_breath20, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_breath21, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_breath22, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_breath23, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
};
panimmove_t player_move_stand = { 23, player_frames_stand, PlayerAnimLowerUpdate };

// Battle-stance idle start.
static panimframe_t player_frames_standreadystart[] =
{
	FRAME_ready1, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_ready2, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_ready3, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
};
panimmove_t player_move_standreadystart = { 3, player_frames_standreadystart, PlayerAnimLowerUpdate };

// Battle-stance idle loop.
static panimframe_t player_frames_standready[] =
{
	FRAME_ready4,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_ready5,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_ready6,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_ready7,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_ready8,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_ready9,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_ready10, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_ready11, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_ready12, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_ready13, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_ready14, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_ready15, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_ready16, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_ready17, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_ready18, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_ready19, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_ready20, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_ready21, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_ready22, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_ready23, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_ready24, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_ready25, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_ready26, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
};
panimmove_t player_move_standready = { 23, player_frames_standready, PlayerAnimLowerUpdate };

// Idle end.
static panimframe_t player_frames_standreadyend[] =
{
	FRAME_ready3, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_ready2, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_ready1, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
};
panimmove_t player_move_standreadyend = { 3, player_frames_standreadyend, PlayerAnimLowerUpdate };

// Look left.
static panimframe_t player_frames_standlookleft[] =
{
	FRAME_lklft1,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lklft2,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lklft3,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lklft4,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lklft5,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lklft6,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lklft7,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lklft8,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lklft9,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lklft10, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lklft11, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lklft12, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lklft13, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lklft14, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lklft15, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lklft16, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lklft17, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lklft18, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lklft19, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lklft20, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lklft21, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lklft22, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
};
panimmove_t player_move_standlookleft = { 22, player_frames_standlookleft, PlayerAnimLowerUpdate };

// Look right.
static panimframe_t player_frames_standlookright[] =
{
	FRAME_lkrt1,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lkrt2,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lkrt3,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lkrt4,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lkrt5,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lkrt6,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lkrt7,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lkrt8,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lkrt9,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lkrt10, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lkrt11, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lkrt12, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lkrt13, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lkrt14, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lkrt15, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lkrt16, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lkrt17, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lkrt18, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lkrt19, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lkrt20, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lkrt21, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lkrt22, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
};
panimmove_t player_move_standlookright = { 22, player_frames_standlookright, PlayerAnimLowerUpdate };

// Bothered by a fly.
static panimframe_t player_frames_pest1[] =
{
	FRAME_breath1, NULL, 0, 0, 0, PlayFly, 0, PlayerAnimLowerIdle,
	FRAME_breath2, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_breath3, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_breath4, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_breath5, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_breath6, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_breath7, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_breath8, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_pestA1,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_pestA2,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_pestA3,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_pestA4,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_pestA5,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_pestA6,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_pestA7,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_pestA8,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_pestA9,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_pestA10, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_pestA11, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_pestA12, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_pestA13, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_pestA14, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle
};
panimmove_t player_move_pest1 = { 22, player_frames_pest1, PlayerAnimLowerUpdate };

// Slap a fly.
static panimframe_t player_frames_pest2[] =
{
	FRAME_breath1, NULL, 0, 0, 0, PlayFly, 0, PlayerAnimLowerIdle,
	FRAME_breath2, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_breath3, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_breath4, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_breath5, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_breath6, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_breath7, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_breath8, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_pestB1,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_pestB2,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_pestB3,  NULL, 0, 0, 0, PlaySlap, 0, PlayerAnimLowerIdle,
	FRAME_pestB4,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_pestB5,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_pestB6,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_pestB7,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_pestB8,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_pestB9,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_pestB10, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_pestB11, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_pestB12, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_pestB13, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_pestB14, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_pestB15, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_pestB16, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_pestB17, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_pestB18, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_pestB19, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_pestB20, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_pestB21, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_pestB22, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle
};
panimmove_t player_move_pest2 = { 30, player_frames_pest2, PlayerAnimLowerUpdate };

// Look back.
static panimframe_t player_frames_idle_lookback[] =
{
	FRAME_lookbck1,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lookbck2,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lookbck3,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lookbck4,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lookbck5,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lookbck6,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lookbck7,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lookbck8,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lookbck9,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lookbck10, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lookbck11, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lookbck12, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lookbck13, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lookbck14, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lookbck15, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lookbck16, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lookbck17, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lookbck18, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lookbck19, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lookbck20, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lookbck21, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lookbck22, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lookbck23, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lookbck24, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lookbck25, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lookbck26, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lookbck27, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lookbck28, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_lookbck29, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
};
panimmove_t player_move_idle_lookback = { 29, player_frames_idle_lookback, PlayerAnimLowerUpdate };

// Scratch bottocks.
static panimframe_t player_frames_idle_scratch_ass[] =
{
	FRAME_asscrtch1,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_asscrtch2,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_asscrtch3,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_asscrtch4,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_asscrtch5,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_asscrtch6,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_asscrtch7,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_asscrtch8,  NULL, 0, 0, 0, PlayScratch, 0, PlayerAnimLowerIdle,
	FRAME_asscrtch9,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_asscrtch10, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_asscrtch11, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_asscrtch12, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_asscrtch13, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_asscrtch14, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_asscrtch15, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_asscrtch16, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_asscrtch17, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_asscrtch18, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_asscrtch19, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_asscrtch20, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_asscrtch21, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_asscrtch22, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_asscrtch23, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_asscrtch24, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_asscrtch25, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
};
panimmove_t player_move_idle_scratch_ass = { 25, player_frames_idle_scratch_ass, PlayerAnimLowerUpdate };

// Phew!
static panimframe_t player_frames_idle_wipe_brow[] =
{
	FRAME_wipebrow1,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_wipebrow2,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_wipebrow3,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_wipebrow4,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_wipebrow5,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_wipebrow6,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_wipebrow7,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_wipebrow8,  NULL, 0, 0, 0, PlaySigh, 0, PlayerAnimLowerIdle,
	FRAME_wipebrow9,  NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_wipebrow10, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_wipebrow11, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_wipebrow12, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_wipebrow13, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_wipebrow14, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_wipebrow15, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_wipebrow16, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_wipebrow17, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_wipebrow18, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_wipebrow19, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
};
panimmove_t player_move_idle_wipe_brow = { 19, player_frames_idle_wipe_brow, PlayerAnimLowerUpdate };

#pragma endregion

#pragma region ========================== PLAYER PAIN ANIMATIONS ==========================

// Pain A.
static panimframe_t player_frames_paina[] =
{
	FRAME_smlpna1, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_smlpna2, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_smlpna3, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
	FRAME_smlpna4, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerIdle,
};
panimmove_t player_move_paina = { 4, player_frames_paina, PlayerAnimLowerUpdate };

// Pain B.
static panimframe_t player_frames_painb[] =
{
	FRAME_smlpnb2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_smlpnb4, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_painb = { 2, player_frames_painb, PlayerAnimLowerUpdate };

#pragma endregion

#pragma region ========================== PLAYER FALLING OFF LEDGES ANIMATIONS ==========================

// Fall left.
static panimframe_t player_frames_fallleft[] =
{
	FRAME_Rstair1, NULL, 0, 0, 0, NULL, 0, BranchLwrStanding,
	FRAME_Rstair2, NULL, 0, 0, 0, NULL, 0, BranchLwrStanding,
	FRAME_Rstair3, NULL, 0, 0, 0, NULL, 0, BranchLwrStanding,
	FRAME_Rstair4, NULL, 0, 0, 0, NULL, 0, BranchLwrStanding,
};
panimmove_t player_move_fallleft = { 4, player_frames_fallleft, PlayerAnimLowerUpdate };

// Fall right.
static panimframe_t player_frames_fallright[] =
{
	FRAME_Lstair1, NULL, 0, 0, 0, NULL, 0, BranchLwrStanding,
	FRAME_Lstair2, NULL, 0, 0, 0, NULL, 0, BranchLwrStanding,
	FRAME_Lstair3, NULL, 0, 0, 0, NULL, 0, BranchLwrStanding,
	FRAME_Lstair4, NULL, 0, 0, 0, NULL, 0, BranchLwrStanding,
};
panimmove_t player_move_fallright = { 4, player_frames_fallright, PlayerAnimLowerUpdate };

// Fall left end.
static panimframe_t player_frames_fallleftend[] =
{
	FRAME_Rstair4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_Rstair4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_Rstair4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_Rstair4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_Rstair4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_Rstair4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_Rstair4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_Rstair4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_Rstair4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_Rstair4, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_fallleftend = { 10, player_frames_fallleftend, PlayerAnimLowerUpdate };

// Fall right end.
static panimframe_t player_frames_fallrightend[] =
{
	FRAME_Lstair4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_Lstair4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_Lstair4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_Lstair4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_Lstair4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_Lstair4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_Lstair4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_Lstair4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_Lstair4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_Lstair4, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t player_move_fallrightend = { 10, player_frames_fallrightend, PlayerAnimLowerUpdate };

#pragma endregion

#pragma region ========================== PLAYER DEATH ANIMATIONS ==========================

static panimframe_t player_frames_death1[] =
{
	FRAME_deathA1,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deathA2,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deathA3,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deathA4,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deathA5,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deathA6,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deathA7,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deathA8,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deathA9,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deathA10, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deathA11, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deathA12, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deathA13, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deathA14, NULL, 0, 0, 0, NULL, 0, PlayerActionSetDead,
};
panimmove_t player_move_death1 = { 14, player_frames_death1, PlayerAnimLowerUpdate };

// Burning death.
static panimframe_t player_frames_death_b[] =
{
	FRAME_burning1,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_burning2,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_burning3,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_burning4,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_burning5,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_burning6,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_burning7,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_burning8,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_burning9,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_burning10, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_burning11, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_burning12, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_burning13, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_burning14, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_burning15, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_burning16, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_burning17, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_burning18, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_burning19, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_burning20, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_burning21, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_burning22, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_burning23, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_burning24, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_burning25, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_burning26, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_burning27, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_burning28, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_burning29, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_burning30, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_burning31, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_burning32, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_burning33, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_burning34, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_burning35, NULL, 0, 0, 0, NULL, 0, PlayerActionSetDead,
};
panimmove_t player_move_death_b = { 35, player_frames_death_b, PlayerAnimLowerUpdate };

// Fall forwards, landing on stomach (mxd. Every 2-nd frame is skipped).
static panimframe_t player_frames_death_fly_forward[] =
{
	FRAME_faceplant1,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_faceplant3,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_faceplant5,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_faceplant7,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_faceplant9,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_faceplant11, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_faceplant13, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_faceplant15, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_faceplant17, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_faceplant19, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_faceplant21, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_faceplant23, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_faceplant24, NULL, 0, 0, 0, NULL, 0, PlayerActionSetDead,
};
panimmove_t player_move_death_fly_forward = { 13, player_frames_death_fly_forward, PlayerAnimLowerUpdate };

// Flip backwards, landing on stomach (mxd. Every 2-nd frame is skipped).
static panimframe_t player_frames_death_fly_back[] =
{
	FRAME_blowback1,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_blowback3,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_blowback5,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_blowback7,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_blowback9,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_blowback11, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_blowback13, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_blowback15, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_blowback17, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_blowback19, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_blowback21, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_blowback23, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_blowback24, NULL, 0, 0, 0, NULL, 0, PlayerActionSetDead,
};
panimmove_t player_move_death_fly_back = { 13, player_frames_death_fly_back, PlayerAnimLowerUpdate };

// Choking death animation.
static panimframe_t player_frames_death_choke[] =
{
	FRAME_choking1,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_choking2,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_choking3,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_choking4,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_choking5,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_choking6,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_choking7,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_choking8,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_choking9,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_choking10, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_choking11, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_choking12, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_choking13, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_choking14, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_choking15, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_choking16, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_choking17, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_choking18, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_choking19, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_choking20, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_choking21, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_choking22, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_choking23, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_choking24, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_choking25, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_choking26, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_choking27, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_choking28, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_choking29, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_choking30, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_choking31, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_choking32, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_choking33, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_choking34, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_choking35, NULL, 0, 0, 0, NULL, 0, PlayerActionSetDead,
};
panimmove_t player_move_death_choke = { 35, player_frames_death_choke, PlayerAnimLowerUpdate };

#pragma endregion

#pragma region ========================== PLAYER DROWN ANIMATIONS ==========================

// Player drown animation.
static panimframe_t player_frames_drown[] =
{
	FRAME_drown1,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_drown2,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_drown3,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_drown4,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_drown5,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_drown6,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_drown7,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_drown8,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_drown9,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_drown10, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_drown11, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_drown12, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_drown13, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_drown14, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_drown15, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_drown16, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_drown17, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_drown18, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_drown19, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_drown20, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_drown21, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_drown22, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_drown23, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_drown24, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_drown25, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_drown26, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_drown27, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_drown28, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_drown29, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_drown30, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_drown31, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_drown32, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_drown33, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_drown34, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_drown35, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_drown36, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_drown37, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_drown38, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_drown39, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_drown40, NULL, 0, 0, 0, NULL, 0, PlayerActionSetDead,
};
panimmove_t	player_move_drown = { 40, player_frames_drown, PlayerAnimLowerUpdate };

// Drowning idle.
static panimframe_t player_frames_drown_idle[] =
{
	FRAME_drownidle1,  NULL, 0, 0, 0, NULL, 0, PlayerActionDrownFloatUp,
	FRAME_drownidle2,  NULL, 0, 0, 0, NULL, 0, PlayerActionDrownFloatUp,
	FRAME_drownidle3,  NULL, 0, 0, 0, NULL, 0, PlayerActionDrownFloatUp,
	FRAME_drownidle4,  NULL, 0, 0, 0, NULL, 0, PlayerActionDrownFloatUp,
	FRAME_drownidle5,  NULL, 0, 0, 0, NULL, 0, PlayerActionDrownFloatUp,
	FRAME_drownidle6,  NULL, 0, 0, 0, NULL, 0, PlayerActionDrownFloatUp,
	FRAME_drownidle7,  NULL, 0, 0, 0, NULL, 0, PlayerActionDrownFloatUp,
	FRAME_drownidle8,  NULL, 0, 0, 0, NULL, 0, PlayerActionDrownFloatUp,
	FRAME_drownidle9,  NULL, 0, 0, 0, NULL, 0, PlayerActionDrownFloatUp,
	FRAME_drownidle10, NULL, 0, 0, 0, NULL, 0, PlayerActionDrownFloatUp,
	FRAME_drownidle11, NULL, 0, 0, 0, NULL, 0, PlayerActionDrownFloatUp,
	FRAME_drownidle12, NULL, 0, 0, 0, NULL, 0, PlayerActionDrownFloatUp,
	FRAME_drownidle13, NULL, 0, 0, 0, NULL, 0, PlayerActionDrownFloatUp,
	FRAME_drownidle14, NULL, 0, 0, 0, NULL, 0, PlayerActionDrownFloatUp,
	FRAME_drownidle15, NULL, 0, 0, 0, NULL, 0, PlayerActionDrownFloatUp,
	FRAME_drownidle16, NULL, 0, 0, 0, NULL, 0, PlayerActionDrownFloatUp,
	FRAME_drownidle17, NULL, 0, 0, 0, NULL, 0, PlayerActionDrownFloatUp,
	FRAME_drownidle18, NULL, 0, 0, 0, NULL, 0, PlayerActionDrownFloatUp,
	FRAME_drownidle19, NULL, 0, 0, 0, NULL, 0, PlayerActionDrownFloatUp,
	FRAME_drownidle20, NULL, 0, 0, 0, NULL, 0, PlayerActionDrownFloatUp,
	FRAME_drownidle21, NULL, 0, 0, 0, NULL, 0, PlayerActionDrownFloatUp,
	FRAME_drownidle22, NULL, 0, 0, 0, NULL, 0, PlayerActionDrownFloatUp,
	FRAME_drownidle23, NULL, 0, 0, 0, NULL, 0, PlayerActionDrownFloatUp,
	FRAME_drownidle24, NULL, 0, 0, 0, NULL, 0, PlayerActionDrownFloatUp,
	FRAME_drownidle25, NULL, 0, 0, 0, NULL, 0, PlayerActionDrownFloatUp,
	FRAME_drownidle26, NULL, 0, 0, 0, NULL, 0, PlayerActionDrownFloatUp,
	FRAME_drownidle27, NULL, 0, 0, 0, NULL, 0, PlayerActionDrownFloatUp,
	FRAME_drownidle28, NULL, 0, 0, 0, NULL, 0, PlayerActionDrownFloatUp,
	FRAME_drownidle29, NULL, 0, 0, 0, NULL, 0, PlayerActionDrownFloatUp,
	FRAME_drownidle30, NULL, 0, 0, 0, NULL, 0, PlayerActionDrownFloatUp,
	FRAME_drownidle31, NULL, 0, 0, 0, NULL, 0, PlayerActionDrownFloatUp,
	FRAME_drownidle32, NULL, 0, 0, 0, NULL, 0, PlayerActionDrownFloatUp,
};
panimmove_t player_move_drown_idle = { 32, player_frames_drown_idle, PlayerAnimUpperUpdate };

#pragma endregion

#pragma region ========================== PLAYER SLIDE ANIMATIONS ==========================

// Slide forward.
static panimframe_t player_frames_slide_forward[] =
{
	FRAME_slid4d1,  NULL, 0, 0, 0, SpawnDustPuff, 0, PlayerPlaySlide,
	FRAME_slid4d2,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_slid4d3,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_slid4d4,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_slid4d5,  NULL, 0, 0, 0, SpawnDustPuff, 0, NULL,
	FRAME_slid4d6,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_slid4d7,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_slid4d8,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_slid4d9,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_slid4d10, NULL, 0, 0, 0, SpawnDustPuff, 0, NULL,
	FRAME_slid4d11, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_slid4d12, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t	player_move_slide_forward = { 12, player_frames_slide_forward, PlayerAnimLowerUpdate };

// Slide backward.
static panimframe_t player_frames_slide_backward[] =
{
	FRAME_slidbak1,  NULL, 0, 0, 0, SpawnDustPuff, 0, PlayerPlaySlide,
	FRAME_slidbak2,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_slidbak3,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_slidbak4,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_slidbak5,  NULL, 0, 0, 0, SpawnDustPuff, 0, NULL,
	FRAME_slidbak6,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_slidbak7,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_slidbak8,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_slidbak9,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_slidbak10, NULL, 0, 0, 0, SpawnDustPuff, 0, NULL,
	FRAME_slidbak11, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_slidbak12, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t	player_move_slide_backward = { 12, player_frames_slide_backward, PlayerAnimLowerUpdate };

#pragma endregion

#pragma region ========================== PLAYER ROLL ANIMATIONS ==========================

// Roll left.
static panimframe_t player_frames_roll_l[] =
{
	FRAME_roll1, PlayerMoveFunc, 0, -300 * PHYS_SCALER, 0, NULL, 0, PlayerActionSetCrouchHeight,
	FRAME_roll2, PlayerMoveFunc, 0, -200 * PHYS_SCALER, 0, PlayerActionFootstep, STEP_ROLL, NULL,
	FRAME_roll3, PlayerMoveFunc, 0, -150 * PHYS_SCALER, 0, NULL, 0, NULL,
	FRAME_roll4, PlayerMoveFunc, 0, -150 * PHYS_SCALER, 0, SpawnDustPuff, 0, NULL,
	FRAME_roll5, PlayerMoveFunc, 0, -100 * PHYS_SCALER, 0, NULL, 0, NULL,
	FRAME_roll6, PlayerMoveFunc, 0, -100 * PHYS_SCALER, 0, NULL, 0, NULL,
	FRAME_roll7, PlayerMoveFunc, 0, -100 * PHYS_SCALER, 0, NULL, 0, NULL,
	FRAME_roll8, PlayerMoveFunc, 0, -50 * PHYS_SCALER,  0, NULL, 0, NULL,
	FRAME_roll9, PlayerMoveFunc, 0, -25 * PHYS_SCALER,  0, NULL, 0, NULL,
};
panimmove_t	player_move_roll_l = { 9, player_frames_roll_l, PlayerAnimLowerUpdate };

// Roll right.
static panimframe_t player_frames_roll_r[] =
{
	FRAME_roll9, PlayerMoveFunc, 0, 300 * PHYS_SCALER, 0, NULL, 0, PlayerActionSetCrouchHeight,
	FRAME_roll8, PlayerMoveFunc, 0, 200 * PHYS_SCALER, 0, PlayerActionFootstep, STEP_ROLL, NULL,
	FRAME_roll7, PlayerMoveFunc, 0, 150 * PHYS_SCALER, 0, NULL, 0, NULL,
	FRAME_roll6, PlayerMoveFunc, 0, 150 * PHYS_SCALER, 0, SpawnDustPuff, 0, NULL,
	FRAME_roll5, PlayerMoveFunc, 0, 100 * PHYS_SCALER, 0, NULL, 0, NULL,
	FRAME_roll4, PlayerMoveFunc, 0, 100 * PHYS_SCALER, 0, NULL, 0, NULL,
	FRAME_roll3, PlayerMoveFunc, 0, 100 * PHYS_SCALER, 0, NULL, 0, NULL,
	FRAME_roll2, PlayerMoveFunc, 0, 50 * PHYS_SCALER,  0, NULL, 0, NULL,
	FRAME_roll1, PlayerMoveFunc, 0, 25 * PHYS_SCALER,  0, NULL, 0, NULL,
};
panimmove_t	player_move_roll_r = { 9, player_frames_roll_r, PlayerAnimLowerUpdate };

// Roll backwards.
static panimframe_t player_frames_roll_b[] =
{
	FRAME_backroll1, PlayerMoveFunc, -300 * PHYS_SCALER, 0, 0, NULL, 0, PlayerActionSetCrouchHeight,
	FRAME_backroll2, PlayerMoveFunc, -200 * PHYS_SCALER, 0, 0, PlayerActionFootstep, STEP_ROLL, NULL,
	FRAME_backroll3, PlayerMoveFunc, -150 * PHYS_SCALER, 0, 0, NULL, 0, NULL,
	FRAME_backroll4, PlayerMoveFunc, -150 * PHYS_SCALER, 0, 0, SpawnDustPuff, 0, NULL,
	FRAME_backroll5, PlayerMoveFunc, -100 * PHYS_SCALER, 0, 0, NULL, 0, NULL,
	FRAME_backroll6, PlayerMoveFunc, -100 * PHYS_SCALER, 0, 0, NULL, 0, NULL,
	FRAME_backroll7, PlayerMoveFunc, -100 * PHYS_SCALER, 0, 0, NULL, 0, NULL,
	FRAME_backroll8, PlayerMoveFunc, -50 * PHYS_SCALER,  0, 0, NULL, 0, NULL,
	FRAME_backroll9, PlayerMoveFunc, -25 * PHYS_SCALER,  0, 0, NULL, 0, NULL,
};
panimmove_t	player_move_roll_b = { 9, player_frames_roll_b, PlayerAnimLowerUpdate };

#pragma endregion

#pragma region ========================== PLAYER ROPE CLIMB ANIMATIONS ==========================

// Transfer onto rope.
static panimframe_t player_frames_climb_on[] =
{
	FRAME_grabrope1, PlayerClimbingMoveFunc, 0,  0, 0, NULL, 0, NULL,
	FRAME_grabrope2, PlayerClimbingMoveFunc, 2,  0, 0, NULL, 0, NULL,
	FRAME_grabrope3, PlayerClimbingMoveFunc, 8,  0, 0, NULL, 0, NULL,
	FRAME_grabrope4, PlayerClimbingMoveFunc, 16, 0, 0, NULL, 0, NULL,
	FRAME_grabrope5, PlayerClimbingMoveFunc, 8,  0, 0, NULL, 0, NULL,
	FRAME_grabrope6, PlayerClimbingMoveFunc, 4,  0, 0, NULL, 0, NULL,
	FRAME_grabrope7, PlayerClimbingMoveFunc, 2,  0, 0, NULL, 0, NULL,
};
panimmove_t	player_move_climb_on = { 7, player_frames_climb_on, PlayerAnimLowerUpdate };

// Hold rope (right hand up).
static panimframe_t player_frames_climb_hold_r[] =
{
	FRAME_Ridle1,  PlayerClimbingMoveFunc, 0, 0, 0, PlayerActionCheckRopeMove, 0, PlayerAnimLowerIdle,
	FRAME_Ridle2,  PlayerClimbingMoveFunc, 0, 0, 0, PlayerActionCheckRopeMove, 0, PlayerAnimLowerIdle,
	FRAME_Ridle3,  PlayerClimbingMoveFunc, 0, 0, 0, PlayerActionCheckRopeMove, 0, PlayerAnimLowerIdle,
	FRAME_Ridle4,  PlayerClimbingMoveFunc, 0, 0, 0, PlayerActionCheckRopeMove, 0, PlayerAnimLowerIdle,
	FRAME_Ridle5,  PlayerClimbingMoveFunc, 0, 0, 0, PlayerActionCheckRopeMove, 0, PlayerAnimLowerIdle,
	FRAME_Ridle6,  PlayerClimbingMoveFunc, 0, 0, 0, PlayerActionCheckRopeMove, 0, PlayerAnimLowerIdle,
	FRAME_Ridle7,  PlayerClimbingMoveFunc, 0, 0, 0, PlayerActionCheckRopeMove, 0, PlayerAnimLowerIdle,
	FRAME_Ridle8,  PlayerClimbingMoveFunc, 0, 0, 0, PlayerActionCheckRopeMove, 0, PlayerAnimLowerIdle,
	FRAME_Ridle9,  PlayerClimbingMoveFunc, 0, 0, 0, PlayerActionCheckRopeMove, 0, PlayerAnimLowerIdle,
	FRAME_Ridle10, PlayerClimbingMoveFunc, 0, 0, 0, PlayerActionCheckRopeMove, 0, PlayerAnimLowerIdle,
	FRAME_Ridle11, PlayerClimbingMoveFunc, 0, 0, 0, PlayerActionCheckRopeMove, 0, PlayerAnimLowerIdle,
	FRAME_Ridle12, PlayerClimbingMoveFunc, 0, 0, 0, PlayerActionCheckRopeMove, 0, PlayerAnimLowerIdle,
	FRAME_Ridle13, PlayerClimbingMoveFunc, 0, 0, 0, PlayerActionCheckRopeMove, 0, PlayerAnimLowerIdle,
	FRAME_Ridle14, PlayerClimbingMoveFunc, 0, 0, 0, PlayerActionCheckRopeMove, 0, PlayerAnimLowerIdle,
	FRAME_Ridle15, PlayerClimbingMoveFunc, 0, 0, 0, PlayerActionCheckRopeMove, 0, PlayerAnimLowerIdle,
	FRAME_Ridle16, PlayerClimbingMoveFunc, 0, 0, 0, PlayerActionCheckRopeMove, 0, PlayerAnimLowerIdle,
	FRAME_Ridle17, PlayerClimbingMoveFunc, 0, 0, 0, PlayerActionCheckRopeMove, 0, PlayerAnimLowerIdle,
	FRAME_Ridle18, PlayerClimbingMoveFunc, 0, 0, 0, PlayerActionCheckRopeMove, 0, PlayerAnimLowerIdle,
	FRAME_Ridle19, PlayerClimbingMoveFunc, 0, 0, 0, PlayerActionCheckRopeMove, 0, PlayerAnimLowerIdle,
	FRAME_Ridle20, PlayerClimbingMoveFunc, 0, 0, 0, PlayerActionCheckRopeMove, 0, PlayerAnimLowerIdle,
	FRAME_Ridle21, PlayerClimbingMoveFunc, 0, 0, 0, PlayerActionCheckRopeMove, 0, PlayerAnimLowerIdle,
};
panimmove_t	player_move_climb_hold_r = { 21, player_frames_climb_hold_r, PlayerAnimLowerUpdate };

// Hold rope (left hand up).
static panimframe_t player_frames_climb_hold_l[] =
{
	FRAME_Lidle1,  PlayerClimbingMoveFunc, 0, 0, 0, PlayerActionCheckRopeMove, 0, PlayerAnimLowerIdle,
	FRAME_Lidle2,  PlayerClimbingMoveFunc, 0, 0, 0, PlayerActionCheckRopeMove, 0, PlayerAnimLowerIdle,
	FRAME_Lidle3,  PlayerClimbingMoveFunc, 0, 0, 0, PlayerActionCheckRopeMove, 0, PlayerAnimLowerIdle,
	FRAME_Lidle4,  PlayerClimbingMoveFunc, 0, 0, 0, PlayerActionCheckRopeMove, 0, PlayerAnimLowerIdle,
	FRAME_Lidle5,  PlayerClimbingMoveFunc, 0, 0, 0, PlayerActionCheckRopeMove, 0, PlayerAnimLowerIdle,
	FRAME_Lidle6,  PlayerClimbingMoveFunc, 0, 0, 0, PlayerActionCheckRopeMove, 0, PlayerAnimLowerIdle,
	FRAME_Lidle7,  PlayerClimbingMoveFunc, 0, 0, 0, PlayerActionCheckRopeMove, 0, PlayerAnimLowerIdle,
	FRAME_Lidle8,  PlayerClimbingMoveFunc, 0, 0, 0, PlayerActionCheckRopeMove, 0, PlayerAnimLowerIdle,
	FRAME_Lidle9,  PlayerClimbingMoveFunc, 0, 0, 0, PlayerActionCheckRopeMove, 0, PlayerAnimLowerIdle,
	FRAME_Lidle10, PlayerClimbingMoveFunc, 0, 0, 0, PlayerActionCheckRopeMove, 0, PlayerAnimLowerIdle,
	FRAME_Lidle11, PlayerClimbingMoveFunc, 0, 0, 0, PlayerActionCheckRopeMove, 0, PlayerAnimLowerIdle,
	FRAME_Lidle12, PlayerClimbingMoveFunc, 0, 0, 0, PlayerActionCheckRopeMove, 0, PlayerAnimLowerIdle,
	FRAME_Lidle13, PlayerClimbingMoveFunc, 0, 0, 0, PlayerActionCheckRopeMove, 0, PlayerAnimLowerIdle,
	FRAME_Lidle14, PlayerClimbingMoveFunc, 0, 0, 0, PlayerActionCheckRopeMove, 0, PlayerAnimLowerIdle,
	FRAME_Lidle15, PlayerClimbingMoveFunc, 0, 0, 0, PlayerActionCheckRopeMove, 0, PlayerAnimLowerIdle,
	FRAME_Lidle16, PlayerClimbingMoveFunc, 0, 0, 0, PlayerActionCheckRopeMove, 0, PlayerAnimLowerIdle,
	FRAME_Lidle17, PlayerClimbingMoveFunc, 0, 0, 0, PlayerActionCheckRopeMove, 0, PlayerAnimLowerIdle,
	FRAME_Lidle18, PlayerClimbingMoveFunc, 0, 0, 0, PlayerActionCheckRopeMove, 0, PlayerAnimLowerIdle,
	FRAME_Lidle19, PlayerClimbingMoveFunc, 0, 0, 0, PlayerActionCheckRopeMove, 0, PlayerAnimLowerIdle,
	FRAME_Lidle20, PlayerClimbingMoveFunc, 0, 0, 0, PlayerActionCheckRopeMove, 0, PlayerAnimLowerIdle,
	FRAME_Lidle21, PlayerClimbingMoveFunc, 0, 0, 0, PlayerActionCheckRopeMove, 0, PlayerAnimLowerIdle,
};
panimmove_t	player_move_climb_hold_l = { 21, player_frames_climb_hold_l, PlayerAnimLowerUpdate };

// Staring out from the idle position into climb up (right hand up).
static panimframe_t player_frames_climb_up_start_r[] =
{
	FRAME_Rclmbstrt1, PlayerClimbingMoveFunc, 2, 0, 0, PlayerActionCheckRopeMove, 0, NULL,
	FRAME_Rclmbstrt2, PlayerClimbingMoveFunc, 4, 0, 0, PlayerActionCheckRopeMove, 0, NULL,
	FRAME_Rclmbstrt3, PlayerClimbingMoveFunc, 8, 0, 0, PlayerActionCheckRopeMove, 0, NULL,
};
panimmove_t	player_move_climb_up_start_r = { 3, player_frames_climb_up_start_r, PlayerAnimLowerUpdate };

// Staring out from the idle position into climb up (left hand up).
static panimframe_t player_frames_climb_up_start_l[] =
{
	FRAME_Lclmbstrt1, PlayerClimbingMoveFunc, 2, 0, 0, PlayerActionCheckRopeMove, 0, NULL,
	FRAME_Lclmbstrt2, PlayerClimbingMoveFunc, 4, 0, 0, PlayerActionCheckRopeMove, 0, NULL,
	FRAME_Lclmbstrt3, PlayerClimbingMoveFunc, 8, 0, 0, PlayerActionCheckRopeMove, 0, NULL,
};
panimmove_t	player_move_climb_up_start_l = { 3, player_frames_climb_up_start_l, PlayerAnimLowerUpdate };

// Staring out from the idle position into climb down (right hand up).
static panimframe_t player_frames_climb_down_start_r[] =
{
	FRAME_Rclmbstrt3, PlayerClimbingMoveFunc, -2, 0, 0, PlayerActionCheckRopeMove, 0, NULL,
	FRAME_Rclmbstrt2, PlayerClimbingMoveFunc, -4, 0, 0, PlayerActionCheckRopeMove, 0, NULL,
	FRAME_Rclmbstrt1, PlayerClimbingMoveFunc, -8, 0, 0, PlayerActionCheckRopeMove, 0, NULL,
};
panimmove_t	player_move_climb_down_start_r = { 3, player_frames_climb_down_start_r, PlayerAnimLowerUpdate };

// Staring out from the idle position into climb down (left hand up).
static panimframe_t player_frames_climb_down_start_l[] =
{
	FRAME_Lclmbstrt3, PlayerClimbingMoveFunc, -2, 0, 0, PlayerActionCheckRopeMove, 0, NULL,
	FRAME_Lclmbstrt2, PlayerClimbingMoveFunc, -4, 0, 0, PlayerActionCheckRopeMove, 0, NULL,
	FRAME_Lclmbstrt1, PlayerClimbingMoveFunc, -8, 0, 0, PlayerActionCheckRopeMove, 0, NULL,
};
panimmove_t	player_move_climb_down_start_l = { 3, player_frames_climb_down_start_l, PlayerAnimLowerUpdate };

// Climb up loop (right hand up).
static panimframe_t player_frames_climb_up_r[] =
{
	FRAME_Rclimb1, PlayerClimbingMoveFunc, 2, 0, 0, PlayerActionCheckRopeMove, 0, NULL,
	FRAME_Rclimb2, PlayerClimbingMoveFunc, 4, 0, 0, PlayerActionCheckRopeMove, 0, NULL,
	FRAME_Rclimb3, PlayerClimbingMoveFunc, 8, 0, 0, PlayerActionCheckRopeMove, 0, NULL,
	FRAME_Rclimb4, PlayerClimbingMoveFunc, 4, 0, 0, PlayerActionCheckRopeMove, 0, NULL,
};
panimmove_t	player_move_climb_up_r = { 4, player_frames_climb_up_r, PlayerAnimLowerUpdate };

// Climb up loop (left hand up).
static panimframe_t player_frames_climb_up_l[] =
{
	FRAME_Lclimb1, PlayerClimbingMoveFunc, 2, 0, 0, PlayerActionCheckRopeMove, 0, NULL,
	FRAME_Lclimb2, PlayerClimbingMoveFunc, 4, 0, 0, PlayerActionCheckRopeMove, 0, NULL,
	FRAME_Lclimb3, PlayerClimbingMoveFunc, 8, 0, 0, PlayerActionCheckRopeMove, 0, NULL,
	FRAME_Lclimb4, PlayerClimbingMoveFunc, 4, 0, 0, PlayerActionCheckRopeMove, 0, NULL,
};
panimmove_t	player_move_climb_up_l = { 4, player_frames_climb_up_l, PlayerAnimLowerUpdate };

// Climb down loop (right hand up).
static panimframe_t player_frames_climb_down_r[] =
{
	FRAME_Rclimb4, PlayerClimbingMoveFunc, -6,  0, 0, PlayerActionCheckRopeMove, 0, NULL,
	FRAME_Rclimb3, PlayerClimbingMoveFunc, -12, 0, 0, PlayerActionCheckRopeMove, 0, NULL,
	FRAME_Rclimb2, PlayerClimbingMoveFunc, -4,  0, 0, PlayerActionCheckRopeMove, 0, NULL,
	FRAME_Rclimb1, PlayerClimbingMoveFunc, -2,  0, 0, PlayerActionCheckRopeMove, 0, NULL,
};
panimmove_t	player_move_climb_down_r = { 4, player_frames_climb_down_r, PlayerAnimLowerUpdate };

// Climb down loop (left hand up).
static panimframe_t player_frames_climb_down_l[] =
{
	FRAME_Lclimb4, PlayerClimbingMoveFunc, -6,  0, 0, PlayerActionCheckRopeMove, 0, NULL,
	FRAME_Lclimb3, PlayerClimbingMoveFunc, -12, 0, 0, PlayerActionCheckRopeMove, 0, NULL,
	FRAME_Lclimb2, PlayerClimbingMoveFunc, -4,  0, 0, PlayerActionCheckRopeMove, 0, NULL,
	FRAME_Lclimb1, PlayerClimbingMoveFunc, -2,  0, 0, PlayerActionCheckRopeMove, 0, NULL,
};
panimmove_t	player_move_climb_down_l = { 4, player_frames_climb_down_l, PlayerAnimLowerUpdate };

// Transfer from climb to idle (right hand up).
static panimframe_t player_frames_climb_settle_r[] =
{
	FRAME_Rhndset1, PlayerClimbingMoveFunc, 0, 0, 0, PlayerActionCheckRopeMove, 0, NULL,
	FRAME_Rhndset2, PlayerClimbingMoveFunc, 0, 0, 0, PlayerActionCheckRopeMove, 0, NULL,
	FRAME_Rhndset3, PlayerClimbingMoveFunc, 0, 0, 0, PlayerActionCheckRopeMove, 0, NULL,
};
panimmove_t	player_move_climb_settle_r = { 3, player_frames_climb_settle_r, PlayerAnimLowerUpdate };

// Transfer from climb to idle (left hand up).
static panimframe_t player_frames_climb_settle_l[] =
{
	FRAME_Lhndset1, PlayerClimbingMoveFunc, 0, 0, 0, PlayerActionCheckRopeMove, 0, NULL,
	FRAME_Lhndset2, PlayerClimbingMoveFunc, 0, 0, 0, PlayerActionCheckRopeMove, 0, NULL,
	FRAME_Lhndset3, PlayerClimbingMoveFunc, 0, 0, 0, PlayerActionCheckRopeMove, 0, NULL,
	FRAME_Lhndset4, PlayerClimbingMoveFunc, 0, 0, 0, PlayerActionCheckRopeMove, 0, NULL,
	FRAME_Lhndset5, PlayerClimbingMoveFunc, 0, 0, 0, PlayerActionCheckRopeMove, 0, NULL,
};
panimmove_t	player_move_climb_settle_l = { 5, player_frames_climb_settle_l, PlayerAnimLowerUpdate };

// Transfer off the rope.
static panimframe_t player_frames_climb_off[] =
{
	FRAME_grabrope7, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_grabrope5, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_grabrope3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_grabrope1, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t	player_move_climb_off = { 4, player_frames_climb_off, PlayerAnimLowerUpdate };

#pragma endregion

#pragma region ========================== PLAYER KNOCKDOWN ANIMATIONS ==========================

// Knockdown animation.
static panimframe_t player_frames_knockdown[] =
{
	FRAME_kodown1,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_kodown2,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_kodown3,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_kodown4,  NULL, 0, 0, 0, NULL, 0, PlayerActionSetCrouchHeight,
	FRAME_kodown5,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_kodown6,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_kodown7,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_kodown8,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_kodown9,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_kodown10, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_kodown11, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_kodown12, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_kodown13, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_kodown14, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_kodown15, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_kodown16, NULL, 0, 0, 0, NULL, 0, NULL
};
panimmove_t	player_move_knockdown = { 16, player_frames_knockdown, PlayerAnimLowerUpdate };

// Get up from knockdown animation.
static panimframe_t player_frames_knockdown_getup[] =
{
	FRAME_kodown17, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_kodown18, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_kodown19, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_kodown20, NULL, 0, 0, 0, NULL, 0, PlayerActionCheckUncrouchToFinishSeq,
	FRAME_kodown21, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_kodown22, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t	player_move_knockdown_getup = { 6, player_frames_knockdown_getup, PlayerAnimLowerUpdate };

// Evade knockdown animation.
static panimframe_t player_frames_knockdown_evade[] =
{
	FRAME_backroll5, PlayerMoveFunc, -100 * PHYS_SCALER, 0, 0, PlayerActionFootstep, 0, PlayerActionSetCrouchHeight,
	FRAME_backroll6, PlayerMoveFunc, -90 * PHYS_SCALER,  0, 0, SpawnDustPuff, 0, NULL,
	FRAME_backroll7, PlayerMoveFunc, -75 * PHYS_SCALER,  0, 0, NULL, 0, NULL,
	FRAME_backroll8, PlayerMoveFunc, -50 * PHYS_SCALER,  0, 0, NULL, 0, NULL,
	FRAME_backroll9, PlayerMoveFunc, -25 * PHYS_SCALER,  0, 0, NULL, 0, NULL,
};
panimmove_t	player_move_knockdown_evade = { 5, player_frames_knockdown_evade, PlayerAnimLowerUpdate };

#pragma endregion

#pragma region ========================== PLAYER VISITS SHRINE ANIMATION ==========================

// Use shrine animation
static panimframe_t player_frames_shrine[] =
{
	FRAME_shrine1,    NULL, 0, 0, 0, PlayerActionShrineEffect, 0, NULL,
	FRAME_shrine2,    NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_shrine3,    NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_shrine4,    NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_shrine5,    NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_shrine6,    NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_shrine7,    NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_shrine8,    NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_shrine9,    NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_shrine10,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_shrine10,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_shrine10,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_shrine10,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_shrine10,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_shrine10,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_shrine10,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_shrine10,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_trnshrine1, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_trnshrine2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_trnshrine3, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t	player_move_shrine = { 20, player_frames_shrine, PlayerAnimLowerUpdate };

#pragma endregion

#pragma region ========================== PLAYER STRAFING ANIMATIONS ==========================

//NOTE: If you alter the walking, running, or creeping animations, those MUST be directly mirrored in these structures!

// Strafe-walk left.
static panimframe_t player_frames_walk_strafe_left[] =
{
	FRAME_Lstrafe1, PlayerMoveFunc, 160 * PHYS_SCALER, -150, 0, PlayerActionFootstep, STEP_WALK, PlayerActionCheckWalkUnStrafe,
	FRAME_Lstrafe2, PlayerMoveFunc, 170 * PHYS_SCALER, -150, 0, NULL, 0, PlayerActionCheckWalkUnStrafe,
	FRAME_Lstrafe3, PlayerMoveFunc, 160 * PHYS_SCALER, -150, 0, NULL, 0, PlayerActionCheckWalkUnStrafe,
	FRAME_Lstrafe4, PlayerMoveFunc, 155 * PHYS_SCALER, -150, 0, NULL, 0, PlayerActionCheckWalkUnStrafe,
	FRAME_Lstrafe5, PlayerMoveFunc, 160 * PHYS_SCALER, -150, 0, PlayerActionFootstep, STEP_WALK2, PlayerActionCheckWalkUnStrafe,
	FRAME_Lstrafe6, PlayerMoveFunc, 170 * PHYS_SCALER, -150, 0, NULL, 0, PlayerActionCheckWalkUnStrafe,
	FRAME_Lstrafe7, PlayerMoveFunc, 160 * PHYS_SCALER, -150, 0, NULL, 0, PlayerActionCheckWalkUnStrafe,
	FRAME_Lstrafe8, PlayerMoveFunc, 155 * PHYS_SCALER, -150, 0, NULL, 0, PlayerActionCheckWalkUnStrafe,
};
panimmove_t player_move_walk_strafe_left = { 8, player_frames_walk_strafe_left, PlayerAnimLowerUpdate };

// Strafe-walk right.
static panimframe_t player_frames_walk_strafe_right[] =
{
	FRAME_Rstrafe1, PlayerMoveFunc, 160 * PHYS_SCALER, 150, 0, PlayerActionFootstep, STEP_WALK, PlayerActionCheckWalkUnStrafe,
	FRAME_Rstrafe2, PlayerMoveFunc, 170 * PHYS_SCALER, 150, 0, NULL, 0, PlayerActionCheckWalkUnStrafe,
	FRAME_Rstrafe3, PlayerMoveFunc, 160 * PHYS_SCALER, 150, 0, NULL, 0, PlayerActionCheckWalkUnStrafe,
	FRAME_Rstrafe4, PlayerMoveFunc, 155 * PHYS_SCALER, 150, 0, NULL, 0, PlayerActionCheckWalkUnStrafe,
	FRAME_Rstrafe5, PlayerMoveFunc, 160 * PHYS_SCALER, 150, 0, PlayerActionFootstep, STEP_WALK2, PlayerActionCheckWalkUnStrafe,
	FRAME_Rstrafe6, PlayerMoveFunc, 170 * PHYS_SCALER, 150, 0, NULL, 0, PlayerActionCheckWalkUnStrafe,
	FRAME_Rstrafe7, PlayerMoveFunc, 160 * PHYS_SCALER, 150, 0, NULL, 0, PlayerActionCheckWalkUnStrafe,
	FRAME_Rstrafe8, PlayerMoveFunc, 155 * PHYS_SCALER, 150, 0, NULL, 0, PlayerActionCheckWalkUnStrafe,
};
panimmove_t player_move_walk_strafe_right = { 8, player_frames_walk_strafe_right, PlayerAnimLowerUpdate };

// Strafe-run left.
static panimframe_t player_frames_run_strafe_left[] =
{
	FRAME_Lsprint1, PlayerMoveFunc, 340 * PHYS_SCALER, -340 * PHYS_SCALER, 0, PlayerActionFootstep, STEP_RUN, PlayerActionCheckRunUnStrafe,
	FRAME_Lsprint2, PlayerMoveFunc, 340 * PHYS_SCALER, -340 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckRunUnStrafe,
	FRAME_Lsprint3, PlayerMoveFunc, 340 * PHYS_SCALER, -340 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckRunUnStrafe,
	FRAME_Lsprint4, PlayerMoveFunc, 340 * PHYS_SCALER, -340 * PHYS_SCALER, 0, PlayerActionFootstep, STEP_RUN2, PlayerActionCheckRunUnStrafe,
	FRAME_Lsprint5, PlayerMoveFunc, 340 * PHYS_SCALER, -340 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckRunUnStrafe,
	FRAME_Lsprint6, PlayerMoveFunc, 340 * PHYS_SCALER, -340 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckRunUnStrafe,
};
panimmove_t player_move_run_strafe_left = { 6, player_frames_run_strafe_left, PlayerAnimLowerUpdate };

// Strafe-run right.
static panimframe_t player_frames_run_strafe_right[] =
{
	FRAME_Rsprint1, PlayerMoveFunc, 340 * PHYS_SCALER, 340 * PHYS_SCALER, 0, PlayerActionFootstep, STEP_RUN, PlayerActionCheckRunUnStrafe,
	FRAME_Rsprint2, PlayerMoveFunc, 340 * PHYS_SCALER, 340 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckRunUnStrafe,
	FRAME_Rsprint3, PlayerMoveFunc, 340 * PHYS_SCALER, 340 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckRunUnStrafe,
	FRAME_Rsprint4, PlayerMoveFunc, 340 * PHYS_SCALER, 340 * PHYS_SCALER, 0, PlayerActionFootstep, STEP_RUN2, PlayerActionCheckRunUnStrafe,
	FRAME_Rsprint5, PlayerMoveFunc, 340 * PHYS_SCALER, 340 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckRunUnStrafe,
	FRAME_Rsprint6, PlayerMoveFunc, 340 * PHYS_SCALER, 340 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckRunUnStrafe,
};
panimmove_t player_move_run_strafe_right = { 6, player_frames_run_strafe_right, PlayerAnimLowerUpdate };

// Strafe-walk back-left.
static panimframe_t player_frames_walkb_strafe_left[] =
{
	FRAME_Ljogback1, PlayerMoveFunc, -160 * PHYS_SCALER, -150, 0, PlayerActionFootstep, STEP_WALK, PlayerActionCheckWalkBackUnStrafe,
	FRAME_Ljogback2, PlayerMoveFunc, -170 * PHYS_SCALER, -150, 0, NULL, 0, PlayerActionCheckWalkBackUnStrafe,
	FRAME_Ljogback3, PlayerMoveFunc, -160 * PHYS_SCALER, -150, 0, NULL, 0, PlayerActionCheckWalkBackUnStrafe,
	FRAME_Ljogback4, PlayerMoveFunc, -155 * PHYS_SCALER, -150, 0, NULL, 0, PlayerActionCheckWalkBackUnStrafe,
	FRAME_Ljogback5, PlayerMoveFunc, -160 * PHYS_SCALER, -150, 0, PlayerActionFootstep, STEP_WALK2, PlayerActionCheckWalkBackUnStrafe,
	FRAME_Ljogback6, PlayerMoveFunc, -170 * PHYS_SCALER, -150, 0, NULL, 0, PlayerActionCheckWalkBackUnStrafe,
};
panimmove_t player_move_walkb_strafe_left = { 6, player_frames_walkb_strafe_left, PlayerAnimLowerUpdate };

// Strafe-walk back-right.
static panimframe_t player_frames_walkb_strafe_right[] =
{
	FRAME_Rjogback1, PlayerMoveFunc, -160 * PHYS_SCALER, 150, 0, PlayerActionFootstep, STEP_WALK, PlayerActionCheckWalkBackUnStrafe,
	FRAME_Rjogback2, PlayerMoveFunc, -170 * PHYS_SCALER, 150, 0, NULL, 0, PlayerActionCheckWalkBackUnStrafe,
	FRAME_Rjogback3, PlayerMoveFunc, -160 * PHYS_SCALER, 150, 0, NULL, 0, PlayerActionCheckWalkBackUnStrafe,
	FRAME_Rjogback4, PlayerMoveFunc, -155 * PHYS_SCALER, 150, 0, NULL, 0, PlayerActionCheckWalkBackUnStrafe,
	FRAME_Rjogback5, PlayerMoveFunc, -160 * PHYS_SCALER, 150, 0, PlayerActionFootstep, STEP_WALK2, PlayerActionCheckWalkBackUnStrafe,
	FRAME_Rjogback6, PlayerMoveFunc, -170 * PHYS_SCALER, 150, 0, NULL, 0, PlayerActionCheckWalkBackUnStrafe,
};
panimmove_t player_move_walkb_strafe_right = { 6, player_frames_walkb_strafe_right, PlayerAnimLowerUpdate };

#pragma endregion

#pragma region ========================== PLAYER 180 TURN ANIMATION ==========================

// Turn 180
static panimframe_t player_frames_turn180[] =
{
	FRAME_180turn2, NULL, 0, 0, 0, PlayerActionSetQTEndTime, 0, PlayerActionTurn180,
	FRAME_180turn3, NULL, 0, 0, 0, NULL, 0, PlayerActionTurn180,
	FRAME_180turn5, NULL, 0, 0, 0, NULL, 0, PlayerActionTurn180,
	FRAME_180turn7, NULL, 0, 0, 0, NULL, 0, PlayerActionTurn180,
	FRAME_180turn9, NULL, 0, 0, 0, NULL, 0, PlayerActionTurn180,
};
panimmove_t player_move_turn180 = { 5, player_frames_turn180, PlayerAnimLowerUpdate };

#pragma endregion

#pragma region ========================== PLAYER DASH ANIMATIONS ==========================

// Dashing left start.
static panimframe_t player_frames_dash_left_go[] =
{
	FRAME_fastleft1, PlayerMoveFunc, 0, -200 * PHYS_SCALER, 0, PlayerActionFootstep, STEP_RUN, PlayerActionCheckBranchRunningStrafe,
};
panimmove_t player_move_dash_left_go = { 1, player_frames_dash_left_go, PlayerAnimLowerUpdate };

// Dashing left.
static panimframe_t player_frames_dash_left[] =
{
	FRAME_fastleft2, PlayerMoveFunc, 0, -200 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckBranchRunningStrafe,
	FRAME_fastleft3, PlayerMoveFunc, 0, -200 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckBranchRunningStrafe,
	FRAME_fastleft4, PlayerMoveFunc, 0, -200 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckBranchRunningStrafe,
	FRAME_fastleft5, PlayerMoveFunc, 0, -200 * PHYS_SCALER, 0, PlayerActionFootstep, STEP_RUN, PlayerActionCheckBranchRunningStrafe,
	FRAME_fastleft6, PlayerMoveFunc, 0, -200 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckBranchRunningStrafe,
	FRAME_fastleft7, PlayerMoveFunc, 0, -200 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckBranchRunningStrafe,
	FRAME_fastleft8, PlayerMoveFunc, 0, -200 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckBranchRunningStrafe,
};
panimmove_t player_move_dash_left = { 7, player_frames_dash_left, PlayerAnimLowerUpdate };

// Dashing right start.
static panimframe_t player_frames_dash_right_go[] =
{
	FRAME_fastrite1, PlayerMoveFunc, 0, 200 * PHYS_SCALER, 0, PlayerActionFootstep, STEP_RUN, PlayerActionCheckBranchRunningStrafe,
};
panimmove_t player_move_dash_right_go = { 1, player_frames_dash_right_go, PlayerAnimLowerUpdate };

// Dashing right.
static panimframe_t player_frames_dash_right[] =
{
	FRAME_fastrite2, PlayerMoveFunc, 0, 200 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckBranchRunningStrafe,
	FRAME_fastrite3, PlayerMoveFunc, 0, 200 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckBranchRunningStrafe,
	FRAME_fastrite4, PlayerMoveFunc, 0, 200 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckBranchRunningStrafe,
	FRAME_fastrite5, PlayerMoveFunc, 0, 200 * PHYS_SCALER, 0, PlayerActionFootstep, STEP_RUN, PlayerActionCheckBranchRunningStrafe,
	FRAME_fastrite6, PlayerMoveFunc, 0, 200 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckBranchRunningStrafe,
	FRAME_fastrite7, PlayerMoveFunc, 0, 200 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckBranchRunningStrafe,
	FRAME_fastrite8, PlayerMoveFunc, 0, 200 * PHYS_SCALER, 0, NULL, 0, PlayerActionCheckBranchRunningStrafe,
};
panimmove_t player_move_dash_right = { 7, player_frames_dash_right, PlayerAnimLowerUpdate };

#pragma endregion