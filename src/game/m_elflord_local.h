//Elf Lord header

typedef enum AnimID_e
{
	ANIM_HOVER,				//hover
	ANIM_FLOAT_FORWARD,		//float forward
	ANIM_CHARGE,			//charge forward
	ANIM_CHARGE_BTRANS,		//transition to charge
	ANIM_FLOAT_BACK,		//float backwards (w / attack)
	ANIM_DODGE_RIGHT,		//dodge right
	ANIM_DODGE_LEFT,		//dodge	left
	ANIM_ATTACK_SOA_BTRANS,	//attack 1 (lightning sphere) beginning
	ANIM_ATTACK_SOA_LOOP,	//attack 1 (lightning sphere) loop
	ANIM_ATTACK_SOA_END,	//attack 1 (lightning sphere) ending
	ANIM_ATTACK_LS,			//attack 2 (light surge)
	ANIM_PAIN1,				//pain
	ANIM_DIE_BTRANS,		//death beginning
	ANIM_DIE_LOOP,			//death loop
	ANIM_SHIELD,			//shield
	ANIM_ATTACK,
	ANIM_MOVE,
	ANIM_WAIT,
	ANIM_COME_TO_LIFE,
	NUM_ANIMS
} AnimID_t;


typedef enum SoundID_e
{
	SND_PAIN1,
	SND_PAIN2,
	SND_DIE,

	SND_SACHARGE,
	SND_SAFIRE,
	SND_SAHIT,

	SND_PROJ1,
	SND_BEAM,

	NUM_SOUNDS
} SoundID_t;

extern animmove_t elflord_move_idle;
extern animmove_t elflord_move_run;
extern animmove_t elflord_move_charge;
extern animmove_t elflord_move_charge_trans;
extern animmove_t elflord_move_floatback;
extern animmove_t elflord_move_dodgeright;
extern animmove_t elflord_move_dodgeleft;
extern animmove_t elflord_move_soa_begin;
extern animmove_t elflord_move_soa_loop;
extern animmove_t elflord_move_soa_end;
extern animmove_t elflord_move_ls;
extern animmove_t elflord_move_pain;
extern animmove_t elflord_move_death_btrans;
extern animmove_t elflord_move_death_loop;
extern animmove_t elflord_move_shield;
extern animmove_t elflord_move_attack;
extern animmove_t elflord_move_move;
extern animmove_t elflord_move_wait;
extern animmove_t elflord_move_come_to_life;

qboolean elfLordCheckAttack (edict_t *self);

void elflord_decell(edict_t *self, float value);
void elflord_decide_movement (edict_t *self);
void elflord_ai_stand (edict_t *self, float dist);
void elflord_stand(edict_t *self, G_Message_t *msg);
void elflord_run(edict_t *self, G_Message_t *msg);
void elflord_death_start(edict_t *self, G_Message_t *msg);
void elflord_soa_start(edict_t *self, G_Message_t *msg);
void elflordRandomRushSound(edict_t *self);
void elflordSound(edict_t *self, float channel, float sndindex, float atten);
void elflord_flymove (edict_t *self, float dist);
void elfLordPause(edict_t *self);
void elfLordGoCharge(edict_t *self);
void elflord_soa_loop(edict_t *self);
void elflord_soa_end(edict_t *self);

void elflord_StartBeam(edict_t *self);
void elflord_EndBeam(edict_t *self);
void elford_Attack( edict_t *self );

void elflord_face(edict_t *self);
void elflord_track(edict_t *self);
void elflord_SlideMeter( edict_t *self );
void elflord_soa_go(edict_t *self);
void elflord_soa_charge(edict_t *self);
void elflord_FixAngles(edict_t *self);

void ai_charge2(edict_t *self, float dist);
float ai_face_goal(edict_t *self);
void MG_CheckEvade (edict_t *self);
