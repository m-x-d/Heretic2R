//
// Message.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "SinglyLinkedList.h"

typedef struct MsgQueue_s
{
	SinglyLinkedList_t msgs;
} MsgQueue_t;

extern size_t MSG_SetParms(SinglyLinkedList_t *parms, const char *format, va_list marker);
extern int MSG_GetParms(SinglyLinkedList_t *parms, const char *format, va_list marker);
extern void MSG_Queue(MsgQueue_t *self, void *msg);