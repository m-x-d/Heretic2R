//
// SinglyLinkedList.h
//
// Copyright 1998 Raven Software
//

#ifndef	SINGLYLINKEDLIST_H
#define	SINGLYLINKEDLIST_H

#include "H2Common.h"
#include "GenericUnions.h"

typedef struct SinglyLinkedList_s
{
	struct SinglyLinkedListNode_s *rearSentinel;
	struct SinglyLinkedListNode_s *front;
	struct SinglyLinkedListNode_s *current;
} SinglyLinkedList_t;

H2COMMON_API void SLList_DefaultCon(SinglyLinkedList_t *this_ptr);
H2COMMON_API void SLList_Des(SinglyLinkedList_t *this_ptr);
H2COMMON_API qboolean SLList_AtEnd(SinglyLinkedList_t *this_ptr);
H2COMMON_API qboolean SLList_AtLast(SinglyLinkedList_t *this_ptr);
H2COMMON_API qboolean SLList_IsEmpty(SinglyLinkedList_t *this_ptr);
H2COMMON_API const GenericUnion4_t SLList_Increment(SinglyLinkedList_t *this_ptr);
H2COMMON_API const GenericUnion4_t SLList_PostIncrement(SinglyLinkedList_t *this_ptr);
H2COMMON_API GenericUnion4_t SLList_Front(SinglyLinkedList_t *this_ptr);
H2COMMON_API GenericUnion4_t SLList_ReplaceCurrent(SinglyLinkedList_t *this_ptr, const GenericUnion4_t toReplace);
H2COMMON_API void SLList_PushEmpty(SinglyLinkedList_t *this_ptr);
H2COMMON_API void SLList_Push(SinglyLinkedList_t *this_ptr, const GenericUnion4_t toInsert);
H2COMMON_API GenericUnion4_t SLList_Pop(SinglyLinkedList_t *this_ptr);
H2COMMON_API void SLList_Chop(SinglyLinkedList_t *this_ptr);
H2COMMON_API void SLList_InsertAfter(SinglyLinkedList_t *this_ptr, const GenericUnion4_t toInsert);

#endif