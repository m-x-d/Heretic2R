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

#ifdef _GAME_DLL
size_t SetParms(SinglyLinkedList_t *this_ptr, const char *format, va_list marker, qboolean entsAsNames);
#else
size_t SetParms(SinglyLinkedList_t *this_ptr, const char *format, va_list marker);
#endif

int GetParms(SinglyLinkedList_t *this_ptr, const char *format, va_list marker);
void QueueMessage(MsgQueue_t *this_ptr, void *msg);