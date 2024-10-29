#ifndef MESSAGE_H
#define MESSAGE_H

#include "SinglyLinkedList.h"

typedef struct MsgQueue_s
{
	SinglyLinkedList_t msgs;
} MsgQueue_t;

#ifdef _GAME_DLL
size_t SetParms(SinglyLinkedList_t *this_ptr, char *format, va_list marker, qboolean entsAsNames);
#else
size_t SetParms(SinglyLinkedList_t *this_ptr, char *format, va_list marker);
#endif

int GetParms(SinglyLinkedList_t *this_ptr, char *format, va_list marker);
void QueueMessage(MsgQueue_t *this_ptr, void *msg);

#endif