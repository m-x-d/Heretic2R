//
// m_chicken_shared.h -- Data and function declarations shared between m_chicken.c and m_chicken_anim.c.
//
// Copyright 1998 Raven Software
//

#pragma once

#include "g_local.h" //mxd

typedef enum AnimID_e
{
	ANIM_STAND1,
	ANIM_WALK,
	ANIM_RUN,
	ANIM_CLUCK,
	ANIM_ATTACK,
	ANIM_EAT,
	ANIM_JUMP,

	NUM_ANIMS
} AnimID_t;

typedef enum SoundID_e
{
	// For the cluck animation.
	SND_CLUCK1,
	SND_CLUCK2,

	// For the foot falls.
	SND_CLAW,

	// For getting hit - even though right now, it dies immediately - they want this changed.
	SND_PAIN1, //TODO: unused.
	SND_PAIN2, //TODO: unused.

	// For dying - we only ever get gibbed, so no other sound is required.
	SND_DIE,

	// For biting the player.
	SND_BITE1,
	SND_BITE2,
	SND_BITE3, //TODO: unused, no sound file.

	// For pecking the ground.
	SND_PECK1,
	SND_PECK2,

	// And lastly, I thought it might be cool to have some cries for when the chicken jumps.
	SND_JUMP1,
	SND_JUMP2,
	SND_JUMP3,

	NUM_SOUNDS
} SoundID_t;

extern const animmove_t chicken_move_stand1;
extern const animmove_t chicken_move_walk;
extern const animmove_t chicken_move_run;
extern const animmove_t chicken_move_cluck;
extern const animmove_t chicken_move_attack;
extern const animmove_t chicken_move_eat;
extern const animmove_t chicken_move_jump;

// Dummy animmoves to catch sequence leaks.
extern const animmove_t chickenp_move_dummy;

extern const animmove_t chickenp_move_stand;
extern const animmove_t chickenp_move_stand1;
extern const animmove_t chickenp_move_stand2;
extern const animmove_t chickenp_move_walk;
extern const animmove_t chickenp_move_run;
extern const animmove_t chickenp_move_back;
extern const animmove_t chickenp_move_runb;
extern const animmove_t chickenp_move_bite;
extern const animmove_t chickenp_move_strafel;
extern const animmove_t chickenp_move_strafer;
extern const animmove_t chickenp_move_jump;
extern const animmove_t chickenp_move_wjump;
extern const animmove_t chickenp_move_wjumpb;
extern const animmove_t chickenp_move_rjump;
extern const animmove_t chickenp_move_rjumpb;
extern const animmove_t chickenp_move_jump_loop;
extern const animmove_t chickenp_move_attack;

void chicken_pause(edict_t* self);
void chicken_check_unmorph(edict_t* self);
void chicken_eat_again(edict_t* self);
void chicken_bite(edict_t* self);
void chicken_sound(edict_t* self, float channel, float sound_index, float attenuation);