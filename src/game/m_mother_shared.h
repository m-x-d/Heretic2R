//
// m_mother_shared.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "g_local.h"

typedef enum AnimID_e
{
	ANIM_PAIN,
	ANIM_STAND,

	NUM_ANIMS
} AnimID_t;

typedef enum SoundID_e
{
	SND_GROWL1,
	SND_GROWL2,
	SND_PAIN,
	SND_GIB,

	NUM_SOUNDS
} SoundID_t;

extern const animmove_t mother_move_pain;
extern const animmove_t mother_move_stand;

extern void mother_pause(edict_t* self);
extern void mother_growl(edict_t* self);