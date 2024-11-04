
typedef enum AnimID_e
{
	ANIM_BITEUP,
	ANIM_BITELOW,
	ANIM_BITEUP2,
	ANIM_EATING_TWITCH,
	ANIM_EATING,
	ANIM_EATDOWN,
	ANIM_WALK,
	ANIM_WALKLEFT,
	ANIM_WALKRT,
	ANIM_JUMP,
	ANIM_FJUMP,
	ANIM_INAIR,
	ANIM_LAND,
	ANIM_GINAIR,
	ANIM_GLAND,
	ANIM_STAND,
	ANIM_DELAY,
	ANIM_DIE,
	ANIM_DIE_NORM,
	ANIM_CHARGE,
	ANIM_ROAR,
	ANIM_WALKATK,
	ANIM_STUN,
	ANIM_SNATCH,
	ANIM_READY_CATCH,
	ANIM_CATCH,
	ANIM_BITEUP_SFIN,
	ANIM_BITELOW_SFIN,
	ANIM_BITEUP2_SFIN,
	ANIM_QUICK_CHARGE,
	NUM_ANIMS
} AnimID_t;

typedef enum SoundID_e
{
	SND_ROAR,
	SND_ROAR2,
	SND_SNORT1,
	SND_SNORT2,

	SND_STEP1,
	SND_STEP2,
	SND_LAND,

	SND_GROWL1,
	SND_GROWL2,
	SND_GROWL3,

	SND_SWIPE,
	SND_SLAM,
	SND_SNATCH,
	SND_CHOMP,
	SND_TEAR1,
	SND_TEAR2,
	SND_THROW,
	SND_CATCH,
	SND_SWALLOW,

	SND_PAIN1,
	SND_PAIN2,
	SND_DIE,

	SND_CORVUS_SCREAM1,
	SND_CORVUS_SCREAM2,
	SND_CORVUS_SCREAM3,
	SND_CORVUS_DIE,
	NUM_SOUNDS
} SoundID_t;

extern animmove_t tbeast_move_biteup;
extern animmove_t tbeast_move_bitelow;
extern animmove_t tbeast_move_biteup2;
extern animmove_t tbeast_move_eating_twitch;
extern animmove_t tbeast_move_eating;
extern animmove_t tbeast_move_eatdown;
extern animmove_t tbeast_move_walk;
extern animmove_t tbeast_move_walkleft;
extern animmove_t tbeast_move_walkrt;
extern animmove_t tbeast_move_jump;
extern animmove_t tbeast_move_forced_jump;
extern animmove_t tbeast_move_inair;
extern animmove_t tbeast_move_land;
extern animmove_t tbeast_move_ginair;
extern animmove_t tbeast_move_gland;
extern animmove_t tbeast_move_stand;
extern animmove_t tbeast_move_delay;
extern animmove_t tbeast_move_die;
extern animmove_t tbeast_move_die_norm;
extern animmove_t tbeast_move_charge;
extern animmove_t tbeast_move_roar;
extern animmove_t tbeast_move_walkatk;
extern animmove_t tbeast_move_stun;
extern animmove_t tbeast_move_snatch;
extern animmove_t tbeast_move_ready_catch;
extern animmove_t tbeast_move_catch;
extern animmove_t tbeast_move_biteup_sfin;
extern animmove_t tbeast_move_bitelow_sfin;
extern animmove_t tbeast_move_biteup2_sfin;
extern animmove_t tbeast_move_quick_charge;


void tbeast_snort (edict_t *self);
void tbeast_growl (edict_t *self);
qboolean tbeastCheckMood(edict_t *self);
void tbeast_pause (edict_t *self);
void tbeastbite (edict_t *self, float ofsf, float ofsr, float ofsu);
void tbeast_land(edict_t *self);
void tbeast_roar(edict_t *self);
void tbeast_jump (edict_t *self);
void tbeast_apply_jump (edict_t *self);
void tbeast_ready_catch (edict_t *self);
void tbeast_throw_toy(edict_t *self);
void tbeast_toy_ofs(edict_t *self, float ofsf, float ofsr, float ofsu);
void tbeast_check_snatch(edict_t *self, float ofsf, float ofsr, float ofsu);
void tbeast_gore_toy(edict_t *self, float jumpht);
void tbeast_anger_sound (edict_t *self);
void tbeast_leap (edict_t *self, float fwdf, float rghtf, float upf);
void tbeast_eatorder (edict_t *self);
void tbeast_footstep (edict_t *self);
void tbeast_walkorder (edict_t *self);
void tbeast_standorder (edict_t *self);
void tbeast_dead(edict_t *self);
void tbeast_charge (edict_t *self, float force);
void tbeast_done_gore (edict_t *self);
void tbeast_run_think (edict_t *self, float dist);
float MG_ChangeYaw (edict_t *self);
qboolean MG_CheckBottom (edict_t *ent);
void tbeast_check_landed (edict_t *self);
void tbeast_inair (edict_t *self);
void tbeast_gcheck_landed (edict_t *self);
void tbeast_ginair (edict_t *self);
void tbeast_go_snatch (edict_t *self);
void tbeast_check_impacts(edict_t *self);
void tbeast_roar_knockdown(edict_t *self);
void tbeast_roar_short(edict_t *self);
void tbeast_gibs(edict_t *self);
edict_t *check_hit_beast(vec3_t start, vec3_t end);

enum
{
	FX_TB_PUFF,
	FX_TB_SNORT,
};

#define TB_HIBITE_F	150
#define TB_HIBITE_R	0
#define TB_HIBITE_U	108

#define TB_LOBITE_F	150
#define TB_LOBITE_R	0
#define TB_LOBITE_U	36

#define TB_WLKBITE_F	224
#define TB_WLKBITE_U	72

#define TBEAST_STD_MELEE_RNG	128
#define TBEAST_STD_MAXHOP_RNG	600

#define TB_JUMP_GRAV	0.8

#define TB_FWD_OFFSET	-64//-32
#define TB_UP_OFFSET	-32
#define TB_RT_OFFSET	-24
