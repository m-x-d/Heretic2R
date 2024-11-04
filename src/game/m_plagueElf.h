#ifndef __PLAGUEELF__
#define __PLAGUEELF__

typedef enum AnimID_e
{	
	ANIM_STAND1,	
	ANIM_WALK1,
	ANIM_WALK2,
	ANIM_RUN1,	
	ANIM_RUNATK1,
	ANIM_FJUMP,
	ANIM_INAIR,
	ANIM_LAND,
	ANIM_MELEE1,
	ANIM_MELEE2,
	ANIM_DIE1,
	ANIM_DIE2,
	ANIM_DIE3,
	ANIM_DIE4,
	ANIM_FIST1,
	ANIM_LEAN1, 
	ANIM_SHAKE1, 	
	ANIM_PAIN1,
	ANIM_DELAY,
	ANIM_MISSILE,
	//kDEATH
	ANIM_KDEATH_GO,
	ANIM_KDEATH_LOOP,
	ANIM_KDEATH_END,
	//NEW ANIMS
	ANIM_CRAZY_A,//check
	ANIM_CRAZY_B,//check
	ANIM_CURSING,
	ANIM_POINT,
	ANIM_SCARED,//check

	ANIM_C_IDLE1,
	ANIM_C_IDLE2,
	ANIM_C_IDLE3,
	ANIM_C_WALK1,
	ANIM_C_WALK2,
	ANIM_C_RUN1,
	ANIM_C_ATTACK1,
	ANIM_C_ATTACK2,
	ANIM_C_ATTACK3,
	ANIM_C_ATTACK4,
	ANIM_C_PAIN1,
	ANIM_C_DEATH1,
	ANIM_C_DEATH2,
	ANIM_C_DEATH3,
	ANIM_C_DEATH4,
	ANIM_C_THINKAGAIN,

	NUM_ANIMS
} AnimID_t;

typedef enum SoundID_e
{
	SND_PAIN1,
	SND_PAIN2,
	SND_PAIN3,
	SND_DIE1,
	SND_DIE2,
	SND_DIE3,
	SND_GIB,
	SND_ATTACKHIT1,
	SND_ATTACKHIT2,
	SND_ATTACKMISS1,
	SND_MOAN1,
	SND_MOAN2, 
	SND_SHIVER,
	SND_PANT,
	SND_GASP,
	SND_SIGH,
	SND_ATTACK1,
	SND_ATTACK2,
	SND_ATTACK3,
	SND_DISMEMBER1,
	SND_DISMEMBER2,

	//Sight
	VOICE_FIRST_GROUP,
	VOICE_SIGHT_EAT_FLESH1,
	VOICE_SIGHT_GET_HIM1,
	VOICE_SIGHT_GET_HIM2,
	VOICE_SIGHT_GET_HIM3,
	VOICE_SIGHT_THERES_ONE,
	
	//Support
	VOICE_SUPPORT_LIVER,
	
	//Valid single sight
	VOICE_FIRST_ALONE,
	VOICE_MISC_DIE,
	VOICE_MISC_FLESH,
	VOICE_SUPPORT_GONNA_DIE1,
	VOICE_SUPPORT_YES,
	VOICE_LAST_GROUP,

	VOICE_MISC_LEAVE_ME1,
	VOICE_MISC_NO,
	
	NUM_SOUNDS
};

//Helper defines
#define FIRST_SIGHT_ALONE	VOICE_FIRST_ALONE + 1
#define LAST_SIGHT_ALONE	VOICE_LAST_GROUP - 1

#define FIRST_SIGHT_GROUP	VOICE_FIRST_GROUP + 1
#define LAST_SIGHT_GROUP	VOICE_LAST_GROUP - 1

#define NUM_VOICES	((NUM_SOUNDS - FIRST_SIGHT_GROUP) - 3)

enum
{
	FX_PE_MAKE_SPELL,
	FX_PE_EXPLODE_SPELL,
	FX_PE_MAKE_SPELL2,
	FX_PE_EXPLODE_SPELL2,
	FX_PE_MAKE_SPELL3,
	FX_PE_EXPLODE_SPELL3,
};

typedef struct pelfConversation_s
{
	float	responseDelay;
} pelfConversation_t;

extern animmove_t plagueElf_move_death1;
extern animmove_t plagueElf_move_death2;
extern animmove_t plagueElf_move_death3;
extern animmove_t plagueElf_move_death4;
extern animmove_t plagueElf_move_pain1;
extern animmove_t plagueElf_move_melee1;
extern animmove_t plagueElf_move_melee2;
extern animmove_t plagueElf_move_run1;
extern animmove_t plagueElf_move_runatk1;
extern animmove_t plagueElf_move_fjump;
extern animmove_t plagueElf_move_inair;
extern animmove_t plagueElf_move_land;
extern animmove_t plagueElf_move_walk1;
extern animmove_t plagueElf_move_walk2;
extern animmove_t plagueElf_move_stand1;
extern animmove_t plagueElf_fist1;
extern animmove_t plagueElf_lean1; 
extern animmove_t plagueElf_shake1; 
extern animmove_t plagueElf_delay; 
extern animmove_t plagueElf_move_missile;

extern animmove_t plagueElf_move_kdeath_go; 
extern animmove_t plagueElf_move_kdeath_loop; 
extern animmove_t plagueElf_move_kdeath_end; 

extern animmove_t plagueElf_crazy_A;
extern animmove_t plagueElf_crazy_B;
extern animmove_t plagueElf_cursing;
extern animmove_t plagueElf_point;
extern animmove_t plagueElf_scared;


extern animmove_t plagueElf_move_c_idle1; 
extern animmove_t plagueElf_move_c_idle2; 
extern animmove_t plagueElf_move_c_idle3; 
extern animmove_t plagueElf_move_c_walk; 
extern animmove_t plagueElf_move_c_walk2; 
extern animmove_t plagueElf_move_c_run; 
extern animmove_t plagueElf_move_c_attack1; 
extern animmove_t plagueElf_move_c_attack2; 
extern animmove_t plagueElf_move_c_attack3; 
extern animmove_t plagueElf_move_c_attack4; 
extern animmove_t plagueElf_move_c_pain1; 
extern animmove_t plagueElf_move_c_death1; 
extern animmove_t plagueElf_move_c_death2; 
extern animmove_t plagueElf_move_c_death3; 
extern animmove_t plagueElf_move_c_death4; 
void plagueElf_c_spell(edict_t *self);

void plagueElf_blocked(edict_t *self, G_Message_t *msg);
void plagueElf_death(edict_t *self, G_Message_t *msg);
void plagueElf_run(edict_t *self, G_Message_t *msg);
void plagueElf_walk(edict_t *self, G_Message_t *msg);
void plagueElf_melee(edict_t *self, G_Message_t *msg);
void plagueElf_stand(edict_t *self, G_Message_t *msg);
void plagueElf_pain(edict_t *self, G_Message_t *msg);
void plagueElf_dismember_msg(edict_t *self, G_Message_t *msg);
void plagueElf_dismember(edict_t *self, int	damage,	int HitLocation);

//void plagueElf_dead(edict_t *self);
void plagueElfdeathsqueal(edict_t *self);
void plagueElfsqueal(edict_t *self);
void plagueElfgrowl(edict_t *self);
void plagueElf_strike(edict_t *self);
void plagueElf_think_pain(edict_t *self);
void plagueElf_pause (edict_t *self);
qboolean plagueElf_dropweapon (edict_t *self, int damage);
void ai_charge2 (edict_t *self, float dist);
void plagueElfattack(edict_t *self);

void plagueelf_death_loop( edict_t *self );
void plagueelf_check_land( edict_t *self );
void plagueElf_spell(edict_t *self);
void plagueElfApplyJump(edict_t *self);

void SP_monster_plagueElf (edict_t *self);
void MG_InitMoods(edict_t *self);
qboolean ok_to_wake (edict_t *monster, qboolean gorgon_roar, qboolean ignore_ambush);

void plagueElfSpellTouch (edict_t *self, edict_t *Other, cplane_t *Plane, csurface_t *Surface);

//Voice support
void pelf_SightSound   ( edict_t *self, G_Message_t *msg );
void pelf_PollResponse ( edict_t *self, int sound_event, int sound_id, float time );
void pelf_EchoResponse ( edict_t *self, G_Message_t *msg );
void pelf_EchoSound	   ( edict_t *self, G_Message_t *msg );

void pelf_init_phase_out (edict_t *self);
void pelf_init_phase_in (edict_t *self);
void pelf_check_too_close(edict_t *self);
void pelf_land(edict_t *self);
void pelf_go_inair(edict_t *self);
void MG_CheckLanded (edict_t *self, float next_anim);
void plagueElf_go_run(edict_t *self);


#define BIT_BASE	0
#define BIT_HANDLE	1
#define BIT_HOE		2
#define BIT_GAFF	4
#define BIT_HAMMER	8
#define BIT_BODY	16
#define BIT_L_LEG	32
#define BIT_R_LEG	64
#define BIT_R_ARM	128
#define BIT_L_ARM	256
#define BIT_HEAD	512

#endif