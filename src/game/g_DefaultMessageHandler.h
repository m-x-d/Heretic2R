//
// g_DefaultMessageHandler.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "g_Message.h"

extern G_MsgReceiver_t DefaultMessageReceivers[NUM_MESSAGES];

extern void DefaultMsgHandler(struct edict_s* self, G_Message_t* msg);
extern void DismemberMsgHandler(edict_t* self, G_Message_t* msg);
extern void DyingMsgHandler(struct edict_s* self, G_Message_t* msg);
extern void DeadMsgHandler(edict_t* self, G_Message_t* msg);

extern void DefaultReceiver_Repulse(struct edict_s* self, G_Message_t* msg);
extern void DefaultReceiver_SetAnim(struct edict_s* self, G_Message_t* msg);
extern void DefaultReceiver_RemoveSelf(struct edict_s* self, G_Message_t* msg);
extern void DefaultReceiver_Suspend(struct edict_s* self, G_Message_t* msg);
extern void DefaultReceiver_Unsuspend(struct edict_s* self, G_Message_t* msg);