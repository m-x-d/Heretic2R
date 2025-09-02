//
// g_DefaultMessageHandler.c
//
// Copyright 1998 Raven Software
//

#include "g_DefaultMessageHandler.h"
#include "g_local.h"
#include "Utilities.h"

G_MsgReceiver_t DefaultMessageReceivers[NUM_MESSAGES] =
{
	NULL,						// MSG_STAND
	NULL,						// MSG_CROUCH
	NULL,						// MSG_DUCKDOWN
	NULL,						// MSG_DUCKHOLD
	NULL,						// MSG_DUCKUP
	NULL,						// MSG_WALK
	NULL,						// MSG_RUN
	NULL,						// MSG_JUMP
	NULL,						// MSG_MELEE
	NULL,						// MSG_MISSILE
	NULL,						// MSG_WATCH
	NULL,						// MSG_EAT
	NULL,						// MSG_PAIN
	NULL,						// MSG_DEATH
	NULL,						// MSG_FLY
	NULL,						// MSG_FLYBACK
	NULL,						// MSG_HOVER
	NULL,						// MSG_FLEE
	NULL,						// MSG_FLYATTACK
	DefaultReceiver_Repulse,	// MSG_REPULSE
	NULL,						// MSG_IDLE
	NULL,						// MSG_TOUCH
	NULL,						// MSG_FALLBACK
	NULL,						// MSG_SEARCH
	NULL,						// MSG_DODGE
	NULL,						// MSG_ATTACK
	NULL,						// MSG_SIGHT
	NULL,						// MSG_TURN
	NULL,						// MSG_TURNLEFT
	NULL,						// MSG_TURNRIGHT
	NULL, 						// MSG_BLOCKED

	NULL,						// G_MSG_KNOCKEDBACK
	NULL,						// G_MSG_RESTSTATE
	DefaultReceiver_SetAnim,	// G_MSG_SET_ANIM
	DefaultReceiver_RemoveSelf,	// G_MSG_REMOVESELF
	DefaultReceiver_Suspend,	// G_MSG_SUSPEND
	DefaultReceiver_Unsuspend,	// G_MSG_UNSUSPEND

	NULL,						// MSG_VOICE_SIGHT
	NULL,						// MSG_VOICE_POLL
	NULL,						// MSG_VOICE_PUPPET
	NULL,						// MSG_CHECK_MOOD
};

void DefaultMsgHandler(edict_t* self, G_Message_t* msg)
{
	if (msg->ID == MSG_PAIN)
	{
		edict_t* target;
		edict_t* activator;
		ParseMsgParms(msg, "ee", &target, &activator);

		if (target->pain_target != NULL)
		{
			char* o_target = target->target;
			target->target = target->pain_target;

			G_UseTargets(target, activator);

			target->target = o_target;
			target->pain_target = NULL;
		}
	}

	const G_MsgReceiver_t receiver = classStatics[self->classID].msgReceivers[msg->ID];

	if (receiver != NULL)
		receiver(self, msg);
	else if (DefaultMessageReceivers[msg->ID] != NULL) // If and when there are a good number of defaults, change the NULL to be an Empty function.
		DefaultMessageReceivers[msg->ID](self, msg);
}

void DefaultReceiver_Repulse(edict_t* self, G_Message_t* msg)
{
	vec3_t vel;
	ParseMsgParms(msg, "fff", &vel[0], &vel[1], &vel[2]);
}

void DefaultReceiver_SetAnim(edict_t* self, G_Message_t* msg)
{
	if (msg->priority >= PRI_DIRECTIVE)
	{
		int id;
		ParseMsgParms(msg, "i", &id);
		SetAnim(self, id);
	}
}

void DefaultReceiver_RemoveSelf(edict_t* self, G_Message_t* msg)
{
	self->think = G_FreeEdict;
	self->nextthink = level.time + FRAMETIME; //BUGFIX: mxd. 0.0f in original logic.
}

void DefaultReceiver_Suspend(edict_t* self, G_Message_t* msg)
{
	self->flags |= FL_SUSPENDED;
}

void DefaultReceiver_Unsuspend(edict_t* self, G_Message_t* msg)
{
	self->flags &= ~FL_SUSPENDED;
}