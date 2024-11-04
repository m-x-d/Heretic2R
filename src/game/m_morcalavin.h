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
//quake attack
	SND_QUAKE,
//straightt-fire beam
	SND_BEAM,
	SND_BEAMHIT,
//homing balls
	SND_HOMING,
	SND_HOMEHIT,
//power Puff
	SND_PPCHARGE,
	SND_PPFIRE,
	SND_PPEXPLODE,
//Lightning from eyes
	SND_LIGHTNING,
	SND_LGHTNGHIT,
//Shove
	SND_FORCEWALL,
//Shield
	SND_MAKESHIELD,
	SND_SHIELDHIT,
	SND_SHIELDPULSE,
	SND_SHIELDGONE,
	SND_SHIELDBREAK,
//Fly forward
	SND_RUSH,
//hurt and get up	
	SND_FALL,
	SND_REVIVE,
//strafing beams attack
	SND_STRAFEON,
	SND_STRFSWNG,
	SND_STRAFEOFF,
//hurt/kill player laugh
	SND_LAUGH,

//Taunt sounds
	TAUNT_LAUGH1,
	TAUNT_LAUGH2,
	TAUNT_LAUGH3,
	TAUNT_LAUGH4,

	TAUNT_BELLY1,
	TAUNT_BELLY2,
	TAUNT_BELLY3,
	
	NUM_SOUNDS
} SoundID_t;

extern animmove_t morcalavin_move_float,
					morcalavin_move_hurtidle,
					morcalavin_move_attack1,
					morcalavin_move_attack2,
					morcalavin_move_attack2b,
					morcalavin_move_attack3,
					morcalavin_move_def1,
					morcalavin_move_def2,
					morcalavin_move_walk,
					morcalavin_move_fly,
					morcalavin_move_getup,
					morcalavin_move_retort,
					morcalavin_move_fall,
					morcalavin_move_glide,
					morcalavin_move_ground_attack,
					morcalavin_move_tracking_attack1,
					morcalavin_move_attack4;

//void morcalavin_fire1( edict_t *self, float pitch_ofs, float yaw_ofs, float roll_ofs );
//void morcalavin_strike( edict_t *self, float damage );
void morcalavin_idle( edict_t *self );
void morcalavin_move( edict_t *self, float vf, float vr, float vu );
void morcalavin_pause( edict_t *self );
void morcalavin_retort( edict_t *self);
void morcalavin_getup( edict_t *self);
void morcalavin_hurtidle( edict_t *self);
//void morcalavin_shove (edict_t *self, float mindist, float maxdist, float throwproj);
//void morcalavin_summon_shield (edict_t *self);
//void morcalavin_lightning (edict_t *self);
//void morcalavin_powerpuff(edict_t *self);
//void morcalavin_startpuff (edict_t *self);
void morcalavin_quake( edict_t *self, float pitch_ofs, float yaw_ofs, float roll_ofs );
//void morcalavin_galaxy (edict_t *self);
void morcalavin_beam( edict_t *self);
void morcalavin_beam2( edict_t *self);
void morcalavin_rush_sound (edict_t *self);
void mork_ai_run (edict_t *self, float dist);
void mork_ai_hover (edict_t *self, float dist);

void morcalavin_ground_attack( edict_t *self );
void morcalavin_quake_pause( edict_t *self );
void morcalavin_init_phase_out (edict_t *self);
void MG_CheckEvade (edict_t *self);
void morcalavin_fade_out(edict_t *self);
void morcalavin_taunt_shot(edict_t *self);

void morcalavin_start_missile(edict_t *self);
void morcalavin_release_missile(edict_t *self);
void morcalavin_proj1_blocked( edict_t *self, trace_t *trace );
void morcalavin_tracking_projectile ( edict_t *self, float pitch, float yaw, float roll);
void morcalavin_big_shot( edict_t *self );
void morcalavin_check_lightning(edict_t *self);
void morcalavin_proj2_blocked( edict_t *self, trace_t *trace );
void morcalavin_attack_fade_out(edict_t *self);
void morcalavin_end_retort(edict_t *self);
void morcalavin_proj3_blocked( edict_t *self, trace_t *trace );

#define MORK_GRAV	0.3