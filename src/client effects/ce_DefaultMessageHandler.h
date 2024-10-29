#ifndef	CE_DEFAULTMESSAGEHANDLER_H
#define	CE_DEFAULTMESSAGEHANDLER_H

#include "ce_Message.h"

extern CE_MsgReceiver_t DefaultMessageReceivers[NUM_MESSAGES];
extern void CE_DefaultMsgHandler(struct client_entity_s *self, CE_Message_t *msg);

#endif