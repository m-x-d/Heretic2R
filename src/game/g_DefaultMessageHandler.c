#include "g_DefaultMessageHandler.h"
#include "g_Physics.h"
#include "g_local.h"
#include "Vector.h"
#include "Utilities.h"
#include "g_HitLocation.h"

G_MsgReceiver_t DefaultMessageReceivers[NUM_MESSAGES] =
{
	NULL,								// MSG_STAND
	NULL,								// MSG_CROUCH  
	NULL,								// MSG_DUCKDOWN,
	NULL,								// MSG_DUCKHOLD,
	NULL,								// MSG_DUCKUP,
	NULL,								// MSG_WALK    
	NULL,								// MSG_RUN      
	NULL,								// MSG_JUMP     
	NULL,								// MSG_MELEE    
	NULL,								// MSG_MISSILE  
	NULL,								// MSG_WATCH    
	NULL,								// MSG_EAT      
	NULL,								// MSG_PAIN     
	NULL,								// MSG_DEATH    
	NULL,								// MSG_FLY      
	NULL,								// MSG_FLYBACK  
	NULL,								// MSG_HOVER    
	NULL,								// MSG_FLEE     
	NULL,								// MSG_FLYATTACK
	DefaultReceiver_Repulse,			// MSG_REPULSE  
	NULL,								// MSG_IDLE,
	NULL,								// MSG_TOUCH,
	NULL,								// MSG_FALLBACK,
	NULL,								// MSG_SEARCH,
	NULL,								// MSG_DODGE,
	NULL,								// MSG_ATTACK,
	NULL,								// MSG_SIGHT,
	NULL,								// MSG_TURN,
	NULL,								// MSG_TURNLEFT,
	NULL,								// MSG_TURNRIGHT,
	NULL, 								// MSG_BLOCKED,
	NULL,								// G_MSG_KNOCKEDBACK,
	NULL,								// G_MSG_RESTSTATE,
	DefaultReceiver_SetAnim,			// G_MSG_SET_ANIM,
	DefaultReceiver_RemoveSelf,			// G_MSG_REMOVESELF,
	DefaultReceiver_Suspend,			// G_MSG_SUSPEND,
	DefaultReceiver_Unsuspend,			// G_MSG_UNSUSPEND,

	NULL,								// MSG_VOICE_SIGHT,
	NULL,								// MSG_VOICE_POLL,
	NULL,								// MSG_VOICE_PUPPET,
	NULL,								// MSG_CHECK_MOOD,
};

void DefaultMsgHandler(edict_t *self, G_Message_t *msg)
{
	G_MsgReceiver_t receiver;
	char	*o_target;

	if(msg->ID == MSG_PAIN)
	{
		edict_t *targ, *activator;

		ParseMsgParms(msg, "ee", &targ, &activator);

		if(targ->pain_target)
		{
			o_target = targ->target;
			targ->target = targ->pain_target;
			G_UseTargets(targ, activator);
			targ->target = o_target;
			targ->pain_target = NULL;
		}
	}

	receiver = classStatics[self->classID].msgReceivers[msg->ID];

	if(receiver)
	{
		receiver(self, msg);
	}
	else
	{
		// if and when there are a good number of defaults, change the NULL to be an Empty
		// function, overall that should be faster to just always call the function then
		// do the check
		receiver = DefaultMessageReceivers[msg->ID];

		if(receiver)
		{
			DefaultMessageReceivers[msg->ID](self, msg);
		}
	}
}

void DefaultReceiver_Repulse(edict_t *self, G_Message_t *msg)
{
	vec3_t		vel;

	ParseMsgParms(msg, "fff", &vel[0], &vel[1], &vel[2]);
//	VectorAdd(self->velocity, vel, self->velocity);
}

void DefaultReceiver_SetAnim(edict_t *self, G_Message_t *msg)
{
	int ID;

	if(msg->priority >= PRI_DIRECTIVE)
	{
		ParseMsgParms(msg, "i", &ID);

		SetAnim(self, ID);
	}
}

void DefaultReceiver_RemoveSelf(edict_t *self, G_Message_t *msg)
{
	self->think = G_FreeEdict;
	
	self->nextthink = 0;
}

void DefaultReceiver_Suspend(edict_t *self, G_Message_t *msg)
{
	self->flags |= FL_SUSPENDED;
}

void DefaultReceiver_Unsuspend(edict_t *self, G_Message_t *msg)
{
	self->flags &= ~FL_SUSPENDED;
}