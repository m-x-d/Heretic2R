//
// c_victimSsithra_anim.c
//
// Copyright 1998 Raven Software
//

#include "c_victimssithra_anim.h"
#include "c_ai.h"

// Ssithra victim action 1.
static animframe_t ssithra_frames_c_action1[] =
{
	{ FRAME_rackpain1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_rackpain2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_rackpain3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_rackpain4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_rackpain5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_rackpain6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_rackpain7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_rackpain8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_rackpain9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_rackpain10, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_rackpain11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_rackpain12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_rackpain13, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_rackpain14, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_rackpain15, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_rackpain16, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_rackpain17, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_rackpain18, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_rackpain19, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_rackpain20, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
animmove_t victimSsithra_move_c_action1 = { 20, ssithra_frames_c_action1, ai_c_cycleend };

// Ssithra victim action 2.
static animframe_t ssithra_frames_c_action2[] =
{
	{ FRAME_release1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_release2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_release3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_release4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_release5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_release6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_release7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_release8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_release9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_release10, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_release11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_release12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_release13, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_release14, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_release15, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_release16, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_release17, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_release18, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_release19, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_release20, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_release21, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_release22, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_release23, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
animmove_t victimSsithra_move_c_action2 = { 23, ssithra_frames_c_action2, ai_c_cycleend };

// Ssithra victim action 3.
static animframe_t ssithra_frames_c_action3[] =
{
	{ FRAME_recover1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_recover2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_recover3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_recover4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_recover5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_recover6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_recover7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_recover8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_recover9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_recover10, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_recover11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_recover12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_recover13, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_recover14, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_recover15, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_recover16, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_recover17, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_recover18, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_recover19, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_recover20, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_recover21, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_recover22, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_recover23, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_recover24, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_recover25, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_recover26, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_recover27, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_recover28, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_recover29, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_recover30, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_recover31, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_recover32, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_recover33, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
animmove_t victimSsithra_move_c_action3 = { 33, ssithra_frames_c_action3, ai_c_cycleend };

// Ssithra victim action 4.
static animframe_t ssithra_frames_c_action4[] =
{
	{ FRAME_tv_toolate1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate10, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_tv_toolate11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate13, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate14, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate15, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate16, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate17, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate18, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate19, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate20, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_tv_toolate21, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate22, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate23, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate24, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate25, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate26, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate27, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate28, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate29, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate30, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_tv_toolate31, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate32, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate33, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate34, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate35, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate36, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate37, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate38, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate39, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate40, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_tv_toolate41, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate42, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate43, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate44, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate45, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate46, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate47, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate48, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate49, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate50, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_tv_toolate51, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate52, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate53, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate54, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate55, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate56, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate57, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate58, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate59, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate60, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_tv_toolate61, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate62, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate63, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate64, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate65, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate66, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate67, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate68, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate69, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate70, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_tv_toolate71, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate72, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate73, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate74, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate75, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate76, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate77, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate78, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate79, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate80, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_tv_toolate81, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate82, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate83, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate84, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate85, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate86, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate87, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate88, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate89, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate90, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_tv_toolate91, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate92, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate93, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate94, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate95, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate96, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate97, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate98, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate99, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate100, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_tv_toolate101, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate102, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate103, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate104, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate105, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate106, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate107, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate108, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate109, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate110, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_tv_toolate111, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate112, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate113, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate114, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate115, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate116, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate117, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate118, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate119, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate120, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_tv_toolate121, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate122, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate123, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate124, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate125, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate126, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate127, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate128, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate129, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate130, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_tv_toolate131, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate132, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate133, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate134, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate135, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate136, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate137, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate138, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate139, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate140, ai_c_move, 0, 0, 0, NULL, 0, NULL },

	{ FRAME_tv_toolate141, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_toolate142, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
animmove_t victimSsithra_move_c_action4 = { 142, ssithra_frames_c_action4, ai_c_cycleend };

// Ssithra victim action 5.
static animframe_t ssithra_frames_c_action5[110] =
{
	{ FRAME_tv_itwillA1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA10, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA13, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA14, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA15, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA16, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA17, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA18, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA19, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA20, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA21, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA22, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA23, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA24, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA25, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA26, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA27, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA28, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA29, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA30, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA31, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA32, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA33, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA34, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA35, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA36, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA37, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA38, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA39, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA40, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA41, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA42, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA43, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA44, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA45, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA46, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA47, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA48, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA49, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA50, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA51, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA52, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA53, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA54, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA55, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA56, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA57, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA58, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA59, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA60, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA61, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA62, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA63, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA64, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA65, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA66, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA67, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA68, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA69, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA70, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA71, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA72, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA73, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA74, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA75, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA76, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA77, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA78, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA79, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA80, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA81, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA82, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA83, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA84, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA85, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA86, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA87, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA88, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA89, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA90, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA91, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA92, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA93, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA94, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA95, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA96, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA97, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA98, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA99, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA100, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA101, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA102, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA103, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA104, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA105, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA106, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA107, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA108, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA109, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillA110, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
animmove_t victimSsithra_move_c_action5 = { 110, ssithra_frames_c_action5, ai_c_cycleend };

// Ssithra victim action 6.
static animframe_t ssithra_frames_c_action6[129] =
{
	{ FRAME_tv_itwillB1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB10, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB13, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB14, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB15, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB16, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB17, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB18, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB19, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB20, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB21, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB22, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB23, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB24, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB25, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB26, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB27, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB28, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB29, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB30, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB31, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB32, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB33, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB34, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB35, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB36, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB37, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB38, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB39, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB40, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB41, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB42, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB43, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB44, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB45, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB46, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB47, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB48, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB49, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB50, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB51, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB52, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB53, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB54, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB55, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB56, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB57, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB58, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB59, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB60, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB61, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB62, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB63, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB64, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB65, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB66, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB67, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB68, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB69, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB70, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB71, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB72, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB73, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB74, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB75, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB76, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB77, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB78, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB79, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB80, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB81, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB82, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB83, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB84, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB85, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB86, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB87, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB88, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB89, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB90, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB91, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB92, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB93, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB94, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB95, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB96, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB97, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB98, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB99, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB100, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB101, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB102, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB103, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB104, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB105, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB106, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB107, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB108, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB109, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB110, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB111, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB112, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB113, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB114, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB115, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB116, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB117, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB118, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB119, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB120, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB121, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB122, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB123, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB124, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB125, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB126, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB127, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB128, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_tv_itwillB129, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
animmove_t victimSsithra_move_c_action6 = { 129, ssithra_frames_c_action6, ai_c_cycleend };