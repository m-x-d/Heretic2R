//
// m_harpy_anim.c
//
// Copyright 1998 Raven Software
//

#include "m_harpy_anim.h"
#include "m_harpy_shared.h"

// Harpy Die 1.
static const animframe_t harpy_frames_die1[] =
{
	{ FRAME_death1,		NULL, 0, 0, 0, NULL, 0, harpy_fix_angles },
	{ FRAME_death2,		NULL, 0, 0, 0, NULL, 0, harpy_fix_angles },
	{ FRAME_death3,		NULL, 0, 0, 0, NULL, 0, harpy_fix_angles },
	{ FRAME_death4,		NULL, 0, 0, 0, NULL, 0, harpy_fix_angles },
	{ FRAME_death5,		NULL, 0, 0, 0, NULL, 0, harpy_fix_angles },
	{ FRAME_death6,		NULL, 0, 0, 0, NULL, 0, harpy_fix_angles },
	{ FRAME_death7,		NULL, 0, 0, 0, NULL, 0, harpy_fix_angles },
	{ FRAME_death8,		NULL, 0, 0, 0, NULL, 0, harpy_fix_angles },
	{ FRAME_death9,		NULL, 0, 0, 0, NULL, 0, harpy_fix_angles },
	{ FRAME_death10,	NULL, 0, 0, 0, NULL, 0, harpy_fix_angles },
	{ FRAME_death11,	NULL, 0, 0, 0, NULL, 0, harpy_fix_angles },
	{ FRAME_death12,	NULL, 0, 0, 0, NULL, 0, harpy_fix_angles },
	{ FRAME_death13,	NULL, 0, 0, 0, NULL, 0, harpy_fix_angles },
	{ FRAME_death14,	NULL, 0, 0, 0, NULL, 0, harpy_fix_angles },
	{ FRAME_death15,	NULL, 0, 0, 0, NULL, 0, harpy_fix_angles },
};
const animmove_t harpy_move_die1 = ANIMMOVE(harpy_frames_die1, harpy_dead);

// Harpy Fly 1.
static const animframe_t harpy_frames_fly1[] =
{
	{ FRAME_fly1,	harpy_ai_fly, 64,  0, 0, NULL, 15, harpy_flap_noise },
	{ FRAME_fly2,	harpy_ai_fly, 64,  0, 0, NULL, 15, NULL },
	{ FRAME_fly3,	harpy_ai_fly, 72,  0, 0, NULL, 15, NULL },
	{ FRAME_fly4,	harpy_ai_fly, 128, 0, 0, NULL, 15, NULL },
	{ FRAME_fly5,	harpy_ai_fly, 114, 0, 0, NULL, 15, NULL },
	{ FRAME_fly6,	harpy_ai_fly, 108, 0, 0, NULL, 15, NULL },
	{ FRAME_fly7,	harpy_ai_fly, 84,  0, 0, NULL, 15, NULL },
	{ FRAME_fly8,	harpy_ai_fly, 72,  0, 0, NULL, 15, NULL },
	{ FRAME_fly9,	harpy_ai_fly, 64,  0, 0, NULL, 15, NULL },
	{ FRAME_fly10,	harpy_ai_fly, 52,  0, 0, NULL, 15, NULL },
	{ FRAME_fly11,	harpy_ai_fly, 48,  0, 0, NULL, 15, NULL },
	{ FRAME_fly12,	harpy_ai_fly, 52,  0, 0, NULL, 15, NULL },
};
const animmove_t harpy_move_fly1 = ANIMMOVE(harpy_frames_fly1, harpy_pause);

// Harpy Fly Backwards 1.
static const animframe_t harpy_frames_flyback1[] =
{
	{ FRAME_flyback1, harpy_ai_fly, -32, 0, 16, NULL, 0, harpy_flap_fast_noise },
	{ FRAME_flyback2, harpy_ai_fly, -64, 0, 64, NULL, 0, NULL },
	{ FRAME_flyback3, harpy_ai_fly, -52, 0, 54, NULL, 0, NULL },
	{ FRAME_flyback4, harpy_ai_fly, -48, 0, 48, NULL, 0, NULL },
	{ FRAME_flyback5, harpy_ai_fly, -42, 0, 32, NULL, 0, NULL },
	{ FRAME_flyback6, harpy_ai_fly, -36, 0, 24, NULL, 0, NULL },
};
const animmove_t harpy_move_flyback1 = ANIMMOVE(harpy_frames_flyback1, harpy_pause);

// Harpy Hover 1.
static const animframe_t harpy_frames_hover1[] =
{
	{ FRAME_hover1, NULL, 0, 0, 0, harpy_ai_hover,  2, harpy_flap_noise },
	{ FRAME_hover2, NULL, 0, 0, 0, harpy_ai_hover,  1, NULL },
	{ FRAME_hover3, NULL, 0, 0, 0, harpy_ai_hover, -1, NULL },
	{ FRAME_hover4, NULL, 0, 0, 0, harpy_ai_hover, -2, NULL },
	{ FRAME_hover5, NULL, 0, 0, 0, harpy_ai_hover, -2, harpy_check_dodge },
	{ FRAME_hover6, NULL, 0, 0, 0, harpy_ai_hover, -1, NULL },
	{ FRAME_hover7, NULL, 0, 0, 0, harpy_ai_hover,  1, NULL },
	{ FRAME_hover8, NULL, 0, 0, 0, harpy_ai_hover,  2, NULL },
};
const animmove_t harpy_move_hover1 = ANIMMOVE(harpy_frames_hover1, harpy_hover_move);

// Harpy Tumble.
static const animframe_t harpy_frames_tumble[] =
{
	{ FRAME_hover1, NULL, 0, 0, 0, NULL, 0, harpy_tumble_move },
	{ FRAME_hover2, NULL, 0, 0, 0, NULL, 0, harpy_tumble_move },
	{ FRAME_hover3, NULL, 0, 0, 0, NULL, 0, harpy_tumble_move },
	{ FRAME_hover4, NULL, 0, 0, 0, NULL, 0, harpy_tumble_move },
	{ FRAME_hover5, NULL, 0, 0, 0, NULL, 0, harpy_tumble_move },
	{ FRAME_hover6, NULL, 0, 0, 0, NULL, 0, harpy_tumble_move },
	{ FRAME_hover7, NULL, 0, 0, 0, NULL, 0, harpy_tumble_move },
	{ FRAME_hover8, NULL, 0, 0, 0, NULL, 0, harpy_tumble_move },
};
const animmove_t harpy_move_tumble = ANIMMOVE(harpy_frames_tumble, NULL);

// Harpy hovering and screaming.
static const animframe_t harpy_frames_hoverscream[] =
{
	{ FRAME_hoverscream1, NULL, 0, 0, 0, harpy_ai_hover,  2, harpy_flap_noise },
	{ FRAME_hoverscream2, NULL, 0, 0, 0, harpy_ai_hover,  1, NULL },
	{ FRAME_hoverscream3, NULL, 0, 0, 0, harpy_ai_hover, -1, NULL },
	{ FRAME_hoverscream4, NULL, 0, 0, 0, harpy_ai_hover, -2, NULL },
	{ FRAME_hoverscream5, NULL, 0, 0, 0, harpy_ai_hover, -2, NULL },
	{ FRAME_hoverscream6, NULL, 0, 0, 0, harpy_ai_hover, -1, NULL },
	{ FRAME_hoverscream7, NULL, 0, 0, 0, harpy_ai_hover,  1, NULL },
	{ FRAME_hoverscream8, NULL, 0, 0, 0, harpy_ai_hover,  2, NULL },
	{ FRAME_hoverscream9, NULL, 0, 0, 0, harpy_ai_hover,  2, harpy_flap_noise }, //mxd. +harpy_flap_noise.
	{ FRAME_hoverscream10,NULL, 0, 0, 0, harpy_ai_hover,  1, NULL },
	{ FRAME_hoverscream11,NULL, 0, 0, 0, harpy_ai_hover, -1, NULL },
	{ FRAME_hoverscream12,NULL, 0, 0, 0, harpy_ai_hover, -2, NULL },
	{ FRAME_hoverscream13,NULL, 0, 0, 0, harpy_ai_hover, -2, NULL },
	{ FRAME_hoverscream14,NULL, 0, 0, 0, harpy_ai_hover, -1, NULL },
	{ FRAME_hoverscream15,NULL, 0, 0, 0, harpy_ai_hover,  1, NULL },
	{ FRAME_hoverscream16,NULL, 0, 0, 0, harpy_ai_hover,  2, NULL },
};
const animmove_t harpy_move_hoverscream = ANIMMOVE(harpy_frames_hoverscream, harpy_hover_move);

// Harpy diving start.
static const animframe_t harpy_frames_dive_go[] =
{
	{ FRAME_dive01, NULL, 0, 0, 0, NULL, 0, harpy_dive_noise },
	{ FRAME_dive02, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_dive03, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_dive04, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_dive05, NULL, 0, 0, 0, NULL, 0, harpy_dive_move },
	{ FRAME_dive06, NULL, 0, 0, 0, NULL, 0, harpy_dive_move },
};
const animmove_t harpy_move_dive_go = ANIMMOVE(harpy_frames_dive_go, harpy_dive_loop);

// Harpy diving loop.
static const animframe_t harpy_frames_dive_loop[] =
{
	{ FRAME_dive07, NULL, 0, 0, 0, NULL, 0, harpy_dive_move },
	{ FRAME_dive08, NULL, 0, 0, 0, NULL, 0, harpy_dive_move },
	{ FRAME_dive09, NULL, 0, 0, 0, NULL, 0, harpy_dive_move },
	{ FRAME_dive10, NULL, 0, 0, 0, NULL, 0, harpy_dive_move },
	{ FRAME_dive11, NULL, 0, 0, 0, NULL, 0, harpy_dive_move },
	{ FRAME_dive12, NULL, 0, 0, 0, NULL, 0, harpy_dive_move },
};
const animmove_t harpy_move_dive_loop = ANIMMOVE(harpy_frames_dive_loop, NULL);

// Harpy transition from dive.
static const animframe_t harpy_frames_dive_trans[] =
{
	{ FRAME_dive13, NULL, 0, 0, 0, NULL, 0, harpy_dive_end_move },
	{ FRAME_dive14, NULL, 0, 0, 0, NULL, 0, harpy_dive_end_move },
	{ FRAME_dive15, NULL, 0, 0, 0, NULL, 0, harpy_dive_end_move },
	{ FRAME_dive16, NULL, 0, 0, 0, NULL, 0, harpy_dive_end_move },
	{ FRAME_dive17, NULL, 0, 0, 0, NULL, 0, harpy_dive_end_move },
};
const animmove_t harpy_move_dive_trans = ANIMMOVE(harpy_frames_dive_trans, harpy_hit_loop);

// Harpy dive hit loop.
static const animframe_t harpy_frames_dive_hit_loop[] =
{
	{ FRAME_dive18, NULL, 0, 0, 0, NULL, 0, harpy_dive_end_move },
};
const animmove_t harpy_move_dive_hit_loop = ANIMMOVE(harpy_frames_dive_hit_loop, NULL);

// Harpy dive end.
static const animframe_t harpy_frames_dive_end[] =
{
	{ FRAME_dive19, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_dive20, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_dive21, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_dive22, NULL, 0, 0, 0, NULL, 0, harpy_hit },
	{ FRAME_dive23, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_dive24, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_dive25, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_dive26, NULL, 0, 0, 0, NULL, 0, harpy_flap_noise }, //mxd. +harpy_flap_noise
	{ FRAME_dive27, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_dive28, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_dive29, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_dive30, NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t harpy_move_dive_end = ANIMMOVE(harpy_frames_dive_end, harpy_flyback);

// Harpy attacking up close/
static const animframe_t harpy_frames_closeattack[] =
{
	{ FRAME_dive17, NULL, 0, 0, 0, NULL, 6, harpy_hit },
	{ FRAME_dive19, NULL, 0, 0, 0, NULL, 6, NULL },
	{ FRAME_dive21, NULL, 0, 0, 0, NULL, 6, NULL },
	{ FRAME_dive23, NULL, 0, 0, 0, NULL, 6, NULL },
	{ FRAME_dive25, NULL, 0, 0, 0, NULL, 6, NULL },
	{ FRAME_dive27, NULL, 0, 0, 0, NULL, 6, NULL },
};
const animmove_t harpy_closeattack = ANIMMOVE(harpy_frames_closeattack, harpy_pause);

// Harpy pain.
static const animframe_t harpy_frames_pain1[] =
{
	{ FRAME_pain1, NULL, 0, 0, 0, NULL,  2, harpy_hover_move },
	{ FRAME_pain2, NULL, 0, 0, 0, NULL,  1, harpy_hover_move },
	{ FRAME_pain3, NULL, 0, 0, 0, NULL, -1, harpy_hover_move },
	{ FRAME_pain4, NULL, 0, 0, 0, NULL, -2, harpy_hover_move },
	{ FRAME_pain5, NULL, 0, 0, 0, NULL, -2, harpy_hover_move },
	{ FRAME_pain6, NULL, 0, 0, 0, NULL, -1, harpy_hover_move },
	{ FRAME_pain7, NULL, 0, 0, 0, NULL,  1, harpy_hover_move },
	{ FRAME_pain8, NULL, 0, 0, 0, NULL,  2, harpy_hover_move },
};
const animmove_t harpy_move_pain1 = ANIMMOVE(harpy_frames_pain1, harpy_pause);

// Harpy glide.
static const animframe_t harpy_frames_glide[] =
{
	{ FRAME_glide1,		harpy_ai_glide, 64, 0, 0, NULL, 0, NULL },
	{ FRAME_glide2,		harpy_ai_glide, 64, 0, 0, NULL, 0, NULL },
	{ FRAME_glide3,		harpy_ai_glide, 64, 0, 0, NULL, 0, NULL },
	{ FRAME_glide4,		harpy_ai_glide, 64, 0, 0, NULL, 0, NULL },
	{ FRAME_glide5,		harpy_ai_glide, 64, 0, 0, NULL, 0, NULL },
	{ FRAME_glide6,		harpy_ai_glide, 64, 0, 0, NULL, 0, NULL },
	{ FRAME_glide7,		harpy_ai_glide, 64, 0, 0, NULL, 0, NULL },
	{ FRAME_glide8,		harpy_ai_glide, 64, 0, 0, NULL, 0, NULL },
	{ FRAME_glide9,		harpy_ai_glide, 64, 0, 0, NULL, 0, NULL },
	{ FRAME_glide10,	harpy_ai_glide, 64, 0, 0, NULL, 0, NULL },
	{ FRAME_glide11,	harpy_ai_glide, 64, 0, 0, NULL, 0, NULL },
	{ FRAME_glide12,	harpy_ai_glide, 64, 0, 0, NULL, 0, NULL },
};
const animmove_t harpy_move_glide = ANIMMOVE(harpy_frames_glide, NULL);

// Harpy Perch 1 Idle.
static const animframe_t harpy_frames_perch1_idle[] =
{
	{ FRAME_perch1,		NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch2,		NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch3,		NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch4,		NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch5,		NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch6,		NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch7,		NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch8,		NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch9,		NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch10,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
};
const animmove_t harpy_move_perch1_idle = ANIMMOVE(harpy_frames_perch1_idle, NULL);

// Harpy Perch 2 Idle.
static const animframe_t harpy_frames_perch2_idle[] =
{
	{ FRAME_perch11,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch12,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch13,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch14,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch15,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch16,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch17,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch18,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch19,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch20,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
};
const animmove_t harpy_move_perch2_idle = ANIMMOVE(harpy_frames_perch2_idle, NULL);

// Harpy Perch 3 Idle.
static const animframe_t harpy_frames_perch3_idle[] =
{
	{ FRAME_perch21,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch22,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch23,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch24,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch25,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch26,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch27,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch28,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch29,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch30,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
};
const animmove_t harpy_move_perch3_idle = ANIMMOVE(harpy_frames_perch3_idle, NULL);

// Harpy Perch 4 Idle.
static const animframe_t harpy_frames_perch4_idle[] =
{
	{ FRAME_perch31,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch32,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch33,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch34,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch35,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch36,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch37,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch38,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch39,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch40,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
};
const animmove_t harpy_move_perch4_idle = ANIMMOVE(harpy_frames_perch4_idle, NULL);

// Harpy Perch 5 Idle.
static const animframe_t harpy_frames_perch5_idle[] =
{
	{ FRAME_perch41,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch42,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch43,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch44,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch45,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch46,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch47,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch48,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch49,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch50,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
};
const animmove_t harpy_move_perch5_idle = ANIMMOVE(harpy_frames_perch5_idle, NULL);

// Harpy Perch 6 Idle.
static const animframe_t harpy_frames_perch6_idle[] =
{
	{ FRAME_perch51,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch52,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch53,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch54,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch55,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch56,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch57,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch58,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch59,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch60,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
};
const animmove_t harpy_move_perch6_idle = ANIMMOVE(harpy_frames_perch6_idle, NULL);

// Harpy Perch 7 Idle.
static const animframe_t harpy_frames_perch7_idle[] =
{
	{ FRAME_perch61,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch62,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch63,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch64,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch65,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch66,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch67,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch68,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch69,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch70,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
};
const animmove_t harpy_move_perch7_idle = ANIMMOVE(harpy_frames_perch7_idle, NULL);

// Harpy Perch 8 Idle.
static const animframe_t harpy_frames_perch8_idle[] =
{
	{ FRAME_perch71,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch72,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch73,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch74,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch75,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch76,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch77,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch78,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch79,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch80,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
};
const animmove_t harpy_move_perch8_idle = ANIMMOVE(harpy_frames_perch8_idle, NULL);

// Harpy Perch 9 Idle.
static const animframe_t harpy_frames_perch9_idle[] =
{
	{ FRAME_perch81,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch82,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch83,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch84,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch85,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch86,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch87,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch88,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch89,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
	{ FRAME_perch90,	NULL, 0, 0, 0, NULL, 0, harpy_ai_perch },
};
const animmove_t harpy_move_perch9_idle = ANIMMOVE(harpy_frames_perch9_idle, NULL);

// Harpy Takeoff.
static const animframe_t harpy_frames_takeoff[] =
{
	{ FRAME_takeoff1,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_takeoff3,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_takeoff5,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_takeoff7,	harpy_ai_fly, -32, 0, 16, NULL, 0, harpy_flap_fast_noise },
	{ FRAME_takeoff9,	harpy_ai_fly, -48, 0, 32, NULL, 0, NULL },
	{ FRAME_takeoff11,	harpy_ai_fly, -32, 0, 32, NULL, 0, NULL },
	{ FRAME_takeoff13,	harpy_ai_fly, -64, 0, 32, NULL, 0, NULL },
	{ FRAME_takeoff15,	harpy_ai_fly, -32, 0, 32, NULL, 0, NULL },
};
const animmove_t harpy_move_takeoff = ANIMMOVE(harpy_frames_takeoff, harpy_pause);

// Harpy Circle.
static const animframe_t harpy_frames_circle[] =
{
	{ FRAME_glide1,		harpy_ai_circle, 32, 0, 0, NULL, 0, NULL },
	{ FRAME_glide2,		harpy_ai_circle, 32, 0, 0, NULL, 0, NULL },
	{ FRAME_glide3,		harpy_ai_circle, 32, 0, 0, NULL, 0, NULL },
	{ FRAME_glide4,		harpy_ai_circle, 32, 0, 0, NULL, 0, NULL },
	{ FRAME_glide5,		harpy_ai_circle, 32, 0, 0, NULL, 0, NULL },
	{ FRAME_glide6,		harpy_ai_circle, 32, 0, 0, NULL, 0, NULL },
	{ FRAME_glide7,		harpy_ai_circle, 32, 0, 0, NULL, 0, NULL },
	{ FRAME_glide8,		harpy_ai_circle, 32, 0, 0, NULL, 0, NULL },
	{ FRAME_glide9,		harpy_ai_circle, 32, 0, 0, NULL, 0, NULL },
	{ FRAME_glide10,	harpy_ai_circle, 32, 0, 0, NULL, 0, NULL },
	{ FRAME_glide11,	harpy_ai_circle, 32, 0, 0, NULL, 0, NULL },
	{ FRAME_glide12,	harpy_ai_circle, 32, 0, 0, NULL, 0, NULL },
};
const animmove_t harpy_move_circle = ANIMMOVE(harpy_frames_circle, harpy_pause);

// Harpy Circle Flap.
static const animframe_t harpy_frames_circle_flap[] =
{
	{ FRAME_fly1,		harpy_ai_circle, 32, 0, 0, NULL, 0, harpy_flap_noise },
	{ FRAME_fly2,		harpy_ai_circle, 48, 0, 0, NULL, 0, NULL },
	{ FRAME_fly3,		harpy_ai_circle, 64, 0, 0, NULL, 0, NULL },
	{ FRAME_fly4,		harpy_ai_circle, 64, 0, 0, NULL, 0, NULL },
	{ FRAME_fly5,		harpy_ai_circle, 56, 0, 0, NULL, 0, NULL },
	{ FRAME_fly6,		harpy_ai_circle, 56, 0, 0, NULL, 0, NULL },
	{ FRAME_fly7,		harpy_ai_circle, 50, 0, 0, NULL, 0, NULL },
	{ FRAME_fly8,		harpy_ai_circle, 50, 0, 0, NULL, 0, NULL },
	{ FRAME_fly9,		harpy_ai_circle, 44, 0, 0, NULL, 0, NULL },
	{ FRAME_fly10,		harpy_ai_circle, 40, 0, 0, NULL, 0, NULL },
	{ FRAME_fly11,		harpy_ai_circle, 36, 0, 0, NULL, 0, NULL },
	{ FRAME_fly12,		harpy_ai_circle, 32, 0, 0, NULL, 0, NULL },
};
const animmove_t harpy_move_circle_flap = ANIMMOVE(harpy_frames_circle_flap, harpy_pause);