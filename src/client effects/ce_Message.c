//
// ce_Message.c
//
// Copyright 1998 Raven Software
//

#include "ce_Message.h"
#include "Message.h"
#include "Client Entities.h"
#include "ResourceManager.h"
#include "SinglyLinkedList.h"

static ResourceManager_t ce_messages_manager;

void InitMsgMngr(void)
{
#define MESSAGE_BLOCK_SIZE	256

	ResMngr_Con(&ce_messages_manager, sizeof(CE_Message_t), MESSAGE_BLOCK_SIZE);
}

void ReleaseMsgMngr(void)
{
	ResMngr_Des(&ce_messages_manager);
}

void QPostMessage(client_entity_t* to, const CE_MsgID_t ID, char* format, ...)
{
	assert(to->msgHandler); //mxd. Changed from check to assert.

	CE_Message_t* msg = ResMngr_AllocateResource(&ce_messages_manager, sizeof(CE_Message_t));
	SinglyLinkedList_t* parms = &msg->parms;

	SLList_DefaultCon(parms);
	SLList_PushEmpty(parms); // Should make a constructor fo CE_Message_t.

	msg->ID = ID;

	if (format != NULL)
	{
		va_list marker;

		va_start(marker, format);
		SetParms(parms, format, marker);
		va_end(marker);
	}

	QueueMessage(&to->msgQ, msg);
}

int ParseMsgParms(CE_Message_t* this, char* format, ...)
{
	assert(this);

	SinglyLinkedList_t* parms = &this->parms;
	SLList_Front(parms);

	va_list marker;
	va_start(marker, format);
	const int args_filled = GetParms(parms, format, marker);
	va_end(marker);

	return args_filled;
}

void ProcessMessages(client_entity_t* this)
{
	assert(this->msgHandler);

	SinglyLinkedList_t* msgs = &this->msgQ.msgs;

	while (!SLList_IsEmpty(msgs))
	{
		CE_Message_t* msg = SLList_Pop(msgs).t_void_p;
		SinglyLinkedList_t* parms = &msg->parms;

		if (!SLList_AtLast(parms) && !SLList_AtEnd(parms))
			SLList_Chop(parms);

		this->msgHandler((struct client_entity_s*)this, msg);
		SLList_Des(parms);
		ResMngr_DeallocateResource(&ce_messages_manager, msg, sizeof(CE_Message_t));
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

		ResMngr_DeallocateResource(&ce_messages_manager, msg, sizeof(CE_Message_t));
	}
}