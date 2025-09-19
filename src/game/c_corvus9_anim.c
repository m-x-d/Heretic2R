//
// c_corvus9_anim.c
//
// Copyright 1998 Raven Software
//

#include "c_corvus9_anim.h"
#include "c_ai.h"

// Corvus action 1.
static const animframe_t corvus9_frames_c_action1[] =
{
	{ FRAME_cstartled1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cstartled2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cstartled3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cstartled4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cstartled5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cstartled6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cstartled7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cstartled8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cstartled9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cstartled10, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cstartled11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cstartled12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cstartled13, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cstartled14, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cstartled15, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cstartled16, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cstartled17, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cstartled18, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cstartled19, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cstartled20, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cstartled21, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cstartled22, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cstartled23, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cstartled24, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cstartled25, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cstartled26, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus9_move_c_action1 = ANIMMOVE(corvus9_frames_c_action1, ai_c_cycleend);

// Corvus action 2.
static const animframe_t corvus9_frames_c_action2[] =
{
	{ FRAME_ctranslate1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctranslate2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctranslate3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctranslate4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctranslate5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctranslate6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctranslate7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctranslate8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctranslate9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctranslate10, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctranslate11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctranslate12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctranslate13, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctranslate14, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctranslate15, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctranslate16, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctranslate17, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctranslate18, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctranslate19, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus9_move_c_action2 = ANIMMOVE(corvus9_frames_c_action2, ai_c_cycleend);

// Corvus action 3.
static const animframe_t corvus9_frames_c_action3[] =
{
	{ FRAME_ctell1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell10, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell13, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell14, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell15, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell16, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell17, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell18, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell19, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell20, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell21, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell22, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell23, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell24, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell25, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell26, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell27, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell28, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell29, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell30, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell31, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell32, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell33, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell34, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell35, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell36, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell37, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell38, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell39, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell40, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell41, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell42, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell43, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell44, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell45, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell46, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell47, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell48, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell49, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell50, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell51, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell52, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell53, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell54, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell55, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell56, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell57, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell58, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell59, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell60, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell61, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell62, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ctell63, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus9_move_c_action3 = ANIMMOVE(corvus9_frames_c_action3, ai_c_cycleend);

// Corvus action 4.
static const animframe_t corvus9_frames_c_action4[] =
{
	{ FRAME_cbecause1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause10, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause13, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause14, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause15, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause16, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause17, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause18, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause19, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause20, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause21, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause22, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause23, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause24, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause25, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause26, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause27, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause28, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause29, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause30, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause31, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause32, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause33, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause34, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause35, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause36, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause37, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause38, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause39, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause40, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause41, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause42, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause43, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause44, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause45, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause46, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause47, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause48, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause49, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause50, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause51, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause52, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause53, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause54, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause55, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause56, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause57, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause58, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause59, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause60, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause61, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause62, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause63, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause64, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause65, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause66, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause67, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause68, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause69, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause70, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause71, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause72, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause73, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause74, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause75, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause76, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cbecause77, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus9_move_c_action4 = ANIMMOVE(corvus9_frames_c_action4, ai_c_cycleend);

// Corvus action 5.
static const animframe_t corvus9_frames_c_action5[] =
{
	{ FRAME_cchallenge1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cchallenge2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cchallenge3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cchallenge4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cchallenge5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cchallenge6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cchallenge7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cchallenge8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cchallenge9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cchallenge10, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cchallenge11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cchallenge12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cchallenge13, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus9_move_c_action5 = ANIMMOVE(corvus9_frames_c_action5, ai_c_cycleend);

// Corvus action 6.
static const animframe_t corvus9_frames_c_action6[] =
{
	{ FRAME_cfall1, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cfall2, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cfall3, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cfall4, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cfall5, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus9_move_c_action6 = ANIMMOVE(corvus9_frames_c_action6, ai_c_cycleend);

// Corvus action 7.
static const animframe_t corvus9_frames_c_action7[] =
{
	{ FRAME_kodown1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kodown2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kodown3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kodown4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kodown5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kodown6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kodown7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kodown8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kodown9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kodown10, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kodown11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kodown12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus9_move_c_action7 = ANIMMOVE(corvus9_frames_c_action7, ai_c_cycleend);

// Corvus action 8.
static const animframe_t corvus9_frames_c_action8[] =
{
	{ FRAME_ready1, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ready2, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ready3, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus9_move_c_action8 = ANIMMOVE(corvus9_frames_c_action8, ai_c_cycleend);

// Corvus action 9.
static const animframe_t corvus9_frames_c_action9[] =
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
const animmove_t corvus9_move_c_action9 = ANIMMOVE(corvus9_frames_c_action9, ai_c_cycleend);

// Corvus action 10.
static const animframe_t corvus9_frames_c_action10[] =
{
	{ FRAME_ready3, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ready2, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ready1, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus9_move_c_action10 = ANIMMOVE(corvus9_frames_c_action10, ai_c_cycleend);

// Corvus action 11.
static const animframe_t corvus9_frames_c_action11[] =
{
	{ FRAME_rolla1,  ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_rolla2,  ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_rolla3,  ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_rolla4,  ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_rolla5,  ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_rolla6,  ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_rolla7,  ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_rolla8,  ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_rolla9,  ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_rolla10, ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_rolla11, ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_rolla12, ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_rolla13, ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_rolla14, ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_rolla15, ai_c_move, 4, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus9_move_c_action11 = ANIMMOVE(corvus9_frames_c_action11, ai_c_cycleend);

// Corvus idle 1.
static const animframe_t corvus9_frames_c_idle1[] =
{
	{ FRAME_breath1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_breath2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_breath3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_breath4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_breath5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_breath6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_breath7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_breath8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_breath9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_breath10, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_breath11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_breath12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_breath13, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_breath14, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_breath15, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_breath16, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_breath17, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_breath18, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_breath19, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_breath20, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_breath21, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_breath22, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_breath23, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus9_move_c_idle1 = ANIMMOVE(corvus9_frames_c_idle1, ai_c_cycleend);

// Corvus idle 2.
static const animframe_t corvus9_frames_c_idle2[] =
{
	{ FRAME_1breathing1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1breathing2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1breathing3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1breathing4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1breathing5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1breathing6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1breathing7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1breathing8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1breathing9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1breathing10, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1breathing11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1breathing12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1breathing13, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1breathing14, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1breathing15, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1breathing16, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1breathing17, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1breathing18, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1breathing19, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1breathing20, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1breathing21, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1breathing22, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1breathing23, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1breathing24, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1breathing25, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1breathing26, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus9_move_c_idle2 = ANIMMOVE(corvus9_frames_c_idle2, ai_c_cycleend);

// Corvus idle 3.
static const animframe_t corvus9_frames_c_idle3[] =
{
	{ FRAME_cfinalidle, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_cfinalidle, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus9_move_c_idle3 = ANIMMOVE(corvus9_frames_c_idle3, ai_c_cycleend);

// Corvus walk 1.
static const animframe_t corvus9_frames_c_walk1[] =
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
const animmove_t corvus9_move_c_walk1 = ANIMMOVE(corvus9_frames_c_walk1, ai_c_cycleend);

// Corvus walk 2.
static const animframe_t corvus9_frames_c_walk2[] =
{
	{ FRAME_cinewalk16, ai_c_move, -4, 0, 0, NULL, 0, NULL },
	{ FRAME_cinewalk15, ai_c_move, -4, 0, 0, NULL, 0, NULL },
	{ FRAME_cinewalk14, ai_c_move, -4, 0, 0, NULL, 0, NULL },
	{ FRAME_cinewalk13, ai_c_move, -4, 0, 0, NULL, 0, NULL },
	{ FRAME_cinewalk12, ai_c_move, -4, 0, 0, NULL, 0, NULL },
	{ FRAME_cinewalk11, ai_c_move, -4, 0, 0, NULL, 0, NULL },
	{ FRAME_cinewalk10, ai_c_move, -4, 0, 0, NULL, 0, NULL },
	{ FRAME_cinewalk9,  ai_c_move, -4, 0, 0, NULL, 0, NULL },
	{ FRAME_cinewalk8,  ai_c_move, -4, 0, 0, NULL, 0, NULL },
	{ FRAME_cinewalk7,  ai_c_move, -4, 0, 0, NULL, 0, NULL },
	{ FRAME_cinewalk6,  ai_c_move, -4, 0, 0, NULL, 0, NULL },
	{ FRAME_cinewalk5,  ai_c_move, -4, 0, 0, NULL, 0, NULL },
	{ FRAME_cinewalk4,  ai_c_move, -4, 0, 0, NULL, 0, NULL },
	{ FRAME_cinewalk3,  ai_c_move, -4, 0, 0, NULL, 0, NULL },
	{ FRAME_cinewalk2,  ai_c_move, -4, 0, 0, NULL, 0, NULL },
	{ FRAME_cinewalk1,  ai_c_move, -4, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus9_move_c_walk2 = ANIMMOVE(corvus9_frames_c_walk2, ai_c_cycleend);