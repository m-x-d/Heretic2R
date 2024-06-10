//
// SinglyLinkedList.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "H2Common.h"
#include "GenericUnions.h"

typedef struct SinglyLinkedList_s
{
	struct SinglyLinkedListNode_s* rearSentinel;
	struct SinglyLinkedListNode_s* front;
	struct SinglyLinkedListNode_s* current;
} SinglyLinkedList_t;

H2COMMON_API void SLList_DefaultCon(SinglyLinkedList_t* this_ptr);
H2COMMON_API void SLList_Des(SinglyLinkedList_t* this_ptr);
H2COMMON_API qboolean SLList_AtEnd(const SinglyLinkedList_t* this_ptr);
H2COMMON_API qboolean SLList_AtLast(const SinglyLinkedList_t* this_ptr);
H2COMMON_API qboolean SLList_IsEmpty(const SinglyLinkedList_t* this_ptr);
H2COMMON_API GenericUnion4_t SLList_Increment(SinglyLinkedList_t* this_ptr);
H2COMMON_API GenericUnion4_t SLList_PostIncrement(SinglyLinkedList_t* this_ptr);
H2COMMON_API GenericUnion4_t SLList_Front(SinglyLinkedList_t* this_ptr);
H2COMMON_API GenericUnion4_t SLList_ReplaceCurrent(const SinglyLinkedList_t* this_ptr, GenericUnion4_t to_replace);
H2COMMON_API void SLList_PushEmpty(SinglyLinkedList_t* this_ptr);
H2COMMON_API void SLList_Push(SinglyLinkedList_t* this_ptr, GenericUnion4_t to_insert);
H2COMMON_API GenericUnion4_t SLList_Pop(SinglyLinkedList_t* this_ptr);
H2COMMON_API void SLList_Chop(SinglyLinkedList_t* this_ptr);
H2COMMON_API void SLList_InsertAfter(const SinglyLinkedList_t* this_ptr, GenericUnion4_t to_insert);