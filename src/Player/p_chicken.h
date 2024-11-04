#ifndef _P_CHICKEN_H_
#define _P_CHICKEN_H_

#include "p_types.h"

typedef enum SoundID_e
{
	SND_BITEHIT1,
	SND_BITEMISS1,
	SND_BITEMISS2,

	SND_SCRATCH,

	SND_HISS,

	SND_PAIN1,
	SND_PAIN2,

	SND_CHATTER1,
	SND_CHATTER2,
	SND_CHATTER3,

	SND_CHEW1,
	SND_CHEW2,
	SND_CHEW3,

	SND_SWALLOW,

	SND_DIE,
	SND_GIB,
	NUM_SOUNDS
} SoundID_t;

extern panimmove_t chicken_move_stand1;
extern panimmove_t chicken_move_walk;
extern panimmove_t chicken_move_run;
extern panimmove_t chicken_move_cluck;
extern panimmove_t chicken_move_attack;
extern panimmove_t chicken_move_eat;
extern panimmove_t chicken_move_jump;

//Dummy anim to catch sequence leaks
extern panimmove_t chickenp_move_dummy;

extern panimmove_t chickenp_move_stand;
extern panimmove_t chickenp_move_stand1;
extern panimmove_t chickenp_move_stand2;
extern panimmove_t chickenp_move_walk;
extern panimmove_t chickenp_move_run;
extern panimmove_t chickenp_move_back;
extern panimmove_t chickenp_move_runb;
extern panimmove_t chickenp_move_bite;
extern panimmove_t chickenp_move_strafel;
extern panimmove_t chickenp_move_strafer;
extern panimmove_t chickenp_move_jump;
extern panimmove_t chickenp_move_wjump;
extern panimmove_t chickenp_move_wjumpb;
extern panimmove_t chickenp_move_rjump;
extern panimmove_t chickenp_move_rjumpb;
extern panimmove_t chickenp_move_jump_loop;
extern panimmove_t chickenp_move_attack;
extern panimmove_t chickenp_move_jump_flap;
extern panimmove_t chickenp_move_runattack;
extern panimmove_t chickenp_move_swim_idle;
extern panimmove_t chickenp_move_swim;

void PlayerChickenBite(playerinfo_t *playerinfo);
void PlayerChickenSqueal(playerinfo_t *playerinfo);
int PlayerChickenJump(playerinfo_t *playerinfo);

void PlayerChickenFlap ( playerinfo_t *playerinfo );
void PlayerChickenCheckFlap ( playerinfo_t *playerinfo );

void ChickenAssert(playerinfo_t *playerinfo);
void PlayerChickenCluck(playerinfo_t *playerinfo, float force);

void ChickenStepSound(playerinfo_t *playerinfo, float value);

#endif // _P_CHICKEN_H_