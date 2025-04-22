//
// m_morcalavin_shared.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "g_local.h"

typedef enum AnimID_e
{
	ANIM_FLOAT,
	ANIM_HURTIDLE,
	ANIM_ATTACK1,
	ANIM_ATTACK2,
	ANIM_ATTACK2B,
	ANIM_ATTACK3,
	ANIM_DEF1,
	ANIM_DEF2,
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

extern const animmove_t morcalavin_move_float;
extern const animmove_t morcalavin_move_hurtidle;
extern const animmove_t morcalavin_move_attack1;
extern const animmove_t morcalavin_move_attack2;
extern const animmove_t morcalavin_move_attack2b;
extern const animmove_t morcalavin_move_attack3;
extern const animmove_t morcalavin_move_attack4;
extern const animmove_t morcalavin_move_def1;
extern const animmove_t morcalavin_move_def2;
extern const animmove_t morcalavin_move_walk;
extern const animmove_t morcalavin_move_fly;
extern const animmove_t morcalavin_move_getup;
extern const animmove_t morcalavin_move_retort;
extern const animmove_t morcalavin_move_fall;
extern const animmove_t morcalavin_move_glide;
extern const animmove_t morcalavin_move_ground_attack;
extern const animmove_t morcalavin_move_tracking_attack1;

void morcalavin_pause(edict_t* self);
void morcalavin_retort(edict_t* self);
void morcalavin_get_up(edict_t* self);
void morcalavin_hurtidle(edict_t* self);
void morcalavin_quake(edict_t* self, float pitch_ofs, float yaw_ofs, float roll_ofs);
void morcalavin_beam(edict_t* self);
void morcalavin_beam2(edict_t* self);
void morcalavin_rush_sound(edict_t* self);
void mork_ai_run(edict_t* self, float distance);
void mork_ai_hover(edict_t* self, float distance);

void morcalavin_quake_pause(edict_t* self);
void morcalavin_fade_out(edict_t* self);
void morcalavin_taunt_shot(edict_t* self);

void morcalavin_start_missile(edict_t* self);
void morcalavin_release_missile(edict_t* self);
void morcalavin_tracking_projectile(edict_t* self, float pitch, float yaw, float roll);
void morcalavin_big_shot(edict_t* self);
void morcalavin_end_retort(edict_t* self);