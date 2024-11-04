//
// p_anim_data2.h
//
// Heretic II
// Copyright 1998 Raven Software
//

#ifndef _P_ANIM_DATA2_H_
#define _P_ANIM_DATA2_H_

#include "p_anims.h"

typedef struct seqctrl_s
{
	int command;
	int continueseq;
	int ceaseseq;
	int (*branchfunc)(playerinfo_t *playerinfo);
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
	STEP_CREEP,
	STEP_WALK,
	STEP_RUN,
	STEP_ROLL,

	STEP_OFFSET,
	STEP_CREEP2,
	STEP_WALK2,
	STEP_RUN2,
	STEP_ROLL2,

	STEP_MAX
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

extern char *SeqName[ASEQ_MAX];
extern int PlayerAnimWeaponSwitchSeq[WEAPON_READY_MAX][WEAPON_READY_MAX];
extern seqctrl_t SeqCtrl[ASEQ_MAX];
extern PLAYER_API paceldata_t PlayerSeqData[ASEQ_MAX];
extern seqctrl_t ChickenCtrl[ASEQ_MAX];
extern PLAYER_API paceldata_t PlayerChickenData[ASEQ_MAX];

extern panimmove_t 
	player_move_nothing,			
	player_move_stand,			
	player_move_vaulthigh,		
	player_move_grabstartwall,	
	player_move_grabstartwall,	
	player_move_grabhold,		
	player_move_grabmoveleft,	
	player_move_grabmoveright,	
	player_move_pullupwall,	
	player_move_pullupwall,	
	player_move_spellfireball,		
	player_move_spellarray,			
	player_move_spellspherestart,	
	player_move_spellspherecharge,	
	player_move_spellspherefire1,
	player_move_spellspherefire2,
	player_move_spellspherefire3,
	player_move_spellspherefire4,
	player_move_spellfirewall,		
	player_move_spellripper,	
	player_move_spellbigball,	
	player_move_spellblast,
	player_move_bowready,	
	player_move_rrbowdrawarrow,
	player_move_phbowdrawarrow,
	player_move_bowholdarrow,
	player_move_rrbowfire,	
	player_move_phbowfire,	
	player_move_bowholdready,
	player_move_bowunready,	
	player_move_hellready,					// Ready hellstaff
	player_move_hellfire1,					// Fire hellstaff
	player_move_hellfire2,					// Fire hellstaff
	player_move_hellunready,				// Release hellstaff
	player_move_spelldefensive,
	player_move_spellchange,	
	player_move_staffatkstand1,
	player_move_staffatkstand2,
	player_move_staffatkspin,				// Spin attack
	player_move_staffatkspin2,				// Spin attack
	player_move_jumpflipback,				// Backflip
	player_move_jumpfliphandspringgo,		// Handspring Start
	player_move_jumpfliphandspring,		    // Handspring
	player_move_staffatkstep2,
	player_move_staffatkstep,
	player_move_drawsword,		
	player_move_drawhell,		
	player_move_drawbow,		
	player_move_stowsword,		
	player_move_sword2hell,		
	player_move_sword2bow,		
	player_move_stowhell,		
	player_move_hell2sword,		
	player_move_hell2bow,		
	player_move_stowbow,		
	player_move_bow2sword,		
	player_move_bow2hell,
	player_move_bow2bow,
	player_move_pushbuttongo,		
	player_move_pushleverleft,		
	player_move_pushleverright,		
	player_move_takepuzzlepiece,		
	player_move_pivotleftgo,		// Pivot to the left start anims
	player_move_pivotleft,			// Pivot to the left continue anims
	player_move_pivotleftend,		// Pivot to the left end
	player_move_pivotrightgo,		// Pivot to the right start anims
	player_move_pivotright,			// Pivot to the right continue anims
	player_move_pivotrightend,		// Pivot to the right end
	player_move_turn180,		
	player_move_runstart,		
	player_move_run,			
	player_move_runstop,		
	player_move_walkstart,
	player_move_walk,
	player_move_walkstop,		
	player_move_creepforward,		
	player_move_creepforward_end,		

	player_move_walkback,		

	player_move_creepback,		
	player_move_creepback_end,

	player_move_crouchdown,	
	player_move_crouch,		
	player_move_crouchup,		
	player_move_crouchpivotleft,		
	player_move_crouchpivotright,		

	player_move_strafeleft,	
	player_move_strafeleft_end,

	player_move_straferight,	
	player_move_straferight_end,

	player_move_standjumpstart,
	player_move_standjumpfwdstart,		
	player_move_walkjumpfwdstart,		
	player_move_runjumpfwdstart,		
	player_move_jumpfwd,		
	player_move_jumpup,		
	player_move_jumpuploop,
	player_move_jumpback,		
	player_move_jumpflipleft,	
	player_move_jumpflipright,	
	player_move_jumpflipback,	
	player_move_rolldivefwdwalk,	
	player_move_rolldivefwdrun,	
	player_move_rollfromfflip,
	player_move_dash_forward,
	player_move_dash_back,
	player_move_polevault1walk,
	player_move_polevault1run,
	player_move_polevault2,
	player_move_land1,			
	player_move_land3,			
	player_move_slide,			
	player_move_fall,			
	player_move_fallarmsup,			
	player_move_fallwalkstart,		
	player_move_fallwalkloop,		
	player_move_painstand,		
	player_move_painstand,		
	player_move_vaultwall,		
	player_move_sswimgo,		
	player_move_sswimidle,		
	player_move_sswimfwdgo,	
	player_move_sswimfwd,		
	player_move_sswimfwdstop,	
	player_move_sswimbackgo,	
	player_move_sswimback,		
	player_move_sswimbackstop,	
	player_move_sswim_left_go,
	player_move_sswim_right_go,
	player_move_sswim_left,
	player_move_sswim_right,
	player_move_sswim_left_stop,
	player_move_sswim_right_stop,
	player_move_swimstart,
	player_move_grabstart,
	player_move_grabloop,
	player_move_pullup,
	player_move_pulluphalfwall,
	player_move_tumbleon1,
	player_move_tumbleon2,
	player_move_lstair4,
	player_move_lstair8,
	player_move_lstair12,
	player_move_lstair16,
	player_move_rstair4,
	player_move_rstair8,
	player_move_rstair12,
	player_move_rstair16,
	player_move_standreadystart,
	player_move_standready,
	player_move_standreadyend,
	player_move_standlookleft,
	player_move_standlookright,
	player_move_paina,
	player_move_painb,
	player_move_pest1,
	player_move_pest2,
	player_move_fallleft,
	player_move_fallright,
	player_move_fallleftend,
	player_move_fallrightend,
	player_move_death1,
	player_move_uswimfwd_go,
	player_move_uswimfwd,
	player_move_uswimfwd_end,
	player_move_dive1,
	player_move_swimgo,		
	player_move_swimidle,		
	player_move_uswimbackgo,	
	player_move_uswimback,		
	player_move_uswimbackstop,	
	player_move_uswim_left_go,
	player_move_uswim_right_go,
	player_move_uswim_left,
	player_move_uswim_right,
	player_move_uswim_left_stop,
	player_move_uswim_right_stop,
	player_move_slide_forward,
	player_move_slide_backward,
	player_move_resurface,
	player_move_roll_l,
	player_move_roll_r,
	player_move_idle_under,
	player_move_roll_b,
	player_move_climb_on,
	player_move_climb_up_start_r,
	player_move_climb_up_start_l,
	player_move_climb_up_r,
	player_move_climb_up_l,
	player_move_climb_down_start_r,
	player_move_climb_down_start_l,
	player_move_climb_down_r,
	player_move_climb_down_l,
	player_move_climb_off,
	player_move_climb_hold_r,
	player_move_climb_hold_l,
	player_move_climb_settle_r,
	player_move_climb_settle_l,
	player_move_climb_idle_r,
	player_move_climb_idle_l,
	player_move_knockdown,
	player_move_knockdown_getup,
	player_move_knockdown_evade,
	player_move_shrine,
	player_move_pushpull_ready,
	player_move_pushpull_stand,
	player_move_push_go,
	player_move_push,
	player_move_push_end_l,
	player_move_push_end_r,
	player_move_takepuzzleunderwater,
	player_move_pull_go,
	player_move_pull,
	player_move_pull_end,
	player_move_drown,
	player_move_forward_flip_l_go,
	player_move_forward_flip_r_go,
	player_move_forward_flip_l,
	player_move_forward_flip_r,

	player_move_walk_strafe_left,
	player_move_walk_strafe_right,

	player_move_run_strafe_left,
	player_move_run_strafe_right,

	player_move_standjumpbackstart,
	player_move_walkjumpbackstart,	
	player_move_runjumpbackstart,	
	player_move_jumpback,
	player_move_jumpbackflip,

	player_move_standjumpleftstart,
	player_move_walkjumpleftstart,	
	player_move_runjumpleftstart,	
	player_move_jumpleft,			
	player_move_jumpleftflip,		

	player_move_standjumprightstart,
	player_move_walkjumprightstart,
	player_move_runjumprightstart,	
	player_move_jumpright,			
	player_move_jumprightflip,
	
	player_move_dash_back,
	player_move_dash_forward,

	player_move_drown_idle,

	player_move_turn180,
	
	player_move_dash_left_go,
	player_move_dash_left,
	player_move_dash_right_go,
	player_move_dash_right,
	
	player_move_walkb_strafe_left,
	player_move_walkb_strafe_right,

	player_move_overhang,
	player_move_death_b,
	player_move_death_fly_forward,
	player_move_death_fly_back,
	player_move_death_choke,
	
	player_move_idle_lookback,
	player_move_idle_scratch_ass,
	player_move_idle_wipe_brow,
	
	player_move_creepb_strafe_left,
	player_move_creepb_strafe_right,
	player_move_creep_strafe_left,
	player_move_creep_strafe_right,
	
	player_move_crouch_creep_forward,
	player_move_crouch_creep_back,
	player_move_crouch_creep_left,
	player_move_crouch_creep_right,
	
	player_move_swim_fast_go,
	player_move_swim_fast,

	player_move_staffatkback,
	player_move_staffdownstab,
	player_move_staffstabhold,
	player_move_staffpullout,
	player_move_staffblockleft,    
	player_move_staffblockleft2,    
	player_move_staffblockleftatk,
	player_move_staffblockright,  
	player_move_staffblockright2,  
	player_move_staffblockrightatk,
	player_move_staffblockedleft,
	player_move_staffblockedright,     
	player_move_staffspinblockedleft,
	player_move_staffspinblockedright,
	player_move_stafflowerdownstab,
	player_move_stafflowerpullout;

#endif // _P_ANIM_DATA2_H_

