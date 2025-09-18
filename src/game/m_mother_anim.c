//
// m_mother_anim.c
//
// Copyright 1998 Raven Software
//

#include "m_mother_anim.h"
#include "m_mother_shared.h"

// Mother Pain.
static const animframe_t mother_frames_pain[] =
{
	{ FRAME_pain001, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pain003, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pain005, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pain007, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pain009, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pain011, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pain013, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pain015, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pain017, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pain019, NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t mother_move_pain = ANIMMOVE(mother_frames_pain, mother_pause);

// Mother Stand.
static const animframe_t mother_frames_stand[] =
{
	{ FRAME_poly000, NULL, 0, 0, 0, NULL, 0, mother_growl },
	{ FRAME_poly001, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly002, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly003, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly004, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly005, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly006, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly007, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly008, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly009, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly010, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly011, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly012, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly013, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly014, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly015, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly016, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly017, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly018, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly019, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly020, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly021, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly022, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly023, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly024, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly025, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly026, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly027, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly028, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly029, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly030, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly031, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly032, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly033, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly034, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly035, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly036, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly037, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly038, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly039, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly040, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly041, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly042, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly043, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly044, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly045, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly046, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly047, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly048, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly049, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly050, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly051, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly052, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly053, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly054, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly055, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly056, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly057, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly058, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly059, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_poly060, NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t mother_move_stand = ANIMMOVE(mother_frames_stand, mother_pause);