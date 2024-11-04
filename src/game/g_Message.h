#ifndef G_MESSAGE_H
#define G_MESSAGE_H

#include "SinglyLinkedList.h"

// Be sure to update DefaultMessageReceivers after adding a new message
typedef enum G_MsgID_e
{
	MSG_STAND = 0,
	MSG_CROUCH,
	MSG_DUCKDOWN, //going down to duck
	MSG_DUCKHOLD,
	MSG_DUCKUP,  //rising up from duck
	MSG_WALK,
	MSG_RUN,
	MSG_JUMP,
	MSG_MELEE,
	MSG_MISSILE,
	MSG_WATCH,//10
	MSG_EAT,
	MSG_PAIN,
	MSG_DEATH,
	MSG_FLY,
	MSG_FLYBACK,//15
	MSG_HOVER,
	MSG_FLEE,
	MSG_FLYATTACK, 
	MSG_REPULSE,
	//the following messages will probably be dumped when josh finishes his AI stuff
	//however, for the time being I need them for spreader to work for E3 --bb
	MSG_IDLE,//20		//these are basically more things that were originally function *'s
	MSG_TOUCH,
	MSG_FALLBACK,
	MSG_SEARCH,
	MSG_DODGE,
	MSG_ATTACK,//25
	MSG_SIGHT,
	MSG_TURN,
	MSG_TURNLEFT,
	MSG_TURNRIGHT,
	MSG_BLOCKED,//30
	//end E3 bb 

//--------------------------------------------
// High Level Utitlity messages
//--------------------------------------------

	G_MSG_KNOCKEDBACK,
	G_MSG_RESTSTATE,

//--------------------------------------------
// Low Level Utitlity messages
//--------------------------------------------
	G_MSG_SET_ANIM,
		// int anim ID
	G_MSG_REMOVESELF,		// the only time an ent should be freed is in response to this_ptr message,
							// which should set self->think to G_FreeEdict and nextthink to 0
	G_MSG_SUSPEND,//35
		// float time ( <= 0 indicates indefinite suspend )
	G_MSG_UNSUSPEND,

	
//--------------------------------------------
// Voice messages
//--------------------------------------------

	MSG_VOICE_SIGHT,	//Sight sounds
	MSG_VOICE_POLL,		//Polled for a reply sound
	MSG_VOICE_PUPPET,	//Forced sound by a trigger

	MSG_CHECK_MOOD,	//forced to check it's mood

	//end

	MSG_DISMEMBER,

	MSG_EVADE,				//if set, new ai_run will check and see if going to get hit and send evasion message with hitloc
	MSG_DEATH_PAIN,			//taking pain after death- for dismemberment or twitch

//--------------------------------------------
// Cinematic messages
//--------------------------------------------

	MSG_C_ACTION1,	// Differs between monsters
	MSG_C_ACTION2,	// 
	MSG_C_ACTION3,	// 
	MSG_C_ACTION4,	// 
	MSG_C_ACTION5,	// 
	MSG_C_ACTION6,	// 
	MSG_C_ACTION7,	// 
	MSG_C_ACTION8,	// 
	MSG_C_ACTION9,	// 
	MSG_C_ACTION10,	// 
	MSG_C_ACTION11,	// 
	MSG_C_ACTION12,	// 
	MSG_C_ACTION13,	// 
	MSG_C_ACTION14,	// 
	MSG_C_ACTION15,	// 
	MSG_C_ACTION16,	// 
	MSG_C_ACTION17,	// 
	MSG_C_ACTION18,	// 
	MSG_C_ACTION19,	// 
	MSG_C_ACTION20,	// 
	MSG_C_ATTACK1,
	MSG_C_ATTACK2,
	MSG_C_ATTACK3,
	MSG_C_BACKPEDAL1,
	MSG_C_DEATH1,
	MSG_C_DEATH2,
	MSG_C_DEATH3,
	MSG_C_DEATH4,
	MSG_C_GIB1,
	MSG_C_IDLE1,
	MSG_C_IDLE2,
	MSG_C_IDLE3,
	MSG_C_IDLE4,
	MSG_C_IDLE5,
	MSG_C_IDLE6,
	MSG_C_JUMP1,
	MSG_C_PAIN1,
	MSG_C_PAIN2,
	MSG_C_PAIN3,
	MSG_C_PIVOTLEFTGO,	
	MSG_C_PIVOTLEFT,	
	MSG_C_PIVOTLEFTSTOP,	
	MSG_C_PIVOTRIGHTGO,	
	MSG_C_PIVOTRIGHT,	
	MSG_C_PIVOTRIGHTSTOP,	
	MSG_C_RUN1,
	MSG_C_STEPLEFT,	
	MSG_C_STEPRIGHT,	
	MSG_C_THINKAGAIN, // Turns off Cinematic AI and puts monster into idle state
	MSG_C_TRANS1,
	MSG_C_TRANS2,
	MSG_C_TRANS3,
	MSG_C_TRANS4,
	MSG_C_TRANS5,
	MSG_C_TRANS6,
	MSG_C_WALKSTART,
	MSG_C_WALK1,
	MSG_C_WALK2,
	MSG_C_WALK3,
	MSG_C_WALK4,
	MSG_C_WALKSTOP1,
	MSG_C_WALKSTOP2,
	MSG_C_ATTACK4,
	MSG_C_ATTACK5,


	NUM_MESSAGES
} G_MsgID_t;

typedef enum G_MsgPriority_e
{
	PRI_SUGGESTION,	// message from high level ai that dosn't need to be accepted unless convienent
	PRI_ORDER,		// message from high level ai that should be accepted if possible
	PRI_DIRECTIVE,	// message from self that must be accepted unless it conflicts with a 
					// higher priority message
	PRI_PHYSICS,	// message that has physical meaning such as knockback or damage
	PRI_SYSTEM,		// message from the system that must be accepted
	NUM_MSG_PRIORITIES
} G_MsgPriority_t;

typedef struct G_Message_s
{
	G_MsgID_t ID;
	G_MsgPriority_t priority;
	SinglyLinkedList_t parms;
} G_Message_t;

typedef void (*G_MessageHandler_t)(struct edict_s *self, G_Message_t *msg);
typedef void (*G_MsgReceiver_t)(struct edict_s *self, G_Message_t *msg);

void G_Message_DefaultCon(G_Message_t *this_ptr);
G_Message_t *G_Message_new(G_MsgID_t ID, G_MsgPriority_t priority);
void G_Message_delete(G_Message_t *this_ptr);

void QPostMessage(struct edict_s *to, G_MsgID_t ID, G_MsgPriority_t priority, char *format, ...);

#ifdef __cplusplus

extern "C" 
{
	void PostGameMessage(struct edict_s *to, G_MsgID_t ID, G_MsgPriority_t priority, char *format, ...);
}

#endif

int ParseMsgParms(G_Message_t *this_ptr, char *format, ...);
void ProcessMessages(struct edict_s *this_ptr);
void ClearMessageQueue(struct edict_s *this_ptr);

#endif
