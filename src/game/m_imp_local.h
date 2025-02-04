typedef enum AnimID_e
{
	ANIM_DIE,
	ANIM_FLY1, 
	ANIM_FLYBACK1, 
	ANIM_HOVER1, 
	ANIM_FIREBALL,
	ANIM_DIVE_GO,
	ANIM_DIVE_LOOP,
	ANIM_DIVE_END,
	ANIM_DIVE_OUT,
	ANIM_PAIN1,
	ANIM_TUMBLE,
	ANIM_PERCH,
	ANIM_TAKEOFF,
	ANIM_DUP,
	ANIM_DDOWN,
	NUM_ANIMS
} AnimID_t;

typedef enum SoundID_e
{
	SND_GIB,
	SND_FLAP,
	SND_SCREAM,
	SND_DIVE,
	SND_DEATH,
//	SND_PAIN1,
//	SND_PAIN2,
	SND_HIT,
	SND_ATTACK,
	SND_FIZZLE,
	SND_FBHIT,
//	SND_IDLE1,
//	SND_IDLE2,
	NUM_SOUNDS
} SoundID_t;

void move_imp_tumble(edict_t *self);

void imp_flap_noise(edict_t *self);
void imp_flap_fast_noise(edict_t *self);
void imp_dive_noise(edict_t *self);
void imp_death_noise(edict_t *self);
void imp_pain1_noise(edict_t *self);
void imp_pain2_noise(edict_t *self);
void imp_attack_noise(edict_t *self);

extern animmove_t imp_move_die1;
extern animmove_t imp_move_fly1;
extern animmove_t imp_move_flyback;
extern animmove_t imp_move_hover1;
extern animmove_t imp_move_fireball;
extern animmove_t imp_move_dive_go;
extern animmove_t imp_move_dive_loop;
extern animmove_t imp_move_dive_end;
extern animmove_t imp_move_dive_out;
extern animmove_t imp_move_pain1;
extern animmove_t imp_move_tumble;
extern animmove_t imp_move_takeoff;
//Pirches
extern animmove_t imp_move_perch;
extern animmove_t imp_move_dup;
extern animmove_t imp_move_ddown;


qboolean SV_StepDirection (edict_t *ent, float yaw, float dist);
qboolean SV_FlyDirection (edict_t *ent, vec3_t  *attitude, float dist);
qboolean SV_movestep (edict_t *ent, vec3_t move, qboolean relink);
qboolean ai_checkattack (edict_t *self, float dist);
void SV_NewChaseDir (edict_t *actor, edict_t *enemy, float dist);
void extrapolateFiredir (edict_t *self,vec3_t p1,float pspeed,edict_t *targ,float accept,vec3_t vec2);;

void imp_ai_fly   (edict_t *self, float fd, float rd, float ud);
void imp_ai_pirch (edict_t *self);
void imp_ai_hover (edict_t *self, float dist);

void imp_flight_model (edict_t *self);

void imp_dive_loop(edict_t *self);
void imp_hit_loop(edict_t *self);
void imp_check_dodge(edict_t *self);

void imp_dead (edict_t *self);
void imp_hit (edict_t *self, float stop_swoop);
void imp_pause(edict_t *self);
void imp_die(edict_t *self, G_Message_t *msg);
void imp_fly(edict_t *self, G_Message_t *msg);
void imp_fly_attack(edict_t *self, G_Message_t *msg);
void imp_flyback(edict_t *self);
void imp_hover(edict_t *self, G_Message_t *msg);
void imp_stand(edict_t *self, G_Message_t *msg);
void imp_hoverscream(edict_t *self, G_Message_t *msg);
void imp_dive(edict_t *self, G_Message_t *msg);
void imp_pain(edict_t *self, G_Message_t *msg);
void imp_perch(edict_t *self, G_Message_t *msg);

void SP_monster_imp(edict_t* self);
void imp_init(void);
void imp_pain_init(edict_t *self);

void imp_fix_angles(edict_t *self);

void move_imp_die(edict_t *self);
void move_imp_hover(edict_t *self);
void move_imp_fly(edict_t *self);
void move_imp_dive(edict_t *self);
void move_imp_dive_end(edict_t *self);

void imp_hover_anim(edict_t *self);
void imp_fireball (edict_t *self);
void imp_death_pain (edict_t *self, G_Message_t *msg);

