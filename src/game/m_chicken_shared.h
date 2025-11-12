//
// m_chicken_shared.h -- Data and function declarations shared between m_chicken.c and m_chicken_anim.c.
//
// Copyright 1998 Raven Software
//

#pragma once

#include "g_Edict.h"

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

extern void chicken_pause(edict_t* self);
extern void chicken_check_unmorph(edict_t* self);
extern void chicken_eat_again(edict_t* self);
extern void chicken_bite(edict_t* self);
extern void chicken_sound(edict_t* self, float channel, float sound_index, float attenuation);