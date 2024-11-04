typedef enum AnimID_e
{	
	ANIM_BACK,
	ANIM_DEATHFR,
	ANIM_IDLE,
	ANIM_LAND,
	ANIM_INAIR,
	ANIM_FORCED_JUMP,	
	ANIM_FINAIR,
	ANIM_FJUMP,	
	ANIM_PAINA,
	ANIM_PAINC,
	ANIM_RUN,
	ANIM_SPEAR,
	ANIM_SWORD,
	ANIM_SPELL,
	ANIM_SPELL2,
	ANIM_WALK,
	ANIM_DELAY,
	ANIM_KNOCK1_GO,
	ANIM_KNOCK1_LOOP,
	ANIM_KNOCK1_END,
	ANIM_TWITCH,


	ANIM_C_ACTION1,
	ANIM_C_ACTION2,
	ANIM_C_ACTION3,
	ANIM_C_ACTION4,
	ANIM_C_ATTACK1,
	ANIM_C_ATTACK2,
	ANIM_C_ATTACK3,
	ANIM_C_BACKPEDAL,
	ANIM_C_DEATH1,
	ANIM_C_IDLE1,
	ANIM_C_IDLE2,
	ANIM_C_IDLE3,
	ANIM_C_PAIN1,
	ANIM_C_RUN1,
	ANIM_C_WALK1,
	NUM_ANIMS
} AnimID_t;

typedef enum SoundID_e
{
	SND_PAINM,
	SND_PAINF,
	SND_DIEM,
	SND_DIEF,
	SND_GIB,
	SND_SWIPE,
	SND_SWIPEHITF,
	SND_SWIPEHITW,
	SND_SPELLM,
	SND_SPELLM2,
	SND_SPLPWRUPF,
	SND_SPELLF,
	SND_GROWLM1,
	SND_GROWLM2,
	SND_GROWLF1,
	SND_GROWLF2,
	SND_THUD,
	NUM_SOUNDS
} SoundID_t;

extern animmove_t insect_move_back;// = {10, insect_frames_back, insect_pause};
extern animmove_t insect_move_deathfr;// = {23, insect_frames_deathfr, insect_dead};
extern animmove_t insect_move_idle;// = { 50, insect_frames_idle, NULL};
extern animmove_t insect_move_land;// = {6, insect_frames_land, insect_pause};
extern animmove_t insect_move_inair;// = {1, insect_frames_inair, NULL};
extern animmove_t insect_move_forcedjump;// = {9, insect_frames_forcedjump, insect_go_inair};
extern animmove_t insect_move_finair;// = {1, insect_frames_inair, NULL};
extern animmove_t insect_move_fjump;// = {9, insect_frames_forcedjump, insect_go_inair};
extern animmove_t insect_move_paina;// = {7, insect_frames_paina, insect_pause};
extern animmove_t insect_move_painc;// = {6, insect_frames_painc, insect_pause};
extern animmove_t insect_move_run;// = {10, insect_frames_run, insect_pause};
extern animmove_t insect_move_spear;// = {11, insect_frames_spear, insect_pause};
extern animmove_t insect_move_sword;// = {9, insect_frames_sword, insect_pause};
extern animmove_t insect_move_spell;// = {8, insect_frames_spell, insect_pause};
extern animmove_t insect_move_spell2;// = {8, insect_frames_spell, insect_pause};
extern animmove_t insect_move_walk;// = {20, insect_frames_walk, insect_pause};
extern animmove_t insect_delay;// = {50 , insect_frames_delay, insect_pause};

extern animmove_t insect_move_c_action1;
extern animmove_t insect_move_c_action2;
extern animmove_t insect_move_c_action3;
extern animmove_t insect_move_c_action4;
extern animmove_t insect_move_c_idle1;
extern animmove_t insect_move_c_idle2;
extern animmove_t insect_move_c_idle3;
extern animmove_t insect_move_c_walk;
extern animmove_t insect_move_c_run;
extern animmove_t insect_move_c_backpedal;
extern animmove_t insect_move_c_attack1;
extern animmove_t insect_move_c_attack2;
extern animmove_t insect_move_c_attack3;
extern animmove_t insect_move_c_death1;
extern animmove_t insect_move_c_pain1;
extern animmove_t insect_move_c_pain2;
extern animmove_t insect_move_c_jump;
extern animmove_t insect_move_c_inair;
extern animmove_t insect_move_knock1_go;
extern animmove_t insect_move_knock1_loop;
extern animmove_t insect_move_knock1_end;
extern animmove_t insect_move_twitch;

extern void insect_c_walk(edict_t *self, G_Message_t *msg);
extern void insect_c_backpedal(edict_t *self, G_Message_t *msg);
extern void insect_c_action1(edict_t *self, G_Message_t *msg);
extern void insect_c_action2(edict_t *self, G_Message_t *msg);
extern void insect_c_action3(edict_t *self, G_Message_t *msg);
extern void insect_c_action4(edict_t *self, G_Message_t *msg);
extern void insect_c_idle1(edict_t *self, G_Message_t *msg);
extern void insect_c_idle2(edict_t *self, G_Message_t *msg);
extern void insect_c_idle3(edict_t *self, G_Message_t *msg);
extern void insect_c_attack1(edict_t *self, G_Message_t *msg);
extern void insect_c_attack2(edict_t *self, G_Message_t *msg);
extern void insect_c_attack3(edict_t *self, G_Message_t *msg);
extern void insect_c_death1(edict_t *self, G_Message_t *msg);
extern void insect_c_pain1(edict_t *self, G_Message_t *msg);
extern void insect_c_jump(edict_t *self, G_Message_t *msg);
extern void insect_c_pause (edict_t *self, G_Message_t *msg);
extern void insect_c_reallydead (edict_t *self);
extern void insect_c_go_inair (edict_t *self);

void MG_CheckLanded (edict_t *self, float next_anim);
void MG_InAirMove (edict_t *self, float fwdspd,float upspd,float rtspd);
void MG_ApplyJump (edict_t *self);

void SpellCastInsectStaff(edict_t *Caster,vec3_t StartPos,vec3_t AimAngles,vec3_t AimDir,qboolean power);
void SpellCastGlobeOfOuchiness(edict_t *Caster,vec3_t StartPos,vec3_t AimAngles,vec3_t AimDir);
void insectReleaseSpell (edict_t *self);
void insectReadySpell (edict_t *self);
void insectDoneSpell (edict_t *self);

void insectCut (edict_t *self, float attacktype);
void insectSpell(edict_t *self, float whichspell);
void insectStaff(edict_t *self);
void insectCheckLoop (edict_t *self, float frame);

void insect_blocked(edict_t *self, G_Message_t *msg);
void insect_death(edict_t *self, G_Message_t *msg);
void insect_run(edict_t *self, G_Message_t *msg);
void insect_walk(edict_t *self, G_Message_t *msg);
void insect_melee(edict_t *self, G_Message_t *msg);
void insect_stand(edict_t *self, G_Message_t *msg);
void insect_pain(edict_t *self, G_Message_t *msg);

void insect_dead(edict_t *self);
void insectdeathsqueal(edict_t *self);
void insectsqueal(edict_t *self);
void insectgrowl(edict_t *self);
void insectbite(edict_t *self);
void insect_think_pain(edict_t *self);
void insect_pause (edict_t *self);
void insect_dropweapon (edict_t *self, int weapon);
void insect_go_inair(edict_t *self);
void ai_charge2 (edict_t *self, float dist);
void insect_sound(edict_t *self, float channel, float soundnum, float attn);
void insect_wait_twitch (edict_t *self);
void insect_flyback_loop(edict_t *self);
void insect_flyback_move(edict_t *self);
void insect_random_idle_sound (edict_t *self);
void insect_go_finair(edict_t *self);

void SP_monster_insect (edict_t *self);
void MG_InitMoods(edict_t *self);


#define BIT_MASTER		0
#define BIT_LLEG		1
#define BIT_HEAD		2
#define BIT_LMANDIBLE	4
#define BIT_RMANDIBLE	8
#define BIT_CROWN		16
#define BIT_L2NDARM		32
#define BIT_SPEAR		64
#define BIT_FEMHAND		128
#define BIT_SWORD		256
#define BIT_STAFF		512
#define BIT_GEM			1024
#define BIT_R2NDARM		2048
#define BIT_RWINGS		4096	
#define BIT_LWINGS		8192
#define BIT_RLEG		16384

#define TC_ATK_STAB	1
#define TC_ATK_HACK	2

#define TC_SPL_FIRE		1
#define TC_SPL_GLOW		2
#define TC_SPL_FIRE2	3

enum 
{
	FX_I_SWORD,
	FX_I_SPEAR,
	FX_I_SP_MSL_HIT,
	FX_I_GLOBE,
	FX_I_GLOW,
	FX_I_STAFF,
	FX_I_ST_MSL_HIT,
	FX_I_RREFS,
	FX_I_SPEAR2,
	FX_I_SP_MSL_HIT2
};
