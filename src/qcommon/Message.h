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

extern size_t SetParms(SinglyLinkedList_t *this_ptr, const char *format, va_list marker);
extern int GetParms(SinglyLinkedList_t *this_ptr, const char *format, va_list marker);
extern void QueueMessage(MsgQueue_t *this_ptr, void *msg);