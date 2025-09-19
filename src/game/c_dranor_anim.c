//
// c_dranor_anim.c
//
// Copyright 1998 Raven Software
//

#include "c_dranor_anim.h"
#include "c_ai.h"

// Dranor idle 1.
static const animframe_t dranor_frames_c_idle1[] =
{
	{ FRAME_1idle1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1idle2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1idle3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1idle4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1idle5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1idle6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1idle7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1idle8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1idle9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1idle10, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t dranor_move_c_idle1 = ANIMMOVE(dranor_frames_c_idle1, ai_c_cycleend);

// Dranor idle 2.
static const animframe_t dranor_frames_c_idle2[] =
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
};
const animmove_t dranor_move_c_idle2 = ANIMMOVE(dranor_frames_c_idle2, ai_c_cycleend);

// Dranor idle 3.
static const animframe_t dranor_frames_c_idle3[] =
{
	{ FRAME_2idle1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2idle2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2idle3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2idle4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2idle5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2idle6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2idle7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2idle8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2idle9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2idle10, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2idle11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2idle12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2idle13, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2idle14, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2idle15, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2idle16, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2idle17, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2idle18, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2idle19, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2idle20, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t dranor_move_c_idle3 = ANIMMOVE(dranor_frames_c_idle3, ai_c_cycleend);

// Dranor action 1.
static const animframe_t dranor_frames_c_action1[] =
{
	{ FRAME_go1, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go2, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go3, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go4, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t dranor_move_c_action1 = ANIMMOVE(dranor_frames_c_action1, ai_c_cycleend);

// Dranor action 2.
static const animframe_t dranor_frames_c_action2[] =
{
	{ FRAME_go5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go10, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_go11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go13, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go14, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go15, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go16, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go17, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go18, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go19, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go20, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_go21, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go22, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go23, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go24, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go25, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go26, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go27, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go28, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go29, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go30, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_go31, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go32, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go33, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go34, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go35, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go36, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go37, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go38, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go39, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go40, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_go41, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go42, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go43, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go44, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go45, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go46, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go47, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go48, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go49, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go50, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_go51, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go52, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go53, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go54, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go55, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go56, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go57, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go58, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go59, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go60, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_go61, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go62, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go63, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go64, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go65, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go66, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go67, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go68, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go69, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go70, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_go71, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go72, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go73, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go74, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go75, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go76, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go77, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go78, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go79, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go80, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_go81, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go82, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go83, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go84, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go85, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go86, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go87, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go88, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go89, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go90, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_go91, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go92, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go93, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go94, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go95, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go96, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go97, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go98, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go99, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go100, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_go101, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go102, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go103, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go104, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go105, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go106, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go107, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go108, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go109, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go110, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_go111, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go112, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go113, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_go114, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t dranor_move_c_action2 = ANIMMOVE(dranor_frames_c_action2, ai_c_cycleend);

// Dranor action 3.
static const animframe_t dranor_frames_c_action3[] =
{
	{ FRAME_slayer1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slayer2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slayer3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slayer4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slayer5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slayer6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slayer7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slayer8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slayer9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slayer10, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_slayer11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slayer12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slayer13, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slayer14, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slayer15, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slayer16, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slayer17, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slayer18, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slayer19, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slayer20, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_slayer21, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slayer22, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slayer23, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slayer24, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slayer25, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slayer26, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slayer27, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slayer28, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slayer29, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slayer30, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_slayer31, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slayer32, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slayer33, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slayer34, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slayer35, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slayer36, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slayer37, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slayer38, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slayer39, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slayer40, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_slayer41, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slayer42, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slayer43, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slayer44, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slayer45, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slayer46, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slayer47, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slayer48, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slayer49, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slayer50, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_slayer51, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slayer52, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t dranor_move_c_action3 = ANIMMOVE(dranor_frames_c_action3, ai_c_cycleend);

// Dranor action 4.
static const animframe_t dranor_frames_c_action4[] =
{
	{ FRAME_1iwas1, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1iwas2, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1iwas3, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1iwas4, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1iwas5, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1iwas6, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1iwas7, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1iwas8, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t dranor_move_c_action4 = ANIMMOVE(dranor_frames_c_action4, ai_c_cycleend);

// Dranor action 5.
static const animframe_t dranor_frames_c_action5[] =
{
	{ FRAME_2iwas1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas10, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_2iwas11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas13, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas14, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas15, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas16, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas17, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas18, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas19, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas20, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_2iwas21, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas22, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas23, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas24, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas25, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas26, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas27, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas28, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas29, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas30, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_2iwas31, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas32, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas33, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas34, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas35, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas36, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas37, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas38, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas39, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas40, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_2iwas41, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas42, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas43, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas44, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas45, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas46, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas47, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas48, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas49, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas50, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_2iwas51, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas52, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas53, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas54, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas55, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas56, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas57, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas58, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas59, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas60, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_2iwas61, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas62, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas63, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas64, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas65, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas66, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas67, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas68, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas69, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas70, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_2iwas71, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas72, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas73, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas74, ai_c_move, 0, 0, 0, NULL, 0, NULL },

};
const animmove_t dranor_move_c_action5 = ANIMMOVE(dranor_frames_c_action5, ai_c_cycleend);

// Dranor action 6.
static const animframe_t dranor_frames_c_action6[] =
{
	{ FRAME_1iwas8, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1iwas7, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1iwas6, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1iwas5, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1iwas4, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1iwas3, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1iwas2, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1iwas1, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t dranor_move_c_action6 = ANIMMOVE(dranor_frames_c_action6, ai_c_cycleend);

// Dranor action 7.
static const animframe_t dranor_frames_c_action7[] =
{
	{ FRAME_2iwas74, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas73, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas72, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas71, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas70, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_2iwas69, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas68, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas67, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas66, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas65, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas64, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas63, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas62, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas61, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas60, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_2iwas59, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas58, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas57, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas56, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas55, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas54, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas53, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas52, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas51, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas50, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_2iwas49, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas48, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas47, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas46, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas45, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas44, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas43, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas42, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas41, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas40, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_2iwas39, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas38, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas37, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas36, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas35, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas34, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas33, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas32, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas31, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas30, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_2iwas29, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas28, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas27, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas26, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas25, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas24, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas23, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas22, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas21, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas20, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_2iwas19, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas18, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas17, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas16, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas15, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas14, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas13, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas10, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_2iwas9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2iwas1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_1iwas8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1iwas7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1iwas6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1iwas5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1iwas4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1iwas3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1iwas2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1iwas1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t dranor_move_c_action7 = ANIMMOVE(dranor_frames_c_action7, ai_c_cycleend); //mxd. numframes:74 in original logic.

// Dranor action 8.
static const animframe_t dranor_frames_c_action8[] =
{
	{ FRAME_1butyou1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou10, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_1butyou11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou13, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou14, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou15, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou16, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou17, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou18, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou19, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou20, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_1butyou21, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou22, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou23, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou24, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou25, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou26, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou27, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou28, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou29, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou30, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_1butyou31, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou32, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou33, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou34, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou35, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou36, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou37, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou38, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou39, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou40, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_1butyou41, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou42, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou43, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou44, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou45, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou46, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou47, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou48, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou49, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou50, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_1butyou51, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou52, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou53, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou54, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou55, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou56, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou57, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou58, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou59, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou60, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_1butyou61, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou62, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou63, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou64, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou65, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou66, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou67, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou68, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou69, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1butyou70, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_1butyou71, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t dranor_move_c_action8 = ANIMMOVE(dranor_frames_c_action8, ai_c_cycleend);

// Dranor action 9.
static const animframe_t dranor_frames_c_action9[] =
{
	{ FRAME_2butyou1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2butyou2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2butyou3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2butyou4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2butyou5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2butyou6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2butyou7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2butyou8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2butyou9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2butyou10, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_2butyou11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2butyou12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2butyou13, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2butyou14, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2butyou15, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2butyou16, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2butyou17, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2butyou18, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2butyou19, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2butyou20, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_2butyou21, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2butyou22, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2butyou23, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2butyou24, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2butyou25, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2butyou26, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2butyou27, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2butyou28, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2butyou29, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2butyou30, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_2butyou31, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2butyou32, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2butyou33, ai_c_move, 0, 0, 0, NULL, 0, NULL },

};
const animmove_t dranor_move_c_action9 = ANIMMOVE(dranor_frames_c_action9, ai_c_cycleend);

// Dranor action 10.
static const animframe_t dranor_frames_c_action10[] =
{
	{ FRAME_2insert1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2insert2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2insert3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2insert4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2insert5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2insert6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2insert7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2insert8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2insert9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2insert10, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_2insert11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2insert12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2insert13, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2insert14, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2insert15, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2insert16, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2insert17, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2insert18, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2insert19, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2insert20, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_2insert21, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2insert22, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2insert23, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2insert24, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2insert25, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2insert26, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2insert27, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2insert28, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2insert29, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2insert30, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_2insert31, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2insert32, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_2insert33, ai_c_move, 0, 0, 0, NULL, 0, NULL },

};
const animmove_t dranor_move_c_action10 = ANIMMOVE(dranor_frames_c_action10, ai_c_cycleend);

// Dranor action 11.
static const animframe_t dranor_frames_c_action11[] =
{
	{ FRAME_1insert1, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1insert2, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1insert3, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1insert4, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1insert5, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1insert6, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1insert7, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_1insert8, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t dranor_move_c_action11 = ANIMMOVE(dranor_frames_c_action11, ai_c_cycleend);

// Dranor action 12.
static const animframe_t dranor_frames_c_action12[] =
{
	{ FRAME_2insert33, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t dranor_move_c_action12 = ANIMMOVE(dranor_frames_c_action12, ai_c_cycleend);

// Dranor death 1.
static const animframe_t dranor_frames_c_death1[] =
{
	{ FRAME_death1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death10, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death13, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death14, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death15, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death16, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death17, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death18, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death19, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death20, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t dranor_move_c_death1 = ANIMMOVE(dranor_frames_c_death1, ai_c_cycleend);