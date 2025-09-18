//
// c_corvus4_anim.c
//
// Copyright 1998 Raven Software
//

#include "c_corvus4_anim.h"
#include "c_ai.h"

// Corvus action 1.
static const animframe_t corvus4_frames_c_action1[] =
{
	{ FRAME_ss_kneel1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_kneel2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_kneel3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_kneel4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_kneel5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_kneel6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_kneel7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_kneel8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_kneel9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_kneel10, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_kneel11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_kneel12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus4_move_c_action1 = ANIMMOVE(corvus4_frames_c_action1, ai_c_cycleend);

// Corvus action 2.
static const animframe_t corvus4_frames_c_action2[] =
{
	{ FRAME_ss_youare1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_youare2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_youare3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_youare4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_youare5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_youare6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_youare7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_youare8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_youare9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_youare10, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_youare11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_youare12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_youare13, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_youare14, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_youare15, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_youare16, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_youare17, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_youare18, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_youare19, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_youare20, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_youare21, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_youare22, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_youare23, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_youare24, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_youare25, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_youare26, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_youare27, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_youare28, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_youare29, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_youare30, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_youare31, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_youare32, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_youare33, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_youare34, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_youare35, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_youare36, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_youare37, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_youare38, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_youare39, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_youare40, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus4_move_c_action2 = ANIMMOVE(corvus4_frames_c_action2, ai_c_cycleend);

// Corvus action 3.
static const animframe_t corvus4_frames_c_action3[] =
{
	{ FRAME_ss_arethe1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_arethe2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_arethe3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_arethe4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_arethe5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_arethe6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_arethe7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_arethe8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_arethe9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_arethe10, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_arethe11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_arethe12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_arethe13, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_arethe14, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_arethe15, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_arethe16, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus4_move_c_action3 = ANIMMOVE(corvus4_frames_c_action3, ai_c_cycleend);

// Corvus action 4.
static const animframe_t corvus4_frames_c_action4[] =
{
	{ FRAME_ss_myjourn1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_myjourn2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_myjourn3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_myjourn4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_myjourn5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_myjourn6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_myjourn7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_myjourn8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_myjourn9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_myjourn10, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_myjourn11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_myjourn12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_myjourn13, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_myjourn14, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_myjourn15, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_myjourn16, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_myjourn17, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_myjourn18, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_myjourn19, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_myjourn20, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_myjourn21, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_myjourn22, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_myjourn23, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_myjourn24, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus4_move_c_action4 = ANIMMOVE(corvus4_frames_c_action4, ai_c_cycleend);

// Corvus action 5.
static const animframe_t corvus4_frames_c_action5[] =
{
	{ FRAME_ss_getup1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_getup2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_getup3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_getup4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_getup5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_getup6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_getup7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_getup8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_getup9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_getup10, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_getup11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_getup12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_getup13, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_getup14, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_getup15, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_getup16, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_getup17, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_getup18, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_getup19, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_getup20, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_getup21, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_getup22, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_getup23, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_getup24, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_getup25, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_getup26, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_getup27, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_getup28, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_getup29, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_getup30, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_getup31, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_getup32, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_getup33, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_getup34, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_getup35, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_getup36, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_getup37, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_getup38, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_getup39, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus4_move_c_action5 = ANIMMOVE(corvus4_frames_c_action5, ai_c_cycleend);

// Corvus idle 1.
static const animframe_t corvus4_frames_c_idle1[] =
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
const animmove_t corvus4_move_c_idle1 = ANIMMOVE(corvus4_frames_c_idle1, ai_c_cycleend);

// Corvus idle 2.
static const animframe_t corvus4_frames_c_idle2[] =
{
	{ FRAME_ss_idle1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_idle2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_idle3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_idle4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_idle5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_idle6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_idle7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_idle8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_idle9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_idle10, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_idle11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_idle12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_idle13, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_idle14, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_idle15, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_idle16, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_idle17, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_idle18, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_idle19, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ss_idle20, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus4_move_c_idle2 = ANIMMOVE(corvus4_frames_c_idle2, ai_c_cycleend);

// Corvus - starting his pivot to the left.
static const animframe_t corvus4_frames_c_pivotleftgo[] =
{
	{ FRAME_Lpivot1, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Lpivot2, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus4_move_c_pivotleftgo = ANIMMOVE(corvus4_frames_c_pivotleftgo, ai_c_cycleend);

static const animframe_t corvus4_frames_c_pivotleft[] =
{
	{ FRAME_Lpivot3, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Lpivot4, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus4_move_c_pivotleft = ANIMMOVE(corvus4_frames_c_pivotleft, ai_c_cycleend);

static const animframe_t corvus4_frames_c_pivotleftstop[] =
{
	{ FRAME_Lpivot4, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus4_move_c_pivotleftstop = ANIMMOVE(corvus4_frames_c_pivotleftstop, ai_c_cycleend);

// Corvus - starting his pivot to the right.
static const animframe_t corvus4_frames_c_pivotrightgo[] =
{
	{ FRAME_Lpivot4, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Lpivot3, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus4_move_c_pivotrightgo = ANIMMOVE(corvus4_frames_c_pivotrightgo, ai_c_cycleend);

static const animframe_t corvus4_frames_c_pivotright[] =
{
	{ FRAME_Lpivot2, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Lpivot1, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus4_move_c_pivotright = ANIMMOVE(corvus4_frames_c_pivotright, ai_c_cycleend);

static const animframe_t corvus4_frames_c_pivotrightstop[] =
{
	{ FRAME_Lpivot1, NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus4_move_c_pivotrightstop = ANIMMOVE(corvus4_frames_c_pivotrightstop, ai_c_cycleend);

// Corvus walk 1.
static const animframe_t corvus4_frames_c_walk1[] =
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
const animmove_t corvus4_move_c_walk1 = ANIMMOVE(corvus4_frames_c_walk1, ai_c_cycleend);

// Corvus walk 2.
static const animframe_t corvus4_frames_c_walk2[] =
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
const animmove_t corvus4_move_c_walk2 = ANIMMOVE(corvus4_frames_c_walk2, ai_c_cycleend);

// Corvus - starting his walk anims.
static const animframe_t corvus4_frames_c_walkstart[] =
{
	{ FRAME_gorun2, ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_gorun3, ai_c_move, 4, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus4_move_c_walkstart = ANIMMOVE(corvus4_frames_c_walkstart, ai_c_cycleend);

// Corvus - stopping his walk with right foot in front.
static const animframe_t corvus4_frames_c_walkstop1[] =
{
	{ FRAME_jog1, ai_c_move, 2, 0, 0, NULL, 0, NULL },
	{ FRAME_jog2, ai_c_move, 2, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus4_move_c_walkstop1 = ANIMMOVE(corvus4_frames_c_walkstop1, ai_c_cycleend);

// Corvus - stopping his walk with left foot in front.
static const animframe_t corvus4_frames_c_walkstop2[] =
{
	{ FRAME_jog5, ai_c_move, 2, 0, 0, NULL, 0, NULL },
	{ FRAME_jog6, ai_c_move, 2, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus4_move_c_walkstop2 = ANIMMOVE(corvus4_frames_c_walkstop2, ai_c_cycleend);