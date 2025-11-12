//
// c_corvus5_anim.c
//
// Copyright 1998 Raven Software
//

#include "c_corvus5_anim.h"
#include "c_ai.h"
#include "g_local.h"

// Corvus action 1. 
static const animframe_t corvus5_frames_c_action1[] =
{
	{ FRAME_youknow1, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_youknow2, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_youknow3, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_youknow4, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_youknow5, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_youknow6, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_youknow7, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_youknow8, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus5_move_c_action1 = ANIMMOVE(corvus5_frames_c_action1, ai_c_cycleend);

// Corvus action 2.
static const animframe_t corvus5_frames_c_action2[] =
{
	{ FRAME_who1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_who2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_who3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_who4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_who5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_who6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_who7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_who8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_who9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_who10, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_who11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_who12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_who13, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_who14, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_who15, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_who16, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_who17, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_who18, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_who19, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_who20, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_who21, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_who22, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_who23, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_who24, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_who25, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_who26, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_who27, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_who28, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_who29, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_who30, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_who31, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_who32, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_who33, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_who34, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_who35, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_who36, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_who37, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_who38, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_who39, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_who40, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_who41, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_who42, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_who43, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_who44, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_who45, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_who46, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_who47, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus5_move_c_action2 = ANIMMOVE(corvus5_frames_c_action2, ai_c_cycleend);

// Corvus action 3.
static const animframe_t corvus5_frames_c_action3[] =
{
	{ FRAME_spared1, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_spared2, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_spared3, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_spared4, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_spared5, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_spared6, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_spared7, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus5_move_c_action3 = ANIMMOVE(corvus5_frames_c_action3, ai_c_cycleend);

// Corvus action 4.
static const animframe_t corvus5_frames_c_action4[] =
{
	{ FRAME_itsnot1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_itsnot2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_itsnot3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_itsnot4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_itsnot5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_itsnot6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_itsnot7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_itsnot8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_itsnot9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_itsnot10, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_itsnot11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_itsnot12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_itsnot13, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_itsnot14, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_itsnot15, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_itsnot16, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_itsnot17, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_itsnot18, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_itsnot19, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_itsnot20, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_itsnot21, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_itsnot22, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_itsnot23, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus5_move_c_action4 = ANIMMOVE(corvus5_frames_c_action4, ai_c_cycleend);

// Corvus action 5.
static const animframe_t corvus5_frames_c_action5[] =
{
	{ FRAME_iwill1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_iwill2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_iwill3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_iwill4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_iwill5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_iwill6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_iwill7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_iwill8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_iwill9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_iwill10, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_iwill11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_iwill12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_iwill13, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_iwill14, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_iwill15, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_iwill16, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_iwill17, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_iwill18, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_iwill19, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_iwill20, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_iwill21, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_iwill22, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_iwill23, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_iwill24, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_iwill25, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_iwill26, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_iwill27, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_iwill28, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_iwill29, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_iwill30, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_iwill31, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_iwill32, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_iwill33, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus5_move_c_action5 = ANIMMOVE(corvus5_frames_c_action5, ai_c_cycleend);

// Corvus action 6.
static const animframe_t corvus5_frames_c_action6[] =
{
	{ FRAME_ready1, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ready2, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ready3, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus5_move_c_action6 = ANIMMOVE(corvus5_frames_c_action6, ai_c_cycleend);

// Corvus action 7
static const animframe_t corvus5_frames_c_action7[] =
{
	{ FRAME_ready4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ready5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ready6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ready7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ready8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ready9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ready10, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ready11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ready12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ready13, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ready14, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ready15, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ready16, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ready17, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ready18, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ready19, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ready20, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ready21, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ready22, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ready23, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ready24, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ready25, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ready26, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus5_move_c_action7 = ANIMMOVE(corvus5_frames_c_action7, ai_c_cycleend);

// Corvus action 8.
static const animframe_t corvus5_frames_c_action8[] =
{
	{ FRAME_ready3, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ready2, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ready1, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus5_move_c_action8 = ANIMMOVE(corvus5_frames_c_action8, ai_c_cycleend);

// Corvus idle 1.
static const animframe_t corvus5_frames_c_idle1[] =
{
	{ FRAME_Breath1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Breath2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Breath3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Breath4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Breath5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Breath6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Breath7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Breath8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Breath9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Breath10, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Breath11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Breath12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Breath13, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Breath14, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Breath15, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Breath16, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Breath17, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Breath18, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Breath19, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Breath20, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Breath21, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Breath22, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Breath23, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus5_move_c_idle1 = ANIMMOVE(corvus5_frames_c_idle1, ai_c_cycleend);

// Corvus idle 2.
static const animframe_t corvus5_frames_c_idle2[] =
{
	{ FRAME_who1, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus5_move_c_idle2 = ANIMMOVE(corvus5_frames_c_idle2, ai_c_cycleend);

// Corvus idle 3.
static const animframe_t corvus5_frames_c_idle3[] =
{
	{ FRAME_stance1,  ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_stance2,  ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_stance3,  ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_stance4,  ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_stance5,  ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_stance6,  ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_stance7,  ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_stance8,  ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_stance9,  ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_stance10, ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_stance11, ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_stance12, ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_stance13, ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_stance14, ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_stance15, ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_stance16, ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_stance17, ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_stance18, ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_stance19, ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_stance20, ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_stance21, ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_stance22, ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_stance23, ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_stance24, ai_c_move, 4, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus5_move_c_idle3 = ANIMMOVE(corvus5_frames_c_idle3, ai_c_cycleend);

// Corvus - starting his pivot to the left.
static const animframe_t corvus5_frames_c_pivotleftgo[] =
{
	{ FRAME_Lpivot1, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Lpivot2, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus5_move_c_pivotleftgo = ANIMMOVE(corvus5_frames_c_pivotleftgo, ai_c_cycleend);

static const animframe_t corvus5_frames_c_pivotleft[] =
{
	{ FRAME_Lpivot3, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Lpivot4, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus5_move_c_pivotleft = ANIMMOVE(corvus5_frames_c_pivotleft, ai_c_cycleend);

static const animframe_t corvus5_frames_c_pivotleftstop[] =
{
	{ FRAME_Lpivot4, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus5_move_c_pivotleftstop = ANIMMOVE(corvus5_frames_c_pivotleftstop, ai_c_cycleend);

// Corvus - starting his pivot to the right.
static const animframe_t corvus5_frames_c_pivotrightgo[] =
{
	{ FRAME_Lpivot4, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Lpivot3, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus5_move_c_pivotrightgo = ANIMMOVE(corvus5_frames_c_pivotrightgo, ai_c_cycleend);

static const animframe_t corvus5_frames_c_pivotright[] =
{
	{ FRAME_Lpivot2, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Lpivot1, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus5_move_c_pivotright = ANIMMOVE(corvus5_frames_c_pivotright, ai_c_cycleend);

static const animframe_t corvus5_frames_c_pivotrightstop[] =
{
	{ FRAME_Lpivot1, NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus5_move_c_pivotrightstop = ANIMMOVE(corvus5_frames_c_pivotrightstop, ai_c_cycleend);

// Corvus walk 1.
static const animframe_t corvus5_frames_c_walk1[] =
{
	{ FRAME_jog1, ai_c_move, 10, 0, 0, NULL, 0, NULL },
	{ FRAME_jog2, ai_c_move, 16, 0, 0, NULL, 0, NULL },
	{ FRAME_jog3, ai_c_move, 15, 0, 0, NULL, 0, NULL },
	{ FRAME_jog4, ai_c_move, 16, 0, 0, NULL, 0, NULL },
	{ FRAME_jog5, ai_c_move, 17, 0, 0, NULL, 0, NULL },
	{ FRAME_jog6, ai_c_move, 16, 0, 0, NULL, 0, NULL },
	{ FRAME_jog7, ai_c_move, 15, 0, 0, NULL, 0, NULL },
	{ FRAME_jog8, ai_c_move, 16, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus5_move_c_walk1 = ANIMMOVE(corvus5_frames_c_walk1, ai_c_cycleend);

// Corvus walk 2.
static const animframe_t corvus5_frames_c_walk2[] =
{
	{ FRAME_cinewalk1,  ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_cinewalk2,  ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_cinewalk3,  ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_cinewalk4,  ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_cinewalk5,  ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_cinewalk6,  ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_cinewalk7,  ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_cinewalk8,  ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_cinewalk9,  ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_cinewalk10, ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_cinewalk11, ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_cinewalk12, ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_cinewalk13, ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_cinewalk14, ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_cinewalk15, ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_cinewalk16, ai_c_move, 4, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus5_move_c_walk2 = ANIMMOVE(corvus5_frames_c_walk2, ai_c_cycleend);

// Corvus - starting his walk anims.
static const animframe_t corvus5_frames_c_walkstart[] =
{
	{ FRAME_gorun2, ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_gorun3, ai_c_move, 4, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus5_move_c_walkstart = ANIMMOVE(corvus5_frames_c_walkstart, ai_c_cycleend);

// Corvus - stopping his walk with right foot in front.
static const animframe_t corvus5_frames_c_walkstop1[] =
{
	{ FRAME_jog1, ai_c_move, 2, 0, 0, NULL, 0, NULL },
	{ FRAME_jog2, ai_c_move, 2, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus5_move_c_walkstop1 = ANIMMOVE(corvus5_frames_c_walkstop1, ai_c_cycleend);

// Corvus - stopping his walk with left foot in front.
static const animframe_t corvus5_frames_c_walkstop2[] =
{
	{ FRAME_jog5, ai_c_move, 2, 0, 0, NULL, 0, NULL },
	{ FRAME_jog6, ai_c_move, 2, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus5_move_c_walkstop2 = ANIMMOVE(corvus5_frames_c_walkstop2, ai_c_cycleend);