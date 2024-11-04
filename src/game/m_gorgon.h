typedef enum AnimID_e
{
	ANIM_STAND1,
	ANIM_STAND2,
	ANIM_STAND3,
	ANIM_STAND4,
	ANIM_WALK1,
	ANIM_WALK2,
	ANIM_WALK3,//10
	ANIM_MELEE1,
	ANIM_MELEE2,
	ANIM_MELEE3,
	ANIM_MELEE4,
	ANIM_MELEE5,
	ANIM_MELEE6,
	ANIM_MELEE7,
	ANIM_MELEE8,
	ANIM_MELEE9,
	ANIM_MELEE10,//20
	ANIM_FJUMP,
	ANIM_RUN1,
	ANIM_RUN2,
	ANIM_RUN3,
	ANIM_PAIN1,
	ANIM_PAIN2,
	ANIM_PAIN3,
	ANIM_DIE1,
	ANIM_DIE2,
	ANIM_SNATCH,//30
	ANIM_CATCH,
	ANIM_MISS,
	ANIM_READY_CATCH,
	ANIM_SNATCHHI,
	ANIM_SNATCHLOW,
	ANIM_SLIP,
	ANIM_SLIP_PAIN,
	ANIM_DELAY,
	ANIM_ROAR,
	ANIM_ROAR2,//40
	ANIM_LAND2,
	ANIM_LAND,
	ANIM_INAIR,

	ANIM_TO_SWIM,
	ANIM_SWIM,
	ANIM_SWIM_BITE_A,
	ANIM_SWIM_BITE_B,
	ANIM_OUT_WATER,

	ANIM_EAT_DOWN,
	ANIM_EAT_UP,//50
	ANIM_EAT_LOOP,//51
	ANIM_EAT_TEAR,//52
	ANIM_EAT_PULLBACK,//53
	ANIM_LOOK_AROUND,//54
	ANIM_EAT_LEFT,//55
	ANIM_EAT_RIGHT,//56
	ANIM_EAT_SNAP,//57
	ANIM_EAT_REACT,

	NUM_ANIMS
} AnimID_t;

typedef enum SoundID_e
{
	SND_PAIN1,
	SND_PAIN2,
	SND_GURGLE,
	SND_DIE,
	SND_GIB,
	SND_MELEEHIT1,
	SND_MELEEHIT2,
	SND_MELEEMISS1,
	SND_MELEEMISS2,
	SND_STEP1,
	SND_STEP2,
	SND_STEP3,
	SND_STEP4,
	SND_GROWL1,
	SND_GROWL2,
	SND_GROWL3,
	SND_LAND,
	NUM_SOUNDS
} SoundID_t;

extern animmove_t gorgon_move_stand1;
extern animmove_t gorgon_move_stand2;
extern animmove_t gorgon_move_stand3;
extern animmove_t gorgon_move_stand4;
extern animmove_t gorgon_move_walk;
extern animmove_t gorgon_move_walk2;
extern animmove_t gorgon_move_walk3;
extern animmove_t gorgon_move_melee1;
extern animmove_t gorgon_move_melee2;
extern animmove_t gorgon_move_melee3;
extern animmove_t gorgon_move_melee4;
extern animmove_t gorgon_move_melee5;
extern animmove_t gorgon_move_melee6;
extern animmove_t gorgon_move_melee7;
extern animmove_t gorgon_move_melee8;
extern animmove_t gorgon_move_melee9;
extern animmove_t gorgon_move_melee10;
extern animmove_t gorgon_move_fjump;
extern animmove_t gorgon_move_run1;
extern animmove_t gorgon_move_run2;
extern animmove_t gorgon_move_run3;
extern animmove_t gorgon_move_pain1;
extern animmove_t gorgon_move_pain2;
extern animmove_t gorgon_move_pain3;
extern animmove_t gorgon_move_die1;
extern animmove_t gorgon_move_die2;
extern animmove_t gorgon_move_jump;
extern animmove_t gorgon_move_catch;
extern animmove_t gorgon_move_snatch;
extern animmove_t gorgon_move_miss;
extern animmove_t gorgon_move_readycatch;
extern animmove_t gorgon_move_snatchhi;
extern animmove_t gorgon_move_snatchlow;
extern animmove_t gorgon_move_slip;
extern animmove_t gorgon_move_slip_pain;
extern animmove_t gorgon_move_delay;
extern animmove_t gorgon_move_roar;
extern animmove_t gorgon_move_roar2;
extern animmove_t gorgon_move_land2;
extern animmove_t gorgon_move_land;
extern animmove_t gorgon_move_inair;

extern animmove_t gorgon_move_to_swim;
extern animmove_t gorgon_move_swim;
extern animmove_t gorgon_move_swim_bite_a;
extern animmove_t gorgon_move_swim_bite_a;
extern animmove_t gorgon_move_outwater;

extern animmove_t gorgon_move_eat_down;
extern animmove_t gorgon_move_eat_up;
extern animmove_t gorgon_move_eat_loop;
extern animmove_t gorgon_move_eat_tear;
extern animmove_t gorgon_move_eat_pullback;
extern animmove_t gorgon_move_look_around;
extern animmove_t gorgon_move_eat_left;
extern animmove_t gorgon_move_eat_right;
extern animmove_t gorgon_move_eat_snap;
extern animmove_t gorgon_move_eat_react;


void gorgon_eat(edict_t *self, G_Message_t *msg);
void gorgon_stand(edict_t *self, G_Message_t *msg);
void gorgon_walk(edict_t *self, G_Message_t *msg);
void gorgon_melee(edict_t *self, G_Message_t *msg);
void gorgon_run(edict_t *self, G_Message_t *msg);
void gorgon_pain(edict_t *self, G_Message_t *msg);
void gorgon_death(edict_t *self, G_Message_t *msg);
void gorgonbite (edict_t *self);
void gorgon_footstep (edict_t *self);
void gorgon_eatorder (edict_t *self);
void gorgon_dead(edict_t *self);
void gorgon_hop (edict_t *self);
void gorgon_growl (edict_t *self);
void gorgon_jump (edict_t *self);
void gorgon_ready_catch (edict_t *self);
void gorgon_throw_toy(edict_t *self);
void gorgon_toy_ofs(edict_t *self, float ofsf, float ofsr, float ofsu);
void gorgon_check_snatch(edict_t *self, float ofsf, float ofsr, float ofsu);
void gorgon_gore_toy(edict_t *self, float jumpht);
void gorgon_miss_sound (edict_t *self);
void gorgon_anger_sound (edict_t *self);
void gorgon_go_snatch (edict_t *self);
void gorgon_done_gore (edict_t *self);
void gorgon_blocked (edict_t *self, trace_t *trace);
void gorgonRoll (edict_t *self, float rollangle);
void gorgonLerpOff (edict_t *self);
void gorgonLerpOn (edict_t *self);
void gorgonCheckSlip (edict_t *self);
void gorgonSlide (edict_t *self, float force);
qboolean gorgonCheckMood (edict_t *self);
void ai_goal_charge (edict_t *self, float dist);
void gorgonApplyJump (edict_t *self);
void gorgonRoar (edict_t *self);
void gorgon_roar_sound (edict_t *self);
void gorgon_go_inair (edict_t *self);
void gorgon_check_landed (edict_t *self);
void gorgonJumpOutWater (edict_t *self);
void gorgonGoSwim (edict_t *self);
void gorgonCheckInWater (edict_t *self);
void gorgon_ai_swim (edict_t *self, float dist);
void gorgonForward (edict_t *self, float dist);
void gorgonFixPitch (edict_t *self);
void gorgonZeroPitch (edict_t *self);
void gorgon_death2twitch (edict_t *self);
void gorgonChooseDeath (edict_t *self);
void gorgon_ai_eat(edict_t *self, float crap);

float MG_ChangePitchForZVel(edict_t *self, float speed, float cap_vel, float max_angle);
void MG_SetNormalizeVelToGoal(edict_t *self, vec3_t vec);
float MG_ChangeYaw (edict_t *self);
void MG_Pathfind(edict_t *self, qboolean check_clear_path);
float MG_ChangePitch(edict_t *self, float ideal, float speed);
void fish_under_water_wake (edict_t *self);
