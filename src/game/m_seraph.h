#include "g_local.h"

typedef enum AnimID_e
{
	ANIM_WALK1,
	ANIM_WALK2,
	ANIM_ATTACK1,
	ANIM_ATTACK1_LOOP,
	ANIM_ATTACK1_END,
	ANIM_STAND1,
	ANIM_STAND1_TR,
	ANIM_STAND1_R,
	ANIM_STAND1_TRC,
	ANIM_STAND1_TL,
	ANIM_STAND1_L,
	ANIM_STAND1_TLC,
	ANIM_POINT1,
	ANIM_RUN1,
	ANIM_FJUMP,
	ANIM_RUN1_WHIP,
	ANIM_PAIN,
	ANIM_SWIPE,
	ANIM_GET2WORK,
	ANIM_GET2WORK2,
	ANIM_STARTLE,
	ANIM_READY2IDLE,
	ANIM_BACKUP,
	ANIM_DEATH1,
	ANIM_DEATH2_GO,
	ANIM_DEATH2_LOOP,
	ANIM_DEATH2_END,
	ANIM_BACKUP2,

	NUM_ANIMS
} AnimID_t;

typedef enum SoundID_e
{
	SND_ATTACK,
	SND_SCOLD1,
	SND_SCOLD2,
	SND_SCOLD3,
	SND_STARTLE,
	SND_SLAP,
	SND_DEATH1,
	SND_DEATH2,
	SND_DEATH3,
	SND_DEATH4,
	SND_PAIN1,
	SND_PAIN2,
	SND_PAIN3,
	SND_PAIN4,
	SND_SCARE,
	SND_SIGHT1,
	SND_SIGHT2,
	SND_SIGHT3,
	NUM_SOUNDS
} SoundID_t;

extern animmove_t	seraph_move_walk1,
					seraph_move_walk2,
					seraph_move_whip1,
					seraph_move_whip1_loop,
					seraph_move_whip1_end,
					seraph_move_stand1,
					seraph_move_stand1_tr,
					seraph_move_stand1_r,
					seraph_move_stand1_trc,
					seraph_move_stand1_tl,
					seraph_move_stand1_l,
					seraph_move_stand1_tlc,
					seraph_move_point1,
					seraph_move_run1,
					seraph_move_fjump,
					seraph_move_run1_whip,
					seraph_move_pain,
					seraph_move_swipe,
					seraph_move_get2work,
					seraph_move_get2work2,
					seraph_move_startle,
					seraph_move_ready2idle,
					seraph_move_backup,
					seraph_move_death1,
					seraph_move_death2_go,
					seraph_move_death2_loop,
					seraph_move_death2_end,
					seraph_move_backup2;

void SP_monster_seraph_overlord(edict_t* self);

void seraph_done_startle(edict_t *self);
void seraph_done_get2work(edict_t *self);
void seraph_enforce_ogle(edict_t *self);
void seraph_ai_walk(edict_t *self, float dist);
void seraph_idle(edict_t *self);
void seraph_pause(edict_t *self);
void seraph_enforce(edict_t *self);
void seraph_strike(edict_t *self, float damage, float a, float b);

void seraph_dead ( edict_t *self );

void seraph_death_loop ( edict_t *self );
void seraph_check_land ( edict_t *self );

void seraph_sound_startle(edict_t *self);
void seraph_sound_slap(edict_t *self);
void seraph_sound_scold(edict_t *self);
void seraph_sound_yell(edict_t *self);
void seraph_sound_whip(edict_t *self);
void seraphApplyJump (edict_t *self);

void seraph_back (edict_t *self, float dist);
void seraph_sound_scold2(edict_t *self);
void seraph2idle (edict_t *self);

qboolean MG_BoolWalkMove (edict_t *self, float yaw, float dist);

#define BIT_BASEBIN		0
#define BIT_PITHEAD		1
#define BIT_SHOULDPAD	2
#define BIT_GUARDHEAD	4
#define BIT_LHANDGRD	8
#define BIT_LHANDBOSS	16
#define BIT_RHAND		32
#define BIT_FRTORSO		64
#define BIT_ARMSPIKES	128
#define BIT_LFTUPARM	256
#define BIT_RTLEG		512
#define BIT_RTARM		1024
#define BIT_LFTLEG		2048
#define BIT_BKTORSO		4096
#define BIT_AXE			8192
#define BIT_WHIP		16384