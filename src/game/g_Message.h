//
// g_Message.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "SinglyLinkedList.h"

// Be sure to update DefaultMessageReceivers after adding a new message
typedef enum G_MsgID_e
{
	MSG_STAND,
	MSG_CROUCH, //TODO: unused?
	MSG_DUCKDOWN, // Going down to duck. //TODO: unused?
	MSG_DUCKHOLD, //TODO: unused?
	MSG_DUCKUP, // Rising up from duck. //TODO: unused?
	MSG_WALK,
	MSG_RUN,
	MSG_JUMP,
	MSG_MELEE,
	MSG_MISSILE,
	MSG_WATCH,
	MSG_EAT,
	MSG_PAIN,
	MSG_DEATH,
	MSG_FLY, //TODO: handled, but never posted?
	MSG_FLYBACK, //TODO: unused?
	MSG_HOVER, //TODO: unused?
	MSG_FLEE, //TODO: unused?
	MSG_FLYATTACK, //TODO: unused?
	MSG_REPULSE, //TODO: only posted, never handled?

	// The following messages will probably be dumped when josh finishes his AI stuff.
	// However, for the time being I need them for spreader to work for E3 --bb
	MSG_IDLE, // These are basically more things that were originally function *'s. //TODO: unused?
	MSG_TOUCH, //TODO: unused?
	MSG_FALLBACK,
	MSG_SEARCH, //TODO: unused?
	MSG_DODGE, //TODO: unused?
	MSG_ATTACK, //TODO: unused?
	MSG_SIGHT, //TODO: handled in ElflordStaticsInit, but never posted?
	MSG_TURN, //TODO: unused?
	MSG_TURNLEFT, //TODO: unused?
	MSG_TURNRIGHT, //TODO: unused?
	MSG_BLOCKED, //TODO: only posted, never handled?

	// High-level utility messages.
	G_MSG_KNOCKEDBACK, //TODO: only posted, never handled?
	G_MSG_RESTSTATE, //TODO: only posted, never handled?

	// Low-level utility messages.
	G_MSG_SET_ANIM, // int anim ID //TODO: unused?
	G_MSG_REMOVESELF, // The only time an ent should be freed is in response to this_ptr message, which should set self->think to G_FreeEdict and nextthink to 0. //TODO: unused?
	G_MSG_SUSPEND, // float time (<= 0 indicates indefinite suspend).
	G_MSG_UNSUSPEND,

	// Voice messages.
	MSG_VOICE_SIGHT, // Sight sounds.
	MSG_VOICE_POLL, // Polled for a reply sound.
	MSG_VOICE_PUPPET, // Forced sound by a trigger.

	MSG_CHECK_MOOD, // Forced to check it's mood.

	MSG_DISMEMBER,
	MSG_EVADE, // If set, new ai_run will check and see if going to get hit and send evasion message with hit location.
	MSG_DEATH_PAIN, // Taking pain after death - for dismemberment or twitch.

	// Cinematic messages.
	MSG_C_ACTION1,	// Differs between monsters.
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
	MSG_C_THINKAGAIN, // Turns off Cinematic AI and puts monster into idle state.
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
	MSG_C_ATTACK5, //TODO: unused.

	NUM_MESSAGES
} G_MsgID_t;

typedef enum G_MsgPriority_e
{
	PRI_SUGGESTION,	// Message from high level ai that doesn't need to be accepted unless convenient. //TODO: unused.
	PRI_ORDER,		// Message from high level ai that should be accepted if possible.
	PRI_DIRECTIVE,	// Message from self that must be accepted unless it conflicts with a  higher priority message.
	PRI_PHYSICS,	// Message that has physical meaning such as knockback or damage.
	PRI_SYSTEM,		// Message from the system that must be accepted.

	NUM_MSG_PRIORITIES
} G_MsgPriority_t;

typedef struct G_Message_s
{
	G_MsgID_t ID;
	G_MsgPriority_t priority;
	SinglyLinkedList_t parms;
} G_Message_t;

typedef void (*G_MessageHandler_t)(struct edict_s* self, G_Message_t* msg);
typedef void (*G_MsgReceiver_t)(struct edict_s* self, G_Message_t* msg);

extern G_Message_t* G_Message_New(G_MsgID_t id, G_MsgPriority_t priority);
extern void G_Message_Delete(G_Message_t* msg);

#ifdef __cplusplus // Used by sc_CScript.cpp...
extern "C"
{
#endif
	extern void G_PostMessage(struct edict_s* to, G_MsgID_t id, G_MsgPriority_t priority, const char* format, ...);
#ifdef __cplusplus
}
#endif

extern int G_ParseMsgParms(G_Message_t* msg, char* format, ...);
extern void G_ProcessMessages(struct edict_s* self);
extern void G_ClearMessageQueues(void); //mxd
extern void G_InitMsgMngr(void); //mxd
extern void G_ReleaseMsgMngr(void); //mxd