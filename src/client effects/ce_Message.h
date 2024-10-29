#ifndef CE_MESSAGE_H
#define CE_MESSAGE_H

#include "SinglyLinkedList.h"

typedef enum CE_MsgID_e
{
	MSG_COLLISION = 0, // (parm1) trace_t *trace -- the trace matching the collision, valid only on the frame of collision
	NUM_MESSAGES
} CE_MsgID_t;

typedef struct Message_s
{
	CE_MsgID_t ID;
	SinglyLinkedList_t parms;
} CE_Message_t;

typedef void (*CE_MessageHandler_t)(struct client_entity_s *self, CE_Message_t *msg);
typedef void (*CE_MsgReceiver_t)(struct client_entity_s *self, CE_Message_t *msg);

void InitMsgMngr();
void ReleaseMsgMngr();

void QPostMessage(struct client_entity_s *to, CE_MsgID_t ID, char *format, ...);
int ParseMsgParms(CE_Message_t *this, char *format, ...);
void ProcessMessages(struct client_entity_s *this);
void ClearMessageQueue(struct client_entity_s *this);

#endif