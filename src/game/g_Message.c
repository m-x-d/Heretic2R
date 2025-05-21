//
// g_Message.c
//
// Copyright 1998 Raven Software
//

#include "g_Message.h"
#include "Message.h"
#include "g_local.h"
#include "ResourceManager.h"
#include "SinglyLinkedList.h"

static ResourceManager_t messages_manager;

void InitMsgMngr(void)
{
#define MESSAGE_BLOCK_SIZE 256

	ResMngr_Con(&messages_manager, sizeof(G_Message_t), MESSAGE_BLOCK_SIZE);
}

void ReleaseMsgMngr(void)
{
	ResMngr_Des(&messages_manager);
}

static void G_Message_DefaultCon(G_Message_t* self)
{
	SinglyLinkedList_t* parms = &self->parms;

	SLList_DefaultCon(parms); // Need to port object manager to C.
	SLList_PushEmpty(parms); // Should make a constructor fo G_Message_t too.
}

G_Message_t* G_Message_New(const G_MsgID_t id, const G_MsgPriority_t priority)
{
	G_Message_t* msg = ResMngr_AllocateResource(&messages_manager, sizeof(G_Message_t));

	G_Message_DefaultCon(msg);
	msg->ID = id;
	msg->priority = priority;

	return msg;
}

static void G_Message_Des(G_Message_t* this)
{
	SLList_Des(&this->parms);
}

void G_Message_Delete(G_Message_t* msg)
{
	G_Message_Des(msg);
	ResMngr_DeallocateResource(&messages_manager, msg, sizeof(G_Message_t));
}

void QPostMessage(edict_t* to, const G_MsgID_t id, const G_MsgPriority_t priority, char* format, ...)
{
	// Everything should really have one, but at this point everything doesn't.
	// So, the messages will never get popped of the queue, so don't push them on in the first place.
	if (to->msgHandler == NULL)
		return;

	G_Message_t* msg = ResMngr_AllocateResource(&messages_manager, sizeof(G_Message_t));

	G_Message_DefaultCon(msg); // Need to port object manager to C.
	SinglyLinkedList_t* parms = &msg->parms;
	msg->ID = id;
	msg->priority = priority;

	if (format != NULL)
	{
		va_list marker;
		va_start(marker, format);
		SetParms(parms, format, marker);
		va_end(marker);
	}

	QueueMessage(&to->msgQ, msg);
}

int ParseMsgParms(G_Message_t* msg, char* format, ...)
{
	assert(msg != NULL);

	SinglyLinkedList_t* parms = &msg->parms;
	SLList_Front(parms);

	va_list marker;
	va_start(marker, format);
	const int args_filled = GetParms(parms, format, marker);
	va_end(marker);

	return args_filled;
}

void ProcessMessages(edict_t* self)
{
	assert(self->msgHandler != NULL);

	SinglyLinkedList_t* msgs = &self->msgQ.msgs;

	if (!SLList_IsEmpty(msgs))
		self->flags &= ~FL_SUSPENDED;

	while (!SLList_IsEmpty(msgs))
	{
		G_Message_t* msg = SLList_Pop(msgs).t_void_p;
		SinglyLinkedList_t* parms = &msg->parms;

		if (!SLList_AtLast(parms) && !SLList_AtEnd(parms))
			SLList_Chop(parms);

		self->msgHandler(self, msg);

		G_Message_Des(msg); // Need to port object manager to C.
		ResMngr_DeallocateResource(&messages_manager, msg, sizeof(G_Message_t));
	}
}

static void ClearMessageQueue(edict_t* self)
{
	SinglyLinkedList_t* msgs = &self->msgQ.msgs;

	// If either of these fire - do a rebuild all, otherwise it will try to free random memory and lead to an unstable system.
	assert(msgs->front != NULL);
	assert(msgs->rearSentinel != NULL);

	while (!SLList_IsEmpty(msgs))
	{
		G_Message_t* msg = SLList_Pop(msgs).t_void_p;
		SinglyLinkedList_t* parms = &msg->parms;

		SLList_Des(parms); // Need to port object manager to C.
		ResMngr_DeallocateResource(&messages_manager, msg, sizeof(G_Message_t));
	}
}

void ClearMessageQueues(void)
{
	edict_t* ent = &g_edicts[0];
	for (int i = 0; i < globals.num_edicts; i++, ent++)
		ClearMessageQueue(ent);
}