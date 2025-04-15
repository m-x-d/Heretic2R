//
// m_gorgon_shared.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "g_local.h" //mxd

typedef enum AnimID_e
{
	ANIM_STAND1,
	ANIM_STAND2,
	ANIM_STAND3,
	ANIM_STAND4,

	ANIM_WALK1,
	ANIM_WALK2,
	ANIM_WALK3,

	ANIM_MELEE1,
	ANIM_MELEE2,
	ANIM_MELEE3,
	ANIM_MELEE4,
	ANIM_MELEE5,
	ANIM_MELEE6,
	ANIM_MELEE7,
	ANIM_MELEE8,
	ANIM_MELEE9,
	ANIM_MELEE10,

	ANIM_FJUMP,
	ANIM_RUN1,
	ANIM_RUN2,
	ANIM_RUN3,
	ANIM_PAIN1,
	ANIM_PAIN2,
	ANIM_PAIN3,
	ANIM_DIE1,
	ANIM_DIE2,
	ANIM_SNATCH,
	ANIM_CATCH,
	ANIM_MISS,
	ANIM_READY_CATCH,
	ANIM_SNATCHHI,
	ANIM_SNATCHLOW,
	ANIM_SLIP,
	ANIM_SLIP_PAIN,
	ANIM_DELAY,
	ANIM_ROAR,
	ANIM_ROAR2,
	ANIM_LAND2,
	ANIM_LAND,
	ANIM_INAIR,

	ANIM_TO_SWIM,
	ANIM_SWIM,
	ANIM_SWIM_BITE_A,
	ANIM_SWIM_BITE_B,
	ANIM_OUT_WATER,

	ANIM_EAT_DOWN,
	ANIM_EAT_UP,
	ANIM_EAT_LOOP,
	ANIM_EAT_TEAR,
	ANIM_EAT_PULLBACK,
	ANIM_LOOK_AROUND,
	ANIM_EAT_LEFT,
	ANIM_EAT_RIGHT,
	ANIM_EAT_SNAP,
	ANIM_EAT_REACT,

	NUM_ANIMS
} AnimID_t;

typedef enum SoundID_e
{
	SND_PAIN1,
	SND_PAIN2,
	SND_GURGLE, //TODO: unused.
	SND_DIE,
	SND_GIB,
	SND_MELEEHIT1,
	SND_MELEEHIT2,
	SND_MELEEMISS1,
	SND_MELEEMISS2,
	SND_STEP1,
	SND_STEP2,
	SND_STEP3,
	SND_STEP4,
	SND_GROWL1,
	SND_GROWL2,
	SND_GROWL3,
	SND_LAND,

	NUM_SOUNDS
} SoundID_t;

extern const animmove_t gorgon_move_stand1;
extern const animmove_t gorgon_move_stand2;
extern const animmove_t gorgon_move_stand3;
extern const animmove_t gorgon_move_stand4;
extern const animmove_t gorgon_move_walk;
extern const animmove_t gorgon_move_walk2;
extern const animmove_t gorgon_move_walk3;
extern const animmove_t gorgon_move_melee1;
extern const animmove_t gorgon_move_melee2;
extern const animmove_t gorgon_move_melee3;
extern const animmove_t gorgon_move_melee4;
extern const animmove_t gorgon_move_melee5;
extern const animmove_t gorgon_move_melee6;
extern const animmove_t gorgon_move_melee7;
extern const animmove_t gorgon_move_melee8;
extern const animmove_t gorgon_move_melee9;
extern const animmove_t gorgon_move_melee10;
extern const animmove_t gorgon_move_fjump;
extern const animmove_t gorgon_move_run1;
extern const animmove_t gorgon_move_run2;
extern const animmove_t gorgon_move_run3;
extern const animmove_t gorgon_move_pain1;
extern const animmove_t gorgon_move_pain2;
extern const animmove_t gorgon_move_pain3;
extern const animmove_t gorgon_move_die1;
extern const animmove_t gorgon_move_die2;
extern const animmove_t gorgon_move_catch;
extern const animmove_t gorgon_move_snatch;
extern const animmove_t gorgon_move_miss;
extern const animmove_t gorgon_move_readycatch;
extern const animmove_t gorgon_move_snatchhi;
extern const animmove_t gorgon_move_snatchlow;
extern const animmove_t gorgon_move_slip;
extern const animmove_t gorgon_move_slip_pain;
extern const animmove_t gorgon_move_delay;
extern const animmove_t gorgon_move_roar;
extern const animmove_t gorgon_move_roar2;
extern const animmove_t gorgon_move_land2;
extern const animmove_t gorgon_move_land;
extern const animmove_t gorgon_move_inair;

extern const animmove_t gorgon_move_to_swim;
extern const animmove_t gorgon_move_swim;
extern const animmove_t gorgon_move_swim_bite_a;
extern const animmove_t gorgon_move_swim_bite_b; //BUGFIX: mxd. gorgon_move_swim_bite_a in original logic.
extern const animmove_t gorgon_move_outwater;

extern const animmove_t gorgon_move_eat_down;
extern const animmove_t gorgon_move_eat_up;
extern const animmove_t gorgon_move_eat_loop;
extern const animmove_t gorgon_move_eat_tear;
extern const animmove_t gorgon_move_eat_pullback;
extern const animmove_t gorgon_move_look_around;
extern const animmove_t gorgon_move_eat_left;
extern const animmove_t gorgon_move_eat_right;
extern const animmove_t gorgon_move_eat_snap;
extern const animmove_t gorgon_move_eat_react;

void gorgon_bite(edict_t* self);
void gorgon_footstep(edict_t* self);
void gorgon_dead(edict_t* self);
void gorgon_hop(edict_t* self);
void gorgon_growl(edict_t* self);
void gorgon_jump(edict_t* self);
void gorgon_ready_catch(edict_t* self);
void gorgon_throw_toy(edict_t* self);
void gorgon_shake_toy(edict_t* self, float forward_offset, float right_offset, float up_offset);
void gorgon_check_snatch(edict_t* self, float forward_offset, float right_offset, float up_offset);
void gorgon_gore_toy(edict_t* self, float jump_height);
void gorgon_miss_sound(edict_t* self);
void gorgon_anger_sound(edict_t* self);
void gorgon_snatch_go(edict_t* self);
void gorgon_done_gore(edict_t* self);
void gorgon_set_roll(edict_t* self, float roll_angle);
void gorgonLerpOff(edict_t* self);
void gorgonLerpOn(edict_t* self);
void gorgonCheckSlip(edict_t* self);
void gorgonSlide(edict_t* self, float force);
void gorgon_check_mood(edict_t* self);
void gorgon_apply_jump(edict_t* self);
void gorgon_roar(edict_t* self);
void gorgon_roar_sound(edict_t* self);
void gorgon_inair_go(edict_t* self);
void gorgon_check_landed(edict_t* self);
void gorgon_jump_out_of_water(edict_t* self);
void gorgon_swim_go(edict_t* self);
void gorgon_check_in_water(edict_t* self);
void gorgon_ai_swim(edict_t* self, float distance);
void gorgon_forward(edict_t* self, float dist);
void gorgon_fix_pitch(edict_t* self);
void gorgon_reset_pitch(edict_t* self);
void gorgon_ai_eat(edict_t* self, float switch_animation);

//TODO: move to appropriate headers!
void fish_under_water_wake(edict_t* self);