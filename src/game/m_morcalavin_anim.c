//
// m_morcalavin_anim.c
//
// Copyright 1998 Raven Software
//

#include "m_morcalavin_anim.h"
#include "m_morcalavin_shared.h"
#include "g_ai.h" //mxd

// Morcalavin Float.
static const animframe_t morcalavin_frames_float[] =
{
	{ FRAME_fltidl1,	NULL, 0, 0, 0, morcalavin_ai_hover, 60, NULL },
	{ FRAME_fltidl2,	NULL, 0, 0, 0, morcalavin_ai_hover, 59, NULL },
	{ FRAME_fltidl3,	NULL, 0, 0, 0, morcalavin_ai_hover, 58, NULL },
	{ FRAME_fltidl4,	NULL, 0, 0, 0, morcalavin_ai_hover, 57, NULL },
	{ FRAME_fltidl5,	NULL, 0, 0, 0, morcalavin_ai_hover, 56, NULL },
	{ FRAME_fltidl6,	NULL, 0, 0, 0, morcalavin_ai_hover, 55, NULL },
	{ FRAME_fltidl7,	NULL, 0, 0, 0, morcalavin_ai_hover, 54, NULL },
	{ FRAME_fltidl8,	NULL, 0, 0, 0, morcalavin_ai_hover, 53, NULL },
	{ FRAME_fltidl9,	NULL, 0, 0, 0, morcalavin_ai_hover, 52, NULL },
	{ FRAME_fltidl10,	NULL, 0, 0, 0, morcalavin_ai_hover, 53, NULL },
	{ FRAME_fltidl11,	NULL, 0, 0, 0, morcalavin_ai_hover, 54, NULL },
	{ FRAME_fltidl12,	NULL, 0, 0, 0, morcalavin_ai_hover, 55, NULL },
	{ FRAME_fltidl13,	NULL, 0, 0, 0, morcalavin_ai_hover, 56, NULL },
	{ FRAME_fltidl14,	NULL, 0, 0, 0, morcalavin_ai_hover, 57, NULL },
	{ FRAME_fltidl15,	NULL, 0, 0, 0, morcalavin_ai_hover, 58, NULL },
};
const animmove_t morcalavin_move_float = ANIMMOVE(morcalavin_frames_float, NULL);

// Morcalavin Hurt Idle.
static const animframe_t morcalavin_frames_hurtidle[] =
{
	{ FRAME_hrtidl1,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_hrtidl2,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_hrtidl3,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_hrtidl4,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_hrtidl5,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_hrtidl6,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_hrtidl7,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_hrtidl8,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_hrtidl9,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_hrtidl10,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_hrtidl11,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_hrtidl12,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_hrtidl13,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_hrtidl14,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_hrtidl15,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_hrtidl16,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_hrtidl17,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t morcalavin_move_hurtidle = ANIMMOVE(morcalavin_frames_hurtidle, morcalavin_get_up);

// Morcalavin Attack 1.
static const animframe_t morcalavin_frames_attack1[] =
{
	{ FRAME_ataka1,		NULL,				0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_ataka2,		NULL,				0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_ataka3,		NULL,				0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_ataka4,		NULL,				0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_ataka5,		NULL,				0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_ataka6,		NULL,				0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_ataka7,		NULL,				0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_ataka8,		NULL,				0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_ataka9,		morcalavin_quake,	0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_ataka10,	NULL,				0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_ataka11,	NULL,				0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_ataka12,	NULL,				0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_ataka13,	NULL,				0, 0, 0, ai_charge, 0, NULL },
};
const animmove_t morcalavin_move_attack1 = ANIMMOVE(morcalavin_frames_attack1, morcalavin_quake_pause);

// Morcalavin Attack 2.
static const animframe_t morcalavin_frames_attack2[] =
{
	{ FRAME_atakb1,		NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakb1,		NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakb1,		NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakb1,		NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakb1,		NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakb1,		NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakb2,		NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakb3,		NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakb4,		NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakb5,		NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakb6,		NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakb7,		NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakb8,		NULL, 0, 0, 0, ai_charge, 0, morcalavin_taunt_shot }, // Release.
	{ FRAME_atakb9,		NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakb10,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakb11,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakb12,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakb13,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakb14,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakb15,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakb16,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakb17,	NULL, 0, 0, 0, ai_charge, 0, NULL },
};
const animmove_t morcalavin_move_attack2 = ANIMMOVE(morcalavin_frames_attack2, morcalavin_fade_out);

// Morcalavin Attack 2B.
static const animframe_t morcalavin_frames_attack2b[] =
{
	{ FRAME_atakb1,		NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakb2,		NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakb3,		NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakb4,		NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakb5,		NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakb6,		NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakb7,		NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakb8,		NULL, 0, 0, 0, ai_charge, 0, morcalavin_beam },
	{ FRAME_atakb9,		NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakb10,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakb11,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakb12,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakb13,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakb14,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakb15,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakb16,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakb17,	NULL, 0, 0, 0, ai_charge, 0, NULL },
};
const animmove_t morcalavin_move_attack2b = ANIMMOVE(morcalavin_frames_attack2b, morcalavin_pause);

// Morcalavin Attack 3.
static const animframe_t morcalavin_frames_attack3[] =
{
	{ FRAME_atakc1,		NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakc2,		NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakc3,		NULL, 0, 0, 0, ai_charge, 0, morcalavin_start_missile },
	{ FRAME_atakc4,		NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakc5,		NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakc6,		NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakc7,		NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakc8,		NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakc9,		NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakc10,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakc11,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakc12,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakc13,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakc14,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakc15,	NULL, 0, 0, 0, ai_charge, 0, morcalavin_release_missile },
	{ FRAME_atakc16,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakc17,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakc18,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakc19,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakc20,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakc21,	NULL, 0, 0, 0, ai_charge, 0, NULL },
};
const animmove_t morcalavin_move_attack3 = ANIMMOVE(morcalavin_frames_attack3, morcalavin_pause);

// Morcalavin Attack 4.
static const animframe_t morcalavin_frames_attack4[] =
{
	{ FRAME_defnsa1,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsa2,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsa3,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsa4,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsa5,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsa6,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsa7,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsa8,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsa9,	NULL, 0, 0, 0, ai_charge, 0, morcalavin_big_shot },
	{ FRAME_defnsa10,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsa11,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsa10,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsa9,	NULL, 0, 0, 0, ai_charge, 0, morcalavin_big_shot },
	{ FRAME_defnsa10,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsa11,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsa10,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsa9,	NULL, 0, 0, 0, ai_charge, 0, morcalavin_big_shot },
	{ FRAME_defnsa10,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsa11,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsa10,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsa9,	NULL, 0, 0, 0, ai_charge, 0, morcalavin_big_shot },
	{ FRAME_defnsa10,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsa11,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsa10,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsa9,	NULL, 0, 0, 0, ai_charge, 0, morcalavin_big_shot },
	{ FRAME_defnsa10,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsa11,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsa12,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsa13,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsa14,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsa15,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsa16,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsa17,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsa18,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsa19,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsa20,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsa21,	NULL, 0, 0, 0, ai_charge, 0, NULL },
};
const animmove_t morcalavin_move_attack4 = ANIMMOVE(morcalavin_frames_attack4, morcalavin_pause);

// Morcalavin Defence 1.
static const animframe_t morcalavin_frames_def1[] =
{
	{ FRAME_defnsa1,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsa2,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsa3,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsa4,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsa5,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsa6,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsa7,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsa8,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsa9,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsa10,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsa11,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsa12,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsa13,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsa14,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsa15,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsa15,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsa16,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsa17,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsa18,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsa19,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsa20,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsa21,	NULL, 0, 0, 0, ai_charge, 0, NULL },
};
const animmove_t morcalavin_move_def1 = ANIMMOVE(morcalavin_frames_def1, morcalavin_pause);

// Morcalavin Defence 2.
static const animframe_t morcalavin_frames_def2[] =
{
	{ FRAME_defnsb1,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsb2,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsb3,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsb4,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsb5,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsb6,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsb7,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsb8,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsb9,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsb10,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsb11,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsb12,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsb13,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsb14,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsb15,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsb16,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsb17,	NULL, 0, 0, 0, ai_charge, 0, NULL },
};
const animmove_t morcalavin_move_def2 = ANIMMOVE(morcalavin_frames_def2, morcalavin_pause);

// Morcalavin Walk.
static const animframe_t morcalavin_frames_walk[] =
{
	{ FRAME_walk1,		NULL, 0, 0, 0, morcalavin_ai_run, 6, NULL },
	{ FRAME_walk2,		NULL, 0, 0, 0, morcalavin_ai_run, 8, NULL },
	{ FRAME_walk3,		NULL, 0, 0, 0, morcalavin_ai_run, 8, NULL },
	{ FRAME_walk4,		NULL, 0, 0, 0, morcalavin_ai_run, 8, morcalavin_pause },
	{ FRAME_walk5,		NULL, 0, 0, 0, morcalavin_ai_run, 8, NULL },
	{ FRAME_walk6,		NULL, 0, 0, 0, morcalavin_ai_run, 6, NULL },
	{ FRAME_walk7,		NULL, 0, 0, 0, morcalavin_ai_run, 6, NULL },
	{ FRAME_walk8,		NULL, 0, 0, 0, morcalavin_ai_run, 4, morcalavin_pause },
	{ FRAME_walk9,		NULL, 0, 0, 0, morcalavin_ai_run, 6, NULL },
	{ FRAME_walk10,		NULL, 0, 0, 0, morcalavin_ai_run, 8, NULL },
	{ FRAME_walk11,		NULL, 0, 0, 0, morcalavin_ai_run, 8, NULL },
	{ FRAME_walk12,		NULL, 0, 0, 0, morcalavin_ai_run, 8, NULL },
};
const animmove_t morcalavin_move_walk = ANIMMOVE(morcalavin_frames_walk, morcalavin_pause);

// Morcalavin Fly.
static const animframe_t morcalavin_frames_fly[] =
{
	{ FRAME_fltmve1,	NULL, 0, 0, 0, morcalavin_ai_run, 16, morcalavin_rush_sound },
	{ FRAME_fltmve2,	NULL, 0, 0, 0, morcalavin_ai_run, 24, NULL },
	{ FRAME_fltmve3,	NULL, 0, 0, 0, morcalavin_ai_run, 24, NULL },
	{ FRAME_fltmve4,	NULL, 0, 0, 0, morcalavin_ai_run, 24, NULL },
	{ FRAME_fltmve5,	NULL, 0, 0, 0, morcalavin_ai_run, 24, NULL },
	{ FRAME_fltmve6,	NULL, 0, 0, 0, morcalavin_ai_run, 16, NULL },
	{ FRAME_fltmve7,	NULL, 0, 0, 0, morcalavin_ai_run, 16, NULL },
	{ FRAME_fltmve8,	NULL, 0, 0, 0, morcalavin_ai_run, 12, NULL },
	{ FRAME_fltmve9,	NULL, 0, 0, 0, morcalavin_ai_run, 16, NULL },
	{ FRAME_fltmve10,	NULL, 0, 0, 0, morcalavin_ai_run, 24, NULL },
	{ FRAME_fltmve11,	NULL, 0, 0, 0, morcalavin_ai_run, 24, NULL },
	{ FRAME_fltmve12,	NULL, 0, 0, 0, morcalavin_ai_run, 24, NULL },
	{ FRAME_fltmve13,	NULL, 0, 0, 0, morcalavin_ai_run, 24, NULL },
	{ FRAME_fltmve14,	NULL, 0, 0, 0, morcalavin_ai_run, 16, NULL },
	{ FRAME_fltmve15,	NULL, 0, 0, 0, morcalavin_ai_run, 16, NULL },
};
const animmove_t morcalavin_move_fly = ANIMMOVE(morcalavin_frames_fly, morcalavin_pause);

// Morcalavin Get Up.
static const animframe_t morcalavin_frames_getup[] =
{
	{ FRAME_getupa1,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_getupa2,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_getupa3,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_getupa4,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_getupa5,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_getupa6,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_getupa7,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_getupa8,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_getupa9,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_getupa10,	NULL, 0, 0, 0, morcalavin_ai_hover, 15,  NULL },
	{ FRAME_getupa11,	NULL, 0, 0, 0, morcalavin_ai_hover, 30,  NULL },
	{ FRAME_getupa12,	NULL, 0, 0, 0, morcalavin_ai_hover, 60,  NULL },
	{ FRAME_getupa13,	NULL, 0, 0, 0, morcalavin_ai_hover, 90,  NULL },
	{ FRAME_getupa14,	NULL, 0, 0, 0, morcalavin_ai_hover, 120, NULL },
};
const animmove_t morcalavin_move_getup = ANIMMOVE(morcalavin_frames_getup, morcalavin_retort);

// Morcalavin Retort.
static const animframe_t morcalavin_frames_retort[] =
{
	{ FRAME_getupb1,	NULL, 0, 0, 0, morcalavin_ai_hover, 120, NULL },
	{ FRAME_getupb2,	NULL, 0, 0, 0, morcalavin_ai_hover, 120, NULL },
	{ FRAME_getupb3,	NULL, 0, 0, 0, morcalavin_ai_hover, 120, NULL },
	{ FRAME_getupb4,	NULL, 0, 0, 0, morcalavin_ai_hover, 120, NULL },
	{ FRAME_getupb5,	NULL, 0, 0, 0, morcalavin_ai_hover, 115, NULL },
	{ FRAME_getupb6,	NULL, 0, 0, 0, morcalavin_ai_hover, 110, NULL },
	{ FRAME_getupb7,	NULL, 0, 0, 0, morcalavin_ai_hover, 105, NULL },
	{ FRAME_getupb8,	NULL, 0, 0, 0, morcalavin_ai_hover, 100, NULL },
	{ FRAME_getupb9,	NULL, 0, 0, 0, morcalavin_ai_hover,  80, NULL },
	{ FRAME_getupb10,	NULL, 0, 0, 0, morcalavin_ai_hover,  60, NULL },
	{ FRAME_getupb11,	NULL, 0, 0, 0, morcalavin_ai_hover,  40, NULL },
	{ FRAME_getupb12,	NULL, 0, 0, 0, morcalavin_ai_hover,  20, NULL },
	{ FRAME_getupb13,	NULL, 0, 0, 0, morcalavin_ai_hover,  15, NULL },
	{ FRAME_getupb14,	NULL, 0, 0, 0, morcalavin_ai_hover,  10, NULL },
	{ FRAME_getupb15,	NULL, 0, 0, 0, morcalavin_ai_hover,   5, NULL },
	{ FRAME_getupb16,	NULL, 0, 0, 0, morcalavin_ai_hover,   2, NULL },
	{ FRAME_getupb17,	NULL, 0, 0, 0, morcalavin_ai_hover,   0, NULL },
	{ FRAME_getupb18,	NULL, 0, 0, 0, morcalavin_ai_hover,   0, NULL },
	{ FRAME_getupc1,	NULL, 0, 0, 0, morcalavin_ai_hover,   0, NULL },
	{ FRAME_getupc2,	NULL, 0, 0, 0, morcalavin_ai_hover,   0, NULL },
	{ FRAME_getupc3,	NULL, 0, 0, 0, morcalavin_ai_hover,   0, NULL },
	{ FRAME_getupc4,	NULL, 0, 0, 0, morcalavin_ai_hover,   0, NULL },
	{ FRAME_getupc5,	NULL, 0, 0, 0, morcalavin_ai_hover,   0, NULL },
	{ FRAME_getupc6,	NULL, 0, 0, 0, morcalavin_ai_hover,   0, NULL },
	{ FRAME_getupc7,	NULL, 0, 0, 0, morcalavin_ai_hover,   0, NULL },
	{ FRAME_getupc8,	NULL, 0, 0, 0, morcalavin_ai_hover,   0, NULL },
	{ FRAME_getupc9,	NULL, 0, 0, 0, morcalavin_ai_hover,   0, NULL },
	{ FRAME_getupc10,	NULL, 0, 0, 0, morcalavin_ai_hover,   0, NULL },
	{ FRAME_getupc11,	NULL, 0, 0, 0, morcalavin_ai_hover,   0, NULL },
	{ FRAME_getupc12,	NULL, 0, 0, 0, morcalavin_ai_hover,   0, NULL },
	{ FRAME_getupc13,	NULL, 0, 0, 0, morcalavin_ai_hover,   0, NULL },
	{ FRAME_getupc14,	NULL, 0, 0, 0, morcalavin_ai_hover,   0, NULL },
	{ FRAME_getupc15,	NULL, 0, 0, 0, morcalavin_ai_hover,   0, NULL },
	{ FRAME_getupc16,	NULL, 0, 0, 0, morcalavin_ai_hover,   0, NULL },
};
const animmove_t morcalavin_move_retort = ANIMMOVE(morcalavin_frames_retort, morcalavin_end_retort);

// Morcalavin Fall.
static const animframe_t morcalavin_frames_fall[] =
{
	{ FRAME_knkdwn1,	NULL, 0, 0, 0, ai_move, -8, NULL },
	{ FRAME_knkdwn2,	NULL, 0, 0, 0, ai_move, -8, NULL },
	{ FRAME_knkdwn3,	NULL, 0, 0, 0, ai_move, -8, NULL },
	{ FRAME_knkdwn4,	NULL, 0, 0, 0, ai_move, -8, NULL },
	{ FRAME_knkdwn5,	NULL, 0, 0, 0, ai_move, -8, NULL },
	{ FRAME_knkdwn6,	NULL, 0, 0, 0, ai_move, -8, NULL },
	{ FRAME_knkdwn7,	NULL, 0, 0, 0, ai_move, -8, NULL },
	{ FRAME_knkdwn8,	NULL, 0, 0, 0, ai_move, -8, NULL },
	{ FRAME_knkdwn9,	NULL, 0, 0, 0, ai_move, -8, NULL },
	{ FRAME_knkdwn10,	NULL, 0, 0, 0, ai_move, -8, NULL },
	{ FRAME_knkdwn11,	NULL, 0, 0, 0, ai_move, -8, NULL },
	{ FRAME_knkdwn12,	NULL, 0, 0, 0, ai_move, -8, NULL },
	{ FRAME_knkdwn13,	NULL, 0, 0, 0, ai_move, -8, NULL },
	{ FRAME_knkdwn14,	NULL, 0, 0, 0, ai_move, -8, NULL },
	{ FRAME_knkdwn15,	NULL, 0, 0, 0, ai_move, -8, NULL },
	{ FRAME_knkdwn16,	NULL, 0, 0, 0, ai_move, -8, NULL },
	{ FRAME_knkdwn17,	NULL, 0, 0, 0, ai_move, -8, NULL },
	{ FRAME_knkdwn18,	NULL, 0, 0, 0, ai_move, -8, NULL },
	{ FRAME_knkdwn19,	NULL, 0, 0, 0, ai_move, -8, NULL },
	{ FRAME_knkdwn20,	NULL, 0, 0, 0, ai_move, -8, NULL },
	{ FRAME_knkdwn21,	NULL, 0, 0, 0, ai_move, -8, NULL },
};
const animmove_t morcalavin_move_fall = ANIMMOVE(morcalavin_frames_fall, morcalavin_hurt_idle);

// Morcalavin Glide.
static const animframe_t morcalavin_frames_glide[] =
{
	{ FRAME_fltmve1,	NULL, 0, 0, 0, morcalavin_ai_run, 6, NULL },
	{ FRAME_fltmve2,	NULL, 0, 0, 0, morcalavin_ai_run, 8, NULL },
	{ FRAME_fltmve3,	NULL, 0, 0, 0, morcalavin_ai_run, 8, NULL },
	{ FRAME_fltmve4,	NULL, 0, 0, 0, morcalavin_ai_run, 8, NULL },
	{ FRAME_fltmve5,	NULL, 0, 0, 0, morcalavin_ai_run, 8, NULL },
	{ FRAME_fltmve6,	NULL, 0, 0, 0, morcalavin_ai_run, 6, NULL },
	{ FRAME_fltmve7,	NULL, 0, 0, 0, morcalavin_ai_run, 6, NULL },
	{ FRAME_fltmve8,	NULL, 0, 0, 0, morcalavin_ai_run, 4, NULL },
	{ FRAME_fltmve9,	NULL, 0, 0, 0, morcalavin_ai_run, 6, NULL },
	{ FRAME_fltmve10,	NULL, 0, 0, 0, morcalavin_ai_run, 8, NULL },
	{ FRAME_fltmve11,	NULL, 0, 0, 0, morcalavin_ai_run, 8, NULL },
	{ FRAME_fltmve12,	NULL, 0, 0, 0, morcalavin_ai_run, 8, NULL },
	{ FRAME_fltmve13,	NULL, 0, 0, 0, morcalavin_ai_run, 6, NULL },
	{ FRAME_fltmve14,	NULL, 0, 0, 0, morcalavin_ai_run, 4, NULL },
	{ FRAME_fltmve15,	NULL, 0, 0, 0, morcalavin_ai_run, 8, NULL },
};
const animmove_t morcalavin_move_glide = ANIMMOVE(morcalavin_frames_glide, morcalavin_pause);

// Morcalavin Tracking Attack 1.
static const animframe_t morcalavin_frames_tracking_attack1[] =
{
	{ FRAME_defnsb1,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsb2,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsb3,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsb4,	morcalavin_tracking_projectile, 0, 0, -60, ai_charge, 0, NULL },
	{ FRAME_defnsb5,	morcalavin_tracking_projectile, 0, 0, -30, ai_charge, 0, NULL },
	{ FRAME_defnsb6,	morcalavin_tracking_projectile, 0, 0,   0, ai_charge, 0, NULL },
	{ FRAME_defnsb7,	morcalavin_tracking_projectile, 0, 0,  30, ai_charge, 0, NULL },
	{ FRAME_defnsb8,	morcalavin_tracking_projectile, 0, 0,  60, ai_charge, 0, NULL },
	{ FRAME_defnsb9,	morcalavin_tracking_projectile, 0, 0,  90, ai_charge, 0, NULL },
	{ FRAME_defnsb10,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsb11,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsb12,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsb13,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsb14,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsb15,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsb16,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_defnsb17,	NULL, 0, 0, 0, ai_charge, 0, NULL },
};
const animmove_t morcalavin_move_tracking_attack1 = ANIMMOVE(morcalavin_frames_tracking_attack1, morcalavin_pause);

// Morcalavin Ground Attack.
static const animframe_t morcalavin_frames_ground_attack[] =
{
	{ FRAME_atakb1,		NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakb2,		NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakb3,		NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakb4,		NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakb5,		NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakb6,		NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakb7,		NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakb8,		NULL, 0, 0, 0, ai_charge, 0, morcalavin_beam2 },
	{ FRAME_atakb9,		NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakb10,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakb11,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakb12,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakb13,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakb14,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakb15,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakb16,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atakb17,	NULL, 0, 0, 0, ai_charge, 0, NULL },
};
const animmove_t morcalavin_move_ground_attack = ANIMMOVE(morcalavin_frames_ground_attack, morcalavin_pause);