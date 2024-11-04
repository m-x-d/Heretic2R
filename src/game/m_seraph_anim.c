#include "g_local.h"
#include "m_seraph.h"
#include "m_seraph_anim.h"
#include "g_monster.h"
#include "m_stats.h"

/*----------------------------------------------------
	seraph idles

	stand1		- standing, looking straight ahead
	stand1_tr	- standing, transitioning to looking right
	stand1_r	- standing, looking right
	stand1_trc	- standing, transition to look back to center from looking right
	stand1_tl	- standing, transitioning to looking left
	stand1_l	- standing, looking left
	stand1_trl	- standing, transition to look back to center from looking left

----------------------------------------------------*/

animframe_t seraph_frames_stand1[] =
{
	FRAME_idle1,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle2,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle3,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle4,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle5,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle6,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle7,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle8,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle9,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle10,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle11,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle12,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle13,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle14,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle15,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle16,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle17,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle18,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle19,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle20,  NULL, 0, 0, 0, ai_stand, 0, NULL,
};
animmove_t seraph_move_stand1 = {20,seraph_frames_stand1, seraph_idle};

//Look right

animframe_t seraph_frames_stand1_tr[] =
{
	FRAME_idle_htr1,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_htr2,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_htr3,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_htr4,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_htr5,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_htr6,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_htr7,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_htr8,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_htr9,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_htr10,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_htr11,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_htr12,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_htr13,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_htr14,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_htr15,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_htr16,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_htr17,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_htr18,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_htr19,  NULL, 0, 0, 0, ai_stand, 0, NULL,
};
animmove_t seraph_move_stand1_tr = {19,seraph_frames_stand1_tr, seraph_idle};

animframe_t seraph_frames_stand1_r[] =
{
	FRAME_idle_hrt1,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_hrt2,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_hrt3,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_hrt4,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_hrt5,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_hrt6,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_hrt7,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_hrt8,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_hrt9,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_hrt10,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_hrt11,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_hrt12,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_hrt13,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_hrt14,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_hrt15,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_hrt16,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_hrt17,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_hrt18,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_hrt19,  NULL, 0, 0, 0, ai_stand, 0, NULL,
};
animmove_t seraph_move_stand1_r = {19,seraph_frames_stand1_r, seraph_idle};

animframe_t seraph_frames_stand1_trc[] =
{
	FRAME_idle_hrc1,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_hrc2,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_hrc3,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_hrc4,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_hrc5,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_hrc6,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_hrc7,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_hrc8,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_hrc9,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_hrc10,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_hrc11,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_hrc12,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_hrc13,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_hrc14,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_hrc15,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_hrc16,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_hrc17,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_hrc18,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_hrc19,  NULL, 0, 0, 0, ai_stand, 0, NULL,
};
animmove_t seraph_move_stand1_trc = {19,seraph_frames_stand1_trc, seraph_idle};

//Look left

animframe_t seraph_frames_stand1_tl[] =
{
	FRAME_idle_htl1,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_htl2,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_idle_htl3,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_idle_htl4,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_idle_htl5,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_htl6,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_idle_htl7,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_idle_htl8,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_idle_htl9,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_htl10,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_idle_htl11,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_idle_htl12,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_idle_htl13,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_htl14,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_idle_htl15,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_idle_htl16,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_idle_htl17,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_htl18,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_idle_htl19,  NULL, 0, 0, 0, NULL, 0, NULL,
};
animmove_t seraph_move_stand1_tl = {19,seraph_frames_stand1_tl, seraph_idle};

animframe_t seraph_frames_stand1_l[] =
{
	FRAME_idle_hl1,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_hl2,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_idle_hl3,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_idle_hl4,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_idle_hl5,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_hl6,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_idle_hl7,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_idle_hl8,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_idle_hl9,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_hl10,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_idle_hl11,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_idle_hl12,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_idle_hl13,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_hl14,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_idle_hl15,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_idle_hl16,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_idle_hl17,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_hl18,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_idle_hl19,  NULL, 0, 0, 0, NULL, 0, NULL,
};
animmove_t seraph_move_stand1_l = {19,seraph_frames_stand1_l, seraph_idle};

animframe_t seraph_frames_stand1_tlc[] =
{
	FRAME_idle_hlc1,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_idle_hlc2,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_idle_hlc3,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_idle_hlc4,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_hlc5,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_idle_hlc6,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_idle_hlc7,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_idle_hlc8,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_hlc9,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_idle_hlc10,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_idle_hlc11,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_idle_hlc12,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_hlc13,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_idle_hlc14,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_idle_hlc15,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_idle_hlc16,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle_hlc17,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_idle_hlc18,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_idle_hlc19,  NULL, 0, 0, 0, NULL, 0, NULL,
};
animmove_t seraph_move_stand1_tlc = {19,seraph_frames_stand1_tlc, seraph_idle};

/*----------------------------------------------------
	seraph walking
----------------------------------------------------*/
animframe_t seraph_frames_walk1[] =
{
	FRAME_walk1,  NULL, 0, 0, 0, ai_walk, 6, NULL,
	FRAME_walk2,  NULL, 0, 0, 0, ai_walk, 6, NULL,
	FRAME_walk3,  NULL, 0, 0, 0, ai_walk, 6, NULL,
	FRAME_walk4,  NULL, 0, 0, 0, ai_walk, 5, NULL,
	FRAME_walk5,  NULL, 0, 0, 0, ai_walk, 4, NULL,
	FRAME_walk6,  NULL, 0, 0, 0, ai_walk, 4, NULL,
	FRAME_walk7,  NULL, 0, 0, 0, ai_walk, 5, NULL,
	FRAME_walk8,  NULL, 0, 0, 0, ai_walk, 6, NULL,
	FRAME_walk9,  NULL, 0, 0, 0, ai_walk, 6, NULL,
	FRAME_walk10, NULL, 0, 0, 0, ai_walk, 6, NULL,
	FRAME_walk11, NULL, 0, 0, 0, ai_walk, 6, NULL,
	FRAME_walk12, NULL, 0, 0, 0, ai_walk, 5, NULL,
	FRAME_walk13, NULL, 0, 0, 0, ai_walk, 4, NULL,
	FRAME_walk14, NULL, 0, 0, 0, ai_walk, 4, NULL,
	FRAME_walk15, NULL, 0, 0, 0, ai_walk, 5, NULL,
	FRAME_walk16, NULL, 0, 0, 0, ai_walk, 6, NULL,
};
animmove_t seraph_move_walk1 = {16,seraph_frames_walk1, seraph_pause};

animframe_t seraph_frames_walk2[] =
{
	FRAME_walk1,  NULL, 0, 0, 0, seraph_ai_walk, 6, NULL,
	FRAME_walk2,  NULL, 0, 0, 0, seraph_ai_walk, 6, NULL,
	FRAME_walk3,  NULL, 0, 0, 0, seraph_ai_walk, 6, NULL,
	FRAME_walk4,  NULL, 0, 0, 0, seraph_ai_walk, 5, NULL,
	FRAME_walk5,  NULL, 0, 0, 0, seraph_ai_walk, 4, NULL,
	FRAME_walk6,  NULL, 0, 0, 0, seraph_ai_walk, 4, NULL,
	FRAME_walk7,  NULL, 0, 0, 0, seraph_ai_walk, 5, NULL,
	FRAME_walk8,  NULL, 0, 0, 0, seraph_ai_walk, 6, NULL,
	FRAME_walk9,  NULL, 0, 0, 0, seraph_ai_walk, 6, NULL,
	FRAME_walk10, NULL, 0, 0, 0, seraph_ai_walk, 6, NULL,
	FRAME_walk11, NULL, 0, 0, 0, seraph_ai_walk, 6, NULL,
	FRAME_walk12, NULL, 0, 0, 0, seraph_ai_walk, 5, NULL,
	FRAME_walk13, NULL, 0, 0, 0, seraph_ai_walk, 4, NULL,
	FRAME_walk14, NULL, 0, 0, 0, seraph_ai_walk, 4, NULL,
	FRAME_walk15, NULL, 0, 0, 0, seraph_ai_walk, 5, NULL,
	FRAME_walk16, NULL, 0, 0, 0, seraph_ai_walk, 6, NULL,
};
animmove_t seraph_move_walk2 = {16,seraph_frames_walk2, seraph_pause};

/*----------------------------------------------------
	seraph whipping

	whip1		- full whip, beginning to end
	whip1_loop	- loopable whipping
	whip1_end	- end transition
----------------------------------------------------*/

animframe_t seraph_frames_whip1[] =
{
	FRAME_whip1,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_whip2,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_whip3,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_whip4,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_whip5,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_whip6,   seraph_strike, SERAPH_DMG_WHIP, 0, 0, NULL, 0, NULL,
	FRAME_whip7,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_whip8,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_whip9,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_whip10,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_whip12,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_whip13,  seraph_strike, SERAPH_DMG_WHIP, 0, 0, NULL, 0, NULL,
	FRAME_whip14,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_whip15,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_whip16,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_whip17,  NULL, 0, 0, 0, NULL, 0, NULL,
};
animmove_t seraph_move_whip1 = {16, seraph_frames_whip1, seraph_pause};

animframe_t seraph_frames_whip1_loop[] =
{
	FRAME_whip7,   NULL, 0, 0, 0, ai_charge, 0, NULL,
	FRAME_whip8,   NULL, 0, 0, 0, ai_charge, 0, NULL,
	FRAME_whip9,   NULL, 0, 0, 0, ai_charge, 0, NULL,
	FRAME_whip10,  NULL, 0, 0, 0, ai_charge, 0, NULL,
	FRAME_whip12,  NULL, 0, 0, 0, ai_charge, 0, NULL,
	FRAME_whip13,  seraph_strike, SERAPH_DMG_WHIP, 0, 0, ai_charge, 0, NULL,
};
animmove_t seraph_move_whip1_loop = {6, seraph_frames_whip1_loop, seraph_pause};

animframe_t seraph_frames_whip1_end[] =
{
	FRAME_whip14,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_whip15,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_whip16,  NULL, 0, 0, 0, NULL, 0, NULL,
};
animmove_t seraph_move_whip1_end = {3, seraph_frames_whip1_end, seraph_pause};

/*----------------------------------------------------
	seraph pointing and yelling
----------------------------------------------------*/

animframe_t seraph_frames_point1[] =
{
	FRAME_point1,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_point2,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_point3,   NULL, 0, 0, 0, NULL, 0, seraph_sound_scold,//no talking
	FRAME_point4,   NULL, 0, 0, 0, NULL, 0, seraph_enforce,
	FRAME_point5,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_point6,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_point7,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_point8,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_point9,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_point10,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_point11,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_point12,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_point13,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_point14,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_point15,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_point16,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_point17,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_point18,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_point19,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_point20,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_point21,   NULL, 0, 0, 0, NULL, 0, NULL,
};
animmove_t seraph_move_point1 = {21,seraph_frames_point1, seraph_idle};

/*----------------------------------------------------
	seraph running

	the names are self explanitory
----------------------------------------------------*/

animframe_t seraph_frames_run1[] =
{
	FRAME_run1,   NULL, 0, 0, 0, ai_run, 20, NULL,
	FRAME_run2,   NULL, 0, 0, 0, ai_run, 16, NULL,
	FRAME_run3,   NULL, 0, 0, 0, ai_run, 16, seraph_pause,
	FRAME_run4,   NULL, 0, 0, 0, ai_run, 16, NULL,
	FRAME_run5,   NULL, 0, 0, 0, ai_run, 20, NULL,
	FRAME_run6,   NULL, 0, 0, 0, ai_run, 16, seraph_pause,
	FRAME_run7,   NULL, 0, 0, 0, ai_run, 16, NULL,
	FRAME_run8,   NULL, 0, 0, 0, ai_run, 16, NULL,
};
animmove_t seraph_move_run1 = {8,seraph_frames_run1, seraph_pause};

animframe_t seraph_frames_fjump[] =
{
	FRAME_run1,   NULL, 0, 0, 0, NULL, 0, seraphApplyJump,
	FRAME_run2,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_run3,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_run4,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_run5,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_run6,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_run7,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_run8,   NULL, 0, 0, 0, NULL, 0, NULL,
};
animmove_t seraph_move_fjump = {8,seraph_frames_fjump, seraph_pause};

animframe_t seraph_frames_run1_whip[] =
{
	FRAME_run_whip1,   NULL, 0, 0, 0, ai_charge, 20, NULL,
	FRAME_run_whip2,   NULL, 0, 0, 0, ai_charge, 16, NULL,
	FRAME_run_whip3,   NULL, 0, 0, 0, ai_charge, 16, NULL,
	FRAME_run_whip4,   NULL, 0, 0, 0, ai_charge, 16, NULL,
	FRAME_run_whip5,   NULL, 0, 0, 0, ai_charge, 20, NULL,
	FRAME_run_whip6,   NULL, 0, 0, 0, ai_charge, 16, NULL,
	FRAME_run_whip7,   seraph_strike, SERAPH_DMG_WHIP_RUN, 0, 0, ai_charge, 16, NULL,
	FRAME_run_whip8,   NULL, 0, 0, 0, ai_charge, 16, NULL,
};
animmove_t seraph_move_run1_whip = {8,seraph_frames_run1_whip, seraph_pause};

/*----------------------------------------------------
	seraph pain
----------------------------------------------------*/

animframe_t seraph_frames_pain[] =
{
	FRAME_pain1,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_pain2,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_pain3,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_pain4,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_pain5,   NULL, 0, 0, 0, NULL, 0, NULL,
};
animmove_t seraph_move_pain = {5,seraph_frames_pain, seraph_pause};

/*----------------------------------------------------
	seraph swipe
----------------------------------------------------*/

animframe_t seraph_frames_swipe[] =
{
	FRAME_swipe1,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_swipe2,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_swipe3,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_swipe4,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_swipe5,   NULL, 0, 0, 0, NULL, 0, NULL,
};
animmove_t seraph_move_swipe = {4,seraph_frames_swipe, seraph_pause};

/*----------------------------------------------------
	seraph get2work

	yells at the ogle, slaps the ogle, then goes back to standing
----------------------------------------------------*/

animframe_t seraph_frames_get2work[] =
{
	FRAME_get_work1,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_get_work2,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_get_work3,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_get_work4,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_get_work5,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_get_work6,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_get_work7,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_get_work8,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_get_work9,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_get_work10,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_get_work11,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_get_work12,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_get_work13,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_get_work14,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_get_work15,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_get_work16,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_get_work17,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_get_work18,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_get_work19,   NULL, 0, 0, 0, NULL, 0, seraph_sound_yell,//hey!
	FRAME_get_work20,   NULL, 0, 0, 0, NULL, 0, seraph_enforce,
	FRAME_get_work21,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_get_work22,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_get_work23,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_get_work24,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_get_work25,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_get_work26,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_swipe1,		NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_swipe2,		NULL, 0, 0, 0, NULL, 0, seraph_sound_slap,
	FRAME_swipe3,		NULL, 0, 0, 0, NULL, 0, seraph_enforce_ogle,
	FRAME_swipe4,		NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_swipe5,		NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_backup1,		NULL, 0, 0, 0, ai_run, -8, NULL,//can't this use ai_move or ai_walk?
	FRAME_backup2,		NULL, 0, 0, 0, ai_run, -8, NULL,
	FRAME_backup3,		NULL, 0, 0, 0, ai_run, -8, NULL,
	FRAME_backup4,		NULL, 0, 0, 0, ai_run, -7, NULL,
	FRAME_backup5,		NULL, 0, 0, 0, ai_run, -6, NULL,
	FRAME_backup6,		NULL, 0, 0, 0, ai_run, -7, NULL,
	FRAME_backup7,		NULL, 0, 0, 0, ai_run, -8, NULL,
	FRAME_backup8,		NULL, 0, 0, 0, ai_run, -8, NULL,
	FRAME_backup9,		NULL, 0, 0, 0, ai_run, -8, NULL,
	FRAME_backup10,		NULL, 0, 0, 0, ai_run, -8, NULL,
	FRAME_backup11,		NULL, 0, 0, 0, ai_run, -7, NULL,
	FRAME_backup12,		NULL, 0, 0, 0, ai_run, -6, NULL,
	FRAME_backup13,		NULL, 0, 0, 0, ai_run, -7, NULL,
	FRAME_backup14,		NULL, 0, 0, 0, ai_run, -8, NULL,
	FRAME_backup15,		NULL, 0, 0, 0, ai_run, -8, NULL,
	FRAME_backup16,		NULL, 0, 0, 0, ai_run, -8, NULL,
};
animmove_t seraph_move_get2work = {47,seraph_frames_get2work, seraph_done_get2work};

/*----------------------------------------------------
	seraph get2work2

	yells at the ogle, whips the ogle, then goes back to standing
----------------------------------------------------*/

animframe_t seraph_frames_get2work2[] =
{
	FRAME_get_work1,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_get_work2,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_get_work3,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_get_work4,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_get_work5,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_get_work6,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_get_work7,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_get_work8,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_get_work9,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_get_work10,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_get_work11,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_get_work12,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_get_work13,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_get_work14,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_get_work15,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_get_work16,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_get_work17,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_get_work18,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_get_work19,   NULL, 0, 0, 0, NULL, 0, seraph_sound_scold2,
	FRAME_get_work20,   NULL, 0, 0, 0, NULL, 0, seraph_enforce,
	FRAME_get_work21,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_get_work22,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_get_work23,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_get_work24,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_get_work25,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_get_work26,   NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_whip1,		NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_whip2,		NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_whip3,		NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_whip4,		NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_whip5,		NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_whip6,		NULL, 0, 0, 0, NULL, 0, seraph_sound_whip,
	FRAME_whip7,		NULL, 0, 0, 0, NULL, 0, seraph_enforce,
	FRAME_whip8,		NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_whip9,		NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_whip10,		NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_whip12,		NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_whip13,		NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_whip14,		NULL, 0, 0, 0, NULL, 0, seraph_sound_whip,
	FRAME_whip15,		NULL, 0, 0, 0, NULL, 0, seraph_enforce,
	FRAME_whip16,		NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_whip17,		NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_backup1,		NULL, 0, 0, 0, ai_run, -8, NULL,
	FRAME_backup2,		NULL, 0, 0, 0, ai_run, -8, NULL,
	FRAME_backup3,		NULL, 0, 0, 0, ai_run, -8, NULL,
	FRAME_backup4,		NULL, 0, 0, 0, ai_run, -7, NULL,
	FRAME_backup5,		NULL, 0, 0, 0, ai_run, -6, NULL,
	FRAME_backup6,		NULL, 0, 0, 0, ai_run, -7, NULL,
	FRAME_backup7,		NULL, 0, 0, 0, ai_run, -8, NULL,
	FRAME_backup8,		NULL, 0, 0, 0, ai_run, -8, NULL,
	FRAME_backup9,		NULL, 0, 0, 0, ai_run, -8, NULL,
	FRAME_backup10,		NULL, 0, 0, 0, ai_run, -8, NULL,
	FRAME_backup11,		NULL, 0, 0, 0, ai_run, -7, NULL,
	FRAME_backup12,		NULL, 0, 0, 0, ai_run, -6, NULL,
	FRAME_backup13,		NULL, 0, 0, 0, ai_run, -7, NULL,
	FRAME_backup14,		NULL, 0, 0, 0, ai_run, -8, NULL,
	FRAME_backup15,		NULL, 0, 0, 0, ai_run, -8, NULL,
	FRAME_backup16,		NULL, 0, 0, 0, ai_run, -8, NULL,
};
animmove_t seraph_move_get2work2 = {57,seraph_frames_get2work2, seraph_done_get2work};

/*----------------------------------------------------
	seraph startle
----------------------------------------------------*/

animframe_t seraph_frames_startle[] =
{
	FRAME_startle1,	NULL, 0, 0, 0, NULL, 0, seraph_sound_startle,
	FRAME_startle2,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_startle3,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_startle4,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_startle5,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_startle6,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_startle7,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_startle8,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_startle9,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_startle10,NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_startle11,NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_startle12,NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_startle13,NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_startle14,NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_startle15,NULL, 0, 0, 0, NULL, 0, NULL,
};
animmove_t seraph_move_startle = {15,seraph_frames_startle, seraph_done_startle};

/*----------------------------------------------------
	seraph ready2idle

	transition frame various frames to standing with arms folded
----------------------------------------------------*/

animframe_t seraph_frames_ready2idle[] =
{
	FRAME_rdy_idle1,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_rdy_idle2,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_rdy_idle3,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_rdy_idle4,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_rdy_idle5,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_rdy_idle6,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_rdy_idle7,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_rdy_idle8,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_rdy_idle9,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_rdy_idle10,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_rdy_idle11,	NULL, 0, 0, 0, NULL, 0, NULL,
};
animmove_t seraph_move_ready2idle = {11,seraph_frames_ready2idle, seraph2idle};

/*----------------------------------------------------
	seraph backup

	backing up...
----------------------------------------------------*/

animframe_t seraph_frames_backup[] =
{
	FRAME_backup1,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_backup2,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_backup3,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_backup4,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_backup5,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_backup6,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_backup7,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_backup8,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_backup9,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_backup10,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_backup11,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_backup12,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_backup13,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_backup14,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_backup15,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_backup16,	NULL, 0, 0, 0, NULL, 0, NULL,
};
animmove_t seraph_move_backup = {16,seraph_frames_backup, seraph_pause};

/*----------------------------------------------------
	seraph death

	death1	-	dying due to massive ogle overload
----------------------------------------------------*/

animframe_t seraph_frames_death1[] =
{
	FRAME_odeath1,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_odeath2,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_odeath3,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_odeath4,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_odeath5,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_odeath6,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_odeath7,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_odeath8,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_odeath9,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_odeath10,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_odeath11,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_odeath12,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_odeath13,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_odeath14,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_odeath15,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_odeath16,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_odeath17,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_odeath18,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_odeath19,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_odeath20,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_odeath21,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_odeath22,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_odeath23,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_odeath24,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_odeath25,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_odeath26,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_odeath27,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_odeath28,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_odeath29,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_odeath30,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_odeath31,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_odeath32,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_odeath34,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_odeath34,	NULL, 0, 0, 0, NULL, 0, NULL,
};
animmove_t seraph_move_death1 = {34,seraph_frames_death1, seraph_dead};

animframe_t seraph_frames_death2_go[] =
{
	FRAME_deathb1,   NULL, 0, 0, 0, NULL, 0, seraph_check_land,
	FRAME_deathb2,   NULL, 0, 0, 0, NULL, 0, seraph_check_land,
	FRAME_deathb3,   NULL, 0, 0, 0, NULL, 0, seraph_check_land,
	FRAME_deathb4,   NULL, 0, 0, 0, NULL, 0, seraph_death_loop,
};
animmove_t seraph_move_death2_go = {4, seraph_frames_death2_go, NULL};

animframe_t seraph_frames_death2_loop[] =
{
	FRAME_deathb5,   NULL, 0, 0, 0, NULL, 0, seraph_check_land,
};
animmove_t seraph_move_death2_loop = {1, seraph_frames_death2_loop, NULL};

animframe_t seraph_frames_death2_end[] =
{
	FRAME_deathb6,  NULL, 0, 0, 0, NULL, 0, seraph_check_land,
	FRAME_deathb7,  NULL, 0, 0, 0, NULL, 0, seraph_check_land,
	FRAME_deathb8,  NULL, 0, 0, 0, NULL, 0, seraph_check_land,
	FRAME_deathb9,  NULL, 0, 0, 0, NULL, 0, seraph_check_land,
	FRAME_deathb10, NULL, 0, 0, 0, NULL, 0, seraph_check_land,
	FRAME_deathb11, NULL, 0, 0, 0, NULL, 0, seraph_check_land,
	FRAME_deathb12, NULL, 0, 0, 0, NULL, 0, seraph_check_land,
	FRAME_deathb13, NULL, 0, 0, 0, NULL, 0, seraph_check_land,
	FRAME_deathb14, NULL, 0, 0, 0, NULL, 0, seraph_check_land,
	FRAME_deathb15, NULL, 0, 0, 0, NULL, 0, M_EndDeath,
};
animmove_t seraph_move_death2_end = {10, seraph_frames_death2_end, NULL};

animframe_t seraph_frames_backup2[] =
{
	FRAME_backup1, NULL, 0, 0, 0, seraph_back, 24, NULL,
	FRAME_backup2, NULL, 0, 0, 0, seraph_back, 20, NULL,
	FRAME_backup3, NULL, 0, 0, 0, seraph_back, 20, NULL,
	FRAME_backup4, NULL, 0, 0, 0, seraph_back, 20, NULL,
	FRAME_backup5, NULL, 0, 0, 0, seraph_back, 24, NULL,
	FRAME_backup6, NULL, 0, 0, 0, seraph_back, 20, NULL,
	FRAME_backup7, NULL, 0, 0, 0, seraph_back, 20, NULL,
	FRAME_backup8, NULL, 0, 0, 0, seraph_back, 20, NULL,
	FRAME_backup9, NULL, 0, 0, 0, seraph_back, 20, NULL,
	FRAME_backup10, NULL, 0, 0, 0, seraph_back, 20, NULL,
	FRAME_backup11, NULL, 0, 0, 0, seraph_back, 24, NULL,
	FRAME_backup12, NULL, 0, 0, 0, seraph_back, 20, NULL,
	FRAME_backup13, NULL, 0, 0, 0, seraph_back, 20, NULL,
	FRAME_backup14, NULL, 0, 0, 0, seraph_back, 20, NULL,
	FRAME_backup15, NULL, 0, 0, 0, seraph_back, 24, NULL,
	FRAME_backup16, NULL, 0, 0, 0, seraph_back, 20, NULL,
};
animmove_t seraph_move_backup2 = {16, seraph_frames_backup2, seraph_pause};
