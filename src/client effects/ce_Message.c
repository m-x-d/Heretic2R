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

void CE_InitMsgMngr(void) //mxd. Named 'InitMsgMngr' in original logic.
{
#define MESSAGE_BLOCK_SIZE	256

	ResMngr_Con(&ce_messages_manager, sizeof(CE_Message_t), MESSAGE_BLOCK_SIZE);
}

void CE_ReleaseMsgMngr(void) //mxd. Named 'ReleaseMsgMngr' in original logic.
{
	ResMngr_Des(&ce_messages_manager);
}

void CE_PostMessage(client_entity_t* to, const CE_MsgID_t id, const char* format, ...) //mxd. Named 'QPostMessage' in original logic.
{
	assert(to->msgHandler != NULL); //mxd. Changed from check to assert.

	CE_Message_t* msg = ResMngr_AllocateResource(&ce_messages_manager, sizeof(CE_Message_t));
	SinglyLinkedList_t* parms = &msg->parms;

	SLList_DefaultCon(parms);
	SLList_PushEmpty(parms); // Should make a constructor fo CE_Message_t.

	msg->ID = id;

	if (format != NULL)
	{
		va_list marker;
		va_start(marker, format);
		MSG_SetParms(parms, format, marker);
		va_end(marker);
	}

	MSG_Queue(&to->msgQ, msg);
}

int CE_ParseMsgParms(CE_Message_t* msg, const char* format, ...) //mxd. Named 'ParseMsgParms' in original logic.
{
	assert(msg != NULL);

	SinglyLinkedList_t* parms = &msg->parms;
	SLList_Front(parms);

	va_list marker;
	va_start(marker, format);
	const int args_filled = MSG_GetParms(parms, format, marker);
	va_end(marker);

	return args_filled;
}

void CE_ProcessMessages(client_entity_t* self) //mxd. Named 'ProcessMessages' in original logic.
{
	assert(self->msgHandler != NULL);

	SinglyLinkedList_t* msgs = &self->msgQ.msgs;

	while (!SLList_IsEmpty(msgs))
	{
		CE_Message_t* msg = SLList_Pop(msgs).t_void_p;
		SinglyLinkedList_t* parms = &msg->parms;

		if (!SLList_AtLast(parms) && !SLList_AtEnd(parms))
			SLList_Chop(parms);

		self->msgHandler(self, msg);

		SLList_Des(parms);
		ResMngr_DeallocateResource(&ce_messages_manager, msg, sizeof(CE_Message_t));
	}
}

void CE_ClearMessageQueue(client_entity_t* self) //mxd. Named 'ClearMessageQueue' in original logic.
{
	SinglyLinkedList_t* msgs = &self->msgQ.msgs;

	while (!SLList_IsEmpty(msgs))
	{
		CE_Message_t* msg = SLList_Pop(msgs).t_void_p;
		SinglyLinkedList_t* parms = &msg->parms;

		SLList_Des(parms);
		ResMngr_DeallocateResource(&ce_messages_manager, msg, sizeof(CE_Message_t));
	}
}