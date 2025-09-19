//
// m_harpy_shared.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "g_local.h" //mxd

typedef enum AnimID_e
{
	ANIM_DIE,
	ANIM_FLY1,
	ANIM_FLYBACK1,
	ANIM_HOVER1,
	ANIM_HOVERSCREAM,
	ANIM_DIVE_GO,
	ANIM_DIVE_LOOP,
	ANIM_DIVE_END,
	ANIM_PAIN1,
	//ANIM_GLIDE1, //mxd. Unused.
	ANIM_DIVE_TRANS,
	ANIM_HIT_LOOP,
	ANIM_TUMBLE,
	ANIM_PERCH1,
	ANIM_PERCH2,
	ANIM_PERCH3,
	ANIM_PERCH4,
	ANIM_PERCH5,
	ANIM_PERCH6,
	ANIM_PERCH7,
	ANIM_PERCH8,
	ANIM_PERCH9,
	ANIM_TAKEOFF,
	ANIM_CIRCLING,
	ANIM_CIRCLING_FLAP,

	NUM_ANIMS
} AnimID_t;

typedef enum SoundID_e
{
	SND_FLAP,
	SND_SCREAM,
	SND_FLAP_FAST,
	SND_DIVE,
	SND_DEATH,
	SND_PAIN1,
	SND_PAIN2,
	SND_ATTACK1, //mxd
	SND_ATTACK2, //mxd
	SND_GIB,
	SND_IDLE1,
	SND_IDLE2,

	NUM_SOUNDS
} SoundID_t;

extern const animmove_t harpy_move_die1;
extern const animmove_t harpy_move_fly1;
extern const animmove_t harpy_move_flyback1;
extern const animmove_t harpy_move_hover1;
extern const animmove_t harpy_move_hoverscream;
extern const animmove_t harpy_move_dive_go;
extern const animmove_t harpy_move_dive_loop;
extern const animmove_t harpy_move_dive_end;
extern const animmove_t harpy_move_pain1;
//extern const animmove_t harpy_move_glide;
extern const animmove_t harpy_move_dive_trans;
extern const animmove_t harpy_move_dive_hit_loop;
extern const animmove_t harpy_move_tumble;
extern const animmove_t harpy_move_takeoff;
extern const animmove_t harpy_move_circle;
extern const animmove_t harpy_move_circle_flap;

// Perches.
extern const animmove_t harpy_move_perch1_idle;
extern const animmove_t harpy_move_perch2_idle;
extern const animmove_t harpy_move_perch3_idle;
extern const animmove_t harpy_move_perch4_idle;
extern const animmove_t harpy_move_perch5_idle;
extern const animmove_t harpy_move_perch6_idle;
extern const animmove_t harpy_move_perch7_idle;
extern const animmove_t harpy_move_perch8_idle;
extern const animmove_t harpy_move_perch9_idle;

extern void harpy_ai_fly(edict_t* self, float forward_offset, float right_offset, float up_offset);
extern void harpy_ai_glide(edict_t* self, float forward_offset, float right_offset, float up_offset);
extern void harpy_ai_circle(edict_t* self, float forward_offset, float right_offset, float up_offset);
extern void harpy_ai_perch(edict_t* self);
extern void harpy_ai_hover(edict_t* self, float distance);

extern void harpy_flap_noise(edict_t* self);
extern void harpy_flap_fast_noise(edict_t* self);
extern void harpy_dive_noise(edict_t* self);

extern void harpy_dive_loop(edict_t* self);
extern void harpy_hit_loop(edict_t* self);
extern void harpy_check_dodge(edict_t* self);
extern void harpy_dead(edict_t* self);
extern void harpy_pause(edict_t* self);
extern void harpy_flyback(edict_t* self);
extern void harpy_fix_angles(edict_t* self);

extern void harpy_hover_move(edict_t* self);
extern void harpy_dive_move(edict_t* self);
extern void harpy_dive_end_move(edict_t* self);
extern void harpy_tumble_move(edict_t* self);

#define hl_backspikes		1
#define hl_head				2
#define hl_stinger			3
#define hl_lwing			4
#define hl_lefthand			5
#define hl_rwing			6
#define hl_righthand		7
#define hl_leftupperleg		8
#define hl_leftlowerleg		9
#define hl_rightupperleg	10
#define hl_rightlowerleg	11
#define hl_harpy_max		12

#define BPN_PAIN		0
#define BPN_HEAD		1
#define BPN_HORNS		2
#define BPN_HORN		4
#define BPN_BACKSPIKES	8
#define BPN_NECKSPIKES	16
#define BPN_LUARM		32
#define BPN_LLARM		64
#define BPN_LHAND		128
#define BPN_RUARM		256
#define BPN_RLARM		512
#define BPN_RHAND		1024
#define BPN_TAILSPIKES	2048
#define BPN_RWING		4096
#define BPN_LWING		8192
#define BPN_STINGER		16384