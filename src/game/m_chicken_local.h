typedef enum AnimID_e
{
	ANIM_STAND1,
	ANIM_WALK,
	ANIM_RUN,
	ANIM_CLUCK,
	ANIM_ATTACK,
	ANIM_EAT,
	ANIM_JUMP,
	NUM_ANIMS
} AnimID_t;

typedef enum SoundID_e
{
	//for the cluck animation
	SND_CLUCK1,
	SND_CLUCK2,

	//for the foot falls
	SND_CLAW,

	//for getting hit - even though right now, it dies immediately - they want this changed
	SND_PAIN1,
	SND_PAIN2,

	//for dying - we only ever get gibbed, so no other sound is required
	SND_DIE,

	//for biting the player
	SND_BITE1,
	SND_BITE2,
	SND_BITE3,

	//for pecking the ground
	SND_PECK1,
	SND_PECK2,

	//and lastly, I thought it might be cool to have some cries for when the chicken jumps
	SND_JUMP1,
	SND_JUMP2,
	SND_JUMP3,

	NUM_SOUNDS
} SoundID_t;

extern animmove_t chicken_move_stand1;
extern animmove_t chicken_move_walk;
extern animmove_t chicken_move_run;
extern animmove_t chicken_move_cluck;
extern animmove_t chicken_move_attack;
extern animmove_t chicken_move_eat;
extern animmove_t chicken_move_jump;

//Dummy anim to catch sequence leaks
extern animmove_t chickenp_move_dummy;

extern animmove_t chickenp_move_stand;
extern animmove_t chickenp_move_stand1;
extern animmove_t chickenp_move_stand2;
extern animmove_t chickenp_move_walk;
extern animmove_t chickenp_move_run;
extern animmove_t chickenp_move_back;
extern animmove_t chickenp_move_runb;
extern animmove_t chickenp_move_bite;
extern animmove_t chickenp_move_strafel;
extern animmove_t chickenp_move_strafer;
extern animmove_t chickenp_move_jump;
extern animmove_t chickenp_move_wjump;
extern animmove_t chickenp_move_wjumpb;
extern animmove_t chickenp_move_rjump;
extern animmove_t chickenp_move_rjumpb;
extern animmove_t chickenp_move_jump_loop;
extern animmove_t chickenp_move_attack;

void chicken_stand(edict_t *self, G_Message_t *msg);
void chicken_walk(edict_t *self, G_Message_t *msg);
void chicken_run(edict_t *self, G_Message_t *msg);
void chicken_attack(edict_t *self, G_Message_t *msg);
void chicken_death(edict_t *self, G_Message_t *msg);
void player_chicken_death(edict_t *self, G_Message_t *msg);
void chicken_eat(edict_t *self, G_Message_t *msg);
void chicken_cluck(edict_t *self, G_Message_t *msg);
void chicken_jump(edict_t *self, G_Message_t *msg);

void chicken_pause(edict_t *self);
void chicken_check(edict_t *self);
void chicken_eat_again(edict_t *self);
void chicken_bite (edict_t *self);

int make_chicken_jump(edict_t *self);
void chickenSound (edict_t *self, float channel, float sndindex, float atten);
