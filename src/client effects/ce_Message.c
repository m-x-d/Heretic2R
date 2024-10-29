//
// ce_Message.c
//
// Copyright 1998 Raven Software
//
// Heretic II
//

#include "ce_Message.h"
#include "Message.h"

#include "Client Entities.h"
#include "ResourceManager.h"
#include "SinglyLinkedList.h"

static ResourceManager_t MsgMngr;

void InitMsgMngr()
{
	const int MESSAGE_BLOCK_SIZE = 256;

	ResMngr_Con(&MsgMngr, sizeof(CE_Message_t), MESSAGE_BLOCK_SIZE);
}

void ReleaseMsgMngr()
{
	ResMngr_Des(&MsgMngr);
}

void QPostMessage(client_entity_t *to, CE_MsgID_t ID, char *format, ...)
{
	CE_Message_t *newMsg;
	qboolean append = false;
	SinglyLinkedList_t *parms;
	va_list marker;

	if(!to->msgHandler)	// everything should really have one, but at this point everything
						// doesn't so, the messages will never get popped of the queue
						// so don't push them on in the first place
	{
		return;
	}

	newMsg = ResMngr_AllocateResource(&MsgMngr, sizeof(CE_Message_t));

	parms = &newMsg->parms;

	// Fix Me !!!
	SLList_DefaultCon(parms);  // whoops, need to port object manager to C
	SLList_PushEmpty(parms);			// should make a constructor fo CE_Message_t too

	newMsg->ID = ID;

	if(format)
	{
		va_start(marker, format);

		SetParms(parms, format, marker);

		va_end(marker);
	}

	QueueMessage(&to->msgQ, newMsg);
}

int ParseMsgParms(CE_Message_t *this, char *format, ...)
{
	SinglyLinkedList_t *parms;
	va_list marker;
	int args_filled;

	assert(this);

	parms = &this->parms;

	SLList_Front(parms);

	va_start(marker, format);

	args_filled = GetParms(parms, format, marker);

	va_end(marker);

	return args_filled;
}

void ProcessMessages(client_entity_t *this)
{
	SinglyLinkedList_t *msgs;
	SinglyLinkedList_t *parms;
	CE_Message_t *msg;

	assert(this->msgHandler);

	msgs = &this->msgQ.msgs;

	while(!SLList_IsEmpty(msgs))
	{
		msg = SLList_Pop(msgs).t_void_p;

		parms = &msg->parms;

		if(!SLList_AtLast(parms) && !SLList_AtEnd(parms))
		{
			SLList_Chop(parms);
		}

		this->msgHandler(this, msg);

		// Fix Me !!!
		SLList_Des(parms); // whoops, need to port object manager to C

		ResMngr_DeallocateResource(&MsgMngr, msg, sizeof(CE_Message_t));
	}
}

void ClearMessageQueue(client_entity_t *this)
{
	SinglyLinkedList_t *msgs;
	SinglyLinkedList_t *parms;
	CE_Message_t *msg;

	msgs = &this->msgQ.msgs;

	while(!SLList_IsEmpty(msgs))
	{
		msg = SLList_Pop(msgs).t_void_p;

		parms = &msg->parms;

		// Fix Me !!!
		SLList_Des(parms); // whoops, need to port object manager to C

		ResMngr_DeallocateResource(&MsgMngr, msg, sizeof(CE_Message_t));
	}
}