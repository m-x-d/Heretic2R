//
// c_corvus6_anim.c
//
// Copyright 1998 Raven Software
//

#include "c_corvus6_anim.h"
#include "c_ai.h"

// Corvus action 1.
static const animframe_t corvus6_frames_c_action1[] =
{
	{ FRAME_propup1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_propup2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_propup3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_propup4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_propup5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_propup6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_propup7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_propup8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_propup9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_propup10, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_propup11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus6_move_c_action1 = ANIMMOVE(corvus6_frames_c_action1, ai_c_cycleend);

// Corvus action 2.
static const animframe_t corvus6_frames_c_action2[] =
{
	{ FRAME_groan1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_groan2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_groan3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_groan4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_groan5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_groan6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_groan7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_groan8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_groan9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_groan10, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_groan11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_groan12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_groan13, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_groan14, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_groan15, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_groan16, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_groan17, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_groan18, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_groan19, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_groan20, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus6_move_c_action2 = ANIMMOVE(corvus6_frames_c_action2, ai_c_cycleend);

// Corvus action 3.
static const animframe_t corvus6_frames_c_action3[] =
{
	{ FRAME_moan1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_moan2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_moan3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_moan4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_moan5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_moan6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_moan7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_moan8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_moan9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_moan10, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_moan11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_moan12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus6_move_c_action3 = ANIMMOVE(corvus6_frames_c_action3, ai_c_cycleend);

// Corvus action 4.
static const animframe_t corvus6_frames_c_action4[] =
{
	{ FRAME_lookup1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lookup2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lookup3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lookup4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lookup5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lookup6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lookup7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lookup8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lookup9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lookup10, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lookup11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lookup12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lookup13, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lookup14, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lookup15, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lookup16, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lookup17, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lookup18, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lookup19, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lookup20, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lookup21, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lookup22, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lookup23, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lookup24, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lookup25, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lookup26, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lookup27, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lookup28, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lookup29, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus6_move_c_action4 = ANIMMOVE(corvus6_frames_c_action4, ai_c_cycleend);

// Corvus action 5.
static const animframe_t corvus6_frames_c_action5[] =
{
	{ FRAME_strong1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_strong2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_strong3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_strong4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_strong5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_strong6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_strong7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_strong8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_strong9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_strong10, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_strong11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus6_move_c_action5 = ANIMMOVE(corvus6_frames_c_action5, ai_c_cycleend);

// Corvus action 6.
static const animframe_t corvus6_frames_c_action6[] =
{
	{ FRAME_notnow1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_notnow2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_notnow3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_notnow4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_notnow5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_notnow6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_notnow7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_notnow8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_notnow9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_notnow10, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_notnow11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_notnow12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_notnow13, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_notnow14, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_notnow15, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_notnow16, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_notnow17, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_notnow18, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_notnow19, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_notnow20, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_notnow21, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_notnow22, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_notnow23, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_notnow24, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_notnow25, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_notnow26, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_notnow27, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_notnow28, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_notnow29, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_notnow30, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_notnow31, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_notnow32, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_notnow33, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_notnow34, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_notnow35, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_notnow36, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_notnow37, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_notnow38, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_notnow39, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_notnow40, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_notnow41, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_notnow42, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_notnow43, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_notnow44, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_notnow45, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_notnow46, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_notnow47, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus6_move_c_action6 = ANIMMOVE(corvus6_frames_c_action6, ai_c_cycleend);

// Corvus action 7.
static const animframe_t corvus6_frames_c_action7[] =
{
	{ FRAME_relax1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_relax2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_relax3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_relax4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_relax5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_relax6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_relax7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_relax8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_relax9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_relax10, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_relax11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_relax12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_relax13, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_relax14, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_relax15, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_relax16, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus6_move_c_action7 = ANIMMOVE(corvus6_frames_c_action7, ai_c_cycleend);

// Corvus action 8.
static const animframe_t corvus6_frames_c_action8[] =
{
	{ FRAME_nofear1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_nofear2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_nofear3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_nofear4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_nofear5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_nofear6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_nofear7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_nofear8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_nofear9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_nofear10, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_nofear11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_nofear12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_nofear13, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_nofear14, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_nofear15, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_nofear16, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_nofear17, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_nofear18, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_nofear19, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_nofear20, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_nofear21, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_nofear22, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_nofear23, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_nofear24, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_nofear25, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_nofear26, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_nofear27, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_nofear28, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_nofear29, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_nofear30, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_nofear31, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_nofear32, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_nofear33, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_nofear34, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_nofear35, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus6_move_c_action8 = ANIMMOVE(corvus6_frames_c_action8, ai_c_cycleend);

// Corvus action 9.
static const animframe_t corvus6_frames_c_action9[] =
{
	{ FRAME_standup1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_standup2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_standup3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_standup4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_standup5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_standup6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_standup7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_standup8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_standup9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_standup10, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_standup11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_standup12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_standup13, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_standup14, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_standup15, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_standup16, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_standup17, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_standup18, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus6_move_c_action9 = ANIMMOVE(corvus6_frames_c_action9, ai_c_cycleend);

// Corvus action 10.
static const animframe_t corvus6_frames_c_action10[] =
{
	{ FRAME_falling1,  NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_falling2,  NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_falling3,  NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_falling4,  NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_falling5,  NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_falling6,  NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_falling7,  NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_falling8,  NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_falling9,  NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_falling10, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_falling11, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_falling12, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_falling13, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_falling14, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_falling15, NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus6_move_c_action10 = ANIMMOVE(corvus6_frames_c_action10, ai_c_cycleend);

// Corvus action 11.
static const animframe_t corvus6_frames_c_action11[] =
{
	{ FRAME_kodown1,  NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kodown2,  NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kodown3,  NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kodown4,  NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kodown5,  NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kodown6,  NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kodown7,  NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kodown8,  NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kodown9,  NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kodown10, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kodown11, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kodown12, NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus6_move_c_action11 = ANIMMOVE(corvus6_frames_c_action11, ai_c_cycleend);

// Corvus idle 1.
static const animframe_t corvus6_frames_c_idle1[] =
{
	{ FRAME_propup1, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_propup1, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_propup1, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus6_move_c_idle1 = ANIMMOVE(corvus6_frames_c_idle1, ai_c_cycleend);

// Corvus idle 2.
static const animframe_t corvus6_frames_c_idle2[] =
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
const animmove_t corvus6_move_c_idle2 = ANIMMOVE(corvus6_frames_c_idle2, ai_c_cycleend);

// Corvus idle 3.
static const animframe_t corvus6_frames_c_idle3[] =
{
	{ FRAME_2breath1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2breath2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2breath3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2breath4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2breath5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2breath6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2breath7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2breath8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2breath9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2breath10, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2breath11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2breath12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2breath13, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2breath14, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2breath15, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2breath16, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2breath17, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2breath18, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2breath19, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2breath20, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2breath21, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus6_move_c_idle3 = ANIMMOVE(corvus6_frames_c_idle3, ai_c_cycleend);

// Corvus idle 4.
static const animframe_t corvus6_frames_c_idle4[] =
{
	{ FRAME_1breathing26, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus6_move_c_idle4 = ANIMMOVE(corvus6_frames_c_idle4, ai_c_cycleend);

// Corvus idle 5.
static const animframe_t corvus6_frames_c_idle5[] =
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
const animmove_t corvus6_move_c_idle5 = ANIMMOVE(corvus6_frames_c_idle5, ai_c_cycleend);