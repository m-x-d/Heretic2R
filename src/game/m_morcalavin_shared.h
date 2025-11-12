//
// m_morcalavin_shared.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "g_Edict.h"

typedef enum AnimID_e
{
	ANIM_FLOAT,
	ANIM_HURTIDLE,
	ANIM_ATTACK1,
	ANIM_ATTACK2,
	ANIM_ATTACK2B,
	ANIM_ATTACK3,
	ANIM_DEF1, //TODO: unused.
	ANIM_DEF2, //TODO: unused.
	ANIM_WALK,
	ANIM_FLY,
	ANIM_GETUP,
	ANIM_RETORT,
	ANIM_FALL,
	ANIM_GLIDE,
	ANIM_GROUND_ATTACK,
	ANIM_TRACKING1,
	ANIM_ATTACK4,

	NUM_ANIMS
} AnimID_t;

typedef enum SoundID_e
{
	// Quake attack.
	SND_QUAKE, //TODO: unused. CLFX FX_QUAKE_RING plays the sound.

	// Straight-fire beam.
	SND_BEAM, //TODO: unused.
	SND_BEAMHIT, //TODO: unused.

	// Homing balls.
	SND_HOMING,
	SND_HOMEHIT, //TODO: unused.

	// Power Puff.
	SND_PPCHARGE,
	SND_PPFIRE,
	SND_PPEXPLODE, //TODO: unused.

	// Lightning from eyes.
	SND_LIGHTNING, //TODO: unused.
	SND_LGHTNGHIT, //TODO: unused.

	//Shove.
	SND_FORCEWALL, //TODO: unused.

	// Shield.
	SND_MAKESHIELD,
	SND_SHIELDHIT, //TODO: unused.
	SND_SHIELDPULSE, //TODO: unused.
	SND_SHIELDGONE, //TODO: unused.
	SND_SHIELDBREAK, //TODO: unused.

	// Fly forward.
	SND_RUSH,

	// Hurt and get up.
	SND_FALL,
	SND_REVIVE,

	// Strafing beams attack.
	SND_STRAFEON, //TODO: unused.
	SND_STRFSWNG, //TODO: unused.
	SND_STRAFEOFF, //TODO: unused.

	// Hurt/kill player laugh.
	SND_LAUGH,

	//Taunt sounds.
	TAUNT_LAUGH1,
	TAUNT_LAUGH2,
	TAUNT_LAUGH3,
	TAUNT_LAUGH4,

	TAUNT_BELLY1,
	TAUNT_BELLY2,
	TAUNT_BELLY3,

	NUM_SOUNDS
} SoundID_t;

extern void morcalavin_pause(edict_t* self);
extern void morcalavin_retort(edict_t* self);
extern void morcalavin_get_up(edict_t* self);
extern void morcalavin_hurt_idle(edict_t* self);
extern void morcalavin_quake(edict_t* self, float pitch_ofs, float yaw_ofs, float roll_ofs);
extern void morcalavin_beam(edict_t* self);
extern void morcalavin_beam2(edict_t* self);
extern void morcalavin_rush_sound(edict_t* self);
extern void morcalavin_ai_run(edict_t* self, float distance);
extern void morcalavin_ai_hover(edict_t* self, float distance);

extern void morcalavin_quake_pause(edict_t* self);
extern void morcalavin_fade_out(edict_t* self);
extern void morcalavin_taunt_shot(edict_t* self);

extern void morcalavin_start_missile(edict_t* self);
extern void morcalavin_release_missile(edict_t* self);
extern void morcalavin_tracking_projectile(edict_t* self, float pitch, float yaw, float roll);
extern void morcalavin_big_shot(edict_t* self);
extern void morcalavin_end_retort(edict_t* self);