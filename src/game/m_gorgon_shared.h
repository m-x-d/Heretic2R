//
// m_gorgon_shared.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "g_Edict.h" //mxd

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

extern void gorgon_bite(edict_t* self);
extern void gorgon_footstep(edict_t* self);
extern void gorgon_dead(edict_t* self);
extern void gorgon_death1_fall(edict_t* self); //mxd
extern void gorgon_death2(edict_t* self); //mxd
extern void gorgon_death2_throw(edict_t* self); //mxd
extern void gorgon_death2_slide(edict_t* self); //mxd
extern void gorgon_start_twitch(edict_t* self); //mxd
extern void gorgon_next_twitch(edict_t* self); //mxd
extern void gorgon_hop(edict_t* self);
extern void gorgon_growl(edict_t* self);
extern void gorgon_jump(edict_t* self);
extern void gorgon_land(edict_t* self); //mxd
extern void gorgon_ready_catch(edict_t* self);
extern void gorgon_throw_toy(edict_t* self);
extern void gorgon_shake_toy(edict_t* self, float forward_offset, float right_offset, float up_offset);
extern void gorgon_check_snatch(edict_t* self, float forward_offset, float right_offset, float up_offset);
extern void gorgon_gore_toy(edict_t* self, float jump_height);
extern void gorgon_miss_sound(edict_t* self);
extern void gorgon_anger_sound(edict_t* self);
extern void gorgon_snatch_go(edict_t* self);
extern void gorgon_done_gore(edict_t* self);
extern void gorgon_set_roll(edict_t* self, float roll_angle);
extern void gorgon_lerp_off(edict_t* self);
extern void gorgon_lerp_on(edict_t* self);
extern void gorgon_check_slip(edict_t* self);
extern void gorgon_slide(edict_t* self, float force);
extern void gorgon_check_mood(edict_t* self);
extern void gorgon_apply_jump(edict_t* self);
extern void gorgon_roar(edict_t* self);
extern void gorgon_roar_sound(edict_t* self);
extern void gorgon_inair_go(edict_t* self);
extern void gorgon_check_landed(edict_t* self);
extern void gorgon_jump_out_of_water(edict_t* self);
extern void gorgon_swim_go(edict_t* self);
extern void gorgon_check_in_water(edict_t* self);
extern void gorgon_ai_run(edict_t* self, float distance); //mxd
extern void gorgon_ai_swim(edict_t* self, float distance);
extern void gorgon_forward(edict_t* self, float dist);
extern void gorgon_fix_pitch(edict_t* self);
extern void gorgon_reset_pitch(edict_t* self);
extern void gorgon_ai_eat(edict_t* self, float switch_animation);
extern void gorgon_under_water_wake(edict_t* self); //mxd
extern void gorgon_melee5check(edict_t* self); //mxd
extern void gorgon_ai_charge2(edict_t* self, float distance); //mxd