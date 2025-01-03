//
// ce_DefaultMessageHandler.c
//
// Copyright 1998 Raven Software
//

#include "ce_DefaultMessageHandler.h"
#include "Client Entities.h"

void CE_DefaultMsgHandler(client_entity_t* self, CE_Message_t* msg)
{
	const CE_MsgReceiver_t receiver = ce_class_statics[self->classID].msgReceivers[msg->ID];

	if (receiver != NULL)
		receiver((struct client_entity_s*)self, msg);
}