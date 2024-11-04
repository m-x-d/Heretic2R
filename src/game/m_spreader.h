/*-------------------------------------------------------------------------
m_spreader.h  

Copyright 1998 Raven Software 
All rights reserved    
-------------------------------------------------------------------------*/

#ifndef M_SPREADER_H
#define M_SPREADER_H

typedef enum AnimID_e
{
	ANIM_ATTACK1,
	ANIM_ATTACK2, 
	ANIM_BACKUP,
	ANIM_BACKATTACK,
	ANIM_DEATH1_GO,
	ANIM_DEATH1_LOOP,
	ANIM_DEATH1_END,
	ANIM_DUCK,
	ANIM_DUCKATTACK,
	ANIM_DUCKDOWN,
	ANIM_DUCKSTILL,
	ANIM_DUCKUP,
	ANIM_IDLE1,
	ANIM_PAIN1,
	ANIM_PIVOT_LEFT,
	ANIM_PIVOT_RIGHT,
	ANIM_RUNATTACK,
	ANIM_RUN1, 
	ANIM_LAND, 
	ANIM_INAIR, 
	ANIM_FJUMP, 
	ANIM_WALK1,
	ANIM_WALK2,
	ANIM_DEATH2,
	ANIM_FLY,
	ANIM_FLYLOOP,
	ANIM_FDIE,
	ANIM_DEAD,
	ANIM_DELAY,
	NUM_ANIMS
} AnimID_t;

typedef enum SoundID_e
{
	SND_SPRAYSTART,
	SND_SPRAYLOOP,
	SND_PAIN,
	SND_VOICE1, 
	SND_VOICE2, 
	SND_THROW, 
	SND_DEATH,
	SND_BOMB,
	NUM_SOUNDS
} SoundID_t;

#define BIT_PARENT		0
#define BIT_CHILD		1
#define BIT_BODY		2
#define BIT_BOMB		4
#define BIT_RITLEG		8
#define BIT_LFTARM		16
#define BIT_LFTLEG		32
#define BIT_HEAD		64
#define BIT_RITARM		128
#define BIT_TANK3		256
#define BIT_TANK2		512
#define BIT_TANK1		1024
#define BIT_HOSE		2048

static int Bit_for_MeshNode_ps [13] =
{
	BIT_PARENT,
	BIT_CHILD,	
	BIT_BODY,		
	BIT_BOMB,	
	BIT_RITLEG,
	BIT_LFTARM,	
	BIT_LFTLEG,	
	BIT_HEAD,		
	BIT_RITARM,	
	BIT_TANK3,		
	BIT_TANK2,		
	BIT_TANK1,		
	BIT_HOSE
};

extern animmove_t spreader_move_attack1;
extern animmove_t spreader_move_attack2;
extern animmove_t spreader_move_backup1;
extern animmove_t spreader_move_backattack1;
extern animmove_t spreader_move_death1_go;
extern animmove_t spreader_move_death1_loop;
extern animmove_t spreader_move_death1_end;
extern animmove_t spreader_move_duck1;	
extern animmove_t spreader_move_dkatck1;
extern animmove_t spreader_move_duckdown;
extern animmove_t spreader_move_duckstill;
extern animmove_t spreader_move_duckup;
extern animmove_t spreader_move_idle1;
extern animmove_t spreader_move_pain1;
extern animmove_t spreader_move_pvtlt1;
extern animmove_t spreader_move_pvtrt1;
extern animmove_t spreader_move_rnatck1;
extern animmove_t spreader_move_run1; 
extern animmove_t spreader_move_land; 
extern animmove_t spreader_move_inair; 
extern animmove_t spreader_move_fjump; 
extern animmove_t spreader_move_walk1;
extern animmove_t spreader_move_walk2;
extern animmove_t spreader_move_death2;
extern animmove_t spreader_move_flyloop;
extern animmove_t spreader_move_fly;
extern animmove_t spreader_move_fdie;
extern animmove_t spreader_move_dead;
extern animmove_t spreader_move_delay;

void spreader_mist(edict_t *self, float x, float y, float z);

void SpreaderStaticsInit();
void spreaderFly (edict_t *self);
void spreaderTakeOff (edict_t *self);
void spreaderFlyLoop (edict_t *self);
void spreader_dropweapon (edict_t *self);
void spreader_go_deadloop (edict_t *self);
void spreaderSolidAgain (edict_t *self);
void M_MoveFrame (edict_t *self);

void spreader_dismember(edict_t *self, int damage, int HitLocation);

void spreader_pause(edict_t *self);
void spreader_crouch(edict_t *self);
void spreader_uncrouch(edict_t *self);
void spreader_duckpause(edict_t *self);
void spreader_death(edict_t *self, G_Message_t *msg);
void spreader_dead(edict_t *self);

void spreader_flyback_move(edict_t *self);
void spreader_flyback_loop(edict_t *self);

void spreader_showgrenade(edict_t *self);
void spreader_pain_sound(edict_t *self);
void spreader_miststartsound(edict_t *self);
void spreader_miststopsound(edict_t *self);
void spreader_idlenoise(edict_t *self);
void spreader_hidegrenade(edict_t *self);
void spreaderApplyJump (edict_t *self);
void spreader_land(edict_t *self);
void spreader_jump (edict_t *self, G_Message_t *msg);
void spreader_go_inair (edict_t *self);
void MG_CheckLanded (edict_t *self, float next_anim);

edict_t *RadiusDamageEnt(edict_t *posowner, edict_t *dmgowner, int damage, float d_damage, float radius, float d_radius, int dflags, float lifetime, float thinkIncrement, vec3_t origin, vec3_t offset, float attach);

#endif //M_SPREADER_H
