#ifndef	G_DEFAULTMESSAGEHANDLER_H
#define	G_DEFAULTMESSAGEHANDLER_H

#include "g_Message.h"

extern G_MsgReceiver_t DefaultMessageReceivers[NUM_MESSAGES];
extern void DefaultMsgHandler(struct edict_s *self, G_Message_t *msg);
extern void DyingMsgHandler(struct edict_s *self, G_Message_t *msg);


void DefaultReceiver_Repulse(struct edict_s *self, G_Message_t *msg);
void DefaultReceiver_SetAnim(struct edict_s *self, G_Message_t *msg);
void DefaultReceiver_RemoveSelf(struct edict_s *self, G_Message_t *msg);
void DefaultReceiver_Suspend(struct edict_s *self, G_Message_t *msg);
void DefaultReceiver_Unsuspend(struct edict_s *self, G_Message_t *msg);

#endif