//
// p_anim_data.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "p_anims.h"

typedef struct seqctrl_s
{
	int command;
	int continueseq;
	int ceaseseq;
	int (*branchfunc)(playerinfo_t* playerinfo);
} seqctrl_t;

enum weaponloc_e
{
	WPLOC_NONE,
	WPLOC_BOTH,
	WPLOC_HAND,
	WPLOC_BACK
};

enum footsteptype_e
{
	STEP_NONE,
	STEP_SHUFFLE, //mxd
	STEP_CREEP,
	STEP_WALK,
	STEP_RUN,
	STEP_ROLL,

	STEP_OFFSET,
	STEP_SHUFFLE2, //mxd
	STEP_CREEP2,
	STEP_WALK2,
	STEP_RUN2,
	STEP_ROLL2,
};

enum trailtype_e
{
	TRAIL_SPIN1,
	TRAIL_SPIN2,
	TRAIL_STAND,
	TRAIL_STEP,
	TRAIL_BACK,
	TRAIL_STAB,
	TRAIL_COUNTERLEFT,
	TRAIL_COUNTERRIGHT,
	TRAIL_MAX
};

extern char* SeqNames[ASEQ_MAX];
extern int PlayerAnimWeaponSwitchSeq[WEAPON_READY_MAX][WEAPON_READY_MAX];
extern seqctrl_t SeqCtrl[ASEQ_MAX];
extern PLAYER_API paceldata_t PlayerSeqData[ASEQ_MAX];
extern seqctrl_t ChickenCtrl[ASEQ_MAX];
extern PLAYER_API paceldata_t PlayerChickenData[ASEQ_MAX];

extern panimmove_t player_move_nothing;
extern panimmove_t player_move_stand;
extern panimmove_t player_move_vaulthigh;
extern panimmove_t player_move_pullupwall;
extern panimmove_t player_move_spellfireball;
extern panimmove_t player_move_spellarray;
extern panimmove_t player_move_spellspherestart;
extern panimmove_t player_move_spellspherecharge;
extern panimmove_t player_move_spellspherefire1;
extern panimmove_t player_move_spellspherefire2;
extern panimmove_t player_move_spellspherefire3;
extern panimmove_t player_move_spellspherefire4;
extern panimmove_t player_move_spellfirewall;
extern panimmove_t player_move_spellripper;
extern panimmove_t player_move_spellbigball;
extern panimmove_t player_move_spellblast;
extern panimmove_t player_move_bowready;
extern panimmove_t player_move_rrbowdrawarrow;
extern panimmove_t player_move_phbowdrawarrow;
extern panimmove_t player_move_bowholdarrow;
extern panimmove_t player_move_rrbowfire;
extern panimmove_t player_move_phbowfire;
extern panimmove_t player_move_bowholdready;
extern panimmove_t player_move_bowunready;
extern panimmove_t player_move_hellready;	// Ready hellstaff
extern panimmove_t player_move_hellfire1;	// Fire hellstaff
extern panimmove_t player_move_hellfire2;	// Fire hellstaff
extern panimmove_t player_move_hellunready;	// Release hellstaff
extern panimmove_t player_move_spelldefensive;
extern panimmove_t player_move_spellchange;
extern panimmove_t player_move_staffatkstand1;
extern panimmove_t player_move_staffatkstand2;
extern panimmove_t player_move_staffatkspin;			// Spin attack
extern panimmove_t player_move_staffatkspin2;			// Spin attack
extern panimmove_t player_move_jumpfliphandspringgo;	// Handspring Start
extern panimmove_t player_move_jumpfliphandspring;		// Handspring
extern panimmove_t player_move_staffatkstep2;
extern panimmove_t player_move_staffatkstep;
extern panimmove_t player_move_drawsword;
extern panimmove_t player_move_drawhell;
extern panimmove_t player_move_drawbow;
extern panimmove_t player_move_stowsword;
extern panimmove_t player_move_sword2hell;
extern panimmove_t player_move_sword2bow;
extern panimmove_t player_move_stowhell;
extern panimmove_t player_move_hell2sword;
extern panimmove_t player_move_hell2bow;
extern panimmove_t player_move_stowbow;
extern panimmove_t player_move_bow2sword;
extern panimmove_t player_move_bow2hell;
extern panimmove_t player_move_bow2bow;
extern panimmove_t player_move_pushbuttongo;
extern panimmove_t player_move_pushleverleft;
extern panimmove_t player_move_pushleverright;
extern panimmove_t player_move_takepuzzlepiece;
extern panimmove_t player_move_pivotleftgo;		// Pivot to the left start anims.
extern panimmove_t player_move_pivotleft;		// Pivot to the left continue anims.
extern panimmove_t player_move_pivotleftend;	// Pivot to the left end.
extern panimmove_t player_move_pivotrightgo;	// Pivot to the right start anims.
extern panimmove_t player_move_pivotright;		// Pivot to the right continue anims.
extern panimmove_t player_move_pivotrightend;	// Pivot to the right end.
extern panimmove_t player_move_turn180;
extern panimmove_t player_move_runstart;
extern panimmove_t player_move_run;
extern panimmove_t player_move_runstop;
extern panimmove_t player_move_walkstart;
extern panimmove_t player_move_walk;
extern panimmove_t player_move_walkstop;
extern panimmove_t player_move_creepforward;
extern panimmove_t player_move_creepforward_end;

extern panimmove_t player_move_walkback;

extern panimmove_t player_move_creepback;
extern panimmove_t player_move_creepback_end;

extern panimmove_t player_move_crouchdown;
extern panimmove_t player_move_crouch;
extern panimmove_t player_move_crouchup;
extern panimmove_t player_move_crouchpivotleft;
extern panimmove_t player_move_crouchpivotright;

extern panimmove_t player_move_strafeleft;
extern panimmove_t player_move_strafeleft_end;

extern panimmove_t player_move_straferight;
extern panimmove_t player_move_straferight_end;

extern panimmove_t player_move_standjumpstart;
extern panimmove_t player_move_standjumpfwdstart;
extern panimmove_t player_move_walkjumpfwdstart;
extern panimmove_t player_move_runjumpfwdstart;
extern panimmove_t player_move_jumpfwd;
extern panimmove_t player_move_jumpup;
extern panimmove_t player_move_jumpuploop;
extern panimmove_t player_move_jumpflipleft;
extern panimmove_t player_move_jumpflipright;
extern panimmove_t player_move_jumpflipback;
extern panimmove_t player_move_rolldivefwdwalk;
extern panimmove_t player_move_rolldivefwdrun;
extern panimmove_t player_move_rollfromfflip;
extern panimmove_t player_move_polevault1walk;
extern panimmove_t player_move_polevault1run;
extern panimmove_t player_move_polevault2;
extern panimmove_t player_move_land1;
extern panimmove_t player_move_land3;
extern panimmove_t player_move_fall;
extern panimmove_t player_move_fallarmsup;
extern panimmove_t player_move_fallwalkstart;
extern panimmove_t player_move_fallwalkloop;
extern panimmove_t player_move_vaultwall;
extern panimmove_t player_move_sswimidle;
extern panimmove_t player_move_sswimfwdgo;
extern panimmove_t player_move_sswimfwd;
extern panimmove_t player_move_sswimfwdstop;
extern panimmove_t player_move_sswimbackgo;
extern panimmove_t player_move_sswimback;
extern panimmove_t player_move_sswimbackstop;
extern panimmove_t player_move_sswim_left_go;
extern panimmove_t player_move_sswim_right_go;
extern panimmove_t player_move_sswim_left;
extern panimmove_t player_move_sswim_right;
extern panimmove_t player_move_sswim_left_stop;
extern panimmove_t player_move_sswim_right_stop;
extern panimmove_t player_move_pulluphalfwall;
extern panimmove_t player_move_tumbleon1;
extern panimmove_t player_move_tumbleon2;
extern panimmove_t player_move_lstair4;
extern panimmove_t player_move_lstair8;
extern panimmove_t player_move_lstair12;
extern panimmove_t player_move_lstair16;
extern panimmove_t player_move_rstair4;
extern panimmove_t player_move_rstair8;
extern panimmove_t player_move_rstair12;
extern panimmove_t player_move_rstair16;
extern panimmove_t player_move_standreadystart;
extern panimmove_t player_move_standready;
extern panimmove_t player_move_standreadyend;
extern panimmove_t player_move_standlookleft;
extern panimmove_t player_move_standlookright;
extern panimmove_t player_move_paina;
extern panimmove_t player_move_painb;
extern panimmove_t player_move_pest1;
extern panimmove_t player_move_pest2;
extern panimmove_t player_move_fallleft;
extern panimmove_t player_move_fallright;
extern panimmove_t player_move_fallleftend;
extern panimmove_t player_move_fallrightend;
extern panimmove_t player_move_death1;
extern panimmove_t player_move_uswimfwd_go;
extern panimmove_t player_move_uswimfwd;
extern panimmove_t player_move_uswimfwd_end;
extern panimmove_t player_move_dive1;
extern panimmove_t player_move_uswimbackgo;
extern panimmove_t player_move_uswimback;
extern panimmove_t player_move_uswimbackstop;
extern panimmove_t player_move_uswim_left_go;
extern panimmove_t player_move_uswim_right_go;
extern panimmove_t player_move_uswim_left;
extern panimmove_t player_move_uswim_right;
extern panimmove_t player_move_uswim_left_stop;
extern panimmove_t player_move_uswim_right_stop;
extern panimmove_t player_move_slide_forward;
extern panimmove_t player_move_slide_backward;
extern panimmove_t player_move_resurface;
extern panimmove_t player_move_roll_l;
extern panimmove_t player_move_roll_r;
extern panimmove_t player_move_idle_under;
extern panimmove_t player_move_roll_b;
extern panimmove_t player_move_climb_on;
extern panimmove_t player_move_climb_up_start_r;
extern panimmove_t player_move_climb_up_start_l;
extern panimmove_t player_move_climb_up_r;
extern panimmove_t player_move_climb_up_l;
extern panimmove_t player_move_climb_down_start_r;
extern panimmove_t player_move_climb_down_start_l;
extern panimmove_t player_move_climb_down_r;
extern panimmove_t player_move_climb_down_l;
extern panimmove_t player_move_climb_off;
extern panimmove_t player_move_climb_hold_r;
extern panimmove_t player_move_climb_hold_l;
extern panimmove_t player_move_climb_settle_r;
extern panimmove_t player_move_climb_settle_l;
extern panimmove_t player_move_knockdown;
extern panimmove_t player_move_knockdown_getup;
extern panimmove_t player_move_knockdown_evade;
extern panimmove_t player_move_shrine;
extern panimmove_t player_move_takepuzzleunderwater;
extern panimmove_t player_move_drown;
extern panimmove_t player_move_forward_flip_l_go;
extern panimmove_t player_move_forward_flip_r_go;
extern panimmove_t player_move_forward_flip_l;
extern panimmove_t player_move_forward_flip_r;

extern panimmove_t player_move_walk_strafe_left;
extern panimmove_t player_move_walk_strafe_right;

extern panimmove_t player_move_run_strafe_left;
extern panimmove_t player_move_run_strafe_right;

extern panimmove_t player_move_standjumpbackstart;
extern panimmove_t player_move_walkjumpbackstart;
extern panimmove_t player_move_runjumpbackstart;
extern panimmove_t player_move_jumpback;
extern panimmove_t player_move_jumpbackflip;

extern panimmove_t player_move_standjumpleftstart;
extern panimmove_t player_move_walkjumpleftstart;
extern panimmove_t player_move_runjumpleftstart;
extern panimmove_t player_move_jumpleft;
extern panimmove_t player_move_jumpleftflip;

extern panimmove_t player_move_standjumprightstart;
extern panimmove_t player_move_walkjumprightstart;
extern panimmove_t player_move_runjumprightstart;
extern panimmove_t player_move_jumpright;
extern panimmove_t player_move_jumprightflip;

extern panimmove_t player_move_drown_idle;

extern panimmove_t player_move_dash_left_go;
extern panimmove_t player_move_dash_left;
extern panimmove_t player_move_dash_right_go;
extern panimmove_t player_move_dash_right;

extern panimmove_t player_move_walkb_strafe_left;
extern panimmove_t player_move_walkb_strafe_right;

extern panimmove_t player_move_overhang;
extern panimmove_t player_move_death_b;
extern panimmove_t player_move_death_fly_forward;
extern panimmove_t player_move_death_fly_back;
extern panimmove_t player_move_death_choke;

extern panimmove_t player_move_idle_lookback;
extern panimmove_t player_move_idle_scratch_ass;
extern panimmove_t player_move_idle_wipe_brow;

extern panimmove_t player_move_creepb_strafe_left;
extern panimmove_t player_move_creepb_strafe_right;
extern panimmove_t player_move_creep_strafe_left;
extern panimmove_t player_move_creep_strafe_right;

extern panimmove_t player_move_crouch_creep_forward;
extern panimmove_t player_move_crouch_creep_back;
extern panimmove_t player_move_crouch_creep_left;
extern panimmove_t player_move_crouch_creep_right;

extern panimmove_t player_move_swim_fast_go;
extern panimmove_t player_move_swim_fast;

extern panimmove_t player_move_staffatkback;
extern panimmove_t player_move_staffdownstab;
extern panimmove_t player_move_staffstabhold;
extern panimmove_t player_move_staffpullout;
extern panimmove_t player_move_staffblockleft;
extern panimmove_t player_move_staffblockleft2;
extern panimmove_t player_move_staffblockleftatk;
extern panimmove_t player_move_staffblockright;
extern panimmove_t player_move_staffblockright2;
extern panimmove_t player_move_staffblockrightatk;
extern panimmove_t player_move_staffblockedleft;
extern panimmove_t player_move_staffblockedright;
extern panimmove_t player_move_staffspinblockedleft;
extern panimmove_t player_move_staffspinblockedright;
extern panimmove_t player_move_stafflowerdownstab;
extern panimmove_t player_move_stafflowerpullout;