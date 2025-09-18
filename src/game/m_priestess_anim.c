//
// m_priestess_anim.c
//
// Copyright 1998 Raven Software
//

#include "m_priestess_anim.h"
#include "m_priestess_shared.h"
#include "g_ai.h" //mxd
#include "mg_ai.h" //mxd

// Priestess Stand.
static const animframe_t priestess_frames_stand1[] =
{
	{ FRAME_idle1,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle2,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle3,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle4,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle5,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle6,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle7,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle8,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle9,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle10,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle11,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle12,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle13,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle14,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle15,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle16,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle17,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle18,	NULL, 0, 0, 0, ai_stand, 0, NULL },
};
const animmove_t priestess_move_stand1 = ANIMMOVE(priestess_frames_stand1, NULL);

// Priestess Attack 1 Start.
static const animframe_t priestess_frames_attack1_go[] =
{
	{ FRAME_attackA7,	NULL,				0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_attackA8,	NULL,				0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_attackA9,	NULL,				0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_attackA10,	priestess_fire1,	0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_attackA11,	NULL,				0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_attackA12,	NULL,				0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_attackA13,	NULL,				0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_attackA14,	NULL,				0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_attackA15,	NULL,				0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_attackA16,	NULL,				0, 0, 0, ai_charge, 0, NULL },
};
const animmove_t priestess_move_attack1_go = ANIMMOVE(priestess_frames_attack1_go, priestess_attack1_pause);

// Priestess Attack 1 Loop.
static const animframe_t priestess_frames_attack1_loop[] =
{
	{ FRAME_attackA7,	NULL,				0, 0, 0, NULL, 0, NULL },
	{ FRAME_attackA8,	NULL,				0, 0, 0, NULL, 0, NULL },
	{ FRAME_attackA9,	NULL,				0, 0, 0, NULL, 0, NULL },
	{ FRAME_attackA10,	NULL,				0, 0, 0, NULL, 0, NULL },
	{ FRAME_attackA11,	priestess_fire1,	0, 0, 0, NULL, 0, NULL },
	{ FRAME_attackA12,	NULL,				0, 0, 0, NULL, 0, NULL },
};
const animmove_t priestess_move_attack1_loop = ANIMMOVE(priestess_frames_attack1_loop, NULL);

// Priestess Attack 1 End.
static const animframe_t priestess_frames_attack1_end[] =
{
	{ FRAME_attackA13,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attackA14,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attackA15,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attackA16,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t priestess_move_attack1_end = ANIMMOVE(priestess_frames_attack1_end, priestess_pause);

// Priestess Attack 2.
static const animframe_t priestess_frames_attack2[] =
{
	{ FRAME_attackB1,	NULL, 0, 0, 0, ai_charge, 2, NULL },
	{ FRAME_attackB2,	NULL, 0, 0, 0, ai_charge, 2, NULL },
	{ FRAME_attackB3,	NULL, 0, 0, 0, ai_charge, 2, NULL },
	{ FRAME_attackB4,	NULL, 0, 0, 0, ai_charge, 4, NULL },
	{ FRAME_attackB5,	NULL, 0, 0, 0, ai_charge, 4, NULL },
	{ FRAME_attackB6,	NULL, 0, 0, 0, ai_charge, 4, NULL },
	{ FRAME_attackB7,	NULL, 0, 0, 0, ai_charge, 2, NULL },
	{ FRAME_attackB8,	NULL, 0, 0, 0, priestess_strike, 16, NULL },
	{ FRAME_attackB9,	NULL, 0, 0, 0, ai_charge, 1, NULL },
	{ FRAME_attackB10,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_attackB11,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_attackB12,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_attackB13,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_attackB14,	NULL, 0, 0, 0, priestess_strike, 8, NULL },
	{ FRAME_attackB15,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_attackB16,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_attackB17,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_attackB18,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_attackB19,	NULL, 0, 0, 0, ai_charge, 0, NULL },
};
const animmove_t priestess_move_attack2 = ANIMMOVE(priestess_frames_attack2, priestess_pause);

// Priestess Pounce Attack.
static const animframe_t priestess_frames_pounce_attack[] =
{
	{ FRAME_atttrans1,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_atttrans2,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_attackB19,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_attackB12,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_attackB13,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_attackB14,	NULL, 0, 0, 0, priestess_strike, 8, NULL },
	{ FRAME_attackB15,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_attackB16,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_attackB17,	NULL, 0, 0, 0, ai_charge, 0, NULL },
};
const animmove_t priestess_move_pounce_attack = ANIMMOVE(priestess_frames_pounce_attack, priestess_pounce_attack);

// Priestess Backup.
static const animframe_t priestess_frames_backup[] =
{
	{ FRAME_backup3,	NULL, 0, 0, 0, MG_AI_Run, -8, NULL },
	{ FRAME_backup4,	NULL, 0, 0, 0, MG_AI_Run, -8, NULL },
	{ FRAME_backup5,	NULL, 0, 0, 0, MG_AI_Run, -8, NULL },
	{ FRAME_backup6,	NULL, 0, 0, 0, MG_AI_Run, -8, NULL },
	{ FRAME_backup7,	NULL, 0, 0, 0, MG_AI_Run, -8, NULL },
	{ FRAME_backup8,	NULL, 0, 0, 0, MG_AI_Run, -8, NULL },
	{ FRAME_backup9,	NULL, 0, 0, 0, MG_AI_Run, -8, NULL },
	{ FRAME_backup10,	NULL, 0, 0, 0, MG_AI_Run, -8, NULL },
	{ FRAME_backup11,	NULL, 0, 0, 0, MG_AI_Run, -8, NULL },
	{ FRAME_backup12,	NULL, 0, 0, 0, MG_AI_Run, -8, NULL },
	{ FRAME_backup13,	NULL, 0, 0, 0, MG_AI_Run, -8, NULL },
	{ FRAME_backup14,	NULL, 0, 0, 0, MG_AI_Run, -8, NULL },
};
const animmove_t priestess_move_backup = ANIMMOVE(priestess_frames_backup, priestess_pause);

// Priestess Death.
static const animframe_t priestess_frames_death[] =
{
	{ FRAME_death1,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death2,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death3,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death4,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death5,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death6,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death7,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death8,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death9,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death10,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death11,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death12,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death13,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death14,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death15,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death16,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death17,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death18,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death19,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death20,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death21,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death22,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death23,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death24,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death25,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death26,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death27,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death28,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death29,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death30,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death31,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death32,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death33,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death34,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death35,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death36,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death37,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death38,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death39,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death40,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death41,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death42,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death43,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death44,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death45,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death46,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death47,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death48,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death49,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death50,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death51,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death52,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death53,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death54,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death55,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death56,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death57,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death58,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death59,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death60,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death61,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death62,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death63,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death64,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death65,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death66,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death67,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death68,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death69,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death70,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t priestess_move_death = ANIMMOVE(priestess_frames_death, priestess_dead);

// Priestess Idle.
static const animframe_t priestess_frames_idle[] =
{
	{ FRAME_idle1,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle2,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle3,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle4,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle5,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle6,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle7,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle8,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle9,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle10,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle11,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle12,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle13,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle14,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle15,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle16,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle17,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle18,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t priestess_move_idle = ANIMMOVE(priestess_frames_idle, NULL);

// Priestess Jump.
static const animframe_t priestess_frames_jump[] =
{
	{ FRAME_jump1,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump2,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump3,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump4,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump5,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump6,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump7,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump8,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump9,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump10,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump11,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump12,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump13,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump14,	NULL, 0, 0, 0, ai_charge, 0, NULL },
};
const animmove_t priestess_move_jump = ANIMMOVE(priestess_frames_jump, priestess_pause);

// Priestess Jump Forward.
static const animframe_t priestess_frames_jump_forward[] =
{
	{ FRAME_jump1,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump2,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump3,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump4,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump5,	NULL, 0, 0, 0, ai_charge, 0, priestess_jump_forward },
	{ FRAME_jump6,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump7,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump8,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump9,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump10,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump11,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump12,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump13,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump14,	NULL, 0, 0, 0, ai_charge, 0, NULL },
};
const animmove_t priestess_move_jump_forward = ANIMMOVE(priestess_frames_jump_forward, priestess_pause);

// Priestess Jump Back.
static const animframe_t priestess_frames_jump_back[] =
{
	{ FRAME_jump1,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump2,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump3,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump4,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump5,	NULL, 0, 0, 0, ai_charge, 0, priestess_jump_back },
	{ FRAME_jump6,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump7,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump8,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump9,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump10,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump11,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump12,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump13,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump14,	NULL, 0, 0, 0, ai_charge, 0, NULL },
};
const animmove_t priestess_move_jump_back = ANIMMOVE(priestess_frames_jump_back, priestess_pause);

// Priestess Jump Right.
static const animframe_t priestess_frames_jump_right[] =
{
	{ FRAME_jump1,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump2,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump3,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump4,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump5,	NULL, 0, 0, 0, ai_charge, 0, priestess_jump_right },
	{ FRAME_jump6,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump7,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump8,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump9,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump10,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump11,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump12,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump13,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump14,	NULL, 0, 0, 0, ai_charge, 0, NULL },
};
const animmove_t priestess_move_jump_right = ANIMMOVE(priestess_frames_jump_right, priestess_pause);

// Priestess Jump Left.
static const animframe_t priestess_frames_jump_left[] =
{
	{ FRAME_jump1,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump2,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump3,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump4,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump5,	NULL, 0, 0, 0, ai_charge, 0, priestess_jump_left },
	{ FRAME_jump6,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump7,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump8,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump9,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump10,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump11,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump12,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump13,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump14,	NULL, 0, 0, 0, ai_charge, 0, NULL },
};
const animmove_t priestess_move_jump_left = ANIMMOVE(priestess_frames_jump_left, priestess_pause);

// Priestess Jump Pounce.
static const animframe_t priestess_frames_jump_pounce[] =
{
	{ FRAME_jump1,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump2,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump3,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump4,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump5,	NULL, 0, 0, 0, ai_charge, 0, priestess_pounce },
	{ FRAME_jump6,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump7,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump8,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump9,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump10,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump11,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump12,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump13,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump14,	NULL, 0, 0, 0, ai_charge, 0, NULL },
};
const animmove_t priestess_move_jump_pounce = ANIMMOVE(priestess_frames_jump_pounce, priestess_pounce_attack);

// Priestess Pain.
static const animframe_t priestess_frames_pain[] =
{
	{ FRAME_pain1,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pain2,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pain3,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pain4,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pain5,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pain6,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t priestess_move_pain = ANIMMOVE(priestess_frames_pain, priestess_pause);

// Priestess Idle Pose.
static const animframe_t priestess_frames_idle_pose[] =
{
	{ FRAME_poseidle1,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poseidle2,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poseidle3,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poseidle4,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poseidle5,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poseidle6,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poseidle7,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poseidle8,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poseidle9,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t priestess_move_idle_pose = ANIMMOVE(priestess_frames_idle_pose, NULL);

// Priestess Idle Pose to Ready Transition.
static const animframe_t priestess_frames_pose_trans[] =
{
	{ FRAME_pose2ready1,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pose2ready2,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pose2ready3,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pose2ready4,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pose2ready5,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pose2ready6,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t priestess_move_pose_trans = ANIMMOVE(priestess_frames_pose_trans, NULL);

// Priestess Shield Start.
static const animframe_t priestess_frames_shield_go[] =
{
	{ FRAME_shield1,	NULL, 0, 0, 0, NULL,					0, NULL },
	{ FRAME_shield2,	NULL, 0, 0, 0, NULL,					0, priestess_teleport_go },
	{ FRAME_shield3,	NULL, 0, 0, 0, NULL,					0, NULL },
	{ FRAME_shield4,	NULL, 0, 0, 0, NULL,					0, NULL },
	{ FRAME_shield5,	NULL, 0, 0, 0, NULL,					0, NULL },
	{ FRAME_shield6,	NULL, 0, 0, 0, NULL,					0, NULL },
	{ FRAME_shield7,	NULL, 0, 0, 0, NULL,					0, priestess_teleport_self_effects },
	{ FRAME_shield8,	NULL, 0, 0, 0, priestess_delta_alpha, -32, NULL },
	{ FRAME_shield8,	NULL, 0, 0, 0, priestess_delta_alpha, -32, NULL },
	{ FRAME_shield8,	NULL, 0, 0, 0, priestess_delta_alpha, -32, NULL },
	{ FRAME_shield8,	NULL, 0, 0, 0, priestess_delta_alpha, -32, NULL },
	{ FRAME_shield8,	NULL, 0, 0, 0, priestess_delta_alpha, -32, NULL },
	{ FRAME_shield8,	NULL, 0, 0, 0, priestess_delta_alpha, -32, NULL },
	{ FRAME_shield8,	NULL, 0, 0, 0, priestess_delta_alpha, -32, priestess_teleport_move },
	{ FRAME_shield8,	NULL, 0, 0, 0, priestess_delta_alpha, -32, priestess_teleport_return },
};
const animmove_t priestess_move_shield_go = ANIMMOVE(priestess_frames_shield_go, NULL);

// Priestess Shield End.
static const animframe_t priestess_frames_shield_end[] =
{
	{ FRAME_shield8,	NULL, 0, 0, 0, priestess_delta_alpha, 32, priestess_teleport_end },
	{ FRAME_shield7,	NULL, 0, 0, 0, priestess_delta_alpha, 32, NULL },
	{ FRAME_shield6,	NULL, 0, 0, 0, priestess_delta_alpha, 32, NULL },
	{ FRAME_shield5,	NULL, 0, 0, 0, priestess_delta_alpha, 32, NULL },
	{ FRAME_shield4,	NULL, 0, 0, 0, priestess_delta_alpha, 32, NULL },
	{ FRAME_shield3,	NULL, 0, 0, 0, priestess_delta_alpha, 32, NULL },
	{ FRAME_shield2,	NULL, 0, 0, 0, priestess_delta_alpha, 32, NULL },
	{ FRAME_shield1,	NULL, 0, 0, 0, priestess_delta_alpha, 32, priestess_stop_alpha },
	{ FRAME_shield1,	NULL, 0, 0, 0, NULL,				   0, priestess_stop_alpha }, //TODO: why priestess_stop_alpha() is called twice?
};
const animmove_t priestess_move_shield_end = ANIMMOVE(priestess_frames_shield_end, priestess_pause);

// Priestess Dodge Left.
static const animframe_t priestess_frames_dodge_left[] =
{
	{ FRAME_sidestplft1,	priestess_move, 0,   0, 0, NULL, 0, NULL },
	{ FRAME_sidestplft2,	priestess_move, 0,   0, 0, NULL, 0, NULL },
	{ FRAME_sidestplft3,	priestess_move, 0,  -4, 0, NULL, 0, NULL },
	{ FRAME_sidestplft4,	priestess_move, 0,  -8, 0, NULL, 0, NULL },
	{ FRAME_sidestplft5,	priestess_move, 0, -16, 0, NULL, 0, NULL },
	{ FRAME_sidestplft6,	priestess_move, 0, -16, 0, NULL, 0, NULL },
	{ FRAME_sidestplft7,	priestess_move, 0, -16, 0, NULL, 0, NULL },
	{ FRAME_sidestplft8,	priestess_move, 0, -12, 0, NULL, 0, NULL },
	{ FRAME_sidestplft9,	priestess_move, 0,  -8, 0, NULL, 0, NULL },
	{ FRAME_sidestplft10,	priestess_move, 0,  -4, 0, NULL, 0, NULL },
	{ FRAME_sidestplft11,	priestess_move, 0,   0, 0, NULL, 0, NULL },
};
const animmove_t priestess_move_dodge_left = ANIMMOVE(priestess_frames_dodge_left, priestess_pause);

// Priestess Dodge Right.
static const animframe_t priestess_frames_dodge_right[] =
{
	{ FRAME_sidestprt1,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_sidestprt2,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_sidestprt3,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_sidestprt4,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_sidestprt5,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_sidestprt6,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_sidestprt7,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_sidestprt8,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_sidestprt9,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_sidestprt10,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_sidestprt11,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t priestess_move_dodge_right = ANIMMOVE(priestess_frames_dodge_right, priestess_pause);

// Priestess Walk.
static const animframe_t priestess_frames_walk[] =
{
	{ FRAME_walk1,	NULL, 0, 0, 0, MG_AI_Run, 6, NULL },
	{ FRAME_walk2,	NULL, 0, 0, 0, MG_AI_Run, 8, NULL },
	{ FRAME_walk3,	NULL, 0, 0, 0, MG_AI_Run, 8, NULL },
	{ FRAME_walk4,	NULL, 0, 0, 0, MG_AI_Run, 8, NULL },
	{ FRAME_walk5,	NULL, 0, 0, 0, MG_AI_Run, 8, NULL },
	{ FRAME_walk6,	NULL, 0, 0, 0, MG_AI_Run, 6, NULL },
	{ FRAME_walk7,	NULL, 0, 0, 0, MG_AI_Run, 6, NULL },
	{ FRAME_walk8,	NULL, 0, 0, 0, MG_AI_Run, 4, NULL },
	{ FRAME_walk9,	NULL, 0, 0, 0, MG_AI_Run, 6, NULL },
	{ FRAME_walk10,	NULL, 0, 0, 0, MG_AI_Run, 8, NULL },
	{ FRAME_walk11,	NULL, 0, 0, 0, MG_AI_Run, 8, NULL },
	{ FRAME_walk12,	NULL, 0, 0, 0, MG_AI_Run, 8, NULL },
	{ FRAME_walk13,	NULL, 0, 0, 0, MG_AI_Run, 8, NULL },
	{ FRAME_walk14,	NULL, 0, 0, 0, MG_AI_Run, 6, NULL },
	{ FRAME_walk15,	NULL, 0, 0, 0, MG_AI_Run, 6, NULL },
	{ FRAME_walk16,	NULL, 0, 0, 0, MG_AI_Run, 4, NULL },
};
const animmove_t priestess_move_walk = ANIMMOVE(priestess_frames_walk, priestess_pause);

// Priestess Attack 3 Start.
static const animframe_t priestess_frames_attack3_go[] =
{
	{ FRAME_attackc1,	NULL, 0, 0, 0, ai_charge, 6, NULL },
	{ FRAME_attackc2,	NULL, 0, 0, 0, ai_charge, 6, NULL },
	{ FRAME_attackc3,	NULL, 0, 0, 0, ai_charge, 6, NULL },
	{ FRAME_attackc4,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_attackc5,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_attackc6,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_attackc7,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_attackc8,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_attackc9,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_attackc10,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_attackc11,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_attackc12,	NULL, 0, 0, 0, ai_charge, 0, NULL },
};
const animmove_t priestess_move_attack3_go = ANIMMOVE(priestess_frames_attack3_go, priestess_attack3_loop);

// Priestess Attack 3 Loop.
static const animframe_t priestess_frames_attack3_loop[] =
{
	{ FRAME_attackc13,	NULL, 0, 0, 0, NULL, 0, priestess_attack3_loop_fire },
};
const animmove_t priestess_move_attack3_loop = ANIMMOVE(priestess_frames_attack3_loop, NULL);

// Priestess Attack 3 End.
static const animframe_t priestess_frames_attack3_end[] =
{
	{ FRAME_attackc14,	NULL, 0, 0, 0, ai_charge, -8, NULL },
	{ FRAME_attackc15,	NULL, 0, 0, 0, ai_charge, -8, NULL },
	{ FRAME_attackc16,	NULL, 0, 0, 0, ai_charge, -2, NULL },
};
const animmove_t priestess_move_attack3_end = ANIMMOVE(priestess_frames_attack3_end, priestess_pause);

// Priestess Jump Attack.
static const animframe_t priestess_frames_jump_attack[] =
{
	{ FRAME_jumpatt1,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jumpatt2,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jumpatt3,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jumpatt4,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jumpatt5,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jumpatt6,	NULL, 0, 0, 0, ai_charge, 0, priestess_jump_attack },
	{ FRAME_jumpatt7,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jumpatt8,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jumpatt9,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jumpatt10,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jumpatt11,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jumpatt12,	NULL, 0, 0, 0, priestess_strike, 8, NULL },
	{ FRAME_jumpatt13,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jumpatt14,	NULL, 0, 0, 0, ai_charge, 0, NULL },
};
const animmove_t priestess_move_jump_attack = ANIMMOVE(priestess_frames_jump_attack, priestess_pause);