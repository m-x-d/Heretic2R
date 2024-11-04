typedef enum AnimID_e
{
	ANIM_DIE,
	ANIM_FLY1, 
	ANIM_FLYBACK1, 
	ANIM_HOVER1, 
	ANIM_HOVERSCREAM,
	ANIM_DIVE_GO,
	ANIM_DIVE_LOOP,
	ANIM_DIVE_END,
	ANIM_PAIN1,
	ANIM_GLIDE1,
	ANIM_DIVE_TRANS,
	ANIM_HIT_LOOP,
	ANIM_TUMBLE,
	ANIM_PIRCH1,
	ANIM_PIRCH2,
	ANIM_PIRCH3,	
	ANIM_PIRCH4,
	ANIM_PIRCH5,
	ANIM_PIRCH6,
	ANIM_PIRCH7,
	ANIM_PIRCH8,
	ANIM_PIRCH9,
	ANIM_TAKEOFF,
	ANIM_CIRCLING,
	ANIM_CIRCLING_FLAP,
	NUM_ANIMS
} AnimID_t;

typedef enum SoundID_e
{
	SND_FLAP,
	SND_SCREAM,
	SND_FLAP_FAST,
	SND_DIVE,
	SND_DEATH,
	SND_PAIN1,
	SND_PAIN2,
	SND_ATTACK,
	SND_GIB,
	SND_IDLE1,
	SND_IDLE2,
	NUM_SOUNDS
} SoundID_t;

void move_harpy_tumble(edict_t *self);

void harpy_flap_noise(edict_t *self);
void harpy_flap_fast_noise(edict_t *self);
void harpy_dive_noise(edict_t *self);
void harpy_death_noise(edict_t *self);
void harpy_pain1_noise(edict_t *self);
void harpy_pain2_noise(edict_t *self);
void harpy_attack_noise(edict_t *self);

extern animmove_t harpy_move_die1;
extern animmove_t harpy_move_fly1;
extern animmove_t harpy_move_flyback1;
extern animmove_t harpy_move_hover1;
extern animmove_t harpy_move_hoverscream;
extern animmove_t harpy_move_dive_go;
extern animmove_t harpy_move_dive_loop;
extern animmove_t harpy_move_dive_end;
extern animmove_t harpy_move_pain1;
extern animmove_t harpy_move_glide;
extern animmove_t harpy_move_dive_trans;
extern animmove_t harpy_move_dive_hit_loop;
extern animmove_t harpy_move_tumble;
extern animmove_t harpy_move_takeoff;
extern animmove_t harpy_move_circle;
extern animmove_t harpy_move_circle_flap;
//Pirches
extern animmove_t harpy_move_pirch1_idle;
extern animmove_t harpy_move_pirch2_idle;
extern animmove_t harpy_move_pirch3_idle;
extern animmove_t harpy_move_pirch4_idle;
extern animmove_t harpy_move_pirch5_idle;
extern animmove_t harpy_move_pirch6_idle;
extern animmove_t harpy_move_pirch7_idle;
extern animmove_t harpy_move_pirch8_idle;
extern animmove_t harpy_move_pirch9_idle;

void harpy_dismember(edict_t *self, int damage, int HitLocation0);

qboolean SV_StepDirection (edict_t *ent, float yaw, float dist);
qboolean SV_FlyDirection (edict_t *ent, vec3_t  *attitude, float dist);
qboolean SV_movestep (edict_t *ent, vec3_t move, qboolean relink);
qboolean ai_checkattack (edict_t *self, float dist);
void SV_NewChaseDir (edict_t *actor, edict_t *enemy, float dist);

void harpy_ai_fly   (edict_t *self, float fd, float rd, float ud);
void harpy_ai_glide (edict_t *self, float fd, float rd, float ud);
void harpy_ai_circle (edict_t *self, float fd, float rd, float ud);
void harpy_ai_pirch (edict_t *self);
void harpy_ai_hover (edict_t *self, float dist);

void harpy_flight_model (edict_t *self);

void harpy_dive_loop(edict_t *self);
void harpy_hit_loop(edict_t *self);
void harpy_check_dodge(edict_t *self);

void harpy_dead (edict_t *self);
void harpy_hit (edict_t *self);
void harpy_pause(edict_t *self);
void harpy_die(edict_t *self, G_Message_t *msg);
void harpy_fly(edict_t *self, G_Message_t *msg);
void harpy_fly_attack(edict_t *self, G_Message_t *msg);
void harpy_flyback(edict_t *self);
void harpy_hover(edict_t *self, G_Message_t *msg);
void harpy_stand(edict_t *self, G_Message_t *msg);
void harpy_hoverscream(edict_t *self, G_Message_t *msg);
void harpy_dive(edict_t *self, G_Message_t *msg);
void harpy_pain(edict_t *self, G_Message_t *msg);
void harpy_perch(edict_t *self, G_Message_t *msg);

void SP_monster_harpy(edict_t* self);
void harpy_init(void);
void harpy_pain_init(edict_t *self);

void harpy_fix_angles(edict_t *self);

void move_harpy_die(edict_t *self);
void move_harpy_hover(edict_t *self);
void move_harpy_fly(edict_t *self);
void move_harpy_dive(edict_t *self);
void move_harpy_dive_end(edict_t *self);

void harpy_hover_anim(edict_t *self);

#define hl_backspikes		1
#define hl_head				2
#define hl_stinger			3
#define hl_lwing			4
#define hl_lefthand			5
#define hl_rwing			6
#define hl_righthand		7
#define hl_leftupperleg		8
#define hl_leftlowerleg		9
#define hl_rightupperleg	10
#define hl_rightlowerleg	11
#define hl_harpy_max		12

#define BPN_PAIN		0
#define BPN_HEAD		1
#define BPN_HORNS		2
#define BPN_HORN		4
#define BPN_BACKSPIKES	8
#define BPN_NECKSPIKES	16
#define BPN_LUARM		32
#define BPN_LLARM		64
#define BPN_LHAND		128
#define BPN_RUARM		256
#define BPN_RLARM		512
#define BPN_RHAND		1024
#define BPN_TAILSPIKES	2048
#define BPN_RWING		4096
#define BPN_LWING		8192
#define BPN_STINGER		16384

