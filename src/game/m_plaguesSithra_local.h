//m_plagueSsithra.h


typedef enum AnimID_e
{
	ANIM_STAND1,
	ANIM_WALK1,
	ANIM_BACKPEDAL,
	ANIM_BOUND,
	ANIM_DEATH_A,
	ANIM_DEATH_B,
	ANIM_DIVE,
	ANIM_DUCKSHOOT,
	ANIM_DUCK,
	ANIM_GALLOP,
	ANIM_FJUMP,
	ANIM_IDLEBASIC,
	ANIM_IDLERIGHT,
	ANIM_MELEE,
	ANIM_MELEE_STAND,
	ANIM_NAMOR,
	ANIM_PAIN_A,
	ANIM_SHOOT,
	ANIM_STARTLE,
	ANIM_SWIMFORWARD,
	ANIM_SWIMWANDER,
	ANIM_WATER_DEATH,//19
	ANIM_WATER_IDLE,//
	ANIM_WATER_PAIN_A,
	ANIM_WATER_PAIN_B,
	ANIM_WATER_SHOOT,
	ANIM_RUN1,
	ANIM_SPINRIGHT,
	ANIM_SPINRIGHT_GO,
	ANIM_SPINLEFT,
	ANIM_SPINLEFT_GO,
	ANIM_FACEANDNAMOR,
	ANIM_DEAD_A,
	ANIM_LOOKRIGHT,
	ANIM_LOOKLEFT,
	ANIM_TRANSUP,
	ANIM_TRANSDOWN,
	ANIM_HEADLESS,
	ANIM_HEADLESSLOOP,
	ANIM_DEATH_C,
	ANIM_DEAD_B,
	ANIM_DEAD_WATER,
	ANIM_SLICED,
	ANIM_DELAY,
	ANIM_DUCKLOOP,
	ANIM_UNDUCK,
	ANIM_LUNGE,
	NUM_ANIMS
} AnimID_t;

typedef enum SoundID_e
{
	SND_PAIN1,//
	SND_PAIN2,//
	SND_DIE,//
	SND_GIB,//
	SND_SWIPE,//
	SND_SWIPEHIT,//
	SND_ARROW1,//
	SND_ARROW2,//
	SND_GROWL1,//hiss
	SND_GROWL2,//shriek
	SND_GROWL3,//grunt
	SND_INWATER,//
	SND_NAMOR,//
	SND_LAND,//???
	SND_SWIM,//
	SND_SWIM2,

	//Voices
	SND_SIGHT1,
	SND_SIGHT2,
	SND_SIGHT3,
	SND_SIGHT4,
	SND_SIGHT5,
	SND_SIGHT6,
	
	SND_ARROW_CHARGE,
	SND_ARROW_FIRE,

	NUM_SOUNDS
} SoundID_t;

extern animmove_t ssithra_move_idle1;
extern animmove_t ssithra_move_walk1;
extern animmove_t ssithra_move_backpedal1;
extern animmove_t ssithra_move_bound1;
extern animmove_t ssithra_move_death_a1;
extern animmove_t ssithra_move_death_b1;
extern animmove_t ssithra_move_dive1;
extern animmove_t ssithra_move_duckshoot1;
extern animmove_t ssithra_move_duck1;
extern animmove_t ssithra_move_gallop1;
extern animmove_t ssithra_move_fjump;
extern animmove_t ssithra_move_idlebasic1;
extern animmove_t ssithra_move_idleright1;
extern animmove_t ssithra_move_melee1;
extern animmove_t ssithra_move_meleest;
extern animmove_t ssithra_move_namor1;
extern animmove_t ssithra_move_pain_a1;
extern animmove_t ssithra_move_shoot1;
extern animmove_t ssithra_move_startle1;
extern animmove_t ssithra_move_swimforward1;
extern animmove_t ssithra_move_swimwander;
extern animmove_t ssithra_move_water_death1;
extern animmove_t ssithra_move_water_idle1;
extern animmove_t ssithra_move_water_pain_a1;
extern animmove_t ssithra_move_water_pain_b1;
extern animmove_t ssithra_move_water_shoot1;
extern animmove_t ssithra_move_run1;
extern animmove_t ssithra_move_spinright;
extern animmove_t ssithra_move_spinright_go;
extern animmove_t ssithra_move_spinleft;
extern animmove_t ssithra_move_spinleft_go;
extern animmove_t ssithra_move_faceandnamor;
extern animmove_t ssithra_move_dead;
extern animmove_t ssithra_move_lookright;
extern animmove_t ssithra_move_lookleft;
extern animmove_t ssithra_move_transup;
extern animmove_t ssithra_move_transdown;
extern animmove_t ssithra_move_headless;
extern animmove_t ssithra_move_headlessloop;
extern animmove_t ssithra_move_death_c;
extern animmove_t ssithra_move_dead_a;
extern animmove_t ssithra_move_dead_b;
extern animmove_t ssithra_move_dead_water;
extern animmove_t ssithra_move_sliced;
extern animmove_t ssithra_move_delay;
extern animmove_t ssithra_move_duckloop;
extern animmove_t ssithra_move_unduck;
extern animmove_t ssithra_move_lunge;

void ssithra_stand(edict_t *self, G_Message_t *msg);
void ssithra_walk(edict_t *self, G_Message_t *msg);
void ssithra_gallop(edict_t *self, G_Message_t *msg);
void ssithra_melee(edict_t *self, G_Message_t *msg);
void ssithra_idlebasic(edict_t *self, G_Message_t *msg);
void ssithra_dismember(edict_t *self, int damage, int HitLocation);
void ssithra_dismember_msg(edict_t *self, G_Message_t *msg);
void ssithra_pain(edict_t *self, G_Message_t *msg);
void ssithra_death(edict_t *self, G_Message_t *msg);
void ssithra_swimforward(edict_t *self, G_Message_t *msg);

void ssithra_dead(edict_t *self);

void ssithra_decide_stand(edict_t *self);
void ssithra_decide_gallop(edict_t *self);
void ssithra_decide_swimforward(edict_t *self);
void ssithra_decide_backpedal(edict_t *self);
//void ssithra_decide_walk(edict_t *self);
void ssithra_startle_go(edict_t *self);
//Actions
void ssithraSwipe(edict_t *self);
void ssithraArrow(edict_t *self);
void ssithraJump (edict_t *self, float upspd,float fwdspd,float rtspd);
void ssithraBoundCheck (edict_t *self);
void ssithraDiveCheck (edict_t *self);
void ssithraWaterDead(edict_t *self);
void ssithraForward (edict_t *self, float forwarddist);
void ssithraCheckLeaveWaterSplash (edict_t *self);
void ssithraCheckHitWaterSplash (edict_t *self);
void ssithraNamorJump (edict_t *self);
void ssithraCheckRipple (edict_t *self);
qboolean ai_checkattack (edict_t *self, float dist);
void ai_spin (edict_t *self, float amount);
void ssithra_spinright_go(edict_t *self);
void ssithra_spinleft_go(edict_t *self);
void ssithraCheckFacedNamor (edict_t *self);
void ssithra_check_namor(edict_t *self);
void ssithraVOfs(edict_t *self, float pofs, float yofs, float rofs);
void ssithraPanicArrow(edict_t *self);
void ssithra_pain_react (edict_t *self);
void ssithra_water_shoot (edict_t *self);
void ssithraCollapse(edict_t *self);
void ssithraKillSelf (edict_t *self);
void ssithraSound(edict_t *self, float soundnum, float channel, float attenuation);
qboolean SsithraCheckMood (edict_t *self);
void ssithraMsgJump(edict_t *self, G_Message_t *msg);
void MG_InitMoods(edict_t *self);
void ssithraApplyJump (edict_t *self);
void ssithraCheckDuckArrow (edict_t *self);
void ssithraCheckUnDuck (edict_t *self);
void ssithraJumpEvade (edict_t *self);
void ssithraCrouch (edict_t *self);
void ssithraUnCrouch(edict_t *self);
void ssithraCheckLoop (edict_t *self);
void ssithraGrowlSound(edict_t *self);
void ssithraStartDuckArrow(edict_t *self);


#define BIT_POLY				0
#define BIT_LOWERTORSO			1
#define BIT_CAPLOWERTORSO		2
#define BIT_LEFTLEG				4
#define BIT_RIGHTLEG			8
#define BIT_UPPERTORSO			16
#define BIT_CAPTOPUPPERTORSO	32
#define BIT_CAPBOTTOMUPPERTORSO	64
#define BIT_LEFTARM				128
#define BIT_RIGHTARM			256
#define BIT_HEAD				512
#define BIT_CENTERSPIKE			1024
#define BIT_LEFT1SPIKE			2048
#define BIT_RIGHT1SPIKE			4096
#define BIT_RIGHT2SPIKE			8192
#define BIT_CAPHEAD				16384
