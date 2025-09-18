//
// m_imp_anim.c
//
// Copyright 1998 Raven Software
//

#include "m_imp_anim.h"
#include "m_imp_shared.h"

// Imp dying 1.
static const animframe_t imp_frames_die1[] =
{
	{ FRAME_death1,		NULL, 0, 0, 0, NULL, 0, imp_fix_angles },
	{ FRAME_death2,		NULL, 0, 0, 0, NULL, 0, imp_fix_angles },
	{ FRAME_death3,		NULL, 0, 0, 0, NULL, 0, imp_fix_angles },
	{ FRAME_death4,		NULL, 0, 0, 0, NULL, 0, imp_fix_angles },
	{ FRAME_death5,		NULL, 0, 0, 0, NULL, 0, imp_fix_angles },
	{ FRAME_death6,		NULL, 0, 0, 0, NULL, 0, imp_fix_angles },
	{ FRAME_death7,		NULL, 0, 0, 0, NULL, 0, imp_fix_angles },
	{ FRAME_death8,		NULL, 0, 0, 0, NULL, 0, imp_fix_angles },
	{ FRAME_death9,		NULL, 0, 0, 0, NULL, 0, imp_fix_angles },
	{ FRAME_death10,	NULL, 0, 0, 0, NULL, 0, imp_fix_angles },
	{ FRAME_death11,	NULL, 0, 0, 0, NULL, 0, imp_fix_angles },
	{ FRAME_death12,	NULL, 0, 0, 0, NULL, 0, imp_fix_angles },
	{ FRAME_death13,	NULL, 0, 0, 0, NULL, 0, imp_fix_angles },
	{ FRAME_death14,	NULL, 0, 0, 0, NULL, 0, imp_fix_angles },
};
const animmove_t imp_move_die1 = ANIMMOVE(imp_frames_die1, imp_dead);

// Imp flying 1.
static const animframe_t imp_frames_fly1[] =
{
	{ FRAME_impfly1,	imp_ai_fly, 32, 0, 0, NULL, 15, imp_flap_noise },
	{ FRAME_impfly2,	imp_ai_fly, 32, 0, 0, NULL, 15, imp_fly_move },
	{ FRAME_impfly3,	imp_ai_fly, 32, 0, 0, NULL, 15, imp_fly_move },
	{ FRAME_impfly4,	imp_ai_fly, 36, 0, 0, NULL, 15, imp_fly_move },
	{ FRAME_impfly5,	imp_ai_fly, 36, 0, 0, NULL, 15, imp_fly_move },
	{ FRAME_impfly6,	imp_ai_fly, 64, 0, 0, NULL, 15, imp_fly_move },
	{ FRAME_impfly7,	imp_ai_fly, 64, 0, 0, NULL, 15, imp_fly_move },
	{ FRAME_impfly8,	imp_ai_fly, 57, 0, 0, NULL, 15, imp_fly_move },
	{ FRAME_impfly9,	imp_ai_fly, 54, 0, 0, NULL, 15, imp_fly_move },
	{ FRAME_impfly9,	imp_ai_fly, 54, 0, 0, NULL, 15, imp_fly_move },
	{ FRAME_impfly11,	imp_ai_fly, 42, 0, 0, NULL, 15, imp_fly_move },
	{ FRAME_impfly11,	imp_ai_fly, 42, 0, 0, NULL, 15, imp_fly_move },
	{ FRAME_impfly13,	imp_ai_fly, 36, 0, 0, NULL, 15, imp_fly_move },
	{ FRAME_impfly14,	imp_ai_fly, 32, 0, 0, NULL, 15, imp_fly_move },
	{ FRAME_impfly14,	imp_ai_fly, 32, 0, 0, NULL, 15, imp_fly_move },
	{ FRAME_impfly16,	imp_ai_fly, 26, 0, 0, NULL, 15, imp_fly_move },
	{ FRAME_impfly16,	imp_ai_fly, 26, 0, 0, NULL, 15, imp_fly_move },
	{ FRAME_impfly18,	imp_ai_fly, 24, 0, 0, NULL, 15, imp_fly_move },
	{ FRAME_impfly18,	imp_ai_fly, 24, 0, 0, NULL, 15, imp_fly_move },
	{ FRAME_impfly20,	imp_ai_fly, 26, 0, 0, NULL, 15, imp_fly_move },
};
const animmove_t imp_move_fly1 = ANIMMOVE(imp_frames_fly1, imp_pause);

// Imp flying backwards 1.
static const animframe_t imp_frames_flyback[] =
{
	{ FRAME_impfly1,	imp_ai_fly, -16, 0, 8,  NULL, 0, imp_flap_noise },
	{ FRAME_impfly2,	imp_ai_fly, -32, 0, 32, NULL, 0, imp_fly_move },
	{ FRAME_impfly3,	imp_ai_fly, -26, 0, 27, NULL, 0, imp_fly_move },
	{ FRAME_impfly4,	imp_ai_fly, -24, 0, 24, NULL, 0, imp_fly_move },
	{ FRAME_impfly5,	imp_ai_fly, -21, 0, 16, NULL, 0, imp_fly_move },
	{ FRAME_impfly6,	imp_ai_fly, -18, 0, 24, NULL, 0, imp_fly_move },
	{ FRAME_impfly7,	imp_ai_fly, -16, 0, 8,  NULL, 0, imp_flap_noise },
	{ FRAME_impfly8,	imp_ai_fly, -32, 0, 32, NULL, 0, imp_fly_move },
	{ FRAME_impfly9,	imp_ai_fly, -26, 0, 27, NULL, 0, imp_fly_move },
	{ FRAME_impfly10,	imp_ai_fly, -24, 0, 24, NULL, 0, imp_fly_move },
	{ FRAME_impfly11,	imp_ai_fly, -21, 0, 16, NULL, 0, imp_fly_move },
	{ FRAME_impfly12,	imp_ai_fly, -18, 0, 24, NULL, 0, imp_fly_move },
	{ FRAME_impfly13,	imp_ai_fly, -16, 0, 8,  NULL, 0, imp_flap_noise },
	{ FRAME_impfly14,	imp_ai_fly, -32, 0, 32, NULL, 0, imp_fly_move },
	{ FRAME_impfly15,	imp_ai_fly, -26, 0, 27, NULL, 0, imp_fly_move },
	{ FRAME_impfly16,	imp_ai_fly, -24, 0, 24, NULL, 0, imp_fly_move },
	{ FRAME_impfly17,	imp_ai_fly, -21, 0, 16, NULL, 0, imp_fly_move },
	{ FRAME_impfly18,	imp_ai_fly, -18, 0, 24, NULL, 0, imp_fly_move },
	{ FRAME_impfly19,	imp_ai_fly, -26, 0, 27, NULL, 0, imp_fly_move },
	{ FRAME_impfly20,	imp_ai_fly, -24, 0, 24, NULL, 0, imp_fly_move },
};
const animmove_t imp_move_flyback = ANIMMOVE(imp_frames_flyback, imp_pause);

// Imp hit from swoop.
static const animframe_t imp_frames_dive_end[] =
{
	{ FRAME_swpend1,	imp_ai_fly, -32, 0, 16, NULL, 0, imp_flap_noise },
	{ FRAME_swpend2,	imp_ai_fly, -64, 0, 64, NULL, 0, imp_fly_move },
	{ FRAME_swpend3,	imp_ai_fly, -52, 0, 54, NULL, 0, imp_fly_move },
	{ FRAME_swpend4,	imp_ai_fly, -48, 0, 48, NULL, 0, imp_fly_move },
	{ FRAME_swpend5,	imp_ai_fly, -42, 0, 32, NULL, 0, imp_fly_move },
	{ FRAME_swpend6,	imp_ai_fly, -36, 0, 24, NULL, 0, imp_fly_move },
	{ FRAME_swpend7,	imp_ai_fly, -32, 0, 16, NULL, 0, imp_flap_noise },
	{ FRAME_swpend8,	imp_ai_fly, -64, 0, 64, NULL, 0, imp_fly_move },
	{ FRAME_swpend9,	imp_ai_fly, -52, 0, 54, NULL, 0, imp_fly_move },
	{ FRAME_swpend10,	imp_ai_fly, -48, 0, 48, NULL, 0, imp_fly_move },
	{ FRAME_swpend11,	imp_ai_fly, -42, 0, 32, NULL, 0, imp_fly_move },
	{ FRAME_swpend12,	imp_ai_fly, -36, 0, 24, NULL, 0, imp_fly_move },
	{ FRAME_swpend13,	imp_ai_fly, -32, 0, 16, NULL, 0, imp_flap_noise },
	{ FRAME_swpend14,	imp_ai_fly, -64, 0, 64, NULL, 0, imp_fly_move },
	{ FRAME_swpend15,	imp_ai_fly, -52, 0, 54, NULL, 0, imp_fly_move },
};
const animmove_t imp_move_dive_end = ANIMMOVE(imp_frames_dive_end, imp_flyback);

// Imp hit from swoop.
static const animframe_t imp_frames_dive_out[] =
{
	{ FRAME_swpout1,	imp_ai_fly, -32, 0, 16, NULL, 0, imp_flap_noise },
	{ FRAME_swpout2,	imp_ai_fly, -64, 0, 64, NULL, 0, imp_fly_move },
	{ FRAME_swpout3,	imp_ai_fly, -52, 0, 54, NULL, 0, imp_fly_move },
	{ FRAME_swpout4,	imp_ai_fly, -48, 0, 48, NULL, 0, imp_fly_move },
	{ FRAME_swpout5,	imp_ai_fly, -42, 0, 32, NULL, 0, imp_fly_move },
	{ FRAME_swpout6,	imp_ai_fly, -36, 0, 24, NULL, 0, imp_fly_move },
	{ FRAME_swpout7,	imp_ai_fly, -32, 0, 16, NULL, 0, imp_flap_noise },
	{ FRAME_swpout8,	imp_ai_fly, -64, 0, 64, NULL, 0, imp_fly_move },
	{ FRAME_swpout9,	imp_ai_fly, -52, 0, 54, NULL, 0, imp_fly_move },
	{ FRAME_swpout10,	imp_ai_fly, -48, 0, 48, NULL, 0, imp_fly_move },
	{ FRAME_swpout11,	imp_ai_fly, -42, 0, 32, NULL, 0, imp_fly_move },
	{ FRAME_swpout12,	imp_ai_fly, -36, 0, 24, NULL, 0, imp_fly_move },
	{ FRAME_swpout13,	imp_ai_fly, -32, 0, 16, NULL, 0, imp_flap_noise },
	{ FRAME_swpout14,	imp_ai_fly, -64, 0, 64, NULL, 0, imp_fly_move },
	{ FRAME_swpout15,	imp_ai_fly, -52, 0, 54, NULL, 0, imp_fly_move },
};
const animmove_t imp_move_dive_out = ANIMMOVE(imp_frames_dive_out, imp_flyback);

// Imp hovering.
static const animframe_t imp_frames_hover1[] =
{
	{ FRAME_impfly1,	NULL, 0, 0, 0, imp_ai_hover,  2, imp_flap_noise },
	{ FRAME_impfly2,	NULL, 0, 0, 0, imp_ai_hover,  2, imp_check_dodge },
	{ FRAME_impfly3,	NULL, 0, 0, 0, imp_ai_hover,  1, NULL },
	{ FRAME_impfly4,	NULL, 0, 0, 0, imp_ai_hover,  1, NULL },
	{ FRAME_impfly5,	NULL, 0, 0, 0, imp_ai_hover, -1, imp_check_dodge },
	{ FRAME_impfly6,	NULL, 0, 0, 0, imp_ai_hover, -1, NULL },
	{ FRAME_impfly7,	NULL, 0, 0, 0, imp_ai_hover, -1, NULL },
	{ FRAME_impfly8,	NULL, 0, 0, 0, imp_ai_hover, -2, imp_check_dodge },
	{ FRAME_impfly9,	NULL, 0, 0, 0, imp_ai_hover, -2, NULL },
	{ FRAME_impfly10,	NULL, 0, 0, 0, imp_ai_hover, -2, NULL },
	{ FRAME_impfly11,	NULL, 0, 0, 0, imp_ai_hover, -2, imp_check_dodge },
	{ FRAME_impfly12,	NULL, 0, 0, 0, imp_ai_hover, -1, NULL },
	{ FRAME_impfly13,	NULL, 0, 0, 0, imp_ai_hover, -1, NULL },
	{ FRAME_impfly14,	NULL, 0, 0, 0, imp_ai_hover, -1, imp_check_dodge },
	{ FRAME_impfly15,	NULL, 0, 0, 0, imp_ai_hover,  1, NULL },
	{ FRAME_impfly16,	NULL, 0, 0, 0, imp_ai_hover,  1, NULL },
	{ FRAME_impfly17,	NULL, 0, 0, 0, imp_ai_hover,  2, imp_check_dodge },
	{ FRAME_impfly18,	NULL, 0, 0, 0, imp_ai_hover,  2, NULL },
	{ FRAME_impfly19,	NULL, 0, 0, 0, imp_ai_hover,  2, NULL },
	{ FRAME_impfly20,	NULL, 0, 0, 0, imp_ai_hover,  2, imp_check_dodge },
};
const animmove_t imp_move_hover1 = ANIMMOVE(imp_frames_hover1, imp_hover_move);

// Imp tumble.
static const animframe_t imp_frames_tumble[] =
{
	{ FRAME_impfly1,	NULL, 0, 0, 0, imp_ai_hover,  2, imp_tumble_move },
	{ FRAME_impfly2,	NULL, 0, 0, 0, imp_ai_hover,  2, imp_tumble_move },
	{ FRAME_impfly3,	NULL, 0, 0, 0, imp_ai_hover,  1, imp_tumble_move },
	{ FRAME_impfly4,	NULL, 0, 0, 0, imp_ai_hover,  1, imp_tumble_move },
	{ FRAME_impfly5,	NULL, 0, 0, 0, imp_ai_hover, -1, imp_tumble_move },
	{ FRAME_impfly6,	NULL, 0, 0, 0, imp_ai_hover, -1, imp_tumble_move },
	{ FRAME_impfly7,	NULL, 0, 0, 0, imp_ai_hover, -1, imp_tumble_move },
	{ FRAME_impfly8,	NULL, 0, 0, 0, imp_ai_hover, -1, imp_tumble_move },
	{ FRAME_impfly9,	NULL, 0, 0, 0, imp_ai_hover, -2, imp_tumble_move },
	{ FRAME_impfly10,	NULL, 0, 0, 0, imp_ai_hover, -2, imp_tumble_move },
	{ FRAME_impfly11,	NULL, 0, 0, 0, imp_ai_hover, -2, imp_tumble_move },
	{ FRAME_impfly12,	NULL, 0, 0, 0, imp_ai_hover, -2, imp_tumble_move },
	{ FRAME_impfly13,	NULL, 0, 0, 0, imp_ai_hover, -1, imp_tumble_move },
	{ FRAME_impfly14,	NULL, 0, 0, 0, imp_ai_hover, -1, imp_tumble_move },
	{ FRAME_impfly15,	NULL, 0, 0, 0, imp_ai_hover, -1, imp_tumble_move },
	{ FRAME_impfly16,	NULL, 0, 0, 0, imp_ai_hover, -1, imp_tumble_move },
	{ FRAME_impfly17,	NULL, 0, 0, 0, imp_ai_hover,  1, imp_tumble_move },
	{ FRAME_impfly18,	NULL, 0, 0, 0, imp_ai_hover,  1, imp_tumble_move },
	{ FRAME_impfly19,	NULL, 0, 0, 0, imp_ai_hover,  2, imp_tumble_move },
	{ FRAME_impfly20,	NULL, 0, 0, 0, imp_ai_hover,  2, imp_tumble_move },
};
const animmove_t imp_move_tumble = ANIMMOVE(imp_frames_tumble, NULL);

// Imp firing fireball.
static const animframe_t imp_frames_fireball[] =
{
	{ FRAME_impfir1,	NULL, 0, 0, 0, imp_ai_hover,  2, imp_flap_noise },
	{ FRAME_impfir2,	NULL, 0, 0, 0, imp_ai_hover,  1, NULL },
	{ FRAME_impfir3,	NULL, 0, 0, 0, imp_ai_hover, -1, NULL },
	{ FRAME_impfir4,	NULL, 0, 0, 0, imp_ai_hover, -2, imp_check_dodge },
	{ FRAME_impfir5,	NULL, 0, 0, 0, imp_ai_hover, -2, NULL },
	{ FRAME_impfir6,	NULL, 0, 0, 0, imp_ai_hover, -1, NULL },
	{ FRAME_impfir7,	NULL, 0, 0, 0, imp_ai_hover,  1, imp_check_dodge },
	{ FRAME_impfir8,	NULL, 0, 0, 0, imp_ai_hover,  2, NULL },
	{ FRAME_impfir9,	NULL, 0, 0, 0, imp_ai_hover,  2, NULL },
	{ FRAME_impfir10,	NULL, 0, 0, 0, imp_ai_hover,  1, NULL },
	{ FRAME_impfir11,	NULL, 0, 0, 0, imp_ai_hover, -1, imp_check_dodge },
	{ FRAME_impfir12,	NULL, 0, 0, 0, imp_ai_hover, -2, NULL },
	{ FRAME_impfir13,	NULL, 0, 0, 0, imp_ai_hover, -2, NULL },
	{ FRAME_impfir14,	NULL, 0, 0, 0, imp_ai_hover, -1, NULL },
	{ FRAME_impfir15,	NULL, 0, 0, 0, imp_ai_hover,  1, NULL },
	{ FRAME_impfir16,	NULL, 0, 0, 0, imp_ai_hover,  2, imp_fireball },
	{ FRAME_impfir15,	NULL, 0, 0, 0, imp_ai_hover, -2, NULL },
	{ FRAME_impfir16,	NULL, 0, 0, 0, imp_ai_hover, -1, NULL },
	{ FRAME_impfir17,	NULL, 0, 0, 0, imp_ai_hover,  1, NULL },
	{ FRAME_impfir18,	NULL, 0, 0, 0, imp_ai_hover,  2, NULL },
	{ FRAME_impfir19,	NULL, 0, 0, 0, imp_ai_hover,  2, imp_check_dodge },
	{ FRAME_impfir20,	NULL, 0, 0, 0, imp_ai_hover,  1, NULL },
	{ FRAME_impfir21,	NULL, 0, 0, 0, imp_ai_hover, -1, NULL },
};
const animmove_t imp_move_fireball = ANIMMOVE(imp_frames_fireball, imp_hover_move);

// Imp diving and attacking.
static const animframe_t imp_frames_dive_go[] =
{
	{ FRAME_swoop1,		NULL, 0, 0, 0, NULL,	0, imp_dive_noise },
	{ FRAME_swoop2,		NULL, 0, 0, 0, NULL,	0, NULL },
	{ FRAME_swoop3,		NULL, 0, 0, 0, NULL,	0, NULL },
	{ FRAME_swoop4,		NULL, 0, 0, 0, NULL,	0, NULL },
	{ FRAME_swoop5,		NULL, 0, 0, 0, NULL,	0, imp_dive_move },
	{ FRAME_swoop6,		NULL, 0, 0, 0, NULL,	0, imp_dive_move },
	{ FRAME_swoop7,		NULL, 0, 0, 0, NULL,	0, imp_dive_move },
	{ FRAME_swoop8,		NULL, 0, 0, 0, NULL,	0, imp_dive_move },
	{ FRAME_swoop9,		NULL, 0, 0, 0, NULL,	0, imp_dive_move },
	{ FRAME_swoop10,	NULL, 0, 0, 0, NULL,	0, imp_dive_move },
	{ FRAME_swoop11,	NULL, 0, 0, 0, NULL,	0, imp_dive_move },
	{ FRAME_swoop12,	NULL, 0, 0, 0, NULL,	0, imp_dive_move },
	{ FRAME_swoop13,	NULL, 0, 0, 0, NULL,	0, imp_dive_move },
	{ FRAME_swoop14,	NULL, 0, 0, 0, NULL,	0, imp_dive_move },
	{ FRAME_swoop15,	NULL, 0, 0, 0, NULL,	0, imp_dive_move },
	{ FRAME_swoop16,	NULL, 0, 0, 0, imp_hit,	0, imp_dive_move },
	{ FRAME_swoop17,	NULL, 0, 0, 0, imp_hit,	0, imp_dive_move },
	{ FRAME_swoop18,	NULL, 0, 0, 0, imp_hit,	0, imp_dive_move },
	{ FRAME_swoop19,	NULL, 0, 0, 0, imp_hit,	0, imp_dive_move },
	{ FRAME_swoop20,	NULL, 0, 0, 0, imp_hit,	0, imp_dive_move },
};
const animmove_t imp_move_dive_go = ANIMMOVE(imp_frames_dive_go, imp_dive_loop);

// Imp diving loop.
static const animframe_t imp_frames_dive_loop[] =
{
	{ FRAME_swpcyc1,	NULL, 0, 0, 0, imp_hit, 0, imp_dive_move },
	{ FRAME_swpcyc2,	NULL, 0, 0, 0, imp_hit, 0, imp_dive_move },
	{ FRAME_swpcyc3,	NULL, 0, 0, 0, imp_hit, 0, imp_dive_move },
	{ FRAME_swpcyc4,	NULL, 0, 0, 0, imp_hit, 0, imp_dive_move },
	{ FRAME_swpcyc1,	NULL, 0, 0, 0, imp_hit, 0, imp_dive_move },
	{ FRAME_swpcyc2,	NULL, 0, 0, 0, imp_hit, 0, imp_dive_move },
	{ FRAME_swpcyc3,	NULL, 0, 0, 0, imp_hit, 0, imp_dive_move },
	{ FRAME_swpcyc4,	NULL, 0, 0, 0, imp_hit, 0, imp_dive_move },
	{ FRAME_swpcyc1,	NULL, 0, 0, 0, imp_hit, 0, imp_dive_move },
	{ FRAME_swpcyc2,	NULL, 0, 0, 0, imp_hit, 0, imp_dive_move },
	{ FRAME_swpcyc3,	NULL, 0, 0, 0, imp_hit, 0, imp_dive_move },
	{ FRAME_swpcyc4,	NULL, 0, 0, 0, imp_hit, 0, imp_dive_move },
	{ FRAME_swpcyc1,	NULL, 0, 0, 0, imp_hit, 0, imp_dive_move },
	{ FRAME_swpcyc2,	NULL, 0, 0, 0, imp_hit, 0, imp_dive_move },
	{ FRAME_swpcyc3,	NULL, 0, 0, 0, imp_hit, 0, imp_dive_move },
	{ FRAME_swpcyc4,	NULL, 0, 0, 0, imp_hit, 0, imp_dive_move },
	{ FRAME_swpcyc1,	NULL, 0, 0, 0, imp_hit, 0, imp_dive_move },
	{ FRAME_swpcyc2,	NULL, 0, 0, 0, imp_hit, 0, imp_dive_move },
	{ FRAME_swpcyc3,	NULL, 0, 0, 0, imp_hit, 0, imp_dive_move },
	{ FRAME_swpcyc4,	NULL, 0, 0, 0, imp_hit, 0, imp_dive_move },
	{ FRAME_swpcyc1,	NULL, 0, 0, 0, imp_hit, 0, imp_dive_move },
	{ FRAME_swpcyc2,	NULL, 0, 0, 0, imp_hit, 0, imp_dive_move },
	{ FRAME_swpcyc3,	NULL, 0, 0, 0, imp_hit, 0, imp_dive_move },
	{ FRAME_swpcyc4,	NULL, 0, 0, 0, imp_hit, 1, imp_dive_move },
};
const animmove_t imp_move_dive_loop = ANIMMOVE(imp_frames_dive_loop, NULL);

// Imp pain.
static const animframe_t imp_frames_pain1[] =
{
	{ FRAME_death1,		NULL, 0, 0, 0, NULL,  2, imp_hover_move },
	{ FRAME_death2,		NULL, 0, 0, 0, NULL,  1, imp_hover_move },
	{ FRAME_death3,		NULL, 0, 0, 0, NULL, -1, imp_hover_move },
	{ FRAME_death2,		NULL, 0, 0, 0, NULL, -2, imp_hover_move },
	{ FRAME_death1,		NULL, 0, 0, 0, NULL, -2, imp_hover_move },
};
const animmove_t imp_move_pain1 = ANIMMOVE(imp_frames_pain1, imp_pause);

// Imp perch.
static const animframe_t imp_frames_perch[] =
{
	{ FRAME_impwat1,	NULL, 0, 0, 0, NULL, 0, imp_ai_perch },
	{ FRAME_impwat2,	NULL, 0, 0, 0, NULL, 0, imp_ai_perch },
	{ FRAME_impwat3,	NULL, 0, 0, 0, NULL, 0, imp_ai_perch },
	{ FRAME_impwat4,	NULL, 0, 0, 0, NULL, 0, imp_ai_perch },
	{ FRAME_impwat5,	NULL, 0, 0, 0, NULL, 0, imp_ai_perch },
	{ FRAME_impwat6,	NULL, 0, 0, 0, NULL, 0, imp_ai_perch },
	{ FRAME_impwat7,	NULL, 0, 0, 0, NULL, 0, imp_ai_perch },
	{ FRAME_impwat8,	NULL, 0, 0, 0, NULL, 0, imp_ai_perch },
	{ FRAME_impwat9,	NULL, 0, 0, 0, NULL, 0, imp_ai_perch },
	{ FRAME_impwat10,	NULL, 0, 0, 0, NULL, 0, imp_ai_perch },
	{ FRAME_impwat11,	NULL, 0, 0, 0, NULL, 0, imp_ai_perch },
	{ FRAME_impwat12,	NULL, 0, 0, 0, NULL, 0, imp_ai_perch },
	{ FRAME_impwat13,	NULL, 0, 0, 0, NULL, 0, imp_ai_perch },
	{ FRAME_impwat14,	NULL, 0, 0, 0, NULL, 0, imp_ai_perch },
	{ FRAME_impwat15,	NULL, 0, 0, 0, NULL, 0, imp_ai_perch },
	{ FRAME_impwat16,	NULL, 0, 0, 0, NULL, 0, imp_ai_perch },
	{ FRAME_impwat17,	NULL, 0, 0, 0, NULL, 0, imp_ai_perch },
	{ FRAME_impwat18,	NULL, 0, 0, 0, NULL, 0, imp_ai_perch },
	{ FRAME_impwat19,	NULL, 0, 0, 0, NULL, 0, imp_ai_perch },
	{ FRAME_impwat20,	NULL, 0, 0, 0, NULL, 0, imp_ai_perch },
	{ FRAME_impwat21,	NULL, 0, 0, 0, NULL, 0, imp_ai_perch },
	{ FRAME_impwat22,	NULL, 0, 0, 0, NULL, 0, imp_ai_perch },
	{ FRAME_impwat23,	NULL, 0, 0, 0, NULL, 0, imp_ai_perch },
	{ FRAME_impwat24,	NULL, 0, 0, 0, NULL, 0, imp_ai_perch },
};
const animmove_t imp_move_perch = ANIMMOVE(imp_frames_perch, NULL);

// Imp take off.
static const animframe_t imp_frames_takeoff[] =
{
	{ FRAME_impup1,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_impup2,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_impup3,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_impup4,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_impup5,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_impup6,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_impup7,		imp_ai_fly, 16, 0, 16, NULL, 0, imp_flap_noise },
	{ FRAME_impup8,		imp_ai_fly, 18, 0, 16, NULL, 0, NULL },
	{ FRAME_impup9,		imp_ai_fly, 24, 0, 32, NULL, 0, NULL },
	{ FRAME_impup10,	imp_ai_fly, 36, 0, 64, NULL, 0, NULL },
	{ FRAME_impup11,	imp_ai_fly, 24, 0, 32, NULL, 0, NULL },
	{ FRAME_impup12,	imp_ai_fly, 18, 0, 16, NULL, 0, NULL },
	{ FRAME_impup13,	imp_ai_fly, 16, 0, 32, NULL, 0, NULL },
	{ FRAME_impup14,	imp_ai_fly, 12, 0, 64, NULL, 0, NULL },
	{ FRAME_impup15,	imp_ai_fly,  8, 0, 32, NULL, 0, NULL },
	{ FRAME_impup16,	imp_ai_fly,  6, 0, 32, NULL, 0, NULL },
	{ FRAME_impup17,	imp_ai_fly,  4, 0, 16, NULL, 0, imp_flap_noise },
	{ FRAME_impup18,	imp_ai_fly,  2, 0, 16, NULL, 0, NULL },
	{ FRAME_impup19,	imp_ai_fly, -2, 0, 32, NULL, 0, NULL },
	{ FRAME_impup20,	imp_ai_fly, -4, 0, 64, NULL, 0, NULL },
	{ FRAME_impup21,	imp_ai_fly, -6, 0, 32, NULL, 0, NULL },
	{ FRAME_impup22,	imp_ai_fly, -8, 0, 16, NULL, 0, NULL },
	{ FRAME_impup23,	imp_ai_fly, -16,0, 32, NULL, 0, NULL },
};
const animmove_t imp_move_takeoff = ANIMMOVE(imp_frames_takeoff, imp_pause);

// Imp dive up.
static const animframe_t imp_frames_dup[] =
{
	{ FRAME_swpend6,	imp_ai_fly, -36, 0, 24, NULL, 0, imp_fly_move },
	{ FRAME_swpend7,	imp_ai_fly, -32, 0, 16, NULL, 0, imp_flap_noise },
	{ FRAME_swpend8,	imp_ai_fly, -64, 0, 64, NULL, 0, imp_fly_move },
	{ FRAME_swpend9,	imp_ai_fly, -52, 0, 54, NULL, 0, imp_fly_move },
	{ FRAME_swpend10,	imp_ai_fly, -48, 0, 48, NULL, 0, imp_fly_move },
	{ FRAME_swpend11,	imp_ai_fly, -42, 0, 32, NULL, 0, imp_fly_move },
	{ FRAME_swpend12,	imp_ai_fly, -36, 0, 24, NULL, 0, imp_fly_move },
	{ FRAME_swpend13,	imp_ai_fly, -32, 0, 16, NULL, 0, imp_flap_noise },
	{ FRAME_swpend14,	imp_ai_fly, -64, 0, 64, NULL, 0, imp_fly_move },
	{ FRAME_swpend15,	imp_ai_fly, -52, 0, 54, NULL, 0, imp_fly_move },
};
const animmove_t imp_move_dup = ANIMMOVE(imp_frames_dup, imp_pause);

// Imp dive down.
static const animframe_t imp_frames_ddown[] =
{
	{ FRAME_swoop1,		NULL, 0, 0, 0, NULL, 0, imp_dive_noise },
	{ FRAME_swoop2,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_swoop4,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_swoop5,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_swoop7,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_swoop10,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_swoop14,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_swoop17,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_swoop20,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_swpout1,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_swpout4,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_swpout6,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_swpout9,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_swpout12,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_swpout15,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t imp_move_ddown = ANIMMOVE(imp_frames_ddown , imp_pause);